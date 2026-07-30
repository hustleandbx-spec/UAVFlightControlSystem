function test_ros2_gateway_contract()
% Verify the FlightCore ROS2 interface source contract.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
ros2Dir = fullfile(projectRoot, '3_Integration', 'ROS2');

requiredFiles = {
    fullfile(ros2Dir, 'flightcore_msgs', 'package.xml')
    fullfile(ros2Dir, 'flightcore_msgs', 'CMakeLists.txt')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Imu.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Gps.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'FlightCmd.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'EscCmd.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'StateEst.msg')
    fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'SystemHealth.msg')
    fullfile(ros2Dir, 'generate_flightcore_ros2_messages.m')
    };

for i = 1:numel(requiredFiles)
    assert(isfile(requiredFiles{i}), 'Missing ROS2 contract file: %s', requiredFiles{i});
end

requiredTopics = {
    '/uav/sensors/imu'
    '/uav/sensors/gps'
    '/uav/cmd/flight'
    '/uav/actuator/esc_cmd'
    '/uav/estimator/state'
    '/uav/health/status'
    };

requiredMsgs = {
    'flightcore_msgs/Imu'
    'flightcore_msgs/Gps'
    'flightcore_msgs/FlightCmd'
    'flightcore_msgs/EscCmd'
    'flightcore_msgs/StateEst'
    'flightcore_msgs/SystemHealth'
    };

assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Imu.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'accel_mps2', 'gyro_radps'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'Gps.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'lat_deg', 'lon_deg', 'alt_m', 'velocity_ned_mps'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'FlightCmd.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'mode', 'position_ned_sp_m', 'velocity_ned_sp_mps', 'yaw_sp_rad'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'EscCmd.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'armed', 'valid', 'motor_cmd'});
assertMsgFields(fullfile(ros2Dir, 'flightcore_msgs', 'msg', 'StateEst.msg'), ...
    {'timestamp_sec', 'sequence', 'source_id', 'valid', 'position_ned_m', 'velocity_ned_mps', 'attitude_quat_wxyz', 'status'});

matlabGeneratorText = fileread(fullfile(ros2Dir, 'generate_flightcore_ros2_messages.m'));
assert(contains(matlabGeneratorText, 'ros2genmsg(ros2Dir)'), ...
    'MATLAB custom-message generator must invoke ros2genmsg for the ROS2 directory.');

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
