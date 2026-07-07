function [VarTable, meta] = readParamsFromYaml(yamlFilePath)
% readParamsFromYaml  从 YAML 参数源文件读取参数定义，返回 VarTable 元胞数组。
%
% 输入:
%   yamlFilePath - YAML 文件路径（字符串）
%
% 输出:
%   VarTable - 元胞数组 {Name, Value, DataType, StorageClass, Description}
%              可直接传入 addVarsFromCell()
%   meta     - _meta 结构体（字典名、路径、引用等）
%
% YAML 格式要求:
%   _meta:
%     dict_name: "xxx.sldd"
%     dict_path: "..."
%     description: "..."
%     references: [Dict1.sldd, Dict2.sldd]
%
%   参数名:
%     type: single | uint8 | string         # 数据类型
%     default: <value>                      # 默认值
%     storage_class: ExportedGlobal         # 存储类
%     description: "说明文字"                # 参数说明
%     ...                                   # 其他可选字段（min/max/unit/group 等）
%
% 示例:
%   [VarTable, meta] = readParamsFromYaml('ParamSources/flight_control_params.yaml');

    % 检查项目固定的 MATLAB YAML 解析器是否已由 start.m 注册。
    if isempty(which('yaml.loadFile'))
        error(['yaml.loadFile 不可用。请确认项目依赖 ', ...
               'third_party/yaml 已安装，并通过 start.m 加入 MATLAB 路径。']);
    end

    % 读取 YAML
    try
        data = yaml.loadFile(yamlFilePath, 'ConvertToArray', true);
    catch ME
        error('读取 YAML 文件失败:\n  文件: %s\n  错误: %s', yamlFilePath, ME.message);
    end

    % 提取元数据
    metaField = matlab.lang.makeValidName('_meta');
    if isfield(data, metaField)
        meta = data.(metaField);
        data = rmfield(data, metaField);
    else
        error('YAML 文件缺少 _meta 段: %s', yamlFilePath);
    end

    % 提取参数名列表（排除 _meta）
    paramNames = fieldnames(data);
    nParams = numel(paramNames);
    VarTable = cell(nParams, 5);

    for i = 1:nParams
        name = paramNames{i};
        p = data.(name);

        % 校验必要字段
        required = {'type', 'default', 'description'};
        for f = 1:numel(required)
            if ~isfield(p, required{f})
                error('参数 "%s" 缺少必要字段 "%s"。文件: %s', name, required{f}, yamlFilePath);
            end
        end

        % === 列1: 参数名 ===
        VarTable{i, 1} = name;

        % === 列2: 默认值（按类型转换）===
        VarTable{i, 2} = convertValue(p.default, p.type);

        % === 列3: 数据类型 ===
        VarTable{i, 3} = p.type;

        % === 列4: 存储类（默认 ExportedGlobal）===
        if isfield(p, 'storage_class')
            VarTable{i, 4} = p.storage_class;
        else
            VarTable{i, 4} = 'ExportedGlobal';
        end

        % === 列5: 说明 ===
        VarTable{i, 5} = p.description;
    end
end

% =============================================================================
% 辅助函数：将 YAML 读入的 double 值转换为目标 Simulink 类型
% =============================================================================
function val = convertValue(defaultVal, dataType)
    switch dataType
        case 'single'
            val = single(defaultVal);
        case 'uint8'
            val = uint8(defaultVal);
        case 'uint16'
            val = uint16(defaultVal);
        case 'uint32'
            val = uint32(defaultVal);
        case 'int8'
            val = int8(defaultVal);
        case 'int16'
            val = int16(defaultVal);
        case 'int32'
            val = int32(defaultVal);
        case {'double', 'auto'}
            val = double(defaultVal);
        case 'string'
            val = string(defaultVal);
        case 'boolean'
            val = logical(defaultVal);
        otherwise
            warning('未知数据类型 "%s"，保持原始类型。参数值: %s', dataType, mat2str(defaultVal));
            val = defaultVal;
    end
end
