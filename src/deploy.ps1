# deploy.ps1 - 将 Windows bridge/wsl/ 单向镜像到 WSL 工作区 src/。

param(
    [string]$Distro = 'Ubuntu-24.04',
    [string]$Workspace = '/home/hustle/uavsingle_ros2_ws'
)

$ErrorActionPreference = 'Stop'

$sourceWindows = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$sourceForWsl = $sourceWindows -replace '\\', '/'
$source = (& wsl.exe -d $Distro -- wslpath -a $sourceForWsl).Trim()
$destination = "$Workspace/src"

& wsl.exe -d $Distro -- test -d $destination
if ($LASTEXITCODE -ne 0) {
    throw "WSL workspace src directory not found: $destination"
}

Write-Host "Mirror $source/ -> $destination/"
& wsl.exe -d $Distro -- rsync -av --delete --delete-excluded `
    --exclude='__pycache__/' `
    --exclude='*.pyc' `
    --exclude='*.pyo' `
    "$source/" "$destination/"
if ($LASTEXITCODE -ne 0) {
    throw "WSL mirror failed with exit code $LASTEXITCODE"
}
