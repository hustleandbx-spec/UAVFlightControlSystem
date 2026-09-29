classdef DelayEKFBlock < matlab.System
%DELAYEKFBLO CK  自包含 EKF 块：调度（环形缓冲+时间对齐）+ EKF 预测/更新 + 求差 + 互补滤波
%
% 输入（总线直入，一次解包打包进数组，行 = 一个样本）：
%   ImuBus / GpsBus / MagBus / BaroBus（字段契约见 BusConfig/StateEst）
%
% 每拍流程：
%   1) 四路总线入队：IMU 存 ImuDataBuffer(nImu x 8)，
%      测量存 MeasDataBuffer(nMeas x 7 x 3，页 k=1 GPS, 2 BARO, 3 MAG)；
%   2) AdvanceFusionHorizon：推进延迟索引 DelayedIdx，
%      融合时刻 FuseTimeUs = 最新IMU − DelayMaxUs，取延迟样本（FuseDtUs/Gyro/Accel）；
%   3) FusePredict：延迟样本推进融合态 XFused(16x1)/PFused(15x15)；
%   4) 观测时间对齐：tValid = 交付时刻 − SensorDelay，陈旧丢弃/到期出队，
%      按 FuseMask 序贯 FuseUpdateGps/Baro/Mag（GPS 经纬高经外部 GpsToNed + 类内原点锁存，
%      气压经外部 BaroToNedZ 换算）；
%   5) 求差 DeltaFuse(XFused, XPred) + 互补滤波（B 结构）：
%      输出态 = 最新 IMU 外推（高速支路） + 融合修正量 delta 反馈（低速支路）；
%   6) StatePack 打包五路契约输出 est/snapshot/attitude/health/diagnostic。
%
% 状态布局 16 维：[p(3); v(3); q(4,wxyz); ba(3); bg(3)]，协方差 15x15（误差态）。
% 外部依赖：FusePredict/FuseUpdateGps/FuseUpdateBaro/FuseUpdateMag/DeltaFuse/StatePack/
%           BaroToNedZ/GpsToNed（同目录或 5_Tool 的 quat_*/skew_sym）。

    properties (Nontunable)
        % ===== 调度 / 时间对齐 =====
        DelayMaxUs      = 30000   % 融合延迟：融合时刻 = 最新IMU时刻 − DelayMaxUs
        MaxLateUs       = 30000   % 早于融合时刻超过此值 → 陈旧丢弃
        ImuBufLen       = 48      % IMU 环形缓冲容量（保留延迟窗口的历史）
        MeasBufLen      = 8       % 每路测量环形缓冲容量
        MaxFusePerTick  = 1       % 每拍每路最多出队样本数
        GpsDelayUs      = 20000   % 镜像 sensor 侧 GPS 标称延迟
        BaroDelayUs     = 5000    % 镜像 sensor 侧气压计标称延迟
        MagDelayUs      = 10000   % 镜像 sensor 侧磁强计标称延迟
        ImuSampleTimeUs = 1000    % 首拍 dt 兜底
        % ===== EKF 配置 =====
        GravityNed      = single([0; 0; 9.81])          % g_n（verify B 用 9.81）
        NoiseCfg        = single([0.05; 0.01; 1e-3; 1e-4])  % [sigma_acc; sigma_gyr; sigma_ba; sigma_bg]（以 state_estimation_params.yaml 为准）
        RPos            = single(1.5)                    % GPS 位置噪声（以 EKF.slx 为准）
        RVel            = single(0.3)                     % GPS 速度噪声
        RBaro           = single(4.0)                     % 气压高度噪声
        RMag            = single(25.0)                    % 磁力计噪声 uT^2（sigma~5uT，PX4 量级；初值大航向误差不被野值门限卡死）
        Gate2Gps        = single(25.0)                    % χ²(6,0.999)
        Gate2Baro       = single(12.0)                    % χ²(1,0.999)
        Gate2Mag        = single(16.0)                    % χ²(3,0.999)
        MagNed          = single([33.9322; -1.4444; 38.2691])  % 成都参考场 uT（verify / SE_EKF_MAG_NED）
        XInit           = single([0;0;0; 0;0;0; 1;0;0;0; 0;0;0; 0;0;0])  % 初始融合态 16x1
        PInit           = single(diag([0.25 0.25 0.25 0.04 0.04 0.04 0.01 0.01 0.01 2.5e-3 2.5e-3 2.5e-3 1e-4 1e-4 1e-4]))  % 初始协方差 15x15
        % ===== GPS 转换（外部 GpsToNed）=====
        MetersPerDeg    = 111320.0                        % 纬度每度米数（verify）
        OriginLat0      = single(30.0)                    % 兜底原点（首帧有效 GPS 覆盖锁存）
        OriginLon0      = single(104.0)
        RefAltGps0      = single(0.0)
        % ===== 气压转换（外部 BaroToNedZ）=====
        QnhPa           = single(101325.0)
        RefAltBaro      = single(0.0)
        HScale          = single(44330.0)
        IsaExp          = single(0.190263)
    end

    properties (Access = private)
        % ===== 调度缓冲（行 = 一个样本）=====
        ImuDataBuffer   % nImu x 8：[TsUs, DtUs, Accel(3), Gyro(3)]
        ImuHead         % 单调递增逻辑序号（下一个待写入位置）
        ImuTail         % 单调递增逻辑序号（最老样本位置）
        ImuNewestUs     % 缓冲内最新样本时刻（窗口右边界）
        MeasDataBuffer  % nMeas x 7 x 3：[TsUs, Z(6)]，页 k=1 GPS, 2 BARO, 3 MAG
        MeasHead        % 1x3
        MeasTail        % 1x3
        DelayedIdx      % 延迟样本逻辑序号，NaN = 未建立
        Ready           % 融合窗口就绪
        LastImuUs       % 上一 IMU 时刻（dt 计算）
        % ===== EKF 状态 =====
        XFused          % 16x1 融合态（锚定 FuseTimeUs）
        PFused          % 15x15 融合协方差
        XPred           % 16x1 本拍预测态（DeltaFuse 求差基准）
        XOut            % 16x1 输出态（当前时刻，互补滤波）
        AngRateOut      % 3x1 输出角速率（陀螺 − 零偏）
        % ===== GPS 原点锁存（类内持有状态，公式在外部 GpsToNed）=====
        GpsLocked       % logical：首帧有效 GPS 后为 true，不再变
        OriginLat       % 锁存原点（锁存后不变）
        OriginLon
        OriginRefAlt
        % ===== 最近帧（供 StatePack）=====
        LastImuSeq      % uint32
        LastImuTime     % double 秒
        LastImuValid    % logical
        LastMagHeadingValid
        LastBaroValid
        LastAccel       % 3x1 最近一帧原始比力（StatePack 诊断用）
        % ===== 内部计数（保留内部，不出端口）=====
        MeasFused       % 1x3
        MeasDroppedLate % 1x3
        MeasDroppedFull % 1x3
        ImuOverflow
    end

    methods (Access = protected)

        function [sz1, sz2, sz3, sz4, sz5] = getOutputSizeImpl(~)
            sz1 = [1 1]; sz2 = [1 1]; sz3 = [1 1]; sz4 = [1 1]; sz5 = [1 1];
        end

        function [dt1, dt2, dt3, dt4, dt5] = getOutputDataTypeImpl(~)
            dt1 = 'Bus: StateEstBus';
            dt2 = 'Bus: StateSnapshot';
            dt3 = 'Bus: StateAttitudeSnapshotBus';
            dt4 = 'Bus: StateEstHealthBus';
            dt5 = 'Bus: StateEstDiagnosticBus';
        end

        function [dt1, dt2, dt3, dt4] = getInputDataTypeImpl(~)
            dt1 = 'Bus: IMUBus';
            dt2 = 'Bus: GPSBus';
            dt3 = 'Bus: MagnetometerBus';
            dt4 = 'Bus: BarometerBus';
        end

        function [f1, f2, f3, f4, f5] = isOutputFixedSizeImpl(~)
            f1 = true; f2 = true; f3 = true; f4 = true; f5 = true;
        end

        function [c1, c2, c3, c4, c5] = isOutputComplexImpl(~)
            c1 = false; c2 = false; c3 = false; c4 = false; c5 = false;
        end

        function setupImpl(obj)
            nImu  = double(obj.ImuBufLen);
            nMeas = double(obj.MeasBufLen);
            obj.ImuDataBuffer  = zeros(nImu, 8);
            obj.MeasDataBuffer = zeros(nMeas, 7, 3);
            obj.ResetState();
        end

        function [est, snapshot, attitude, health, diagnostic] = stepImpl(obj, ...
                  ImuBus, GpsBus, MagBus, BaroBus)

            %% 1. 总线入队（一次解包打包）
            if ImuBus.Valid && ImuBus.IsNew
                obj.PushImu(ImuBus);
                obj.LastImuSeq    = ImuBus.Sequence;
                obj.LastImuTime   = ImuBus.Timestamp;
                obj.LastImuValid  = ImuBus.Valid;
                obj.LastAccel     = single(double(ImuBus.Accel(:)));
            end
            if GpsBus.Valid && GpsBus.IsNew
                obj.PushMeas(1, round(double(GpsBus.Timestamp) * 1e6), obj.PackGpsZ(GpsBus));
            end
            if BaroBus.Valid && BaroBus.IsNew
                obj.PushMeas(2, round(double(BaroBus.Timestamp) * 1e6), obj.PackBaroZ(BaroBus));
                obj.LastBaroValid = BaroBus.Valid;
            end
            if MagBus.Valid && MagBus.IsNew
                obj.PushMeas(3, round(double(MagBus.Timestamp) * 1e6), obj.PackMagZ(MagBus));
                obj.LastMagHeadingValid = MagBus.HeadingValid;
            end

            %% 2. 最新 IMU（输出外推用）
            accelNew = single(zeros(3,1));
            gyroNew  = single(zeros(3,1));
            dtNew    = 0;
            if (obj.ImuHead - obj.ImuTail) > 0
                sNew = obj.SlotOf(obj.ImuHead - 1, obj.ImuBufLen);
                accelNew = single(obj.ImuDataBuffer(sNew, 3:5).');
                gyroNew  = single(obj.ImuDataBuffer(sNew, 6:8).');
                dtNew    = double(obj.ImuDataBuffer(sNew, 2)) / 1e6;   % us -> s
            end

            %% 3. 融合状态推进（延迟样本 → FusePredict）
            [obj.Ready, FuseAdvanced, FuseTimeUs, FuseDtUs, FuseGyro, FuseAccel] = obj.AdvanceFusionHorizon();
            if obj.Ready && FuseAdvanced && (FuseDtUs > 0)
                [x_p, P_p] = FusePredict(obj.XFused, obj.PFused, ...
                    FuseAccel, FuseGyro, double(FuseDtUs)/1e6, obj.GravityNed, obj.NoiseCfg);   % FuseDtUs[us]->dt[s]
                obj.XPred  = x_p;      % 求差基准：本拍预测态（更新前）
                obj.XFused = x_p;
                obj.PFused = P_p;
            end

            %% 4. 观测时间对齐 → 序贯更新（GPS → BARO → MAG）
            if obj.Ready
                FuseMask = false(1,3);
                ZOut     = zeros(6,3);
                for k = 1:3
                    nThisTick = 0;
                    while (obj.MeasHead(k) - obj.MeasTail(k) > 0) && ...
                          (nThisTick < double(obj.MaxFusePerTick))
                        [TDelivered, Z] = obj.PeekMeas(k);
                        TValid = TDelivered - obj.SensorDelayUs(k);
                        if TValid < FuseTimeUs - double(obj.MaxLateUs)
                            % 陈旧：丢弃，绝不回放
                            obj.PopMeas(k);
                            obj.MeasDroppedLate(k) = obj.MeasDroppedLate(k) + 1;
                            continue;
                        end
                        if TValid > FuseTimeUs
                            break;   % 未到期：留在缓冲，等融合时刻追上来
                        end
                        FuseMask(k) = true;
                        ZOut(:,k)   = Z;
                        obj.PopMeas(k);
                        obj.MeasFused(k) = obj.MeasFused(k) + 1;
                        nThisTick = nThisTick + 1;
                    end
                end
                if FuseMask(1)
                    posNed = obj.GpsToNedPos(ZOut(1:3,1));
                    velNed = single(ZOut(4:6,1));
                    [obj.XFused, obj.PFused] = FuseUpdateGps(obj.XFused, obj.PFused, ...
                        posNed, velNed, obj.RPos, obj.RVel, obj.Gate2Gps);
                end
                if FuseMask(2)
                    zBaro = BaroToNedZ(ZOut(1,2), obj.QnhPa, obj.RefAltBaro, obj.HScale, obj.IsaExp);
                    [obj.XFused, obj.PFused] = FuseUpdateBaro(obj.XFused, obj.PFused, ...
                        zBaro, obj.RBaro, obj.Gate2Baro);
                end
                if FuseMask(3)
                    magBody = single(ZOut(1:3,3));
                    [obj.XFused, obj.PFused] = FuseUpdateMag(obj.XFused, obj.PFused, ...
                        magBody, obj.MagNed, obj.RMag, obj.Gate2Mag);
                end
            end

            %% 5. 求差 + 互补滤波（B 结构：IMU 外推高速支路 + 融合修正低速支路）
            if obj.Ready
                delta = DeltaFuse(obj.XFused, obj.XPred);
            else
                delta = single([0;0;0; 0;0;0; 1;0;0;0; 0;0;0; 0;0;0]);
            end
            obj.OutputComplementary(delta, accelNew, gyroNew, dtNew);

            %% 6. 打包输出（五路契约）
            [est, snapshot, attitude, health, diagnostic] = StatePack(...
                obj.XOut, obj.LastAccel, obj.AngRateOut, obj.LastImuSeq, ...
                obj.LastImuTime, obj.LastImuValid, obj.Ready, obj.GpsLocked, ...
                obj.LastMagHeadingValid, obj.LastBaroValid);
        end

        function resetImpl(obj)
            obj.ResetState();
        end
    end

    methods (Access = private)

        function ResetState(obj)
            obj.ImuHead = 1;  obj.ImuTail = 1;  obj.ImuNewestUs = NaN;
            obj.DelayedIdx = NaN;
            obj.MeasHead = ones(1,3);  obj.MeasTail = ones(1,3);
            obj.Ready = false;
            obj.LastImuUs = NaN;
            obj.XFused  = single(obj.XInit);
            obj.PFused  = single(obj.PInit);
            obj.XPred   = single(obj.XInit);
            obj.XOut    = single(obj.XInit);
            obj.AngRateOut = single(zeros(3,1));
            obj.GpsLocked  = false;
            obj.OriginLat    = double(obj.OriginLat0);
            obj.OriginLon    = double(obj.OriginLon0);
            obj.OriginRefAlt = double(obj.RefAltGps0);
            obj.LastImuSeq   = uint32(0);
            obj.LastImuTime  = NaN;
            obj.LastImuValid = false;
            obj.LastMagHeadingValid = false;
            obj.LastBaroValid = false;
            obj.LastAccel = single(zeros(3,1));
            obj.MeasFused = zeros(1,3);
            obj.MeasDroppedLate = zeros(1,3);
            obj.MeasDroppedFull = zeros(1,3);
            obj.ImuOverflow = 0;
        end

        function s = SlotOf(~, Idx, Capacity)
            % 逻辑序号 → 物理槽位。Idx 单调递增，只有这里取模。
            s = mod(Idx - 1, double(Capacity)) + 1;
        end

        function PushImu(obj, ImuBus)
            % 满：丢最老（已越过融合时刻的样本无价值），保延迟窗口与最新样本。
            if (obj.ImuHead - obj.ImuTail) >= double(obj.ImuBufLen)
                obj.ImuOverflow = obj.ImuOverflow + 1;
                obj.ImuTail     = obj.ImuTail + 1;
                return;
            end
            % 一次解包打包：IMU 总线 → 1x8 行向量 [TsUs, DtUs, Accel(3), Gyro(3)]，一行一个样本
            TsUs = round(double(ImuBus.Timestamp) * 1e6);
            if isnan(obj.LastImuUs)
                DtUs = double(obj.ImuSampleTimeUs);
            else
                DtUs = TsUs - obj.LastImuUs;
            end
            s = obj.SlotOf(obj.ImuHead, obj.ImuBufLen);
            obj.ImuDataBuffer(s, :) = [TsUs, DtUs, double(ImuBus.Accel(:)).', double(ImuBus.Gyro(:)).'];
            obj.ImuHead     = obj.ImuHead + 1;
            obj.LastImuUs   = TsUs;
            obj.ImuNewestUs = TsUs;

            % 保留窗口退休：缓冲只需覆盖 [Newest − DelayMax − 一个IMU周期, Newest]。
            % 滑出窗口的最老样本对融合已无价值，主动推进 ImuTail。
            while (obj.ImuHead - obj.ImuTail) > 0 && ...
                  (obj.ImuNewestUs - obj.ImuDataBuffer(obj.SlotOf(obj.ImuTail, obj.ImuBufLen), 1)) > ...
                  (double(obj.DelayMaxUs) + double(obj.ImuSampleTimeUs))
                obj.ImuTail = obj.ImuTail + 1;
            end
        end

        function PushMeas(obj, k, TsUs, Z)
            % 测量入队：每行一个样本 [TsUs, Z(6)]，页 k = 传感器
            if (obj.MeasHead(k) - obj.MeasTail(k)) >= double(obj.MeasBufLen)
                obj.MeasDroppedFull(k) = obj.MeasDroppedFull(k) + 1;
                return;
            end
            s = obj.SlotOf(obj.MeasHead(k), obj.MeasBufLen);
            obj.MeasDataBuffer(s, :, k) = [TsUs, double(Z(1:6)).'];
            obj.MeasHead(k) = obj.MeasHead(k) + 1;
        end

        function [TsUs, Z] = PeekMeas(obj, k)
            s = obj.SlotOf(obj.MeasTail(k), obj.MeasBufLen);
            TsUs = obj.MeasDataBuffer(s, 1, k);
            Z    = obj.MeasDataBuffer(s, 2:7, k).';
        end

        function PopMeas(obj, k)
            obj.MeasTail(k) = obj.MeasTail(k) + 1;
        end

        function d = SensorDelayUs(obj, k)
            if k == 1
                d = double(obj.GpsDelayUs);
            elseif k == 2
                d = double(obj.BaroDelayUs);
            else
                d = double(obj.MagDelayUs);
            end
        end

        function Z = PackGpsZ(~, Bus)
            % GPS 打包：6 行满 [Lat; Lon; Alt; Velocity(3)]
            Z = [double(Bus.Lat); double(Bus.Lon); double(Bus.Alt); double(Bus.Velocity(:))];
        end

        function Z = PackBaroZ(~, Bus)
            % BARO 打包：只用第 1 行（Pressure），其余补零
            Z = [double(Bus.Pressure); zeros(5,1)];
        end

        function Z = PackMagZ(~, Bus)
            % MAG 打包：用 1:3 行（Mag_Body），其余补零
            Z = [double(Bus.Mag_Body(:)); zeros(3,1)];
        end

        function [Ready, Advanced, FuseTimeUs, DtUs, Gyro, Accel] = AdvanceFusionHorizon(obj)
            % 把延迟索引 DelayedIdx 推进到"时间戳 >= 最新−DelayMax"的最老样本，
            % 该样本即融合时刻的延迟样本，供类内 FusePredict 推进融合状态。
            % Advanced：本拍 DelayedIdx 是否真的前进（或首次建立）。
            %   必须以它为门控：时钟未推进时重复使用同一条 FuseDtUs 会重复积分。
            Ready      = false;
            Advanced   = false;
            FuseTimeUs = NaN;
            DtUs       = 0;
            Gyro       = single(zeros(3,1));
            Accel      = single(zeros(3,1));
            IdxBefore  = obj.DelayedIdx;

            % 无 IMU 或缓冲空 → 无法建立融合窗口
            if isnan(obj.ImuNewestUs) || (obj.ImuHead - obj.ImuTail <= 0)
                return;
            end
            % 缓冲跨度未达融合延迟 → 融合窗口未建立（启动阶段）
            if (obj.ImuNewestUs - obj.ImuDataBuffer(obj.SlotOf(obj.ImuTail, obj.ImuBufLen), 1)) < ...
               double(obj.DelayMaxUs)
                return;
            end

            fuseAnchorUs = obj.ImuNewestUs - double(obj.DelayMaxUs);
            if isnan(obj.DelayedIdx)
                obj.DelayedIdx = obj.ImuTail;
            elseif obj.DelayedIdx < obj.ImuTail
                obj.DelayedIdx = obj.ImuTail;   % 防御：最老样本被满缓冲丢弃
            end
            % 推进到第一个时刻 >= fuseAnchorUs 的样本（缓冲内时间戳单调）
            while (obj.DelayedIdx <= obj.ImuHead - 1) && ...
                  (obj.ImuDataBuffer(obj.SlotOf(obj.DelayedIdx, obj.ImuBufLen), 1) < fuseAnchorUs)
                obj.DelayedIdx = obj.DelayedIdx + 1;
            end

            if obj.DelayedIdx <= obj.ImuHead - 1
                s = obj.SlotOf(obj.DelayedIdx, obj.ImuBufLen);
                Ready      = true;
                FuseTimeUs = obj.ImuDataBuffer(s, 1);
                DtUs       = obj.ImuDataBuffer(s, 2);
                Gyro       = single(obj.ImuDataBuffer(s, 6:8).');
                Accel      = single(obj.ImuDataBuffer(s, 3:5).');
                if isnan(IdxBefore)
                    Advanced = true;                       % 首次建立融合窗口
                else
                    Advanced = obj.DelayedIdx > IdxBefore; % 本拍推进了一格或多格
                end
            end
        end

        function posNed = GpsToNedPos(obj, latLonAlt)
            % GPS 经纬高 → NED 位置（外部纯转换 GpsToNed + 类内原点锁存）
            lat = double(latLonAlt(1));
            lon = double(latLonAlt(2));
            alt = double(latLonAlt(3));
            if ~obj.GpsLocked
                obj.GpsLocked    = true;
                obj.OriginLat    = lat;
                obj.OriginLon    = lon;
                obj.OriginRefAlt = alt;   % 首帧有效 GPS 高度作 NED 下向基准
            end
            posNed = GpsToNed(lat, lon, alt, obj.OriginLat, obj.OriginLon, obj.OriginRefAlt, obj.MetersPerDeg);
        end

        function OutputComplementary(obj, delta, accelNew, gyroNew, dtNew)
            % 互补滤波（B 结构）：
            %   高速支路 —— 输出态用最新 IMU 外推（保留 IMU 高频响应）；
            %   低速支路 —— 融合修正量 delta 反馈注入输出（低频校正，拉回漂移）。
            g_n = obj.GravityNed;
            if obj.Ready
                if dtNew > 0
                    ba = obj.XOut(11:13);  bg = obj.XOut(14:16);
                    accel_corr = accelNew - ba;
                    omega_corr = gyroNew  - bg;
                    dq = quat_exp(omega_corr, dtNew);
                    q  = quat_normalize(quat_mult(obj.XOut(7:10), dq));
                    C_bn = quat2rotm(obj.XOut(7:10));
                    a_n  = C_bn * accel_corr + g_n;
                    v = obj.XOut(4:6) + a_n * dtNew;
                    p = obj.XOut(1:3) + v * dtNew + 0.5 * a_n * dtNew^2;
                    obj.XOut = [p; v; q; ba; bg];
                end
                % 融合修正反馈（求差量注入输出）
                obj.XOut(1:3)   = obj.XOut(1:3)   + delta(1:3);
                obj.XOut(4:6)   = obj.XOut(4:6)   + delta(4:6);
                obj.XOut(7:10)  = quat_normalize(quat_mult(delta(7:10), obj.XOut(7:10)));
                obj.XOut(11:13) = obj.XOut(11:13) + delta(11:13);
                obj.XOut(14:16) = obj.XOut(14:16) + delta(14:16);
                obj.AngRateOut  = gyroNew - obj.XOut(14:16);
            elseif dtNew > 0
                % 启动段（融合窗口未建立）：输出态自身外推，保持输出连续
                ba = obj.XOut(11:13);  bg = obj.XOut(14:16);
                accel_corr = accelNew - ba;
                omega_corr = gyroNew  - bg;
                dq = quat_exp(omega_corr, dtNew);
                q  = quat_normalize(quat_mult(obj.XOut(7:10), dq));
                C_bn = quat2rotm(obj.XOut(7:10));
                a_n  = C_bn * accel_corr + g_n;
                v = obj.XOut(4:6) + a_n * dtNew;
                p = obj.XOut(1:3) + v * dtNew + 0.5 * a_n * dtNew^2;
                obj.XOut = [p; v; q; ba; bg];
                obj.AngRateOut = omega_corr;
            end
        end
    end
end
