#!/usr/bin/env bash
set -euo pipefail

WS="${FLIGHTCORE_ROS2_WS:-${HOME}/uavsingle_ros2_ws}"

if [[ -f /opt/ros/jazzy/setup.bash ]]; then
  set +u
  # shellcheck disable=SC1091
  source /opt/ros/jazzy/setup.bash
  set -u
elif [[ -f /opt/ros/humble/setup.bash ]]; then
  set +u
  # shellcheck disable=SC1091
  source /opt/ros/humble/setup.bash
  set -u
fi

if [[ -f "${WS}/install/setup.bash" ]]; then
  set +u
  # shellcheck disable=SC1091
  source "${WS}/install/setup.bash"
  set -u
fi

export AMENT_PREFIX_PATH="${WS}/install/flightcore_msgs:${AMENT_PREFIX_PATH:-}"

ros2 interface show flightcore_msgs/msg/Imu
ros2 interface show flightcore_msgs/msg/Gps
ros2 interface show flightcore_msgs/msg/FlightCmd
ros2 interface show flightcore_msgs/msg/EscCmd
ros2 interface show flightcore_msgs/msg/StateEst
ros2 interface show flightcore_msgs/msg/SystemHealth
