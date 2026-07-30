#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>

#include "rclcpp/rclcpp.hpp"
#include "rosgraph_msgs/msg/clock.hpp"

#include "simulation_coordinator/clock_authority.hpp"

#include "simulation_coordinator_interfaces/msg/command_queued.hpp"
#include "simulation_coordinator_interfaces/msg/control_ready.hpp"
#include "simulation_coordinator_interfaces/msg/observation_ready.hpp"
#include "simulation_coordinator_interfaces/msg/plant_step_done.hpp"
#include "simulation_coordinator_interfaces/msg/step_commit.hpp"
#include "simulation_coordinator_interfaces/msg/step_grant.hpp"

namespace simulation_coordinator
{

using CommandQueued =
  simulation_coordinator_interfaces::msg::CommandQueued;
using ControlReady =
  simulation_coordinator_interfaces::msg::ControlReady;
using ObservationReady =
  simulation_coordinator_interfaces::msg::ObservationReady;
using PlantStepDone =
  simulation_coordinator_interfaces::msg::PlantStepDone;
using StepCommit =
  simulation_coordinator_interfaces::msg::StepCommit;
using StepGrant =
  simulation_coordinator_interfaces::msg::StepGrant;

enum class ProtocolState
{
  WAIT_OBSERVATION,
  WAIT_CONTROL_BARRIER,
  WAIT_PLANT_RESULT,
  ABORTED
};

class SimulationCoordinatorNode : public rclcpp::Node
{
public:
  SimulationCoordinatorNode()
  : Node("simulation_coordinator")
  {
    const auto physics_dt_ns =
      declare_parameter<std::int64_t>(
      "physics_dt_ns", 1'000'000);

    authority_ =
      std::make_unique<ClockAuthority>(physics_dt_ns);

    if (!authority_->reset()) {
      throw std::runtime_error(
              "权威时钟初始化失败");
    }

    const auto protocol_qos = rclcpp::QoS(rclcpp::KeepLast(10)).reliable();

    step_grant_pub_ =
      create_publisher<StepGrant>(
      "/simulation/lockstep/step_grant",
      protocol_qos);

    step_commit_pub_ =
      create_publisher<StepCommit>(
      "/simulation/lockstep/step_commit",
      protocol_qos);

    clock_pub_ =
      create_publisher<rosgraph_msgs::msg::Clock>(
      "/clock",
      rclcpp::ClockQoS());

    observation_ready_sub_ =
      create_subscription<ObservationReady>(
      "/simulation/lockstep/observation_ready",
      protocol_qos,
      std::bind(
        &SimulationCoordinatorNode::on_observation_ready,
        this,
        std::placeholders::_1));

    control_ready_sub_ =
      create_subscription<ControlReady>(
      "/simulation/lockstep/control_ready",
      protocol_qos,
      std::bind(
        &SimulationCoordinatorNode::on_control_ready,
        this,
        std::placeholders::_1));

    command_queued_sub_ =
      create_subscription<CommandQueued>(
      "/simulation/lockstep/command_queued",
      protocol_qos,
      std::bind(
        &SimulationCoordinatorNode::on_command_queued,
        this,
        std::placeholders::_1));

    plant_step_done_sub_ =
      create_subscription<PlantStepDone>(
      "/simulation/lockstep/plant_step_done",
      protocol_qos,
      std::bind(
        &SimulationCoordinatorNode::on_plant_step_done,
        this,
        std::placeholders::_1));

    RCLCPP_INFO(
      get_logger(),
      "调度器就绪：generation=%llu，step=%llu，"
      "time_ns=%lld，dt_ns=%lld",
      static_cast<unsigned long long>(
        authority_->generation()),
      static_cast<unsigned long long>(
        authority_->committed_step()),
      static_cast<long long>(
        authority_->committed_time_ns()),
      static_cast<long long>(
        authority_->physics_dt_ns()));
  }

private:
  bool generation_matches(const std::uint64_t message_generation)
  {
    if (message_generation == authority_->generation()) {
      return true;
    }

    RCLCPP_WARN(
      get_logger(),
      "丢弃非当前代消息：message_generation=%llu，"
      "current_generation=%llu",
      static_cast<unsigned long long>(
        message_generation),
      static_cast<unsigned long long>(
        authority_->generation()));

    return false;
  }

