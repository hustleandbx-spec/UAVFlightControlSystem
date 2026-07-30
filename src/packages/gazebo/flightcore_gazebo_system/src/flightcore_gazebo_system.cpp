// Gazebo Harmonic System Plugin for the FlightCore strict-lockstep test plant.
//
// Responsibilities:
//   * cache and identity-check one ActuatorCommand at a time;
//   * apply the command in PreUpdate for exactly its declared iteration;
//   * capture the resulting state in PostUpdate;
//   * publish plant completion ACKs, then wait for Coordinator CommitRelease;
//   * release IMU every epoch and GPS at the configured divider;
//   * publish SensorBatchPublished after all selected sensor publish() calls.
//
// Non-responsibilities:
//   * it never grants /clock;
//   * it never calls WorldControl;
//   * it never claims that DDS subscribers have consumed a sensor message.
//
// The plugin is both a Gazebo callback object and a ROS node. state_mutex_
// serializes Gazebo Pre/PostUpdate with the dedicated ROS executor thread.

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <stdexcept>
#include <thread>

#include "gz/math/Quaternion.hh"
#include "gz/math/Vector3.hh"
#include "gz/plugin/Register.hh"
#include "gz/sim/Link.hh"
#include "gz/sim/Model.hh"
#include "gz/sim/System.hh"
#include "rclcpp/rclcpp.hpp"
#include "sdf/Element.hh"

#include "flightcore_gazebo_msgs/msg/command_cached.hpp"
#include "flightcore_gazebo_msgs/msg/actuator_command.hpp"
#include "flightcore_gazebo_msgs/msg/commit_release.hpp"
#include "flightcore_gazebo_msgs/msg/plant_step_done.hpp"
#include "flightcore_gazebo_msgs/msg/result_ready.hpp"
#include "flightcore_gazebo_msgs/msg/sensor_batch_published.hpp"
#include "flightcore_gazebo_msgs/srv/prime_session.hpp"
#include "flightcore_msgs/msg/gps.hpp"
#include "flightcore_msgs/msg/imu.hpp"
#include "std_srvs/srv/trigger.hpp"

namespace flightcore_gazebo
{

using ActuatorCommand = flightcore_gazebo_msgs::msg::ActuatorCommand;
using CommandCached = flightcore_gazebo_msgs::msg::CommandCached;
using CommitRelease = flightcore_gazebo_msgs::msg::CommitRelease;
using PlantStepDone = flightcore_gazebo_msgs::msg::PlantStepDone;
using PrimeSession = flightcore_gazebo_msgs::srv::PrimeSession;
using ResultReady = flightcore_gazebo_msgs::msg::ResultReady;
using SensorBatchPublished =
  flightcore_gazebo_msgs::msg::SensorBatchPublished;
using Gps = flightcore_msgs::msg::Gps;
using Imu = flightcore_msgs::msg::Imu;
using Trigger = std_srvs::srv::Trigger;

namespace
{
constexpr double kGravity = 9.80665;
constexpr double kEarthRadiusM = 6378137.0;
constexpr double kReferenceLatitudeDeg = 0.0;
constexpr double kReferenceLongitudeDeg = 10.0;
constexpr double kReferenceAltitudeM = 0.0;
constexpr double kPi = 3.14159265358979323846;

template<typename T>
T read_sdf_value(
  const std::shared_ptr<const sdf::Element> & sdf,
  const std::string & name,
  const T & fallback)
{
  if (sdf && sdf->HasElement(name)) {
    return sdf->Get<T>(name);
  }
  return fallback;
}

std::int64_t to_nanoseconds(const std::chrono::steady_clock::duration value)
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(value).count();
}
}  // namespace

