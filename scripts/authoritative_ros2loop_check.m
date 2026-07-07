function authoritative_ros2loop_check()
%AUTHORITATIVE_ROS2LOOP_CHECK Prove which ROS2 loop model compiles and sims.

projectPath = 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject\FC_SimulinkProject.prj';
modelName = 'FlightCore_ROS2_loop';

fprintf('=== authoritative_ros2loop_check ===\n');
fprintf('MATLAB:  %s\n', version);
fprintf('Project: %s\n', projectPath);

proj = openProject(projectPath);
cleanupProject = onCleanup(@() close(proj));

load_system(modelName);
cleanupModel = onCleanup(@() close_system(modelName, 0));

modelFile = get_param(modelName, 'FileName');
fprintf('Loaded model name: %s\n', get_param(modelName, 'Name'));
fprintf('Loaded model file: %s\n', modelFile);
fprintf('Dirty: %s\n', get_param(modelName, 'Dirty'));

expectedFile = fullfile(fileparts(projectPath), '3_Integration', [modelName '.slx']);
fprintf('Expected file:     %s\n', expectedFile);
if ~strcmpi(modelFile, expectedFile)
    error('Loaded model is not the expected ROS2 loop model.');
end

fprintf('Running explicit update...\n');
set_param(modelName, 'SimulationCommand', 'update');
fprintf('UPDATE_PASS\n');

fprintf('Running blocking sim StopTime=2...\n');
simOut = sim(modelName, 'StopTime', '2', 'ReturnWorkspaceOutputs', 'on');
fprintf('SIM_PASS\n');

try
    stopEvent = simOut.SimulationMetadata.ExecutionInfo.StopEvent;
    fprintf('StopEvent: %s\n', string(stopEvent));
catch
end
fprintf('Final status: %s\n', get_param(modelName, 'SimulationStatus'));
end
