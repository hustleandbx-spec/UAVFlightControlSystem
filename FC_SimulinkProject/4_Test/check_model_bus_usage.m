function check_model_bus_usage()
%CHECK_MODEL_BUS_USAGE L3 检查：各 .slx 模型引用的 Bus 类型名 vs 新字典（21 条总线）。
%   目的：识别 ① 模型引用了新字典中的哪些 Bus（覆盖面）② 废弃/已改名总线（FlightCmdBus/
%         NavigationObjectiveBus/GPS_BUS/IMU_BUS）在模型中的残留——残留 = 模型重构待办。
%   方法：解压 .slx 提取 XML 文本，检索 Bus 类型名出现情况（与 test_navigator_contract.m 同型）。
%   输出：逐模型 → 引用新字典 Bus 清单 + 废弃残留清单。

root = 'D:/Project/UAVSingleFlightControl/FC_SimulinkProject';
newBuses = { ...
    'NavigationContractBus','TrajectorySetpointBus','StateEstBus','StateSnapshot', ...
    'MissionPlanBus','MissionPlanAvailableBus','CommanderRequestBus','CommandAckBus', ...
    'CommanderStatusBus','MissionControlRequestBus','MissionStatusBus','NavigationStatusBus', ...
    'EscCmdBus','GPSBus','IMUBus','DynamicModelBus','ThrustTorqueBus','RuntimeTruthBus', ...
    'ExperimentTraceBus','RouteWindowBus','GcsLinkStatusBus' };
deprecated = { 'FlightCmdBus','NavigationObjectiveBus','GPS_BUS','IMU_BUS' };

% 收集 2_Model 与 3_Integration 下所有 .slx
modelFiles = {};
for d = { '2_Model', '3_Integration' }
    fd = dir(fullfile(root, d{1}, '**', '*.slx'));
    for i = 1:numel(fd)
        modelFiles{end+1} = fullfile(fd(i).folder, fd(i).name); %#ok<AGROW>
    end
end

fprintf('=== L3 模型 Bus 引用检查（%d 个模型）===\n', numel(modelFiles));
for m = 1:numel(modelFiles)
    [~, mdl, ~] = fileparts(modelFiles{m});
    text = extractAllSlxSystemText(modelFiles{m});
    refs = {};
    for b = 1:numel(newBuses)
        if contains(text, newBuses{b}), refs{end+1} = newBuses{b}; end %#ok<AGROW>
    end
    depRefs = {};
    for i = 1:numel(deprecated)
        if contains(text, deprecated{i}), depRefs{end+1} = deprecated{i}; end %#ok<AGROW>
    end
    fprintf('[%s]\n', mdl);
    fprintf('  引用新字典: %s\n', joinList(refs));
    fprintf('  废弃残留:   %s\n', joinList(depRefs));
end
fprintf('=== L3 检查完成 ===\n');
end

function s = joinList(items)
if isempty(items), s = '(无)'; return; end
s = strjoin(items, ', ');
end

function text = extractAllSlxSystemText(slxPath)
tmpDir = tempname;
mkdir(tmpDir);
cleanup = onCleanup(@() rmdir(tmpDir, 's'));
unzip(slxPath, tmpDir);
files = dir(fullfile(tmpDir, 'simulink', 'systems', '*.xml'));
parts = strings(numel(files) + 1, 1);
parts(1) = string(fileread(fullfile(tmpDir, 'simulink', 'blockdiagram.xml')));
for i = 1:numel(files)
    parts(i + 1) = string(fileread(fullfile(files(i).folder, files(i).name)));
end
text = join(parts, newline);
clear cleanup;
end
