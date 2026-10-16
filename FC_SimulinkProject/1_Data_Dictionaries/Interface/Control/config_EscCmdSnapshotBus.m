function cfg = config_EscCmdSnapshotBus()
cfg.contractDomain = 'Control'; cfg.busName = 'EscCmdSnapshotBus';
cfg.description = '执行机构指令观察快照';
cfg.elements = {'MotorCmd','single',4,'1','电机归一化指令'; 'Armed','boolean',1,'','执行器解锁状态'; 'CommandValid','boolean',1,'','原指令有效'; 'Sequence','uint32',1,'','快照序号'; 'Timestamp','double',1,'s','快照时间戳'; 'Valid','boolean',1,'','快照完整有效'};
end
