function cfg = config_GatewayGlobalPositionTxRequestBus()
%CONFIG_GATEWAYGLOBALPOSITIONTXREQUESTBUS Gateway internal global-position request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayGlobalPositionTxRequestBus';
cfg.description = 'Gateway internal MAVLink GLOBAL_POSITION_INT encode request';
cfg.elements = {
    'Valid',        'boolean', 1, '', 'Request valid';
    'time_boot_ms', 'uint32',  1, 'ms', 'Boot-relative timestamp';
    'lat',          'int32',   1, 'degE7', 'Latitude';
    'lon',          'int32',   1, 'degE7', 'Longitude';
    'alt',          'int32',   1, 'mm', 'MSL altitude';
    'relative_alt', 'int32',   1, 'mm', 'Relative altitude';
    'vx',           'int16',   1, 'cm/s', 'North velocity';
    'vy',           'int16',   1, 'cm/s', 'East velocity';
    'vz',           'int16',   1, 'cm/s', 'Down velocity';
    'hdg',          'uint16',  1, 'cdeg', 'Heading';
    };
end
