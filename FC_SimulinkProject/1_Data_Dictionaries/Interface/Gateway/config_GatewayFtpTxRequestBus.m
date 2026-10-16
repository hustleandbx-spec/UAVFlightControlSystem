function cfg = config_GatewayFtpTxRequestBus()
%CONFIG_GATEWAYFTPTXREQUESTBUS Gateway 内部 FTP 响应编码请求。

cfg.contractDomain = 'Gateway';
cfg.busName = 'GatewayFtpTxRequestBus';
cfg.description = 'Gateway 内部 MAVLink FILE_TRANSFER_PROTOCOL 响应编码请求';
cfg.elements = {
    'target_network',   'uint8', 1,   '', '响应目标网络';
    'target_system',    'uint8', 1,   '', '响应目标 SystemId';
    'target_component', 'uint8', 1,   '', '响应目标 ComponentId';
    'payload',          'uint8', 251, '', 'MAVLink FTP payload';
    };
end
