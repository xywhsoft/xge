# XGE Sokol 后端边界说明

本文档记录当前 vendored Sokol 后端的实际覆盖边界，避免把计划目标误写成已实现能力。

## 当前 vendored Sokol 状态

当前 `lib/sokol/sokol_app.h` 的 Linux 桌面路径基于 X11：

- 默认 `SOKOL_GLCORE` 使用 X11 + GLX。
- 定义 `SOKOL_FORCE_EGL` 时可使用 X11 + EGL。
- 定义 `SOKOL_GLES3` 时使用 GLES3/EGL。
- 当前文件中未发现 Wayland 后端实现。

因此，XGE 当前不能把 Linux Wayland 标记为“由 Sokol 运行路径支持”。

`xgePlatformCapsGet` 会报告当前 Sokol 编译目标和图形后端：

- `sSokolTargetName`：当前编译目标，例如 `windows`、`linux-x11`、`android`、`web-emscripten`。
- `bSokolGLCore`：当前二进制使用 Sokol GLCore 后端。
- `bSokolGLES3`：当前二进制使用 Sokol GLES3 后端。
- `bSokolD3D11`：当前二进制使用 Sokol D3D11 后端。
- `bSokolMetal`：当前二进制使用 Sokol Metal 后端。
- `bSokolDummy`：当前二进制使用 Sokol dummy backend。

这些字段用于 smoke 示例输出诊断。它们只能说明编译路径，不代表窗口、输入、IME、gamepad 或高 DPI 已在目标机验证。

## Linux 路径划分

第一阶段 Linux 桌面实际可推进路径：

- Linux X11 + OpenGL Core：`build_exe.sh`。
- Linux X11 + EGL/GLES3：`build_linux_egl_exe.sh`。

第一阶段 Linux Wayland 只保留设计目标：

- 后续升级到支持 Wayland 的平台库。
- 或实现 XGE 自有 Wayland/EGL 平台后端。
- 或在运行时 fallback 到 X11/XWayland。

## spec 规则

- `Linux Sokol X11 路径` 需要 Linux X11 实机完成窗口冒烟后才能关闭。
- `Linux Sokol Wayland 路径或能力检测` 当前只能代表能力检测/fallback 设计，不代表已有 Wayland 窗口实现。
- 如果后续必须原生 Wayland，需新增独立任务，不能复用当前 Sokol X11 任务直接关闭。
