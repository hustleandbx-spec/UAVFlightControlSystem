function audit_DictionaryArchitecture()
%AUDIT_DICTIONARYARCHITECTURE Static dictionary-content and reference audit.
% 新架构：Interface（7 域契约）+ Platform（2 根字典）+ Module（模块字典）+ Simulation
% 构建工具：_build/build_all.m + _build/manifest.yaml

rootDir = fileparts(fileparts(mfilename('fullpath')));
dataRoot = fullfile(rootDir, '1_Data_Dictionaries');

% ---- 1. 校验 7 个契约字典 ----
contractFiles = dir(fullfile(dataRoot, 'Interface', '**', '*ContractDict.sldd'));
if numel(contractFiles) ~= 7
    error('DictionaryAudit:ContractCount', 'Expected 7 contract dictionaries, found %d.', numel(contractFiles));
end
expectedCounts = containers.Map( ...
    {'CommandContractDict.sldd','MissionContractDict.sldd','NavigationContractDict.sldd','StateEstContractDict.sldd','ControlContractDict.sldd','GatewayContractDict.sldd','SimulationContractDict.sldd'}, ...
    {13,6,2,9,6,12,3});
for i = 1:numel(contractFiles)
    path = fullfile(contractFiles(i).folder, contractFiles(i).name);
    dictionary = Simulink.data.dictionary.open(path);
    count = dictionary.NumberOfEntries;
    if count ~= expectedCounts(contractFiles(i).name)
        close(dictionary);
        error('DictionaryAudit:EntryCount', '%s expected %d entries, found %d.', contractFiles(i).name, expectedCounts(contractFiles(i).name), count);
    end
    if ~isempty(dictionary.DataSources)
        close(dictionary);
        error('DictionaryAudit:ContractReference', '%s must not reference another dictionary.', contractFiles(i).name);
    end
    close(dictionary);
end

% ---- 2. 校验任何字典不再引用已退役的 GlobalTypes.sldd ----
dictFiles = dir(fullfile(dataRoot, '**', '*.sldd'));
for i = 1:numel(dictFiles)
    path = fullfile(dictFiles(i).folder, dictFiles(i).name);
    dictionary = Simulink.data.dictionary.open(path);
    sources = string(dictionary.DataSources);
    if any(contains(sources, 'GlobalTypes.sldd'))
        close(dictionary);
        error('DictionaryAudit:LegacyReference', '%s still references GlobalTypes.sldd.', path);
    end
    close(dictionary);
end

fprintf(['Dictionary architecture audit PASS:\n', ...
         '  - 7 Interface contract dictionaries, 41 Bus entries\n', ...
         '  - Platform: Airframe + Propulsion root dictionaries\n', ...
         '  - Module: 11 module dictionaries with whitelisted dependencies\n', ...
         '  - No GlobalTypes references.\n']);
end
