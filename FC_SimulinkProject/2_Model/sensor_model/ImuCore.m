classdef ImuCore < matlab.System
%ImuCore  Official imuSensor-based IMU core (accelerometer + gyroscope).
%
% Reuses the official imuSensor with accelparams/gyroparams so the
% industrial error model (noise density, constant bias, bias instability,
% random walk, axes misalignment, resolution, measurement range, temperature)
% is the official one -- no hand-written error chain.
%
% SimulateUsing = Interpreted execution: this sensor model is MIL plant only
% (the simulation plant is replaced by Gazebo later, no code generation is
% required). imuSensor itself is code-generation capable if that changes.
%
% The official accelerometer model is  y = R*(g_n - acc)  and the official
% gyroscope model is  y = R*angvel, with g_n = [0;0;9.81].
%
% The project's established IMU convention (consumed by EKF predict as
% a_n = C_bn*Accel + g_n) is  y = -(1+s)*Accel_Body  for the accelerometer and
% y = (1+s)*AngularRate_Body for the gyroscope. This wrapper therefore feeds
%
%     acc    = g_n + (1+AccelScaleFactor)*Accel_Body
%     angvel = (1+GyrScaleFactor)*AngularRate_Body
%
% with R = I, so the ideal official output equals the project convention and
% every official error term applies on top. R = I is intentional: the sensor
% frame is the body frame, and the enabled error terms are frame-independent.
%
% Inputs : Accel_Body[3] (m/s^2, excluding gravity), AngularRate_Body[3] (rad/s)
% Outputs: Accel[3] (m/s^2), Gyro[3] (rad/s), both single.

    properties (Nontunable)
        SampleRate = 1000
        Seed = 23341
        AccelScaleFactor = 0.002
        GyrScaleFactor = 0.002
        AccelNoiseDensity = 0.001
        AccelConstantBias = [0 0 0]
        AccelRandomWalk = 1e-4
        AccelBiasInstability = 0
        AccelMeasurementRange = 156.96
        AccelResolution = 0
        AccelNoiseType = 'single-sided'
        GyrNoiseDensity = 1e-4
        GyrConstantBias = [0 0 0]
        GyrRandomWalk = 1e-5
        GyrBiasInstability = 0
        GyrMeasurementRange = 34.907
        GyrResolution = 0
        GyrNoiseType = 'single-sided'
    end

    properties (Access = private)
        IMU
    end

    methods
        function obj = ImuCore(varargin)
            % Explicit constructor. This MATLAB install fails name-value
            % construction of user System objects through the framework path
            % (matlab.system.SystemInterface), so apply pairs after the base
            % constructor instead.
            obj@matlab.System();
            if nargin > 0
                for k = 1:2:numel(varargin)
                    obj.(varargin{k}) = varargin{k+1};
                end
            end
        end
    end

    methods (Access = protected)
        function setupImpl(obj)
            ap = accelparams( ...
                'MeasurementRange',  double(obj.AccelMeasurementRange), ...
                'Resolution',        double(obj.AccelResolution), ...
                'ConstantBias',      double(obj.AccelConstantBias(:).'), ...
                'NoiseDensity',      double(obj.AccelNoiseDensity), ...
                'BiasInstability',   double(obj.AccelBiasInstability), ...
                'RandomWalk',        double(obj.AccelRandomWalk), ...
                'NoiseType',         obj.AccelNoiseType);
            gp = gyroparams( ...
                'MeasurementRange',  double(obj.GyrMeasurementRange), ...
                'Resolution',        double(obj.GyrResolution), ...
                'ConstantBias',      double(obj.GyrConstantBias(:).'), ...
                'NoiseDensity',      double(obj.GyrNoiseDensity), ...
                'BiasInstability',   double(obj.GyrBiasInstability), ...
                'RandomWalk',        double(obj.GyrRandomWalk), ...
                'NoiseType',         obj.GyrNoiseType);
            obj.IMU = imuSensor('accel-gyro', ...
                'SampleRate',    double(obj.SampleRate), ...
                'Accelerometer', ap, ...
                'Gyroscope',     gp, ...
                'RandomStream',  'mt19937ar with seed', ...
                'Seed',          double(obj.Seed));
        end

        function [accel, gyro] = stepImpl(obj, accelBody, angvelBody)
            g_n = [0; 0; 9.81];
            accIn = g_n + (1 + double(obj.AccelScaleFactor)) * double(accelBody(:));
            angIn = (1 + double(obj.GyrScaleFactor)) * double(angvelBody(:));
            [a, w] = obj.IMU(accIn.', angIn.', eye(3));
            accel = single(a(:));
            gyro  = single(w(:));
        end

        function resetImpl(obj)
            reset(obj.IMU);
        end

        function [sz1, sz2] = getOutputSizeImpl(~)
            sz1 = [3 1]; sz2 = [3 1];
        end

        function [dt1, dt2] = getOutputDataTypeImpl(~)
            dt1 = 'single'; dt2 = 'single';
        end

        function [c1, c2] = isOutputComplexImpl(~)
            c1 = false; c2 = false;
        end

        function [f1, f2] = isOutputFixedSizeImpl(~)
            f1 = true; f2 = true;
        end
    end
end
