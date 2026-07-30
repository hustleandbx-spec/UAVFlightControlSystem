
//
// File ros2nodeinterface.cpp
//
// Code generated for Simulink model 'ros2_time_stepping_probe'.
//
// Model version                  : 1.2
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Tue Jul 28 16:51:10 2026
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
#include "ros2_time_stepping_probe.h"
#include "ros2nodeinterface.h"
#include <thread>
#include <chrono>
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
//
// Function to publish notification update status of step call
//
static void publishDone(bool status) {
  static std_msgs::msg::String notifyMsg;
  // Publish model name
  if(status == false){
    notifyMsg.data = "-ros2_time_stepping_probe";
  } else {
    notifyMsg.data = "+ros2_time_stepping_probe";
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
        std::string NodeName("ros2_time_stepping_probe");
        SLROSNodePtr = std::make_shared<rclcpp::Node>(NodeName);
        RCLCPP_INFO(SLROSNodePtr->get_logger(),"** Starting the model \"ros2_time_stepping_probe\" **\n");
        mExec = std::make_shared<rclcpp::executors::SLMultiThreadedExecutor>();
        mExec->add_node(SLROSNodePtr);
        //initialize the model which will initialize the publishers and subscribers
        mModel = std::make_shared<ros2_time_stepping_probe>(
        );
        ROS_SET_RTM_ERROR_STATUS(NULL);
        mModel->initialize();
        //create the threads for the rates in the Model
        mBaseRateThread = std::make_shared<std::thread>(&NodeInterface::baseRateTask, this);
		mSchedulerThread = std::make_shared<std::thread>(&NodeInterface::schedulerThreadCallback, this);
		for(size_t ctr = 0; ctr<SLROSSubscribers.size();ctr++){
           mExec->stopSubscriberCallback(SLROSSubscribers[ctr]);
       }
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
  pubStepNotify = SLROSNodePtr->create_publisher<std_msgs::msg::String>("/flightcore/time_probe/step_notify", qos);
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
