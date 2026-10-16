function cfg = config_GatewayCommandAckTxRequestBus()
%CONFIG_GATEWAYCOMMANDACKTXREQUESTBUS Gateway 内部 COMMAND_ACK 编码请求。

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayCommandAckTxRequestBus';
cfg.description = 'Gateway 内部 MAVLink COMMAND_ACK 编码请求';
cfg.elements = {
    'Valid',            'boolean', 1, '', '请求有效';
    'command',          'uint16',  1, '', '外部 MAV_CMD';
    'result',           'uint8',   1, '', 'MAV_RESULT';
    'progress',         'uint8',   1, '', '第一阶段固定 0';
    'result_param2',    'int32',   1, '', '第一阶段固定 0';
    'target_system',    'uint8',   1, '', 'ACK 目标 SystemId';
    'target_component', 'uint8',   1, '', 'ACK 目标 ComponentId';
    };
end
