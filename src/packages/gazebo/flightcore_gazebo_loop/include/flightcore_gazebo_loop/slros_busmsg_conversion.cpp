#ifndef _SLROS_BUSMSG_CONVERSION_H_
#define _SLROS_BUSMSG_CONVERSION_H_

#include "rclcpp/rclcpp.hpp"
#include <builtin_interfaces/msg/time.hpp>
#include <flightcore_gazebo_msgs/msg/actuator_command.hpp>
#include <flightcore_msgs/msg/gps.hpp>
#include <flightcore_msgs/msg/imu.hpp>
#include "FlightCore_Gazebo_loop_types.h"
#include "slros_msgconvert_utils.h"


[[maybe_unused]] static void convertFromBus(builtin_interfaces::msg::Time& msgPtr, SL_Bus_builtin_interfaces_Time const* busPtr);
[[maybe_unused]] static void convertToBus(SL_Bus_builtin_interfaces_Time* busPtr, const builtin_interfaces::msg::Time& msgPtr);

[[maybe_unused]] static void convertFromBus(flightcore_gazebo_msgs::msg::ActuatorCommand& msgPtr, SL_Bus_flightcore_gazebo_msgs_ActuatorCommand const* busPtr);
[[maybe_unused]] static void convertToBus(SL_Bus_flightcore_gazebo_msgs_ActuatorCommand* busPtr, const flightcore_gazebo_msgs::msg::ActuatorCommand& msgPtr);

[[maybe_unused]] static void convertFromBus(flightcore_msgs::msg::Gps& msgPtr, SL_Bus_flightcore_msgs_Gps const* busPtr);
[[maybe_unused]] static void convertToBus(SL_Bus_flightcore_msgs_Gps* busPtr, const flightcore_msgs::msg::Gps& msgPtr);

[[maybe_unused]] static void convertFromBus(flightcore_msgs::msg::Imu& msgPtr, SL_Bus_flightcore_msgs_Imu const* busPtr);
[[maybe_unused]] static void convertToBus(SL_Bus_flightcore_msgs_Imu* busPtr, const flightcore_msgs::msg::Imu& msgPtr);



// Conversions between SL_Bus_builtin_interfaces_Time and builtin_interfaces::msg::Time

[[maybe_unused]] static void convertFromBus(builtin_interfaces::msg::Time& msgPtr, SL_Bus_builtin_interfaces_Time const* busPtr)
{
  const std::string rosMessageType("builtin_interfaces/Time");

  msgPtr.nanosec =  busPtr->nanosec;
  msgPtr.sec =  busPtr->sec;
}

[[maybe_unused]] static void convertToBus(SL_Bus_builtin_interfaces_Time* busPtr, const builtin_interfaces::msg::Time& msgPtr)
{
  const std::string rosMessageType("builtin_interfaces/Time");

  busPtr->nanosec =  msgPtr.nanosec;
  busPtr->sec =  msgPtr.sec;
}


// Conversions between SL_Bus_flightcore_gazebo_msgs_ActuatorCommand and flightcore_gazebo_msgs::msg::ActuatorCommand

[[maybe_unused]] static void convertFromBus(flightcore_gazebo_msgs::msg::ActuatorCommand& msgPtr, SL_Bus_flightcore_gazebo_msgs_ActuatorCommand const* busPtr)
{
  const std::string rosMessageType("flightcore_gazebo_msgs/ActuatorCommand");

  convertFromBusFixedPrimitiveArray(msgPtr.actuator_values, busPtr->actuator_values);
  msgPtr.armed =  busPtr->armed;
  msgPtr.command_id =  busPtr->command_id;
  msgPtr.source_step_id =  busPtr->source_step_id;
  msgPtr.target_step_id =  busPtr->target_step_id;
  msgPtr.valid =  busPtr->valid;
  msgPtr.valid_from_iteration =  busPtr->valid_from_iteration;
}

