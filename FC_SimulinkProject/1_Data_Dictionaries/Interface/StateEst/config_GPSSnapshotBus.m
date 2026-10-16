function cfg = config_GPSSnapshotBus()
cfg.contractDomain = 'StateEst'; cfg.busName = 'GPSSnapshotBus';
cfg.description = 'GPS 测量观察快照';
cfg.elements = {'Lat','single',1,'deg','纬度'; 'Lon','single',1,'deg','经度'; 'Alt','single',1,'m','海拔（WGS84 椭球高）'; 'Velocity','single',3,'m/s','NED 速度'; 'FixType','uint8',1,'','定位类型（MAVLink GPS_FIX_TYPE）'; 'NumSat','uint8',1,'','参与解算卫星数'; 'Eph','single',1,'m','水平定位不确定度(1σ)'; 'Epv','single',1,'m','垂直定位不确定度(1σ)'; 'MeasurementValid','boolean',1,'','原测量有效'; 'Sequence','uint32',1,'','测量序号'; 'Timestamp','double',1,'s','测量时间戳'; 'Valid','boolean',1,'','快照完整有效'};
end