  void abort_protocol(const char * reason)
  {
    authority_->abort();
    protocol_state_ = ProtocolState::ABORTED;

    RCLCPP_ERROR(
      get_logger(),
      "锁步协议终止：%s",
      reason);
  }

  void on_observation_ready(
    const ObservationReady::SharedPtr msg)
  {
    if (!generation_matches(msg->generation)) {
      return;
    }

    if (protocol_state_ !=
      ProtocolState::WAIT_OBSERVATION)
    {
      abort_protocol(
        "在错误状态收到ObservationReady");
      return;
    }

    if (msg->step_id != authority_->committed_step() ||
      msg->sim_time_ns !=
      authority_->committed_time_ns())
    {
      abort_protocol(
        "ObservationReady与已提交时间不一致");
      return;
    }

    if (expected_observation_id_.has_value() &&
      msg->observation_id !=
      expected_observation_id_.value())
    {
      abort_protocol(
        "ObservationReady的observation_id不符合预期");
      return;
    }

    expected_observation_id_.reset();
    reset_control_barrier();
    protocol_state_ = ProtocolState::WAIT_CONTROL_BARRIER;

    RCLCPP_INFO(
      get_logger(),
      "观测已接受：step=%llu，observation=%llu",
      static_cast<unsigned long long>(msg->step_id),
      static_cast<unsigned long long>(
        msg->observation_id));
  }

  void on_control_ready(
    const ControlReady::SharedPtr msg)
  {
    if (!generation_matches(msg->generation)) {
      return;
    }

    // 已提交step对应的迟到消息不再影响当前事务。
    if (msg->target_step <= authority_->committed_step()) {
      RCLCPP_WARN(
        get_logger(),
        "忽略旧ControlReady：target_step=%llu",
        static_cast<unsigned long long>(msg->target_step));
      return;
    }

    // StepGrant发出后，完全相同的消息按幂等重复处理。
    if (protocol_state_ == ProtocolState::WAIT_PLANT_RESULT) {
      if (msg->target_step == expected_target_step_ &&
        msg->command_id == expected_command_id_)
      {
        RCLCPP_WARN(get_logger(), "忽略在途重复ControlReady");
        return;
      }

      abort_protocol(
        "Plant推进期间收到冲突的ControlReady");
      return;
    }

    if (protocol_state_ != ProtocolState::WAIT_CONTROL_BARRIER) {
      abort_protocol(
        "在控制屏障之外收到ControlReady");
      return;
    }

    if (msg->source_step !=
      authority_->committed_step())
    {
      abort_protocol(
        "ControlReady的source_step不正确");
      return;
    }

    if (msg->target_step !=
      authority_->committed_step() + 1)
    {
      abort_protocol(
        "ControlReady的target_step不正确");
      return;
    }

    if (msg->command_id != msg->target_step) {
      abort_protocol(
        "ControlReady要求command_id等于target_step");
      return;
    }

    // 屏障内相同消息保持幂等，字段冲突则终止本代运行。
    if (pending_control_ready_.has_value()) {
      const auto & previous = pending_control_ready_.value();

      if (previous.source_step == msg->source_step &&
        previous.target_step == msg->target_step &&
        previous.command_id == msg->command_id)
      {
        RCLCPP_WARN(get_logger(), "忽略重复ControlReady");
        return;
      }

      abort_protocol("收到相互冲突的重复ControlReady");
      return;
    }

    pending_control_ready_ = *msg;

    RCLCPP_INFO(
      get_logger(),
      "ControlReady进入屏障：target_step=%llu，command_id=%llu",
      static_cast<unsigned long long>(
        msg->target_step),
      static_cast<unsigned long long>(
        msg->command_id));

    try_complete_control_barrier();
  }

