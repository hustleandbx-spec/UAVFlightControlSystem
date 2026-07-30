
//
// File ros2nodeinterface.cpp
//
// Code generated for Simulink model 'FlightCore_Gazebo_loop'.
//
// Model version                  : 1.41
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Wed Jul 29 15:51:34 2026
//
// PROJECT RUNTIME PATCH (2026-07-30)
// ----------------------------------
// Simulink Coder generated the base node wrapper, but the FlightCore–Gazebo
// lockstep route requires sensor callbacks to execute before /clock. The
// project-specific code below:
//   * leaves IMU/GPS subscriptions executable by SLMultiThreadedExecutor;
//   * collects sensor sequence/timestamp receipts independently of model step;
//   * joins receipts with Gazebo SensorBatchPublished metadata;
//   * publishes ObservationReady exactly once per iteration.
//
// Re-running code generation may overwrite this file. Any regenerated package
// must reapply/review this section and repeat a test crossing GPS iteration 200.
//
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4265)
#pragma warning(disable : 4458)
#pragma warning(disable : 4100)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wshadow"
#endif //_MSC_VER
#include "rclcpp/rclcpp.hpp"
#include "FlightCore_Gazebo_loop.h"
#include "ros2nodeinterface.h"
#include "flightcore_gazebo_msgs/msg/observation_ready.hpp"
#include "flightcore_gazebo_msgs/msg/sensor_batch_published.hpp"
#include <thread>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <map>
#include <mutex>
#include <optional>
#include <utility>
#undef ROS_SET_RTM_ERROR_STATUS
#undef ROS_GET_RTM_ERROR_STATUS
#undef ROS_RTM_STEP_TASK
#define ROS_SET_RTM_ERROR_STATUS(status) mModel->getRTM()->setErrorStatus(status)
#define ROS_GET_RTM_ERROR_STATUS()       mModel->getRTM()->getErrorStatus()
#define ROS_RTM_STEP_TASK(id)            mModel->getRTM()->StepTask(id)
#include "slros2_multi_threaded_executor.h"
std::vector<rclcpp::SubscriptionBase*> SLROSSubscribers;
extern rclcpp::Node::SharedPtr SLROSNodePtr;
#ifndef RT_MEMORY_ALLOCATION_ERROR_DEF
#define RT_MEMORY_ALLOCATION_ERROR_DEF
const char *RT_MEMORY_ALLOCATION_ERROR = "memory allocation error";
#endif
//
// Forward declare the ROS 2 Time stepping and notification related
// functions and global flags/variables
//
// Flag to determine that model should continue to run
static volatile bool modelRuns;
static int numOverruns;
static int32_t _clock_msg_sec;
static uint32_t _clock_msg_nsec;
// Publisher object to publish notification on the step
static rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pubStepNotify;
static rclcpp::Publisher<
  flightcore_gazebo_msgs::msg::ObservationReady>::SharedPtr
  pubObservationReady;

