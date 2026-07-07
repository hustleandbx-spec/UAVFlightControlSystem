function test_runtime_truth_contract()
%TEST_RUNTIME_TRUTH_CONTRACT Verify truth data is consumed outside FlightCore.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
openProject(fullfile(projectRoot, 'FC_SimulinkProject.prj'));

load_system('SimAdapter');
load_system('FlightCore');
load_system('FlightCore_SimAdapter_loop');

globalTypesPath = fullfile(projectRoot, '1_Data_Dictionaries', 'GlobalTypes.sldd');
ddObj = Simulink.data.dictionary.open(globalTypesPath);
cleanupObj = onCleanup(@() close(ddObj)); %#ok<NASGU>
dData = getSection(ddObj, 'Design Data');
assert(dictionaryHasEntry(dData, 'RuntimeTruthBus'), 'GlobalTypes must define RuntimeTruthBus.');
assert(dictionaryHasEntry(dData, 'ExperimentTraceBus'), 'GlobalTypes must define ExperimentTraceBus.');

assert(~isempty(find_system('SimAdapter', 'SearchDepth', 2, 'Name', 'NetworkSensorInput')), ...
    'SimAdapter must contain NetworkSensorInput.');
assert(~isempty(find_system('SimAdapter', 'SearchDepth', 2, 'Name', 'ExperimentTrace')), ...
    'SimAdapter must contain ExperimentTrace consumer subsystem.');

simAdapterOutports = find_system('SimAdapter', 'SearchDepth', 1, 'BlockType', 'Outport');
simAdapterOutportNames = string(get_param(simAdapterOutports, 'Name'));
assert(any(simAdapterOutportNames == "ExperimentTraceBus"), ...
    'SimAdapter must expose ExperimentTraceBus for inspection/logging.');

flightCorePorts = [ ...
    string(get_param(find_system('FlightCore', 'SearchDepth', 1, 'BlockType', 'Inport'), 'Name')); ...
    string(get_param(find_system('FlightCore', 'SearchDepth', 1, 'BlockType', 'Outport'), 'Name'))];
assert(~any(contains(flightCorePorts, 'Truth')), ...
    'FlightCore boundary must not expose truth data.');
assert(~any(flightCorePorts == "ExperimentTraceBus"), ...
    'FlightCore boundary must not expose ExperimentTraceBus.');

loopLines = find_system('FlightCore_SimAdapter_loop', 'FindAll', 'on', 'SearchDepth', 1, 'Type', 'line');
lineNames = strings(size(loopLines));
for i = 1:numel(loopLines)
    lineNames(i) = string(get_param(loopLines(i), 'Name'));
end
assert(any(lineNames == "ExperimentTraceBus"), ...
    'FlightCore_SimAdapter_loop must contain visible ExperimentTraceBus line.');

traceLine = loopLines(find(lineNames == "ExperimentTraceBus", 1));
dstPorts = get_param(traceLine, 'DstPortHandle');
assert(numel(dstPorts) == 1, 'ExperimentTraceBus should have exactly one top-level consumer.');
dstBlock = get_param(dstPorts(1), 'Parent');
assert(endsWith(string(dstBlock), "/RealtimeMonitor"), ...
    'ExperimentTraceBus must feed RealtimeMonitor, not FlightCore, in the top-level integration model.');

assert(~isempty(find_system('FlightCore_SimAdapter_loop', 'SearchDepth', 1, 'Name', 'RealtimeMonitor')), ...
    'FlightCore_SimAdapter_loop must contain RealtimeMonitor.');
expectedDashboardBlocks = [
    "IMUValidLamp"
    "GPSValidLamp"
    "TraceValidLamp"
];
for i = 1:numel(expectedDashboardBlocks)
    assert(~isempty(find_system('FlightCore_SimAdapter_loop', ...
        'SearchDepth', 1, 'Name', char(expectedDashboardBlocks(i)))), ...
        'Missing realtime Dashboard block: %s', expectedDashboardBlocks(i));
end

realtimeOutports = find_system('FlightCore_SimAdapter_loop/RealtimeMonitor', ...
    'SearchDepth', 1, 'BlockType', 'Outport');
assert(numel(realtimeOutports) == 8, ...
    'RealtimeMonitor must expose 8 trace breakout signals.');

set_param('FlightCore_SimAdapter_loop', 'SimulationCommand', 'update');

disp('RUNTIME_TRUTH_CONTRACT_PASS');
end

function tf = dictionaryHasEntry(sectionObj, entryName)
try
    getEntry(sectionObj, entryName);
    tf = true;
catch
    tf = false;
end
end
