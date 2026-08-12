function test_mission_manager_contract()
%TEST_MISSION_MANAGER_CONTRACT 验证独立 MissionManager 的静态架构契约。
% 本测试直接读取 SLX 内部 XML，不启动仿真，因此可以快速发现端口、组件命名、
% 目标字段投影或协议隔离被意外破坏的问题；动态状态行为由 Gherkin MIL 覆盖。

% 从测试目录向上定位工程根目录，避免依赖调用者的当前工作路径。
projectRoot = fileparts(fileparts(mfilename('fullpath')));
modelPath = fullfile(projectRoot, '2_Model', 'mission_manager', ...
    'MissionManager.slx');
assert(isfile(modelPath), 'Missing standalone MissionManager.slx.');

% 将 Simulink 系统 XML 和 Stateflow XML 合并成可检索文本。
modelText = extractAllSlxSystemText(modelPath);
% 这些名称共同定义四入两出边界和增量航线执行的六个内部组件。
requiredText = {
    'MissionPlan'
    'MissionControlRequest'
    'CommanderStatus'
    'NavigationStatus'
    'NavigationContract'
    'MissionStatus'
    'MissionPlanStore'
    'MissionExecutionExecutive'
    'RouteContextSelector'
    'NavigationContractCompiler'
    'ActiveContractRegister'
    'MissionStatusAssembler'
    'PreviousExecutionPhase'
    };
for i = 1:numel(requiredText)
    assert(contains(modelText, requiredText{i}), ...
        'MissionManager missing required element: %s', requiredText{i});
end

% 目标投影必须保留计划身份、零基索引、NED 几何和到达约束。
% 使用精确赋值文本可以防止字段被常量替代或遗漏。
requiredObjectiveProjection = {
    'window.PlanId = plan.PlanId'
    'window.ItemIndex = currentItemIndex'
    'window.CurrentPositionNED = reshape(plan.TargetPositionNED(idx,:),[3 1])'
    'contract.SegmentEndPositionNED = window.CurrentPositionNED'
    'contract.TargetYaw = window.CurrentYaw'
    'contract.AcceptanceRadius = window.AcceptanceRadius'
    'contract.RequiredStableTime = window.DwellTime'
    };
for i = 1:numel(requiredObjectiveProjection)
    assert(contains(modelText, requiredObjectiveProjection{i}), ...
        'MissionManager does not project required objective field: %s', ...
        requiredObjectiveProjection{i});
end

% 禁止退化回单块 MissionLogic，也禁止叶子算法用 persistent 隐藏任务状态；
% MissionExecutive 必须是任务执行状态的唯一所有者。
assert(~contains(modelText, '>MissionLogic<'), ...
    'MissionManager must not use the monolithic MissionLogic block.');
assert(~contains(modelText, 'persistent'), ...
    'MissionManager leaf algorithms must not hide persistent mission state.');
assert(~contains(modelText, 'ObjectiveType'), ...
    'MissionManager must not expose task phase through NavigationObjective.');
assert(~contains(modelText, 'holdObjective', 'IgnoreCase', true), ...
    'MissionManager must not compile a Hold task type for Navigator.');

% MissionManager 是协议无关的任务层，不应依赖地面站、传输协议或具体仿真环境。
forbiddenText = {
    'MAVLink'
    'QGroundControl'
    'QgcMavlinkGateway'
    'ROS2'
    'Gazebo'
    'FlightCmdBus'
    };
for i = 1:numel(forbiddenText)
    assert(~contains(modelText, forbiddenText{i}), ...
        'MissionManager contains forbidden external/protocol term: %s', ...
        forbiddenText{i});
end

% 六个总线配置文件构成 MissionManager 可独立构建和审查的类型边界。
busConfigDir = fullfile(projectRoot, '1_Data_Dictionaries', 'BusConfig');
requiredBusConfigs = {
    'config_MissionPlanBus.m'
    'config_CommanderStatusBus.m'
    'config_RouteWindowBus.m'
    'config_NavigationContractBus.m'
    'config_NavigationStatusBus.m'
    'config_MissionStatusBus.m'
    'config_MissionControlRequestBus.m'
    };
for i = 1:numel(requiredBusConfigs)
    assert(isfile(fullfile(busConfigDir, requiredBusConfigs{i})), ...
        'Missing MissionManager bus config: %s', requiredBusConfigs{i});
end

fprintf('MISSION_MANAGER_CONTRACT_PASS\n');
end

function text = extractAllSlxSystemText(slxPath)
%EXTRACTALLSLXSYSTEMTEXT 解压 SLX 并汇总系统与 Stateflow XML 文本。
% 临时目录由 onCleanup 保证在正常返回或异常退出时都能删除。
tmpDir = tempname;
mkdir(tmpDir);
cleanup = onCleanup(@() rmdir(tmpDir, 's'));
unzip(slxPath, tmpDir);
systemsDir = fullfile(tmpDir, 'simulink', 'systems');
% blockdiagram.xml 保存模型级信息，systems 与 stateflow 子目录保存实现细节。
files = dir(fullfile(systemsDir, '*.xml'));
stateflowFiles = dir(fullfile(tmpDir, 'simulink', 'stateflow', '*.xml'));
parts = strings(numel(files) + numel(stateflowFiles) + 1, 1);
parts(1) = string(fileread(fullfile(tmpDir, ...
    'simulink', 'blockdiagram.xml')));
% 按文件枚举顺序拼接即可；本测试只做包含性检查，不依赖 XML 顺序。
for i = 1:numel(files)
    parts(i + 1) = string(fileread(fullfile( ...
        files(i).folder, files(i).name)));
end
for i = 1:numel(stateflowFiles)
    parts(numel(files) + i + 1) = string(fileread(fullfile( ...
        stateflowFiles(i).folder, stateflowFiles(i).name)));
end
text = join(parts, newline);
% 显式清除 cleanup，使临时目录在函数返回前完成回收。
clear cleanup;
end
