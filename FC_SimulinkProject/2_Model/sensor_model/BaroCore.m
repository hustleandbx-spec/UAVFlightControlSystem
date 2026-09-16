classdef BaroCore < matlab.System
% BaroCore  气压计传感器物理内核（薄外壳）
%
% 复用官方 altimeterSensor（Sensor Fusion and Tracking Toolbox）承载
% 常值偏置 / 噪声密度 / bias instability / 衰减因子，并用官方 atmoscoesa
% （Aerospace Toolbox，1976 COESA 标准大气）把高度读数换算为静压。
%
% 外壳只补官方不覆盖的系统语义：
%   - 内部一阶低通（数字滤波带宽）
%   - 量程饱和（器件 30~110 kPa）
%   - 输出量化（ADC LSB）
%   - 温度通道偏置与噪声
%   - 参考高度/坐标换算
%
% 输入 : pos_ned (3x1 single) — NED 位置真值
% 输出 : pressure (scalar single, Pa), temperature (scalar single, degC)
% 契约见 BarometerBus。
%
% SimulateUsing = Interpreted execution：本传感器模型仅作 MIL plant，后期植物
% 由 Gazebo 承担，不做代码生成。官方 atmoscoesa 不支持代码生成（见 stepImpl），
% 因此本类无法切到 Code generation，除非放弃官方标准大气模型。

    properties (Nontunable)
        SampleRate      = 50      % Hz，必须与所在采样率一致
        ConstantBias    = 0       % m
        NoiseDensity    = 0       % m/sqrt(Hz)
        BiasInstability = 0       % m
        DecayFactor     = 0       % -
        Seed            = 45601   % -
        RefAltMSL       = 0       % m，本地 NED 原点海拔
        TempOffset      = 0       % degC
        TempNoise       = 0       % degC，1-sigma
        LpfTau          = 0       % s，内部低通时间常数，0 = 关闭
        RangeMin        = 0       % Pa，量程下限，0 = 不饱和
        RangeMax        = 1e12    % Pa，量程上限
        Resolution      = 0       % Pa/LSB，量化步长，0 = 不量化
    end

    properties (Access = private)
        pAlt
        pStream
        pLpf
        pHasLpf
    end

    methods
        function obj = BaroCore(varargin)
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
            obj.pAlt = altimeterSensor( ...
                'SampleRate',      double(obj.SampleRate), ...
                'ConstantBias',    double(obj.ConstantBias), ...
                'NoiseDensity',    double(obj.NoiseDensity), ...
                'BiasInstability', double(obj.BiasInstability), ...
                'DecayFactor',     double(obj.DecayFactor), ...
                'RandomStream',    'mt19937ar with seed', ...
                'Seed',            double(obj.Seed));
            obj.pStream = RandStream('mt19937ar', 'Seed', double(obj.Seed) + 1);
            obj.pLpf    = 0;
            obj.pHasLpf = false;
        end

        function resetImpl(obj)
            reset(obj.pAlt);
            obj.pLpf    = 0;
            obj.pHasLpf = false;
        end

        function [pressure, temperature] = stepImpl(obj, pos_ned)
            % --- truth -> absolute altitude -> official altimeter reading ---
            h_abs  = double(obj.RefAltMSL) - double(pos_ned(3));
            h_read = double(obj.pAlt(single([0 0 -h_abs])));
            [T, ~, P, ~] = atmoscoesa(h_read, 'None');
            p = double(P);

            % --- internal first-order low-pass (digital filter bandwidth) ---
            if obj.LpfTau > 0
                Ts = 1 / double(obj.SampleRate);
                if ~obj.pHasLpf
                    obj.pLpf = p;
                    obj.pHasLpf = true;
                else
                    alpha = Ts / (double(obj.LpfTau) + Ts);
                    obj.pLpf = obj.pLpf + alpha * (p - obj.pLpf);
                end
                p = obj.pLpf;
            end

            % --- range saturation (device full scale) ---
            if obj.RangeMax > obj.RangeMin
                p = min(max(p, double(obj.RangeMin)), double(obj.RangeMax));
            end

            % --- output quantization (ADC LSB) ---
            if obj.Resolution > 0
                p = round(p / double(obj.Resolution)) * double(obj.Resolution);
            end

            pressure    = single(p);
            temperature = single(double(T) + double(obj.TempOffset) + ...
                                 double(obj.TempNoise) * randn(obj.pStream));
        end

        function [sz1, sz2] = getOutputSizeImpl(~)
            sz1 = [1 1];
            sz2 = [1 1];
        end

        function [dt1, dt2] = getOutputDataTypeImpl(~)
            dt1 = 'single';
            dt2 = 'single';
        end

        function [c1, c2] = isOutputComplexImpl(~)
            c1 = false;
            c2 = false;
        end

        function [f1, f2] = isOutputFixedSizeImpl(~)
            f1 = true;
            f2 = true;
        end
    end
end
