function checkBusConfig(busName, expectedElements, forbiddenFields)
%CHECKBUSCONFIG 校验 config_<BusName>.m 与 A2 契约一致（纯文本检查，无需 MATLAB 运行模型）。
%
%   busName           — Bus 类型名，对应文件 1_Data_Dictionaries/BusConfig/config_<BusName>.m
%   expectedElements  — 期望字段 cell { {name,type,dims}, {name,type,dims}, ... }，逐项必须出现
%   forbiddenFields   — 禁止字段名 cell { 'f1', 'f2', ... }，一个都不允许出现
%
% 通过判据：每个期望字段以 "name,type,dims" 紧凑形式出现；每个禁止字段名不得出现。
% 失败时抛异常（assert）；通过时打印 BUS_CONTRACT_<BUSNAME>_PASS。
%
% 用法示例：
%   checkBusConfig('NavigationContractBus', ...
%       { {'ContractId','uint32',1}, {'PassPositionNED','single','[64 3]'} }, ...
%       { 'AcceptanceRadius', 'ItemType' });

projectRoot = fileparts(fileparts(fileparts(mfilename('fullpath'))));  % bus_contracts -> 4_Test -> FC_SimulinkProject
busFile = fullfile(projectRoot, '1_Data_Dictionaries', 'BusConfig', ...
    sprintf('config_%s.m', busName));
assert(isfile(busFile), '缺少 config_%s.m 总线定义文件。', busName);

text = fileread(busFile);
compact = regexprep(text, '\s+', '');   % 去空白，使字段匹配对空格健壮

for i = 1:numel(expectedElements)
    e = expectedElements{i};
    name = e{1};
    typ  = e{2};
    dimsVal = e{3};
    if isnumeric(dimsVal)
        dims = num2str(dimsVal);
    else
        dims = char(dimsVal);
    end
    dims = regexprep(dims, '\s+', '');   % 与 compact 同步去空白（如 [64 3]→[643]）
    pat = sprintf("'%s','%s',%s", name, typ, dims);
    assert(contains(compact, pat), ...
        'Bus %s 缺少契约字段 %s（期望 %s）。', busName, name, pat);
end

for i = 1:numel(forbiddenFields)
    fname = forbiddenFields{i};
    assert(~contains(compact, sprintf("'%s'", fname)), ...
        'Bus %s 出现禁止字段 %s（判据泄漏/任务语义/旧字段）。', busName, fname);
end

fprintf('BUS_CONTRACT_%s_PASS\n', upper(busName));
end
