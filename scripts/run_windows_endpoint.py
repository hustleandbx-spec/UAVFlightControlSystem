#!/usr/bin/env python3
"""Windows-side launch script for AirSim -> WSL -> FlightCore episode.

This script launches the AirSim endpoint (mock or real) and coordinates
with the WSL-side launch script.

Usage:
  # Mock mode (no AirSim needed):
  python run_windows_endpoint.py --mock --duration 30

  # Real AirSim mode:
  python run_windows_endpoint.py --airsim --duration 30

The script will output an episode directory path that can be passed to
the WSL evaluation script.
"""

import argparse
import logging
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser(description="Windows AirSim endpoint launcher")
    parser.add_argument("--mock", action="store_true", help="Use mock state (no AirSim)")
    parser.add_argument("--airsim", action="store_true", help="Use real AirSim")
    parser.add_argument("--duration", type=float, default=30.0, help="Episode duration (sec)")
    parser.add_argument("--rate-hz", type=float, default=50.0, help="State/IMU rate (Hz)")
    parser.add_argument("--gps-rate-hz", type=float, default=5.0, help="GPS rate (Hz)")
    parser.add_argument("--bridge-host", default="127.0.0.1", help="WSL2 bridge IP")
    parser.add_argument("--state-port", type=int, default=56000)
    parser.add_argument("--control-port", type=int, default=56001)
    parser.add_argument("--motor-order", default="0,1,2,3", help="Endpoint motor API argument order")
    parser.add_argument("--episode-dir", default=None, help="Episode output directory")
    parser.add_argument("--local-origin-lat", type=float, default=47.641468)
    parser.add_argument("--local-origin-lon", type=float, default=-122.140165)
    parser.add_argument("--local-origin-alt", type=float, default=122.0)
    parser.add_argument("--airsim-host", default="127.0.0.1")
    parser.add_argument("--airsim-rpc-port", type=int, default=41451)
    parser.add_argument("--vehicle-name", default="")
    parser.add_argument("--log-level", default="INFO")
    args = parser.parse_args()

    if not args.mock and not args.airsim:
        parser.error("Specify --mock or --airsim")

    # Episode directory
    if args.episode_dir:
        episode_dir = Path(args.episode_dir)
    else:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        episode_dir = Path(__file__).resolve().parents[1] / "episodes" / f"{timestamp}_airsim_hover_v0"

    episode_dir.mkdir(parents=True, exist_ok=True)
    endpoint_log = episode_dir / "endpoint.log"

    # Build endpoint command
    endpoint_py = Path(__file__).resolve().parent.parent / "bridge" / "airsim_ros2_udp_bridge" / "windows" / "airsim_udp_endpoint.py"
    cmd = [
        sys.executable, str(endpoint_py),
        "--bridge-host", args.bridge_host,
        "--state-port", str(args.state_port),
        "--control-port", str(args.control_port),
        "--rate-hz", str(args.rate_hz),
        "--gps-rate-hz", str(args.gps_rate_hz),
        "--duration-sec", str(args.duration),
        "--local-origin-lat", str(args.local_origin_lat),
        "--local-origin-lon", str(args.local_origin_lon),
        "--local-origin-alt", str(args.local_origin_alt),
        "--motor-order", args.motor_order,
        "--airsim-host", args.airsim_host,
        "--airsim-rpc-port", str(args.airsim_rpc_port),
        "--vehicle-name", args.vehicle_name,
        "--log-period", "2.0",
    ]
    if args.mock:
        cmd.append("--mock")
    if args.airsim:
        cmd.append("--enable-api-control")
        cmd.append("--arm")

    logging.basicConfig(level=getattr(logging, args.log_level.upper(), logging.INFO))

    print("=" * 60)
    print(f"Episode: {episode_dir}")
    print(f"Mode: {'MOCK' if args.mock else 'AIRSIM'}")
    print(f"Duration: {args.duration}s")
    print(f"Endpoint UDP: {args.bridge_host}:{args.state_port} (tx) / :{args.control_port} (rx)")
    print("=" * 60)
    print(f"Endpoint log: {endpoint_log}")
    print()

    try:
        with open(endpoint_log, "w") as log_f:
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
            )

            # Tee output to both console and log file
            for line in process.stdout:
                log_f.write(line)
                log_f.flush()
                print(line, end="", flush=True)

            process.wait()
            return process.returncode

    except KeyboardInterrupt:
        print("\nInterrupted by user")
        return 0
    except Exception as e:
        print(f"Error: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
