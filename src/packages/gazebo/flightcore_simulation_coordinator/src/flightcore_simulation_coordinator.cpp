// FlightCore–Gazebo strict lockstep coordinator.
//
// Ownership:
//   * the only publisher allowed to grant generated model execution via /clock;
//   * the only caller allowed to advance Gazebo through native WorldControl;
//   * the epoch identity validator for command, plant, sensor and model ACKs.
//
// The coordinator intentionally carries no flight-control payload logic. It
// closes the causal chain around two independent participants:
//   generated FlightCore: /clock -> ActuatorCommand -> step_notify
//   Gazebo: command cache -> physical step -> sensor batch
//
// A sensor publish is not a receive acknowledgement. The next /clock therefore
// waits for FlightCore ObservationReady, not Gazebo producer completion. This
// distinction removes the former scheduling-dependent deadlock in which sensor
// subscriptions were only consumed from inside model->step().
//
// Threading: all ROS callbacks and the watchdog may touch protocol state, so
// state_mutex_ protects every transition. The asynchronous Gazebo transport
// WorldControl response also re-enters through the same mutex.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "gz/msgs/boolean.pb.h"
#include "gz/msgs/server_control.pb.h"
#include "gz/msgs/world_control.pb.h"
#include "gz/transport/Node.hh"
#include "rclcpp/rclcpp.hpp"
#include "rosgraph_msgs/msg/clock.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "flightcore_gazebo_msgs/msg/actuator_command.hpp"
#include "flightcore_gazebo_msgs/msg/command_cached.hpp"
#include "flightcore_gazebo_msgs/msg/commit_release.hpp"
#include "flightcore_gazebo_msgs/msg/plant_step_done.hpp"
#include "flightcore_gazebo_msgs/msg/result_ready.hpp"
#include "flightcore_gazebo_msgs/msg/observation_ready.hpp"

