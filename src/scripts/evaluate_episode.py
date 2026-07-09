#!/usr/bin/env python3
"""Evaluate a hover episode against PASS/FAIL criteria.

Reads episode directory and produces metrics.json with verdict.

Usage:
  python3 evaluate_episode.py --episode-dir episodes/20260707_120000_airsim_hover_v0

PASS/FAIL criteria (from DEVELOPMENT_PLAN.md):
  - Runnable: episode >= 30s, no unrecoverable crash
  - Recordable: rosbag2, manifest, endpoint log, clock_offsets all exist
  - Replayable: ros2 bag play can replay recorded topics
  - Comparable: truth-vs-estimate plots generated
  - Reproducible: manifest records git commits and runtime params
  - Performance: divergence not sustained
"""

import argparse
import json
import math
import subprocess
import sys
from pathlib import Path


def criteria_pass_fail(episode_dir: Path) -> dict:
    """Evaluate all PASS/FAIL criteria for an episode."""
    manifest_path = episode_dir / "manifest.yaml"
    bag_dir = episode_dir / "rosbag2"
    endpoint_log = episode_dir / "endpoint.log"
    matlab_log = episode_dir / "matlab.log"
    clock_offsets = episode_dir / "clock_offsets.csv"
    plots_dir = episode_dir / "plots"

    results = {}

    # C1: Runnable - episode at least 30s
    duration_pass = False
    duration_sec = 0.0
    if manifest_path.exists():
        import yaml
        with open(manifest_path) as f:
            manifest = yaml.safe_load(f)
        duration_sec = manifest.get("criteria", {}).get("duration_sec", 0)
        duration_pass = duration_sec >= 30.0
    results["c1_runnable_duration_sec"] = {
        "pass": duration_pass,
        "value": duration_sec,
        "detail": "Episode duration >= 30s" if duration_pass else f"Duration {duration_sec}s < 30s",
    }

    # Check for crashes in logs
    crashes = 0
    for log_path in [endpoint_log, matlab_log]:
        if log_path.exists():
            content = log_path.read_text()
            crash_keywords = ["traceback", "Traceback", "SEGV", "segfault", "failed", "FATAL"]
            crashes += sum(1 for kw in crash_keywords if kw in content)
    results["c1_runnable_no_crash"] = {
        "pass": crashes == 0,
        "value": crashes,
        "detail": f"{crashes} crash keywords found" if crashes > 0 else "No crashes detected",
    }

    # C2: Recordable - all artifacts exist
    artifacts = {
        "rosbag2": any(bag_dir.rglob("*.bag*")),
        "manifest": manifest_path.exists(),
        "endpoint_log": endpoint_log.exists(),
        "clock_offsets": clock_offsets.exists(),
    }
    all_recorded = all(artifacts.values())
    results["c2_recordable"] = {
        "pass": all_recorded,
        "value": artifacts,
        "detail": "All artifacts present" if all_recorded else f"Missing: {[k for k, v in artifacts.items() if not v]}",
    }

    # C3: Replayable - check rosbag2 has files
    bag_files = list(bag_dir.rglob("*.bag*")) if bag_dir.exists() else []
    replayable = len(bag_files) > 0
    results["c3_replayable_bag_exists"] = {
        "pass": replayable,
        "value": len(bag_files),
        "detail": f"{len(bag_files)} bag file(s) found" if replayable else "No bag files",
    }

    # C4: Comparable - plots generated
    plot_files = list(plots_dir.glob("*.png")) + list(plots_dir.glob("*.pdf")) if plots_dir.exists() else []
    comparable = len(plot_files) >= 2
    results["c4_comparable_plots"] = {
        "pass": comparable,
        "value": len(plot_files),
        "detail": f"{len(plot_files)} plot(s) generated" if comparable else "< 2 plots",
    }

    # C5: Reproducible - manifest has git commits
    git_doc = False
    if manifest_path.exists():
        import yaml
        with open(manifest_path) as f:
            manifest = yaml.safe_load(f)
        git = manifest.get("project_git", {})
        git_doc = bool(git.get("windows_repo") and git.get("windows_repo") != "unknown")
    results["c5_reproducible_git"] = {
        "pass": git_doc,
        "value": bool(git_doc),
        "detail": "Git commits recorded" if git_doc else "Git commits missing or unknown",
    }

    # C6: Performance - check clock offsets for stability
    offset_std = None
    if clock_offsets.exists():
        try:
            import csv
            with open(clock_offsets) as f:
                reader = csv.DictReader(f)
                offsets = []
                for row in reader:
                    wall = float(row.get("wall_clock_sec", 0))
                    ros = float(row.get("ros_time_sec", 0))
                    offsets.append(ros - wall)
                if offsets:
                    mean = sum(offsets) / len(offsets)
                    variance = sum((o - mean) ** 2 for o in offsets) / len(offsets)
                    offset_std = math.sqrt(variance)
        except Exception:
            pass
    results["c6_performance_clock_jitter"] = {
        "pass": offset_std is not None and offset_std < 1.0,
        "value": offset_std,
        "detail": f"Clock offset std={offset_std:.4f}s" if offset_std is not None else "No clock data",
    }

    return results


def main() -> None:
    parser = argparse.ArgumentParser(description="Evaluate hover episode")
    parser.add_argument("--episode-dir", required=True, help="Episode directory")
    parser.add_argument("--output", default=None, help="Output path (default: episode_dir/metrics.json)")
    args = parser.parse_args()

    episode_dir = Path(args.episode_dir)
    if not episode_dir.exists():
        print(f"ERROR: episode directory not found: {episode_dir}")
        sys.exit(1)

    results = criteria_pass_fail(episode_dir)
    output_path = Path(args.output) if args.output else episode_dir / "metrics.json"

    with open(output_path, "w") as f:
        json.dump(results, f, indent=2)

    # Print summary
    all_pass = all(r.get("pass", False) for r in results.values())
    passed = sum(1 for r in results.values() if r.get("pass", False))
    total = len(results)

    print("=" * 60)
    print(f"Episode: {episode_dir.name}")
    print(f"Verdict: {'PASS' if all_pass else 'FAIL'} ({passed}/{total} criteria met)")
    print("=" * 60)
    for key, result in results.items():
        status = "PASS" if result.get("pass") else "FAIL"
        print(f"  [{status}] {key}: {result.get('detail', '')}")
    print("=" * 60)
    print(f"Results saved to {output_path}")

    sys.exit(0 if all_pass else 1)


if __name__ == "__main__":
    main()
