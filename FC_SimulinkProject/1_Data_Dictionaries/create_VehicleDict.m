function create_VehicleDict()
% 从 ParamSources/vehicle_params.yaml 生成 VehicleDict.sldd
% 字典生成位置：本脚本所在目录 (1_Data_Dictionaries/)

scriptDir = fileparts(mfilename('fullpath'));
dictName  = fullfile(scriptDir, 'VehicleDict.sldd');
yamlFile  = fullfile(scriptDir, 'ParamSources', 'vehicle_params.yaml');

% 添加 5_Tool 到路径
toolDir = fullfile(scriptDir, '..', '5_Tool');
addpath(toolDir);

checkAndCloseDictionary(dictName);

% ---- 从 YAML 读取参数 ----
[VarTable, meta] = readParamsFromYaml(yamlFile);
fprintf('从 %s 读取 %d 个参数。\n', yamlFile, size(VarTable, 1));

% ---- 创建数据字典 ----
ddObj = Simulink.data.dictionary.create(dictName);
dData = getSection(ddObj, 'Design Data');

addVarsFromCell(dData, VarTable);

saveChanges(ddObj);
close(ddObj);
fprintf('字典 %s 创建完成（无外部引用），共 %d 个参数。\n', dictName, size(VarTable, 1));
end
