# Message Generation Scaffold

The package is a normal ROS2 interface package. Build it with `colcon build` in a ROS2 workspace, then point MATLAB ROS Toolbox to the generated custom message path before compiling `FlightCore_ROS2_loop.slx`.

The current Simulink model upgrade script is:

```matlab
upgrade_FlightCore_ROS2_loop_contract
```

It sets topic and message-type parameters on existing ROS2 Subscribe/Publish blocks and preserves the FlightCore Bus boundary.
