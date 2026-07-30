function generate_flightcore_ros2_messages()
%GENERATE_FLIGHTCORE_ROS2_MESSAGES 生成 FlightCore 与 Gazebo 联合仿真 ROS2 接口。
%
% 本目录同时包含产品边界 flightcore_msgs 和 harness 边界
% flightcore_gazebo_msgs。ros2genmsg 统一生成两包的 MATLAB 支持文件，
% 但 Gazebo 专用服务只能由 3_Integration/Gazebo 使用，不能进入 FlightCore。
%
% SensorBatchPublished/ObservationReady 是两个不同的屏障：
% 前者只表示 Gazebo 完成本拍传感器 publish，后者表示 generated FlightCore
% 的异步订阅回调已经缓存 required_mask。不得重新合并为 SensorReleased，
% 否则 Coordinator 无法区分生产端完成与消费端完成。

ros2Dir = fileparts(mfilename('fullpath'));

% Windows 的 ROS2 introspection 目标会生成很深的对象路径。直接在
% 3_Integration/ROS2 下构建会超过 MSVC 路径上限，因此把两份权威接口源
% 同步到仓库根部短目录 .r2g 后生成；该目录只含机械复制和生成物。
repositoryRoot = fileparts(fileparts(fileparts(ros2Dir)));
stagingDir = fullfile(repositoryRoot, '.r2g');
if ~isfolder(stagingDir)
    mkdir(stagingDir);
end
packageNames = { ...
    'flightcore_msgs', ...
    'flightcore_gazebo_msgs'};
for i = 1:numel(packageNames)
    sourcePackage = fullfile(ros2Dir, packageNames{i});
    stagedPackage = fullfile(stagingDir, packageNames{i});
    if ~isfolder(stagedPackage)
        mkdir(stagedPackage);
    end
    copyfile(fullfile(sourcePackage, '*'), stagedPackage, 'f');
end

% MATLAB R2025b 检测到 %USERPROFILE%\source 已存在时会拒绝执行 ros2genmsg。
% 生成期间临时使用 matlab_msg_gen/home 作为隔离 Home；onCleanup 保证无论
% 生成成功还是报错，都会恢复原来的 USERPROFILE 和 HOME。
isolatedHome = fullfile(stagingDir, 'matlab_msg_gen', 'home');
if ~isfolder(isolatedHome)
    mkdir(isolatedHome);
end
originalUserProfile = getenv('USERPROFILE');
originalHome = getenv('HOME');
originalBuildParallelLevel = getenv('CMAKE_BUILD_PARALLEL_LEVEL');
environmentCleanup = onCleanup(@() restoreEnvironment( ...
    originalUserProfile, originalHome, originalBuildParallelLevel));
setenv('USERPROFILE', isolatedHome);
setenv('HOME', isolatedHome);
% ros2genmsg invokes Ninja. Building all generated type-support translation
% units concurrently can exhaust the MSVC compiler heap on this workstation.
setenv('CMAKE_BUILD_PARALLEL_LEVEL', '1');

% 从短路径 staging 中的标准 ROS2 接口包生成 MATLAB 支持文件。
ros2genmsg(stagingDir);

% 验证全部 FlightCore 自定义消息都能被 MATLAB 正常构造。
messageTypes = {
    'flightcore_msgs/Imu'
    'flightcore_msgs/Gps'
    'flightcore_msgs/FlightCmd'
    'flightcore_msgs/EscCmd'
    'flightcore_msgs/StateEst'
    'flightcore_msgs/SystemHealth'
    'flightcore_gazebo_msgs/ActuatorCommand'
    'flightcore_gazebo_msgs/CommandCached'
    'flightcore_gazebo_msgs/PlantStepDone'
    'flightcore_gazebo_msgs/ResultReady'
    'flightcore_gazebo_msgs/CommitRelease'
    'flightcore_gazebo_msgs/SensorBatchPublished'
    'flightcore_gazebo_msgs/ObservationReady'
    };

for i = 1:numel(messageTypes)
    ros2message(messageTypes{i});
end

% 服务类型不需要真实服务端即可构造客户端和空请求，用于验证注册结果。
validationNode = ros2node('/flightcore_message_generation_check');
primeClient = ros2svcclient(validationNode, ...
    '/flightcore_message_generation_check/prime', ...
    'flightcore_gazebo_msgs/PrimeSession');
ros2message(primeClient);
clear primeClient validationNode

disp('FlightCore 与 Gazebo ROS2 消息/服务生成并验证成功。');

% 立即执行清理函数，恢复本次调用前的进程环境。
clear environmentCleanup
end

function restoreEnvironment(userProfile, homeDirectory, buildParallelLevel)
%RESTOREENVIRONMENT 恢复 ros2genmsg 调用前的用户目录环境变量。
setenv('USERPROFILE', userProfile);
setenv('HOME', homeDirectory);
setenv('CMAKE_BUILD_PARALLEL_LEVEL', buildParallelLevel);
end
