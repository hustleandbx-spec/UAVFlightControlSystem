function build_all()
%BUILD_ALL  统一构建 FC_SimulinkProject 所有数据字典。
%
% 用法：在 MATLAB 里运行 build_all;
% 清单文件：同目录 manifest.yaml
%
% 构建顺序（依赖拓扑）：
%   1. Interface 层（7 个 Bus 契约字典，叶子节点）
%   2. Platform  层（Airframe / Propulsion 物理参数）
%   3. Module    层（各功能模块字典，依赖 Interface + Platform）
%   4. Simulation 层（仿真配置）
%
% 新增/修改模块参数只需编辑 manifest.yaml，不再需要写 create_*.m。

scriptDir = fileparts(mfilename('fullpath'));
dataRoot  = fullfile(scriptDir, '..');            % 1_Data_Dictionaries/
projectRoot = fullfile(dataRoot, '..');
toolDir   = fullfile(projectRoot, '5_Tool');

addpath(toolDir);
addpath(genpath(dataRoot));

manifestPath = fullfile(scriptDir, 'manifest.yaml');
fprintf('读取清单: %s\n', manifestPath);
manifest = yaml.loadFile(manifestPath, 'ConvertToArray', true);

% ---- 1. Interface 层 ----
fprintf('\n=== [1/4] Interface 契约字典 ===\n');
for i = 1:numel(manifest.interface)
    item = manifest.interface(i);
    build_contract_dict(dataRoot, item);
end

% ---- 2. Platform 层 ----
fprintf('\n=== [2/4] Platform 平台参数字典 ===\n');
for i = 1:numel(manifest.platform)
    item = manifest.platform(i);
    build_param_dict(dataRoot, item, 'platform');
end

% ---- 3. Module 层 ----
fprintf('\n=== [3/4] Module 模块字典 ===\n');
for i = 1:numel(manifest.module)
    item = manifest.module(i);
    build_param_dict(dataRoot, item, 'module');
end

% ---- 4. Simulation 层 ----
fprintf('\n=== [4/4] Simulation 仿真配置 ===\n');
for i = 1:numel(manifest.simulation)
    item = manifest.simulation(i);
    build_param_dict(dataRoot, item, 'simulation');
end

fprintf('\n全部字典构建完成。\n');
end


% =========================================================================
% 构建一个 Interface 契约字典（从 config_*.m + 可选 contract_values.yaml）
% =========================================================================
function build_contract_dict(dataRoot, item)
domain = char(item.domain);
busDir = fullfile(dataRoot, char(item.bus_config_dir));
outPath = fullfile(dataRoot, char(item.output));

files = dir(fullfile(busDir, 'config_*.m'));
if isempty(files)
    error('build_all:NoBusConfig', '域 %s 下没有 config_*.m', domain);
end

configs = cell(numel(files), 1);
names = strings(0);
for j = 1:numel(files)
    [~, fn] = fileparts(files(j).name);
    cfg = feval(fn);
    if ~isfield(cfg, 'contractDomain') || ~strcmp(cfg.contractDomain, domain)
        error('build_all:WrongDomain', '%s 声明域不是 %s', fn, domain);
    end
    if any(names == string(cfg.busName))
        error('build_all:DuplicateBus', 'Bus %s 重复定义', cfg.busName);
    end
    names(end+1) = string(cfg.busName); %#ok<AGROW>
    configs{j} = cfg;
end

checkAndCloseDictionary(outPath);
if isfile(outPath), delete(outPath); end
dict = Simulink.data.dictionary.create(outPath);
dd = getSection(dict, 'Design Data');
for j = 1:numel(configs)
    cfg = configs{j};
    addBusFromCell(dd, cfg.busName, cfg.description, cfg.elements);
end

% 可选的共享枚举值
valueCount = 0;
if ~isempty(item.values_yaml) && char(item.values_yaml) ~= ""
    valuesPath = fullfile(dataRoot, char(item.values_yaml));
    if isfile(valuesPath)
        [values, ~] = readParamsFromYaml(valuesPath);
        addVarsFromCell(dd, values);
        valueCount = size(values, 1);
    end
end

saveChanges(dict); close(dict);
fprintf('  %-12s: %2d Bus, %2d 值 -> %s\n', domain, numel(configs), valueCount, char(item.output));
end


% =========================================================================
% 构建一个参数字典（Platform / Module / Simulation 通用）
% =========================================================================
function build_param_dict(dataRoot, item, layer)
dictName = char(item.name);
outPath = fullfile(dataRoot, char(item.output));

