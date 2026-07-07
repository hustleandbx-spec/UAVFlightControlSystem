#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROS2_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
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
else
  echo "ERROR: No supported ROS2 setup.bash found under /opt/ros." >&2
  exit 2
fi

mkdir -p "${WS}/src"
rm -rf "${WS}/src/flightcore_msgs"
cp -R "${ROS2_DIR}/flightcore_msgs" "${WS}/src/flightcore_msgs"

cd "${WS}"
unset CATKIN_INSTALL_INTO_PREFIX_ROOT
unset CATKIN_DEVEL_PREFIX
unset CATKIN_WORKSPACE
rm -rf build/flightcore_msgs
rm -rf install/flightcore_msgs
rm -rf log
colcon build --packages-select flightcore_msgs

echo "FLIGHTCORE_ROS2_WSL_BUILD_PASS"
echo "source ${WS}/install/setup.bash"
