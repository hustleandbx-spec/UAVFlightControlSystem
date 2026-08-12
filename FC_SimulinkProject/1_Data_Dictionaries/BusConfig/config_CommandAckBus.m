function cfg = config_CommandAckBus()
%CONFIG_COMMANDACKBUS 契约 #6（DEC-089/090）：命令结果逐条 Ack，支持多事件时序（同一 CommandId 可
% approved→aborted，GCS 按 CommandId 聚合取最新）；Ack 事件流入 Logging 持久化（事后排查）。

cfg.busName = 'CommandAckBus';
cfg.description = '命令结果逐条确认（含多事件时序）';
cfg.elements = {
    % {Name,       DataType, Dimensions, Unit, Description}
    'CommandId',   'uint32',  1,    '',  '命令关联号（回显 CommanderRequestBus）';
    'Result',      'uint8',   1,    '',  '0=已批准, 1=已拒绝, 2=已中止';
    'ReasonCode',  'uint16',  1,    '',  '0=无, 1..7=起飞门#1..#7, 8=阶段非法, 9=参数非法, 10=安全否决(后置)';
    'Message',     'uint8',   32,   '',  'ASCII 原因文本（供操作者 HMI，DEC-003）';
    'Valid',       'boolean', 1,    '',  '确认事件完整有效';
    };
end
