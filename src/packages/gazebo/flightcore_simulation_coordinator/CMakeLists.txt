cmake_minimum_required(VERSION 3.8)
project(flightcore_simulation_coordinator)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
# /clock 使用 ROS 标准时钟消息；启动门使用标准 Trigger 服务。
find_package(rosgraph_msgs REQUIRED)
# 生成节点用 std_msgs/String 在 step_notify 上确认完整 mModel->step() 已返回。
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(flightcore_gazebo_msgs REQUIRED)
find_package(gz-msgs10 REQUIRED)
find_package(gz-transport13 REQUIRED)

add_executable(flightcore_simulation_coordinator
  src/flightcore_simulation_coordinator.cpp
)
target_compile_features(flightcore_simulation_coordinator PUBLIC cxx_std_17)
target_link_libraries(flightcore_simulation_coordinator
  gz-msgs10::gz-msgs10
  gz-transport13::gz-transport13
)
ament_target_dependencies(flightcore_simulation_coordinator
  rclcpp
  rosgraph_msgs
  std_msgs
  std_srvs
  flightcore_gazebo_msgs
)

install(TARGETS flightcore_simulation_coordinator
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
