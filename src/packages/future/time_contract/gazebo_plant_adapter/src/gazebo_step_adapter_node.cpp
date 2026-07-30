#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "ros_gz_interfaces/msg/world_statistics.hpp"
#include "ros_gz_interfaces/srv/control_world.hpp"

#include "simulation_coordinator_interfaces/msg/plant_step_done.hpp"
#include "simulation_coordinator_interfaces/msg/step_grant.hpp"

namespace gazebo_plant_adapter
{

using PlantStepDone =
  simulation_coordinator_interfaces::msg::PlantStepDone;

using StepGrant =
  simulation_coordinator_interfaces::msg::StepGrant;

using WorldStatistics =
  ros_gz_interfaces::msg::WorldStatistics;

using ControlWorld =
  ros_gz_interfaces::srv::ControlWorld;

class GazeboStepAdapterNode : public rclcpp::Node
{
public:
  GazeboStepAdapterNode()
  : Node("gazebo_step_adapter")
  {
    world_name_ = declare_parameter<std::string>(
      "world_name", "lockstep_world");

    const auto protocol_qos =
      rclcpp::QoS(rclcpp::KeepLast(10)).reliable();

    const std::string control_service =
      "/world/" + world_name_ + "/control";

    const std::string statistics_topic =
      "/world/" + world_name_ + "/stats";

    control_client_ =
      create_client<ControlWorld>(control_service);

    plant_step_done_pub_ =
      create_publisher<PlantStepDone>(
      "/simulation/lockstep/plant_step_done",
      protocol_qos);

    step_grant_sub_ =
      create_subscription<StepGrant>(
      "/simulation/lockstep/step_grant",
      protocol_qos,
      std::bind(
        &GazeboStepAdapterNode::on_step_grant,
        this,
        std::placeholders::_1));

    statistics_sub_ =
      create_subscription<WorldStatistics>(
      statistics_topic,
      protocol_qos,
      std::bind(
        &GazeboStepAdapterNode::on_statistics,
        this,
        std::placeholders::_1));

    RCLCPP_INFO(
      get_logger(),
      "Gazebo step adapter configured for world '%s'",
      world_name_.c_str());
  }

private:
  static std::int64_t to_nanoseconds(
    const builtin_interfaces::msg::Time & time)
  {
    constexpr std::int64_t kNsPerSecond =
      1'000'000'000;

    return
      static_cast<std::int64_t>(time.sec) *
      kNsPerSecond +
      static_cast<std::int64_t>(time.nanosec);
  }

  void on_step_grant(const StepGrant::SharedPtr msg)
  {
    if (step_pending_) {
      publish_failure(
        *msg,
        "Received StepGrant while another step is pending");
      return;
    }

    if (msg->command_id != msg->target_step) {
      publish_failure(
        *msg,
        "StepGrant command_id must equal target_step");
      return;
    }

    if (msg->target_step != last_backend_iteration_ + 1) {
      publish_failure(
        *msg,
        "StepGrant target is not the next backend iteration");
      return;
    }

    if (msg->target_time_ns !=
      last_backend_time_ns_ + msg->step_size_ns)
    {
      publish_failure(
        *msg,
        "StepGrant target time is inconsistent");
      return;
    }

    if (!control_client_->service_is_ready()) {
      publish_failure(
        *msg,
        "Gazebo ControlWorld service is not ready");
      return;
    }

    pending_grant_ = *msg;
    expected_backend_iteration_ =
      last_backend_iteration_ + 1;

    service_accepted_ = false;
    statistics_observed_ = false;
    step_pending_ = true;

    auto request =
      std::make_shared<ControlWorld::Request>();

    request->world_control.pause = true;
    request->world_control.step = true;
    request->world_control.multi_step = 0;

    control_client_->async_send_request(
      request,
      std::bind(
        &GazeboStepAdapterNode::on_control_response,
        this,
        std::placeholders::_1));

    RCLCPP_INFO(
      get_logger(),
      "Requested Gazebo step: generation=%llu, "
      "target_step=%llu, target_time_ns=%lld",
      static_cast<unsigned long long>(
        msg->generation),
      static_cast<unsigned long long>(
        msg->target_step),
      static_cast<long long>(
        msg->target_time_ns));
  }

