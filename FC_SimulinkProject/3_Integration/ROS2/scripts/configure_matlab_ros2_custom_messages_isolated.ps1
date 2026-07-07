$ErrorActionPreference = "Stop"

$projectRoot = "D:\Project\UAVSingleFlightControl\FC_SimulinkProject"
$ros2Dir = Join-Path $projectRoot "3_Integration\ROS2"
$homeDir = Join-Path $projectRoot ".cache\matlab_ros2_home"
$tempDir = Join-Path $projectRoot ".cache\matlab_ros2_temp"
$matlabExe = "D:\MATLAB\R2025b\bin\matlab.exe"
$pythonExe = "D:\Miniconda\envs\airsim\python.exe"

New-Item -ItemType Directory -Force -Path $homeDir | Out-Null
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

$env:USERPROFILE = $homeDir
$env:HOME = $homeDir
$env:TEMP = $tempDir
$env:TMP = $tempDir

& $matlabExe -batch "cd('$ros2Dir'); configure_matlab_ros2_custom_messages('$pythonExe')"
