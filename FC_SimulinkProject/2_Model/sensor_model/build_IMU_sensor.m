function build_IMU_sensor()
% build_IMU_sensor  程序化构建 sensor_IMU_model.slx
% 输入: DynamicModelBus, 输出: IMUBus (Accel, Gyro, Valid, Timestamp)
% 噪声参数来自 SensorDict.sldd

scriptDir = fileparts(mfilename('fullpath'));
modelName = 'sensor_IMU_model';
mdlFile   = fullfile(scriptDir, [modelName '.slx']);

% 关闭已打开的同名模型
if bdIsLoaded(modelName), close_system(modelName, 0); end

% 如果旧文件存在，备份后删除
if exist(mdlFile, 'file')
    backupFile = fullfile(scriptDir, [modelName '_old.slx']);
    copyfile(mdlFile, backupFile, 'f');
    delete(mdlFile);
    fprintf('已备份旧模型到 %s\n', backupFile);
end

% ---- 创建新模型 ----
new_system(modelName);
open_system(modelName);

% 求解器配置: 定步长离散, 1ms
set_param(modelName, 'SolverType', 'Fixed-step');
set_param(modelName, 'Solver', 'FixedStepDiscrete');
set_param(modelName, 'FixedStep', '1 / double(IMU_SAMPLE_RATE)');
set_param(modelName, 'StopTime', '20');
set_param(modelName, 'ReturnWorkspaceOutputs', 'on');

% 数据字典
ddPath = fullfile(scriptDir, 'SensorDict.sldd');
set_param(modelName, 'DataDictionary', 'SensorDict.sldd');

% ============================================================
% 端口
% ============================================================
add_block('simulink/Ports & Subsystems/Inport', [modelName '/in']);
set_param([modelName '/in'], 'PortDimensions', '1', 'OutDataTypeStr', 'Bus: DynamicModelBus');

add_block('simulink/Ports & Subsystems/Outport', [modelName '/out']);
set_param([modelName '/out'], 'PortDimensions', '1', 'OutDataTypeStr', 'Bus: IMUBus');

% ============================================================
% Bus Selector: 从 DynamicModelBus 提取信号
% ============================================================
add_block('simulink/Signal Routing/Bus Selector', [modelName '/BusSel']);
set_param([modelName '/BusSel'], 'OutputSignals', 'Attitude_quat,Accel_Body,AngularRate_Body');
add_line(modelName, 'in/1', 'BusSel/1');

% ============================================================
% 加速度计路径
% ============================================================

% --- 1. 重力补偿: 真实加速度 = Accel_Body + R'*[0;0;g] ---
% 使用 MATLAB Function 实现
add_block('simulink/User-Defined Functions/MATLAB Function', [modelName '/AccelTrue']);
set_param([modelName '/AccelTrue'], 'Position', [300 50 450 100]);
% 设置函数代码 (稍后通过脚本设置)

% --- 2. 比例因子误差 ---
add_block('simulink/Math Operations/Gain', [modelName '/AccelScale']);
set_param([modelName '/AccelScale'], 'Gain', '(1 + IMU_ACC_SCALE)', 'Position', [480 50 530 80]);

% --- 3. 加偏置 ---
add_block('simulink/Math Operations/Sum', [modelName '/AccelAddBias']);
set_param([modelName '/AccelAddBias'], 'Inputs', '+++', 'Position', [560 40 590 90]);

% --- 4. 白噪声源 ---
add_block('simulink/Sources/Band-Limited White Noise', [modelName '/AccelWhiteNoise']);
set_param([modelName '/AccelWhiteNoise'], ...
    'Cov', 'IMU_ACC_NOISE^2 * IMU_SAMPLE_RATE', ...
    'Ts', '1 / double(IMU_SAMPLE_RATE)', ...
    'Seed', '23341', ...
    'Position', [480 130 530 160]);
add_block('simulink/Attributes/Data Type Conversion', ...
    [modelName '/AccelWhiteNoiseSingle']);
set_param([modelName '/AccelWhiteNoiseSingle'], ...
    'OutDataTypeStr', 'single', 'Position', [545 130 590 160]);

