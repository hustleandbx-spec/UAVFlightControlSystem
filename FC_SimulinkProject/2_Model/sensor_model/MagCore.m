classdef MagCore < matlab.System
%MagCore  磁力计传感器物理内核（薄外壳）。
%
% 复用官方 imuSensor('accel-mag') + magparams 承载硬磁/软磁/轴失准/白噪声/
% bias instability/随机游走/温度项；参考场用官方 wrldmagm（Aerospace Toolbox，
% WMM）由 NED 位置换算，与 BaroCore 用 atmoscoesa 的做法同构。
% 本类只做量纲、坐标与参考场换算，不重写任何误差机理。
%
% 输入 : pos_ned       (3x1 single) NED 位置真值 [m]
%        attitude_quat (4x1 single) body->NED 四元数 [w x y z]
% 输出 : magBody       (3x1 single) 机体磁场测量 [uT]
%        decl_rad      (1x1 single) 该点 WMM 磁偏角 [rad]
%
% 契约见 MagnetometerBus。已实测: magparams 单位为 uT；AxesMisalignment 为 %，
% 100 = 单位阵；imuSensor 的 orientation 为 NED->body（C_nb，即 Attitude_quat 的共轭）。
%
% SimulateUsing = Interpreted execution：本传感器模型仅作 MIL plant，后期植物
% 由 Gazebo 承担，不做代码生成。官方 wrldmagm 不支持代码生成（见 stepImpl），
% 因此本类无法切到 Code generation，除非放弃官方 WMM 参考场。

    properties (Nontunable)
        SampleRate             = 100            % Hz
        Seed                   = 77101          % -
        RefLat                 = 0              % deg，本地 NED 原点纬度
        RefLon                 = 10             % deg，本地 NED 原点经度
        WmmEpoch               = 2025           % 十进制年
        ConstantBias           = [0 0 0]        % uT
        AxesMisalignment       = 100*eye(3)     % %，100 = 单位阵
        NoiseDensity           = [0 0 0]        % uT/sqrt(Hz)
        BiasInstability        = [0 0 0]        % uT
        RandomWalk             = [0 0 0]        % uT*sqrt(Hz)
        TemperatureBias        = [0 0 0]        % uT/degC
        TemperatureScaleFactor = [0 0 0]        % %/degC
        Temperature            = 25             % degC
    end

    properties (Access = private)
        pIMU
    end

    methods
        function obj = MagCore(varargin)
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
            mp = magparams;
            mp.ConstantBias           = double(obj.ConstantBias(:).');
            mp.AxesMisalignment       = double(obj.AxesMisalignment);
            mp.NoiseDensity           = double(obj.NoiseDensity(:).');
            mp.BiasInstability        = double(obj.BiasInstability(:).');
            mp.RandomWalk             = double(obj.RandomWalk(:).');
            mp.TemperatureBias        = double(obj.TemperatureBias(:).');
            mp.TemperatureScaleFactor = double(obj.TemperatureScaleFactor(:).');

            imu = imuSensor('accel-mag', ...
                'SampleRate',   double(obj.SampleRate), ...
                'Magnetometer', mp);
            imu.RandomStream = 'mt19937ar with seed';
            imu.Seed         = double(obj.Seed);
            imu.Temperature  = double(obj.Temperature);
            obj.pIMU = imu;
        end

        function resetImpl(obj)
            reset(obj.pIMU);
        end

        function [magBody, decl_rad] = stepImpl(obj, pos_ned, attitude_quat)
            R   = single(6371000);
            r2d = single(180/pi);
            d2r = single(pi/180);
            lat = single(obj.RefLat) + (pos_ned(1)/R)*r2d;
            cl  = cos(single(obj.RefLat)*d2r);
            lon = single(obj.RefLon) + (pos_ned(2)/(R*cl))*r2d;
            h   = -pos_ned(3);

            [XYZ,~,D,~,~] = wrldmagm(double(h), double(lat), double(lon), double(obj.WmmEpoch));
            m_n = single(XYZ(:))/single(1000);          % nT -> uT, NED

            C_bn = double(quat2rotm(double(attitude_quat(:).')));   % body->NED
            C_nb = C_bn.';                                          % NED->body (imuSensor needs double)

            obj.pIMU.MagneticField = double(m_n).';   % 位置相关，逐点更新
            [~, mb] = obj.pIMU(zeros(1,3), zeros(1,3), C_nb);

            magBody  = single(mb(:));
            decl_rad = single(D)*d2r;
        end

        function [sz1, sz2] = getOutputSizeImpl(~)
            sz1 = [3 1]; sz2 = [1 1];
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
