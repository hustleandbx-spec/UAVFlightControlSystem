function run_matlab_flightcore_ros2_episode(durationSec, logPath)
%RUN_MATLAB_FLIGHTCORE_ROS2_EPISODE Run FlightCore_ROS2_loop for an episode.
%
% This is intended to be launched by scripts/test_hover_episode.ps1.  It does
% not publish mock sensor data itself; WSL runtime nodes should provide /uav/*
% inputs and consume /uav/actuator/esc_cmd.

if nargin < 1 || isempty(durationSec)
    durationSec = 30;
end
if nargin < 2
    logPath = '';
end

if ~isempty(logPath)
    logDir = fileparts(logPath);
    if ~isempty(logDir) && ~exist(logDir, 'dir')
        mkdir(logDir);
    end
    diary(logPath);
    cleanupDiary = onCleanup(@() diary('off'));
end

fprintf('=== MATLAB FlightCore ROS2 episode runner ===\n');
fprintf('Duration: %.3f s\n', durationSec);

projectPath = 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject\FC_SimulinkProject.prj';
modelName = 'FlightCore_ROS2_loop';

proj = openProject(projectPath);
cleanupProject = onCleanup(@() close(proj));

load_system(modelName);
cleanupModel = onCleanup(@() close_system(modelName, 0));

stopTime = max(durationSec + 2.0, durationSec);
set_param(modelName, 'StopTime', num2str(stopTime, '%.3f'));
try
    set_param(modelName, 'EnablePacing', 'on');
    set_param(modelName, 'PacingRate', '1');
    fprintf('Simulation pacing enabled at 1.0x wall time\n');
catch pacingError
    fprintf('WARNING: could not enable simulation pacing: %s\n', pacingError.message);
end

% Pre-simulation ROS2 check
fprintf('Pre-sim ROS2 check:\n');
try
    topics = ros2('topic', 'list');
    uavTopics = topics(contains(topics, '/uav/'));
    aircraftTopics = topics(contains(topics, '/aircraft/'));
    fprintf('  %d /uav/* topics, %d /aircraft/* topics\n', length(uavTopics), length(aircraftTopics));
    if ~isempty(uavTopics)
        for i = 1:length(uavTopics)
            fprintf('    %s\n', uavTopics{i});
        end
    end
catch e
    fprintf('  ROS2 topic check failed: %s\n', e.message);
end

fprintf('Starting %s, StopTime=%.3f\n', modelName, stopTime);
set_param(modelName, 'SimulationCommand', 'start');

startWall = tic;
while toc(startWall) < stopTime + 5.0
    status = get_param(modelName, 'SimulationStatus');
    fprintf('t=%.3f status=%s\n', toc(startWall), status);
    if any(strcmp(status, {'stopped', 'terminating'}))
        % Capture diagnostic messages if simulation stopped early
        try
            diagMsgs = get_param(modelName, 'DiagnosticMessages');
            if ~isempty(diagMsgs)
                fprintf('Diagnostic messages:\n');
                for i = 1:length(diagMsgs)
                    fprintf('  [%s] %s: %s\n', diagMsgs(i).Type, diagMsgs(i).Component, diagMsgs(i).Message);
                end
            end
        catch diagErr
            fprintf('Could not get diagnostics: %s\n', diagErr.message);
        end
        % Also check last error
        le = lasterror();
        if ~isempty(le.message)
            fprintf('Last error: %s\n', le.message);
        end
        break;
    end
    pause(1.0);
end

status = get_param(modelName, 'SimulationStatus');
if ~strcmp(status, 'stopped')
    fprintf('Stopping %s from status=%s\n', modelName, status);
    set_param(modelName, 'SimulationCommand', 'stop');
    pause(0.5);
end

fprintf('MATLAB runner complete, final status=%s\n', get_param(modelName, 'SimulationStatus'));

% Post-simulation test: send a test EscCmd to verify MATLAB->WSL DDS path
fprintf('Post-sim DDS test: publishing EscCmd...\n');
try
    testNode = ros2node('/post_sim_test_node');
    testPub = ros2publisher(testNode, '/uav/actuator/esc_cmd', 'flightcore_msgs/EscCmd');
    testMsg = ros2message(testPub);
    testMsg.motor_cmd = single([0.11, 0.22, 0.33, 0.44]);
    testMsg.mode = 'actuator';
    testMsg.valid = true;
    pause(1);  % wait for DDS discovery
    send(testPub, testMsg);
    fprintf('  Test EscCmd sent: motor_cmd=[%.2f %.2f %.2f %.2f]\n', ...
        testMsg.motor_cmd(1), testMsg.motor_cmd(2), ...
        testMsg.motor_cmd(3), testMsg.motor_cmd(4));
    pause(2);  % give DDS time to deliver
catch e2
    fprintf('  Post-sim pub failed: %s\n', e2.message);
end
fprintf('Post-sim test complete\n');
end
