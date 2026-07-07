function build_GPS_sensor()
% build_GPS_sensor  程序化构建 sensor_GPS_model.slx
% 输入: DynamicModelBus, 输出: GPS_BUS (Lat, Lon, Alt, Velocity, Valid, Timestamp)
% 噪声参数来自 SensorDict.sldd
% 架构: NED->LLA -> RateTransition(1kHz->5Hz) -> DT(single) -> 噪声注入 -> BusCreator
%
% 注意: 噪声块在 RateTransition 之后运行(5Hz), Cov = σ² * Ts = σ² / GPS_SAMPLE_RATE

scriptDir = fileparts(mfilename('fullpath'));
modelName = 'sensor_GPS_model';
mdlFile   = fullfile(scriptDir, [modelName '.slx']);

if bdIsLoaded(modelName), close_system(modelName, 0); end

if exist(mdlFile, 'file')
    backupFile = fullfile(scriptDir, [modelName '_old.slx']);
    copyfile(mdlFile, backupFile, 'f');
    delete(mdlFile);
    fprintf('已备份旧模型到 %s\n', backupFile);
end

% ---- 创建新模型 ----
new_system(modelName);
open_system(modelName);

set_param(modelName, 'SolverType', 'Fixed-step');
set_param(modelName, 'Solver', 'FixedStepDiscrete');
set_param(modelName, 'FixedStep', '0.001');
set_param(modelName, 'StopTime', '20');
set_param(modelName, 'ReturnWorkspaceOutputs', 'on');
set_param(modelName, 'DataDictionary', 'SensorDict.sldd');

% ============================================================
% 端口
% ============================================================
add_block('simulink/Ports & Subsystems/In1', [modelName '/In']);
set_param([modelName '/In'], 'OutDataTypeStr', 'Bus: DynamicModelBus');

add_block('simulink/Ports & Subsystems/Out1', [modelName '/Out']);
set_param([modelName '/Out'], 'OutDataTypeStr', 'Bus: GPS_BUS');

% ============================================================
% Bus Selector
% ============================================================
add_block('simulink/Signal Routing/Bus Selector', [modelName '/BusSel']);
set_param([modelName '/BusSel'], 'OutputSignals', 'Position_NED,Velocity_NED');
add_line(modelName, 'In/1', 'BusSel/1');

% ============================================================
% Ned2Lla: NED(m) -> LLA(deg,deg,m)
% ============================================================
add_block('simulink/User-Defined Functions/MATLAB Function', [modelName '/Ned2Lla']);

% ============================================================
% Rate Transition: 1kHz -> GPS_SAMPLE_RATE (5Hz)
% ============================================================
add_block('simulink/Signal Attributes/Rate Transition', [modelName '/RT_Lat']);
add_block('simulink/Signal Attributes/Rate Transition', [modelName '/RT_Lon']);
add_block('simulink/Signal Attributes/Rate Transition', [modelName '/RT_Alt']);
add_block('simulink/Signal Attributes/Rate Transition', [modelName '/RT_Vel']);
add_block('simulink/Signal Attributes/Rate Transition', [modelName '/RT_Time']);

% ============================================================
% DataTypeConversion: double -> single (RateTransition 输出 double)
% ============================================================
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT_Lat']);
set_param([modelName '/DT_Lat'], 'OutDataTypeStr', 'single');
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT_Lon']);
set_param([modelName '/DT_Lon'], 'OutDataTypeStr', 'single');
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT_Alt']);
set_param([modelName '/DT_Alt'], 'OutDataTypeStr', 'single');
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT_Vel']);
set_param([modelName '/DT_Vel'], 'OutDataTypeStr', 'single');

% ============================================================
% 噪声注入 (5Hz 运行, Cov = σ² / GPS_SAMPLE_RATE)
% ============================================================
add_block('simulink/Sources/Band-Limited White Noise', [modelName '/NoiseLat']);
set_param([modelName '/NoiseLat'], 'Cov', 'GPS_POS_H_NOISE^2 / GPS_SAMPLE_RATE', ...
    'Ts', '1 / double(GPS_SAMPLE_RATE)', 'Seed', '45601');

