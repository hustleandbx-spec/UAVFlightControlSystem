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
import csv
import json
import math
import sys
from pathlib import Path


def _load_yaml(path: Path) -> dict:
    if not path.exists():
        return {}
    import yaml
    with open(path, encoding="utf-8") as f:
        loaded = yaml.safe_load(f)
    return loaded if isinstance(loaded, dict) else {}


def _load_json(path: Path) -> dict:
    if not path.exists():
        return {}
    with open(path, encoding="utf-8-sig") as f:
        loaded = json.load(f)
    return loaded if isinstance(loaded, dict) else {}


def _criterion(passed: bool, value, detail: str, *, required: bool = True) -> dict:
    return {
        "pass": bool(passed),
        "required": bool(required),
        "value": value,
        "detail": detail,
    }


def _bag_files(bag_dir: Path) -> list[Path]:
    if not bag_dir.exists():
        return []
    patterns = ["metadata.yaml", "*.db3", "*.mcap", "*.bag", "*.bag2"]
    out: list[Path] = []
    for pattern in patterns:
        out.extend(bag_dir.rglob(pattern))
    return sorted(set(out))


def _contains_any(path: Path, needles: list[str]) -> bool:
    if not path.exists():
        return False
    content = path.read_text(encoding="utf-8", errors="ignore")
    return any(item in content for item in needles)


