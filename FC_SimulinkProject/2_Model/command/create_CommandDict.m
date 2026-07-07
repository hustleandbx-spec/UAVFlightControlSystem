function create_CommandDict()
%CREATE_COMMANDDICT Generate CommandDict.sldd from command_params.yaml.

scriptDir = fileparts(mfilename('fullpath'));
dictName  = fullfile(scriptDir, 'CommandDict.sldd');
yamlFile  = fullfile(scriptDir, '..', '..', '1_Data_Dictionaries', ...
    'ParamSources', 'command_params.yaml');

toolDir = fullfile(scriptDir, '..', '..', '5_Tool');
addpath(toolDir);

checkAndCloseDictionary(dictName);

[VarTable, meta] = readParamsFromYaml(yamlFile); %#ok<NASGU>
fprintf('从 %s 读取 %d 个参数。\n', yamlFile, size(VarTable, 1));

ddObj = Simulink.data.dictionary.create(dictName);

globalDictDir = fullfile(scriptDir, '..', '..', '1_Data_Dictionaries');
addpath(globalDictDir);
ddObj.addDataSource('GlobalTypes.sldd');

dData = getSection(ddObj, 'Design Data');
addVarsFromCell(dData, VarTable);

saveChanges(ddObj);
close(ddObj);
fprintf('字典 %s 创建完成（引用 GlobalTypes.sldd），共 %d 个参数。\n', ...
    dictName, size(VarTable, 1));
end
