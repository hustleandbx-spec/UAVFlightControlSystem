function cfg = config_StateSnapshot()
%CONFIG_STATESNAPSHOT 契约 #4（DEC-073/074）：SE→Navigator 段边界锚点（订阅，宽松时序，仅段边界衔接用）。
% 不含偏航——Navigator=制导（轨迹生成），yaw 为输出量非输入量（DEC-065/084）。

cfg.busName = 'StateSnapshot';
cfg.description = 'SE→Navigator 当前状态快照（段边界锚点，仅位置/速度）';
cfg.elements = {
    % {Name,           DataType, Dimensions, Unit, Description}
    'Position_NED',    'single',  3, 'm',    '当前估计位置（NED）';
    'Velocity_NED',    'single',  3, 'm/s',  '当前估计速度（NED）';
    };
end
