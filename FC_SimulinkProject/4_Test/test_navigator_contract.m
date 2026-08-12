function test_navigator_contract()
%TEST_NAVIGATOR_CONTRACT Verify the standalone Navigator architecture.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
modelPath = fullfile(projectRoot, '2_Model', 'navigator', 'Navigator.slx');
assert(isfile(modelPath), 'Missing standalone Navigator.slx.');

modelText = extractAllSlxSystemText(modelPath);
requiredText = {
    'NavigationContract'
    'StateEstBus'
    'TrajectorySetpoint'
    'NavigationStatus'
    'DirectToPointNavigator'
    'ContractSelector'
    'StateSelector'
    'HorizontalDistance'
    'VerticalDistance'
    'RemainingDistance'
    'DwellElapsed'
    'ContractChanged'
    'Detect Change'
    'TrajectorySetpointBuilder'
    'NavigationStatusBuilder'
    };
for i = 1:numel(requiredText)
    assert(contains(modelText, requiredText{i}), ...
        'Navigator missing required element: %s', requiredText{i});
end

assert(~contains(modelText, 'persistent'), ...
    'Navigator must expose temporal state through Simulink state blocks.');
assert(~contains(modelText, 'PreviousObjectiveId'), ...
    'Objective change detection must use the native Detect Change block.');

forbiddenText = {
    'MAVLink'
    'QGroundControl'
    'MissionState'
    'MissionPlan'
    'ROS2'
    'Gazebo'
    'FlightCmdBus'
    };
for i = 1:numel(forbiddenText)
    assert(~contains(modelText, forbiddenText{i}), ...
        'Navigator contains forbidden task/protocol/runtime term: %s', ...
        forbiddenText{i});
end

busConfigDir = fullfile(projectRoot, '1_Data_Dictionaries', 'BusConfig');
contractBusText = fileread(fullfile(busConfigDir, ...
    'config_NavigationContractBus.m'));
forbiddenContractFields = {"'ObjectiveType'", "'TaskType'", "'ItemType'"};
for i = 1:numel(forbiddenContractFields)
    assert(~contains(contractBusText, forbiddenContractFields{i}), ...
        'Navigator input contract must remain task-semantic free.');
end
requiredBusConfigs = {
    'config_NavigationContractBus.m'
    'config_StateEstBus.m'
    'config_TrajectorySetpointBus.m'
    'config_NavigationStatusBus.m'
    };
for i = 1:numel(requiredBusConfigs)
    assert(isfile(fullfile(busConfigDir, requiredBusConfigs{i})), ...
        'Missing Navigator bus config: %s', requiredBusConfigs{i});
end

fprintf('NAVIGATOR_CONTRACT_PASS\n');
end

function text = extractAllSlxSystemText(slxPath)
tmpDir = tempname;
mkdir(tmpDir);
cleanup = onCleanup(@() rmdir(tmpDir, 's'));
unzip(slxPath, tmpDir);
files = dir(fullfile(tmpDir, 'simulink', 'systems', '*.xml'));
parts = strings(numel(files) + 1, 1);
parts(1) = string(fileread(fullfile(tmpDir, ...
    'simulink', 'blockdiagram.xml')));
for i = 1:numel(files)
    parts(i + 1) = string(fileread(fullfile(files(i).folder, files(i).name)));
end
text = join(parts, newline);
clear cleanup;
end