struct SensorSnapshot
{
  std::uint64_t iteration{0};
  std::int64_t sim_time_ns{0};
  std::array<float, 3> imu_accel_mps2{0.0F, 0.0F, -9.80665F};
  std::array<float, 3> imu_gyro_radps{0.0F, 0.0F, 0.0F};
  double latitude_deg{kReferenceLatitudeDeg};
  double longitude_deg{kReferenceLongitudeDeg};
  double altitude_m{kReferenceAltitudeM};
  std::array<float, 3> velocity_ned_mps{0.0F, 0.0F, 0.0F};
  std::array<float, 3> position_ned_m{0.0F, 0.0F, 0.0F};
  std::array<float, 4> attitude_quat_wxyz{1.0F, 0.0F, 0.0F, 0.0F};
};

struct PendingCommand
{
  std::uint64_t session_id{0};
  std::uint64_t source_step_id{0};
  std::uint64_t step_id{0};
  std::uint64_t command_id{0};
  std::uint64_t valid_from_iteration{0};
  bool armed{false};
  bool valid{false};
  std::array<float, 4> actuator_values{0.0F, 0.0F, 0.0F, 0.0F};
};

class FlightCoreGazeboSystem final:
  public gz::sim::System,
  public gz::sim::ISystemConfigure,
  public gz::sim::ISystemPreUpdate,
  public gz::sim::ISystemPostUpdate
{
public:
  ~FlightCoreGazeboSystem() override
  {
    if (executor_) {
      executor_->cancel();
    }
    if (executor_thread_.joinable()) {
      executor_thread_.join();
    }
  }

  void Configure(
    const gz::sim::Entity & entity,
    const std::shared_ptr<const sdf::Element> & sdf,
    gz::sim::EntityComponentManager & ecm,
    gz::sim::EventManager &) override
  {
    model_ = gz::sim::Model(entity);
    const auto base_link_name = read_sdf_value<std::string>(
      sdf, "base_link", "base_link");
    base_link_entity_ = model_.LinkByName(ecm, base_link_name);
    if (base_link_entity_ == gz::sim::kNullEntity) {
      throw std::runtime_error("未找到 FlightCore Gazebo base_link");
    }

    link_ = gz::sim::Link(base_link_entity_);
    link_.EnableVelocityChecks(ecm, true);

    world_name_ = read_sdf_value<std::string>(
      sdf, "world_name", "flightcore_world");
    max_thrust_n_ = read_sdf_value<double>(
      sdf, "max_thrust_n", 4.179446268);
    max_reaction_torque_nm_ = read_sdf_value<double>(
      sdf, "max_reaction_torque_nm", 0.055562);
    motor_arm_length_m_ = read_sdf_value<double>(
      sdf, "motor_arm_length_m", 0.2275);
    linear_drag_n_per_mps_ = read_sdf_value<double>(
      sdf, "linear_drag_n_per_mps", 1.2);
    gps_rate_divider_ = std::max<std::uint64_t>(
      1, read_sdf_value<std::uint64_t>(sdf, "gps_rate_divider", 200));
    service_timeout_ = std::chrono::milliseconds(
      read_sdf_value<int>(sdf, "service_timeout_ms", 5000));

    if (!rclcpp::ok()) {
      int argc = 0;
      char ** argv = nullptr;
      rclcpp::init(argc, argv);
    }

    node_ = std::make_shared<rclcpp::Node>("flightcore_gazebo_system");
    callback_group_ = node_->create_callback_group(
      rclcpp::CallbackGroupType::MutuallyExclusive);

    command_cached_pub_ = node_->create_publisher<CommandCached>(
      "/flightcore/gazebo/command_cached",
      rclcpp::QoS(rclcpp::KeepLast(8)).reliable());
    plant_step_done_pub_ = node_->create_publisher<PlantStepDone>(
      "/flightcore/gazebo/plant_step_done",
      rclcpp::QoS(rclcpp::KeepLast(8)).reliable());
    result_ready_pub_ = node_->create_publisher<ResultReady>(
      "/flightcore/gazebo/result_ready",
      rclcpp::QoS(rclcpp::KeepLast(8)).reliable());
    // Gazebo 是本拍 required_mask 的唯一权威。该消息只证明生产端已完成
    // publish；下一拍 /clock 必须等待 FlightCore 的 ObservationReady。
    sensor_batch_published_pub_ =
      node_->create_publisher<SensorBatchPublished>(
      "/flightcore/gazebo/sensor_batch_published",
      rclcpp::QoS(rclcpp::KeepLast(8)).reliable());

    imu_pub_ = node_->create_publisher<Imu>(
      "/flightcore/gazebo/imu",
      rclcpp::QoS(rclcpp::KeepLast(10)).reliable());
    gps_pub_ = node_->create_publisher<Gps>(
      "/flightcore/gazebo/gps",
      rclcpp::QoS(rclcpp::KeepLast(10)).reliable());
    actuator_command_sub_ = node_->create_subscription<ActuatorCommand>(
      "/flightcore/gazebo/actuator_command",
      rclcpp::QoS(rclcpp::KeepLast(4)).reliable(),
      std::bind(
        &FlightCoreGazeboSystem::on_blocking_actuator_command,
        this,
        std::placeholders::_1));
    commit_release_sub_ = node_->create_subscription<CommitRelease>(
      "/flightcore/gazebo/commit_release",
      rclcpp::QoS(rclcpp::KeepLast(8)).reliable(),
      std::bind(
        &FlightCoreGazeboSystem::on_commit_release,
        this,
        std::placeholders::_1));

    prime_service_ = node_->create_service<PrimeSession>(
      "/flightcore/gazebo/prime_session",
      std::bind(
        &FlightCoreGazeboSystem::on_prime_session,
        this,
        std::placeholders::_1,
        std::placeholders::_2),
      rclcpp::ServicesQoS(),
      callback_group_);
    blocking_status_service_ = node_->create_service<Trigger>(
      "/flightcore/gazebo/blocking_status",
      std::bind(
        &FlightCoreGazeboSystem::on_blocking_status,
        this,
        std::placeholders::_1,
        std::placeholders::_2),
      rclcpp::ServicesQoS(),
      callback_group_);
    // ROS callback 仅校验、缓存、ACK 后返回；WorldControl 由进程外
    // Coordinator 独占，插件执行器不承担全局推进语义。
    executor_ = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
    executor_->add_node(node_);
    executor_thread_ = std::thread([this]() {executor_->spin();});

    RCLCPP_INFO(
      node_->get_logger(),
      "FlightCore Gazebo 轻量锁步插件已加载，world=%s",
      world_name_.c_str());
  }

  void PreUpdate(
    const gz::sim::UpdateInfo & info,
    gz::sim::EntityComponentManager & ecm) override
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!command_pending_ || command_applied_ || info.paused) {
      return;
    }

    if (info.iterations < pending_command_.valid_from_iteration) {
      return;
    }
    if (info.iterations > pending_command_.valid_from_iteration) {
      transaction_error_ = "Gazebo PreUpdate 已越过目标 iteration";
      return;
    }

    apply_wrench(pending_command_, ecm);
    command_applied_ = true;
    applied_iteration_ = info.iterations;
  }

  void PostUpdate(
    const gz::sim::UpdateInfo & info,
    const gz::sim::EntityComponentManager & ecm) override
  {
    PlantStepDone plant_done;
    ResultReady result_ready;
    bool publish_completion = false;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      latest_snapshot_ = capture_snapshot(info, ecm);
      initial_snapshot_ready_ = true;
      state_cv_.notify_all();

      if (!command_pending_ || !command_applied_ || completion_announced_) {
        return;
      }
      if (info.iterations < applied_iteration_) {
        return;
      }
      if (info.iterations > applied_iteration_) {
        transaction_error_ = "Gazebo PostUpdate 已越过命令应用 iteration";
      }

      cached_snapshot_ = latest_snapshot_;
      result_payload_ready_ = true;
      completion_announced_ = true;

      plant_done.session_id = pending_command_.session_id;
      plant_done.step_id = pending_command_.step_id;
      plant_done.command_id = pending_command_.command_id;
      plant_done.iteration = latest_snapshot_.iteration;
      plant_done.sim_time_ns = latest_snapshot_.sim_time_ns;
      plant_done.accepted = transaction_error_.empty();
      plant_done.status = plant_done.accepted ?
        "PLANT_STEP_DONE" : transaction_error_;

      result_ready.session_id = plant_done.session_id;
      result_ready.step_id = plant_done.step_id;
      result_ready.command_id = plant_done.command_id;
      result_ready.iteration = plant_done.iteration;
      result_ready.sim_time_ns = plant_done.sim_time_ns;
      result_ready.accepted = plant_done.accepted;
      result_ready.status = result_ready.accepted ?
        "RESULT_READY" : transaction_error_;
      publish_completion = true;
    }
    if (publish_completion) {
      plant_step_done_pub_->publish(plant_done);
      result_ready_pub_->publish(result_ready);
    }
  }

