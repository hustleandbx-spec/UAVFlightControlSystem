function cfg = config_MissionControlRequestBus()
%CONFIG_MISSIONCONTROLREQUESTBUS 契约 #6 路由（DEC-066/071）：Commander 审查批准后路由到 MissionManager
% 的操作请求。Action 集对齐模式级命令（DEC-101/110）与 go-to-point 激活（DEC-102）；
% 暂停/继续/取消 = 已定义未落地（断点续飞 DEC-037 后置）。

cfg.busName = 'MissionControlRequestBus';
cfg.description = 'Commander 批准后路由到 MissionManager 的操作请求';
cfg.elements = {
    % {Name,               DataType, Dimensions, Unit, Description}
    'RequestId',        'uint32',  1,    '',  '请求关联号（单调递增）';
    'CommandId',        'uint32',  1,    '',  '源命令编号（用于 Ack 关联）';
    'Action',           'uint8',   1,    '',  '1=开始任务, 2=返航, 3=降落, 4=悬停, 5=接管, 6=go-to-point 激活；暂停/继续/取消 后置';
    'TargetPositionNED','single',  3,    'm', 'go-to-point 目标 NED（Action=6）；其他动作忽略';
    'Valid',            'boolean', 1,    '',  '请求字段已完成校验（不等于 Commander 已授权）';
    };
end
