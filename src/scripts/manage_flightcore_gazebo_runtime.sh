#!/usr/bin/env bash
set -euo pipefail

action="${1:?action is required}"
pid_file="${2:-/tmp/flightcore_gazebo_runtime.pid}"
expected_marker="ros2 launch flightcore_gazebo_system flightcore_gazebo_cosim.launch.py"

read_owned_pgid() {
    [[ -s "${pid_file}" ]] || return 1
    local pgid
    pgid="$(tr -d '[:space:]' < "${pid_file}")"
    [[ "${pgid}" =~ ^[1-9][0-9]*$ ]] || {
        echo "Invalid FlightCore runtime ownership file: ${pid_file}" >&2
        return 2
    }
    printf '%s' "${pgid}"
}

group_processes() {
    local pgid="$1"
    ps -eo pgid=,args= |
        awk -v expected_pgid="${pgid}" '$1 == expected_pgid {$1=""; sub(/^ /, ""); print}'
}

verify_owned_group() {
    local pgid="$1"
    local processes
    processes="$(group_processes "${pgid}")"
    [[ -n "${processes}" ]] || return 1
    grep -Fq "${expected_marker}" <<< "${processes}" || {
        echo "PID group ${pgid} is not the owned FlightCore runtime; refusing cleanup." >&2
        return 2
    }
}

stop_owned_group() {
    local pgid="$1"
    verify_owned_group "${pgid}" || {
        local status=$?
        if [[ ${status} -eq 1 ]]; then
            rm -f "${pid_file}"
            return 0
        fi
        return "${status}"
    }

    kill -TERM -- "-${pgid}"
    for _ in {1..50}; do
        if [[ -z "$(group_processes "${pgid}")" ]]; then
            rm -f "${pid_file}"
            return 0
        fi
        sleep 0.1
    done
    kill -KILL -- "-${pgid}"
    rm -f "${pid_file}"
}

case "${action}" in
    cleanup|stop)
        if pgid="$(read_owned_pgid)"; then
            stop_owned_group "${pgid}"
        else
            status=$?
            if [[ ${status} -eq 1 ]]; then
                exit 0
            fi
            exit "${status}"
        fi
        ;;
    assert-clear)
        if pgid="$(read_owned_pgid)"; then
            if verify_owned_group "${pgid}"; then
                echo "Owned FlightCore runtime is still active: PGID ${pgid}" >&2
                exit 1
            fi
            status=$?
            [[ ${status} -eq 1 ]] || exit "${status}"
            rm -f "${pid_file}"
        else
            status=$?
            if [[ ${status} -ne 1 ]]; then
                exit "${status}"
            fi
        fi
        ;;
    wait-visual)
        deadline=$((SECONDS + 30))
        while (( SECONDS < deadline )); do
            if pgid="$(read_owned_pgid 2>/dev/null)" &&
                verify_owned_group "${pgid}"; then
                processes="$(group_processes "${pgid}")"
                if grep -Eq 'gz([[:space:]]+|.*/gz[[:space:]]+)sim.*(^|[[:space:]])-s([[:space:]]|$)' <<< "${processes}" &&
                    grep -Eq 'gz([[:space:]]+|.*/gz[[:space:]]+)sim.*(^|[[:space:]])-g([[:space:]]|$)' <<< "${processes}"; then
                    echo "GAZEBO_VISUAL_PROCESSES_READY pgid=${pgid}"
                    exit 0
                fi
            fi
            sleep 0.2
        done
        echo "Gazebo Server/GUI processes did not become ready." >&2
        exit 1
        ;;
    *)
        echo "Unsupported action: ${action}" >&2
        exit 2
        ;;
esac
