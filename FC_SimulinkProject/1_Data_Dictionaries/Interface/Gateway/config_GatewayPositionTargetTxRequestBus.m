function cfg = config_GatewayPositionTargetTxRequestBus()
%CONFIG_GATEWAYPOSITIONTARGETTXREQUESTBUS Gateway internal target request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayPositionTargetTxRequestBus';
cfg.description = 'Gateway internal MAVLink POSITION_TARGET_LOCAL_NED encode request';
cfg.elements = {
    'Valid',            'boolean', 1, '', 'Request valid';
    'time_boot_ms',     'uint32',  1, 'ms', 'Boot-relative timestamp';
    'x',                'single',  1, 'm', 'NED north target';
    'y',                'single',  1, 'm', 'NED east target';
    'z',                'single',  1, 'm', 'NED down target';
    'vx',               'single',  1, 'm/s', 'North velocity target';
    'vy',               'single',  1, 'm/s', 'East velocity target';
    'vz',               'single',  1, 'm/s', 'Down velocity target';
    'afx',              'single',  1, 'm/s^2', 'North acceleration target';
    'afy',              'single',  1, 'm/s^2', 'East acceleration target';
    'afz',              'single',  1, 'm/s^2', 'Down acceleration target';
    'yaw',              'single',  1, 'rad', 'Yaw target';
    'yaw_rate',         'single',  1, 'rad/s', 'Yaw-rate target';
    'type_mask',        'uint16',  1, '', 'MAVLink type mask';
    'coordinate_frame', 'uint8',   1, '', 'MAVLink coordinate frame';
    };
end
