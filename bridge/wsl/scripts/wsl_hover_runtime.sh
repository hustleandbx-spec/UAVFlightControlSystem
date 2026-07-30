#!/usr/bin/env bash
set -euo pipefail

EPISODE_DIR="${1:?missing EPISODE_DIR}"
DURATION="${2:?missing DURATION}"
ENDPOINT_HOST="${3:?missing ENDPOINT_HOST}"

WS_DIR="${HOME}/uavsingle_ros2_ws"
BRIDGE_LOG="${EPISODE_DIR}/bridge.log"
ADAPTER_LOG="${EPISODE_DIR}/adapter.log"
ROSBAG_LOG="${EPISODE_DIR}/rosbag.log"
CLOCK_LOG="${EPISODE_DIR}/clock_offsets.log"
BAG_DIR="${EPISODE_DIR}/rosbag2"
ROSBAG_QOS="${EPISODE_DIR}/rosbag_qos_overrides.yaml"

mkdir -p "${EPISODE_DIR}/plots"
echo "WSL episode dir: ${EPISODE_DIR}"
echo "Endpoint host for actuator UDP: ${ENDPOINT_HOST}"

set +u
source /opt/ros/jazzy/setup.bash
source "${WS_DIR}/install/setup.bash"
set -u

# ── DDS 配置 ──────────────────────────────────────────────────────
# CycloneDDS 替代 Fast-DDS：NAT 模式下 Fast-DDS 与 MATLAB 私有 DDS 数据面不互通。
# CycloneDDS 通过 DDSI-RTPS 线协议与 MATLAB DDS 双向匹配。
#
# 关键配置：禁用 shared memory，强制绑定 eth0 走 UDP multicast。
# WSL2 不同 wsl.exe 会话有独立的 shared memory 命名空间，这是导致
# "ros2 topic list 看不到其他会话的 topic" 的根本原因。
# 见 cyclone_dds_wsl.xml（由脚本自动生成到 WSL 侧 config/）。
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
CYCLONEDDS_CONFIG="${WS_DIR}/src/config/cyclonedds_wsl.xml"
if [[ ! -f "${CYCLONEDDS_CONFIG}" ]]; then
    mkdir -p "$(dirname "${CYCLONEDDS_CONFIG}")"
    cat >"${CYCLONEDDS_CONFIG}" <<'DDSXML'
<CycloneDDS>
  <Domain>
    <SharedMemory>
      <Enable>false</Enable>
    </SharedMemory>
    <General>
      <Interfaces>
        <NetworkInterface name="eth0"/>
      </Interfaces>
      <AllowMulticast>true</AllowMulticast>
    </General>
  </Domain>
</CycloneDDS>
DDSXML
fi
export CYCLONEDDS_URI="file://${CYCLONEDDS_CONFIG}"
echo "CycloneDDS config: ${CYCLONEDDS_CONFIG}"
# Unbuffered Python I/O ensures adapter logs are not lost on SIGTERM.
export PYTHONUNBUFFERED=1
if git -C "${WS_DIR}/src" rev-parse HEAD >/dev/null 2>&1; then
    echo "WSL commit: $(git -C "${WS_DIR}/src" rev-parse HEAD)"
else
    echo "WSL commit: unknown"
fi

BRIDGE_ARGS=(--ros-args -p "control_target_host:=${ENDPOINT_HOST}")
ADAPTER_ARGS=(--ros-args -p "actuator_target_host:=${ENDPOINT_HOST}" -p "gps_fallback_from_state:=true")

