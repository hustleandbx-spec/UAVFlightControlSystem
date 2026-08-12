function cfg = config_MissionPlanBus()
%CONFIG_MISSIONPLANBUS 契约 #5（DEC-088）：Gateway→MissionManager 航线数据（数据通道，订阅发布）。
% 定长 400 航点，承载 A1 schema 全字段（扁平化为 [400] / [400 3] 定长数组，满足 Simulink 代码生成）；
% 未实现字段（动作/载荷/风险/协调转弯）置空并标"已定义未落地"。内存 ~30KB（DEC-075 预算内）。

cfg.busName = 'MissionPlanBus';
cfg.description = 'A1 任务航线快照（最多 400 航点，定长扁平）';
cfg.elements = {
    % ---- Meta / Coordinate / Config（DEC-082/083/088）----
    'TaskId',            'uint32',  1,     '',     '任务编号';
    'Version',           'uint32',  1,     '',     '计划版本';
    'Timestamp',         'double',  1,     's',    '计划生成时间';
    'Frame',             'uint8',   1,     '',     '坐标参考系：0=NED 局部';
    'ReferencePointNED', 'single',  3,     'm',    '锚定参考点（A-003）';
    'HeightRef',         'uint8',   1,     '',     '高度参考：0=相对声明参考点';
    'FinishAction',      'uint8',   1,     '',     '末点后行为：0=无, 1=返航, 2=降落, 3=悬停';
    'LinkLossDefault',   'uint8',   1,     '',     '链路丢失默认：0=悬停, 1=返航, 2=继续（DEC-004）';
    'SafeTakeoffHeight', 'single',  1,     'm',    '起飞段安全起飞高度（DEC-045）';
    'DepartureSpeed',    'single',  1,     'm/s',  '去首点速度（DEC-082）';
    'DefaultSpeed',      'single',  1,     'm/s',  '段默认巡航速度';
    'DefaultClimbRate',  'single',  1,     'm/s',  '默认爬升/下降率';
    'ArrivalDefault',    'single',  4,     '',     '默认到达判据 [位置,速度,高度,偏航]（DEC-055）';
    'DefaultYawMode',    'uint8',   1,     '',     '默认偏航：0=跟随航线, 1=固定航向';
    % ---- Waypoint[400] 扁平数组（DEC-083/082/078/088）----
    'WaypointCount',     'uint16',  1,     '',     '有效航点数（≤400）';
    'Lat',               'single',  400,   'deg',  '航点纬度';
    'Lon',               'single',  400,   'deg',  '航点经度';
    'Height',            'single',  400,   'm',    '航点高度（相对参考点）';
    'Speed',             'single',  400,   'm/s',  '航点速度覆盖（0=用 config 默认）';
    'ClimbRate',         'single',  400,   'm/s',  '航点爬升率覆盖（0=用 config 默认）';
    'PassMode',          'uint8',   400,   '',     '0=直线/停, 1=直线/过, 2=曲线/停, 3=曲线/过';
    'YawMode',           'uint8',   400,   '',     '0=跟随航线, 1=固定航向';
    'YawParam',          'single',  400,   'rad',  '固定航向角度（YawMode=1 时）';
    'ArrivalPos',        'single',  400,   'm',    '到达位置半径覆盖（0=config 默认）';
    'ArrivalVel',        'single',  400,   'm/s',  '到达速度覆盖（0=config 默认）';
    'ArrivalAlt',        'single',  400,   'm',    '到达高度覆盖（0=config 默认）';
    'ArrivalYaw',        'single',  400,   'rad',  '到达偏航覆盖（0=config 默认）';
    'ActionTrigger',     'uint8',   400,   '',     '动作触发：0=无, 1=到达触发 — 已定义未落地';
    'ActionCount',       'uint8',   400,   '',     '动作序列条数 — 已定义未落地';
    'PayloadValid',      'boolean', 400,   '',     '云台载荷字段 — 已定义未落地';
    'IsRisky',           'boolean', 400,   '',     '风险标记 — 已定义未落地';
    'CoordinatedTurn',   'boolean', 400,   '',     '协调转弯 — 已定义未落地';
    'Valid',             'boolean', 1,     '',     '整个快照已完成校验';
    };
end
