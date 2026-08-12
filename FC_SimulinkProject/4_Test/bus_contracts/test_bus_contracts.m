function test_bus_contracts()
%TEST_BUS_CONTRACTS 批量验证全部 Bus 定义与 A2 契约一致（L1 静态契约符合性）。
%   目的：本步骤（数据字典/契约落地）的核心校验——每个 config_*.m 的字段集 == 契约
%         （docs/product/10_Interface_Definition §4），无缺失、无多余、无判据泄漏/任务语义泄漏。
%   数据源：bus_contracts.m 单一权威契约表（本测试只遍历，不内嵌契约，避免双源漂移）。
%   方法：逐 Bus 调 checkBusConfig 校验（期望字段逐项存在 + 禁止字段逐项不存在）。
%   输出：逐 Bus PASS/FAIL + 汇总；任一失败抛异常（assert），通过打印 ALL_BUS_CONTRACTS_PASS。
%   环境：纯文本检查，无需 MATLAB 运行模型（与 test_navigator_contract.m 同型）。
%
%   分层说明：本测 = L1 静态契约符合性；L2 字典可构建（create_GlobalTypes）、
%   L3 模型引用一致性、L4 行为随模型重构后补（见 13 V&V 计划）。

C = bus_contracts();
fprintf('=== Bus 契约符合性批量校验（%d 个 Bus）===\n', numel(C));
nfail = 0;
for i = 1:numel(C)
    try
        checkBusConfig(C(i).name, C(i).expected, C(i).forbidden);
        fprintf('  [PASS] %-24s %s\n', C(i).name, C(i).contract);
    catch err
        nfail = nfail + 1;
        fprintf('  [FAIL] %-24s %s\n         %s\n', C(i).name, C(i).contract, err.message);
    end
end
npass = numel(C) - nfail;
fprintf('=== 汇总：%d 通过 / %d 失败 ===\n', npass, nfail);
assert(nfail == 0, '存在 %d 个 Bus 契约不符合项。', nfail);
fprintf('ALL_BUS_CONTRACTS_PASS\n');
end