namespace {

constexpr std::uint8_t kImuMask = 0x01U;
constexpr std::uint8_t kGpsMask = 0x02U;

struct SensorReceipt {
  // Receipt identity is stored per sensor because DDS is free to deliver IMU,
  // GPS and SensorBatchPublished in any order. A bit is not accepted merely
  // because a message exists; its timestamp must also match the batch.
  std::uint8_t mask{0U};
  std::int64_t imuTimeNs{-1};
  std::int64_t gpsTimeNs{-1};
};

std::mutex observationMutex;
std::map<std::uint64_t, SensorReceipt> sensorReceipts;
std::optional<flightcore_gazebo_msgs::msg::SensorBatchPublished> pendingBatch;
std::optional<std::uint64_t> lastReadyIteration;

void tryPublishObservationReadyLocked()
{
  // This is the only function allowed to publish ObservationReady. It is
  // called after either a sensor callback or batch callback, making the join
  // order-independent while observationMutex keeps the transition atomic.
  if (!pendingBatch.has_value() || !pubObservationReady) {
    return;
  }
  const auto & batch = pendingBatch.value();
  const auto receiptIt = sensorReceipts.find(batch.iteration);
  if (receiptIt == sensorReceipts.end()) {
    return;
  }
  const auto & receipt = receiptIt->second;
  if ((receipt.mask & batch.required_mask) != batch.required_mask) {
    return;
  }
  if (((batch.required_mask & kImuMask) != 0U &&
      receipt.imuTimeNs != batch.sim_time_ns) ||
    ((batch.required_mask & kGpsMask) != 0U &&
      receipt.gpsTimeNs != batch.sim_time_ns))
  {
    RCLCPP_ERROR(
      SLROSNodePtr->get_logger(),
      "OBSERVATION_IDENTITY_MISMATCH iteration=%llu batch_time_ns=%lld "
      "imu_time_ns=%lld gps_time_ns=%lld",
      static_cast<unsigned long long>(batch.iteration),
      static_cast<long long>(batch.sim_time_ns),
      static_cast<long long>(receipt.imuTimeNs),
      static_cast<long long>(receipt.gpsTimeNs));
    return;
  }

  flightcore_gazebo_msgs::msg::ObservationReady ready;
  ready.session_id = batch.session_id;
  ready.step_id = batch.step_id;
  ready.command_id = batch.command_id;
  ready.iteration = batch.iteration;
  ready.sim_time_ns = batch.sim_time_ns;
  ready.required_mask = batch.required_mask;
  ready.received_mask = receipt.mask;
  ready.accepted = true;
  ready.status = "OBSERVATION_READY";
  pubObservationReady->publish(ready);
  lastReadyIteration = batch.iteration;

  RCLCPP_INFO(
    SLROSNodePtr->get_logger(),
    "OBSERVATION_READY session=%llu epoch=%llu iteration=%llu "
    "required_mask=%u received_mask=%u",
    static_cast<unsigned long long>(ready.session_id),
    static_cast<unsigned long long>(ready.step_id),
    static_cast<unsigned long long>(ready.iteration),
    static_cast<unsigned int>(ready.required_mask),
    static_cast<unsigned int>(ready.received_mask));

  sensorReceipts.erase(sensorReceipts.begin(), std::next(receiptIt));
  pendingBatch.reset();
}

void sensorBatchPublishedCallback(
  const flightcore_gazebo_msgs::msg::SensorBatchPublished::SharedPtr msg)
{
  // Gazebo owns required_mask. This runtime validates producer completion but
  // deliberately does not recalculate the GPS divider.
  std::lock_guard<std::mutex> lock(observationMutex);
  if (!msg->accepted ||
    msg->required_mask == 0U ||
    (msg->published_mask & msg->required_mask) != msg->required_mask)
  {
    RCLCPP_ERROR(
      SLROSNodePtr->get_logger(),
      "SENSOR_BATCH_REJECTED iteration=%llu required_mask=%u "
      "published_mask=%u status=%s",
      static_cast<unsigned long long>(msg->iteration),
      static_cast<unsigned int>(msg->required_mask),
      static_cast<unsigned int>(msg->published_mask),
      msg->status.c_str());
    return;
  }
  if (lastReadyIteration.has_value() &&
    msg->iteration <= lastReadyIteration.value())
  {
    return;
  }
  if (pendingBatch.has_value() &&
    pendingBatch->iteration != msg->iteration)
  {
    RCLCPP_ERROR(
      SLROSNodePtr->get_logger(),
      "SENSOR_BATCH_FUTURE current_iteration=%llu received_iteration=%llu",
      static_cast<unsigned long long>(pendingBatch->iteration),
      static_cast<unsigned long long>(msg->iteration));
    return;
  }
  pendingBatch = *msg;
  tryPublishObservationReadyLocked();
}

}  // namespace

