function output = commanderCommandAuditStep(input)
%COMMANDERCOMMANDAUDITSTEP Audit QGC commands inside Commander.
%
% The simulation-facing Commander model uses a fixed double-vector boundary.
% This function performs protocol-neutral command admission only. It does not
% plan missions, convert GPS coordinates, or generate navigation objectives.
%
% Input layout:
%   1 LinkConnected            11 CommanderRequestType
%   2 LinkSourceId             12 CommanderRequestValid
%   3 Armed                    13 MissionRequestId
%   4 ControlOwner             14 MissionSourceId
%   5 MissionStatusPlanId      15 MissionPlanId
%   6 MissionExecutionPhase    16 MissionAction
%   7 MissionItemCount         17 TargetItemIndex
%   8 MissionStatusValid       18 TargetAltitude_m
%   9 CommanderRequestId       19 MissionRequestValid
%  10 CommanderSourceId
%
% Output layout:
%   1..5  approved CommanderRequest fields
%   6..12 approved MissionControlRequest fields
%   13    audited unified RequestId
%   14    audit result: 0=none, 1=accepted, 2=denied

persistent lastCommanderRequestId lastMissionRequestId

if ischar(input) || (isstring(input) && isscalar(input))
    if strcmp(char(input), 'reset')
        lastCommanderRequestId = intmax('uint32');
        lastMissionRequestId = intmax('uint32');
        output = zeros(1, 14);
        return
    end
    error('CommanderCommandAuditStep:UnknownAction', ...
        'Unknown action: %s', char(input));
end

if isempty(lastCommanderRequestId)
    lastCommanderRequestId = intmax('uint32');
    lastMissionRequestId = intmax('uint32');
end

if numel(input) ~= 19
    error('CommanderCommandAuditStep:InvalidInputWidth', ...
        'Expected 19 input elements, received %d.', numel(input));
end

linkConnected = logical(input(1));
linkSourceId = uint32(input(2));
armed = logical(input(3));
controlOwner = uint8(input(4));
missionStatusPlanId = uint32(input(5));
missionExecutionPhase = uint8(input(6));
missionItemCount = uint8(input(7));
missionStatusValid = logical(input(8));
commanderRequestId = uint32(input(9));
commanderSourceId = uint32(input(10));
commanderRequestType = uint8(input(11));
commanderRequestValid = logical(input(12));
missionRequestId = uint32(input(13));
missionSourceId = uint32(input(14));
missionPlanId = uint32(input(15));
missionAction = uint8(input(16));
targetItemIndex = uint32(input(17));
targetAltitudeM = single(input(18));
missionRequestValid = logical(input(19));

approvedCommander = zeros(1, 5);
approvedMission = zeros(1, 7);
auditRequestId = uint32(0);
auditResult = uint8(0);

commonCommanderValid = linkConnected && controlOwner == uint8(1) && ...
    commanderSourceId ~= uint32(0) && commanderSourceId == linkSourceId;
commonMissionValid = linkConnected && controlOwner == uint8(1) && ...
    missionSourceId ~= uint32(0) && missionSourceId == linkSourceId;

isNewCommander = commanderRequestValid && ...
    commanderRequestId ~= lastCommanderRequestId;
isNewMission = missionRequestValid && ...
    missionRequestId ~= lastMissionRequestId;

if isNewCommander
    lastCommanderRequestId = commanderRequestId;
    auditRequestId = commanderRequestId;
    supported = commanderRequestType >= uint8(3) && ...
        commanderRequestType <= uint8(5);
    stateAllowed = commanderRequestType ~= uint8(5) || armed;
    accepted = commonCommanderValid && supported && stateAllowed;
    if accepted
        approvedCommander = double([commanderRequestId, ...
            commanderSourceId, commanderRequestType, uint8(0), true]);
        auditResult = uint8(1);
    else
        auditResult = uint8(2);
    end
elseif isNewMission
    lastMissionRequestId = missionRequestId;
    auditRequestId = missionRequestId;
    planMatches = missionPlanId == uint32(0) || ...
        missionPlanId == missionStatusPlanId;
    switch missionAction
        case uint8(1) % Start
            actionAllowed = armed && missionStatusValid && ...
                missionExecutionPhase == uint8(0) && ...
                missionItemCount > uint8(0) && planMatches;
        case uint8(2) % Pause
            actionAllowed = armed && missionStatusValid && ...
                missionExecutionPhase == uint8(1) && planMatches;
        case uint8(3) % Resume
            actionAllowed = armed && missionStatusValid && ...
                missionExecutionPhase == uint8(2) && planMatches;
        case uint8(4) % Cancel
            actionAllowed = missionStatusValid && ...
                (missionExecutionPhase == uint8(1) || ...
                missionExecutionPhase == uint8(2)) && planMatches;
        otherwise
            actionAllowed = false;
    end

    accepted = commonMissionValid && actionAllowed;
    if accepted
        approvedMission = double([missionRequestId, missionSourceId, ...
            missionPlanId, missionAction, targetItemIndex, ...
            targetAltitudeM, true]);
        auditResult = uint8(1);
    else
        auditResult = uint8(2);
    end
end

output = double([approvedCommander, approvedMission, ...
    auditRequestId, auditResult]);
end
