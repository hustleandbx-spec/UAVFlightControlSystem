function cfg = config_ThrustTorqueBus()
% 总线名称：ThrustTorqueBus
% 描述：动力系统输出给动力学模型的力与力矩

cfg.busName = 'ThrustTorqueBus';
cfg.description = '动力系统输出的合力与合力矩（机体坐标系）';
cfg.elements = {
    % {Name,             DataType, Dimensions, Unit,   Description}
    'Thrust_Body',       'single',   3,        'N',    '机体推力 [Fx, Fy, Fz]';
    'Torque_Body',       'single',   3,        'N*m',  '机体力矩 [Mx, My, Mz]';
    % 'ThrottleFeedback',  'single',   4,        '0..1', '各通道实际输出（用于监测）'
    };
end