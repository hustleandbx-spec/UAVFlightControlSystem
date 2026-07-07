from __future__ import annotations

import argparse
import logging
import math
import socket
import sys
import threading
import time
from pathlib import Path
from typing import Any, Sequence

PACKAGE_SRC = Path(__file__).resolve().parents[1] / "ros2_ws" / "src" / "aircraft_udp_bridge"
sys.path.insert(0, str(PACKAGE_SRC))

from aircraft_udp_bridge.protocol import (  # noqa: E402
    ProtocolError,
    decode_datagram,
    encode_packet,
    make_state_packet,
    validate_command_packet,
)


def _vec3(vector: Any) -> list[float]:
    return [float(vector.x_val), float(vector.y_val), float(vector.z_val)]


def _quat(quaternion: Any) -> list[float]:
    return [
        float(quaternion.x_val),
        float(quaternion.y_val),
        float(quaternion.z_val),
        float(quaternion.w_val),
    ]


def _parse_motor_order(value: str) -> list[int]:
    try:
        order = [int(item.strip()) for item in value.split(",")]
    except ValueError as exc:
        raise argparse.ArgumentTypeError("motor order must contain integer indexes") from exc
    if sorted(order) != [0, 1, 2, 3]:
        raise argparse.ArgumentTypeError("motor order must be a permutation of 0,1,2,3")
    return order


class LatestControl:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._packet: dict[str, Any] | None = None
        self._sequence = -1

    def update(self, packet: dict[str, Any]) -> None:
        with self._lock:
            self._packet = packet

    def take_new(self) -> dict[str, Any] | None:
        with self._lock:
            packet = self._packet
        if packet is None:
            return None
        sequence = int(packet["sequence"])
        if sequence == self._sequence:
            return None
        self._sequence = sequence
        return packet


def control_rx_loop(
    *,
    bind_host: str,
    bind_port: int,
    latest_control: LatestControl,
    stop_event: threading.Event,
    stats: dict[str, int],
) -> None:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((bind_host, bind_port))
    sock.settimeout(0.2)
    logging.info("listening for control UDP on %s:%d", bind_host, bind_port)
    try:
        while not stop_event.is_set():
            try:
                data, addr = sock.recvfrom(8192)
            except socket.timeout:
                continue
            except OSError:
                break
            try:
                packet = validate_command_packet(decode_datagram(data))
            except ProtocolError as exc:
                stats["rx_errors"] += 1
                logging.warning("dropped command packet from %s: %s", addr, exc)
                continue
            latest_control.update(packet)
            stats["rx_control"] += 1
    finally:
        sock.close()


def connect_airsim(args: argparse.Namespace) -> Any:
    try:
        import airsim
    except ImportError as exc:
        raise RuntimeError(
            "AirSim Python package is not importable. Use --mock to test UDP only."
        ) from exc

    client = airsim.MultirotorClient(
        ip=args.airsim_host,
        port=args.airsim_rpc_port,
        timeout_value=args.airsim_timeout,
    )
    client.confirmConnection()
    if args.enable_api_control:
        client.enableApiControl(True, vehicle_name=args.vehicle_name)
    if args.arm:
        client.armDisarm(True, vehicle_name=args.vehicle_name)
    return client


def read_airsim_state(client: Any, vehicle_name: str, sequence: int) -> dict[str, Any]:
    state = client.getMultirotorState(vehicle_name=vehicle_name)
    kin = state.kinematics_estimated
    return make_state_packet(
        sequence=sequence,
        position=_vec3(kin.position),
        orientation=_quat(kin.orientation),
        linear_velocity=_vec3(kin.linear_velocity),
        angular_velocity=_vec3(kin.angular_velocity),
    )


def read_mock_state(sequence: int, start_time: float) -> dict[str, Any]:
    t = time.monotonic() - start_time
    return make_state_packet(
        sequence=sequence,
        position=[math.sin(0.2 * t), math.cos(0.2 * t), -2.0],
        orientation=[0.0, 0.0, 0.0, 1.0],
        linear_velocity=[0.2 * math.cos(0.2 * t), -0.2 * math.sin(0.2 * t), 0.0],
        angular_velocity=[0.0, 0.0, 0.0],
    )


def _ordered_motor_cmd(packet: dict[str, Any], motor_order: Sequence[int]) -> list[float]:
    motor_cmd = [float(item) for item in packet["motor_cmd"]]
    return [motor_cmd[index] for index in motor_order]


