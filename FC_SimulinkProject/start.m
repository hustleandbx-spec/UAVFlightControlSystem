% start.m - 无人机飞控项目环境初始化
%
% 注意：打开 Simulink Project (.prj) 后，MATLAB 路径已由项目自动管理，
% 各子模型引用的数据字典也会自动加载。本脚本仅负责项目路径机制无法覆盖的
% 配置项。

% 将 Simulink 仿真缓存统一重定向到 .cache/，避免 slprj/ 分散污染源码目录
rootDir = fileparts(mfilename('fullpath'));
cacheDir = fullfile(rootDir, '.cache');
codegenDir = fullfile(cacheDir, 'codegen');

% YAML 参数是所有数据字典的唯一事实来源。注册固定版本的 MATLAB
% YAML 解析器，使 ParamSources/*.yaml -> readParamsFromYaml -> *.sldd
% 在干净环境中也能复现。
yamlDir = fullfile(rootDir, 'third_party', 'yaml');
if isfolder(fullfile(yamlDir, '+yaml'))
    addpath(yamlDir);
else
    error('缺少 YAML 参数链依赖: %s', yamlDir);
end

% 子系统字典按名称引用 GlobalTypes.sldd / VehicleDict.sldd；该目录必须在
% 整个 MATLAB 会话内保持可解析，不能由某个 create_*Dict 临时移除。
globalDictDir = fullfile(rootDir, '1_Data_Dictionaries');
addpath(globalDictDir);

% 闭环顶层使用 Model Reference。即使用户直接打开 UAV_FC_loop.slx，
% 不依赖 Simulink Project 元数据时，这些被引用模型也必须可解析。
modelRefDirs = {
    fullfile(rootDir, '3_Integration')
    fullfile(rootDir, '3_Integration', 'FlightCore')
    fullfile(rootDir, '3_Integration', 'Gazebo')
    fullfile(rootDir, '3_Integration', 'SimAdapter')
    fullfile(rootDir, '2_Model', 'command')
    fullfile(rootDir, '2_Model', 'commander')
    fullfile(rootDir, '2_Model', 'mission_manager')
    fullfile(rootDir, '2_Model', 'navigator')
    fullfile(rootDir, '2_Model', 'control')
    fullfile(rootDir, '2_Model', 'dynamic_model')
    fullfile(rootDir, '2_Model', 'power_system')
    fullfile(rootDir, '2_Model', 'sensor_model')
    fullfile(rootDir, '2_Model', 'state_estimation', 'EKF')
    fullfile(rootDir, '2_Model', 'state_estimation', 'ESKF')
    fullfile(rootDir, '2_Model', 'state_estimation', 'UKF')
};
for i = 1:numel(modelRefDirs)
    if isfolder(modelRefDirs{i})
        addpath(modelRefDirs{i});
    end
end

% 自定义 ROS2 接口在仓库根部短路径 .r2g 中生成，避免 Windows 对象路径
% 超限。这里只注册生成后的 MATLAB 接口；源定义仍位于 3_Integration/ROS2。
generatedRos2Dir = fullfile(fileparts(rootDir), '.r2g', ...
    'matlab_msg_gen', 'win64', 'install', 'm');
if isfolder(generatedRos2Dir)
    addpath(generatedRos2Dir);
end

% 确保缓存目录存在（首次启动或被清理后自动重建）
if ~exist(cacheDir, 'dir')
    mkdir(cacheDir);
end
if ~exist(codegenDir, 'dir')
    mkdir(codegenDir);
end

Simulink.fileGenControl('set', ...
    'CacheFolder', cacheDir, ...
    'CodeGenFolder', codegenDir, ...
    'keepPreviousPath', false);

% 初始化 Simulink Agentic Toolkit — 供 AI 编码助手通过 MCP 连接 MATLAB
try
    addpath(fullfile(getenv('USERPROFILE'), '.matlab', 'agentic-toolkits', 'simulink'));
    satk_initialize;
catch ME
    warning('Agentic Toolkit 初始化失败（不影响项目正常使用）: %s', ME.message);
end

disp('无人机飞控项目环境已加载');
