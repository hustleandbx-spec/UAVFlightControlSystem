function add_test_suites()
% 向 UAV_FlightControlSystem_Test_Case.mldatx 中追加测试用例
% 每个子系统一个测试套件，不新建 .mldatx 文件

testFile = fullfile(fileparts(mfilename('fullpath')), ...
    'UAV_FlightControlSystem_Test_Case.mldatx');

tf = sltest.testmanager.load(testFile);

% getTestSuites 返回文件内所有顶级套件
% .mldatx 中命名根套件在 API 层面不暴露为 Suite 对象
% 所有子套件平级返回
existingSuites = tf.getTestSuites;
suiteMap = containers.Map;
for i = 1:numel(existingSuites)
    suiteMap(existingSuites(i).Name) = existingSuites(i);
end
fprintf('已打开测试文件, 已有套件: ');
for i = 1:numel(existingSuites)
    fprintf('%s ', existingSuites(i).Name);
end
fprintf('\n');

%% ===== 1. Dynamic_Model_Test_Case: 追加 MATLAB Function 单元测试 =====
if suiteMap.isKey('Dynamic_Model_Test_Case')
    dmSuite = suiteMap('Dynamic_Model_Test_Case');
else
    dmSuite = rootSuite.createTestSuite('Dynamic_Model_Test_Case');
end
dmSuite.Description = '六自由度动力学模型 — 子系统集成测试与 MATLAB Function 单元测试';

% 1a. 四元数微分
tc = dmSuite.createTestCase('QuatDiff_UnitTest', 'TestType', 'simulation');
tc.setProperty('Model', 'UAV_dynamic_model');
tc.setProperty('HarnessName', 'UAV_dynamic_model_Harness');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% 四元数微分: 零角速度时四元数不变\n', ...
    'qdot = logsout.get(''q_dot'');\n', ...
    'if ~isempty(qdot) && max(abs(qdot.Values.Data(end,:))) < 0.1\n', ...
    '    sltest.testmanager.TestResultOutcome.Passed;\n', ...
    'else\n', ...
    '    sltest.testmanager.TestResultOutcome.Failed;\n', ...
    'end\n']);

% 1b. quat2rotm
tc = dmSuite.createTestCase('Quat2Rotm_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'UAV_dynamic_model');
tc.setProperty('HarnessName', 'UAV_dynamic_model_Harness');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% quat2rotm: 单位四元数 → 正交旋转矩阵\n', ...
    'R_sig = logsout.get(''R'');\n', ...
    'if ~isempty(R_sig)\n', ...
    '    R = reshape(R_sig.Values.Data(end,:), 3, 3);\n', ...
    '    if abs(det(R)-1) < 0.01 && norm(R*R''-eye(3)) < 0.01\n', ...
    '        sltest.testmanager.TestResultOutcome.Passed;\n', ...
    '    else, sltest.testmanager.TestResultOutcome.Failed; end\n', ...
    'end\n']);

% 1c. 四元数归一化
tc = dmSuite.createTestCase('QuatNormalize_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'UAV_dynamic_model');
tc.setProperty('HarnessName', 'UAV_dynamic_model_Harness');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% fcn_quat_normalize: 输出模长=1\n', ...
    'qn = logsout.get(''q_norm'');\n', ...
    'if ~isempty(qn) && abs(norm(qn.Values.Data(end,:))-1) < 0.001\n', ...
    '    sltest.testmanager.TestResultOutcome.Passed;\n', ...
    'else, sltest.testmanager.TestResultOutcome.Failed; end\n']);

fprintf('Dynamic_Model_Test_Case: +3 测试 (%d total)\n', numel(dmSuite.getTestCases));

%% ===== 2. Power_System_Test_Case: 追加 MATLAB Function 单元测试 =====
if suiteMap.isKey('Power_System_Test_Case')
    psSuite = suiteMap('Power_System_Test_Case');
else
    psSuite = rootSuite.createTestSuite('Power_System_Test_Case');
end
psSuite.Description = '动力系统模型 — 子系统集成测试与 MATLAB Function 单元测试';

% 2a. fcn_calc_wss
tc = psSuite.createTestCase('CalcWss_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'power_system_model');
tc.setProperty('HarnessName', 'power_system_model_Harness');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% fcn_calc_wss: 给定油门 → 稳态转速 > 0\n', ...
    'ws = logsout.get(''w_ss'');\n', ...
    'if ~isempty(ws) && all(ws.Values.Data(end,:) > 0)\n', ...
    '    sltest.testmanager.TestResultOutcome.Passed;\n', ...
    'else, sltest.testmanager.TestResultOutcome.Failed; end\n']);

% 2b. fcn_prop_forces
tc = psSuite.createTestCase('PropForces_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'power_system_model');
tc.setProperty('HarnessName', 'power_system_model_Harness');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% fcn_prop_forces: F=Ct*w^2, 推力 > 0\n', ...
    'Fv = logsout.get(''F_vec'');\n', ...
    'if ~isempty(Fv) && all(Fv.Values.Data(end,:) > 0)\n', ...
    '    sltest.testmanager.TestResultOutcome.Passed;\n', ...
    'else, sltest.testmanager.TestResultOutcome.Failed; end\n']);

% 2c. fcn_geometry_mixer
tc = psSuite.createTestCase('GeometryMixer_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'power_system_model');
tc.setProperty('HarnessName', 'power_system_model_Harness');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% fcn_geometry_mixer: 四路均等 → 仅Z推力，横滚/俯仰力矩≈0\n', ...
    'Tv = logsout.get(''Torque_vec'');\n', ...
    'if ~isempty(Tv) && max(abs(Tv.Values.Data(end,1:2))) < 0.01\n', ...
    '    sltest.testmanager.TestResultOutcome.Passed;\n', ...
    'else, sltest.testmanager.TestResultOutcome.Failed; end\n']);

