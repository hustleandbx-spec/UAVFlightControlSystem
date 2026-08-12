function cfg = config_RouteWindowBus()
%CONFIG_ROUTEWINDOWBUS MissionManager 内部当前航段滑动窗口（Prev/Cur/Next，含 IsFirst/IsLast）。
% 只来自已锁存计划（DEC-093 单计划槽 / DEC-085 预切分段视图），该总线不跨模块发布（DEC-087）。

cfg.busName = 'RouteWindowBus';
cfg.description = '已锁存计划的当前航段滑动窗口（MM 内部段视图）';
cfg.elements = {
    % {Name,               DataType,  Dimensions, Unit,  Description}
    'PlanId',              'uint32',  1,    '',     '所属计划编号';
    'ItemIndex',           'uint8',   1,    '',     '当前项零基索引';
    'PreviousValid',       'boolean', 1,    '',     '前一锚点可用';
    'PreviousPositionNED', 'single',  3,    'm',    '前一导航锚点位置（NED）';
    'CurrentTaskType',     'uint8',   1,    '',     '当前任务项类型';
    'CurrentPositionNED',  'single',  3,    'm',    '当前航点位置（NED）';
    'CurrentYaw',          'single',  1,    'rad',  '当前航点偏航';
    'AcceptanceRadius',    'single',  1,    'm',    '当前终止停点位置容差';
    'TerminalBehavior',    'uint8',   1,    '',     '0=停, 1=过, 2=停留';
    'AdvancePolicy',       'uint8',   1,    '',     '0=显式推进, 1=自动';
    'DwellTime',           'single',  1,    's',    '要求的终止停留时间';
    'CruiseSpeed',         'single',  1,    'm/s',  '请求的段巡航速度';
    'MaxAcceleration',     'single',  1,    'm/s^2','段最大加速度';
    'MaxJerk',             'single',  1,    'm/s^3','段最大加加速度';
    'NextValid',           'boolean', 1,    '',     '下一锚点可用';
    'NextPositionNED',     'single',  3,    'm',    '下一导航锚点位置（NED）';
    'IsFirstItem',         'boolean', 1,    '',     '当前项为首项';
    'IsLastItem',          'boolean', 1,    '',     '当前项为末项';
    'Valid',               'boolean', 1,    '',     '窗口完整且有界';
    };
end
