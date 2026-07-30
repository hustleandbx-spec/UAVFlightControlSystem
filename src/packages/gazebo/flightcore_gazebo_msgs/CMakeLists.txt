cmake_minimum_required(VERSION 3.8)
project(flightcore_gazebo_msgs)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  # 以下消息共同构成轻量锁步 epoch 的显式屏障协议。
  "msg/ActuatorCommand.msg"
  "msg/CommandCached.msg"
  "msg/PlantStepDone.msg"
  "msg/ResultReady.msg"
  "msg/CommitRelease.msg"
  "msg/SensorBatchPublished.msg"
  "msg/ObservationReady.msg"
  "srv/PrimeSession.srv"
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
