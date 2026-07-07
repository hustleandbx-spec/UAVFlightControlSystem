function create_SimulationDict()
%CREATE_SIMULATIONDICT Generate SimulationDict.sldd from simulation_params.yaml.

scriptDir = fileparts(mfilename('fullpath'));
dictName  = fullfile(scriptDir, 'SimulationDict.sldd');
yamlFile  = fullfile(scriptDir, 'ParamSources', 'simulation_params.yaml');

toolDir = fullfile(scriptDir, '..', '5_Tool');
addpath(toolDir);

checkAndCloseDictionary(dictName);

[VarTable, meta] = readParamsFromYaml(yamlFile); %#ok<NASGU>
fprintf('从 %s 读取 %d 个参数。\n', yamlFile, size(VarTable, 1));

ddObj = Simulink.data.dictionary.create(dictName);
ddObj.addDataSource('GlobalTypes.sldd');
ddObj.addDataSource('VehicleDict.sldd');

dData = getSection(ddObj, 'Design Data');
addVarsFromCell(dData, VarTable);

saveChanges(ddObj);
close(ddObj);
fprintf(['字典 %s 创建完成（引用 GlobalTypes.sldd, VehicleDict.sldd），', ...
    '共 %d 个参数。\n'], dictName, size(VarTable, 1));
end
