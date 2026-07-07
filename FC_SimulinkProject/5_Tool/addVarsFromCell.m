%% 辅助函数：根据元胞数组创建变量并添加到字典
function addVarsFromCell(dData, varTable)
    % varTable 格式: {Name, Value, DataType, StorageClass, Description}
    for i = 1:size(varTable, 1)
        p = Simulink.Parameter;
        p.Value             = varTable{i,2};
        p.DataType          = varTable{i,3};
        p.CoderInfo.StorageClass = varTable{i,4};
        p.Description       = varTable{i,5};
        
        % 将变量添加到字典设计数据区
        addEntry(dData, varTable{i,1}, p);
    end
end