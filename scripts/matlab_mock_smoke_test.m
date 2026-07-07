function matlab_mock_smoke_test()
    % FlightCore ROS2 Loop Mock Smoke Test
    % Tests the full internal pipeline without WSL

    fprintf('=== FlightCore ROS2 Loop Mock Smoke Test ===\n');
    t_start_all = tic;

    % Open project
    try
        proj = openProject('D:\Project\UAVSingleFlightControl\FC_SimulinkProject\FC_SimulinkProject.prj');
        fprintf('[OK] Project opened: %s\n', proj.Name);
    catch e
        fprintf('[FAIL] Project open: %s\n', e.message);
        return;
    end

    % Create ROS2 node for test
    try
        test_node = ros2node('/mock_test_node');
        fprintf('[OK] Test ROS2 node created\n');
    catch e
        fprintf('[FAIL] ROS2 node: %s\n', e.message);
        return;
    end

    % Create input publishers
    try
        imu_pub = ros2publisher(test_node, '/uav/sensors/imu', 'flightcore_msgs/Imu');
        gps_pub = ros2publisher(test_node, '/uav/sensors/gps', 'flightcore_msgs/Gps');
        cmd_pub = ros2publisher(test_node, '/uav/cmd/flight', 'flightcore_msgs/FlightCmd');
        fprintf('[OK] Input publishers (imu/gps/cmd) created\n');
    catch e
        fprintf('[FAIL] Publisher creation: %s\n', e.message);
        delete(test_node);
        return;
    end

    % Subscriber for actuator output
    actuator_msgs = {};
    esc_sub = ros2subscriber(test_node, '/uav/actuator/esc_cmd', ...
        'flightcore_msgs/EscCmd', @(msg) on_actuator(msg));
    fprintf('[OK] ESC command subscriber created\n');

    function on_actuator(msg)
        actuator_msgs{end+1} = msg;
        n = length(actuator_msgs);
        if mod(n, 10) == 1 || n <= 3
            fprintf('  Actuator[%d]: motor=[%.3f %.3f %.3f %.3f] seq=%d\n', ...
                n, msg.motor_cmd(1), msg.motor_cmd(2), ...
                msg.motor_cmd(3), msg.motor_cmd(4), msg.sequence);
        end
    end

    % Load model
    try
        load_system('FlightCore_ROS2_loop');
        set_param('FlightCore_ROS2_loop', 'StopTime', '8');
        fprintf('[OK] Model loaded, StopTime=8s\n');
    catch e
        fprintf('[FAIL] Model load: %s\n', e.message);
        delete(imu_pub); delete(gps_pub); delete(cmd_pub); delete(esc_sub);
        delete(test_node);
        return;
    end

    % Send hover command
    fprintf('Sending hover command (pos=[0,0,-2] mode=1)...\n');
    pause(0.5);  % Allow ROS2 server to stabilize
    flightCmdMsg = ros2message('flightcore_msgs/FlightCmd');
    flightCmdMsg.stamp.sec = int32(1);
    flightCmdMsg.timestamp_sec = 1.0;
    flightCmdMsg.sequence = uint32(1);
    flightCmdMsg.source_id = uint8(1);
    flightCmdMsg.valid = true;
    flightCmdMsg.mode = uint8(1);
    flightCmdMsg.position_ned_sp_m = single([0.0, 0.0, -2.0]);
    flightCmdMsg.velocity_ned_sp_mps = single([0.0, 0.0, 0.0]);
    flightCmdMsg.yaw_sp_rad = single(0.0);
    send(cmd_pub, flightCmdMsg);

    % Start simulation
    fprintf('Starting simulation (8 seconds)...\n');
    try
        set_param('FlightCore_ROS2_loop', 'SimulationCommand', 'start');
        fprintf('[OK] Simulation started\n');
    catch e
        fprintf('[FAIL] Simulation start: %s\n', e.message);
    end

    % Publish mock sensor data during simulation
    imu_seq = 0;
    gps_seq = 0;
    rx_count = 0;

    while toc(t_start_all) < 7
        pause(0.02);
        imu_seq = imu_seq + 1;

        % IMU at ~50 Hz with slight noise
        imuMsg = ros2message('flightcore_msgs/Imu');
        imuMsg.stamp.sec = int32(1 + toc(t_start_all));
        imuMsg.timestamp_sec = 1.0 + toc(t_start_all);
        imuMsg.sequence = uint32(imu_seq);
        imuMsg.source_id = uint8(0);
        imuMsg.valid = true;
        imuMsg.accel_mps2 = single([0.0, 0.0, -9.80665]);
        imuMsg.gyro_radps = single([0.0, 0.0, 0.0]);
        send(imu_pub, imuMsg);

        % GPS at ~5 Hz
        if mod(imu_seq, 10) == 0
            gps_seq = gps_seq + 1;
            gpsMsg = ros2message('flightcore_msgs/Gps');
            gpsMsg.stamp.sec = int32(1 + toc(t_start_all));
            gpsMsg.timestamp_sec = 1.0 + toc(t_start_all);
            gpsMsg.sequence = uint32(gps_seq);
            gpsMsg.source_id = uint8(0);
            gpsMsg.valid = true;
            gpsMsg.lat_deg = single(47.641468);
            gpsMsg.lon_deg = single(-122.140165);
            gpsMsg.alt_m = single(122.0);
            gpsMsg.velocity_ned_mps = single([0.0, 0.0, 0.0]);
            send(gps_pub, gpsMsg);
        end

        % Brief pause to allow ROS2 message processing
        pause(0.001);
    end

    % Collect final messages
    pause(0.5);

    % Stop simulation
    fprintf('Stopping simulation...\n');
    set_param('FlightCore_ROS2_loop', 'SimulationCommand', 'stop');
    pause(0.5);

    % Try to spin for any remaining messages
    try
        ros2Spin(test_node, 0.1);
    catch
        % ros2Spin may not be available in all MATLAB versions
    end

    % Results
    n_actuator = length(actuator_msgs);
    fprintf('\n========================================\n');
    fprintf('  MOCK SMOKE TEST RESULTS\n');
    fprintf('========================================\n');
    fprintf('  IMU messages sent:     %d\n', imu_seq);
    fprintf('  GPS messages sent:     %d\n', gps_seq);
    fprintf('  Actuator messages rx:  %d\n', n_actuator);
    fprintf('----------------------------------------\n');

    if n_actuator >= 5
        first = actuator_msgs{1};
        last = actuator_msgs{end};
        fprintf('  First motor cmd: [%.3f %.3f %.3f %.3f]\n', ...
            first.motor_cmd(1), first.motor_cmd(2), first.motor_cmd(3), first.motor_cmd(4));
        fprintf('  Last motor cmd:  [%.3f %.3f %.3f %.3f]\n', ...
            last.motor_cmd(1), last.motor_cmd(2), last.motor_cmd(3), last.motor_cmd(4));
        fprintf('  VERDICT: PASS\n');
    else
        fprintf('  VERDICT: FAIL (insufficient actuator output)\n');
    end
    fprintf('========================================\n');
end
