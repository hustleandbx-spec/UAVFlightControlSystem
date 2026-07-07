[CmdletBinding()]
param(
    [ValidateSet('mock', 'airsim')]
    [string]$Mode = 'airsim',
    [int]$DurationSec = 30,
    [string]$EpisodeDir = '',
    [string]$WslEpisodeDir = '',
    [string]$Distro = 'Ubuntu-24.04',
    [string]$BridgeHost = '',
    [string]$EndpointHost = '',
    [string]$MotorOrder = '0,1,2,3',
    [string]$VehicleName = 'Drone1',
    [string]$PythonExe = 'python',
    [string]$EndpointPythonExe = 'D:\Miniconda\envs\airsim\python.exe',
    [string]$MatlabExe = 'D:\MATLAB\R2025b\bin\matlab.exe',
    [string]$UE4EditorExe = 'D:\UE4\UE_4.27\Engine\Binaries\Win64\UE4Editor.exe',
    [string]$AirSimProjectDir = 'D:\AirsimScene\CityParkEnvironmentCollec',
    [string]$AirSimProject = '',
    [string]$AirSimMap = '/Game/CityPark/Maps/Overview?game=/Script/AirSim.AirSimGameMode',
    [string]$AirSimRpcHost = '127.0.0.1',
    [int]$AirSimRpcPort = 41451,
    [int]$AirSimStartupTimeoutSec = 180,
    [int]$WslLeadSec = 2,
    [int]$EndpointLeadSec = 3,
    [int]$MatlabLeadSec = 0,
    [switch]$SkipAirSim,
    [switch]$KeepAirSimOpen,
    [switch]$SkipMatlab,
    [switch]$SkipWsl,
    [switch]$SkipUnitTests,
    [switch]$NoEvaluate,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir '..')).Path

if (-not $EpisodeDir) {
    $stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
    $EpisodeDir = Join-Path $RepoRoot "episodes\$($stamp)_airsim_hover_v0"
}
$EpisodeDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EpisodeDir)
New-Item -ItemType Directory -Force -Path $EpisodeDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $EpisodeDir 'plots') | Out-Null

function Quote-ProcessArgument {
    param([AllowEmptyString()][string]$Argument)
    if ($null -eq $Argument -or $Argument.Length -eq 0) {
        return '""'
    }
    if ($Argument -match '[\s"]') {
        return '"' + ($Argument -replace '"', '\"') + '"'
    }
    return $Argument
}

function Join-ProcessArguments {
    param([string[]]$Arguments)
    return (($Arguments | ForEach-Object { Quote-ProcessArgument $_ }) -join ' ')
}

function Invoke-ProcessCapture {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$WorkingDirectory = $RepoRoot
    )
    if ($DryRun) {
        Write-Host "[DRY-RUN] $FilePath $(Join-ProcessArguments $Arguments)"
        return [pscustomobject]@{ ExitCode = 0; Stdout = ''; Stderr = '' }
    }

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $FilePath
    $psi.Arguments = Join-ProcessArguments $Arguments
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true

    $p = [System.Diagnostics.Process]::new()
    $p.StartInfo = $psi
    [void]$p.Start()
    $stdoutTask = $p.StandardOutput.ReadToEndAsync()
    $stderrTask = $p.StandardError.ReadToEndAsync()
    $p.WaitForExit()
    return [pscustomobject]@{
        ExitCode = $p.ExitCode
        Stdout = $stdoutTask.Result
        Stderr = $stderrTask.Result
    }
}

function Invoke-Logged {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$LogPath,
        [string]$WorkingDirectory = $RepoRoot
    )
    $cmdLine = "$FilePath $(Join-ProcessArguments $Arguments)"
    $result = Invoke-ProcessCapture -FilePath $FilePath -Arguments $Arguments -WorkingDirectory $WorkingDirectory
    $content = @(
        "COMMAND: $cmdLine",
        "EXIT_CODE: $($result.ExitCode)",
        "",
        "[stdout]",
        $result.Stdout,
        "",
        "[stderr]",
        $result.Stderr
    )
    Set-Content -Encoding UTF8 -Path $LogPath -Value $content
    return $result.ExitCode
}