add_block('simulink/Sources/Band-Limited White Noise', [modelName '/NoiseLon']);
set_param([modelName '/NoiseLon'], 'Cov', 'GPS_POS_H_NOISE^2 / GPS_SAMPLE_RATE', ...
    'Ts', '1 / double(GPS_SAMPLE_RATE)', 'Seed', '45602');

add_block('simulink/Sources/Band-Limited White Noise', [modelName '/NoiseAlt']);
set_param([modelName '/NoiseAlt'], 'Cov', 'GPS_POS_V_NOISE^2 / GPS_SAMPLE_RATE', ...
    'Ts', '1 / double(GPS_SAMPLE_RATE)', 'Seed', '45603');

add_block('simulink/Sources/Band-Limited White Noise', [modelName '/NoiseVel']);
set_param([modelName '/NoiseVel'], 'Cov', 'GPS_VEL_NOISE^2 / GPS_SAMPLE_RATE', ...
    'Ts', '1 / double(GPS_SAMPLE_RATE)', 'Seed', '45604');

noiseNames = {'NoiseLat', 'NoiseLon', 'NoiseAlt', 'NoiseVel'};
for noiseIndex = 1:numel(noiseNames)
    conversionName = [noiseNames{noiseIndex} 'Single'];
    add_block('simulink/Attributes/Data Type Conversion', ...
        [modelName '/' conversionName]);
    set_param([modelName '/' conversionName], 'OutDataTypeStr', 'single');
end

% ============================================================
% 噪声加法器
% ============================================================
add_block('simulink/Math Operations/Sum', [modelName '/LatAdd']);
set_param([modelName '/LatAdd'], 'Inputs', '++');
add_block('simulink/Math Operations/Sum', [modelName '/LonAdd']);
set_param([modelName '/LonAdd'], 'Inputs', '++');
add_block('simulink/Math Operations/Sum', [modelName '/AltAdd']);
set_param([modelName '/AltAdd'], 'Inputs', '++');
add_block('simulink/Math Operations/Sum', [modelName '/VelAdd']);
set_param([modelName '/VelAdd'], 'Inputs', '++');

% ============================================================
% 输出类型转换 (噪声输出 double -> single)
% ============================================================
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT2_Lat']);
set_param([modelName '/DT2_Lat'], 'OutDataTypeStr', 'single');
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT2_Lon']);
set_param([modelName '/DT2_Lon'], 'OutDataTypeStr', 'single');
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT2_Alt']);
set_param([modelName '/DT2_Alt'], 'OutDataTypeStr', 'single');
add_block('simulink/Attributes/Data Type Conversion', [modelName '/DT2_Vel']);
set_param([modelName '/DT2_Vel'], 'OutDataTypeStr', 'single');

add_block('simulink/Sources/Constant', [modelName '/Valid']);
set_param([modelName '/Valid'], ...
    'Value', 'true', ...
    'OutDataTypeStr', 'boolean', ...
    'SampleTime', '1 / double(GPS_SAMPLE_RATE)');

add_block('simulink/Sources/Clock', [modelName '/Timestamp']);
set_param([modelName '/Timestamp'], 'DisplayTime', 'off');

% ============================================================
% Bus Creator
% ============================================================
add_block('simulink/Signal Routing/Bus Creator', [modelName '/BusCreator']);
set_param([modelName '/BusCreator'], ...
    'Inputs', 'Lat,Lon,Alt,Velocity,Valid,Timestamp', ...
    'OutDataTypeStr', 'Bus: GPS_BUS', ...
    'NonVirtualBus', 'on', ...
    'InheritFromInputs', 'off');

% ============================================================
% 连线
% ============================================================
% 输入 -> BusSel -> Ned2Lla
add_line(modelName, 'BusSel/1', 'Ned2Lla/1');