[[maybe_unused]] static void convertToBus(SL_Bus_flightcore_gazebo_msgs_ActuatorCommand* busPtr, const flightcore_gazebo_msgs::msg::ActuatorCommand& msgPtr)
{
  const std::string rosMessageType("flightcore_gazebo_msgs/ActuatorCommand");

  convertToBusFixedPrimitiveArray(busPtr->actuator_values, msgPtr.actuator_values, slros::NoopWarning());
  busPtr->armed =  msgPtr.armed;
  busPtr->command_id =  msgPtr.command_id;
  busPtr->source_step_id =  msgPtr.source_step_id;
  busPtr->target_step_id =  msgPtr.target_step_id;
  busPtr->valid =  msgPtr.valid;
  busPtr->valid_from_iteration =  msgPtr.valid_from_iteration;
}


// Conversions between SL_Bus_flightcore_msgs_Gps and flightcore_msgs::msg::Gps

[[maybe_unused]] static void convertFromBus(flightcore_msgs::msg::Gps& msgPtr, SL_Bus_flightcore_msgs_Gps const* busPtr)
{
  const std::string rosMessageType("flightcore_msgs/Gps");

  msgPtr.alt_m =  busPtr->alt_m;
  msgPtr.lat_deg =  busPtr->lat_deg;
  msgPtr.lon_deg =  busPtr->lon_deg;
  msgPtr.sequence =  busPtr->sequence;
  msgPtr.source_id =  busPtr->source_id;
  convertFromBus(msgPtr.stamp, &busPtr->stamp);
  msgPtr.timestamp_sec =  busPtr->timestamp_sec;
  msgPtr.valid =  busPtr->valid;
  convertFromBusFixedPrimitiveArray(msgPtr.velocity_ned_mps, busPtr->velocity_ned_mps);
}

[[maybe_unused]] static void convertToBus(SL_Bus_flightcore_msgs_Gps* busPtr, const flightcore_msgs::msg::Gps& msgPtr)
{
  const std::string rosMessageType("flightcore_msgs/Gps");

  busPtr->alt_m =  msgPtr.alt_m;
  busPtr->lat_deg =  msgPtr.lat_deg;
  busPtr->lon_deg =  msgPtr.lon_deg;
  busPtr->sequence =  msgPtr.sequence;
  busPtr->source_id =  msgPtr.source_id;
  convertToBus(&busPtr->stamp, msgPtr.stamp);
  busPtr->timestamp_sec =  msgPtr.timestamp_sec;
  busPtr->valid =  msgPtr.valid;
  convertToBusFixedPrimitiveArray(busPtr->velocity_ned_mps, msgPtr.velocity_ned_mps, slros::NoopWarning());
}


// Conversions between SL_Bus_flightcore_msgs_Imu and flightcore_msgs::msg::Imu

[[maybe_unused]] static void convertFromBus(flightcore_msgs::msg::Imu& msgPtr, SL_Bus_flightcore_msgs_Imu const* busPtr)
{
  const std::string rosMessageType("flightcore_msgs/Imu");

  convertFromBusFixedPrimitiveArray(msgPtr.accel_mps2, busPtr->accel_mps2);
  convertFromBusFixedPrimitiveArray(msgPtr.gyro_radps, busPtr->gyro_radps);
  msgPtr.sequence =  busPtr->sequence;
  msgPtr.source_id =  busPtr->source_id;
  convertFromBus(msgPtr.stamp, &busPtr->stamp);
  msgPtr.timestamp_sec =  busPtr->timestamp_sec;
  msgPtr.valid =  busPtr->valid;
}

[[maybe_unused]] static void convertToBus(SL_Bus_flightcore_msgs_Imu* busPtr, const flightcore_msgs::msg::Imu& msgPtr)
{
  const std::string rosMessageType("flightcore_msgs/Imu");

  convertToBusFixedPrimitiveArray(busPtr->accel_mps2, msgPtr.accel_mps2, slros::NoopWarning());
  convertToBusFixedPrimitiveArray(busPtr->gyro_radps, msgPtr.gyro_radps, slros::NoopWarning());
  busPtr->sequence =  msgPtr.sequence;
  busPtr->source_id =  msgPtr.source_id;
  convertToBus(&busPtr->stamp, msgPtr.stamp);
  busPtr->timestamp_sec =  msgPtr.timestamp_sec;
  busPtr->valid =  msgPtr.valid;
}



#endif
