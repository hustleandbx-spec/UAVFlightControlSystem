function cfg = config_GatewayVfrHudTxRequestBus()
%CONFIG_GATEWAYVFRHUDTXREQUESTBUS Gateway internal VFR_HUD encode request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayVfrHudTxRequestBus';
cfg.description = 'Gateway internal MAVLink VFR_HUD encode request';
cfg.elements = {
    'Valid',        'boolean', 1, '',    'Request valid';
    'heading',      'int16',   1, 'cdeg','Heading (cdeg)';
    'throttle',     'uint16',  1, '0.1%','Throttle (0..1000 = 0..100%)';
    'alt',          'single',  1, 'm',   'Altitude';
    'airspeed',     'single',  1, 'm/s', 'Airspeed';
    'groundspeed',  'single',  1, 'm/s', 'Groundspeed';
    'climb',        'single',  1, 'm/s', 'Climb rate';
    };
end
