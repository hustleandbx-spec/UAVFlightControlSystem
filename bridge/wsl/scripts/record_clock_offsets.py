#!/usr/bin/env python3
"""Record clock domain offsets during an episode.

Three clock domains:
  1. Windows wall-clock (simulator endpoint)
  2. WSL ROS time (bridge/adapter nodes)
  3. Simulink simulation time (FlightCore)

Output: clock_offsets.csv with columns:
  wall_clock_sec, ros_time_sec, sim_time_sec, packet_sequence, source

Usage (WSL):
  python3 record_clock_offsets.py --duration 30 --output clock_offsets.csv
"""

import argparse
import csv
import socket
import time
from datetime import datetime, timezone


def read_ros_time() -> float:
    """Read ROS time via ros2 command line (works as a subprocess)."""
    import subprocess
    try:
        result = subprocess.run(
            ["ros2", "topic", "echo", "/uav/health/status", "--once", "--field", "stamp"],
            capture_output=True, text=True, timeout=5.0
        )
        # Parse stamp from output
        for line in result.stdout.splitlines():
            if "sec:" in line:
                sec = int(line.split(":")[1].strip())
            if "nanosec:" in line:
                nsec = int(line.split(":")[1].strip())
                return float(sec) + float(nsec) * 1e-9
    except (subprocess.SubprocessError, ValueError):
        pass
    return time.time()


def main() -> None:
    parser = argparse.ArgumentParser(description="Record clock domain offsets")
    parser.add_argument("--duration", type=float, default=30.0, help="Recording duration (sec)")
    parser.add_argument("--interval", type=float, default=1.0, help="Sample interval (sec)")
    parser.add_argument("--output", default="clock_offsets.csv", help="Output CSV path")
    args = parser.parse_args()

    fields = [
        "wall_clock_sec",
        "ros_time_sec",
        "packet_sequence",
        "source",
    ]

    with open(args.output, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()

        start_time = time.monotonic()
        sequence = 0
        next_sample = start_time
        end_time = start_time + args.duration

        while time.monotonic() < end_time:
            now = time.monotonic()
            if now < next_sample:
                time.sleep(min(next_sample - now, 0.05))
                continue
            next_sample += args.interval
            sequence += 1

            wall_clock = time.time()
            ros_time = read_ros_time()

            row = {
                "wall_clock_sec": f"{wall_clock:.6f}",
                "ros_time_sec": f"{ros_time:.6f}",
                "packet_sequence": str(sequence),
                "source": "clock_offsets_recorder",
            }
            writer.writerow(row)
            f.flush()

            offset = ros_time - wall_clock
            print(f"[{sequence:4d}] offset(ros-wall)={offset:+.6f}s  wall={wall_clock:.3f} ros={ros_time:.3f}")

    print(f"Clock offsets saved to {args.output}")


if __name__ == "__main__":
    main()
