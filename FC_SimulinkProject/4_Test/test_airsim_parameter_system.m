classdef test_airsim_parameter_system < matlab.unittest.TestCase
    methods (Test)
        function vehicleMatchesAirSimGenericQuad(testCase)
            vehicle = testCase.readDesignData('1_Data_Dictionaries/VehicleDict.sldd');
            testCase.verifyEqual(vehicle.mass.Value, single(1.0));
            testCase.verifyEqual(vehicle.g.Value, single(9.80665), 'AbsTol', single(1e-6));
            testCase.verifyEqual(vehicle.motorArmLength_m.Value, single(0.2275), ...
                'AbsTol', single(1e-7));
            testCase.verifyEqual(vehicle.inertia.Value, ...
                single([0.006721; 0.008041; 0.014279]), 'AbsTol', single(1e-6));
        end

        function powerSystemOwnsAirSimActuatorCalibration(testCase)
            power = testCase.readDesignData('2_Model/power_system/PowerSystemDict.sldd');
            testCase.verifyEqual(power.motorMaxThrust_N.Value, ...
                single(4.179446268), 'AbsTol', single(1e-6));
            testCase.verifyEqual(power.motorMaxReactionTorque_Nm.Value, ...
                single(0.055562), 'AbsTol', single(1e-7));
            testCase.verifyEqual(power.motorTimeConstant_s.Value, ...
                single(0.005), 'AbsTol', single(1e-7));
            legacy = {'U_bat', 'w_bias', 'Cb', 'Ct', 'Cm', 'Jrp'};
            for idx = 1:numel(legacy)
                testCase.verifyFalse(isfield(power, legacy{idx}));
            end
        end

        function flightControlReferencesButDoesNotOwnPlantCalibration(testCase)
            projectRoot = fileparts(fileparts(mfilename('fullpath')));
            dictionary = Simulink.data.dictionary.open(fullfile(projectRoot, ...
                '2_Model/control/FlightControlDict.sldd'));
            cleanup = onCleanup(@() close(dictionary));
            section = getSection(dictionary, 'Design Data');
            testCase.verifyEqual(getEntry(section, 'motorMaxThrust_N').DataSource, ...
                'PowerSystemDict.sldd');
            testCase.verifyEqual(getEntry(section, 'motorArmLength_m').DataSource, ...
                'VehicleDict.sldd');
            testCase.verifyEqual(getEntry(section, ...
                'motorMaxReactionTorque_Nm').DataSource, 'PowerSystemDict.sldd');
        end
    end

    methods (Static, Access = private)
        function values = readDesignData(relativePath)
            projectRoot = fileparts(fileparts(mfilename('fullpath')));
            dictionary = Simulink.data.dictionary.open(fullfile(projectRoot, relativePath));
            cleanup = onCleanup(@() close(dictionary));
            section = getSection(dictionary, 'Design Data');
            entries = find(section);
            values = struct;
            for idx = 1:numel(entries)
                values.(entries(idx).Name) = getValue(entries(idx));
            end
        end
    end
end
