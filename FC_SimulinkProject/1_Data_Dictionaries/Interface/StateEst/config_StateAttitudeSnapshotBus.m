function cfg = config_StateAttitudeSnapshotBus()
cfg.contractDomain = 'StateEst'; cfg.busName = 'StateAttitudeSnapshotBus';
cfg.description = '姿态与角速率状态快照';
cfg.elements = snapshotElements({
    'Attitude_quat','single',4,'','姿态四元数 [w x y z]';
    'AngularRate_Body','single',3,'rad/s','机体角速度';
    });
end
function elements = snapshotElements(fields)
elements = [fields; {'Sequence','uint32',1,'','快照序号'; 'Timestamp','double',1,'s','快照时间戳'; 'Valid','boolean',1,'','快照完整有效'}];
end
