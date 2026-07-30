cmake_minimum_required(VERSION 3.8)
project(simulation_coordinator)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(simulation_coordinator_interfaces REQUIRED)
find_package(rosgraph_msgs REQUIRED)

add_library(clock_authority
  src/clock_authority.cpp
)

target_include_directories(clock_authority PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(clock_authority PUBLIC cxx_std_17)

add_executable(simulation_coordinator_node
  src/simulation_coordinator_node.cpp
)

target_compile_features(
  simulation_coordinator_node PUBLIC cxx_std_17
)

ament_target_dependencies(
  simulation_coordinator_node
  rclcpp
  rosgraph_msgs
  simulation_coordinator_interfaces
)

target_link_libraries(
  simulation_coordinator_node
  clock_authority
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

install(
  TARGETS
    clock_authority
    simulation_coordinator_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_package()