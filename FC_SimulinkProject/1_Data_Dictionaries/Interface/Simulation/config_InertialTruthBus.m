function cfg = config_InertialTruthBus()
cfg.contractDomain = 'Simulation'; cfg.busName = 'InertialTruthBus';
cfg.description = 'MIL plant 惯性真值';
cfg.elements = {'Attitude_quat','single',4,'','姿态四元数'; 'AngularRate_Body','single',3,'rad/s','机体角速度'; 'Accel_Body','single',3,'m/s^2','机体加速度'; 'Sequence','uint32',1,'','真值序号'; 'Timestamp','double',1,'s','真值时间戳'; 'Valid','boolean',1,'','真值完整有效'};
end