namespace flightcore_gazebo
{

using ActuatorCommand = flightcore_gazebo_msgs::msg::ActuatorCommand;
using Clock = rosgraph_msgs::msg::Clock;
using CommandCached = flightcore_gazebo_msgs::msg::CommandCached;
using CommitRelease = flightcore_gazebo_msgs::msg::CommitRelease;
using PlantStepDone = flightcore_gazebo_msgs::msg::PlantStepDone;
using ResultReady = flightcore_gazebo_msgs::msg::ResultReady;
using ObservationReady = flightcore_gazebo_msgs::msg::ObservationReady;
using StepNotify = std_msgs::msg::String;
using Trigger = std_srvs::srv::Trigger;

class FlightCoreSimulationCoordinator final : public rclcpp::Node
{
public:
  FlightCoreSimulationCoordinator()
  : Node("flightcore_simulation_coordinator")
  {
    const auto world_name = declare_parameter<std::string>(
      "world_name", "flightcore_world");
    world_control_service_ = "/world/" + world_name + "/control";

    const auto configured_max_epochs = declare_parameter<std::int64_t>(
      "max_epochs", 20);
    if (configured_max_epochs <= 0) {
      throw std::invalid_argument("max_epochs 必须为正整数");
    }
    max_epochs_ = static_cast<std::uint64_t>(configured_max_epochs);

    const auto configured_progress_timeout_ms =
      declare_parameter<std::int64_t>("progress_timeout_ms", 2000);
    if (configured_progress_timeout_ms <= 0) {
      throw std::invalid_argument("progress_timeout_ms 必须为正整数");
    }
    progress_timeout_ =
      std::chrono::milliseconds(configured_progress_timeout_ms);
    last_progress_wall_ = std::chrono::steady_clock::now();

    const auto barrier_qos = rclcpp::QoS(rclcpp::KeepLast(8)).reliable();
    commit_release_pub_ = create_publisher<CommitRelease>(
      "/flightcore/gazebo/commit_release", barrier_qos);

    // /clock 是生成节点执行 base rate 的唯一许可。MathWorks R2025b 生成节点
    // 对 /clock 明确请求 Reliable；rclcpp::ClockQoS() 默认 Best Effort，会被
    // DDS 判定为不兼容。因此这里按真实订阅端使用 Reliable + Volatile，
    // 同时保持深度 1，确保每个时钟值只发布一次且必须严格递增。
    const auto generated_node_clock_qos =
      rclcpp::QoS(rclcpp::KeepLast(1)).reliable().durability_volatile();
    clock_pub_ = create_publisher<Clock>(
      "/clock", generated_node_clock_qos);

    actuator_command_sub_ = create_subscription<ActuatorCommand>(
      "/flightcore/gazebo/actuator_command", barrier_qos,
      std::bind(
        &FlightCoreSimulationCoordinator::on_control_ready,
        this, std::placeholders::_1));
    command_cached_sub_ = create_subscription<CommandCached>(
      "/flightcore/gazebo/command_cached", barrier_qos,
      std::bind(
        &FlightCoreSimulationCoordinator::on_command_cached,
        this, std::placeholders::_1));
    plant_step_done_sub_ = create_subscription<PlantStepDone>(
      "/flightcore/gazebo/plant_step_done", barrier_qos,
      std::bind(
        &FlightCoreSimulationCoordinator::on_plant_step_done,
        this, std::placeholders::_1));
    result_ready_sub_ = create_subscription<ResultReady>(
      "/flightcore/gazebo/result_ready", barrier_qos,
      std::bind(
        &FlightCoreSimulationCoordinator::on_result_ready,
        this, std::placeholders::_1));
    observation_ready_sub_ = create_subscription<ObservationReady>(
      "/flightcore/gazebo/observation_ready", barrier_qos,
      std::bind(
        &FlightCoreSimulationCoordinator::on_observation_ready,
        this, std::placeholders::_1));
    // MathWorks 生成节点在完整 mModel->step() 返回后发布
    // "+FlightCore_Gazebo_loop"。ActuatorCommand 在 step 内部更早发布，不能
    // 单独证明该帧 /clock 已消费完；等待此通知可消除下一帧时钟覆盖竞态。
    step_notify_sub_ = create_subscription<StepNotify>(
      "/flightcore/gazebo/step_notify", barrier_qos,
      std::bind(
        &FlightCoreSimulationCoordinator::on_step_notify,
        this, std::placeholders::_1));

    // 人工调用该服务才发布首帧 /clock=0。这样启动进程、PRIME 世界和
    // 检查图连通性可以分开完成，避免生成节点在准备阶段提前计算。
    start_service_ = create_service<Trigger>(
      "/flightcore/gazebo/start_coordinator",
      std::bind(
        &FlightCoreSimulationCoordinator::on_start,
        this, std::placeholders::_1, std::placeholders::_2));

    watchdog_timer_ = create_wall_timer(
      std::chrono::milliseconds(100),
      std::bind(&FlightCoreSimulationCoordinator::on_watchdog, this));

    RCLCPP_INFO(
      get_logger(),
      "COORDINATOR_READY world_control=%s max_epochs=%llu progress_timeout_ms=%lld",
      world_control_service_.c_str(),
      static_cast<unsigned long long>(max_epochs_),
      static_cast<long long>(progress_timeout_.count()));
  }

  int exit_code() const
  {
    return fatal_exit_requested_.load() ? EXIT_FAILURE : EXIT_SUCCESS;
  }

private:
  void mark_progress_locked()
  {
    last_progress_wall_ = std::chrono::steady_clock::now();
  }

  void on_watchdog()
  {
    // Wall time is used deliberately: simulated time cannot advance while the
    // protocol is stalled. A ROS/simulation-time timer would never fire in the
    // exact failure mode this watchdog is intended to diagnose.
    std::string reason;
    {
      std::lock_guard<std::mutex> lock(state_mutex_);
      if (!started_ || completed_ || aborted_) {
        return;
      }

      const auto now = std::chrono::steady_clock::now();
      const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_progress_wall_);
      if (elapsed < progress_timeout_) {
        return;
      }

      std::ostringstream status;
      status
        << "PROGRESS_TIMEOUT elapsed_ms=" << elapsed.count()
        << " committed_epoch=" << committed_step_
        << " pending_epoch=" << pending_step_
        << " last_clock_ns=" << last_clock_ns_
        << " control_ready=" << control_ready_.has_value()
        << " command_cached=" << command_cached_.has_value()
        << " world_control_requested=" << world_control_requested_
        << " world_control_accepted=" << world_control_accepted_
        << " plant_step_done=" << plant_step_done_.has_value()
        << " result_ready=" << result_ready_.has_value()
        << " awaiting_observation_ready=" << awaiting_observation_ready_
        << " observation_ready=" << observation_ready_.has_value()
        << " required_mask="
        << (observation_ready_.has_value() ?
          static_cast<unsigned int>(observation_ready_->required_mask) : 0U)
        << " received_mask="
        << (observation_ready_.has_value() ?
          static_cast<unsigned int>(observation_ready_->received_mask) : 0U)
        << " model_step_done=" << model_step_done_;
      reason = status.str();
      aborted_ = true;
      fatal_exit_requested_.store(true);
    }

