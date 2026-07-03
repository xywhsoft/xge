# Linux X11 窗口

本案例说明 Linux 桌面 X11 路径的接入和验证重点。

[返回范例解析](README.md) | [平台教程](../guide/platform-intro.md) | [Platform API](../api/platform.md)

## 状态

该页面描述接入脚手架和验证方法，不代表所有发行版已实机通过。实测结果以 `dev/docs/平台后端验证结果.md` 为准。

## 目标

```text
Sokol Linux backend
  -> X11 window
  -> OpenGL 3.3 或 GLES3/EGL
  -> XGE runtime
```

## 验证步骤

1. 运行最小窗口或平台冒烟程序。
2. 确认窗口显示 clear 颜色和 shape。
3. 确认 keyboard、mouse、text input 事件计数变化。
4. 输出 `xgeDebugDumpCaps`。
5. 关闭窗口并确认进程退出。

## 关键检查

```c
xge_platform_caps_t caps;
xge_platform_runtime_t rt;

xgePlatformCapsGet(&caps);
xgePlatformRuntimeGet(&rt);
```

## 常见失败原因

缺少 X11 开发/运行库。

远程或容器环境没有可用 display。

OpenGL 驱动版本不足或软件渲染被误用。
