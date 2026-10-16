function cfg = config_TrajectorySetpointSnapshotBus()
cfg.contractDomain = 'Control'; cfg.busName = 'TrajectorySetpointSnapshotBus';
cfg.description = '轨迹参考观察快照';
cfg.elements = {'Position_NED_SP','single',3,'m','NED 位置参考'; 'Velocity_NED_SP','single',3,'m/s','NED 速度参考'; 'Yaw_SP','single',1,'rad','偏航参考'; 'ReferenceValid','boolean',1,'','原参考有效'; 'Sequence','uint32',1,'','快照序号'; 'Timestamp','double',1,'s','快照时间戳'; 'Valid','boolean',1,'','快照完整有效'};
end