    RCLCPP_FATAL(
      get_logger(),
      "COORDINATOR_STALLED: %s; stopping FlightCore-Gazebo simulation",
      reason.c_str());

    gz::msgs::ServerControl stop_request;
    gz::msgs::Boolean stop_response;
    bool stop_result{false};
    stop_request.set_stop(true);
    const bool stop_executed = gazebo_transport_node_.Request(
      "/server_control", stop_request, 1000U, stop_response, stop_result);
    if (!stop_executed || !stop_result || !stop_response.data()) {
      RCLCPP_ERROR(
        get_logger(),
        "GAZEBO_STOP_FAILED executed=%d result=%d response=%d",
        stop_executed, stop_result, stop_response.data());
    }

    rclcpp::shutdown();
  }

  bool accept_session_locked(const std::uint64_t session_id)
  {
    if (session_id == 0) {
      abort_locked("session_id 必须非零");
      return false;
    }
    if (active_session_id_ == 0) {
      active_session_id_ = session_id;
      RCLCPP_INFO(
        get_logger(), "Coordinator session latched: %llu",
        static_cast<unsigned long long>(active_session_id_));
    }
    if (session_id != active_session_id_) {
      RCLCPP_WARN(
        get_logger(), "忽略非当前 session 消息：%llu",
        static_cast<unsigned long long>(session_id));
      return false;
    }
    return !aborted_;
  }

  void abort_locked(const std::string & reason)
  {
    if (!aborted_) {
      RCLCPP_ERROR(get_logger(), "COORDINATOR_ABORTED: %s", reason.c_str());
    }
    aborted_ = true;
  }

  bool publish_clock_locked(const std::int64_t sim_time_ns)
  {
    // 首帧允许 0；此后必须严格大于上一帧。拒绝重复时间可直接阻止
    // Stage 1 探针发现的“重复 /clock=0 导致重复 base-rate”风险。
    if (sim_time_ns < 0 ||
      (last_clock_ns_ >= 0 && sim_time_ns <= last_clock_ns_))
    {
      abort_locked("尝试发布非严格递增的 /clock");
      return false;
    }

    Clock clock;
    clock.clock.sec = static_cast<std::int32_t>(
      sim_time_ns / 1000000000LL);
    clock.clock.nanosec = static_cast<std::uint32_t>(
      sim_time_ns % 1000000000LL);
    clock_pub_->publish(clock);
    last_clock_ns_ = sim_time_ns;
    mark_progress_locked();
    RCLCPP_INFO(
      get_logger(), "CLOCK_GRANTED sim_time_ns=%lld",
      static_cast<long long>(sim_time_ns));
    return true;
  }

  void try_start_clock_locked()
  {
    // PRIME may arrive before or after the operator opens the start gate.
    // Publishing /clock=0 is deferred until both conditions are true, and the
    // prime ObservationReady object is consumed immediately to make duplicate
    // zero clocks impossible.
    if (aborted_ || !started_ || last_clock_ns_ >= 0 ||
      !observation_ready_.has_value())
    {
      return;
    }
    const auto & ready = observation_ready_.value();
    if (!ready.accepted ||
      ready.step_id != 0U ||
      ready.command_id != 0U ||
      ready.iteration != 0U ||
      ready.sim_time_ns != 0 ||
      ready.required_mask != 0x03U ||
      (ready.received_mask & ready.required_mask) != ready.required_mask)
    {
      abort_locked("PRIME ObservationReady identity/mask 校验失败");
      return;
    }
    if (publish_clock_locked(0)) {
      observation_ready_.reset();
    }
  }