  void on_command_queued(
    const CommandQueued::SharedPtr msg)
  {
    if (!generation_matches(msg->generation)) {
      return;
    }

    // 已提交step对应的迟到ACK直接忽略。
    if (msg->target_step <= authority_->committed_step()) {
      RCLCPP_WARN(
        get_logger(),
        "忽略旧CommandQueued：target_step=%llu",
        static_cast<unsigned long long>(msg->target_step));
      return;
    }

    // Plant推进期间只允许完全相同的ACK重复到达。
    if (protocol_state_ == ProtocolState::WAIT_PLANT_RESULT) {
      if (msg->accepted &&
        msg->target_step == expected_target_step_ &&
        msg->command_id == expected_command_id_)
      {
        RCLCPP_WARN(get_logger(), "忽略在途重复CommandQueued");
        return;
      }

      abort_protocol(
        "Plant推进期间收到冲突的CommandQueued");
      return;
    }

    if (protocol_state_ != ProtocolState::WAIT_CONTROL_BARRIER) {
      abort_protocol(
        "在控制屏障之外收到CommandQueued");
      return;
    }

    if (!msg->accepted) {
      abort_protocol(
        "Plant后端拒绝了控制指令");
      return;
    }

    if (msg->command_id != msg->target_step) {
      abort_protocol(
        "CommandQueued要求command_id等于target_step");
      return;
    }

    if (msg->target_step != authority_->committed_step() + 1)
    {
      abort_protocol(
        "CommandQueued的target_step不正确");
      return;
    }

    if (pending_command_queued_.has_value()) {
      const auto & previous = pending_command_queued_.value();

      if (previous.target_step == msg->target_step &&
        previous.command_id == msg->command_id &&
        previous.accepted == msg->accepted)
      {
        RCLCPP_WARN(get_logger(), "忽略重复CommandQueued");
        return;
      }

      abort_protocol("收到相互冲突的重复CommandQueued");
      return;
    }

    pending_command_queued_ = *msg;

    RCLCPP_INFO(
      get_logger(),
      "CommandQueued进入屏障：target_step=%llu，command_id=%llu",
      static_cast<unsigned long long>(msg->target_step),
      static_cast<unsigned long long>(msg->command_id));

    try_complete_control_barrier();
  }

  // 每个新观测对应一个全新的双输入屏障。
  void reset_control_barrier()
  {
    pending_control_ready_.reset();
    pending_command_queued_.reset();
  }

  // 两个参与节点的ACK均到齐且关联字段一致时，才允许生成下一步。
  void try_complete_control_barrier()
  {
    if (protocol_state_ != ProtocolState::WAIT_CONTROL_BARRIER ||
      !pending_control_ready_.has_value() ||
      !pending_command_queued_.has_value())
    {
      return;
    }

    const auto & control = pending_control_ready_.value();
    const auto & queued = pending_command_queued_.value();

    if (control.target_step != queued.target_step ||
      control.command_id != queued.command_id)
    {
      abort_protocol("ControlReady与CommandQueued的关联字段不一致");
      return;
    }

    expected_target_step_ = control.target_step;
    expected_command_id_ = control.command_id;

    if (!authority_->prepare_next_step()) {
      abort_protocol("权威时钟无法生成下一步目标");
      return;
    }

    if (authority_->pending_step() != expected_target_step_) {
      abort_protocol("权威时钟生成的step与控制屏障目标不一致");
      return;
    }

    StepGrant grant;
    grant.generation = authority_->generation();
    grant.target_step = authority_->pending_step();
    grant.target_time_ns = authority_->pending_time_ns();
    grant.step_size_ns = authority_->physics_dt_ns();
    grant.command_id = grant.target_step;

    step_grant_pub_->publish(grant);

    reset_control_barrier();

    protocol_state_ =
      ProtocolState::WAIT_PLANT_RESULT;

    RCLCPP_INFO(
      get_logger(),
      "控制屏障完成：step=%llu，command_id=%llu，time_ns=%lld",
      static_cast<unsigned long long>(
        grant.target_step),
      static_cast<unsigned long long>(
        grant.command_id),
      static_cast<long long>(
        grant.target_time_ns));
  }

