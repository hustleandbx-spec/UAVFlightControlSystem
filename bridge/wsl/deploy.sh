#!/usr/bin/env bash
# deploy.sh — 把 Windows 侧 bridge/wsl/ 部署到 WSL ~/uavsingle_ros2_ws/src/
#
# 用法（在 Windows 终端或 WSL 内均可）:
#   bash deploy.sh
#
# Windows git repo 是 WSL 文件的唯一权威源。
# 本脚本用 rsync 单向同步，目标目录中不在源目录的文件不会被删除，
# 但同名文件会被覆盖。

set -euo pipefail

# 源目录：脚本所在位置即 bridge/wsl/
SRC_DIR="$(cd "$(dirname "$0")" && pwd)"
# 目标：WSL ROS2 workspace 的 src/
DST_DIR="${HOME}/uavsingle_ros2_ws/src"

echo "=== deploy WSL files ==="
echo "  source: ${SRC_DIR}"
echo "  target: ${DST_DIR}"

# ── packages/ → src/ (ROS2 包) ──
echo ""
echo "[1/3] deploying packages..."
rsync -av --delete "${SRC_DIR}/packages/" "${DST_DIR}/"

# ── config/ → src/config/ ──
echo ""
echo "[2/3] deploying config..."
rsync -av --delete "${SRC_DIR}/config/" "${DST_DIR}/config/"

# ── scripts/ → src/scripts/ ──
echo ""
echo "[3/3] deploying scripts..."
rsync -av --delete "${SRC_DIR}/scripts/" "${DST_DIR}/scripts/"

echo ""
echo "=== deploy complete ==="
echo "Run 'colcon build --symlink-install' in ~/uavsingle_ros2_ws to rebuild."