  void on_start(
    const std::shared_ptr<Trigger::Request>,
    std::shared_ptr<Trigger::Response> response)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (aborted_) {
      response->success = false;
      response->message = "Coordinator 已中止";
      return;
    }
    if (started_) {
      response->success = false;
      response->message = "Coordinator 已启动；禁止重复发布首帧 /clock";
      return;
    }
    if (clock_pub_->get_subscription_count() == 0U) {
      response->success = false;
      response->message = "没有 /clock 订阅者；请先启动生成节点";
      return;
    }

    started_ = true;
    mark_progress_locked();
    try_start_clock_locked();
    response->success = true;
    response->message =
      last_clock_ns_ == 0 ?
      "Coordinator 已在 PRIME ObservationReady 后发布 /clock=0" :
      "Coordinator 已启动，等待 PRIME ObservationReady";
  }

  void on_control_ready(const ActuatorCommand::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (aborted_) {
      return;
    }
    if (!started_) {
      // 生成节点在人工启动门之前产生控制命令，说明 ROS 时间步进没有生效。
      abort_locked("启动门打开前收到 ActuatorCommand，生成节点疑似自由运行");
      return;
    }
    if (completed_) {
      abort_locked("达到 max_epochs 后仍收到 ActuatorCommand");
      return;
    }
    if (awaiting_observation_ready_) {
      abort_locked("ObservationReady 屏障完成前收到下一条 ActuatorCommand");
      return;
    }

    // 完整身份的 ActuatorCommand 到达即证明生成节点已完成本 epoch
    // 控制计算。msg->valid 是执行器 payload 语义，不能替代控制阶段 ACK。
    if (msg->source_step_id != committed_step_ ||
      msg->target_step_id != committed_step_ + 1 ||
      msg->command_id != msg->target_step_id)
    {
      abort_locked("ControlReady identity 校验失败");
      return;
    }
    if (control_ready_.has_value()) {
      const auto & previous = control_ready_.value();
      if (previous.target_step_id == msg->target_step_id &&
        previous.command_id == msg->command_id)
      {
        return;
      }
      abort_locked("收到冲突的 ControlReady");
      return;
    }
    control_ready_ = *msg;
    mark_progress_locked();
    RCLCPP_INFO(
      get_logger(),
      "CONTROL_READY_ACK epoch=%llu accepted=true",
      static_cast<unsigned long long>(msg->target_step_id));
    try_request_world_control_locked();
  }

  void on_command_cached(const CommandCached::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!accept_session_locked(msg->session_id)) {
      return;
    }
    if (!started_ || awaiting_observation_ready_ || !msg->accepted ||
      msg->step_id != committed_step_ + 1 ||
      msg->command_id != msg->step_id)
    {
      abort_locked("CommandCached ACK/identity 校验失败");
      return;
    }
    if (command_cached_.has_value()) {
      const auto & previous = command_cached_.value();
      if (previous.session_id == msg->session_id &&
        previous.step_id == msg->step_id &&
        previous.command_id == msg->command_id &&
        previous.accepted == msg->accepted)
      {
        return;
      }
      abort_locked("收到冲突的 CommandCached");
      return;
    }
    command_cached_ = *msg;
    mark_progress_locked();
    RCLCPP_INFO(
      get_logger(),
      "COMMAND_CACHED_ACK session=%llu epoch=%llu accepted=true",
      static_cast<unsigned long long>(msg->session_id),
      static_cast<unsigned long long>(msg->step_id));
    try_request_world_control_locked();
  }

  void try_request_world_control_locked()
  {
    if (aborted_ || !started_ || completed_ || awaiting_observation_ready_ ||
      world_control_requested_ ||
      !control_ready_.has_value() || !command_cached_.has_value())
    {
      return;
    }
    const auto & control = control_ready_.value();
    const auto & cached = command_cached_.value();
    if (control.target_step_id != cached.step_id ||
      control.command_id != cached.command_id)
    {
      abort_locked("pre-step ACK identity 不一致");
      return;
    }

    pending_step_ = control.target_step_id;
    pending_command_id_ = control.command_id;
    pending_iteration_ = cached.cached_iteration + 1;
    world_control_requested_ = true;

    // Coordinator 是唯一 WorldControl 所有者；每个 epoch 精确请求一步。
    gz::msgs::WorldControl request;
    request.set_pause(true);
    request.set_step(false);
    request.set_multi_step(1);
    const bool queued = gazebo_transport_node_.Request(
      world_control_service_, request,
      &FlightCoreSimulationCoordinator::on_world_control_response, this);
    if (!queued) {
      abort_locked("Coordinator 无法投递 WorldControl");
      return;
    }
    RCLCPP_INFO(
      get_logger(),
      "WORLD_CONTROL_REQUESTED session=%llu epoch=%llu multi_step=1",
      static_cast<unsigned long long>(active_session_id_),
      static_cast<unsigned long long>(pending_step_));
  }

  void on_world_control_response(
    const gz::msgs::Boolean & response,
    const bool service_result)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (aborted_) {
      return;
    }
    if (!service_result || !response.data()) {
      abort_locked("WorldControl Boolean=false");
      return;
    }
    world_control_accepted_ = true;
    mark_progress_locked();
    RCLCPP_INFO(
      get_logger(),
      "WORLD_CONTROL_ACCEPTED session=%llu epoch=%llu accepted=true",
      static_cast<unsigned long long>(active_session_id_),
      static_cast<unsigned long long>(pending_step_));
    try_commit_release_locked();
  }

  void on_plant_step_done(const PlantStepDone::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!validate_completion_locked(
        msg->session_id, msg->step_id, msg->command_id,
        msg->iteration, msg->accepted, "PlantStepDone"))
    {
      return;
    }
    plant_step_done_ = *msg;
    mark_progress_locked();
    RCLCPP_INFO(
      get_logger(),
      "PLANT_STEP_DONE_ACK session=%llu epoch=%llu accepted=true",
      static_cast<unsigned long long>(msg->session_id),
      static_cast<unsigned long long>(msg->step_id));
    try_commit_release_locked();
  }

  void on_result_ready(const ResultReady::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!validate_completion_locked(
        msg->session_id, msg->step_id, msg->command_id,
        msg->iteration, msg->accepted, "ResultReady"))
    {
      return;
    }
    result_ready_ = *msg;
    mark_progress_locked();
    RCLCPP_INFO(
      get_logger(),
      "RESULT_READY_ACK session=%llu epoch=%llu accepted=true",
      static_cast<unsigned long long>(msg->session_id),
      static_cast<unsigned long long>(msg->step_id));
    try_commit_release_locked();
  }

  bool validate_completion_locked(
    const std::uint64_t session_id,
    const std::uint64_t step_id,
    const std::uint64_t command_id,
    const std::uint64_t iteration,
    const bool accepted,
    const char * phase)
  {
    if (!accept_session_locked(session_id)) {
      return false;
    }
    if (!world_control_requested_ || awaiting_observation_ready_ ||
      !accepted ||
      step_id != pending_step_ ||
      command_id != pending_command_id_ ||
      iteration != pending_iteration_)
    {
      abort_locked(std::string(phase) + " ACK/identity 校验失败");
      return false;
    }
    return true;
  }

  void try_commit_release_locked()
  {
    if (aborted_ || awaiting_observation_ready_ || !world_control_accepted_ ||
      !plant_step_done_.has_value() || !result_ready_.has_value())
    {
      return;
    }
    const auto & done = plant_step_done_.value();
    const auto & ready = result_ready_.value();
    if (done.session_id != ready.session_id ||
      done.step_id != ready.step_id ||
      done.command_id != ready.command_id ||
      done.iteration != ready.iteration ||
      done.sim_time_ns != ready.sim_time_ns)
    {
      abort_locked("completion ACK identity 不一致");
      return;
    }

    CommitRelease release;
    release.session_id = active_session_id_;
    release.step_id = pending_step_;
    release.command_id = pending_command_id_;
    release.iteration = done.iteration;
    release.sim_time_ns = done.sim_time_ns;
    release.accepted = true;
    release.status = "COMMIT_RELEASE";
    // 先进入等待态再发布，避免 FlightCore 极快返回 ObservationReady 时
    // Coordinator 仍处于上一状态而误判。
    awaiting_observation_ready_ = true;
    commit_release_pub_->publish(release);

    // 此处不能提前更新 committed_step_ 或发布 /clock。插件必须先发布本
    // epoch 的传感器，FlightCore 实际收到 required_mask 后才会确认。
    RCLCPP_INFO(
      get_logger(),
      "COMMIT_RELEASE session=%llu epoch=%llu iteration=%llu",
      static_cast<unsigned long long>(release.session_id),
      static_cast<unsigned long long>(release.step_id),
      static_cast<unsigned long long>(release.iteration));
  }

  void on_observation_ready(const ObservationReady::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!accept_session_locked(msg->session_id)) {
      return;
    }
    if (!msg->accepted ||
      msg->required_mask == 0U ||
      (msg->required_mask & ~0x03U) != 0U ||
      (msg->received_mask & msg->required_mask) != msg->required_mask)
    {
      abort_locked("ObservationReady mask/status 校验失败");
      return;
    }

    // PRIME 观测先于启动门到达是正常情况。它必须包含 IMU+GPS，
    // 并在用户打开启动门后才授权唯一的 /clock=0。
    if (msg->step_id == 0U) {
      if (msg->command_id != 0U ||
        msg->iteration != 0U ||
        msg->sim_time_ns != 0 ||
        msg->required_mask != 0x03U)
      {
        abort_locked("PRIME ObservationReady identity 校验失败");
        return;
      }
      if (observation_ready_.has_value()) {
        return;
      }
      observation_ready_ = *msg;
      mark_progress_locked();
      RCLCPP_INFO(
        get_logger(),
        "PRIME_OBSERVATION_READY session=%llu required_mask=%u "
        "received_mask=%u",
        static_cast<unsigned long long>(msg->session_id),
        static_cast<unsigned int>(msg->required_mask),
        static_cast<unsigned int>(msg->received_mask));
      try_start_clock_locked();
      return;
    }

    if (!awaiting_observation_ready_ ||
      msg->step_id != pending_step_ ||
      msg->command_id != pending_command_id_ ||
      msg->iteration != pending_iteration_ ||
      !plant_step_done_.has_value() ||
      msg->sim_time_ns != plant_step_done_->sim_time_ns)
    {
      abort_locked("ObservationReady ACK/identity 校验失败");
      return;
    }
    if (msg->sim_time_ns <= last_clock_ns_) {
      abort_locked("ObservationReady 时间没有严格推进");
      return;
    }
    if (observation_ready_.has_value()) {
      return;
    }

    RCLCPP_INFO(
      get_logger(),
      "OBSERVATION_READY_ACK session=%llu epoch=%llu iteration=%llu "
      "required_mask=%u received_mask=%u",
      static_cast<unsigned long long>(msg->session_id),
      static_cast<unsigned long long>(msg->step_id),
      static_cast<unsigned long long>(msg->iteration),
      static_cast<unsigned int>(msg->required_mask),
      static_cast<unsigned int>(msg->received_mask));

    observation_ready_ = *msg;
    mark_progress_locked();
    try_finalize_epoch_locked();
  }

  void on_step_notify(const StepNotify::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (aborted_ || !started_ || completed_) {
      return;
    }
    if (msg->data == "-FlightCore_Gazebo_loop") {
      // "-" 表示该时钟槽不足一个 base rate，本配置按 1 ms 严格递增时
      // 不应作为完成确认，也不应推动下一帧。
      RCLCPP_DEBUG(get_logger(), "STEP_NOTIFY_SKIPPED no base-rate step");
      return;
    }
    if (msg->data != "+FlightCore_Gazebo_loop") {
      abort_locked("收到未知的 step_notify 内容");
      return;
    }
    if (model_step_done_) {
      // Reliable DDS 可能重送完全相同的通知；当前 epoch 已记录时保持幂等。
      return;
    }
    model_step_done_ = true;
    mark_progress_locked();
    RCLCPP_INFO(
      get_logger(), "MODEL_STEP_DONE_ACK epoch=%llu",
      static_cast<unsigned long long>(committed_step_ + 1));
    try_finalize_epoch_locked();
  }

  void try_finalize_epoch_locked()
  {
    // Two independent facts are required:
    //   1. ObservationReady: next observation was consumed by FlightCore ROS
    //      callbacks and is available to the next model step.
    //   2. model_step_done_: the previous model step fully returned, so another
    //      clock cannot overwrite the generated base-rate semaphore.
    if (aborted_ || !awaiting_observation_ready_ || !model_step_done_ ||
      !observation_ready_.has_value())
    {
      return;
    }

    const auto released_time_ns = observation_ready_->sim_time_ns;
    committed_step_ = pending_step_;
    // 达到上限时停在已提交状态，不再发下一帧 /clock；否则清空本 epoch
    // 临时 ACK 后，以 Gazebo 完成时刻授权生成节点执行下一次 base rate。
    reset_epoch_locked();
    if (committed_step_ >= max_epochs_) {
      completed_ = true;
      RCLCPP_INFO(
        get_logger(),
        "COORDINATOR_COMPLETE epochs=%llu final_sim_time_ns=%lld",
        static_cast<unsigned long long>(committed_step_),
        static_cast<long long>(released_time_ns));
      return;
    }
    publish_clock_locked(released_time_ns);
  }

  void reset_epoch_locked()
  {
    control_ready_.reset();
    command_cached_.reset();
    plant_step_done_.reset();
    result_ready_.reset();
    observation_ready_.reset();
    pending_step_ = 0;
    pending_command_id_ = 0;
    pending_iteration_ = 0;
    world_control_requested_ = false;
    world_control_accepted_ = false;
    awaiting_observation_ready_ = false;
    model_step_done_ = false;
  }

  std::mutex state_mutex_;
  std::atomic_bool fatal_exit_requested_{false};
  bool aborted_{false};
  bool started_{false};
  bool completed_{false};
  bool awaiting_observation_ready_{false};
  std::int64_t last_clock_ns_{-1};
  std::uint64_t max_epochs_{20};
  std::uint64_t active_session_id_{0};
  std::uint64_t committed_step_{0};
  std::uint64_t pending_step_{0};
  std::uint64_t pending_command_id_{0};
  std::uint64_t pending_iteration_{0};
  bool world_control_requested_{false};
  bool world_control_accepted_{false};
  bool model_step_done_{false};
  std::optional<ActuatorCommand> control_ready_;
  std::optional<CommandCached> command_cached_;
  std::optional<PlantStepDone> plant_step_done_;
  std::optional<ResultReady> result_ready_;
  std::optional<ObservationReady> observation_ready_;
  std::chrono::milliseconds progress_timeout_{2000};
  std::chrono::steady_clock::time_point last_progress_wall_;

  gz::transport::Node gazebo_transport_node_;
  std::string world_control_service_;
  rclcpp::Publisher<Clock>::SharedPtr clock_pub_;
  rclcpp::Publisher<CommitRelease>::SharedPtr commit_release_pub_;
  rclcpp::Subscription<ActuatorCommand>::SharedPtr actuator_command_sub_;
  rclcpp::Subscription<CommandCached>::SharedPtr command_cached_sub_;
  rclcpp::Subscription<PlantStepDone>::SharedPtr plant_step_done_sub_;
  rclcpp::Subscription<ResultReady>::SharedPtr result_ready_sub_;
  rclcpp::Subscription<ObservationReady>::SharedPtr observation_ready_sub_;
  rclcpp::Subscription<StepNotify>::SharedPtr step_notify_sub_;
  rclcpp::Service<Trigger>::SharedPtr start_service_;
  rclcpp::TimerBase::SharedPtr watchdog_timer_;
};

}  // namespace flightcore_gazebo

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  const auto coordinator =
    std::make_shared<
    flightcore_gazebo::FlightCoreSimulationCoordinator>();
  rclcpp::spin(coordinator);
  if (rclcpp::ok()) {
    rclcpp::shutdown();
  }
  return coordinator->exit_code();
}
