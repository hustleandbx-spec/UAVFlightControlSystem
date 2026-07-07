function cfg = config_StateEstBus()
% 总线名称：StateEstBus
% 描述：ESKF输出给飞控的导航状态估计

cfg.busName = 'StateEstBus';
cfg.description = 'ESKF导航状态估计（位置、速度、姿态、偏置等）';
cfg.elements = {
    % {Name,               DataType, Dimensions, Unit,    Description}
    'Position_NED',        'single',   3,        'm',     '位置估计（北东地）';
    'Velocity_NED',        'single',   3,        'm/s',   '速度估计（北东地）';
    'Attitude_quat',       'single',   4,        '',      '姿态四元数 [w x y z]';
    'AngularRate_Body',    'single',   3,        'rad/s', '机体角速度估计';
    'Accel_Body',          'single',   3,        'm/s^2', '机体加速度估计（不含重力）';
    'GyroBias',            'single',   3,        'rad/s', '陀螺零偏估计';
    'AccelBias',           'single',   3,        'm/s^2', '加速度计零偏估计';
    'Wind_NED',            'single',   3,        'm/s',   '风速估计（北东地）';
    'Status',              'uint8',    1,        '',      '估计器状态（0未初始化,1稳定,2错误）'
    };
end