checkAndCloseDictionary(outPath);
if isfile(outPath), delete(outPath); end
dict = Simulink.data.dictionary.create(outPath);

% 挂依赖
deps = {};
if ~isempty(item.depends_on)
    deps = cellstr(item.depends_on);
end
for k = 1:numel(deps)
    dict.addDataSource(deps{k});
end

dd = getSection(dict, 'Design Data');
paramCount = 0;

% 读 yaml 参数（空壳字典没有 yaml）
if ~isempty(item.yaml) && char(item.yaml) ~= ""
    yamlPath = fullfile(dataRoot, char(item.yaml));
    if ~isfile(yamlPath)
        error('build_all:YamlMissing', '%s 找不到 yaml: %s', dictName, yamlPath);
    end
    [vars, ~] = readParamsFromYaml(yamlPath);
    addVarsFromCell(dd, vars);
    paramCount = size(vars, 1);
end

% 特殊后处理 hook
if ~isempty(item.hook) && char(item.hook) ~= ""
    hookFn = str2func(char(item.hook));
    hookFn(dict, dd, dataRoot);
end

saveChanges(dict); close(dict);
fprintf('  %-14s: %2d 参数, %d 依赖 -> %s\n', dictName, paramCount, numel(deps), char(item.output));
end


% =========================================================================
% Hook: Navigator 派生采样时间（从 FlightControl 的 TC_RATE_CONTROL_TS）
% =========================================================================
function derive_nav_sample_time(dict, dd, dataRoot)
fcYaml = fullfile(dataRoot, 'Module', 'FlightControl', 'flight_control_params.yaml');
[fcParams, ~] = readParamsFromYaml(fcYaml);
row = fcParams(strcmp(fcParams(:,1), 'TC_RATE_CONTROL_TS'), :);
if isempty(row)
    error('build_all:Hook', 'flight_control_params.yaml 里找不到 TC_RATE_CONTROL_TS');
end
navSample = row(1, :);
navSample{1} = 'NAV_SAMPLE_TIME';
navSample{4} = 'Auto';
navSample{5} = 'Derived from flight_control_params.yaml TC_RATE_CONTROL_TS';
addVarsFromCell(dd, navSample);
fprintf('    [hook] NAV_SAMPLE_TIME = %.6g s\n', navSample{2});
end


% =========================================================================
% Hook: MissionManager 创建 6 个空 Bus 初始值结构体
% =========================================================================
function mission_manager_empty_buses(dict, dd, ~)
busInit = {
    'MM_EMPTY_RESULT',         'MissionProcessingResultBus';
    'MM_EMPTY_PLAN',           'MissionPlanBus';
    'MM_EMPTY_CONTRACT',       'NavigationContractBus';
    'MM_EMPTY_STATUS',         'MissionStatusBus';
    'MM_EMPTY_PLAN_AVAILABLE',  'MissionPlanAvailableBus';
    'MM_EMPTY_PROGRESS',       'MissionProgressBus';
    };
for k = 1:size(busInit,1)
    val = Simulink.Bus.createMATLABStruct(busInit{k,2}, [], 1, dict);
    addEntry(dd, busInit{k,1}, val);
end
fprintf('    [hook] 生成 %d 个 MM_EMPTY_* 初始值结构体\n', size(busInit,1));
end


% =========================================================================
% Hook: Gateway 创建 12 个消息 ground value + MissionItemsBus
% =========================================================================
function gateway_ground_values(dict, dd, ~)
msgBuses = {
    'GatewayCommandAckTxRequestBus'; 'GatewayHeartbeatTxRequestBus';
    'GatewayMissionTxRequestBus';    'GatewayAttitudeTxRequestBus';
    'GatewayTimeSyncTxRequestBus';   'GatewayLocalPositionTxRequestBus';
    'GatewayGlobalPositionTxRequestBus'; 'GatewayPositionTargetTxRequestBus';
    'GatewayActuatorTxRequestBus';  'GatewayVfrHudTxRequestBus';
    'GatewayGpsRawTxRequestBus';     'GatewaySysStatusTxRequestBus';
    };
for i = 1:numel(msgBuses)
    gv = Simulink.Bus.createMATLABStruct(msgBuses{i}, [], 1, dict);
    addEntry(dd, [msgBuses{i} '_Ground'], gv);
end
addEntry(dd, 'GATE_EMPTY_MISSION', ...
    Simulink.Bus.createMATLABStruct('MissionItemsBus', [], 1, dict));
fprintf('    [hook] 生成 %d 个 Gateway ground value\n', numel(msgBuses)+1);
end
