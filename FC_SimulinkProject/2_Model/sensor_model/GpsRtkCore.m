classdef GpsRtkCore < matlab.System
%GpsRtkCore RTK GPS 测量内核（仿真专用，Simulation 域）。
% 官方 gpsSensor 生成白噪声与失锁衰减；本外壳只承载官方未覆盖的部分：
%   1) 由 GnssScenarioBus 的 HDOP/VDOP/FixType 派生测量 σ，并输出 Eph/Epv；
%   2) 1 ppm 基线相关项与多路径环境残差（固定解误差预算）；
%   3) 一阶 Gauss-Markov 时间相关误差（官方 gpsSensor 不含）。
% 坐标系：局部 NED；参考点 ReferenceLocation = [lat lon alt]。
%
% SimulateUsing = Interpreted execution：本传感器模型仅作 MIL plant，后期仿真
% 植物直接由 Gazebo 承担，不做代码生成（gpsSensor 本身可代码生成）。
    properties (Nontunable)
        SampleRate = 5;
        ReferenceLocation = [0 0 0];
        UereH = 0.6;
        UereV = 1.2;
        UereVel = 0.1;
        RtkSigmaH = 0.015;
        RtkSigmaV = 0.025;
        BaselinePpm = 1.0;
        MultipathSigma = 0.01;
        DecayFactor = 0.5;
        GmTau = 30;
        GmSigma = 0.01;
        Seed = 23341;
    end

    properties (Access = private)
        pGps
        pRng
        pGm = zeros(3,1);
    end

    methods
        function obj = GpsRtkCore(varargin)
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
            obj.pGps = gpsSensor('SampleRate', obj.SampleRate, 'ReferenceLocation', obj.ReferenceLocation, 'PositionInputFormat', 'Local', 'HorizontalPositionAccuracy', obj.RtkSigmaH, 'VerticalPositionAccuracy', obj.RtkSigmaV, 'VelocityAccuracy', obj.UereVel, 'DecayFactor', obj.DecayFactor, 'RandomStream', 'mt19937ar with seed', 'Seed', obj.Seed);
            obj.pRng = RandStream('mt19937ar', 'Seed', obj.Seed);
            obj.pGm = zeros(3,1);
        end

        function resetImpl(obj)
            reset(obj.pGps);
            obj.pGm = zeros(3,1);
        end

        function [lla, vel, fixType, numSat, eph, epv, valid] = stepImpl(obj, posNed, velNed, hdop, vdop, numSatIn, fixTypeIn)
            ft = double(fixTypeIn);
            hd = max(double(hdop), 0.5);
            vd = max(double(vdop), 0.5);
            baseline = norm(double(posNed(:)));
            baseTerm = (obj.BaselinePpm * 1e-6 * baseline)^2 + obj.MultipathSigma^2;
            sigH = sqrt(obj.RtkSigmaH^2 + baseTerm);
            sigV = sqrt(obj.RtkSigmaV^2 + baseTerm);
            if ft < 6
                sigH = hd * obj.UereH;
                sigV = vd * obj.UereV;
            end
            [pMeas, vMeas] = obj.pGps(double(posNed(:)).', double(velNed(:)).');
            nMeas = lla2ned(double(pMeas), double(obj.ReferenceLocation), 'ellipsoid');
            err = nMeas(:) - double(posNed(:));
            err(1:2) = err(1:2) * (sigH / obj.RtkSigmaH);
            err(3)   = err(3)   * (sigV / obj.RtkSigmaV);
            dt = 1 / obj.SampleRate;
            a = exp(-dt / max(obj.GmTau, 1e-3));
            w = obj.GmSigma * sqrt(max(0, 1 - a^2));
            obj.pGm = a * obj.pGm + w * randn(obj.pRng, 3, 1);
            posOut = double(posNed(:)) + err + obj.pGm;
            llaOut = ned2lla(posOut.', double(obj.ReferenceLocation), 'ellipsoid');
            lla     = single(llaOut(:));
            vel     = single(vMeas(:));
            fixType = uint8(min(max(round(ft), 0), 8));
            numSat  = uint8(min(max(round(double(numSatIn)), 0), 64));
            eph     = single(sigH);
            epv     = single(sigV);
            valid   = (ft >= 2);
        end

        function [sz1, sz2, sz3, sz4, sz5, sz6, sz7] = getOutputSizeImpl(~)
            sz1 = [3 1]; sz2 = [3 1]; sz3 = [1 1]; sz4 = [1 1];
            sz5 = [1 1]; sz6 = [1 1]; sz7 = [1 1];
        end

        function [dt1, dt2, dt3, dt4, dt5, dt6, dt7] = getOutputDataTypeImpl(~)
            dt1 = 'single'; dt2 = 'single'; dt3 = 'uint8'; dt4 = 'uint8';
            dt5 = 'single'; dt6 = 'single'; dt7 = 'logical';
        end

        function [c1, c2, c3, c4, c5, c6, c7] = isOutputComplexImpl(~)
            c1 = false; c2 = false; c3 = false; c4 = false;
            c5 = false; c6 = false; c7 = false;
        end

        function [f1, f2, f3, f4, f5, f6, f7] = isOutputFixedSizeImpl(~)
            f1 = true; f2 = true; f3 = true; f4 = true;
            f5 = true; f6 = true; f7 = true;
        end
    end
end
