function cfg = config_NavigationContractBus()
%CONFIG_NAVIGATIONCONTRACTBUS 契约 #1 v2（DEC-085）：MissionManager 向 Navigator 发布的纯运动学合同（段=run）。
% 只表达运动学（到哪、以什么状态结束），不携带任务/模式语义；到达判据归 MM（DEC-062/055）。
% v2 变更：删 PathType/TerminalVelocityNED/ExitDirectionNED/AcceptanceRadius/RequiredStableTime，
%         加 PassCount + PassPositionNED（过点序列），Segment* 改名 Start*/End*，ItemIndex→SegmentIndex。

cfg.busName = 'NavigationContractBus';
cfg.description = '运动学合同（段=run），任务无关（DEC-085）';
cfg.elements = {
    % {Name,               DataType,  Dimensions, Unit,  Description}
    'ContractId',        'uint32',  1,    '',     '合同关联号（单调递增）';
    'PlanId',            'uint32',  1,    '',     '所属计划编号';
    'SegmentIndex',      'uint32',  1,    '',     '段索引（可追溯）';
    'StartValid',        'boolean', 1,    '',     'false = Navigator 快照当前状态（DEC-065）';
    'StartPositionNED',  'single',  3,    'm',    '显式段起点（StartValid=true 时有效）';
    'PassCount',         'uint8',   1,    '',     '过点个数 n（0 = 纯停段）';
    'PassPositionNED',   'single',  [64 3],'m',   '过点序列（仅前 PassCount 项有效）';
    'EndPositionNED',    'single',  3,    'm',    '终止停点位置';
    'CruiseSpeed',       'single',  1,    'm/s',  '段巡航速度';
    'MaxAcceleration',   'single',  1,    'm/s^2','段最大加速度';
    'MaxJerk',           'single',  1,    'm/s^3','段最大加加速度';
    'TargetYaw',         'single',  1,    'rad',  '终止停点目标偏航（DEC-082）';
    'Valid',             'boolean', 1,    '',     '合同完整且已授权';
    };
end
