function cfg = config_MissionItemsBus()
cfg.contractDomain = 'Mission';
cfg.busName = 'MissionItemsBus';
cfg.description = 'Standard MAVLink task items; no private file encoding';
cfg.elements = {
    'TransferId', 'uint32', 1, '', 'Upload identity';
    'Count', 'uint16', 1, '', 'Item count, max 400';
    'Command', 'uint16', 400, '', 'MAV_CMD by zero-based seq';
    'Frame', 'uint8', 400, '', 'MAV_FRAME';
    'Params', 'single', [400 4], '', 'param1 to param4';
    'X', 'int32', 400, '', 'Latitude E7';
    'Y', 'int32', 400, '', 'Longitude E7';
    'Z', 'single', 400, 'm', 'Altitude';
    'AutoContinue', 'uint8', 400, '', 'autocontinue';
    'Ready', 'boolean', 1, '', 'Complete upload event';
    };
end
