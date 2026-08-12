function cfg = config_MissionStatusBus()
%CONFIG_MISSIONSTATUSBUS 契约 MM-018（DEC-087/067）：MissionManager 任务状态流。
% 内容 = 任务状态 + 进度标签 + 可行性标志位；控制环直连总线不并入状态流体系（DEC-087）。
% 第一阶段纯正常流（DEC-111）：暂停/继续/安全相关枚举为已定义未落地（后置）。

cfg.busName = 'MissionStatusBus';
cfg.description = '任务执行状态与进度（内部消费者订阅）';
cfg.elements = {
    % {Name,               DataType, Dimensions, Unit, Description}
    'PlanId',             'uint32',  1, '', '当前锁存航线编号；无有效计划时为 0';
    'PlanValid',          'boolean', 1, '', '完整航线已原子锁存并可供执行';
    'ExecutionPhase',     'uint8',   1, '', '0=待命, 1=执行中, 2=暂停(后置), 3=已结束';
    'MissionOutcome',     'uint8',   1, '', '0=无结果, 1=成功, 2=取消(后置), 3=未成功';
    'MissionReason',      'uint8',   1, '', '结束/暂停原因：0=无, 1=航线完成, 2=操作者取消, 3=操作者暂停(后置), 4=显式推进, 5=安全挂起(后置), 6=内部错误, 7=安全终止(后置)';
    'CurrentItemIndex',   'uint8',   1, '', '当前任务项零基索引';
    'ItemCount',          'uint8',   1, '', '有效任务项总数';
    'ReachedItemIndex',   'int16',   1, '', '最近已到达的零基索引；-1 = 尚无';
    'ActiveContractId',   'uint32',  1, '', 'Navigator 当前应关联的运动学合同编号';
    'ExecutionHealthy',   'boolean', 1, '', '任务执行上下文内部可信';
    'MissionFaultCode',   'uint16',  1, '', '0=无故障；非零只报告 MM 自身异常事实';
    'LandingCompleted',   'boolean', 1, '', '末项 LAND 已满足条件、请求安全加锁（DEC-042）';
    'Valid',              'boolean', 1, '', '状态样本已初始化';
    };
end
