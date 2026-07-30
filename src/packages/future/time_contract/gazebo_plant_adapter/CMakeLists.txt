cmake_minimum_required(VERSION 3.8)
project(gazebo_plant_adapter)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(ros_gz_interfaces REQUIRED)
find_package(simulation_coordinator_interfaces REQUIRED)

add_executable(gazebo_step_adapter_node
  src/gazebo_step_adapter_node.cpp
)

target_compile_features(
  gazebo_step_adapter_node
  PUBLIC cxx_std_17
)

ament_target_dependencies(
  gazebo_step_adapter_node
  rclcpp
  ros_gz_interfaces
  simulation_coordinator_interfaces
)

install(
  TARGETS gazebo_step_adapter_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()