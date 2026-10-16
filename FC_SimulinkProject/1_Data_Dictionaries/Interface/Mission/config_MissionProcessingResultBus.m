function cfg = config_MissionProcessingResultBus()
cfg.contractDomain = 'Mission';
cfg.busName = 'MissionProcessingResultBus';
cfg.description = 'MM acceptance result for Mission protocol';
cfg.elements = {
    'TransferId', 'uint32', 1, '', 'Upload identity';
    'PlanId', 'uint32', 1, '', 'Accepted plan id';
    'Result', 'uint8', 1, '', 'MAV_MISSION_RESULT';
    'Accepted', 'boolean', 1, '', 'Plan usable';
    'Valid', 'boolean', 1, '', 'Processing completed';
    };
end