private:

  std::uint8_t publish_sensor_snapshot(
    const SensorSnapshot & snapshot,
    const bool force_gps)
  {
    // The returned mask is the single source of truth for this observation
    // batch. PRIME forces both sensors so the model starts with an initialized
    // GPS hold value. Normal iterations always release IMU and only release
    // GPS at exact gps_rate_divider boundaries.
    constexpr std::uint8_t kImuMask = 0x01U;
    constexpr std::uint8_t kGpsMask = 0x02U;
    const auto seconds = snapshot.sim_time_ns / 1000000000LL;
    const auto nanoseconds = snapshot.sim_time_ns % 1000000000LL;
    const auto sequence = static_cast<std::uint32_t>(snapshot.iteration);

    Imu imu;
    imu.stamp.sec = static_cast<std::int32_t>(seconds);
    imu.stamp.nanosec = static_cast<std::uint32_t>(nanoseconds);
    imu.timestamp_sec = static_cast<double>(snapshot.sim_time_ns) * 1.0e-9;
    imu.sequence = sequence;
    imu.source_id = 1U;
    imu.valid = true;
    imu.accel_mps2 = snapshot.imu_accel_mps2;
    imu.gyro_radps = snapshot.imu_gyro_radps;
    imu_pub_->publish(imu);

    const bool publish_gps =
      force_gps || (snapshot.iteration % gps_rate_divider_) == 0U;
    if (!publish_gps) {
      return kImuMask;
    }

    Gps gps;
    gps.stamp = imu.stamp;
    gps.timestamp_sec = imu.timestamp_sec;
    gps.sequence = sequence;
    gps.source_id = 1U;
    gps.valid = true;
    gps.lat_deg = static_cast<float>(snapshot.latitude_deg);
    gps.lon_deg = static_cast<float>(snapshot.longitude_deg);
    gps.alt_m = static_cast<float>(snapshot.altitude_m);
    gps.velocity_ned_mps = snapshot.velocity_ned_mps;
    gps_pub_->publish(gps);
    return kImuMask | kGpsMask;
  }

  void publish_sensor_batch(
    const SensorSnapshot & snapshot,
    const std::uint64_t session_id,
    const std::uint64_t step_id,
    const std::uint64_t command_id,
    const std::uint8_t published_mask)
  {
    // required_mask currently equals published_mask because this lightweight
    // plant has no optional best-effort sensors. Keeping both fields explicit
    // preserves the semantic boundary: required is what FlightCore must wait
    // for; published is what Gazebo actually attempted to release.
    SensorBatchPublished batch;
    batch.session_id = session_id;
    batch.step_id = step_id;
    batch.command_id = command_id;
    batch.iteration = snapshot.iteration;
    batch.sim_time_ns = snapshot.sim_time_ns;
    batch.required_mask = published_mask;
    batch.published_mask = published_mask;
    batch.accepted = true;
    batch.status = "SENSOR_BATCH_PUBLISHED";
    sensor_batch_published_pub_->publish(batch);
    RCLCPP_INFO(
      node_->get_logger(),
      "SENSOR_BATCH_PUBLISHED session=%llu epoch=%llu iteration=%llu "
      "required_mask=%u published_mask=%u",
      static_cast<unsigned long long>(session_id),
      static_cast<unsigned long long>(step_id),
      static_cast<unsigned long long>(snapshot.iteration),
      static_cast<unsigned int>(batch.required_mask),
      static_cast<unsigned int>(batch.published_mask));
  }

  void on_blocking_actuator_command(const ActuatorCommand::SharedPtr message)
  {
    std::string rejection;
    CommandCached cached;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      if (active_session_id_ == 0) {
        rejection = "ActuatorCommand arrived before PRIME";
      } else if (message->source_step_id != last_committed_step_) {
        rejection = "ActuatorCommand source_step_id mismatch";
      } else if (message->target_step_id != last_committed_step_ + 1) {
        rejection = "ActuatorCommand target_step_id is not the exact next step";
      } else if (message->command_id != message->target_step_id) {
        rejection = "ActuatorCommand command_id must equal target_step_id";
      } else if (message->valid_from_iteration != latest_snapshot_.iteration + 1) {
        rejection = "ActuatorCommand valid_from_iteration mismatch";
      } else if (command_pending_) {
        rejection = "已有 epoch 正在等待 CommitRelease";
      } else {
        pending_command_.session_id = active_session_id_;
        pending_command_.source_step_id = message->source_step_id;
        pending_command_.step_id = message->target_step_id;
        pending_command_.command_id = message->command_id;
        pending_command_.valid_from_iteration = message->valid_from_iteration;
        pending_command_.armed = message->armed;
        pending_command_.valid = message->valid;
        pending_command_.actuator_values = message->actuator_values;
        ++accepted_command_count_;
        command_pending_ = true;
        command_applied_ = false;
        result_payload_ready_ = false;
        completion_announced_ = false;
        transaction_error_.clear();
      }

      cached.session_id = active_session_id_;
      cached.step_id = message->target_step_id;
      cached.command_id = message->command_id;
      cached.cached_iteration = latest_snapshot_.iteration;
      cached.cached_sim_time_ns = latest_snapshot_.sim_time_ns;
      cached.accepted = rejection.empty();
      cached.status = cached.accepted ? "COMMAND_CACHED" : rejection;
    }
    command_cached_pub_->publish(cached);
    if (!rejection.empty()) {
      RCLCPP_ERROR(node_->get_logger(), "%s", rejection.c_str());
    }
  }

  void on_commit_release(const CommitRelease::SharedPtr message)
  {
    SensorSnapshot released_snapshot;
    std::uint64_t released_session{0};
    std::uint64_t released_step{0};
    std::string rejection;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      if (!message->accepted) {
        rejection = "Coordinator 拒绝 CommitRelease";
      } else if (!command_pending_ || !result_payload_ready_) {
        rejection = "CommitRelease 到达时没有缓存结果";
      } else if (message->session_id != pending_command_.session_id ||
        message->step_id != pending_command_.step_id ||
        message->command_id != pending_command_.command_id ||
        message->iteration != cached_snapshot_.iteration ||
        message->sim_time_ns != cached_snapshot_.sim_time_ns)
      {
        rejection = "CommitRelease identity 与缓存结果不一致";
      } else {
        released_snapshot = cached_snapshot_;
        released_session = pending_command_.session_id;
        released_step = pending_command_.step_id;
        last_committed_step_ = pending_command_.step_id;
        command_pending_ = false;
        command_applied_ = false;
        result_payload_ready_ = false;
        completion_announced_ = false;
      }
    }
    if (!rejection.empty()) {
      RCLCPP_ERROR(node_->get_logger(), "%s", rejection.c_str());
      return;
    }
    // Publish data before metadata. FlightCore accepts either DDS arrival order
    // by caching sensor receipts and batch metadata independently, but this
    // ordering still makes the producer-side contract explicit.
    const auto published_mask =
      publish_sensor_snapshot(released_snapshot, false);
    publish_sensor_batch(
      released_snapshot,
      released_session,
      released_step,
      message->command_id,
      published_mask);
  }

  void on_blocking_status(
    const std::shared_ptr<Trigger::Request>,
    std::shared_ptr<Trigger::Response> response)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    std::ostringstream status;
    status << "session_id=" << active_session_id_
           << ";accepted=" << accepted_command_count_
           << ";committed=" << last_committed_step_
           << ";iteration=" << latest_snapshot_.iteration
           << ";sim_time_ns=" << latest_snapshot_.sim_time_ns;
    response->success = true;
    response->message = status.str();
  }

  void on_prime_session(
    const std::shared_ptr<PrimeSession::Request> request,
    std::shared_ptr<PrimeSession::Response> response)
  {
    RCLCPP_INFO(
      node_->get_logger(), "PrimeSession begin: session=%llu",
      static_cast<unsigned long long>(request->session_id));

    SensorSnapshot snapshot;
    {
      std::unique_lock<std::mutex> lock(state_mutex_);
      const bool snapshot_ready = state_cv_.wait_for(
        lock,
        service_timeout_,
        [this]() {return initial_snapshot_ready_;});
      if (!snapshot_ready) {
        response->success = false;
        response->status = "等待 Gazebo 初始快照超时";
        return;
      }
      if (request->session_id == 0) {
        response->success = false;
        response->status = "PRIME session_id 必须非零";
        return;
      }
      if (active_session_id_ != 0) {
        response->success = false;
        response->status = "当前 Gazebo 进程已经完成 PRIME";
        return;
      }
      if (latest_snapshot_.iteration != 0 ||
        latest_snapshot_.sim_time_ns != 0 ||
        command_pending_ ||
        accepted_command_count_ != 0 ||
        last_committed_step_ != 0)
      {
        response->success = false;
        response->status = "Gazebo 不是可 PRIME 的全新零时刻世界";
        return;
      }
      active_session_id_ = request->session_id;
      snapshot = latest_snapshot_;
    }
    RCLCPP_INFO(
      node_->get_logger(), "PRIME accepted: iteration=%llu time_ns=%lld",
      static_cast<unsigned long long>(snapshot.iteration),
      static_cast<long long>(snapshot.sim_time_ns));

    response->success = true;
    response->status = "PRIME 完成";
    response->session_id = request->session_id;
    response->iteration = snapshot.iteration;
    response->sim_time_ns = snapshot.sim_time_ns;
    response->imu_accel_mps2 = snapshot.imu_accel_mps2;
    response->imu_gyro_radps = snapshot.imu_gyro_radps;
    response->latitude_deg = snapshot.latitude_deg;
    response->longitude_deg = snapshot.longitude_deg;
    response->altitude_m = snapshot.altitude_m;
    response->velocity_ned_mps = snapshot.velocity_ned_mps;
    response->position_ned_m = snapshot.position_ned_m;
    response->attitude_quat_wxyz = snapshot.attitude_quat_wxyz;

    const auto published_mask = publish_sensor_snapshot(snapshot, true);
    publish_sensor_batch(
      snapshot, request->session_id, 0U, 0U, published_mask);
  }

  void apply_wrench(
    const PendingCommand & command,
    gz::sim::EntityComponentManager & ecm)
  {
    std::array<double, 4> normalized{};
    for (std::size_t i = 0; i < normalized.size(); ++i) {
      normalized[i] = command.armed && command.valid ?
        std::clamp(static_cast<double>(command.actuator_values[i]), 0.0, 1.0) : 0.0;
    }

    std::array<double, 4> thrust{};
    std::array<double, 4> reaction{};
    for (std::size_t i = 0; i < thrust.size(); ++i) {
      thrust[i] = max_thrust_n_ * normalized[i];
      reaction[i] = max_reaction_torque_nm_ * normalized[i];
    }

    const double k = std::sqrt(0.5);
    const double total_thrust = thrust[0] + thrust[1] + thrust[2] + thrust[3];
    const double torque_x = motor_arm_length_m_ * k *
      (-thrust[0] + thrust[1] + thrust[2] - thrust[3]);
    const double torque_y = motor_arm_length_m_ * k *
      (-thrust[0] + thrust[1] - thrust[2] + thrust[3]);
    const double torque_z =
      -reaction[0] - reaction[1] + reaction[2] + reaction[3];

    const auto pose = link_.WorldPose(ecm);
    if (!pose.has_value()) {
      transaction_error_ = "无法读取 base_link 世界姿态";
      return;
    }

    // FlightCore 使用 FRD；Gazebo link 使用 FLU，所以 y/z 力矩需要翻转。
    const gz::math::Vector3d force_flu(0.0, 0.0, total_thrust);
    const gz::math::Vector3d torque_flu(torque_x, -torque_y, -torque_z);
    auto force_world = pose->Rot().RotateVector(force_flu);
    // 轻量 plant 用各向同性一阶阻力近似机体/旋翼气动阻尼；静态悬停推力不受影响。
    const auto velocity_world = link_.WorldLinearVelocity(ecm);
    if (velocity_world.has_value()) {
      force_world -= linear_drag_n_per_mps_ * (*velocity_world);
    }
    const auto torque_world = pose->Rot().RotateVector(torque_flu);
    link_.AddWorldWrench(ecm, force_world, torque_world);
  }

  SensorSnapshot capture_snapshot(
    const gz::sim::UpdateInfo & info,
    const gz::sim::EntityComponentManager & ecm)
  {
    SensorSnapshot snapshot;
    snapshot.iteration = info.iterations;
    snapshot.sim_time_ns = to_nanoseconds(info.simTime);

    const auto pose = link_.WorldPose(ecm);
    const auto velocity_world = link_.WorldLinearVelocity(ecm);
    const auto angular_velocity_world = link_.WorldAngularVelocity(ecm);
    if (!pose.has_value() || !velocity_world.has_value() ||
      !angular_velocity_world.has_value())
    {
      return snapshot;
    }

    if (!origin_initialized_) {
      origin_world_position_ = pose->Pos();
      origin_initialized_ = true;
    }

    // ENU -> NED：N=Y, E=X, D=-Z。
    const auto relative = pose->Pos() - origin_world_position_;
    const gz::math::Vector3d position_ned(relative.Y(), relative.X(), -relative.Z());
    const gz::math::Vector3d velocity_ned(
      velocity_world->Y(), velocity_world->X(), -velocity_world->Z());

    snapshot.position_ned_m = {
      static_cast<float>(position_ned.X()),
      static_cast<float>(position_ned.Y()),
      static_cast<float>(position_ned.Z())};
    snapshot.velocity_ned_mps = {
      static_cast<float>(velocity_ned.X()),
      static_cast<float>(velocity_ned.Y()),
      static_cast<float>(velocity_ned.Z())};

    // 构造 NED/FRD 姿态：C_NB = C_NE * C_EF * C_FB。
    const gz::math::Quaterniond q_ne(kPi, 0.0, kPi / 2.0);
    const gz::math::Quaterniond q_fb(kPi, 0.0, 0.0);
    const gz::math::Quaterniond q_nb = q_ne * pose->Rot() * q_fb;
    snapshot.attitude_quat_wxyz = {
      static_cast<float>(q_nb.W()),
      static_cast<float>(q_nb.X()),
      static_cast<float>(q_nb.Y()),
      static_cast<float>(q_nb.Z())};

    const auto angular_velocity_flu =
      pose->Rot().RotateVectorReverse(*angular_velocity_world);
    snapshot.imu_gyro_radps = {
      static_cast<float>(angular_velocity_flu.X()),
      static_cast<float>(-angular_velocity_flu.Y()),
      static_cast<float>(-angular_velocity_flu.Z())};

    gz::math::Vector3d acceleration_world(0.0, 0.0, 0.0);
    const double dt = std::chrono::duration<double>(info.dt).count();
    if (previous_velocity_valid_ && dt > 0.0) {
      acceleration_world = (*velocity_world - previous_velocity_world_) / dt;
    }
    previous_velocity_world_ = *velocity_world;
    previous_velocity_valid_ = true;

    // 加速度计输出比力：f = a - g，再转换到 FRD 体轴。
    const gz::math::Vector3d specific_force_world =
      acceleration_world - gz::math::Vector3d(0.0, 0.0, -kGravity);
    const auto specific_force_flu =
      pose->Rot().RotateVectorReverse(specific_force_world);
    snapshot.imu_accel_mps2 = {
      static_cast<float>(specific_force_flu.X()),
      static_cast<float>(-specific_force_flu.Y()),
      static_cast<float>(-specific_force_flu.Z())};

    const double latitude_rad = kReferenceLatitudeDeg * kPi / 180.0;
    snapshot.latitude_deg = kReferenceLatitudeDeg +
      position_ned.X() / kEarthRadiusM * 180.0 / kPi;
    snapshot.longitude_deg = kReferenceLongitudeDeg +
      position_ned.Y() /
      (kEarthRadiusM * std::max(std::cos(latitude_rad), 1.0e-6)) *
      180.0 / kPi;
    snapshot.altitude_m = kReferenceAltitudeM - position_ned.Z();
    return snapshot;
  }

  gz::sim::Model model_{gz::sim::kNullEntity};
  gz::sim::Entity base_link_entity_{gz::sim::kNullEntity};
  gz::sim::Link link_{gz::sim::kNullEntity};
  std::string world_name_;

  double max_thrust_n_{4.179446268};
  double max_reaction_torque_nm_{0.055562};
  double motor_arm_length_m_{0.2275};
  double linear_drag_n_per_mps_{1.2};
  std::uint64_t gps_rate_divider_{200};
  std::chrono::milliseconds service_timeout_{5000};

  std::mutex state_mutex_;
  std::condition_variable state_cv_;
  PendingCommand pending_command_;
  bool command_pending_{false};
  bool command_applied_{false};
  bool result_payload_ready_{false};
  bool completion_announced_{false};
  bool initial_snapshot_ready_{false};
  std::string transaction_error_;
  std::uint64_t applied_iteration_{0};
  std::uint64_t active_session_id_{0};
  std::uint64_t last_committed_step_{0};
  std::uint64_t accepted_command_count_{0};
  SensorSnapshot latest_snapshot_;
  SensorSnapshot cached_snapshot_;

  bool origin_initialized_{false};
  gz::math::Vector3d origin_world_position_{0.0, 0.0, 0.0};
  bool previous_velocity_valid_{false};
  gz::math::Vector3d previous_velocity_world_{0.0, 0.0, 0.0};

  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr callback_group_;
  rclcpp::Publisher<CommandCached>::SharedPtr command_cached_pub_;
  rclcpp::Publisher<PlantStepDone>::SharedPtr plant_step_done_pub_;
  rclcpp::Publisher<ResultReady>::SharedPtr result_ready_pub_;
  rclcpp::Publisher<SensorBatchPublished>::SharedPtr
    sensor_batch_published_pub_;
  rclcpp::Publisher<Imu>::SharedPtr imu_pub_;
  rclcpp::Publisher<Gps>::SharedPtr gps_pub_;
  rclcpp::Subscription<ActuatorCommand>::SharedPtr actuator_command_sub_;
  rclcpp::Subscription<CommitRelease>::SharedPtr commit_release_sub_;
  rclcpp::Service<PrimeSession>::SharedPtr prime_service_;
  rclcpp::Service<Trigger>::SharedPtr blocking_status_service_;
  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> executor_;
  std::thread executor_thread_;
};

}  // namespace flightcore_gazebo

GZ_ADD_PLUGIN(
  flightcore_gazebo::FlightCoreGazeboSystem,
  gz::sim::System,
  gz::sim::ISystemConfigure,
  gz::sim::ISystemPreUpdate,
  gz::sim::ISystemPostUpdate)

GZ_ADD_PLUGIN_ALIAS(
  flightcore_gazebo::FlightCoreGazeboSystem,
  "flightcore_gazebo::FlightCoreGazeboSystem")
