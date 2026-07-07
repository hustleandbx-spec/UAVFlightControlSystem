cmake_minimum_required(VERSION 3.8)
project(flightcore_msgs)

find_package(ament_cmake REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Imu.msg"
  "msg/Gps.msg"
  "msg/FlightCmd.msg"
  "msg/EscCmd.msg"
  "msg/StateEst.msg"
  "msg/SystemHealth.msg"
  DEPENDENCIES builtin_interfaces
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