  void on_plant_step_done(const PlantStepDone::SharedPtr msg)
  {
    if (!generation_matches(msg->generation)) {
      return;
    }

    if (protocol_state_ !=
      ProtocolState::WAIT_PLANT_RESULT)
    {
      abort_protocol(
        "在错误状态收到PlantStepDone");
      return;
    }

    if (!msg->success) {
      abort_protocol(
        "Plant后端报告物理步执行失败");
      return;
    }

    if (msg->target_step !=
      authority_->pending_step() ||
      msg->target_time_ns !=
      authority_->pending_time_ns())
    {
      abort_protocol(
        "Plant执行结果与待提交目标不一致");
      return;
    }

    /*
     * Plant Backend负责把自己的内部iteration归一化为
     * 当前generation中的锁步step。
     */
    if (msg->backend_iteration !=
      authority_->pending_step())
    {
      abort_protocol(
        "Plant后端推进到了非预期iteration");
      return;
    }

    if (msg->backend_time_ns !=
      authority_->pending_time_ns())
    {
      abort_protocol(
        "Plant后端时间与请求目标不一致");
      return;
    }

    if (msg->applied_command_id !=
      expected_command_id_)
    {
      abort_protocol(
        "Plant应用了非预期控制指令");
      return;
    }

    if (msg->applied_command_id != msg->target_step) {
      abort_protocol(
        "Plant要求applied_command_id等于target_step");
      return;
    }

    if (!authority_->commit_pending_step()) {
      abort_protocol(
        "权威时钟无法提交待定step");
      return;
    }

    StepCommit commit;
    commit.generation = authority_->generation();
    commit.step_id = authority_->committed_step();
    commit.sim_time_ns =
      authority_->committed_time_ns();
    commit.applied_command_id =
      msg->applied_command_id;
    commit.observation_id = msg->observation_id;

    step_commit_pub_->publish(commit);
    publish_clock(authority_->committed_time_ns());

    expected_observation_id_ = msg->observation_id;
    protocol_state_ =
      ProtocolState::WAIT_OBSERVATION;

    RCLCPP_INFO(
      get_logger(),
      "物理步已提交：step=%llu，time_ns=%lld",
      static_cast<unsigned long long>(
        commit.step_id),
      static_cast<long long>(
        commit.sim_time_ns));
  }

  void publish_clock(const SimTimeNs time_ns)
  {
    rosgraph_msgs::msg::Clock clock_msg;

    constexpr SimTimeNs kNanosecondsPerSecond =
      1'000'000'000;

    clock_msg.clock.sec =
      static_cast<std::int32_t>(
      time_ns / kNanosecondsPerSecond);

    clock_msg.clock.nanosec =
      static_cast<std::uint32_t>(
      time_ns % kNanosecondsPerSecond);

    clock_pub_->publish(clock_msg);
  }

  std::unique_ptr<ClockAuthority> authority_;

  ProtocolState protocol_state_{ProtocolState::WAIT_OBSERVATION};

  // 控制计算完成和Plant指令缓存完成来自不同参与节点，必须无序汇合。
  std::optional<ControlReady> pending_control_ready_;
  std::optional<CommandQueued> pending_command_queued_;

  std::uint64_t expected_command_id_{0};
  std::uint64_t expected_target_step_{0};

  std::optional<std::uint64_t>
  expected_observation_id_;

  rclcpp::Publisher<StepGrant>::SharedPtr  step_grant_pub_;

  rclcpp::Publisher<StepCommit>::SharedPtr  step_commit_pub_;

  rclcpp::Publisher<rosgraph_msgs::msg::Clock>::SharedPtr  clock_pub_;

  rclcpp::Subscription<ObservationReady>::SharedPtr  observation_ready_sub_;

  rclcpp::Subscription<ControlReady>::SharedPtr  control_ready_sub_;

  rclcpp::Subscription<CommandQueued>::SharedPtr  command_queued_sub_;

  rclcpp::Subscription<PlantStepDone>::SharedPtr  plant_step_done_sub_;
};

}  // namespace simulation_coordinator

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<
    simulation_coordinator::SimulationCoordinatorNode>();

  /*
   * 当前明确使用单线程执行器，
   * 保证所有协议回调串行处理。
   */
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}