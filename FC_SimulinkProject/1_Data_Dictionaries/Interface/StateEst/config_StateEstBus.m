function cfg = config_StateEstBus()
% 总线名称：StateEstBus
% 描述：ESKF输出给飞控的导航状态估计

cfg.busName = 'StateEstBus';
cfg.contractDomain = 'StateEst';
cfg.description = '状态估计主数据（控制反馈路径）';
cfg.elements = {
    % {Name,               DataType, Dimensions, Unit,    Description}
    'Position_NED',        'single',   3,        'm',     '位置估计（北东地）';
    'Velocity_NED',        'single',   3,        'm/s',   '速度估计（北东地）';
    'Attitude_quat',       'single',   4,        '',      '姿态四元数 [w x y z]';
    'AngularRate_Body',    'single',   3,        'rad/s', '机体角速度估计';
    'Sequence',            'uint32',   1,        '',      '主数据序号';
    'Timestamp',           'double',   1,        's',     '估计时间戳';
    'Valid',               'boolean',  1,        '',      '主数据完整有效';
    };
end