function Start-LoggedProcess {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StdoutPath,
        [string]$StderrPath,
        [string]$WorkingDirectory = $RepoRoot
    )
    $cmdLine = "$FilePath $(Join-ProcessArguments $Arguments)"
    Write-Host "START $cmdLine"
    Set-Content -Encoding UTF8 -Path $StdoutPath -Value "COMMAND: $cmdLine"
    if ($DryRun) {
        return [pscustomobject]@{ Name = $FilePath; Process = $null; DryRun = $true; ExitCode = 0; StdoutPath = $StdoutPath; StderrPath = $StderrPath }
    }

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $FilePath
    $psi.Arguments = Join-ProcessArguments $Arguments
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true

    $p = [System.Diagnostics.Process]::new()
    $p.StartInfo = $psi
    [void]$p.Start()
    $stdoutTask = $p.StandardOutput.ReadToEndAsync()
    $stderrTask = $p.StandardError.ReadToEndAsync()
    return [pscustomobject]@{
        Name = $FilePath
        Process = $p
        DryRun = $false
        ExitCode = $null
        StdoutTask = $stdoutTask
        StderrTask = $stderrTask
        StdoutPath = $StdoutPath
        StderrPath = $StderrPath
    }
}

function Start-LoggedProcessWithInput {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StandardInput,
        [string]$StdoutPath,
        [string]$StderrPath,
        [string]$WorkingDirectory = $RepoRoot
    )
    $cmdLine = "$FilePath $(Join-ProcessArguments $Arguments)"
    Write-Host "START $cmdLine"
    Set-Content -Encoding UTF8 -Path $StdoutPath -Value "COMMAND: $cmdLine"
    if ($DryRun) {
        return [pscustomobject]@{ Name = $FilePath; Process = $null; DryRun = $true; ExitCode = 0; StdoutPath = $StdoutPath; StderrPath = $StderrPath }
    }

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $FilePath
    $psi.Arguments = Join-ProcessArguments $Arguments
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true

    $p = [System.Diagnostics.Process]::new()
    $p.StartInfo = $psi
    [void]$p.Start()
    $stdoutTask = $p.StandardOutput.ReadToEndAsync()
    $stderrTask = $p.StandardError.ReadToEndAsync()
    $p.StandardInput.Write($StandardInput)
    $p.StandardInput.Close()
    return [pscustomobject]@{
        Name = $FilePath
        Process = $p
        DryRun = $false
        ExitCode = $null
        StdoutTask = $stdoutTask
        StderrTask = $stderrTask
        StdoutPath = $StdoutPath
        StderrPath = $StderrPath
    }
}

function Wait-LoggedProcess {
    param(
        $Handle,
        [int]$TimeoutSec = 0
    )
    if ($Handle.DryRun) {
        return 0
    }

    $completed = $true
    if ($TimeoutSec -gt 0) {
        $completed = $Handle.Process.WaitForExit($TimeoutSec * 1000)
    } else {
        $Handle.Process.WaitForExit()
    }

    if (-not $completed) {
        Add-Content -Encoding UTF8 -Path $Handle.StderrPath -Value "TIMEOUT after ${TimeoutSec}s; terminating process."
        try {
            $Handle.Process.Kill()
        } catch {
        }
        $Handle.Process.WaitForExit()
    }

    [System.IO.File]::AppendAllText($Handle.StdoutPath, $Handle.StdoutTask.Result)
    [System.IO.File]::AppendAllText($Handle.StderrPath, $Handle.StderrTask.Result)

    if (-not $completed) {
        return -1
    }
    return $Handle.Process.ExitCode
}

function Stop-LoggedProcess {
    param($Handle)
    if ($null -eq $Handle -or $Handle.DryRun -or $null -eq $Handle.Process) {
        return
    }
    if (-not $Handle.Process.HasExited) {
        try {
            & taskkill.exe /PID $Handle.Process.Id /T /F | Out-Null
            $Handle.Process.WaitForExit(5000) | Out-Null
        } catch {
            try {
                $Handle.Process.Kill()
                $Handle.Process.WaitForExit(5000) | Out-Null
            } catch {
            }
        }
    }
}

function Invoke-WslText {
    param([string]$Command)
    $result = Invoke-ProcessCapture -FilePath 'wsl.exe' -Arguments @('-d', $Distro, '--', 'bash', '-lc', $Command)
    if ($result.ExitCode -ne 0) {
        throw "WSL command failed: $Command`n$($result.Stdout)`n$($result.Stderr)"
    }
    return (($result.Stdout -split "`r?`n" | Select-Object -First 1) -as [string]).Trim()
}

function Select-FirstIpv4 {
    param([string]$Text, [string]$Label)
    if ($Text -match '(\d{1,3}(?:\.\d{1,3}){3})') {
        return $Matches[1]
    }
    throw "Could not parse IPv4 address for ${Label}: $Text"
}

