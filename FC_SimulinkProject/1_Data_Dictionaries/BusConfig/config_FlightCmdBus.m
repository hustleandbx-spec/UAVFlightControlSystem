function cfg = config_FlightCmdBus()
% 总线名称：FlightCmdBus
% 描述：飞控统一命令接口，隔离控制器与具体命令来源

cfg.busName = 'FlightCmdBus';
cfg.description = '飞控统一命令接口（位置、速度、航向、模式和有效标志）';
cfg.elements = {
    % {Name,               DataType,  Dimensions, Unit,    Description}
    'Position_NED_SP',     'single',  3,          'm',     'NED位置目标';
    'Velocity_NED_SP',     'single',  3,          'm/s',   'NED速度前馈目标';
    'Yaw_SP',              'single',  1,          'rad',   '航向目标';
    'Mode',                'uint8',   1,          '',      '命令模式（1=位置保持）';
    'Valid',               'boolean', 1,          '',      '命令有效标志'
    };
end
