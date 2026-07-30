#ifndef _SLROS_BUSMSG_CONVERSION_H_
#define _SLROS_BUSMSG_CONVERSION_H_

#include "rclcpp/rclcpp.hpp"
#include <std_msgs/msg/u_int64.hpp>
#include "ros2_time_stepping_probe_types.h"
#include "slros_msgconvert_utils.h"


[[maybe_unused]] static void convertFromBus(std_msgs::msg::UInt64& msgPtr, SL_Bus_std_msgs_UInt64 const* busPtr);
[[maybe_unused]] static void convertToBus(SL_Bus_std_msgs_UInt64* busPtr, const std_msgs::msg::UInt64& msgPtr);



// Conversions between SL_Bus_std_msgs_UInt64 and std_msgs::msg::UInt64

[[maybe_unused]] static void convertFromBus(std_msgs::msg::UInt64& msgPtr, SL_Bus_std_msgs_UInt64 const* busPtr)
{
  const std::string rosMessageType("std_msgs/UInt64");

  msgPtr.data =  busPtr->data;
}

[[maybe_unused]] static void convertToBus(SL_Bus_std_msgs_UInt64* busPtr, const std_msgs::msg::UInt64& msgPtr)
{
  const std::string rosMessageType("std_msgs/UInt64");

  busPtr->data =  msgPtr.data;
}



#endif