% --- 5. 随机游走 ---
add_block('simulink/Sources/Band-Limited White Noise', [modelName '/AccelBiasNoise']);
set_param([modelName '/AccelBiasNoise'], ...
    'Cov', 'IMU_ACC_BIAS_WALK^2 * IMU_SAMPLE_RATE', ...
    'Ts', '1 / double(IMU_SAMPLE_RATE)', ...
    'Seed', '23342', ...
    'Position', [480 180 530 210]);
add_block('simulink/Attributes/Data Type Conversion', ...
    [modelName '/AccelBiasNoiseSingle']);
set_param([modelName '/AccelBiasNoiseSingle'], ...
    'OutDataTypeStr', 'single', 'Position', [545 180 590 210]);

add_block('simulink/Discrete/Discrete-Time Integrator', [modelName '/AccelBiasIntegrator']);
set_param([modelName '/AccelBiasIntegrator'], ...
    'IntegratorMethod', 'Integration: Forward Euler', ...
    'SampleTime', '1 / double(IMU_SAMPLE_RATE)', ...
    'InitialCondition', 'IMU_ACC_BIAS', ...
    'Position', [560 175 620 215]);

% --- 6. 饱和限幅 ---
add_block('simulink/Discontinuities/Saturation', [modelName '/AccelSat']);
set_param([modelName '/AccelSat'], ...
    'UpperLimit', 'IMU_SAT_ACC', ...
    'LowerLimit', '-IMU_SAT_ACC', ...
    'Position', [640 40 690 90]);

% ============================================================
% 陀螺仪路径
% ============================================================

% --- 1. 比例因子误差 ---
add_block('simulink/Math Operations/Gain', [modelName '/GyroScale']);
set_param([modelName '/GyroScale'], 'Gain', '(1 + IMU_GYR_SCALE)', 'Position', [300 250 350 280]);

% --- 2. 加偏置 ---
add_block('simulink/Math Operations/Sum', [modelName '/GyroAddBias']);
set_param([modelName '/GyroAddBias'], 'Inputs', '+++', 'Position', [380 240 410 290]);

% --- 3. 白噪声源 ---
add_block('simulink/Sources/Band-Limited White Noise', [modelName '/GyroWhiteNoise']);
set_param([modelName '/GyroWhiteNoise'], ...
    'Cov', 'IMU_GYR_NOISE^2 * IMU_SAMPLE_RATE', ...
    'Ts', '1 / double(IMU_SAMPLE_RATE)', ...
    'Seed', '23343', ...
    'Position', [300 320 350 350]);
add_block('simulink/Attributes/Data Type Conversion', ...
    [modelName '/GyroWhiteNoiseSingle']);
set_param([modelName '/GyroWhiteNoiseSingle'], ...
    'OutDataTypeStr', 'single', 'Position', [365 320 410 350]);

% --- 4. 随机游走 ---
add_block('simulink/Sources/Band-Limited White Noise', [modelName '/GyroBiasNoise']);
set_param([modelName '/GyroBiasNoise'], ...
    'Cov', 'IMU_GYR_BIAS_WALK^2 * IMU_SAMPLE_RATE', ...
    'Ts', '1 / double(IMU_SAMPLE_RATE)', ...
    'Seed', '23344', ...
    'Position', [300 370 350 400]);
add_block('simulink/Attributes/Data Type Conversion', ...
    [modelName '/GyroBiasNoiseSingle']);
set_param([modelName '/GyroBiasNoiseSingle'], ...
    'OutDataTypeStr', 'single', 'Position', [365 370 410 400]);

add_block('simulink/Discrete/Discrete-Time Integrator', [modelName '/GyroBiasIntegrator']);
set_param([modelName '/GyroBiasIntegrator'], ...
    'IntegratorMethod', 'Integration: Forward Euler', ...
    'SampleTime', '1 / double(IMU_SAMPLE_RATE)', ...
    'InitialCondition', 'IMU_GYR_BIAS', ...
    'Position', [380 365 440 405]);

% --- 5. 饱和限幅 ---
add_block('simulink/Discontinuities/Saturation', [modelName '/GyroSat']);
set_param([modelName '/GyroSat'], ...
    'UpperLimit', 'IMU_SAT_GYR', ...
    'LowerLimit', '-IMU_SAT_GYR', ...
    'Position', [440 240 490 290]);

