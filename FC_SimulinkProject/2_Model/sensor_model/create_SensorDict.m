function create_SensorDict()
% create_SensorDict  生成 SensorDict.sldd（传感器模型噪声与配置参数）
% 字典生成位置：本脚本所在目录 (2_Model/sensor_model/)
% 引用: GlobalTypes.sldd, VehicleDict.sldd

scriptDir = fileparts(mfilename('fullpath'));
dictName  = fullfile(scriptDir, 'SensorDict.sldd');
yamlFile  = fullfile(scriptDir, '..', '..', '1_Data_Dictionaries', ...
    'ParamSources', 'sensor_params.yaml');

% 添加 5_Tool 到路径
toolDir = fullfile(scriptDir, '..', '..', '5_Tool');
addpath(toolDir);

checkAndCloseDictionary(dictName);

% ---- 从 YAML 唯一参数源读取 ----
[VarTable, meta] = readParamsFromYaml(yamlFile); %#ok<NASGU>
fprintf('从 %s 读取 %d 个参数。\n', yamlFile, size(VarTable, 1));

% ---- 创建数据字典 ----
ddObj = Simulink.data.dictionary.create(dictName);

% 引用全局字典
globalDictDir = fullfile(scriptDir, '..', '..', '1_Data_Dictionaries');
addpath(globalDictDir);
ddObj.addDataSource('GlobalTypes.sldd');
ddObj.addDataSource('VehicleDict.sldd');

dData = getSection(ddObj, 'Design Data');
addVarsFromCell(dData, VarTable);

saveChanges(ddObj);
close(ddObj);
fprintf('字典 %s 创建完成（引用 GlobalTypes.sldd, VehicleDict.sldd），共 %d 个参数。\n', ...
    dictName, size(VarTable, 1));
end
