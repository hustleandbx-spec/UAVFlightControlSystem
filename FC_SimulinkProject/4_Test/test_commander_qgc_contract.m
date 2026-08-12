function test_commander_qgc_contract()
%TEST_COMMANDER_QGC_CONTRACT Verify the single official MAVLink gateway.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
commanderDir = fullfile(projectRoot, '2_Model', 'commander');
gatewayDir = fullfile(projectRoot, '3_Integration', 'MAVLink');
gatewayPath = fullfile(gatewayDir, 'Gateway.slx');
protocolPath = fullfile(gatewayDir, 'GatewayMavlinkProtocol.slx');

assert(isfile(gatewayPath), 'Missing Gateway.slx.');
assert(isfile(protocolPath), 'Missing GatewayMavlinkProtocol.slx.');
assert(isfile(fullfile(commanderDir, 'Commander.slx')), ...
    'Missing Commander.slx.');

obsoleteFiles = {
    'QgcMavlinkGateway.m'
    'qgcMavlinkGatewayVectorStep.m'
    'qgcMavlinkTransport.m'
    'missionPlanAdapter.m'
    };
for i = 1:numel(obsoleteFiles)
    assert(~isfile(fullfile(gatewayDir, obsoleteFiles{i})), ...
        'Obsolete parallel gateway remains: %s', obsoleteFiles{i});
end

load_system(protocolPath);
load_system(gatewayPath);
protocolCleanup = onCleanup(@() closeIfLoaded('GatewayMavlinkProtocol'));
gatewayCleanup = onCleanup(@() closeIfLoaded('Gateway'));

officialBlocks = find_system('GatewayMavlinkProtocol', ...
    'LookUnderMasks', 'all', 'FollowLinks', 'on', ...
    'RegExp', 'on', 'ReferenceBlock', '^uavmavlinklib/MAVLink ');
assert(~isempty(officialBlocks), ...
    'Gateway protocol must retain MathWorks MAVLink library blocks.');
officialReferences = string(get_param(officialBlocks, 'ReferenceBlock'));
assert(any(officialReferences == "uavmavlinklib/MAVLink Blank Message"), ...
    'Official MAVLink Blank Message block is missing.');
assert(any(officialReferences == "uavmavlinklib/MAVLink Serializer"), ...
    'Official MAVLink Serializer block is missing.');
assert(any(officialReferences == "uavmavlinklib/MAVLink Deserializer"), ...
    'Official MAVLink Deserializer block is missing.');

forbiddenProtocolNames = {
    'send_param'
    'Initialize Function'
    'Param read: GDNC_TSTAR'
    'Param read: GDNC_TURN_LEAD'
    };
for i = 1:numel(forbiddenProtocolNames)
    blocks = find_system('GatewayMavlinkProtocol', ...
        'LookUnderMasks', 'all', 'FollowLinks', 'on', ...
        'Name', forbiddenProtocolNames{i});
    assert(isempty(blocks), ...
        'Unused official demo block remains: %s', ...
        forbiddenProtocolNames{i});
end

displayBlocks = find_system('GatewayMavlinkProtocol', ...
    'LookUnderMasks', 'all', 'FollowLinks', 'on', 'BlockType', 'Display');
scopeBlocks = find_system('GatewayMavlinkProtocol', ...
    'LookUnderMasks', 'all', 'FollowLinks', 'on', 'BlockType', 'Scope');
assert(isempty(displayBlocks) && isempty(scopeBlocks), ...
    'Gateway protocol must not retain demo Display or Scope blocks.');

dataStores = find_system('GatewayMavlinkProtocol', ...
    'LookUnderMasks', 'all', 'FollowLinks', 'on', ...
    'BlockType', 'DataStoreMemory');
dataStoreNames = string(get_param(dataStores, 'DataStoreName'));
assert(isequal(sort(dataStoreNames), sort(["newCount"; "newItem"])), ...
    'Only mission upload event stores newCount/newItem may remain.');

protocolOutputs = string(get_param(find_system( ...
    'GatewayMavlinkProtocol', 'SearchDepth', 1, ...
    'BlockType', 'Outport'), 'Name'));
expectedOutputs = ["MissionItems"; "MissionCount"; "MissionAvailable"; ...
    "CommandLongNew"; "RxStatus"; "CommandLong"];
assert(isequal(protocolOutputs, expectedOutputs), ...
    'Official protocol boundary does not match the Gateway contract.');

gatewayReferences = find_system('Gateway', 'SearchDepth', 1, ...
    'BlockType', 'ModelReference', ...
    'ModelName', 'GatewayMavlinkProtocol');
assert(numel(gatewayReferences) == 1, ...
    'Gateway must contain exactly one official MAVLink protocol reference.');

protocolPorts = get_param(gatewayReferences{1}, 'PortHandles');
adapterPorts = get_param('Gateway/ProtocolToFlightCore', 'PortHandles');
commandPorts = get_param('Gateway/CommandLongFields', 'PortHandles');
assert(isempty(find_system('Gateway', 'SearchDepth', 1, ...
    'Name', 'MissionItemFields')), ...
    'Legacy scalar MissionItems Bus Selector must not remain.');
assertDestination(protocolPorts.Outport(1), adapterPorts.Inport(5), ...
    'MissionItems');
assertDestination(protocolPorts.Outport(4), adapterPorts.Inport(4), ...
    'CommandLongNew');
assertDestination(protocolPorts.Outport(5), adapterPorts.Inport(3), ...
    'RxStatus');
assertDestination(protocolPorts.Outport(6), commandPorts.Inport(1), ...
    'CommandLong');
assert(strcmp(get_param('Gateway', 'FixedStep'), '0.01'), ...
    'Gateway fixed step must support the official protocol 0.01 s rate.');
assert(strcmp(get_param('Gateway', ...
    'ModelReferenceNumInstancesAllowed'), 'Single'), ...
    'Gateway must be single-instance because it owns the UDP transport.');

commanderText = extractAllSlxSystemText(fullfile( ...
    commanderDir, 'Commander.slx'));
assert(~contains(commanderText, 'MAVLink') && ...
    ~contains(commanderText, 'UDP'), ...
    'Commander must remain protocol and transport neutral.');

fprintf('COMMANDER_QGC_OFFICIAL_GATEWAY_CONTRACT_PASS\n');
clear gatewayCleanup protocolCleanup;
end

function assertDestination(sourcePort, expectedDestination, description)
lineHandle = get_param(sourcePort, 'Line');
assert(lineHandle ~= -1, '%s is unconnected.', description);
destinations = get_param(lineHandle, 'DstPortHandle');
assert(any(destinations == expectedDestination), ...
    '%s is connected to the wrong destination.', description);
end

function closeIfLoaded(modelName)
if bdIsLoaded(modelName)
    close_system(modelName, 0);
end
end

function text = extractAllSlxSystemText(slxPath)
tmpDir = tempname;
mkdir(tmpDir);
cleanup = onCleanup(@() rmdir(tmpDir, 's'));
unzip(slxPath, tmpDir);
rootFile = fullfile(tmpDir, 'simulink', 'blockdiagram.xml');
systemFiles = dir(fullfile(tmpDir, 'simulink', 'systems', '*.xml'));
parts = strings(numel(systemFiles) + 1, 1);
parts(1) = string(fileread(rootFile));
for i = 1:numel(systemFiles)
    parts(i + 1) = string(fileread(fullfile( ...
        systemFiles(i).folder, systemFiles(i).name)));
end
text = join(parts, newline);
clear cleanup;
end