  void on_control_response(
    rclcpp::Client<ControlWorld>::
    SharedFuture response_future)
  {
    if (!step_pending_) {
      return;
    }

    const auto response = response_future.get();

    if (!response->success) {
      publish_failure(
        pending_grant_,
        "Gazebo rejected ControlWorld request");

      clear_pending_step();
      return;
    }

    service_accepted_ = true;
    try_complete_step();
  }

  void on_statistics(
    const WorldStatistics::SharedPtr msg)
  {
    const auto backend_time_ns =
      to_nanoseconds(msg->sim_time);

    /*
     * 没有pending step时，只记录初始Gazebo状态。
     */
    if (!step_pending_) {
      last_backend_iteration_ = msg->iterations;
      last_backend_time_ns_ = backend_time_ns;
      return;
    }

    if (msg->iterations <
      expected_backend_iteration_)
    {
      return;
    }

    if (msg->iterations >
      expected_backend_iteration_)
    {
      publish_failure(
        pending_grant_,
        "Gazebo advanced more than one iteration");

      clear_pending_step();
      return;
    }

    if (backend_time_ns <
      pending_grant_.target_time_ns)
    {
      return;
    }

    if (backend_time_ns >
      pending_grant_.target_time_ns)
    {
      publish_failure(
        pending_grant_,
        "Gazebo advanced beyond target time");

      clear_pending_step();
      return;
    }

    observed_backend_iteration_ =
      msg->iterations;

    observed_backend_time_ns_ =
      backend_time_ns;

    statistics_observed_ = true;
    try_complete_step();
  }

  void try_complete_step()
  {
    if (!step_pending_ ||
      !service_accepted_ ||
      !statistics_observed_)
    {
      return;
    }

    PlantStepDone result;

    result.generation =
      pending_grant_.generation;

    result.target_step =
      pending_grant_.target_step;

    result.target_time_ns =
      pending_grant_.target_time_ns;

    result.backend_iteration =
      observed_backend_iteration_;

    result.backend_time_ns =
      observed_backend_time_ns_;

    /* 当前空世界中，该编号代表本step的安全空操作。 */
    result.applied_command_id =
      pending_grant_.command_id;

    result.observation_id =
      pending_grant_.target_step;

    result.success = true;
    result.detail = "Gazebo completed one step";

    plant_step_done_pub_->publish(result);

    last_backend_iteration_ =
      observed_backend_iteration_;

    last_backend_time_ns_ =
      observed_backend_time_ns_;

    RCLCPP_INFO(
      get_logger(),
      "Gazebo step completed: iteration=%llu, "
      "time_ns=%lld",
      static_cast<unsigned long long>(
        result.backend_iteration),
      static_cast<long long>(
        result.backend_time_ns));

    clear_pending_step();
  }

  void publish_failure(
    const StepGrant & grant,
    const std::string & detail)
  {
    PlantStepDone result;

    result.generation = grant.generation;
    result.target_step = grant.target_step;
    result.target_time_ns = grant.target_time_ns;

    result.backend_iteration =
      last_backend_iteration_;

    result.backend_time_ns =
      last_backend_time_ns_;

    result.applied_command_id = 0;
    result.observation_id = 0;

    result.success = false;
    result.detail = detail;

    plant_step_done_pub_->publish(result);

    RCLCPP_ERROR(
      get_logger(),
      "%s",
      detail.c_str());
  }

  void clear_pending_step()
  {
    step_pending_ = false;
    service_accepted_ = false;
    statistics_observed_ = false;
  }

  std::string world_name_;

  bool step_pending_{false};
  bool service_accepted_{false};
  bool statistics_observed_{false};

  std::uint64_t last_backend_iteration_{0};
  std::int64_t last_backend_time_ns_{0};

  std::uint64_t expected_backend_iteration_{0};

  std::uint64_t observed_backend_iteration_{0};
  std::int64_t observed_backend_time_ns_{0};

  StepGrant pending_grant_;

  rclcpp::Client<ControlWorld>::SharedPtr
  control_client_;

  rclcpp::Publisher<PlantStepDone>::SharedPtr
  plant_step_done_pub_;

  rclcpp::Subscription<StepGrant>::SharedPtr
  step_grant_sub_;

  rclcpp::Subscription<WorldStatistics>::SharedPtr
  statistics_sub_;
};

}  // namespace gazebo_plant_adapter

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<
    gazebo_plant_adapter::GazeboStepAdapterNode>();

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}