def criteria_pass_fail(episode_dir: Path) -> dict:
    """Evaluate all PASS/FAIL criteria for an episode."""
    manifest_path = episode_dir / "manifest.yaml"
    run_summary_path = episode_dir / "run_summary.json"
    bag_dir = episode_dir / "rosbag2"
    endpoint_log = episode_dir / "endpoint.log"
    matlab_log = episode_dir / "matlab.log"
    clock_offsets = episode_dir / "clock_offsets.csv"
    plots_dir = episode_dir / "plots"

    results = {}
    manifest = _load_yaml(manifest_path)
    run_summary = _load_json(run_summary_path)
    mode = str(run_summary.get("mode") or manifest.get("runtime", {}).get("mode") or "unknown")
    strict_episode = mode == "airsim"
    wsl_enabled = not bool(run_summary.get("skip_wsl", False))
    airsim_runtime = run_summary.get("airsim", {}) if isinstance(run_summary.get("airsim", {}), dict) else {}
    wsl_runtime = run_summary.get("wsl", {}) if isinstance(run_summary.get("wsl", {}), dict) else {}
    wsl_artifacts = wsl_runtime.get("artifacts", {}) if isinstance(wsl_runtime.get("artifacts", {}), dict) else {}

    if strict_episode:
        airsim_rpc_ready = bool(airsim_runtime.get("rpc_ready", False))
        results["c0_airsim_rpc_ready"] = _criterion(
            airsim_rpc_ready,
            {
                "project": airsim_runtime.get("project", ""),
                "rpc_host": airsim_runtime.get("rpc_host", ""),
                "rpc_port": airsim_runtime.get("rpc_port", ""),
                "started_by_script": airsim_runtime.get("started_by_script", False),
            },
            "AirSim RPC became ready before endpoint launch" if airsim_rpc_ready else "AirSim RPC readiness was not proven",
        )

    # C1: Runnable - episode at least 30s
    requested_duration = float(manifest.get("criteria", {}).get("duration_sec", 0.0) or 0.0)
    actual_duration = float(run_summary.get("actual_duration_sec", 0.0) or 0.0)
    min_duration = min(30.0, requested_duration) if mode == "mock" else 30.0
    duration_pass = actual_duration >= min_duration
    results["c1_runnable_duration_sec"] = _criterion(
        duration_pass,
        actual_duration,
        f"Actual duration {actual_duration:.2f}s >= {min_duration:.2f}s"
        if duration_pass else f"Actual duration {actual_duration:.2f}s < {min_duration:.2f}s",
    )

    # Check for crashes in logs
    crashes = 0
    for log_path in [endpoint_log, matlab_log]:
        if log_path.exists():
            content = log_path.read_text(encoding="utf-8", errors="ignore")
            crash_keywords = ["Traceback", "SEGV", "segfault", "FATAL", "RuntimeError"]
            crashes += sum(1 for kw in crash_keywords if kw in content)
    results["c1_runnable_no_crash"] = _criterion(
        crashes == 0,
        crashes,
        f"{crashes} crash keywords found" if crashes > 0 else "No crashes detected",
    )

    return_codes = run_summary.get("return_codes", {})
    if isinstance(return_codes, dict) and return_codes:
        bad = {k: v for k, v in return_codes.items() if v not in (0, None)}
        results["c1_process_return_codes"] = _criterion(
            not bad,
            return_codes,
            "All launched processes exited successfully" if not bad else f"Non-zero return codes: {bad}",
        )

    # C2: Recordable - all artifacts exist
    bag_files = _bag_files(bag_dir)
    wsl_bag_count = int(wsl_artifacts.get("rosbag_files", 0) or 0)
    local_or_wsl_bag_count = len(bag_files) if bag_files else wsl_bag_count
    clock_offsets_present = clock_offsets.exists() or bool(wsl_artifacts.get("clock_offsets_exists", False))

    artifacts = {
        "rosbag2": local_or_wsl_bag_count > 0 if wsl_enabled else True,
        "manifest": manifest_path.exists(),
        "endpoint_log": endpoint_log.exists(),
        "clock_offsets": clock_offsets_present,
        "run_summary": run_summary_path.exists(),
    }
    all_recorded = all(artifacts.values())
    results["c2_recordable"] = _criterion(
        all_recorded,
        artifacts,
        "All required artifacts present" if all_recorded else f"Missing: {[k for k, v in artifacts.items() if not v]}",
    )

    # C3: Replayable - local static evidence only. Full `ros2 bag play` remains manual/WSL-side.
    replayable = local_or_wsl_bag_count > 0
    results["c3_replayable_bag_exists"] = _criterion(
        replayable or not wsl_enabled,
        local_or_wsl_bag_count,
        f"{local_or_wsl_bag_count} rosbag metadata/storage file(s) found"
        if replayable else "No rosbag metadata/storage files",
        required=wsl_enabled,
    )

    # C4: Comparable - plots generated
    plot_files = list(plots_dir.glob("*.png")) + list(plots_dir.glob("*.pdf")) if plots_dir.exists() else []
    comparable = len(plot_files) >= 2
    results["c4_comparable_plots"] = _criterion(
        comparable,
        len(plot_files),
        f"{len(plot_files)} plot(s) generated" if comparable else "< 2 plots",
        required=strict_episode,
    )

    # C5: Reproducible - manifest has git commits
    git_doc = False
    if manifest:
        git = manifest.get("project_git", {})
        git_doc = bool(
            git.get("windows_repo") and git.get("windows_repo") != "unknown"
            and git.get("wsl_repo") and git.get("wsl_repo") != "unknown"
        )
    results["c5_reproducible_git"] = _criterion(
        git_doc,
        git_doc,
        "Windows and WSL git commits recorded" if git_doc else "Windows or WSL git commit missing/unknown",
    )

    actuator_seen = _contains_any(endpoint_log, ["mock actuator mode=", "airsim actuator mode="])
    results["c6_actuator_feedback"] = _criterion(
        actuator_seen,
        actuator_seen,
        "Endpoint log contains actuator feedback" if actuator_seen else "No actuator feedback in endpoint.log",
    )

    # C7: Clock offsets - check ROS/wall offset stability when data exists.
    offset_std = None
    if clock_offsets.exists():
        try:
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
    elif wsl_artifacts.get("clock_offset_std") is not None:
        try:
            offset_std = float(wsl_artifacts["clock_offset_std"])
        except (TypeError, ValueError):
            offset_std = None
    results["c7_clock_jitter"] = _criterion(
        offset_std is not None and offset_std < 1.0,
        offset_std,
        f"Clock offset std={offset_std:.4f}s" if offset_std is not None else "No clock data",
    )

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
    required_results = [r for r in results.values() if r.get("required", True)]
    all_pass = all(r.get("pass", False) for r in required_results)
    passed = sum(1 for r in required_results if r.get("pass", False))
    total = len(required_results)

    print("=" * 60)
    print(f"Episode: {episode_dir.name}")
    print(f"Verdict: {'PASS' if all_pass else 'FAIL'} ({passed}/{total} criteria met)")
    print("=" * 60)
    for key, result in results.items():
        if not result.get("required", True) and not result.get("pass"):
            status = "SKIP"
        else:
            status = "PASS" if result.get("pass") else "FAIL"
        print(f"  [{status}] {key}: {result.get('detail', '')}")
    print("=" * 60)
    print(f"Results saved to {output_path}")

    sys.exit(0 if all_pass else 1)


if __name__ == "__main__":
    main()
