#!/usr/bin/env python3
"""Generate episode manifest.yaml for a hover episode.

Usage:
  python generate_manifest.py --episode-dir episodes/20260707_120000_airsim_hover_v0

Creates manifest.yaml inside the episode directory with git commits,
runtime metadata, and topology description.
"""

import argparse
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


def get_git_commit(repo_path: str | Path) -> str:
    """Get the current git commit hash."""
    try:
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            capture_output=True, text=True, cwd=repo_path, timeout=10
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except (subprocess.SubprocessError, FileNotFoundError):
        pass
    return "unknown"


def get_git_dirty(repo_path: str | Path) -> bool:
    """Check if the repo has uncommitted changes."""
    try:
        result = subprocess.run(
            ["git", "status", "--porcelain"],
            capture_output=True, text=True, cwd=repo_path, timeout=10
        )
        return bool(result.stdout.strip())
    except (subprocess.SubprocessError, FileNotFoundError):
        return True


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate episode manifest")
    parser.add_argument("--episode-dir", required=True, help="Episode output directory")
    parser.add_argument("--episode-id", default=None, help="Override episode ID")
    parser.add_argument("--windows-repo", default=".", help="Path to Windows git repo")
    parser.add_argument("--wsl-repo", default=None, help="Path to WSL git repo (e.g. /home/hustle/uavsingle_ros2_ws/src)")
    parser.add_argument("--wsl-commit", default=None, help="WSL git commit captured by the launcher")
    parser.add_argument("--simulator", default="AirSim")
    parser.add_argument("--vehicle", default="Drone1")
    parser.add_argument("--ros-distro", default="Jazzy")
    parser.add_argument("--matlab-version", default="R2025b")
    parser.add_argument("--mode", default="unknown", choices=["mock", "airsim", "unknown"])
    parser.add_argument("--duration-sec", type=float, default=30.0)
    parser.add_argument("--result", default="pending")
    args = parser.parse_args()

    episode_dir = Path(args.episode_dir)
    episode_dir.mkdir(parents=True, exist_ok=True)

    if args.episode_id:
        episode_id = args.episode_id
    else:
        now = datetime.now(timezone.utc)
        episode_id = now.strftime("%Y%m%d_%H%M%S") + "_airsim_hover_v0"

    windows_commit = get_git_commit(args.windows_repo)
    windows_dirty = get_git_dirty(args.windows_repo)

    wsl_commit = "unknown"
    if args.wsl_commit:
        wsl_commit = args.wsl_commit
    elif args.wsl_repo:
        wsl_commit = get_git_commit(args.wsl_repo)

    manifest = {
        "episode_id": episode_id,
        "creation_time": datetime.now(timezone.utc).isoformat(),
        "project_git": {
            "windows_repo": windows_commit,
            "windows_dirty": windows_dirty,
            "wsl_repo": wsl_commit,
        },
        "runtime": {
            "simulator": args.simulator,
            "vehicle": args.vehicle,
            "ros_distro": args.ros_distro,
            "matlab": args.matlab_version,
            "mode": args.mode,
        },
        "topology": {
            "endpoint_host": "Windows",
            "ros2_runtime_host": "WSL2",
            "flightcore_host": "Windows MATLAB",
            "dds_exception": "/uav/* development-only",
        },
        "topics": {
            "recorded": [
                "/aircraft/state",
                "/aircraft/imu",
                "/aircraft/gps",
                "/uav/sensors/imu",
                "/uav/sensors/gps",
                "/uav/actuator/esc_cmd",
                "/uav/estimator/state",
                "/uav/health/status",
            ]
        },
        "criteria": {
            "duration_sec": args.duration_sec,
            "result": args.result,
        },
    }

    manifest_path = episode_dir / "manifest.yaml"
    import yaml  # type: ignore
    with open(manifest_path, "w") as f:
        yaml.dump(manifest, f, default_flow_style=False, sort_keys=False)

    print(f"Manifest written to {manifest_path}")
    print(f"  episode_id: {episode_id}")
    print(f"  windows: {windows_commit}{' (dirty)' if windows_dirty else ''}")
    print(f"  wsl: {wsl_commit}")


if __name__ == "__main__":
    main()
