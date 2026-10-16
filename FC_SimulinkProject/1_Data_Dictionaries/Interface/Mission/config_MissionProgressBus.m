function cfg = config_MissionProgressBus()
cfg.contractDomain = 'Mission';
cfg.busName = 'MissionProgressBus';
cfg.description = '任务执行进度快照';
cfg.elements = {
    'PlanId',           'uint32', 1, '',  '计划编号';
    'CurrentItemIndex', 'uint16', 1, '',  '当前任务项零基索引';
    'ItemCount',        'uint16', 1, '',  '有效任务项总数';
    'ReachedItemIndex', 'int16',  1, '',  '最近到达索引，-1 表示尚无';
    'ActiveContractId', 'uint32', 1, '',  '当前运动学合同编号';
    'Sequence',         'uint32', 1, '',  '快照序号';
    'Timestamp',        'double', 1, 's', '快照时间戳';
    'Valid',            'boolean',1, '',  '快照完整有效';
    };
end
