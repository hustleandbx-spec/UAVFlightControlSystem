function cfg = config_StateEstHealthBus()
%CONFIG_STATEESTHEALTHBUS 状态估计健康权威快照。
% 业务字段按 Interface Definition §7.2 与 Commander Detailed Design §1.2
% 拆分，避免 Commander 从单一枚举值重新推导四项健康事实。

cfg.contractDomain = 'StateEst';
cfg.busName = 'StateEstHealthBus';
cfg.description = '状态估计器健康权威快照';
cfg.elements = {
    'PositionEstimateValid', 'boolean', 1, '',  '位置估计可用';
    'HeadingEstimateValid',  'boolean', 1, '',  '航向估计可用';
    'SensorsHealthy',        'boolean', 1, '',  '估计所需传感器健康';
    'EstimatorHealthy',      'boolean', 1, '',  '估计器内部状态健康';
    'Sequence',              'uint32',  1, '',  '快照序号';
    'Timestamp',             'double',  1, 's', '快照时间戳';
    'Valid',                 'boolean', 1, '',  '快照完整有效';
    };
end
