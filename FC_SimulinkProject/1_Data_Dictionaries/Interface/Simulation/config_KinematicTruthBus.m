function cfg = config_KinematicTruthBus()
cfg.contractDomain = 'Simulation'; cfg.busName = 'KinematicTruthBus';
cfg.description = 'MIL plant 运动学真值';
cfg.elements = {'Position_NED','single',3,'m','NED 位置'; 'Velocity_NED','single',3,'m/s','NED 速度'; 'Sequence','uint32',1,'','真值序号'; 'Timestamp','double',1,'s','真值时间戳'; 'Valid','boolean',1,'','真值完整有效'};
end
