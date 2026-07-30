cmake_minimum_required(VERSION 3.8)
project(simulation_coordinator_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/ObservationReady.msg"
  "msg/ControlReady.msg"
  "msg/CommandQueued.msg"
  "msg/StepGrant.msg"
  "msg/PlantStepDone.msg"
  "msg/StepCommit.msg"
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()