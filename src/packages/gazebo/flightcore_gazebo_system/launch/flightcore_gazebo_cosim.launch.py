import os

# Process-level entry for the authoritative WSL co-simulation topology.
#
# Gazebo starts paused and no component advances simulation during discovery.
# The operator/test first calls PrimeSession, then start_coordinator. Coordinator
# is the only /clock and WorldControl authority after that gate opens.
#
# Process exits are coupled intentionally: a generated-node or Coordinator
# failure shuts down the full launch so a protocol error cannot leave Gazebo
# running with a stale actuator command. The GUI is an optional read-only
# observer and never participates in the lockstep barrier.

from ament_index_python.packages import get_package_prefix, get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    EmitEvent,
    ExecuteProcess,
    IncludeLaunchDescription,
    RegisterEventHandler,
    SetEnvironmentVariable,
)
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    """启动 paused 的严格锁步世界；PRIME 与启动门共同授权首拍。"""
    package_name = "flightcore_gazebo_system"
    share_dir = get_package_share_directory(package_name)
    install_prefix = get_package_prefix(package_name)
    world_path = os.path.join(share_dir, "worlds", "flightcore_hover.sdf")
    gui_config_path = os.path.join(
        share_dir, "config", "flightcore_lockstep_gui.config"
    )
    ros_gz_sim_share = get_package_share_directory("ros_gz_sim")
    plugin_dir = os.path.join(install_prefix, "lib")
    previous_plugin_path = os.environ.get("GZ_SIM_SYSTEM_PLUGIN_PATH", "")
    combined_plugin_path = os.pathsep.join(
        path for path in (plugin_dir, previous_plugin_path) if path
    )
    coordinator = Node(
        package="flightcore_simulation_coordinator",
        executable="flightcore_simulation_coordinator",
        name="flightcore_simulation_coordinator",
        output="screen",
        parameters=[
            {
                "world_name": "flightcore_world",
                "max_epochs": LaunchConfiguration("max_epochs"),
                "progress_timeout_ms": LaunchConfiguration(
                    "progress_timeout_ms"
                ),
            }
        ],
    )
    generated_flightcore = Node(
        package="flightcore_gazebo_loop",
        executable="FlightCore_Gazebo_loop",
        output="screen",
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "gui",
                default_value="true",
                description="启动只读 Gazebo GUI 观察客户端。",
            ),
            DeclareLaunchArgument(
                "max_epochs",
                default_value="20",
                description=(
                    "Coordinator 最多提交的 epoch 数；达到后不再发布 /clock。"
                ),
            ),
            DeclareLaunchArgument(
                "progress_timeout_ms",
                default_value="2000",
                description=(
                    "启动后协议无有效推进的墙钟超时；超时即报错并停止联合仿真。"
                ),
            ),
            SetEnvironmentVariable(
                "GZ_SIM_SYSTEM_PLUGIN_PATH", combined_plugin_path
            ),
            generated_flightcore,
            coordinator,
            RegisterEventHandler(
                OnProcessExit(
                    target_action=coordinator,
                    on_exit=[
                        EmitEvent(
                            event=Shutdown(
                                reason=(
                                    "FlightCore simulation coordinator exited; "
                                    "stopping the complete co-simulation."
                                )
                            )
                        )
                    ],
                )
            ),
            RegisterEventHandler(
                OnProcessExit(
                    target_action=generated_flightcore,
                    on_exit=[
                        EmitEvent(
                            event=Shutdown(
                                reason=(
                                    "Generated FlightCore node exited; "
                                    "stopping the complete co-simulation."
                                )
                            )
                        )
                    ],
                )
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(ros_gz_sim_share, "launch", "gz_sim.launch.py")
                ),
                # 不使用 -r，确保世界从 paused 状态开始。
                launch_arguments={"gz_args": f"-s -v 3 {world_path}"}.items(),
            ),
            ExecuteProcess(
                cmd=[
                    "gz",
                    "sim",
                    "-g",
                    "-v",
                    "3",
                    "--gui-config",
                    gui_config_path,
                ],
                condition=IfCondition(LaunchConfiguration("gui")),
                output="screen",
            ),
        ]
    )
