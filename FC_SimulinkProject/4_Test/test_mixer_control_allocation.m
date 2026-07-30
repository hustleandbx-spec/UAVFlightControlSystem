classdef test_mixer_control_allocation < matlab.unittest.TestCase
    properties (Constant)
        MixMatrix = single([1 -1 -1 -1; 1 1 1 -1; ...
                            1 1 -1 1; 1 -1 1 1]);
        MotorMin = single(0.05);
        MotorMax = single(1.0);
        MotorMaxThrust_N = single(4.179446268);
        MotorArmLength_m = single(0.2275);
        MotorMaxReactionTorque_Nm = single(0.055562);
    end

    methods (Test)
        function nominalHoverHasMargin(testCase)
            cmd = allocateMotorCommands(single(1.0 * 9.80665), single(zeros(3, 1)), ...
                testCase.MixMatrix, testCase.MotorMaxThrust_N, ...
                testCase.MotorArmLength_m, testCase.MotorMaxReactionTorque_Nm, ...
                testCase.MotorMin, testCase.MotorMax);

            testCase.verifyClass(cmd, 'single');
            testCase.verifySize(cmd, [4 1]);
            testCase.verifyTrue(all(isfinite(cmd)));
            testCase.verifyEqual(max(cmd) - min(cmd), single(0), 'AbsTol', single(1e-6));
            testCase.verifyEqual(cmd, repmat(single(0.586596), 4, 1), ...
                'AbsTol', single(1e-5));
            testCase.verifyGreaterThan(cmd, repmat(testCase.MotorMin, 4, 1));
            testCase.verifyLessThan(cmd, repmat(single(0.8), 4, 1));
        end

        function collectiveThrustIsMonotonic(testCase)
            low = testCase.allocate(single(6), single(zeros(3, 1)));
            high = testCase.allocate(single(12), single(zeros(3, 1)));
            testCase.verifyGreaterThan(high, low);
            testCase.verifyLessThan(high, repmat(testCase.MotorMax, 4, 1));
        end

        function torqueDirectionsMatchXGeometry(testCase)
            base = testCase.allocate(single(9.80665), single(zeros(3, 1)));
            roll = testCase.allocate(single(9.80665), single([0.2; 0; 0]));
            pitch = testCase.allocate(single(9.80665), single([0; 0.2; 0]));
            yaw = testCase.allocate(single(9.80665), single([0; 0; 0.02]));

            testCase.verifyLessThan(roll([1 4]), base([1 4]));
            testCase.verifyGreaterThan(roll([2 3]), base([2 3]));
            testCase.verifyLessThan(pitch([1 3]), base([1 3]));
            testCase.verifyGreaterThan(pitch([2 4]), base([2 4]));
            testCase.verifyLessThan(yaw([1 2]), base([1 2]));
            testCase.verifyGreaterThan(yaw([3 4]), base([3 4]));
        end

        function saturationOccursOnlyBeyondCapability(testCase)
            within = testCase.allocate(single(4 * testCase.MotorMaxThrust_N * 0.9), ...
                single(zeros(3, 1)));
            excessive = testCase.allocate(single(4 * testCase.MotorMaxThrust_N * 1.1), ...
                single(zeros(3, 1)));
            testCase.verifyLessThan(within, repmat(testCase.MotorMax, 4, 1));
            testCase.verifyEqual(excessive, repmat(testCase.MotorMax, 4, 1));
        end

        function lowerBoundAndContractArePreserved(testCase)
            cmd = testCase.allocate(single(-100), single(zeros(3, 1)));
            testCase.verifyEqual(cmd, repmat(testCase.MotorMin, 4, 1));
            testCase.verifyGreaterThanOrEqual(cmd, repmat(single(0), 4, 1));
            testCase.verifyLessThanOrEqual(cmd, repmat(single(1), 4, 1));
        end
    end

    methods (Access = private)
        function cmd = allocate(testCase, thrust_N, torque_Nm)
            cmd = allocateMotorCommands(thrust_N, torque_Nm, testCase.MixMatrix, ...
                testCase.MotorMaxThrust_N, testCase.MotorArmLength_m, ...
                testCase.MotorMaxReactionTorque_Nm, testCase.MotorMin, testCase.MotorMax);
        end
    end
end
