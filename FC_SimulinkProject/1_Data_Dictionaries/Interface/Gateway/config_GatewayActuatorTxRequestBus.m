function cfg = config_GatewayActuatorTxRequestBus()
%CONFIG_GATEWAYACTUATORTXREQUESTBUS Gateway internal actuator request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayActuatorTxRequestBus';
cfg.description = 'Gateway internal MAVLink ACTUATOR_OUTPUT_STATUS encode request';
cfg.elements = {
    'Valid',     'boolean', 1, '', 'Request valid';
    'time_usec', 'uint64',  1, 'us', 'Boot-relative timestamp';
    'active',    'uint32',  1, '', 'Active actuator bit mask';
    'actuator',  'single', 32, '', 'Normalized actuator outputs';
    };
end
