function test_ros2_gateway_contract()
% Verify the FlightCore ROS2 interface source contract.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
ros2Dir = fullfile(projectRoot, '3_Integration', 'ROS2');

requiredFiles = {
    fullfile(ros2Dir, 'topics.yaml')
    fullfile(ros2Dir, 'README.md')
    fullfile(ros2Dir, 'flightcore_msgs', 'package.xml')
    fullfile(ros2Dir, 'flightcore_msgs', 'CMakeLists.txt')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Imu.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Gps.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'FlightCmd.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'EscCmd.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'StateEst.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'SystemHealth.msg')
    fullfile(ros2Dir, 'scripts', 'build_flightcore_msgs_wsl.sh')
    fullfile(ros2Dir, 'scripts', 'show_flightcore_topics_wsl.sh')
    fullfile(ros2Dir, 'scripts', 'pub_sample_topics_wsl.sh')
    };

for i = 1:numel(requiredFiles)
    assert(isfile(requiredFiles{i}), 'Missing ROS2 contract file: %s', requiredFiles{i});
end

topicsText = fileread(fullfile(ros2Dir, 'topics.yaml'));
requiredTopics = {
    '/uav/sensors/imu'
    '/uav/sensors/gps'
    '/uav/cmd/flight'
    '/uav/actuator/esc_cmd'
    '/uav/estimator/state'
    '/uav/health/status'
    };
for i = 1:numel(requiredTopics)
    assert(contains(topicsText, requiredTopics{i}), 'Missing topic in topics.yaml: %s', requiredTopics{i});
end

requiredMsgs = {
    'flightcore_msgs/Imu'
    'flightcore_msgs/Gps'
    'flightcore_msgs/FlightCmd'
    'flightcore_msgs/EscCmd'
    'flightcore_msgs/StateEst'
    'flightcore_msgs/SystemHealth'
    };
for i = 1:numel(requiredMsgs)
    assert(contains(topicsText, requiredMsgs{i}), 'Missing message in topics.yaml: %s', requiredMsgs{i});
end

assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Imu.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'accel_mps2', 'gyro_radps'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Gps.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'lat_deg', 'lon_deg', 'alt_m', 'velocity_ned_mps'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'FlightCmd.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'mode', 'position_ned_sp_m', 'velocity_ned_sp_mps', 'yaw_sp_rad'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'EscCmd.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'motor_cmd'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'StateEst.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'position_ned_m', 'velocity_ned_mps', 'attitude_quat_wxyz', 'status'});

readmeText = fileread(fullfile(ros2Dir, 'README.md'));
assert(contains(readmeText, 'Simulink ROS Toolbox Subscribe'), ...
    'README must describe the direct ROS Toolbox subscribe path.');
assert(contains(readmeText, 'Simulink ROS Toolbox Publish'), ...
    'README must describe the direct ROS Toolbox publish path.');
assert(contains(readmeText, 'legacy'), ...
    'README must mark RuntimeBridge/gateway path as legacy fallback.');

buildScriptText = fileread(fullfile(ros2Dir, 'scripts', 'build_flightcore_msgs_wsl.sh'));
assert(contains(buildScriptText, 'colcon build --packages-select flightcore_msgs'), ...
    'WSL build script must build flightcore_msgs by default.');
assert(~contains(buildScriptText, 'flightcore_ros2_gateway'), ...
    'WSL mainline build script must not build the legacy gateway.');

assert(contains(readmeText, '~/uavsingle_ros2_ws/config/plotjuggler_flightcore_topics.xml'), ...
    'README must point PlotJuggler to the WSL runtime config path.');

modelPath = fullfile(projectRoot, '3_Integration', 'FlightCore_ROS2_loop.slx');
assert(isfile(modelPath), 'Missing FlightCore_ROS2_loop.slx.');
modelText = extractAllSlxSystemText(modelPath);

for i = 1:5
    assert(contains(modelText, requiredTopics{i}), 'FlightCore_ROS2_loop missing topic: %s', requiredTopics{i});
end

for i = 1:5
    assert(contains(modelText, requiredMsgs{i}), 'FlightCore_ROS2_loop missing message type: %s', requiredMsgs{i});
end

forbiddenModelText = {
    'std_msgs/Bool'
    'std_msgs/Float32'
    'DefaultContractInputs'
    'RuntimeBridge'
    'SimAdapter'
    'flightcore_ros2_gateway'
    };
for i = 1:numel(forbiddenModelText)
    assert(~contains(modelText, forbiddenModelText{i}), ...
        'FlightCore_ROS2_loop must not contain forbidden mainline text: %s', forbiddenModelText{i});
end

requiredModelText = {
    'ROS2ToFlightCoreBus'
    'FlightCoreBusToROS2'
    'ImuMsgToBus'
    'GpsMsgToBus'
    'FlightCmdMsgToBus'
    'EscCmdBusToMsg'
    'StateEstBusToMsg'
    'accel_mps2'
    'gyro_radps'
    'lat_deg'
    'lon_deg'
    'alt_m'
    'velocity_ned_mps'
    'position_ned_sp_m'
    'velocity_ned_sp_mps'
    'yaw_sp_rad'
    'motor_cmd'
    'position_ned_m'
    'attitude_quat_wxyz'
    'angular_rate_body_radps'
    'gyro_bias_radps'
    'accel_bias_mps2'
    'wind_ned_mps'
    'IMU_BUS'
    'GPS_BUS'
    'FlightCmdBus'
    'EscCmdBus'
    'StateEstBus'
    };
for i = 1:numel(requiredModelText)
    assert(contains(modelText, requiredModelText{i}), ...
        'FlightCore_ROS2_loop missing expected adapter/model text: %s', requiredModelText{i});
end

fprintf('ROS2_INTERFACE_CONTRACT_PASS\n');
end

function assertMsgFields(filePath, fields)
text = fileread(filePath);
for i = 1:numel(fields)
    assert(contains(text, fields{i}), 'Message %s missing field: %s', filePath, fields{i});
end
end

function text = extractSlxText(slxPath, internalPath)
tmpDir = tempname;
mkdir(tmpDir);
cleanup = onCleanup(@() rmdir(tmpDir, 's'));
unzip(slxPath, tmpDir);
target = fullfile(tmpDir, strrep(internalPath, '/', filesep));
text = string(fileread(target));
clear cleanup;
end

function text = extractAllSlxSystemText(slxPath)
tmpDir = tempname;
mkdir(tmpDir);
cleanup = onCleanup(@() rmdir(tmpDir, 's'));
unzip(slxPath, tmpDir);
systemsDir = fullfile(tmpDir, 'simulink', 'systems');
files = dir(fullfile(systemsDir, '*.xml'));
parts = strings(numel(files) + 1, 1);
parts(1) = string(fileread(fullfile(tmpDir, 'simulink', 'blockdiagram.xml')));
for i = 1:numel(files)
    parts(i + 1) = string(fileread(fullfile(files(i).folder, files(i).name)));
end
text = join(parts, newline);
clear cleanup;
end
