% shutdown.m - 无人机飞控项目环境清理

% 关闭所有打开的数据字典，避免文件锁定
try
    Simulink.data.dictionary.closeAll();
    disp('所有数据字典已关闭');
catch ME
    warning('关闭字典时出错: %s', ME.message);
end

% 重置 Simulink 文件生成控制
try
    Simulink.fileGenControl('reset');
    disp('Simulink 缓存配置已重置');
catch ME
    warning('重置缓存配置时出错: %s', ME.message);
end

disp('无人机飞控项目环境已清理');
