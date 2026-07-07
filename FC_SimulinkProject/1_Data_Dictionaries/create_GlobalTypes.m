function create_GlobalTypes()
% 扫描 BusConfig/ 目录下所有 config_*.m 配置文件，生成 GlobalTypes.sldd
% 字典生成位置：本脚本所在目录 (1_Data_Dictionaries/)

scriptDir = fileparts(mfilename('fullpath'));
configDir = fullfile(scriptDir, 'BusConfig');
dictName  = fullfile(scriptDir, 'GlobalTypes.sldd');

checkAndCloseDictionary(dictName);

% ---- 扫描配置文件 ----
fileList = dir(fullfile(configDir, 'config_*.m'));

busConfigs = struct();
for i = 1:numel(fileList)
    [~, funcName, ~] = fileparts(fileList(i).name);
    try
        cfg = feval(funcName);
    catch ME
        warning('跳过 %s，执行出错: %s', funcName, ME.message);
        continue;
    end

    if ~isfield(cfg, 'busName') || ~isfield(cfg, 'elements')
        warning('配置文件 %s 缺少 busName 或 elements 字段，已跳过', funcName);
        continue;
    end

    busName = cfg.busName;
    if isfield(busConfigs, busName)
        warning('总线名 %s 重复定义，将覆盖之前的配置', busName);
    end
    busConfigs.(busName) = cfg;
end

if isempty(fieldnames(busConfigs))
    error('没有有效的总线配置，字典创建终止');
end

% ---- 创建数据字典 ----
if exist(dictName, 'file')
    warning('字典 %s 已存在，将删除后重建。', dictName);
    delete(dictName);
end

ddObj = Simulink.data.dictionary.create(dictName);
dData = getSection(ddObj, 'Design Data');

busNames = fieldnames(busConfigs);
for i = 1:numel(busNames)
    name = busNames{i};
    cfg = busConfigs.(name);
    addBusFromCell(dData, name, cfg.description, cfg.elements);
end

saveChanges(ddObj);
close(ddObj);
fprintf('全局类型字典 %s 创建完成，共 %d 条总线。\n', dictName, numel(busNames));
end
