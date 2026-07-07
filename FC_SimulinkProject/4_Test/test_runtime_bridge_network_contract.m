function test_runtime_bridge_network_contract()
%TEST_RUNTIME_BRIDGE_NETWORK_CONTRACT Verify realtime bridge files and model boundary.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
repoRoot = fileparts(projectRoot);
runtimeBridgeDir = fullfile(projectRoot, '3_Integration', 'RuntimeBridge');

requiredFiles = {
    fullfile(runtimeBridgeDir, 'bridge_config.yaml')
    fullfile(runtimeBridgeDir, 'protocol', 'sensor_frame.py')
    fullfile(runtimeBridgeDir, 'protocol', 'command_frame.py')
    fullfile(runtimeBridgeDir, 'protocol', 'shadow_frame.py')
    fullfile(runtimeBridgeDir, 'adapters', 'dummy_realtime_bridge.py')
    fullfile(runtimeBridgeDir, 'adapters', 'airsim_realtime_bridge.py')
    fullfile(runtimeBridgeDir, 'adapters', 'shadow_logger.py')
};

for i = 1:numel(requiredFiles)
    assert(isfile(requiredFiles{i}), 'Missing RuntimeBridge file: %s', requiredFiles{i});
end

assert(contains(fileread(fullfile(runtimeBridgeDir, 'protocol', 'sensor_frame.py')), ...
    'SENSOR_FRAME_MAGIC'), 'sensor_frame.py must define SENSOR_FRAME_MAGIC.');
assert(contains(fileread(fullfile(runtimeBridgeDir, 'protocol', 'command_frame.py')), ...
    'COMMAND_FRAME_MAGIC'), 'command_frame.py must define COMMAND_FRAME_MAGIC.');
assert(contains(fileread(fullfile(runtimeBridgeDir, 'protocol', 'shadow_frame.py')), ...
    'SHADOW_FRAME_MAGIC'), 'shadow_frame.py must define SHADOW_FRAME_MAGIC.');
assert(contains(fileread(fullfile(runtimeBridgeDir, 'adapters', 'shadow_logger.py')), ...
    'ShadowFrameStats'), 'shadow_logger.py must define ShadowFrameStats.');

openProject(fullfile(projectRoot, 'FC_SimulinkProject.prj'));
load_system('SimAdapter');
load_system('FlightCore_SimAdapter_loop');

assert(~isempty(find_system('SimAdapter', 'SearchDepth', 2, 'Name', 'NetworkSensorInput')), ...
    'SimAdapter must contain NetworkSensorInput subsystem.');
assert(~isempty(find_system('SimAdapter', 'SearchDepth', 2, 'Name', 'NetworkCommandInput')), ...
    'SimAdapter must contain NetworkCommandInput subsystem.');
assert(~isempty(find_system('SimAdapter', 'SearchDepth', 2, 'Name', 'ShadowOutput')), ...
    'SimAdapter must contain ShadowOutput subsystem.');

simAdapterInports = find_system('SimAdapter', 'SearchDepth', 1, 'BlockType', 'Inport');
simAdapterOutports = find_system('SimAdapter', 'SearchDepth', 1, 'BlockType', 'Outport');
simAdapterPorts = [numel(simAdapterInports), numel(simAdapterOutports)];
assert(isequal(simAdapterPorts, [2 4]), ...
    'SimAdapter must expose 2 inputs and 4 outputs after experiment trace integration.');

loopLines = find_system('FlightCore_SimAdapter_loop', 'FindAll', 'on', 'Type', 'line');
lineNames = strings(size(loopLines));
for i = 1:numel(loopLines)
    lineNames(i) = string(get_param(loopLines(i), 'Name'));
end

expectedLineNames = ["IMU_BUS", "GPS_BUS", "FlightCmdBus", "EscCmdBus", "StateEstBus", "ExperimentTraceBus"];
for i = 1:numel(expectedLineNames)
    assert(any(lineNames == expectedLineNames(i)), ...
        'FlightCore_SimAdapter_loop missing line: %s', expectedLineNames(i));
end

disp('RUNTIME_BRIDGE_NETWORK_CONTRACT_PASS');
end
