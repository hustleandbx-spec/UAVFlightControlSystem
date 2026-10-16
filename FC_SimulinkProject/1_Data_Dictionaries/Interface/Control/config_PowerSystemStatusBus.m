function cfg = config_PowerSystemStatusBus()
%CONFIG_POWERSYSTEMSTATUSBUS 能源充足状态权威快照。
% power_system 生产，Commander 和 Logging 只读消费。

cfg.contractDomain = 'Control';
cfg.busName = 'PowerSystemStatusBus';
cfg.description = '能源充足状态权威快照';
cfg.elements = {
    'EnergySufficient', 'boolean', 1, '',  '能源裕度满足当前任务';
    'Sequence',         'uint32',  1, '',  '快照序号';
    'Timestamp',        'double',  1, 's', '快照时间戳';
    'Valid',            'boolean', 1, '',  '快照完整有效';
    };
end
