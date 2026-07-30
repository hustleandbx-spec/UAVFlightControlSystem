function create_FlightControlDict()
% 从 ParamSources/flight_control_params.yaml 生成 FlightControlDict.sldd
% 字典生成位置：本脚本所在目录 (2_Model/control/)
% 引用: GlobalTypes.sldd, VehicleDict.sldd, PowerSystemDict.sldd

scriptDir = fileparts(mfilename('fullpath'));
dictName  = fullfile(scriptDir, 'FlightControlDict.sldd');
yamlFile  = fullfile(scriptDir, '..', '..', '1_Data_Dictionaries', 'ParamSources', 'flight_control_params.yaml');

% 添加 5_Tool 到路径
toolDir = fullfile(scriptDir, '..', '..', '5_Tool');
addpath(toolDir);

checkAndCloseDictionary(dictName);

% ---- 从 YAML 读取参数 ----
[VarTable, meta] = readParamsFromYaml(yamlFile);
fprintf('从 %s 读取 %d 个参数。\n', yamlFile, size(VarTable, 1));

% ---- 创建数据字典 ----
ddObj = Simulink.data.dictionary.create(dictName);

% 引用全局字典
globalDictDir = fullfile(scriptDir, '..', '..', '1_Data_Dictionaries');
addpath(globalDictDir);
powerSystemDir = fullfile(scriptDir, '..', 'power_system');
addpath(powerSystemDir);
ddObj.addDataSource('GlobalTypes.sldd');
ddObj.addDataSource('VehicleDict.sldd');
ddObj.addDataSource('PowerSystemDict.sldd');

dData = getSection(ddObj, 'Design Data');
addVarsFromCell(dData, VarTable);

saveChanges(ddObj);
close(ddObj);
fprintf('字典 %s 创建完成（引用 GlobalTypes.sldd, VehicleDict.sldd, PowerSystemDict.sldd），共 %d 个参数。\n', ...
    dictName, size(VarTable, 1));
end
