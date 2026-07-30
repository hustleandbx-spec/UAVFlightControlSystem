#!/usr/bin/env bash
set -euo pipefail

workspace="${1:-/home/hustle/uavsingle_ros2_ws}"
pid_file="/tmp/flightcore_gazebo_runtime.pid"

if [[ -z "${DISPLAY:-}" && -z "${WAYLAND_DISPLAY:-}" ]]; then
    echo "Gazebo GUI requires an active WSLg display." >&2
    exit 3
fi

set +u
source /opt/ros/jazzy/setup.bash
source "${workspace}/install/setup.bash"
set -u
unset RMW_IMPLEMENTATION CYCLONEDDS_URI FASTDDS_DEFAULT_PROFILES_FILE

if [[ -f "${pid_file}" ]]; then
    existing_pid="$(cat "${pid_file}")"
    if kill -0 "${existing_pid}" 2>/dev/null; then
        echo "FlightCore-Gazebo runtime is already running with PID ${existing_pid}." >&2
        exit 2
    fi
    rm -f "${pid_file}"
fi

setsid ros2 launch flightcore_gazebo_system \
    flightcore_gazebo_cosim.launch.py gui:=true &
launch_pid=$!
echo "${launch_pid}" > "${pid_file}"
echo "FLIGHTCORE_RUNTIME_OWNED pgid=${launch_pid}"

cleanup() {
    rm -f "${pid_file}"
}
trap cleanup EXIT

wait "${launch_pid}"
