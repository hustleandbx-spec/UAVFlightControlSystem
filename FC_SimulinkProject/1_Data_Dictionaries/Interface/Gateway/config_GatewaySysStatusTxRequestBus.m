function cfg = config_GatewaySysStatusTxRequestBus()
%CONFIG_GATEWAYSYSSTATUSTXREQUESTBUS Gateway internal SYS_STATUS encode request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewaySysStatusTxRequestBus';
cfg.description = 'Gateway internal MAVLink SYS_STATUS encode request';
cfg.elements = {
    'Valid',                          'boolean', 1, '',      'Request valid';
    'onboard_control_sensors_present', 'uint32', 1, '',      'Sensors present';
    'onboard_control_sensors_enabled', 'uint32', 1, '',      'Sensors enabled';
    'onboard_control_sensors_health',  'uint32', 1, '',      'Sensors healthy';
    'load',                           'uint16',  1, '0.1%',  'CPU load (placeholder/TBD)';
    'voltage_battery',                'uint16',  1, 'mV',    'Battery voltage (placeholder/TBD)';
    'current_battery',                'int16',   1, 'cA',    'Battery current (placeholder/TBD)';
    'battery_remaining',              'int8',    1, '%',     'Battery remaining (placeholder/TBD)';
    'drop_rate_comm',                 'uint16',  1, '0.01%', 'Comm drop rate (placeholder/TBD)';
    'errors_comm',                    'uint16',  1, '',      'Comm errors (placeholder/TBD)';
    'errors_count1',                  'uint16',  1, '',      'Error count 1 (placeholder/TBD)';
    'errors_count2',                  'uint16',  1, '',      'Error count 2 (placeholder/TBD)';
    'errors_count3',                  'uint16',  1, '',      'Error count 3 (placeholder/TBD)';
    'errors_count4',                  'uint16',  1, '',      'Error count 4 (placeholder/TBD)';
    };
end