def apply_control(
    client: Any,
    packet: dict[str, Any],
    duration: float,
    vehicle_name: str,
    motor_order: Sequence[int],
) -> None:
    packet_type = str(packet["packet_type"])
    if packet_type == "actuator":
        motor_cmd = _ordered_motor_cmd(packet, motor_order)
        if client is None:
            logging.info(
                "mock actuator mode=%s motor_cmd=[%.3f, %.3f, %.3f, %.3f]",
                packet["mode"],
                motor_cmd[0],
                motor_cmd[1],
                motor_cmd[2],
                motor_cmd[3],
            )
            return
        command = getattr(client, "moveByMotorPWMsAsync", None)
        if command is None:
            logging.warning(
                "received motor actuator packet, but this client has no "
                "moveByMotorPWMsAsync API; command ignored"
            )
            return
        command(
            motor_cmd[0],
            motor_cmd[1],
            motor_cmd[2],
            motor_cmd[3],
            duration,
            vehicle_name=vehicle_name,
        )
        return

    throttle = max(0.0, min(1.0, float(packet["throttle"])))
    roll = float(packet["roll"])
    pitch = float(packet["pitch"])
    yaw = float(packet["yaw"])
    mode = str(packet["mode"])

    if client is None:
        logging.info(
            "mock control mode=%s throttle=%.3f roll=%.3f pitch=%.3f yaw=%.3f",
            mode,
            throttle,
            roll,
            pitch,
            yaw,
        )
        return

    if mode == "rate":
        command = getattr(client, "moveByAngleRatesThrottleAsync", None)
        if command is None:
            command = getattr(client, "moveByRollPitchYawrateThrottleAsync", None)
        if command is None:
            raise RuntimeError("AirSim client has no supported rate-control API")
        command(roll, pitch, yaw, throttle, duration, vehicle_name=vehicle_name)
    elif mode == "attitude":
        command = getattr(client, "moveByRollPitchYawThrottleAsync", None)
        if command is None:
            raise RuntimeError("AirSim client has no supported attitude-control API")
        command(roll, pitch, yaw, throttle, duration, vehicle_name=vehicle_name)
    else:
        raise RuntimeError(f"unsupported control mode {mode}")


def run(args: argparse.Namespace) -> None:
    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

    client = None if args.mock else connect_airsim(args)

    state_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    state_target = (args.bridge_host, args.state_port)
    latest_control = LatestControl()
    stop_event = threading.Event()
    stats = {"tx_state": 0, "rx_control": 0, "rx_errors": 0, "control_errors": 0}
    rx_thread = threading.Thread(
        target=control_rx_loop,
        kwargs={
            "bind_host": args.control_bind_host,
            "bind_port": args.control_port,
            "latest_control": latest_control,
            "stop_event": stop_event,
            "stats": stats,
        },
        daemon=True,
    )
    rx_thread.start()

    period = 1.0 / args.rate_hz
    sequence = 0
    start_time = time.monotonic()
    next_tick = start_time
    last_log_time = start_time

    logging.info("sending state UDP to %s:%d at %.1f Hz", *state_target, args.rate_hz)
    try:
        while True:
            now = time.monotonic()
            if args.duration_sec > 0.0 and now - start_time >= args.duration_sec:
                logging.info("duration reached, stopping")
                break
            if now < next_tick:
                time.sleep(min(next_tick - now, 0.002))
                continue
            next_tick += period
            sequence += 1

            packet = (
                read_mock_state(sequence, start_time)
                if args.mock
                else read_airsim_state(client, args.vehicle_name, sequence)
            )
            state_sock.sendto(encode_packet(packet), state_target)
            stats["tx_state"] += 1

            control = latest_control.take_new()
            if control is not None:
                try:
                    apply_control(
                        client,
                        control,
                        args.control_duration,
                        args.vehicle_name,
                        args.motor_order,
                    )
                except Exception as exc:  # AirSim RPC exceptions vary by version.
                    stats["control_errors"] += 1
                    logging.error("failed to apply control: %s", exc)

            if now - last_log_time >= args.log_period:
                elapsed = now - last_log_time
                logging.info(
                    "udp tx_state=%d (%.1f Hz), rx_control=%d, "
                    "rx_errors=%d, control_errors=%d",
                    stats["tx_state"],
                    stats["tx_state"] / elapsed,
                    stats["rx_control"],
                    stats["rx_errors"],
                    stats["control_errors"],
                )
                stats.update(tx_state=0, rx_control=0, rx_errors=0, control_errors=0)
                last_log_time = now
    except KeyboardInterrupt:
        logging.info("stopping")
    finally:
        stop_event.set()
        state_sock.close()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Windows AirSim UDP endpoint")
    parser.add_argument("--bridge-host", default="127.0.0.1", help="WSL2 bridge IP")
    parser.add_argument("--state-port", type=int, default=56000)
    parser.add_argument("--control-bind-host", default="0.0.0.0")
    parser.add_argument("--control-port", type=int, default=56001)
    parser.add_argument(
        "--motor-order",
        type=_parse_motor_order,
        default=[0, 1, 2, 3],
        help=(
            "comma-separated mapping from endpoint motor API argument order to "
            "FlightCore motor_cmd indexes"
        ),
    )
    parser.add_argument("--rate-hz", type=float, default=50.0)
    parser.add_argument("--vehicle-name", default="")
    parser.add_argument("--airsim-host", default="127.0.0.1")
    parser.add_argument("--airsim-rpc-port", type=int, default=41451)
    parser.add_argument("--airsim-timeout", type=float, default=10.0)
    parser.add_argument("--enable-api-control", action="store_true")
    parser.add_argument("--arm", action="store_true")
    parser.add_argument("--control-duration", type=float, default=0.05)
    parser.add_argument("--duration-sec", type=float, default=0.0)
    parser.add_argument("--log-period", type=float, default=1.0)
    parser.add_argument("--mock", action="store_true", help="send mock state without AirSim")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()
    if args.rate_hz <= 0.0:
        parser.error("--rate-hz must be positive")
    if args.control_duration <= 0.0:
        parser.error("--control-duration must be positive")
    if args.duration_sec < 0.0:
        parser.error("--duration-sec must be non-negative")
    return args


if __name__ == "__main__":
    run(parse_args())
