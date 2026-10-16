function cfg = config_PropulsionStatusBus()
%CONFIG_PROPULSIONSTATUSBUS 动力就绪状态权威快照。
% 动力/执行机构边界生产，Commander 和 Logging 只读消费。

cfg.contractDomain = 'Control';
cfg.busName = 'PropulsionStatusBus';
cfg.description = '动力就绪状态权威快照';
cfg.elements = {
    'PropulsionReady', 'boolean', 1, '',  '动力及执行机构已就绪';
    'Sequence',        'uint32',  1, '',  '快照序号';
    'Timestamp',       'double',  1, 's', '快照时间戳';
    'Valid',           'boolean', 1, '',  '快照完整有效';
    };
end
