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
else
  echo "ERROR: ${WS}/install/setup.bash not found. Run build_flightcore_msgs_wsl.sh first." >&2
  exit 2
fi

echo "WARNING: flightcore_ros2_gateway is a legacy RuntimeBridge/SimAdapter fallback." >&2
echo "WARNING: It is not part of the FlightCore_ROS2_loop -> ROS2 DDS -> PlotJuggler mainline." >&2

export AMENT_PREFIX_PATH="${WS}/install/flightcore_msgs:${WS}/install/flightcore_ros2_gateway:${AMENT_PREFIX_PATH:-}"

ros2 launch flightcore_ros2_gateway runtime_gateway.launch.py
