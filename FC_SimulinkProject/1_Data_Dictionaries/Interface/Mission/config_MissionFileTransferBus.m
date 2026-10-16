function cfg = config_MissionFileTransferBus()
%CONFIG_MISSIONFILETRANSFERBUS D 类原始任务文件交接（DEC-159/163）。

cfg.contractDomain = 'Mission';
cfg.busName = 'MissionFileTransferBus';
cfg.description = 'Gateway 到 MissionManager 的原始任务文件单槽交接';
cfg.elements = {
    'TransferId', 'uint32',  1,       '', '成功提交时分配的传输关联号';
    'FileSize',   'uint32',  1,       'byte', 'Data 中有效文件字节数';
    'Data',       'uint8',   1048576, '', '不透明原始任务文件字节';
    'Ready',      'boolean', 1,       '', 'payload 完整锁存后的单次交付事件标志';
    };
end
