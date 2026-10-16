function cfg = config_MissionPlanBus()
%CONFIG_MISSIONPLANBUS MissionManager 内部规范化单计划槽。
% 标准任务项通过 MissionItemsBus 进入 MissionManager。本类型不作为
% 模块端口，只保存完成一次性 WGS-84->NED 归一化后的最小执行信息。

cfg.busName = 'MissionPlanBus';
cfg.contractDomain = 'Mission';
cfg.description = 'MissionManager 内部规范化计划槽（standard Mission stop-waypoint execution）';
cfg.elements = {
    % {Name,                 DataType, Dimensions, Unit,  Description}
    'TaskId',                'uint32',  1,       '',     '任务编号';
    'SafeTakeoffHeight',     'single',  1,       'm',    '原地起飞目标高度';
    'CruiseSpeed',           'single',  1,       'm/s',  '全部普通航段的巡航速度';
    'WaypointCount',         'uint16',  1,       '',     '有效停点数量（最多 400）';
    'WaypointPositionNED',   'single',  [400 3], 'm',    '规范化停点 NED 坐标，仅前 WaypointCount 行有效';
    'TargetYaw',             'single',  400,     'rad',  '各停点目标偏航，仅前 WaypointCount 项有效';
    'RawItemCount', 'uint16', 1, '', 'Original mission item count';
    'WaypointSeq', 'uint16', 400, '', 'Original seq for each internal waypoint';
    'HoldSeconds', 'single', 400, 's', 'Waypoint dwell time';
    'Valid',                 'boolean', 1,       '',     '计划槽包含完整可执行计划';
    };
end
