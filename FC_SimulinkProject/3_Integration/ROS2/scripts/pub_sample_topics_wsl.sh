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

export AMENT_PREFIX_PATH="${WS}/install/flightcore_msgs:${AMENT_PREFIX_PATH:-}"

ros2 topic pub --once -w 0 /uav/sensors/imu flightcore_msgs/msg/Imu \
  "{timestamp_sec: 0.0, sequence: 1, source_id: 1, valid: true, accel_mps2: [0.0, 0.0, -9.80665], gyro_radps: [0.0, 0.0, 0.0]}"

ros2 topic pub --once -w 0 /uav/sensors/gps flightcore_msgs/msg/Gps \
  "{timestamp_sec: 0.0, sequence: 1, source_id: 1, valid: true, lat_deg: 0.0, lon_deg: 0.0, alt_m: 0.0, velocity_ned_mps: [0.0, 0.0, 0.0]}"

ros2 topic pub --once -w 0 /uav/cmd/flight flightcore_msgs/msg/FlightCmd \
  "{timestamp_sec: 0.0, sequence: 1, source_id: 1, valid: true, mode: 1, position_ned_sp_m: [0.0, 0.0, -5.0], velocity_ned_sp_mps: [0.0, 0.0, 0.0], yaw_sp_rad: 0.0}"

echo "FLIGHTCORE_ROS2_SAMPLE_TOPICS_PUBLISHED"
