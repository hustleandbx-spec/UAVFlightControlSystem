function cfg = config_GatewayLocalPositionTxRequestBus()
%CONFIG_GATEWAYLOCALPOSITIONTXREQUESTBUS Gateway internal local-position request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayLocalPositionTxRequestBus';
cfg.description = 'Gateway internal MAVLink LOCAL_POSITION_NED encode request';
cfg.elements = {
    'Valid',        'boolean', 1, '', 'Request valid';
    'time_boot_ms', 'uint32',  1, 'ms', 'Boot-relative timestamp';
    'x',            'single',  1, 'm', 'NED north position';
    'y',            'single',  1, 'm', 'NED east position';
    'z',            'single',  1, 'm', 'NED down position';
    'vx',           'single',  1, 'm/s', 'NED north velocity';
    'vy',           'single',  1, 'm/s', 'NED east velocity';
    'vz',           'single',  1, 'm/s', 'NED down velocity';
    };
end