cleanup() {
kill "${ESC_TRACE_PID:-}" 2>/dev/null; sleep 0.2; kill -9 "${ESC_TRACE_PID:-}" 2>/dev/null || true

    kill "${STATUS_PID:-}" 2>/dev/null || true
    kill "${BAG_PID:-}" 2>/dev/null || true
    kill "${CLOCK_PID:-}" 2>/dev/null || true
    kill "${PJ_PID:-}" 2>/dev/null || true
    kill "${ADAPTER_PID:-}" 2>/dev/null || true
    kill "${BRIDGE_PID:-}" 2>/dev/null || true
    wait 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# ── 启动 ROS2 节点 ─────────────────────────────────────────────────
# 启动顺序：Bridge(接收UDP→发布/aircraft/*) → Adapter(订阅/aircraft/*→发布/uav/*)
# 两者在同一个 bash session，共享 CycloneDDS participant，topic 发现无问题。

existing_nodes="$(timeout 5 ros2 node list 2>/dev/null || true)"
if printf '%s\n' "${existing_nodes}" | grep -q '/aircraft_udp_bridge'; then
    echo "Bridge already discovered; attaching to existing node"
    BRIDGE_PID=""
else
    echo "Starting bridge..."
    ros2 run aircraft_udp_bridge aircraft_udp_bridge "${BRIDGE_ARGS[@]}" >>"${BRIDGE_LOG}" 2>&1 &
    BRIDGE_PID=$!
fi

if printf '%s\n' "${existing_nodes}" | grep -q '/flightcore_runtime_adapter'; then
    echo "Adapter already discovered; attaching to existing node"
    ADAPTER_PID=""
else
    echo "Starting adapter..."
    ros2 run flightcore_runtime_adapter flightcore_runtime_adapter "${ADAPTER_ARGS[@]}" >>"${ADAPTER_LOG}" 2>&1 &
    ADAPTER_PID=$!
fi

sleep 2

echo "Checking PlotJuggler..."
PLOTJUGGLER_BIN="/opt/ros/jazzy/lib/plotjuggler/plotjuggler"
PLOTJUGGLER_LAYOUT="${WS_DIR}/src/config/plotjuggler_flightcore_topics.xml"
if pgrep -f '/plotjuggler/plotjuggler' >/dev/null 2>&1; then
    echo "PlotJuggler already running; attaching to existing process"
    PJ_PID=""
elif [[ -x "${PLOTJUGGLER_BIN}" ]]; then
    if [[ -f "${PLOTJUGGLER_LAYOUT}" ]]; then
        "${PLOTJUGGLER_BIN}" -l "${PLOTJUGGLER_LAYOUT}" >>"${EPISODE_DIR}/plotjuggler.log" 2>&1 &
        PJ_PID=$!
        echo "PlotJuggler PID=${PJ_PID} (WSLg window should appear on Windows desktop)"
    else
        echo "WARNING: PlotJuggler layout not found at ${PLOTJUGGLER_LAYOUT}, launching without layout"
        "${PLOTJUGGLER_BIN}" >>"${EPISODE_DIR}/plotjuggler.log" 2>&1 &
        PJ_PID=$!
    fi
else
    echo "WARNING: PlotJuggler binary not found at ${PLOTJUGGLER_BIN}, skipping"
    PJ_PID=""
fi

if [[ -e "${BAG_DIR}" ]]; then
    if [[ -d "${BAG_DIR}" ]] && [[ -z "$(find "${BAG_DIR}" -mindepth 1 -maxdepth 1 -print -quit)" ]]; then
        rmdir "${BAG_DIR}"
    else
        echo "ERROR: rosbag output already exists and is not empty: ${BAG_DIR}" >&2
        exit 3
    fi
fi

cat >"${ROSBAG_QOS}" <<'YAML'
/uav/actuator/esc_cmd:
  reliability: best_effort
  durability: volatile
  history: keep_last
  depth: 10
/uav/estimator/state:
  reliability: best_effort
  durability: volatile
  history: keep_last
  depth: 10
/uav/health/status:
  reliability: best_effort
  durability: volatile
  history: keep_last
  depth: 10
YAML

if pgrep -f 'ros2 bag record' >/dev/null 2>&1; then
    echo "Rosbag recorder already running; attaching to existing process"
    BAG_PID=""
else
    echo "Starting rosbag record..."
    ros2 bag record \
        /aircraft/state /aircraft/imu /aircraft/gps \
        /uav/sensors/imu /uav/sensors/gps /uav/cmd/flight \
        /uav/actuator/esc_cmd /uav/estimator/state /uav/health/status \
        --qos-profile-overrides-path "${ROSBAG_QOS}" \
        -o "${BAG_DIR}" >>"${ROSBAG_LOG}" 2>&1 &
    BAG_PID=$!
fi

echo "Starting EscCmd tracer..."
ros2 topic echo --qos-reliability best_effort \
    /uav/actuator/esc_cmd >>"${EPISODE_DIR}/esccmd_trace.log" 2>&1 &
ESC_TRACE_PID=$!

python3 - "${DURATION}" "${EPISODE_DIR}/clock_offsets.csv" <<'PY' >>"${CLOCK_LOG}" 2>&1 &
import csv
import subprocess
import sys
import time

duration = float(sys.argv[1])
output = sys.argv[2]

def read_ros_time():
    try:
        result = subprocess.run(
            ["ros2", "topic", "echo", "/uav/health/status", "--once", "--field", "stamp"],
            capture_output=True,
            text=True,
            timeout=5.0,
        )
        sec = None
        nsec = None
        for line in result.stdout.splitlines():
            if "sec:" in line:
                sec = int(line.split(":", 1)[1].strip())
            if "nanosec:" in line:
                nsec = int(line.split(":", 1)[1].strip())
        if sec is not None and nsec is not None:
            return float(sec) + float(nsec) * 1e-9
    except Exception:
        pass
    return time.time()

fields = ["wall_clock_sec", "ros_time_sec", "simulink_time_sec", "packet_sequence", "source"]
with open(output, "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=fields)
    writer.writeheader()
    start = time.monotonic()
    sequence = 0
    next_sample = start
    while time.monotonic() - start < duration:
        now = time.monotonic()
        if now < next_sample:
            time.sleep(min(next_sample - now, 0.05))
            continue
        next_sample += 1.0
        sequence += 1
        wall = time.time()
        ros = read_ros_time()
        writer.writerow({
            "wall_clock_sec": f"{wall:.6f}",
            "ros_time_sec": f"{ros:.6f}",
            "simulink_time_sec": "",
            "packet_sequence": str(sequence),
            "source": "clock_offsets_recorder_wall_ros",
        })
        f.flush()
        print(f"[{sequence:4d}] offset(ros-wall)={ros-wall:+.6f}s wall={wall:.3f} ros={ros:.3f}")
PY
CLOCK_PID=$!

# Periodic status reporter — writes a tiny JSON for the Windows PowerShell monitor
(
    while true; do
        nodes="$(timeout 5 ros2 node list 2>/dev/null || true)"
        bridge_alive=0; printf '%s\n' "${nodes}" | grep -q '/aircraft_udp_bridge' && bridge_alive=1
        adapter_alive=0; printf '%s\n' "${nodes}" | grep -q '/flightcore_runtime_adapter' && adapter_alive=1
        bag_alive=0; pgrep -f 'ros2 bag record' >/dev/null 2>&1 && bag_alive=1
        pj_alive=0; pgrep -f '/plotjuggler/plotjuggler' >/dev/null 2>&1 && pj_alive=1
        bag_size=$(du -sh "${BAG_DIR}" 2>/dev/null | cut -f1)
        printf '{"ts":%s,"bridge":%s,"adapter":%s,"rosbag":%s,"plotjuggler":%s,"bag_size":"%s"}\n' \
            "$(date +%s)" "$bridge_alive" "$adapter_alive" "$bag_alive" "$pj_alive" "$bag_size" \
            > "${EPISODE_DIR}/wsl_status.json"
        sleep 2
    done
) &
STATUS_PID=$!

echo "Episode running for ${DURATION}s"
sleep "${DURATION}" || true
echo "Stopping WSL runtime services"
cleanup
trap - EXIT
sleep 2

python3 - "${EPISODE_DIR}" <<'PY'
import csv
import json
import math
import pathlib
import sys

episode_dir = pathlib.Path(sys.argv[1])
bag_dir = episode_dir / "rosbag2"
bag_files = []
for pattern in ("metadata.yaml", "*.db3", "*.mcap", "*.bag", "*.bag2"):
    bag_files.extend(bag_dir.rglob(pattern) if bag_dir.exists() else [])
bag_files = sorted({str(p) for p in bag_files})

def line_count(path):
    try:
        return sum(1 for _ in open(path, encoding="utf-8", errors="ignore"))
    except Exception:
        return 0

clock_path = episode_dir / "clock_offsets.csv"
clock_offset_std = None
if clock_path.exists():
    offsets = []
    try:
        with open(clock_path, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                offsets.append(float(row["ros_time_sec"]) - float(row["wall_clock_sec"]))
        if offsets:
            mean = sum(offsets) / len(offsets)
            clock_offset_std = math.sqrt(sum((x - mean) ** 2 for x in offsets) / len(offsets))
    except Exception:
        clock_offset_std = None

summary = {
    "episode_dir": str(episode_dir),
    "rosbag_files": len(bag_files),
    "rosbag_sample": bag_files[:5],
    "clock_offsets_exists": clock_path.exists(),
    "clock_offset_std": clock_offset_std,
    "bridge_log_lines": line_count(episode_dir / "bridge.log"),
    "adapter_log_lines": line_count(episode_dir / "adapter.log"),
    "rosbag_log_lines": line_count(episode_dir / "rosbag.log"),
}
print("WSL_ARTIFACTS_JSON=" + json.dumps(summary, sort_keys=True))
with open(f"{episode_dir}/wsl_artifacts.json", "w") as af:
    json.dump(summary, af, sort_keys=True)
PY
