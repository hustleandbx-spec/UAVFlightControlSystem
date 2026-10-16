function cfg = config_StateEstDiagnosticBus()
cfg.contractDomain = 'StateEst'; cfg.busName = 'StateEstDiagnosticBus';
cfg.description = '状态估计器诊断快照';
cfg.elements = {'Accel_Body','single',3,'m/s^2','机体加速度'; 'GyroBias','single',3,'rad/s','陀螺零偏'; 'AccelBias','single',3,'m/s^2','加速度计零偏'; 'Wind_NED','single',3,'m/s','NED 风速'; 'Sequence','uint32',1,'','快照序号'; 'Timestamp','double',1,'s','快照时间戳'; 'Valid','boolean',1,'','快照完整有效'};
end