fprintf('Power_System_Test_Case: +3 测试 (%d total)\n', numel(psSuite.getTestCases));

%% ===== 3. Flight_Control_Test_Case: 新建飞控测试套件 =====
fcSuite = rootSuite.createTestSuite('Flight_Control_Test_Case');
fcSuite.Description = '飞行控制律 — 串级控制链各子环与 MATLAB Function 测试';

% 3a. AccelToAttitude 单元测试
tc = fcSuite.createTestCase('AccelToAttitude_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'UAV_FlightControl');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% AccelToAttitude: 悬停时力矩指令应接近零\n', ...
    'tq = logsout.get(''torque_sp'');\n', ...
    'if ~isempty(tq) && max(abs(tq.Values.Data(end-10:end,:))) < 1.0\n', ...
    '    sltest.testmanager.TestResultOutcome.Passed;\n', ...
    'else, sltest.testmanager.TestResultOutcome.Failed; end\n']);

% 3b. Mixer 单元测试
tc = fcSuite.createTestCase('Mixer_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'UAV_FlightControl');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% Mixer: 悬停时四路指令相等，且在[motorMin, motorMax]内\n', ...
    'mc = logsout.get(''motor_cmd'');\n', ...
    'if ~isempty(mc)\n', ...
    '    f = mc.Values.Data(end,:);\n', ...
    '    if max(f)-min(f) < 0.1 && all(f >= 0.05 & f <= 1.0)\n', ...
    '        sltest.testmanager.TestResultOutcome.Passed;\n', ...
    '    else, sltest.testmanager.TestResultOutcome.Failed; end\n', ...
    'end\n']);

% 3c. 位置环阶跃响应测试
tc = fcSuite.createTestCase('PositionLoop_StepResponse');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'UAV_FlightControl');
tc.setProperty('SimulationEndTime', '10');
tc.addAssessment('custom_eval', [
    '%% 位置环阶跃响应: 速度指令末期接近零(已收敛到设定点)\n', ...
    'vd = logsout.get(''vel_desired'');\n', ...
    'if ~isempty(vd) && max(abs(vd.Values.Data(end-10:end,:))) < 0.5\n', ...
    '    sltest.testmanager.TestResultOutcome.Passed;\n', ...
    'else, sltest.testmanager.TestResultOutcome.Failed; end\n']);

% 3d. 姿态环误差收敛测试
tc = fcSuite.createTestCase('AttitudeLoop_Convergence');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'UAV_FlightControl');
tc.setProperty('SimulationEndTime', '5');
tc.addAssessment('custom_eval', [
    '%% 姿态环: 初始姿态误差应在控制律作用下收敛\n', ...
    'tq = logsout.get(''torque_sp'');\n', ...
    'if ~isempty(tq)\n', ...
    '    d = tq.Values.Data;\n', ...
    '    if max(abs(d(end-5:end,:))) < max(abs(d(1:5,:)))\n', ...
    '        sltest.testmanager.TestResultOutcome.Passed;\n', ...
    '    else, sltest.testmanager.TestResultOutcome.Failed; end\n', ...
    'end\n']);

fprintf('Flight_Control_Test_Case: 新建 (%d tests)\n', numel(fcSuite.getTestCases));

%% ===== 4. ESKF_Test_Case: 新建导航估计测试套件 =====
eskfSuite = rootSuite.createTestSuite('ESKF_Test_Case');
eskfSuite.Description = '误差状态卡尔曼滤波器 — 预测/更新步单元测试';

% 4a. 预测步测试
tc = eskfSuite.createTestCase('ESKF_Predict_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'ESKF');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% ESKF 预测步: 静止IMU → 位置漂移 < 0.01m\n', ...
    'xp = logsout.get(''x_p'');\n', ...
    'if ~isempty(xp)\n', ...
    '    pos_drift = norm(xp.Values.Data(end,1:3) - xp.Values.Data(1,1:3));\n', ...
    '    if pos_drift < 0.01\n', ...
    '        sltest.testmanager.TestResultOutcome.Passed;\n', ...
    '    else, sltest.testmanager.TestResultOutcome.Failed; end\n', ...
    'end\n']);

% 4b. 更新步测试
tc = eskfSuite.createTestCase('ESKF_Update_UnitTest');
tc.TestCaseType = 2;  % 2 = simulation test
tc.setProperty('Model', 'ESKF');
tc.setProperty('SimulationEndTime', '1');
tc.addAssessment('custom_eval', [
    '%% ESKF 更新步: GPS更新后协方差减小\n', ...
    'Pc = logsout.get(''P_c'');\n', ...
    'Pp = logsout.get(''P_p'');\n', ...
    'if ~isempty(Pc) && ~isempty(Pp)\n', ...
    '    if trace(Pc.Values.Data(:,:,end)) < trace(Pp.Values.Data(:,:,end))\n', ...
    '        sltest.testmanager.TestResultOutcome.Passed;\n', ...
    '    else, sltest.testmanager.TestResultOutcome.Failed; end\n', ...
    'end\n']);

fprintf('ESKF_Test_Case: 新建 (%d tests)\n', numel(eskfSuite.getTestCases));

%% 保存
tf.save;
fprintf('\n=== 测试文件结构 ===\n');
fprintf('UAV_FlightControlSystem_Test_Case.mldatx\n');
allSuites = rootSuite.getTestSuites;
for i = 1:numel(allSuites)
    s = allSuites(i);
    fprintf('  ├── %s (%d tests)\n', s.Name, numel(s.getTestCases));
end
fprintf('总计: %d 个测试套件\n', numel(allSuites));
end
