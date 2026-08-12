function test_commander_command_audit()
%TEST_COMMANDER_COMMAND_AUDIT Verify the Commander admission function.
%
% model_test was attempted first, but the current harness generator cannot
% create a test file for multiple aliases of one vector port. This test calls
% the exact function used by Commander/CommandAudit; it does not duplicate
% the audit algorithm.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
addpath(fullfile(projectRoot, '2_Model', 'commander'));

commanderCommandAuditStep('reset');

readyMission = baseInput();
readyMission(3) = 1;
readyMission(5) = 9;
readyMission(6) = 0;
readyMission(7) = 3;
readyMission(8) = 1;
readyMission(13) = 101;
readyMission(14) = 255;
readyMission(15) = 9;
readyMission(16) = 1;
readyMission(19) = 1;
output = commanderCommandAuditStep(readyMission);
assert(output(12) == 1 && output(14) == 1, ...
    'Idle loaded mission start was not accepted.');

activeMission = readyMission;
activeMission(6) = 1;
activeMission(13) = 102;
activeMission(16) = 2;
output = commanderCommandAuditStep(activeMission);
assert(output(12) == 1 && output(14) == 1, ...
    'Active mission pause was not accepted.');

suspendedMission = readyMission;
suspendedMission(6) = 2;
suspendedMission(13) = 103;
suspendedMission(16) = 3;
output = commanderCommandAuditStep(suspendedMission);
assert(output(12) == 1 && output(14) == 1, ...
    'Suspended mission resume was not accepted.');

unsupportedTakeoff = readyMission;
unsupportedTakeoff(13) = 104;
unsupportedTakeoff(16) = 6;
unsupportedTakeoff(18) = 10;
output = commanderCommandAuditStep(unsupportedTakeoff);
assert(output(12) == 0 && output(14) == 2, ...
    'Unsupported direct takeoff action was released.');

foreignCancel = activeMission;
foreignCancel(13) = 105;
foreignCancel(14) = 42;
foreignCancel(16) = 4;
output = commanderCommandAuditStep(foreignCancel);
assert(output(12) == 0 && output(14) == 2, ...
    'Foreign-source cancel command was released.');

fprintf('COMMANDER_COMMAND_AUDIT_PASS\n');
end

function input = baseInput()
input = zeros(1, 19);
input(1) = 1;
input(2) = 255;
input(4) = 1;
end
