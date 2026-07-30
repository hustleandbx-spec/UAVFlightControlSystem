// Copyright 2022-2024 The MathWorks, Inc.
// Generated 29-Jul-2026 15:51:33
#include "slros2_initialize.h"
const std::string SLROSNodeName("FlightCore_Gazebo_loop");
// FlightCore_Gazebo_loop/GazeboEscCmd/Gazebo_ESC_Publish
SimulinkPublisher<flightcore_gazebo_msgs::msg::ActuatorCommand,SL_Bus_flightcore_gazebo_msgs_ActuatorCommand> Pub_FlightCore_Gazebo_loop_42;
// FlightCore_Gazebo_loop/GazeboGpsSubscribe
SimulinkSubscriber<flightcore_msgs::msg::Gps,SL_Bus_flightcore_msgs_Gps> Sub_FlightCore_Gazebo_loop_189;
// FlightCore_Gazebo_loop/GazeboImuSubscribe
SimulinkSubscriber<flightcore_msgs::msg::Imu,SL_Bus_flightcore_msgs_Imu> Sub_FlightCore_Gazebo_loop_187;
