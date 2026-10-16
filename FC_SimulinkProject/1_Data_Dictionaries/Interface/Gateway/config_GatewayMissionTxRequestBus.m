function cfg = config_GatewayMissionTxRequestBus()
cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayMissionTxRequestBus';
cfg.description = 'Standard Mission response projection';
cfg.elements = {
    'Kind', 'uint8', 1, '', '1 request,2 count,3 ack,4 item,5 current,6 reached';
    'target_system', 'uint8', 1, '', 'target_system';
    'target_component', 'uint8', 1, '', 'target_component';
    'seq', 'uint16', 1, '', 'seq';
    'count', 'uint16', 1, '', 'count';
    'command', 'uint16', 1, '', 'command';
    'frame', 'uint8', 1, '', 'frame';
    'current', 'uint8', 1, '', 'current';
    'autocontinue', 'uint8', 1, '', 'autocontinue';
    'param1', 'single', 1, '', 'param1';
    'param2', 'single', 1, '', 'param2';
    'param3', 'single', 1, '', 'param3';
    'param4', 'single', 1, '', 'param4';
    'x', 'int32', 1, '', 'x';
    'y', 'int32', 1, '', 'y';
    'z', 'single', 1, '', 'z';
    'type', 'uint8', 1, '', 'type';
    'mission_type', 'uint8', 1, '', 'mission_type';
    };
end
