from glob import glob
from setuptools import find_packages, setup

package_name = "flightcore_runtime_adapter"

setup(
    name=package_name,
    version="0.1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        ("share/" + package_name + "/config", glob("config/*.yaml")),
        ("share/" + package_name + "/docs", glob("docs/*.md")),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="UAVSingleFlightControl",
    maintainer_email="devnull@example.com",
    description="ROS2 runtime adapter between aircraft observation topics and FlightCore contract topics.",
    license="Proprietary",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "flightcore_runtime_adapter = flightcore_runtime_adapter.adapter_node:main",
        ],
    },
)
