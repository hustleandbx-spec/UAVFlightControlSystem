function cfg = config_TrajectorySetpointBus()
%CONFIG_TRAJECTORYSETPOINTBUS 契约 #2（DEC-086）：Navigator→FlightControl 连续轨迹参考（直连强时序）。
% 无加速度前馈、偏航单量（DEC-063）；Valid = 运行时有效性（正常流恒 true，最差=悬停保持）。

cfg.busName = 'TrajectorySetpointBus';
cfg.description = 'Navigator 输出的连续轨迹参考（位置/速度/偏航，协议无关）';
cfg.elements = {
    % {Name,              DataType, Dimensions, Unit,  Description}
    'Position_NED_SP',    'single',  3,        'm',   '局部 NED 位置参考';
    'Velocity_NED_SP',    'single',  3,        'm/s', '局部 NED 速度参考';
    'Yaw_SP',             'single',  1,        'rad', '偏航参考';
    'Valid',              'boolean', 1,        '',    '完整已授权轨迹参考';
    };
end
