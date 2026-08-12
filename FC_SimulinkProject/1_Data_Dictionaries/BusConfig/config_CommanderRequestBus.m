function cfg = config_CommanderRequestBus()
%CONFIG_COMMANDERREQUESTBUS 契约 #6（DEC-066/089）：GCS 写命令请求（Gateway → Commander 统一命令入口）。
% Commander 审查（CMD-002）后逐条 Ack（CommandAckBus，DEC-089/090 多事件时序）。
% 命令集对齐 DEC-066/CMD-001：加锁/解锁、开始任务、返航、降落、悬停、接管、go-to-point 激活、载荷控制(后置)。

cfg.busName = 'CommanderRequestBus';
cfg.description = 'GCS 写命令请求（协议无关，Gateway → Commander）';
cfg.elements = {
    % {Name,       DataType, Dimensions, Unit, Description}
    'CommandId',   'uint32',  1,    '',  '命令关联号（单调递增，CommandAckBus 回显）';
    'Command',     'uint8',   1,    '',  '1=加锁, 2=解锁, 3=开始任务, 4=返航, 5=降落, 6=悬停, 7=接管, 8=go-to-point, 9=载荷控制(后置)';
    'Params',      'single',  8,    '',  '命令参数（如 go-to-point 目标 NED）；未用槽位置 0';
    'Valid',       'boolean', 1,    '',  '请求字段已完成校验（不等于 Commander 已授权执行）';
    };
end