function Quote-BashString {
    param([AllowEmptyString()][string]$Value)
    return "'" + ($Value -replace "'", "'\''") + "'"
}

function New-WslHoverRuntimeScript {
    return @'
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

mkdir -p "${EPISODE_DIR}/plots"
echo "WSL episode dir: ${EPISODE_DIR}"
echo "Endpoint host for actuator UDP: ${ENDPOINT_HOST}"

set +u
source /opt/ros/jazzy/setup.bash
source "${WS_DIR}/install/setup.bash"
set -u

	# Use CycloneDDS for cross-boundary DDS with MATLAB (NAT mode).
	# MATLAB proprietary DDS interops with CycloneDDS via DDSI-RTPS wire protocol.
	export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
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

    kill "${BAG_PID:-}" 2>/dev/null || true
    kill "${CLOCK_PID:-}" 2>/dev/null || true
    kill "${ADAPTER_PID:-}" 2>/dev/null || true
    kill "${BRIDGE_PID:-}" 2>/dev/null || true
    wait 2>/dev/null || true
}
trap cleanup EXIT INT TERM

echo "Starting bridge..."
ros2 run aircraft_udp_bridge aircraft_udp_bridge "${BRIDGE_ARGS[@]}" >>"${BRIDGE_LOG}" 2>&1 &
BRIDGE_PID=$!

echo "Starting adapter..."
ros2 run flightcore_runtime_adapter flightcore_runtime_adapter "${ADAPTER_ARGS[@]}" >>"${ADAPTER_LOG}" 2>&1 &
ADAPTER_PID=$!

sleep 2

if [[ -e "${BAG_DIR}" ]]; then
    if [[ -d "${BAG_DIR}" ]] && [[ -z "$(find "${BAG_DIR}" -mindepth 1 -maxdepth 1 -print -quit)" ]]; then
        rmdir "${BAG_DIR}"
    else
        echo "ERROR: rosbag output already exists and is not empty: ${BAG_DIR}" >&2
        exit 3
    fi
fi

echo "Starting rosbag record..."
ros2 bag record \
    /aircraft/state /aircraft/imu /aircraft/gps \
    /uav/sensors/imu /uav/sensors/gps /uav/cmd/flight \
    /uav/actuator/esc_cmd /uav/estimator/state /uav/health/status \
    -o "${BAG_DIR}" >>"${ROSBAG_LOG}" 2>&1 &
BAG_PID=$!

echo "Starting EscCmd tracer..."
ros2 topic echo /uav/actuator/esc_cmd >>"${EPISODE_DIR}/esccmd_trace.log" 2>&1 &
	kill_PID=$!

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
PY
'@
}

function Get-WslArtifactsFromLog {
    param([string]$StdoutPath)
    if (-not (Test-Path -LiteralPath $StdoutPath)) {
        return $null
    }
    $line = Get-Content -LiteralPath $StdoutPath |
        Where-Object { $_ -like 'WSL_ARTIFACTS_JSON=*' } |
        Select-Object -Last 1
    if (-not $line) {
        return $null
    }
    $json = $line.Substring('WSL_ARTIFACTS_JSON='.Length)
    try {
        return $json | ConvertFrom-Json
    } catch {
        return $null
    }
}

function Resolve-AirSimProjectPath {
    if ($AirSimProject) {
        return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($AirSimProject)
    }
    if (-not (Test-Path -LiteralPath $AirSimProjectDir)) {
        throw "AirSim project directory not found: $AirSimProjectDir"
    }
    $projects = @(Get-ChildItem -LiteralPath $AirSimProjectDir -Filter '*.uproject' -File)
    if ($projects.Count -ne 1) {
        throw "Expected exactly one .uproject in $AirSimProjectDir, found $($projects.Count)"
    }
    return $projects[0].FullName
}

function Test-AirSimRpc {
    param(
        [string]$HostName,
        [int]$Port,
        [string]$LogPath
    )
    $probeCode = "import airsim; c=airsim.MultirotorClient(ip='$HostName', port=$Port, timeout_value=2); c.confirmConnection(); print('AIRSIM_RPC_READY')"
    $result = Invoke-ProcessCapture -FilePath $EndpointPythonExe -Arguments @('-c', $probeCode)
    Add-Content -Encoding UTF8 -Path $LogPath -Value @(
        "COMMAND: $EndpointPythonExe -c <airsim rpc probe>",
        "EXIT_CODE: $($result.ExitCode)",
        "[stdout]",
        $result.Stdout,
        "[stderr]",
        $result.Stderr,
        ""
    )
    return $result.ExitCode -eq 0
}

