function cfg = config_MagnetometerBus()
% 总线名称：MagnetometerBus
% 描述：磁力计测量帧（机体磁场 + 磁航向 + 有效性与采样时间戳）
% 供 StateEstimation 消费（中速直连，DEC-105；测量输入）。
%
% Mag_Body 为机体坐标系磁场，单位 uT（工业磁力计常用单位）；
% 参考场（WMM/IGRF）以 nT 给出，在 sensor_MAG_model 边界统一换算为 uT。
%
% Heading_Mag 为倾斜补偿后的磁航向（rad，NED 约定）。
% 它与 Mag_Body 同源、不是第二测量源：Mag_Body 是权威物理量，
% Heading_Mag 是该帧的派生便利量，HeadingValid 单独表达航向可用性
% （例如倾斜超限或场强异常时为假），与 Valid（帧完整性）分离。
%
% 硬磁/软磁/安装失准由 sensor_MAG_model 内部承载，不在契约字段中展开。

cfg.busName = 'MagnetometerBus';
cfg.contractDomain = 'StateEst';
cfg.description = '磁力计测量帧（机体磁场/磁航向 + 有效性与采样时间戳）';
cfg.elements = {
    % {Name,          DataType, Dimensions, Unit,   Description}
    'Mag_Body',      'single',  3,        'uT',   '机体坐标系磁场矢量';
    'Heading_Mag',   'single',  1,        'rad',  '倾斜补偿磁航向（NED）';
    'HeadingValid',  'boolean', 1,        '',     '磁航向可用（倾斜/场强判据通过）';
    'Declination',   'single',  1,        'rad',  '该帧参考场磁偏角（真航向 = 磁航向 + 磁偏角）';
    'Sequence',      'uint32',  1,        '',     '测量帧序号';
    'Timestamp',     'double',  1,        's',    '测量采样时间戳';
    'IsNew',         'boolean', 1,        '',     '本帧为新样本（仅一拍为真）';
    'Valid',         'boolean', 1,        '',     '测量帧有效标志（帧完整性，非质量等级）';
    };
end
