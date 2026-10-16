function cfg = config_GatewayAttitudeTxRequestBus()
%CONFIG_GATEWAYATTITUDETXREQUESTBUS Gateway internal attitude encode request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayAttitudeTxRequestBus';
cfg.description = 'Gateway internal MAVLink ATTITUDE_QUATERNION encode request';
cfg.elements = {
    'Valid',         'boolean', 1, '', 'Request valid';
    'time_boot_ms',  'uint32',  1, 'ms', 'Boot-relative timestamp';
    'q1',            'single',  1, '', 'Quaternion scalar';
    'q2',            'single',  1, '', 'Quaternion x';
    'q3',            'single',  1, '', 'Quaternion y';
    'q4',            'single',  1, '', 'Quaternion z';
    'rollspeed',     'single',  1, 'rad/s', 'Body roll rate';
    'pitchspeed',    'single',  1, 'rad/s', 'Body pitch rate';
    'yawspeed',      'single',  1, 'rad/s', 'Body yaw rate';
    'repr_offset_q', 'single',  4, '', 'Representation offset quaternion';
    };
end
