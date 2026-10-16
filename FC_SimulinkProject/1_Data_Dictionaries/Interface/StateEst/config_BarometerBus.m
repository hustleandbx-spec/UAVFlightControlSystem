function cfg = config_BarometerBus()
% 总线名称：BarometerBus
% 描述：气压计测量帧（含有效性、新样本与采样时间戳）
% 供 StateEstimation 消费（中速直连，DEC-105；测量输入）。
%
% 物理权威 = 静压与温度。高度是派生物，不在本总线内重复表达，
% 由 StateEstimation 依 ISA/QNH 参考换算，避免同一信息两个表示。
%
% 量产传感器误差机理（偏置、噪声密度、bias instability、失锁衰减）
% 由 sensor_BARO_model 内部承载，不在本契约字段中展开。

cfg.busName = 'BarometerBus';
cfg.contractDomain = 'StateEst';
cfg.description = '气压计测量帧（静压/温度 + 有效性与采样时间戳）';
cfg.elements = {
    % {Name,          DataType, Dimensions, Unit,   Description}
    'Pressure',      'single',  1,        'Pa',   '静压（气压计原始物理量）';
    'Temperature',   'single',  1,        'degC', '传感器温度（用于压力补偿）';
    'Sequence',      'uint32',  1,        '',     '测量帧序号';
    'Timestamp',     'double',  1,        's',    '测量采样时间戳';
    'IsNew',         'boolean', 1,        '',     '本帧为新样本（仅一拍为真）';
    'Valid',         'boolean', 1,        '',     '测量帧有效标志（帧完整性，非质量等级）';
    };
end
