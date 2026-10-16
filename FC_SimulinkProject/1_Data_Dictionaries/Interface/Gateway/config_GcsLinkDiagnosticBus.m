function cfg = config_GcsLinkDiagnosticBus()
cfg.contractDomain = 'Gateway'; cfg.busName = 'GcsLinkDiagnosticBus';
cfg.description = 'GCS 链路协议诊断快照（仅供 Logging）';
cfg.elements = {
    'HasAcceptedRx','boolean',1,'','是否存在已接纳帧证据';
    'LastAcceptedRxSequence','uint8',1,'','最后已接纳 MAVLink sequence';
    'LastAcceptedRxTimeSec','double',1,'s','最后已接纳帧时刻';
    'AcceptedRxCount','uint32',1,'','已接纳帧计数';
    'StructureRejectCount','uint32',1,'','结构拒绝计数';
    'SourceRejectCount','uint32',1,'','来源拒绝计数';
    'TargetRejectCount','uint32',1,'','目标拒绝计数';
    'UnsupportedMessageCount','uint32',1,'','不支持消息计数';
    'SequenceGapCount','uint32',1,'','sequence 缺口累计';
    'DuplicateSequenceCount','uint32',1,'','重复 sequence 计数';
    'OutOfOrderSequenceCount','uint32',1,'','乱序 sequence 计数';
    'Sequence','uint32',1,'','快照序号';
    'Timestamp','double',1,'s','快照时间戳';
    'Valid','boolean',1,'','快照完整有效';
    };
end
