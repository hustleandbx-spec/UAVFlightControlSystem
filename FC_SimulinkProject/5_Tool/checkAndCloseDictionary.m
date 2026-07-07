function checkAndCloseDictionary(dictName)
% 检查并安全删除指定的 Simulink 数据字典文件
% dictName: 字典文件路径（绝对路径或相对路径）

if ~exist(dictName, 'file')
    fprintf('字典文件 "%s" 不存在，无需操作。\n', dictName);
    return;
end

% getOpenDictionaryPaths 需要文件名而非完整路径
[~, name, ext] = fileparts(dictName);
fileName = [name ext];

objectPath = Simulink.data.dictionary.getOpenDictionaryPaths(fileName);

if ~isempty(objectPath)
    try
        Simulink.data.dictionary.closeAll();
        fprintf('字典 %s 处于打开状态，已关闭。\n', fileName);
    catch ME
        error('关闭字典 "%s" 失败: %s', fileName, ME.message);
    end
else
    fprintf('字典 %s 未处于打开状态。\n', fileName);
end

try
    delete(dictName);
    fprintf('已删除字典文件: %s\n', dictName);
catch ME
    error('删除字典文件 "%s" 失败: %s', dictName, ME.message);
end
end
