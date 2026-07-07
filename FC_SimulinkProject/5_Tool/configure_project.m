function configure_project()
% 配置 Simulink Project 工程管理环境
% 自动扫描项目目录结构，注册文件夹、添加快捷方式、建立标签体系

proj = currentProject();
rootDir = proj.RootFolder;

fprintf('=== 配置 Simulink Project: %s ===\n', proj.Name);

%% 1. 自动注册源码文件夹到项目
% 排除缓存目录和项目元数据目录，其余全部加入项目
excludeDirs = {'.cache', 'slprj', 'derived', 'resources', '.claude', '.git'};
items = dir(rootDir);
for i = 1:numel(items)
    name = items(i).name;
    if items(i).isdir && ~startsWith(name, '.') && ~ismember(name, excludeDirs)
        try
            addFolderIncludingChildFiles(proj, name);
            fprintf('  + %s\n', name);
        catch
        end
    end
end
fprintf('文件夹注册完成\n');

%% 2. 配置启动与关闭文件
try
    addStartupFile(proj, 'start.m');
    fprintf('启动: start.m\n');
catch
end
try
    addShutdownFile(proj, 'shutdown.m');
    fprintf('关闭: shutdown.m\n');
catch
end

%% 3. 自动发现并创建快捷方式
% 扫描关键文件，按名称模式自动注册为快捷方式
shortcutPatterns = {
    'start.m',                  'Start',            '启动项目环境';
    'shutdown.m',               'Shutdown',         '清理项目环境';
    '5_Tool/configure_project.m','Configure',       '重新配置项目工程环境';
};

% 扫描 create_*.m 脚本
createFiles = dir(fullfile(rootDir, '**/create_*.m'));
for i = 1:numel(createFiles)
    relPath = strrep(createFiles(i).folder, [rootDir filesep], '');
    relPath = fullfile(relPath, createFiles(i).name);
    [~,name] = fileparts(createFiles(i).name);
    shortcutPatterns(end+1,:) = {relPath, ['Build ' name], ['生成: ' name]};
end

% 扫描顶层 .slx 文件
slxFiles = [dir(fullfile(rootDir, '*.slx')); ...
            dir(fullfile(rootDir, '3_Integration', '*.slx'))];
for i = 1:numel(slxFiles)
    relPath = strrep(slxFiles(i).folder, [rootDir filesep], '');
    relPath = fullfile(relPath, slxFiles(i).name);
    [~,name] = fileparts(slxFiles(i).name);
    shortcutPatterns(end+1,:) = {relPath, ['Open ' name], ['打开: ' name]};
end

for i = 1:size(shortcutPatterns, 1)
    if exist(fullfile(rootDir, shortcutPatterns{i,1}), 'file')
        try
            addShortcut(proj, shortcutPatterns{i,1}, shortcutPatterns{i,2});
        catch
        end
    end
end
fprintf('快捷方式创建完成\n');

%% 4. 建立标签体系
labelDefs = {'Algorithm', 'Interface', 'Param', 'Test', 'Doc'};
for i = 1:numel(labelDefs)
    try
        createLabel(proj, labelDefs{i}, 'Tag');
    catch
    end
end

% 自动标记：按目录/文件模式
% Algorithm: stateflow chart 和 MATLAB Function 所在的 .slx
try addLabel(proj, 'Algorithm', '2_Model/state_estimation/ESKF.slx'); catch, end
try addLabel(proj, 'Algorithm', '2_Model/control/UAV_FlightControl.slx'); catch, end
try addLabel(proj, 'Algorithm', '2_Model/dynamic_model/UAV_dynamic_model.slx'); catch, end
try addLabel(proj, 'Algorithm', '2_Model/power_systerm/power_systerm_model.slx'); catch, end

% Interface: BusConfig 下的所有配置脚本
busConfigs = dir(fullfile(rootDir, '1_Data_Dictionaries/BusConfig/config_*.m'));
for i = 1:numel(busConfigs)
    try addLabel(proj, 'Interface', fullfile('1_Data_Dictionaries/BusConfig', busConfigs(i).name)); catch, end
end

% Param: 所有 .sldd 字典文件
slddFiles = dir(fullfile(rootDir, '**/*.sldd'));
for i = 1:numel(slddFiles)
    relPath = strrep(slddFiles(i).folder, [rootDir filesep], '');
    try addLabel(proj, 'Param', fullfile(relPath, slddFiles(i).name)); catch, end
end

% Test: 4_Test 下的所有文件
testFiles = dir(fullfile(rootDir, '4_Test', '*'));
for i = 1:numel(testFiles)
    if ~testFiles(i).isdir
        try addLabel(proj, 'Test', fullfile('4_Test', testFiles(i).name)); catch, end
    end
end
fprintf('标签体系建立完成\n');

%% 5. 设置 Simulink 缓存文件夹
cacheDir = fullfile(rootDir, '.cache');
if ~exist(cacheDir, 'dir')
    mkdir(cacheDir);
end
Simulink.fileGenControl('set', ...
    'CacheFolder', cacheDir, ...
    'CodeGenFolder', fullfile(cacheDir, 'codegen'), ...
    'keepPreviousPath', false);
fprintf('缓存目录: .cache/\n');

fprintf('\n=== 项目配置完成 ===\n');
end
