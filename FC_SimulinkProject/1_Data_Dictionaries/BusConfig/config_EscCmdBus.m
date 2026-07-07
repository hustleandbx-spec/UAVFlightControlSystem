function cfg = config_EscCmdBus()
% 总线名称：EscCmdBus
% 描述：飞行控制系统输出给动力系统的电调控制指令

cfg.busName = 'EscCmdBus';
cfg.description = '飞控至动力系统的电调控制指令';
cfg.elements = {
    % {Name,       DataType, Dimensions, Unit,   Description}
    'MotorCmd',    'single',   4,        '0..1', '各电机油门指令（归一化）';
    %'CmdType',     'uint8',    1,        '',     '指令类型（0:油门, 1:转速, 2:推力...）';
    %'Enable',      'uint8',    1,        '',     '使能信号（0锁定,1运行）'
    };
end