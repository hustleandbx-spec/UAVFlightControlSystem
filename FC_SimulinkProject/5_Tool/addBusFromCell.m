%% 辅助函数：根据元胞数组创建总线并添加到字典
function addBusFromCell(dData, busName, description, elementTable)
    bus = Simulink.Bus;
    bus.Description = description;

    for i = 1:size(elementTable, 1)
        e = Simulink.BusElement;
        e.Name        = elementTable{i,1};
        e.DataType    = elementTable{i,2};
        e.Dimensions  = elementTable{i,3};
        e.Unit        = elementTable{i,4};
        e.Description = elementTable{i,5};
        bus.Elements(end+1) = e;        
    end

    addEntry(dData, busName, bus);
end