function cfg = config_GatewayCommandEnvelopeBus()
%CONFIG_GATEWAYCOMMANDENVELOPEBUS Gateway 内部命令协议统一包络。

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayCommandEnvelopeBus';
cfg.description = 'COMMAND_LONG/COMMAND_INT 解码后的 Gateway 内部统一包络';
cfg.elements = {
    'IsNew',            'boolean', 1, '', '本帧包含新命令';
    'CarrierType',      'uint8',   1, '', '1=COMMAND_LONG，2=COMMAND_INT';
    'SystemID',         'uint8',   1, '', 'MAVLink 来源 SystemId';
    'ComponentID',      'uint8',   1, '', 'MAVLink 来源 ComponentId';
    'Sequence',         'uint8',   1, '', 'MAVLink 入站序号';
    'target_system',    'uint8',   1, '', '目标 SystemId';
    'target_component', 'uint8',   1, '', '目标 ComponentId';
    'command',          'uint16',  1, '', 'MAV_CMD 数值';
    'frame',            'uint8',   1, '', 'COMMAND_INT 坐标系；LONG 为 0';
    'Params',           'double',  8, '', '统一参数向量';
    'ParamValidMask',   'uint16',  1, '', 'Params 有效位';
    };
end
