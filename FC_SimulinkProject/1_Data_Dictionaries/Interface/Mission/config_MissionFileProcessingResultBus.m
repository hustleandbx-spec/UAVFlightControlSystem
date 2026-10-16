function cfg = config_MissionFileProcessingResultBus()
%CONFIG_MISSIONFILEPROCESSINGRESULTBUS D 类处理完成结果（DEC-159/163）。

cfg.contractDomain = 'Mission';
cfg.busName = 'MissionFileProcessingResultBus';
cfg.description = 'MissionManager 返回的任务文件处理完成结果';
cfg.elements = {
    'TransferId',        'uint32',  1, '', '回显当前文件交接关联号';
    'PlanId',            'uint32',  1, '', 'MissionManager 生成的计划标识';
    'Accepted',          'boolean', 1, '', '任务业务接受结论';
    'ReasonCode',        'uint16',  1, '', 'MissionManager 原始原因码';
    'ProcessingComplete','boolean', 1, '', '结果完整锁存后的保持电平';
    };
end
