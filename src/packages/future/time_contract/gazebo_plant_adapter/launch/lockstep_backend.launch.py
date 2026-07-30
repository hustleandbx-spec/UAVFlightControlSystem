import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node


def generate_launch_description():
    package_share = get_package_share_directory(
        "gazebo_plant_adapter"
    )

    world_file = os.path.join(
        package_share,
        "worlds",
        "lockstep_world.sdf"
    )

    ros_gz_sim_share = get_package_share_directory(
        "ros_gz_sim"
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                ros_gz_sim_share,
                "launch",
                "gz_sim.launch.py"
            )
        ),
        launch_arguments={
            # -s表示只启动Gazebo server。
            # 不使用-r，保证仿真初始保持paused。
            "gz_args": f"-s -v 4 {world_file}"
        }.items()
    )

    bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        name="gazebo_lockstep_bridge",
        output="screen",
        arguments=[
            (
                "/world/lockstep_world/control"
                "@ros_gz_interfaces/srv/ControlWorld"
            ),
            (
                "/world/lockstep_world/stats"
                "@ros_gz_interfaces/msg/WorldStatistics"
                "[gz.msgs.WorldStatistics"
            ),
        ],
    )

    return LaunchDescription([
        gazebo,
        bridge,
    ])