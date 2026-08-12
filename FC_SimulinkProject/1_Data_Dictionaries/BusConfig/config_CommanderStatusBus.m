function cfg = config_CommanderStatusBus()
%CONFIG_COMMANDERSTATUSBUS 契约 #6（CMD-006）：Commander 模块状态流。
% 第一阶段实现 = Armed / CurrentMode / LastCommandResult（DEC-108 顶层状态 + DEC-089 Ack 缓存）；
% SafetyState / SafetyDirective / MissionDirective / FailsafeActive = 安全横切（DEC-057）/ FailSafe（DEC-060）
% 后置占位——标"已定义未落地"，第一阶段（DEC-111）不实现安全处置。

cfg.busName = 'CommanderStatusBus';
cfg.description = 'Commander 运行状态流（契约 #6）';
cfg.elements = {
    % {Name,               DataType, Dimensions, Unit, Description}
    'Armed',             'boolean', 1, '', '是否加锁（DEC-103）';
    'CurrentMode',       'uint8',   1, '', '0=待机, 1=任务执行, 2=返航, 3=降落, 4=悬停';
    'LastCommandResult', 'uint8',   1, '', '最近命令结果缓存：0=已批准, 1=已拒绝, 2=已中止（回显 CommandAckBus）';
    'LastCommandId',     'uint32',  1, '', '最近结果对应的 CommandId';
    'SafetyState',       'uint8',   1, '', '0=正常, 1=降级, 2=failsafe, 3=应急 — 已定义未落地（FailSafe 后置）';
    'SafetyDirective',   'uint8',   1, '', '0=无, 1=悬停, 2=返航, 3=降落, 4=禁止执行器 — 已定义未落地';
    'MissionDirective',  'uint8',   1, '', '0=继续, 1=挂起, 2=终止 — 已定义未落地';
    'FailsafeActive',    'boolean', 1, '', 'FailSafe 已触发 — 已定义未落地';
    'Valid',             'boolean', 1, '', '状态样本已初始化';
    };
end