void notifyFlightCoreSensorReceived(
  const std::string & topic,
  const std::uint32_t sequence,
  const std::int64_t simTimeNs)
{
  // This hook is called by the generic generated subscriber callback after it
  // has moved the ROS message into the model-facing cache. Therefore setting a
  // received bit proves both DDS callback execution and payload availability.
  const std::uint8_t sensorMask =
    topic == "/flightcore/gazebo/imu" ? kImuMask :
    topic == "/flightcore/gazebo/gps" ? kGpsMask : 0U;
  if (sensorMask == 0U) {
    return;
  }

  std::lock_guard<std::mutex> lock(observationMutex);
  if (lastReadyIteration.has_value() &&
    static_cast<std::uint64_t>(sequence) <= lastReadyIteration.value())
  {
    return;
  }
  auto & receipt = sensorReceipts[sequence];
  receipt.mask |= sensorMask;
  if (sensorMask == kImuMask) {
    receipt.imuTimeNs = simTimeNs;
  } else {
    receipt.gpsTimeNs = simTimeNs;
  }
  tryPublishObservationReadyLocked();
}
//
// Function to publish notification update status of step call
//
static void publishDone(bool status) {
  static std_msgs::msg::String notifyMsg;
  // Publish model name
  if(status == false){
    notifyMsg.data = "-FlightCore_Gazebo_loop";
  } else {
    notifyMsg.data = "+FlightCore_Gazebo_loop";
  }
  pubStepNotify->publish(notifyMsg);
  if (status == true) {
    modelRuns = false;
  }
}
namespace ros2 {
namespace matlab {
NodeInterface::NodeInterface()
    : mModel()
    , mExec()
    , mBaseRateSem()
    , mBaseRateThread()
    , mSchedulerThread()
    , mStopSem()
    , mRunModel(true){
  }
NodeInterface::~NodeInterface() {
    terminate();
  }
void NodeInterface::initialize(int argc, char * const argv[]) {
    try {
        //initialize ros2
        std::vector<char *> args(argv, argv + argc);
        rclcpp::init(static_cast<int>(args.size()), args.data());
        //create the Node specified in Model
        std::string NodeName("FlightCore_Gazebo_loop");
        SLROSNodePtr = std::make_shared<rclcpp::Node>(NodeName);
        RCLCPP_INFO(SLROSNodePtr->get_logger(),"** Starting the model \"FlightCore_Gazebo_loop\" **\n");
        mExec = std::make_shared<rclcpp::executors::SLMultiThreadedExecutor>();
        mExec->add_node(SLROSNodePtr);
        //initialize the model which will initialize the publishers and subscribers
        mModel = std::make_shared<FlightCore_Gazebo_loop>(
        );
        ROS_SET_RTM_ERROR_STATUS(NULL);
        mModel->initialize();
        //create the threads for the rates in the Model
        mBaseRateThread = std::make_shared<std::thread>(&NodeInterface::baseRateTask, this);
		mSchedulerThread = std::make_shared<std::thread>(&NodeInterface::schedulerThreadCallback, this);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        throw ex;
    }
    catch (...) {
        std::cout << "Unknown exception" << std::endl;
        throw;
    }
}
int NodeInterface::run() {
  // Wait for ROS2Time scheduler to finish
  mStopSem.wait();
  mRunModel = false;
  return 0;
}
boolean_T NodeInterface::getStopRequestedFlag(void) {
    #ifndef rtmGetStopRequested
    return (!(ROS_GET_RTM_ERROR_STATUS()
        == (NULL)));
    #else
    return (!(ROS_GET_RTM_ERROR_STATUS()
        == (NULL)) || rtmGetStopRequested(mModel->getRTM()));
    #endif
}
void NodeInterface::stop(void) {
  if (mExec.get()) {
    mExec->cancel();
    if (SLROSNodePtr) {
      mExec->remove_node(SLROSNodePtr);
    }
    while (mExec.use_count() > 1);
  }
}
void NodeInterface::terminate(void) {
    if (mBaseRateThread.get()) {
        mRunModel = false;
        mBaseRateSem.notify(); // break out wait
        mBaseRateThread->join();
        if (mSchedulerThread.get()) {
            mSchedulerThread->join();
            mSchedulerThread.reset();
        }
        mBaseRateThread.reset();
        if (mModel.get()) {
            mModel->terminate();
        }
        mModel.reset();
        mExec.reset();
        SLROSNodePtr.reset();
        rclcpp::shutdown();
    }
}
//
// ROS Clock topic subscriber callback to run the base rate task
//
void NodeInterface::rosClockSubscriberCallback(const rosgraph_msgs::msg::Clock::SharedPtr msg)
{
  static uint64_t lastEndTime = 0;
  const uint64_t baseRateNanoSec = 1000000;
  const uint64_t currentTime = static_cast<uint64_t>(msg->clock.nanosec) + static_cast<uint64_t>(msg->clock.sec) * 1E9;
  if (lastEndTime == 0) {
    RCLCPP_INFO(SLROSNodePtr->get_logger(),"** Unblocking base-rate at %.6f", (double)currentTime/1E9);
    // Notify model stepping
    modelRuns = true;
    mBaseRateSem.notify();
    // Update lastEndTime
    lastEndTime = currentTime;
  } else {
    const uint64_t interval = currentTime - lastEndTime;
    if (interval >= baseRateNanoSec) {
      uint64_t missedSteps = interval/baseRateNanoSec;
      if (missedSteps < 2) {
        // Missed only one step
        if (modelRuns) {
          // Overrun happens
          ++numOverruns;
          RCLCPP_ERROR(SLROSNodePtr->get_logger(),"Overrun %d\n", numOverruns);
        }
        // Notify model stepping
        modelRuns = true;
        mBaseRateSem.notify();
      } else {
        for (uint64_t i=0; i<missedSteps; ++i) {
          while (modelRuns) {
            // Wait until model finishing the current step
          }
          // Notify model stepping
          modelRuns = true;
          mBaseRateSem.notify();
        }
      }
      lastEndTime = currentTime;
    } else {
      // Publish "done", because there is nothing to do in this time slot
      publishDone(false);
    }
  }
}
//
// Scheduler Task using ROS 2 Time published by "/clock" topic to run base-rate
//
void NodeInterface::schedulerThreadCallback(void)
{
  rclcpp::QoS qos(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default));
#ifdef MW_DEBUG_LOG
  RCLCPP_INFO(SLROSNodePtr->get_logger(),"ROS 2 schedulerTask entered\n");
#endif
  std::string sClockTopic = "/clock";
  pubStepNotify = SLROSNodePtr->create_publisher<std_msgs::msg::String>("/flightcore/gazebo/step_notify", qos);
  const auto barrierQos = rclcpp::QoS(rclcpp::KeepLast(8)).reliable();
  // Barrier endpoints are created before executor spin. Reliable DDS retains
  // messages arriving during discovery; sensor callbacks cannot run before the
  // publisher exists because the same executor has not started spinning yet.
  pubObservationReady =
    SLROSNodePtr->create_publisher<
    flightcore_gazebo_msgs::msg::ObservationReady>(
    "/flightcore/gazebo/observation_ready", barrierQos);
  auto sensorBatchSub =
    SLROSNodePtr->create_subscription<
    flightcore_gazebo_msgs::msg::SensorBatchPublished>(
    "/flightcore/gazebo/sensor_batch_published",
    barrierQos,
    sensorBatchPublishedCallback);
  auto sub = SLROSNodePtr->create_subscription<rosgraph_msgs::msg::Clock>(sClockTopic, qos, std::bind(&ros2::matlab::NodeInterface::rosClockSubscriberCallback,this,std::placeholders::_1));
  if (mExec) {
    mExec->spin();
  }
  // Unblock main thread and terminate
  mStopSem.notify();
  return;
}
//
//Model specific
// Base-rate task
void NodeInterface::baseRateTask(void) {
  mRunModel = (ROS_GET_RTM_ERROR_STATUS() ==
              (NULL));
  while (mRunModel) {
    mBaseRateSem.wait();
#ifdef MW_DEBUG_LOG
    RCLCPP_INFO(SLROSNodePtr->get_logger(),"** Base rate task semaphore received\n");
#endif
    if (!mRunModel) break;
    mModel->step();
    mRunModel &= !NodeInterface::getStopRequestedFlag(); //If RunModel and not stop requested
    publishDone(true);
  }
  NodeInterface::stop();
}
}//namespace matlab
}//namespace ros2
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif //_MSC_VER