function Wait-AirSimRpc {
    param(
        [string]$HostName,
        [int]$Port,
        [int]$TimeoutSec,
        [string]$LogPath
    )
    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    while ((Get-Date) -lt $deadline) {
        if (Test-AirSimRpc -HostName $HostName -Port $Port -LogPath $LogPath) {
            return $true
        }
        Start-Sleep -Seconds 3
    }
    return $false
}

Write-Host "Episode: $EpisodeDir"
Write-Host "Mode:    $Mode"

if (-not $SkipUnitTests) {
    $unitLog = Join-Path $EpisodeDir 'windows_unit_tests.log'
    $unitCode = Invoke-Logged -FilePath $PythonExe `
        -Arguments @('-m', 'unittest', 'discover', 'bridge\airsim_ros2_udp_bridge\tests') `
        -LogPath $unitLog
    if ($unitCode -ne 0) {
        throw "Windows unit tests failed, see $unitLog"
    }
}

if ($Mode -eq 'airsim') {
    if (-not (Test-Path -LiteralPath $EndpointPythonExe)) {
        throw "Endpoint Python not found: $EndpointPythonExe"
    }
    $airsimImportLog = Join-Path $EpisodeDir 'airsim_python_import.log'
    $airsimImport = Invoke-Logged -FilePath $EndpointPythonExe `
        -Arguments @('-c', "import airsim; print('airsim import OK')") `
        -LogPath $airsimImportLog
    if ($airsimImport -ne 0) {
        throw "AirSim Python package is not importable, see $airsimImportLog"
    }
}

$wslCommit = 'unknown'
if (-not $SkipWsl) {
    $episodeName = Split-Path -Leaf $EpisodeDir
    $wslHome = Invoke-WslText 'printf "%s" "$HOME"'
    if (-not $WslEpisodeDir) {
        $WslEpisodeDir = "$wslHome/uavsingle_ros2_ws/episodes/$episodeName"
    }
    if (-not $BridgeHost) {
        $BridgeHost = Select-FirstIpv4 (Invoke-WslText 'hostname -I | awk ''{print $1; exit}''') 'BridgeHost'
    }
    if (-not $EndpointHost) {
        $EndpointHost = Select-FirstIpv4 (Invoke-WslText 'ip route show default | awk ''{print $3; exit}''') 'EndpointHost'
    }
    $wslCommit = Invoke-WslText "cd ~/uavsingle_ros2_ws/src && git rev-parse HEAD"
}

$manifestLog = Join-Path $EpisodeDir 'manifest.log'
$manifestArgs = @(
    (Join-Path $ScriptDir 'generate_manifest.py'),
    '--episode-dir', $EpisodeDir,
    '--windows-repo', $RepoRoot,
    '--wsl-commit', $wslCommit,
    '--mode', $Mode,
    '--duration-sec', $DurationSec.ToString(),
    '--result', 'pending'
)
$manifestCode = Invoke-Logged -FilePath $PythonExe -Arguments $manifestArgs -LogPath $manifestLog
if ($manifestCode -ne 0) {
    throw "Manifest generation failed, see $manifestLog"
}

$handles = @{}
$returnCodes = [ordered]@{}
$wslArtifacts = $null
$airSimProjectResolved = ''
$airSimStartedByScript = $false
$airSimRpcReady = $false
$failureMessage = ''
$startTime = Get-Date
$endTime = $startTime

