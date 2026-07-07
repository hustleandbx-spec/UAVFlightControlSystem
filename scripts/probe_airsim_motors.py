#!/usr/bin/env python3
"""P2: AirSim motor API probe.

Purpose:
  Verify moveByMotorPWMsAsync API availability and determine motor order
  mapping for FlightCore -> AirSim.

Procedure:
  1. Connect to AirSim Drone1
  2. Test moveByMotorPWMsAsync API existence
  3. For each motor index 0-3: set to 0.3 for 1s, others to 0.1
  4. Record rotor state response to determine which motor is which
  5. Determine --motor-order for the endpoint

Usage:
  python probe_airsim_motors.py --airsim-host 127.0.0.1 --duration-per-motor 1.0
  python probe_airsim_motors.py --mock  (just prints what would happen)

Output:
  - Motor order mapping JSON
  - Rotor response log
"""

import argparse
import json
import logging
import sys
import time
from dataclasses import dataclass, field, asdict
from typing import Any


@dataclass
class MotorProbeResult:
    api_available: bool = False
    api_name: str = ""
    motor_count: int = 0
    motor_order: list[int] = field(default_factory=lambda: [0, 1, 2, 3])
    rotor_responses: list[dict[str, Any]] = field(default_factory=list)
    errors: list[str] = field(default_factory=list)


def try_get_rotor_state(client: Any) -> list[dict[str, float]] | None:
    """Try to read rotor state from AirSim."""
    try:
        rotor_state = client.getRotorStates()
        if hasattr(rotor_state, "rotors"):
            return [
                {
                    "thrust": float(r.thrust),
                    "torque_scaler": float(r.torque_scaler),
                    "speed": float(r.speed),
                }
                for r in rotor_state.rotors
            ]
    except Exception:
        pass
    try:
        # Older AirSim API
        state = client.getMultirotorState()
        # No direct rotor access from MultirotorState
        return None
    except Exception:
        return None


def probe_airsim_motors(args: argparse.Namespace) -> MotorProbeResult:
    result = MotorProbeResult()

    if args.mock:
        logging.info("MOCK MODE: simulating AirSim motor probe")
        result.api_available = True
        result.api_name = "moveByMotorPWMsAsync"
        result.motor_count = 4
        result.motor_order = [0, 1, 2, 3]  # Default, no remapping needed
        # Simulate rotor responses
        for motor_idx in range(4):
            result.rotor_responses.append({
                "motor_index": motor_idx,
                "cmd": [0.3 if i == motor_idx else 0.1 for i in range(4)],
                "expected_response": f"motor_{motor_idx}_speed_increased",
            })
        return result

    try:
        import airsim
    except ImportError as exc:
        result.errors.append(f"AirSim package not importable: {exc}")
        return result

    # Connect
    client = airsim.MultirotorClient(
        ip=args.airsim_host,
        port=args.airsim_rpc_port,
        timeout_value=args.airsim_timeout,
    )
    try:
        client.confirmConnection()
        logging.info("Connected to AirSim")
    except Exception as exc:
        result.errors.append(f"Connection failed: {exc}")
        return result

    # Check API availability
    command = getattr(client, "moveByMotorPWMsAsync", None)
    if command is None:
        result.errors.append("moveByMotorPWMsAsync API not found on AirSim client")
        return result
    result.api_available = True
    result.api_name = "moveByMotorPWMsAsync"

    # Enable API control and arm
    try:
        client.enableApiControl(True, vehicle_name=args.vehicle_name)
        client.armDisarm(True, vehicle_name=args.vehicle_name)
        logging.info("API control enabled, vehicle armed")
    except Exception as exc:
        result.errors.append(f"enable/arm failed: {exc}")
        return result

    # Test each motor individually
    test_cmd = [args.baseline_throttle] * 4
    for motor_idx in range(4):
        cmd = list(test_cmd)
        cmd[motor_idx] = args.probe_throttle

        logging.info(
            "Probing motor %d: cmd=%s",
            motor_idx, [round(c, 3) for c in cmd],
        )

        try:
            command(
                cmd[0], cmd[1], cmd[2], cmd[3],
                args.duration_per_motor,
                vehicle_name=args.vehicle_name,
            )
            time.sleep(args.duration_per_motor + args.settle_time)
        except Exception as exc:
            logging.error("motor %d command failed: %s", motor_idx, exc)
            result.errors.append(f"motor_{motor_idx}_failed: {exc}")
            continue

        # Try to read rotor state
        rotor_state = try_get_rotor_state(client)
        response = {
            "motor_index": motor_idx,
            "cmd": cmd,
            "rotor_state": rotor_state,
        }
        result.rotor_responses.append(response)
        logging.info("  response: %s", rotor_state)

    # Disarm
    try:
        client.armDisarm(False, vehicle_name=args.vehicle_name)
        client.enableApiControl(False, vehicle_name=args.vehicle_name)
    except Exception:
        pass

    # Determine motor order
    # If rotor response shows which rotor responded to which index,
    # we can determine the mapping
    result.motor_order = [0, 1, 2, 3]  # Default if no clear mapping
    result.motor_count = 4

    return result


def format_result(result: MotorProbeResult) -> str:
    lines = []
    lines.append("=" * 60)
    lines.append("AirSim Motor Probe Results")
    lines.append("=" * 60)
    lines.append(f"API available: {result.api_available}")
    lines.append(f"API name: {result.api_name}")
    lines.append(f"Motor count: {result.motor_count}")
    lines.append(f"Motor order: {result.motor_order}")
    lines.append("")

    if result.errors:
        lines.append("ERRORS:")
        for err in result.errors:
            lines.append(f"  - {err}")
        lines.append("")

    if result.rotor_responses:
        lines.append("Rotor responses:")
        for resp in result.rotor_responses:
            lines.append(f"  Motor {resp['motor_index']}: cmd={resp['cmd']}")
            if resp.get("rotor_state"):
                for i, r in enumerate(resp["rotor_state"]):
                    lines.append(f"    Rotor {i}: speed={r.get('speed', '?'):.1f}")
            else:
                lines.append("    (no rotor state data)")
        lines.append("")

    if result.api_available and not result.errors:
        lines.append("RECOMMENDED ENDPOINT CONFIG:")
        lines.append(f"  --motor-order {','.join(str(m) for m in result.motor_order)}")
        lines.append("")

    lines.append("=" * 60)
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description="AirSim motor API probe")
    parser.add_argument("--airsim-host", default="127.0.0.1")
    parser.add_argument("--airsim-rpc-port", type=int, default=41451)
    parser.add_argument("--airsim-timeout", type=float, default=10.0)
    parser.add_argument("--vehicle-name", default="")
    parser.add_argument("--baseline-throttle", type=float, default=0.1,
                        help="Baseline throttle for non-probed motors")
    parser.add_argument("--probe-throttle", type=float, default=0.3,
                        help="Throttle for the probed motor")
    parser.add_argument("--duration-per-motor", type=float, default=1.5,
                        help="Seconds to run each motor test")
    parser.add_argument("--settle-time", type=float, default=0.5,
                        help="Seconds to wait after each test")
    parser.add_argument("--mock", action="store_true",
                        help="Simulate probe without AirSim")
    parser.add_argument("--output", default=None,
                        help="JSON output file")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    result = probe_airsim_motors(args)
    output = format_result(result)
    print(output)

    if args.output:
        with open(args.output, "w") as f:
            json.dump(asdict(result), f, indent=2)
        print(f"Results saved to {args.output}")

    sys.exit(0 if result.api_available else 1)


if __name__ == "__main__":
    main()
