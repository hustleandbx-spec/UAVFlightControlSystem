function create_DynamicModelDict()
% 创建 DynamicModel 子系统的数据字典
% 字典生成位置：本脚本所在目录 (2_Model/dynamic_model/)
% 引用 1_Data_Dictionaries/ 下的 GlobalTypes.sldd 和 VehicleDict.sldd

scriptDir = fileparts(mfilename('fullpath'));
dictName  = fullfile(scriptDir, 'DynamicModelDict.sldd');

checkAndCloseDictionary(dictName);

ddObj = Simulink.data.dictionary.create(dictName);

globalDictDir = fullfile(scriptDir, '..', '..', '1_Data_Dictionaries');
addpath(globalDictDir);
ddObj.addDataSource('GlobalTypes.sldd');
ddObj.addDataSource('VehicleDict.sldd');

dData = getSection(ddObj, 'Design Data');

% 质量/inertia/g 已移至 VehicleDict.sldd
VarTable = {
};

addVarsFromCell(dData, VarTable);

saveChanges(ddObj);
close(ddObj);
fprintf('字典 %s 创建完成（引用 GlobalTypes.sldd, VehicleDict.sldd）。\n', dictName);
end
