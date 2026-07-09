from __future__ import annotations

from dataclasses import dataclass
from typing import Any


AIRCRAFT_OBSERVATION = "aircraft_observation"
UAV_CONTRACT = "uav_contract"
CONTRACT_CANDIDATE = "contract_candidate"
TRACE = "trace"
SCENARIO = "scenario"
UNSUPPORTED = "unsupported"

FORBIDDEN_UAV_CATEGORIES = {
    "truth",
    "environment",
    "actuator_feedback",
    "event",
    "visual",
    "unknown",
}

CURRENT_UAV_CONTRACT_TOPICS = {
    "/uav/sensors/imu",
    "/uav/sensors/gps",
    "/uav/cmd/flight",
    "/uav/actuator/esc_cmd",
    "/uav/estimator/state",
    "/uav/health/status",
}


class ObservationRoutingError(ValueError):
    """Raised when an observation registry entry violates routing rules."""


@dataclass(frozen=True)
class ObservationRoute:
    outputs: tuple[str, ...]
    aircraft_topic: str | None = None
    uav_topic: str | None = None


def route_observation_entry(entry: dict[str, Any]) -> ObservationRoute:
    """Classify one registry entry into adapter outputs.

    The router is intentionally conservative: only entries marked
    ``contract_status: current`` may route to a current /uav topic, and truth /
    environment / actuator feedback / event / visual data are blocked from the
    FlightCore contract regardless of topic naming.
    """

    validate_observation_entry(entry)

    adapter_target = str(entry["adapter_target"])
    category = str(entry["category"])
    contract_status = str(entry.get("contract_status", "unsupported"))

    if adapter_target == UNSUPPORTED or contract_status == UNSUPPORTED:
        return ObservationRoute(outputs=(UNSUPPORTED,))

    outputs: list[str] = []
    aircraft_topic = adapter_target if adapter_target.startswith("/aircraft/") else None
    uav_topic = entry.get("current_uav_topic")

    if aircraft_topic:
        outputs.append(AIRCRAFT_OBSERVATION)

    if category == "truth" or adapter_target == "trace-only" or contract_status == "trace_only":
        outputs.append(TRACE)
    elif category in {"visual", "actuator_feedback"}:
        outputs.append(TRACE)

    if category in {"environment", "event"}:
        outputs.append(SCENARIO)
        if category == "event":
            outputs.append(TRACE)

    if contract_status == "current":
        outputs.append(UAV_CONTRACT)
    elif bool(entry.get("may_enter_uav_contract")) and category == "sensor":
        outputs.append(CONTRACT_CANDIDATE)

    return ObservationRoute(outputs=tuple(dict.fromkeys(outputs)), aircraft_topic=aircraft_topic, uav_topic=uav_topic)


def validate_observation_entry(entry: dict[str, Any]) -> None:
    adapter_target = str(entry.get("adapter_target", ""))
    category = str(entry.get("category", ""))
    contract_status = str(entry.get("contract_status", "unsupported"))
    may_enter_uav = bool(entry.get("may_enter_uav_contract", False))
    current_uav_topic = entry.get("current_uav_topic")

    if contract_status == "current":
        if category in FORBIDDEN_UAV_CATEGORIES:
            raise ObservationRoutingError(
                f"{entry.get('id', '<unknown>')} cannot route {category} to /uav"
            )
        if not current_uav_topic:
            raise ObservationRoutingError("current contract entries require current_uav_topic")
        if current_uav_topic not in CURRENT_UAV_CONTRACT_TOPICS:
            raise ObservationRoutingError(f"unknown current /uav topic: {current_uav_topic}")

    if category in FORBIDDEN_UAV_CATEGORIES and may_enter_uav:
        raise ObservationRoutingError(
            f"{entry.get('id', '<unknown>')} marks forbidden category as /uav candidate"
        )

    if adapter_target.startswith("/uav/"):
        raise ObservationRoutingError("registry adapter_target must not be a /uav topic")

    if adapter_target not in {UNSUPPORTED, "trace-only"} and not adapter_target.startswith("/aircraft/"):
        raise ObservationRoutingError(f"unsupported adapter_target: {adapter_target}")
