function test_state_estimation_time_contract()
% test_state_estimation_time_contract verifies scheduling and EKF dt semantics.

projectRoot = fileparts(fileparts(mfilename('fullpath')));
openProject(fullfile(projectRoot, 'FC_SimulinkProject.prj'));

dictPath = fullfile(projectRoot, '2_Model', 'state_estimation', 'StateEstDict.sldd');
ddObj = Simulink.data.dictionary.open(dictPath);
cleanup = onCleanup(@() close(ddObj));
dData = getSection(ddObj, 'Design Data');

imuSampleTime = getValue(getEntry(dData, 'imu_sample_time'));
ekfPredictDt = getValue(getEntry(dData, 'ekf_predict_dt'));

assert(strcmp(imuSampleTime.DataType, 'double'), ...
    'imu_sample_time must be double because it is a scheduling/sample-time parameter.');
assert(strcmp(ekfPredictDt.DataType, 'single'), ...
    'ekf_predict_dt must be single because EKF MATLAB Function dt input is single.');
assert(isa(imuSampleTime.Value, 'double'), 'imu_sample_time.Value must be double.');
assert(isa(ekfPredictDt.Value, 'single'), 'ekf_predict_dt.Value must be single.');
assert(abs(single(imuSampleTime.Value) - ekfPredictDt.Value) < single(1e-9), ...
    'ekf_predict_dt must match imu_sample_time after explicit single conversion.');

load_system('EKF');
dtBlock = 'EKF/ekf_predict_dt';
assert(strcmp(get_param(dtBlock, 'Value'), 'ekf_predict_dt'), ...
    'EKF/ekf_predict_dt Constant must use ekf_predict_dt, not the scheduling sample-time parameter.');
assert(strcmp(get_param(dtBlock, 'OutDataTypeStr'), 'single'), ...
    'EKF/ekf_predict_dt Constant output must remain single to match EKF MATLAB Function dt input.');

assert(strcmp(get_param('EKF/EstimatorStatus', 'SampleTime'), 'imu_sample_time'), ...
    'EKF/EstimatorStatus SampleTime must use the double scheduling parameter imu_sample_time.');

rt = sfroot;
chart = rt.find('-isa', 'Stateflow.EMChart', 'Path', 'EKF/MATLAB Function');
dtData = chart.find('-isa', 'Stateflow.Data', 'Name', 'dt');
assert(strcmp(dtData.DataType, 'single'), ...
    'EKF MATLAB Function dt input must remain single unless the full EKF numeric contract changes.');

disp('STATE_ESTIMATION_TIME_CONTRACT_PASS');
end
