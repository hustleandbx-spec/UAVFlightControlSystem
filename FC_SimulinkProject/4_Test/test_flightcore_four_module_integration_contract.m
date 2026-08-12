function test_flightcore_four_module_integration_contract()
%TEST_FLIGHTCORE_FOUR_MODULE_INTEGRATION_CONTRACT Verify the product chain.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
flightCorePath = fullfile(projectRoot, '3_Integration', 'FlightCore', ...
    'FlightCore.slx');
flightControlPath = fullfile(projectRoot, '2_Model', 'control', ...
    'UAV_FlightControl.slx');
gazeboLoopPath = fullfile(projectRoot, '3_Integration', ...
    'FlightCore_Gazebo_loop.slx');

assert(isfile(flightCorePath), 'Missing FlightCore.slx.');
assert(isfile(flightControlPath), 'Missing UAV_FlightControl.slx.');
assert(isfile(gazeboLoopPath), 'Missing FlightCore_Gazebo_loop.slx.');

flightCoreText = extractAllSlxSystemText(flightCorePath);
flightControlText = extractAllSlxSystemText(flightControlPath);
gazeboLoopText = extractAllSlxSystemText(gazeboLoopPath);

requiredFlightCoreText = {
    'IMU_BUS'
    'GPS_BUS'
    'MissionPlan'
    'CommandIngressSnapshot'
    'Gateway'
    'Commander'
    'MissionManager'
    'Navigator'
    'UAV_FlightControl'
    'CommanderStatus'
    'MissionStatus'
    'NavigationContract'
    'NavigationStatus'
    'TrajectorySetpoint'
    'PreviousMissionStatus'
    'PreviousNavigationStatus'
    'ControlAuthorization'
    'ActuatorEnabled'
    };
for i = 1:numel(requiredFlightCoreText)
    assert(contains(flightCoreText, requiredFlightCoreText{i}), ...
        'FlightCore missing required integration element: %s', ...
        requiredFlightCoreText{i});
end

requiredFlightControlText = {
    'TrajectorySetpointBus'
    'TrajectorySetpointSelector'
    'Velocity_NED_SP'
    'VelocityFeedforwardSum'
    };
for i = 1:numel(requiredFlightControlText)
    assert(contains(flightControlText, requiredFlightControlText{i}), ...
        'UAV_FlightControl missing required trajectory interface element: %s', ...
        requiredFlightControlText{i});
end

assert(~contains(gazeboLoopText, 'InjectedMissionScenario'), ...
    'Gazebo loop must not inject a private mission scenario.');
assert(~contains(gazeboLoopText, 'ArmThenStartSnapshot'), ...
    'Gazebo loop must obtain arm and mission-start commands through FlightCore Gateway.');

forbiddenFlightCoreText = {
    'TestPositionCommandStep'
    'TestArmRequestStep'
    'ROS2'
    'Gazebo'
    };
for i = 1:numel(forbiddenFlightCoreText)
    assert(~contains(flightCoreText, forbiddenFlightCoreText{i}), ...
        'FlightCore contains forbidden test/runtime term: %s', ...
        forbiddenFlightCoreText{i});
end

fprintf('FLIGHTCORE_FOUR_MODULE_INTEGRATION_CONTRACT_PASS\n');
end

function text = extractAllSlxSystemText(slxPath)
tmpDir = tempname;
mkdir(tmpDir);
cleanup = onCleanup(@() rmdir(tmpDir, 's'));
unzip(slxPath, tmpDir);

parts = strings(0, 1);
rootFiles = dir(fullfile(tmpDir, 'simulink', 'blockdiagram.xml'));
systemFiles = dir(fullfile(tmpDir, 'simulink', 'systems', '*.xml'));
stateflowFiles = dir(fullfile(tmpDir, 'simulink', 'stateflow', '*.xml'));
files = [rootFiles; systemFiles; stateflowFiles];
for i = 1:numel(files)
    parts(end + 1, 1) = string(fileread(fullfile( ...
        files(i).folder, files(i).name))); %#ok<AGROW>
end
text = join(parts, newline);
clear cleanup;
end
