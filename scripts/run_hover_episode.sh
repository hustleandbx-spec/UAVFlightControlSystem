#!/usr/bin/env bash
# WSL launch script for AirSim endpoint -> FlightCore hover episode
#
# Usage:
#   ./run_hover_episode.sh [--duration 30] [--episode-dir episodes/...]
#
# Prerequisites:
#   - ROS2 Jazzy workspace built: ~/uavsingle_ros2_ws
#   - Windows AirSim endpoint running (or use --mock for mock endpoint)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WS_DIR="${HOME}/uavsingle_ros2_ws"
EPISODE_DIR="${SCRIPT_DIR}/../episodes/$(date +%Y%m%d_%H%M%S)_airsim_hover_v0"
DURATION=30
MOCK=false
ENDPOINT_HOST=""  # Windows endpoint host as seen from WSL2

# Parse args
while [[ $# -gt 0 ]]; do
    case "$1" in
        --episode-dir) EPISODE_DIR="$2"; shift 2 ;;
        --duration) DURATION="$2"; shift 2 ;;
        --mock) MOCK=true; shift ;;
        --endpoint-host) ENDPOINT_HOST="$2"; shift 2 ;;
        --help)
            echo "Usage: $0 [--mock] [--duration 30] [--episode-dir <path>] [--endpoint-host <ip>]"
            exit 0
            ;;
        *) echo "Unknown: $1"; exit 1 ;;
    esac
done

# Source ROS2
source /opt/ros/jazzy/setup.bash
source "${WS_DIR}/install/setup.bash"

# Create episode directory.  Do not pre-create rosbag2: `ros2 bag record -o`
# expects to create the output directory itself.
mkdir -p "${EPISODE_DIR}/plots"
echo "Episode dir: ${EPISODE_DIR}"

# Record git commit
if git -C "${WS_DIR}/src" rev-parse HEAD &>/dev/null; then
    WSL_COMMIT=$(git -C "${WS_DIR}/src" rev-parse HEAD)
    echo "WSL commit: ${WSL_COMMIT}"
else
    WSL_COMMIT="unknown"
    echo "WARNING: WSL workspace not a git repo or no commits"
fi

# The bridge can learn a legacy control target from the first state packet, but
# the runtime adapter cannot learn where to send actuator UDP.  Use the WSL
# default gateway as the Windows host when the caller does not pass one.
if [[ -z "${ENDPOINT_HOST}" ]]; then
    ENDPOINT_HOST="$(ip route show default 2>/dev/null | awk '{print $3; exit}')"
fi
if [[ -z "${ENDPOINT_HOST}" ]]; then
    echo "ERROR: endpoint host is required; pass --endpoint-host <Windows host IP>" >&2
    exit 2
fi
echo "Endpoint host for actuator UDP: ${ENDPOINT_HOST}"

BRIDGE_ARGS=(--ros-args -p "control_target_host:=${ENDPOINT_HOST}")
ADAPTER_ARGS=(--ros-args -p "actuator_target_host:=${ENDPOINT_HOST}" -p "gps_fallback_from_state:=true")

# Log files
BRIDGE_LOG="${EPISODE_DIR}/bridge.log"
ADAPTER_LOG="${EPISODE_DIR}/adapter.log"
BAG_DIR="${EPISODE_DIR}/rosbag2"

cleanup() {
    kill "${BAG_PID:-}" 2>/dev/null || true
    kill "${CLOCK_PID:-}" 2>/dev/null || true
    kill "${ADAPTER_PID:-}" 2>/dev/null || true
    kill "${BRIDGE_PID:-}" 2>/dev/null || true
    wait 2>/dev/null || true
}
trap cleanup EXIT INT TERM

echo "Starting bridge (log: ${BRIDGE_LOG})..."
ros2 run aircraft_udp_bridge aircraft_udp_bridge "${BRIDGE_ARGS[@]}" \
    >> "${BRIDGE_LOG}" 2>&1 &
BRIDGE_PID=$!

echo "Starting adapter (log: ${ADAPTER_LOG})..."
ros2 run flightcore_runtime_adapter flightcore_runtime_adapter "${ADAPTER_ARGS[@]}" \
    >> "${ADAPTER_LOG}" 2>&1 &
ADAPTER_PID=$!

# Wait for nodes to initialize
sleep 2

# Start rosbag recording
if [[ -e "${BAG_DIR}" ]]; then
    if [[ -d "${BAG_DIR}" ]] && [[ -z "$(find "${BAG_DIR}" -mindepth 1 -maxdepth 1 -print -quit)" ]]; then
        rmdir "${BAG_DIR}"
    else
        echo "ERROR: rosbag output already exists and is not empty: ${BAG_DIR}" >&2
        exit 3
    fi
fi
echo "Starting rosbag record to ${BAG_DIR}..."
ros2 bag record \
    /aircraft/state /aircraft/imu /aircraft/gps \
    /uav/sensors/imu /uav/sensors/gps /uav/cmd/flight \
    /uav/actuator/esc_cmd /uav/estimator/state /uav/health/status \
    -o "${BAG_DIR}" \
    >> "${EPISODE_DIR}/rosbag.log" 2>&1 &
BAG_PID=$!

# Start clock offset recorder (in background)
python3 "${SCRIPT_DIR}/record_clock_offsets.py" \
    --duration "${DURATION}" \
    --interval 1.0 \
    --output "${EPISODE_DIR}/clock_offsets.csv" \
    >> "${EPISODE_DIR}/clock_offsets.log" 2>&1 &
CLOCK_PID=$!

echo ""
echo "=== Episode running ==="
echo "  Duration: ${DURATION}s"
echo "  PID bridge=${BRIDGE_PID} adapter=${ADAPTER_PID} bag=${BAG_PID} clock=${CLOCK_PID}"
echo ""
echo "Waiting for episode duration (${DURATION}s)..."
echo "  (Press Ctrl+C to stop early)"
echo ""

# Sleep for duration (or until interrupted)
sleep "${DURATION}" || true

echo ""
echo "=== Episode complete, stopping services ==="

cleanup
trap - EXIT
sleep 2

echo ""
echo "=== Episode saved to ${EPISODE_DIR} ==="
echo "  rosbag2: $(find "${BAG_DIR}" -maxdepth 2 \( -name metadata.yaml -o -name '*.db3' -o -name '*.mcap' -o -name '*.bag' -o -name '*.bag2' \) 2>/dev/null | head -5)"
echo "  bridge log: ${BRIDGE_LOG}"
echo "  adapter log: ${ADAPTER_LOG}"
echo "  clock_offsets: ${EPISODE_DIR}/clock_offsets.csv"

# Summary
BAG_SIZE=$(du -sh "${BAG_DIR}" 2>/dev/null | cut -f1 || echo "?")
BRIDGE_LINES=$(wc -l < "${BRIDGE_LOG}" 2>/dev/null || echo "?")
ADAPTER_LINES=$(wc -l < "${ADAPTER_LOG}" 2>/dev/null || echo "?")
echo "  bag size: ${BAG_SIZE}"
echo "  bridge lines: ${BRIDGE_LINES}"
echo "  adapter lines: ${ADAPTER_LINES}"