try {
    if ($Mode -eq 'airsim') {
        $rpcProbeLog = Join-Path $EpisodeDir 'airsim_rpc_probe.log'
        if (-not $SkipAirSim) {
            if (-not (Test-Path -LiteralPath $UE4EditorExe)) {
                throw "UE4Editor executable not found: $UE4EditorExe"
            }
            $airSimProjectResolved = Resolve-AirSimProjectPath
            if (-not (Test-Path -LiteralPath $airSimProjectResolved)) {
                throw "AirSim project file not found: $airSimProjectResolved"
            }

            if (Test-AirSimRpc -HostName $AirSimRpcHost -Port $AirSimRpcPort -LogPath $rpcProbeLog) {
                Write-Host "AirSim RPC is already ready at ${AirSimRpcHost}:${AirSimRpcPort}"
                $airSimRpcReady = $true
            } else {
                $airSimArgs = @(
                    $airSimProjectResolved,
                    $AirSimMap,
                    '-game',
                    '-windowed',
                    '-ResX=1280',
                    '-ResY=720',
                    "-AbsLog=$(Join-Path $EpisodeDir 'airsim_ue.log')"
                )
                $handles['airsim'] = Start-LoggedProcess -FilePath $UE4EditorExe `
                    -Arguments $airSimArgs `
                    -StdoutPath (Join-Path $EpisodeDir 'airsim_launcher.stdout.log') `
                    -StderrPath (Join-Path $EpisodeDir 'airsim_launcher.stderr.log') `
                    -WorkingDirectory (Split-Path -Parent $airSimProjectResolved)
                $airSimStartedByScript = $true
            }
        }

        if (-not $airSimRpcReady) {
            Write-Host "Waiting for AirSim RPC ${AirSimRpcHost}:${AirSimRpcPort}..."
            $airSimRpcReady = Wait-AirSimRpc -HostName $AirSimRpcHost -Port $AirSimRpcPort -TimeoutSec $AirSimStartupTimeoutSec -LogPath $rpcProbeLog
        }
        if (-not $airSimRpcReady) {
            throw "AirSim RPC did not become ready within ${AirSimStartupTimeoutSec}s; see $rpcProbeLog"
        }
    }

    $matlabDurationSec = $DurationSec + $MatlabLeadSec + 10
    $wslDurationSec = $DurationSec + 20
    if (-not $SkipWsl) {
        $runtimeScript = New-WslHoverRuntimeScript
        $wslArgs = @(
            '-d', $Distro, '--', 'bash', '-s', '--',
            $WslEpisodeDir,
            $wslDurationSec.ToString(),
            $EndpointHost
        )
        $handles['wsl'] = Start-LoggedProcessWithInput -FilePath 'wsl.exe' `
            -Arguments $wslArgs `
            -StandardInput $runtimeScript `
            -StdoutPath (Join-Path $EpisodeDir 'wsl_runner.stdout.log') `
            -StderrPath (Join-Path $EpisodeDir 'wsl_runner.stderr.log')
        Start-Sleep -Seconds $WslLeadSec
    }

    $endpointDurationSec = $DurationSec + $EndpointLeadSec + 5
    $endpointArgs = @(
        (Join-Path $ScriptDir 'run_windows_endpoint.py'),
        "--$Mode",
        '--duration', $endpointDurationSec.ToString(),
        '--episode-dir', $EpisodeDir,
        '--bridge-host', ($(if ($BridgeHost) { $BridgeHost } else { '127.0.0.1' })),
        '--motor-order', $MotorOrder,
        '--airsim-host', $AirSimRpcHost,
        '--airsim-rpc-port', $AirSimRpcPort.ToString(),
        '--vehicle-name', $VehicleName
    )
    $handles['endpoint'] = Start-LoggedProcess -FilePath $EndpointPythonExe `
        -Arguments $endpointArgs `
        -StdoutPath (Join-Path $EpisodeDir 'endpoint_launcher.stdout.log') `
        -StderrPath (Join-Path $EpisodeDir 'endpoint_launcher.stderr.log')
    Start-Sleep -Seconds $EndpointLeadSec

    $matlabDurationSec = $DurationSec + 5
    if (-not $SkipMatlab) {
        if (-not (Test-Path -LiteralPath $MatlabExe)) {
            throw "MATLAB executable not found: $MatlabExe"
        }
        $matlabLog = Join-Path $EpisodeDir 'matlab.log'
        $fcDir = Join-Path $RepoRoot 'FC_SimulinkProject'
        $matlabScriptDir = $ScriptDir.Replace("'", "''")
        $matlabLogArg = $matlabLog.Replace("'", "''")
        $fcDirArg = $fcDir.Replace("'", "''")
        $matlabCommand = "cd('$fcDirArg'); addpath('$matlabScriptDir'); run_matlab_flightcore_ros2_episode($matlabDurationSec, '$matlabLogArg');"
        $handles['matlab'] = Start-LoggedProcess -FilePath $MatlabExe `
            -Arguments @('-wait', '-batch', $matlabCommand) `
            -StdoutPath (Join-Path $EpisodeDir 'matlab.stdout.log') `
            -StderrPath (Join-Path $EpisodeDir 'matlab.stderr.log') `
            -WorkingDirectory $fcDir
        if ($MatlabLeadSec -gt 0) {
            Start-Sleep -Seconds $MatlabLeadSec
        }
    }

    if ($handles.ContainsKey('matlab')) {
        $returnCodes['matlab'] = Wait-LoggedProcess $handles['matlab'] -TimeoutSec ([Math]::Max(180, $matlabDurationSec + 120))
    }
    if ($handles.ContainsKey('endpoint')) {
        $returnCodes['endpoint'] = Wait-LoggedProcess $handles['endpoint'] -TimeoutSec ([Math]::Max(60, $endpointDurationSec + 30))
    }
    if ($handles.ContainsKey('wsl')) {
        $returnCodes['wsl'] = Wait-LoggedProcess $handles['wsl'] -TimeoutSec ([Math]::Max(90, $wslDurationSec + 30))
        $wslArtifacts = Get-WslArtifactsFromLog (Join-Path $EpisodeDir 'wsl_runner.stdout.log')
    }
    $badReturnCodes = @()
    foreach ($item in $returnCodes.GetEnumerator()) {
        if ($null -ne $item.Value -and $item.Value -ne 0) {
            $badReturnCodes += "$($item.Key)=$($item.Value)"
        }
    }
    if ($badReturnCodes.Count -gt 0) {
        $failureMessage = "Non-zero process return codes: $($badReturnCodes -join ', ')"
    }
} catch {
    $failureMessage = $_.Exception.Message
} finally {
    $endTime = Get-Date
    if ($handles.ContainsKey('airsim') -and -not $KeepAirSimOpen) {
        Stop-LoggedProcess $handles['airsim']
    }
    foreach ($key in @('endpoint', 'wsl', 'matlab')) {
        if ($handles.ContainsKey($key)) {
            Stop-LoggedProcess $handles[$key]
        }
    }
}

$summary = [ordered]@{
    mode = $Mode
    episode_dir = $EpisodeDir
    start_time = $startTime.ToString('o')
    end_time = $endTime.ToString('o')
    actual_duration_sec = [Math]::Round(($endTime - $startTime).TotalSeconds, 3)
    requested_duration_sec = $DurationSec
    skip_airsim = [bool]$SkipAirSim
    skip_matlab = [bool]$SkipMatlab
    skip_wsl = [bool]$SkipWsl
    bridge_host = $BridgeHost
    endpoint_host = $EndpointHost
    motor_order = $MotorOrder
    vehicle_name = $VehicleName
    wsl_commit = $wslCommit
    wsl = @{
        episode_dir = $WslEpisodeDir
        artifacts = $wslArtifacts
    }
    return_codes = $returnCodes
    python = @{
        helper = $PythonExe
        endpoint = $EndpointPythonExe
    }
    airsim = @{
        project_dir = $AirSimProjectDir
        project = $airSimProjectResolved
        map = $AirSimMap
        ue4_editor = $UE4EditorExe
        rpc_host = $AirSimRpcHost
        rpc_port = $AirSimRpcPort
        rpc_ready = $airSimRpcReady
        started_by_script = $airSimStartedByScript
        keep_open = [bool]$KeepAirSimOpen
    }
    timing = @{
        matlab_lead_sec = $MatlabLeadSec
        endpoint_lead_sec = $EndpointLeadSec
        endpoint_duration_sec = $(if ($handles.ContainsKey('endpoint')) { $DurationSec + $EndpointLeadSec + 5 } else { 0 })
        matlab_duration_sec = $(if ($SkipMatlab) { 0 } else { $DurationSec + 5 })
        wsl_lead_sec = $WslLeadSec
        wsl_duration_sec = $(if ($SkipWsl) { 0 } else { $DurationSec + 20 })
    }
    failure = $failureMessage
}
$summary | ConvertTo-Json -Depth 8 | Set-Content -Encoding UTF8 -Path (Join-Path $EpisodeDir 'run_summary.json')

if ($failureMessage) {
    throw "Episode orchestration failed: $failureMessage"
}

if (-not $NoEvaluate) {
    $evalLog = Join-Path $EpisodeDir 'evaluate.log'
    $evalCode = Invoke-Logged -FilePath $PythonExe `
        -Arguments @((Join-Path $ScriptDir 'evaluate_episode.py'), '--episode-dir', $EpisodeDir) `
        -LogPath $evalLog
    Write-Host "Evaluation log: $evalLog"
    if ($evalCode -ne 0) {
        throw "Episode evaluation failed, see $evalLog"
    }
}

Write-Host "Episode test complete: $EpisodeDir"
