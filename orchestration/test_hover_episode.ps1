<#
.SYNOPSIS
  UAV 单机飞控闭环仿真一键编排脚本。
  协调 Windows (AirSim/UE4 + MATLAB/Simulink) 和 WSL2 (ROS2 bridge/adapter/PlotJuggler/rosbag2) 的并发启动与生命周期管理。

.DESCRIPTION
  所有静态配置集中在同目录下的 episode_config.json，换机器只改那一个文件。
  运行时自动检测 IP、自动生成 episode 目录，无需传参。

  架构数据流：
    AirSim(UE4) --RPC--> airsim_udp_endpoint.py --UDP--> WSL aircraft_udp_bridge
      --> ROS2 /aircraft/* --> flightcore_runtime_adapter --> ROS2 /uav/sensors/*
      --> MATLAB Simulink FlightCore_ROS2_loop --> ROS2 /uav/actuator/*, /uav/estimator/*
      --> PlotJuggler (实时曲线) + rosbag2 (录制)

  组件启动顺序：
    1. 单元测试（bridge protocol 校验）
    2. AirSim/UE4 启动 + RPC 就绪等待
    3. WSL runtime 脚本 (bridge + adapter + PlotJuggler + rosbag2)
    4. Windows endpoint (airsim_udp_endpoint.py)
    5. MATLAB Simulink (模型加载，人工点击 Run)
    6. 实时监控仪表盘

  跨 OS DDS 通信：见 docs/contracts/runtime_isolation.md
#>

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir '..')).Path

# ── 加载配置 ───────────────────────────────────────────────────────────
$configPath = Join-Path $ScriptDir 'episode_config.json'
if (-not (Test-Path $configPath)) {
    throw "Config file not found: $configPath"
}
$config = Get-Content -Raw -Path $configPath | ConvertFrom-Json

$EndpointPython = $config.paths.endpoint_python
$MatlabExe      = $config.paths.matlab
$UE4Editor      = $config.paths.ue4_editor
$AirSimProjDir  = $config.paths.airsim_project_dir
$Distro         = $config.wsl.distro
$AirSimMap      = $config.airsim.map
$AirSimRpcPort  = $config.airsim.rpc_port
$AirSimTimeout  = $config.airsim.startup_timeout_sec

# ── Episode 目录 ───────────────────────────────────────────────────────
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$EpisodeDir = Join-Path $RepoRoot "episodes\$($stamp)_airsim_hover_v0"
$EpisodeDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EpisodeDir)
New-Item -ItemType Directory -Force -Path $EpisodeDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $EpisodeDir 'plots') | Out-Null
Write-Host "Episode: $EpisodeDir"

# ── 辅助函数 ───────────────────────────────────────────────────────────

function Quote-ProcessArgument {
    param([AllowEmptyString()][string]$Argument)
    if ($null -eq $Argument -or $Argument.Length -eq 0) { return '""' }
    if ($Argument -match '[\s"]') { return '"' + ($Argument -replace '"', '\"') + '"' }
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
        Name       = $FilePath
        Process    = $p
        ExitCode   = $null
        StdoutTask = $stdoutTask
        StderrTask = $stderrTask
        StdoutPath = $StdoutPath
        StderrPath = $StderrPath
    }
}

function Stop-LoggedProcess {
    param($Handle)
    if ($null -eq $Handle -or $null -eq $Handle.Process) { return }
    if (-not $Handle.Process.HasExited) {
        try {
            & taskkill.exe /PID $Handle.Process.Id /T /F | Out-Null
            $Handle.Process.WaitForExit(5000) | Out-Null
        } catch {
            try {
                $Handle.Process.Kill()
                $Handle.Process.WaitForExit(5000) | Out-Null
            } catch {}
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

function Resolve-AirSimProjectPath {
    param([string]$ProjectDir)
    if (-not (Test-Path -LiteralPath $ProjectDir)) {
        throw "AirSim project directory not found: $ProjectDir"
    }
    $projects = @(Get-ChildItem -LiteralPath $ProjectDir -Filter '*.uproject' -File)
    if ($projects.Count -ne 1) {
        throw "Expected exactly one .uproject in $ProjectDir, found $($projects.Count)"
    }
    return $projects[0].FullName
}

function Test-AirSimRpc {
    param([string]$HostName, [int]$Port, [string]$LogPath)
    $probeCode = "import airsim; c=airsim.MultirotorClient(ip='$HostName', port=$Port, timeout_value=2); c.confirmConnection(); print('AIRSIM_RPC_READY')"
    $result = Invoke-ProcessCapture -FilePath $EndpointPython -Arguments @('-c', $probeCode)
    Add-Content -Encoding UTF8 -Path $LogPath -Value @(
        "COMMAND: $EndpointPython -c <airsim rpc probe>",
        "EXIT_CODE: $($result.ExitCode)",
        "[stdout]", $result.Stdout,
        "[stderr]", $result.Stderr, ""
    )
    return $result.ExitCode -eq 0
}

function Wait-AirSimRpc {
    param([string]$HostName, [int]$Port, [int]$TimeoutSec, [string]$LogPath)
    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    while ((Get-Date) -lt $deadline) {
        if (Test-AirSimRpc -HostName $HostName -Port $Port -LogPath $LogPath) {
            return $true
        }
        Start-Sleep -Seconds 3
    }
    return $false
}

# ── 前置检查 ───────────────────────────────────────────────────────────

$unitLog = Join-Path $EpisodeDir 'windows_unit_tests.log'
$unitCode = Invoke-Logged -FilePath 'python' `
    -Arguments @('-m', 'unittest', 'discover', 'bridge\windows\tests') `
    -LogPath $unitLog
if ($unitCode -ne 0) {
    throw "Windows unit tests failed, see $unitLog"
}

if (-not (Test-Path -LiteralPath $EndpointPython)) {
    throw "Endpoint Python not found: $EndpointPython"
}
$airsimImportLog = Join-Path $EpisodeDir 'airsim_python_import.log'
$airsimImport = Invoke-Logged -FilePath $EndpointPython `
    -Arguments @('-c', "import airsim; print('airsim import OK')") `
    -LogPath $airsimImportLog
if ($airsimImport -ne 0) {
    throw "AirSim Python package is not importable, see $airsimImportLog"
}

# ── 运行时信息 ─────────────────────────────────────────────────────────

$episodeName = Split-Path -Leaf $EpisodeDir
$wslHome = Invoke-WslText 'printf "%s" "$HOME"'
$WslEpisodeDir = "$wslHome/uavsingle_ros2_ws/episodes/$episodeName"

$BridgeHost = Select-FirstIpv4 (Invoke-WslText 'hostname -I | awk ''{print $1; exit}''') 'BridgeHost'
$EndpointHost = Select-FirstIpv4 (Invoke-WslText 'ip route show default | awk ''{print $3; exit}''') 'EndpointHost'
$wslCommit = Invoke-WslText "cd ~/uavsingle_ros2_ws && git rev-parse HEAD"

Write-Host "BridgeHost:   $BridgeHost"
Write-Host "EndpointHost: $EndpointHost"

# ── Manifest ───────────────────────────────────────────────────────────

$manifestLog = Join-Path $EpisodeDir 'manifest.log'
$manifestArgs = @(
    (Join-Path $ScriptDir 'generate_manifest.py'),
    '--episode-dir', $EpisodeDir
)
$manifestCode = Invoke-Logged -FilePath 'python' -Arguments $manifestArgs -LogPath $manifestLog
if ($manifestCode -ne 0) {
    throw "Manifest generation failed, see $manifestLog"
}

$handles = @{}
$airSimProjectResolved = ''
$airSimStartedByScript = $false
$airSimRpcReady = $false
$failureMessage = ''
$matlabLaunched = $false
$startTime = Get-Date
$endTime = $startTime

# ── 主编排 ─────────────────────────────────────────────────────────────

try {
    # ── 1. AirSim ──────────────────────────────────────────────────
    $rpcProbeLog = Join-Path $EpisodeDir 'airsim_rpc_probe.log'
    if (-not (Test-Path -LiteralPath $UE4Editor)) {
        throw "UE4Editor executable not found: $UE4Editor"
    }
    $airSimProjectResolved = Resolve-AirSimProjectPath -ProjectDir $AirSimProjDir
    if (-not (Test-Path -LiteralPath $airSimProjectResolved)) {
        throw "AirSim project file not found: $airSimProjectResolved"
    }

    if (Test-AirSimRpc -HostName '127.0.0.1' -Port $AirSimRpcPort -LogPath $rpcProbeLog) {
        Write-Host "AirSim RPC is already ready at 127.0.0.1:${AirSimRpcPort}"
        $airSimRpcReady = $true
    } else {
        $airSimArgs = @(
            $airSimProjectResolved,
            $AirSimMap,
            '-game', '-windowed',
            '-ResX=1280', '-ResY=720',
            "-AbsLog=$(Join-Path $EpisodeDir 'airsim_ue.log')"
        )
        $handles['airsim'] = Start-LoggedProcess -FilePath $UE4Editor `
            -Arguments $airSimArgs `
            -StdoutPath (Join-Path $EpisodeDir 'airsim_launcher.stdout.log') `
            -StderrPath (Join-Path $EpisodeDir 'airsim_launcher.stderr.log') `
            -WorkingDirectory (Split-Path -Parent $airSimProjectResolved)
        $airSimStartedByScript = $true
    }

    if (-not $airSimRpcReady) {
        Write-Host "Waiting for AirSim RPC 127.0.0.1:${AirSimRpcPort}..."
        $airSimRpcReady = Wait-AirSimRpc -HostName '127.0.0.1' -Port $AirSimRpcPort `
            -TimeoutSec $AirSimTimeout -LogPath $rpcProbeLog
    }
    if (-not $airSimRpcReady) {
        throw "AirSim RPC did not become ready within ${AirSimTimeout}s; see $rpcProbeLog"
    }

    # ── 2. WSL Runtime ─────────────────────────────────────────────
    $wslDurationSec = 86400
    $wslRuntimePath = "$wslHome/uavsingle_ros2_ws/src/scripts/wsl_hover_runtime.sh"

    $wslProcArgs = @(
        '-d', $Distro, '--', 'bash', $wslRuntimePath,
        $WslEpisodeDir,
        $wslDurationSec.ToString(),
        $EndpointHost
    )
    $wslProcess = Start-Process -FilePath 'wsl.exe' -ArgumentList $wslProcArgs -PassThru
    $handles['wsl'] = [pscustomobject]@{
        Name       = 'wsl.exe'
        Process    = $wslProcess
        ExitCode   = $null
        StdoutTask = $null
        StderrTask = $null
        StdoutPath = (Join-Path $EpisodeDir 'wsl_runner.stdout.log')
        StderrPath = (Join-Path $EpisodeDir 'wsl_runner.stderr.log')
    }
    Start-Sleep -Seconds 2

    # ── 3. Endpoint ────────────────────────────────────────────────
    $endpointPy = Join-Path $RepoRoot 'bridge' 'windows' 'airsim_udp_endpoint.py'
    $handles['endpoint'] = Start-LoggedProcess -FilePath $EndpointPython `
        -Arguments @($endpointPy) `
        -StdoutPath (Join-Path $EpisodeDir 'endpoint_launcher.stdout.log') `
        -StderrPath (Join-Path $EpisodeDir 'endpoint_launcher.stderr.log') `
        -WorkingDirectory $EpisodeDir
    Start-Sleep -Seconds 3

    # ── 4. MATLAB ──────────────────────────────────────────────────
    if (-not (Test-Path -LiteralPath $MatlabExe)) {
        throw "MATLAB executable not found: $MatlabExe"
    }
    $fcDir = Join-Path $RepoRoot 'FC_SimulinkProject'
    $fcDirArg = $fcDir.Replace("'", "''")
    $matlabCommand = "cd('$fcDirArg'); openProject('$fcDirArg'); load_system('FlightCore_ROS2_loop');"
    Write-Host "Starting MATLAB desktop with model FlightCore_ROS2_loop..."
    Start-Process -FilePath $MatlabExe -ArgumentList @('-r', $matlabCommand)
    $matlabLaunched = $true

    # ── 5. Dashboard ───────────────────────────────────────────────
    $endpointLog = Join-Path $EpisodeDir 'endpoint.log'
    Write-Host ""
    Write-Host "========================================"
    Write-Host "  Environment ready."
    Write-Host "  AirSim:          running"
    Write-Host "  WSL services:    running (bridge, adapter, rosbag, PlotJuggler)"
    Write-Host "  Endpoint:        running"
    Write-Host "  MATLAB:          model loaded, click Run to start"
    Write-Host "========================================"
    Write-Host "  Live dashboard refreshes every 2s. Press Enter to stop."
    Write-Host ""

    $episodeMonitorStart = Get-Date
    $wslStatusWinPath = "\\wsl`$\$Distro" + ($WslEpisodeDir -replace '/', '\') + "\wsl_status.json"
    Start-Sleep -Seconds 3

    while ($true) {
        if ([Console]::KeyAvailable) {
            $key = [Console]::ReadKey($true)
            if ($key.Key -eq 'Enter') {
                Write-Host ""
                break
            }
        }

        $elapsed = [Math]::Round(((Get-Date) - $episodeMonitorStart).TotalSeconds, 0)

        $airsimOk = $false
        if ($handles.ContainsKey('airsim') -and $handles['airsim'].Process -and !$handles['airsim'].Process.HasExited) {
            $airsimOk = $true
        }

        $endpointOk = $false
        $endpointLine = ''
        if ($handles.ContainsKey('endpoint') -and $handles['endpoint'].Process -and !$handles['endpoint'].Process.HasExited) {
            $endpointOk = $true
            if (Test-Path $endpointLog) {
                $endpointLine = try { (Get-Content -Tail 1 -Path $endpointLog -ErrorAction Stop) -replace '\s+', ' ' } catch { '' }
                if ($endpointLine.Length -gt 130) { $endpointLine = $endpointLine.Substring(0, 130) + '...' }
            }
        }

        $wslBridge = '?'; $wslAdapter = '?'; $wslBag = '?'; $wslPj = '?'; $bagSize = '-'
        try {
            if (Test-Path $wslStatusWinPath) {
                $wslJson = Get-Content -Raw -Path $wslStatusWinPath -ErrorAction Stop
                if ($wslJson -and $wslJson.Trim() -ne '') {
                    $wslStatus = $wslJson | ConvertFrom-Json -ErrorAction Stop
                    if ($wslStatus) {
                        $wslBridge = if ($wslStatus.bridge -eq 1) { '[+]' } else { '[-]' }
                        $wslAdapter = if ($wslStatus.adapter -eq 1) { '[+]' } else { '[-]' }
                        $wslBag = if ($wslStatus.rosbag -eq 1) { '[+]' } else { '[-]' }
                        $wslPj = if ($wslStatus.plotjuggler -eq 1) { '[+]' } else { '[-]' }
                        $bagSize = if ($wslStatus.bag_size) { $wslStatus.bag_size } else { '-' }
                    }
                }
            }
        } catch {}

        Write-Host ("`n===== Episode Dashboard (+{0}s) =====" -f $elapsed)
        Write-Host ("  [{0}] AirSim          RPC 127.0.0.1:{1}" -f $(if($airsimOk){'[+]'}else{'[-]'}), $AirSimRpcPort)
        Write-Host ("  [{0}] Endpoint       {1}" -f $(if($endpointOk){'[+]'}else{'[-]'}), $(if($endpointLine){$endpointLine}else{'waiting for output...'}))
        Write-Host ("  [{0}] Bridge (UDP)   WSL ros2 node" -f $wslBridge)
        Write-Host ("  [{0}] Adapter (ESC)  WSL ros2 node" -f $wslAdapter)
        Write-Host ("  [{0}] Rosbag         {1}" -f $wslBag, $bagSize)
        Write-Host ("  [{0}] PlotJuggler    WSLg real-time curves" -f $wslPj)
        Write-Host ("  [-] MATLAB          Click Run to start simulation")
        Write-Host ("  Press Enter to stop all services...")

        Start-Sleep -Seconds 2
    }
} catch {
    $failureMessage = $_.Exception.Message
} finally {
    $endTime = Get-Date
    if ($handles.ContainsKey('airsim')) {
        Stop-LoggedProcess $handles['airsim']
    }
    foreach ($key in @('endpoint', 'wsl')) {
        if ($handles.ContainsKey($key)) {
            Stop-LoggedProcess $handles[$key]
        }
    }
    if ($matlabLaunched) {
        Write-Host "Stopping MATLAB..."
        try { Stop-Process -Name 'MATLAB' -Force -ErrorAction SilentlyContinue } catch {}
    }
}

# ── Summary ────────────────────────────────────────────────────────────

$summary = [ordered]@{
    episode_dir = $EpisodeDir
    start_time  = $startTime.ToString('o')
    end_time    = $endTime.ToString('o')
    actual_duration_sec = [Math]::Round(($endTime - $startTime).TotalSeconds, 3)
    bridge_host = $BridgeHost
    endpoint_host = $EndpointHost
    wsl = @{
        distro = $Distro
        episode_dir = $WslEpisodeDir
        commit = $wslCommit
    }
    airsim = @{
        project_dir = $AirSimProjDir
        project = $airSimProjectResolved
        map = $AirSimMap
        rpc_ready = $airSimRpcReady
        started_by_script = $airSimStartedByScript
    }
    config = $configPath
    failure = $failureMessage
}
$summary | ConvertTo-Json -Depth 8 | Set-Content -Encoding UTF8 -Path (Join-Path $EpisodeDir 'run_summary.json')

if ($failureMessage) {
    throw "Episode orchestration failed: $failureMessage"
}

Write-Host "Episode complete: $EpisodeDir"
Write-Host "  Logs:     $EpisodeDir"
Write-Host "  Rosbag2:  $WslEpisodeDir/rosbag2"
