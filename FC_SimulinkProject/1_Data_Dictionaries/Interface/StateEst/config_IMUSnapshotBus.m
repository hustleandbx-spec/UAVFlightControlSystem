function cfg = config_IMUSnapshotBus()
cfg.contractDomain = 'StateEst'; cfg.busName = 'IMUSnapshotBus';
cfg.description = 'IMU 测量观察快照';
cfg.elements = {'Accel','single',3,'m/s^2','机体比力'; 'Gyro','single',3,'rad/s','机体角速度'; 'MeasurementValid','boolean',1,'','原测量有效'; 'Sequence','uint32',1,'','测量序号'; 'Timestamp','double',1,'s','测量时间戳'; 'Valid','boolean',1,'','快照完整有效'};
end
