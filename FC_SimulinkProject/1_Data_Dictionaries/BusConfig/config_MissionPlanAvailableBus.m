function cfg = config_MissionPlanAvailableBus()
%CONFIG_MISSIONPLANAVAILABLEBUS 契约 #5（MM-004/005）：计划可用通知（数据通道状态流，不经 CommandAckBus）。
% 任一批校验收失败 → Available=false + ReasonCode，计划槽原内容保留（DEC-106 失败保留）。

cfg.busName = 'MissionPlanAvailableBus';
cfg.description = '任务计划可用性通知（数据通道状态流）';
cfg.elements = {
    % {Name,       DataType, Dimensions, Unit, Description}
    'Available',   'boolean', 1,    '', '计划通过解析+结构校验、可用于执行';
    'ReasonCode',  'uint16',  1,    '', 'Available=false 时拒绝原因：1=不可解析, 2=字段非法, 3=容量>400, 4=末航点非停, 5=段结构非法';
    'PlanId',      'uint32',  1,    '', '被报告的计划编号';
    'Valid',       'boolean', 1,    '', '通知事件完整有效';
    };
end
