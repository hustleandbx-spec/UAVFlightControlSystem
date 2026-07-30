function cfg = config_EscCmdBus()
% 总线名称：EscCmdBus
% 描述：飞行控制系统输出给动力系统的电调控制指令

cfg.busName = 'EscCmdBus';
cfg.description = '飞控至动力系统的电调控制指令';
cfg.elements = {
    % {Name,       DataType, Dimensions, Unit,   Description}
    'MotorCmd',    'single',   4,        '1', '各电机油门指令（归一化）';
    %'CmdType',     'uint8',    1,        '',     '指令类型（0:油门, 1:转速, 2:推力...）';
    'Armed',       'boolean',  1,        '',   '执行器解锁状态，false时必须输出disarmed安全值';
    'Valid',       'boolean',  1 ,        '',   '当前执行器指令是否有效，false时禁止保持旧命令'
    };
end