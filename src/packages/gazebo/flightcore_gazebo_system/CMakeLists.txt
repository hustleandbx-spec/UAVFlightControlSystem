cmake_minimum_required(VERSION 3.8)
project(flightcore_gazebo_system)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(flightcore_gazebo_msgs REQUIRED)
find_package(flightcore_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(gz-sim8 REQUIRED)
find_package(gz-plugin2 REQUIRED COMPONENTS register)

add_library(flightcore_gazebo_system SHARED
  src/flightcore_gazebo_system.cpp
)

target_compile_features(flightcore_gazebo_system PUBLIC cxx_std_17)
target_link_libraries(flightcore_gazebo_system
  gz-sim8::gz-sim8
  gz-plugin2::gz-plugin2
)
ament_target_dependencies(flightcore_gazebo_system
  rclcpp
  flightcore_gazebo_msgs
  flightcore_msgs
  std_srvs
)

install(TARGETS flightcore_gazebo_system
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY config launch worlds
  DESTINATION share/${PROJECT_NAME}
)
install(PROGRAMS scripts/probe_lightweight_cosim.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
