function cfg = config_CommandAckBus()
%CONFIG_COMMANDACKBUS 契约 #6（DEC-089/090）：命令结果逐条 Ack，支持多事件时序（同一 CommandId 可
% approved→aborted，GCS 按 CommandId 聚合取最新）；Ack 事件流入 Logging 持久化（事后排查）。

cfg.busName = 'CommandAckBus';
cfg.contractDomain = 'Command';
cfg.description = '命令结果逐条确认（含多事件时序）';
cfg.elements = {
    % {Name,       DataType, Dimensions, Unit, Description}
    'SourceModule',      'uint8',   1, '',  '逻辑源模块';
    'DestinationModule', 'uint8',   1, '',  '逻辑目标模块';
    'CommandId',   'uint32',  1,    '',  '命令关联号（回显 CommandRequestBus）';
    'Result',      'uint8',   1,    '',  '0=None, 1=Approved, 2=Completed, 3=Rejected, 4=Aborted, 5=Failed';
    'ReasonCode',  'uint16',  1,    '',  '0=无, 1..7=起飞门#1..#7, 8=阶段非法, 9=参数非法, 10=安全否决(后置)';
    'Message',     'uint8',   32,   '',  'ASCII 原因文本（供操作者 HMI，DEC-003）';
    'Timestamp',   'double',  1,    's', '结果事件时间戳';
    'Valid',       'boolean', 1,    '',  '确认事件完整有效';
    };
end
