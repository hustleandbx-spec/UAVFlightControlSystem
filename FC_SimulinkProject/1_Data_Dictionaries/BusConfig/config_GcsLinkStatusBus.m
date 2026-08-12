function cfg = config_GcsLinkStatusBus()
%CONFIG_GCSLINKSTATUSBUS 契约 #7：链路状态（遥测/起飞门 SYS-REQ-008 输入）。

cfg.busName = 'GcsLinkStatusBus';
cfg.description = '协议无关的地面站链路状态';
cfg.elements = {
    % {Name,              DataType, Dimensions, Unit, Description}
    'SourceId',      'uint32',  1, '',  '地面站内部源标识';
    'RxSequence',    'uint32',  1, '',  '接收事件单调序号';
    'LastRxTimeSec', 'double',  1, 's', '最近有效事件接收时刻';
    'LinkQuality',   'single',  1, '',  '归一化链路质量（0~1）';
    'Connected',     'boolean', 1, '',  '地面站已连接';
    'Valid',         'boolean', 1, '',  '链路状态样本已初始化';
    };
end
