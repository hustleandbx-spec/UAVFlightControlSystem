from glob import glob
from setuptools import find_packages, setup

package_name = "aircraft_udp_bridge"

setup(
    name=package_name,
    version="0.1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        ("share/" + package_name + "/schemas", glob("schemas/*.schema.json")),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="UAVSingleFlightControl",
    maintainer_email="devnull@example.com",
    description="UDP-only simulator boundary to ROS2 aircraft state/control topics.",
    license="Proprietary",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "aircraft_udp_bridge = aircraft_udp_bridge.bridge_node:main",
        ],
    },
)
