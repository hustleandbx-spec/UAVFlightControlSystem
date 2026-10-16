function cfg = config_GatewayHeartbeatTxRequestBus()
%CONFIG_GATEWAYHEARTBEATTXREQUESTBUS Gateway 内部 HEARTBEAT 编码请求。

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayHeartbeatTxRequestBus';
cfg.description = 'Gateway 内部 MAVLink HEARTBEAT 编码请求';
cfg.elements = {
    'Valid',         'boolean', 1, '', '请求有效';
    'custom_mode',   'uint32',  1, '', 'MAVLink custom_mode';
    'type',          'uint8',   1, '', 'MAV_TYPE';
    'autopilot',     'uint8',   1, '', 'MAV_AUTOPILOT';
    'base_mode',     'uint8',   1, '', 'MAV_MODE_FLAG';
    'system_status', 'uint8',   1, '', 'MAV_STATE';
    };
end
