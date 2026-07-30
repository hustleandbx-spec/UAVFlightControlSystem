#!/usr/bin/env bash
set -euo pipefail

workspace="${1:-/home/hustle/uavsingle_ros2_ws}"
steps="${2:-100}"
session_id="${3:-2026072702}"
probe_log="/tmp/flightcore_coordinator_probe.log"

set +u
source /opt/ros/jazzy/setup.bash
source "${workspace}/install/setup.bash"
set -u

: > "${probe_log}"
setsid ros2 launch flightcore_gazebo_system \
    flightcore_gazebo_cosim.launch.py gui:=false \
    > "${probe_log}" 2>&1 &
launch_pgid=$!

cleanup() {
    kill -TERM -- "-${launch_pgid}" 2>/dev/null || true
    wait "${launch_pgid}" 2>/dev/null || true
}
trap cleanup EXIT

deadline=$((SECONDS + 30))
while (( SECONDS < deadline )); do
    if ros2 service type /flightcore/gazebo/prime_session >/dev/null 2>&1; then
        break
    fi
    sleep 0.1
done
if ! ros2 service type /flightcore/gazebo/prime_session >/dev/null 2>&1; then
    echo "PrimeSession did not become ready." >&2
    exit 1
fi

ros2 run flightcore_gazebo_system probe_lightweight_cosim.py \
    --steps "${steps}" \
    --session "${session_id}" \
    --timeout 10

grep -E \
    'CONTROL_READY_ACK|COMMAND_CACHED_ACK|WORLD_CONTROL_ACCEPTED|PLANT_STEP_DONE_ACK|RESULT_READY_ACK|COMMIT_RELEASE' \
    "${probe_log}" |
    tail -12
