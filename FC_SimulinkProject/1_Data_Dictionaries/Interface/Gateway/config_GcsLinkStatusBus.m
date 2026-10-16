function cfg = config_GcsLinkStatusBus()
%CONFIG_GCSLINKSTATUSBUS 契约 #7：链路状态（遥测/起飞门 SYS-REQ-008 输入）。

cfg.busName = 'GcsLinkStatusBus';
cfg.contractDomain = 'Gateway';
cfg.description = 'Gateway 发布的授权 GCS 协议连接事实';
cfg.elements = {
    % {Name,              DataType, Dimensions, Unit, Description}
    'Connected',     'boolean', 1, '',  '地面站已连接';
    'Sequence',      'uint32',  1, '',  '快照序号';
    'Timestamp',     'double',  1, 's', '快照时间戳';
    'Valid',         'boolean', 1, '',  '链路状态样本已初始化';
    };
end
