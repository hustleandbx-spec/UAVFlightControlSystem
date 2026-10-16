function cfg = config_GatewayGpsRawTxRequestBus()
%CONFIG_GATEWAYGPSRAWTXREQUESTBUS Gateway internal GPS_RAW_INT encode request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayGpsRawTxRequestBus';
cfg.description = 'Gateway internal MAVLink GPS_RAW_INT encode request';
cfg.elements = {
    'Valid',              'boolean', 1, '',      'Request valid';
    'time_usec',          'uint64',  1, 'us',    'Timestamp (us)';
    'fix_type',           'uint8',   1, '',      'GPS fix type (placeholder/TBD)';
    'lat',                'int32',   1, 'degE7', 'Latitude';
    'lon',                'int32',   1, 'degE7', 'Longitude';
    'alt',                'int32',   1, 'mm',    'MSL altitude';
    'eph',                'uint16',  1, 'cm',    'Horizontal EPS (placeholder/TBD)';
    'epv',                'uint16',  1, 'cm',    'Vertical EPS (placeholder/TBD)';
    'vel',                'uint16',  1, 'cm/s',  'Ground speed';
    'cog',                'uint16',  1, 'cdeg',  'Course over ground';
    'satellites_visible', 'uint8',   1, '',      'Visible satellites (placeholder/TBD)';
    };
end
