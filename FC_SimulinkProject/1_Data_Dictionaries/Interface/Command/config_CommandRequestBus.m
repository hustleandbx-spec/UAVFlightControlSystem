function cfg = config_CommandRequestBus()
%CONFIG_COMMANDERREQUESTBUS 契约 #6（DEC-066/089）：GCS 写命令请求（Gateway → Commander 统一命令入口）。
% Commander 审查（CMD-002）后逐条 Ack（CommandAckBus，DEC-089/090 多事件时序）。
% 命令集对齐 DEC-154/163：加锁/解锁、开始任务、返航、降落、悬停、GoToPoint。

cfg.contractDomain = 'Command';
cfg.busName = 'CommandRequestBus';
cfg.description = '通用单播命令请求';
cfg.elements = {
    'SourceModule',      'uint8',   1, '',  '逻辑源模块';
    'DestinationModule', 'uint8',   1, '',  '逻辑目标模块';
    'CommandId',         'uint32',  1, '',  '命令关联号';
    'Command',           'uint8',   1, '',  '共享命令枚举值（权威归 CommandContractDict，DEC-157）';
    'Params',            'double',  8, '',  '规范化命令参数槽';
    'ParamValidMask',    'uint16',  1, '',  'bit i 表示 Params(i) 有效，bits 8..15 保持 0';
    'Timestamp',         'double',  1, 's', '请求时间戳';
    'Valid',             'boolean', 1, '',  '请求完整有效';
    };
end
