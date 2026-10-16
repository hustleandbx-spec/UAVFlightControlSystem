function cfg = config_GatewayTimeSyncTxRequestBus()
%CONFIG_GATEWAYTIMESYNCTXREQUESTBUS Gateway internal TIMESYNC encode request.

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayTimeSyncTxRequestBus';
cfg.description = 'Gateway internal MAVLink TIMESYNC encode request';
cfg.elements = {
    'Valid', 'boolean', 1, '', 'Request valid';
    'tc1',   'int64',   1, 'ns', 'Local response time';
    'ts1',   'int64',   1, 'ns', 'Requester timestamp echo';
    };
end