add_block('simulink/Sources/Constant', [modelName '/Valid']);
set_param([modelName '/Valid'], ...
    'Value', 'true', ...
    'OutDataTypeStr', 'boolean', ...
    'SampleTime', '1 / double(IMU_SAMPLE_RATE)', ...
    'Position', [640 125 690 155]);

add_block('simulink/Sources/Clock', [modelName '/Timestamp']);
set_param([modelName '/Timestamp'], ...
    'DisplayTime', 'off', ...
    'Position', [640 175 690 205]);

% ============================================================
% Bus Creator: 组装 IMUBus
% ============================================================
add_block('simulink/Signal Routing/Bus Creator', [modelName '/BusCreator']);
set_param([modelName '/BusCreator'], ...
    'Inputs', 'Accel,Gyro,Valid,Timestamp', ...
    'OutDataTypeStr', 'Bus: IMUBus', ...
    'NonVirtualBus', 'on', ...
    'Position', [740 40 790 155]);

% ============================================================
% 连线
% ============================================================

% BusSel -> AccelTrue (Attitude_quat, Accel_Body)
add_line(modelName, 'BusSel/1', 'AccelTrue/1');  % Attitude_quat
add_line(modelName, 'BusSel/2', 'AccelTrue/2');  % Accel_Body

% AccelTrue -> Scale -> AddBias -> Saturation -> BusCreator
add_line(modelName, 'AccelTrue/1', 'AccelScale/1');
add_line(modelName, 'AccelScale/1', 'AccelAddBias/1');
add_line(modelName, 'AccelWhiteNoise/1', 'AccelWhiteNoiseSingle/1');
add_line(modelName, 'AccelWhiteNoiseSingle/1', 'AccelAddBias/2');
add_line(modelName, 'AccelBiasIntegrator/1', 'AccelAddBias/3');
add_line(modelName, 'AccelAddBias/1', 'AccelSat/1');
add_line(modelName, 'AccelSat/1', 'BusCreator/1');

% AccelWhiteNoise -> AccelBiasIntegrator (random walk)
add_line(modelName, 'AccelBiasNoise/1', 'AccelBiasNoiseSingle/1');
add_line(modelName, 'AccelBiasNoiseSingle/1', 'AccelBiasIntegrator/1');

% BusSel (AngularRate_Body) -> GyroScale -> GyroAddBias -> GyroSat -> BusCreator
add_line(modelName, 'BusSel/3', 'GyroScale/1');
add_line(modelName, 'GyroScale/1', 'GyroAddBias/1');
add_line(modelName, 'GyroWhiteNoise/1', 'GyroWhiteNoiseSingle/1');
add_line(modelName, 'GyroWhiteNoiseSingle/1', 'GyroAddBias/2');
add_line(modelName, 'GyroBiasIntegrator/1', 'GyroAddBias/3');
add_line(modelName, 'GyroAddBias/1', 'GyroSat/1');
add_line(modelName, 'GyroSat/1', 'BusCreator/2');
add_line(modelName, 'Valid/1', 'BusCreator/3');
add_line(modelName, 'Timestamp/1', 'BusCreator/4');

% GyroWhiteNoise -> GyroBiasIntegrator (random walk)
add_line(modelName, 'GyroBiasNoise/1', 'GyroBiasNoiseSingle/1');
add_line(modelName, 'GyroBiasNoiseSingle/1', 'GyroBiasIntegrator/1');

% BusCreator -> Outport
add_line(modelName, 'BusCreator/1', 'out/1');

% ============================================================
% 设置 MATLAB Function 代码
% ============================================================
% AccelTrue: 真实比力 = R'*(a_body - [0;0;g])
% 其中 R 从四元数得到, a_body 是体轴加速度(含重力分量)
% IMU 测量的是比力: f = a - g (在体轴系)
% 所以: accel_measured = R' * [0;0;g] + accel_body (近似)
% 更精确: accel_true = accel_body (已经是体轴系的比力)
% 但需要加上重力在体轴系的投影: R'*[0;0;g]

% 保存模型
save_system(modelName, mdlFile);
fprintf('IMU 传感器模型已保存: %s\n', mdlFile);

% 关闭模型
close_system(modelName, 0);
end
