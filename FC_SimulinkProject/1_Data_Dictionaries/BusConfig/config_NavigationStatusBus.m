function cfg = config_NavigationStatusBus()
%CONFIG_NAVIGATIONSTATUSBUS 契约 #6/NAV-005（DEC-087）：Navigator 制导状态流（订阅发布，宽松时序）。
% 承载制导状态含 Valid，供 Commander 命令审查（DEC-086④）/Logging 落盘/Gateway 遥测；
% 正常流无回报、不含到达判据（DEC-062 Navigator 判据无感知，到达判定归 MM）。

cfg.busName = 'NavigationStatusBus';
cfg.description = 'Navigator 制导状态流（含 Valid 可行性标志）';
cfg.elements = {
    % {Name,          DataType, Dimensions, Unit, Description}
    'ContractId',     'uint32',  1, '', '当前关联的运动学合同编号';
    'GuidanceState',  'uint8',   1, '', '0=等待合同, 1=段轨迹求值中, 2=悬停保持, 3=降落剖面';
    'Valid',          'boolean', 1, '', '轨迹参考有效性（正常流恒 true，最差=悬停保持，DEC-086）';
    };
end