% Ned2Lla -> RateTransition -> DT -> 噪声加法 -> DT2 -> BusCreator
add_line(modelName, 'Ned2Lla/1', 'RT_Lat/1');
add_line(modelName, 'Ned2Lla/2', 'RT_Lon/1');
add_line(modelName, 'Ned2Lla/3', 'RT_Alt/1');
add_line(modelName, 'BusSel/2',  'RT_Vel/1');
add_line(modelName, 'Timestamp/1',  'RT_Time/1');

add_line(modelName, 'RT_Lat/1', 'DT_Lat/1');
add_line(modelName, 'RT_Lon/1', 'DT_Lon/1');
add_line(modelName, 'RT_Alt/1', 'DT_Alt/1');
add_line(modelName, 'RT_Vel/1', 'DT_Vel/1');

add_line(modelName, 'DT_Lat/1', 'LatAdd/1');
add_line(modelName, 'NoiseLat/1', 'NoiseLatSingle/1');
add_line(modelName, 'NoiseLatSingle/1', 'LatAdd/2');
add_line(modelName, 'DT_Lon/1', 'LonAdd/1');
add_line(modelName, 'NoiseLon/1', 'NoiseLonSingle/1');
add_line(modelName, 'NoiseLonSingle/1', 'LonAdd/2');
add_line(modelName, 'DT_Alt/1', 'AltAdd/1');
add_line(modelName, 'NoiseAlt/1', 'NoiseAltSingle/1');
add_line(modelName, 'NoiseAltSingle/1', 'AltAdd/2');
add_line(modelName, 'DT_Vel/1', 'VelAdd/1');
add_line(modelName, 'NoiseVel/1', 'NoiseVelSingle/1');
add_line(modelName, 'NoiseVelSingle/1', 'VelAdd/2');

add_line(modelName, 'LatAdd/1', 'DT2_Lat/1');
add_line(modelName, 'LonAdd/1', 'DT2_Lon/1');
add_line(modelName, 'AltAdd/1', 'DT2_Alt/1');
add_line(modelName, 'VelAdd/1', 'DT2_Vel/1');

add_line(modelName, 'DT2_Lat/1', 'BusCreator/1');
add_line(modelName, 'DT2_Lon/1', 'BusCreator/2');
add_line(modelName, 'DT2_Alt/1', 'BusCreator/3');
add_line(modelName, 'DT2_Vel/1', 'BusCreator/4');
add_line(modelName, 'Valid/1', 'BusCreator/5');
add_line(modelName, 'RT_Time/1', 'BusCreator/6');

add_line(modelName, 'BusCreator/1', 'Out/1');

% ============================================================
% 设置 Ned2Lla 函数代码
% ============================================================
rt = sfroot;
chart = rt.find('-isa', 'Stateflow.EMChart', 'Path', [modelName '/Ned2Lla']);
chart.Script = sprintf([...
    'function [lat, lon, alt] = Ned2Lla(pos_ned)\n' ...
    '%%#codegen\n' ...
    '%% NED位置(m) -> LLA(deg, deg, m), 球面近似\n' ...
    '\n' ...
    'R_earth = single(6371000);\n' ...
    'lat_ref = single(0.0);     %% GPS_REF_LAT\n' ...
    'lon_ref = single(10.0);    %% GPS_REF_LON\n' ...
    'rad2deg_val = single(180/pi);\n' ...
    'deg2rad_val = single(pi/180);\n' ...
    '\n' ...
    'lat = lat_ref + (pos_ned(1) / R_earth) * rad2deg_val;\n' ...
    'cos_lat = cos(lat_ref * deg2rad_val);\n' ...
    'lon = lon_ref + (pos_ned(2) / (R_earth * cos_lat)) * rad2deg_val;\n' ...
    'alt = -pos_ned(3);\n' ...
    'end\n']);

save_system(modelName, mdlFile);
fprintf('GPS 传感器模型已保存: %s\n', mdlFile);
close_system(modelName, 0);
end
