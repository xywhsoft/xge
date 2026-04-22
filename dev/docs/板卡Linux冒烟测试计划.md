# 板卡 Linux 冒烟测试计划

目标板卡：近几年支持 GLES3/EGL 的 Linux 板卡，例如 RK3566。

## 环境检查

- `libEGL.so` 可加载。
- `libGLESv2.so` 可加载。
- `/dev/dri/card*` 或窗口系统可用。
- 输入设备权限已配置。

## 测试步骤

0. 运行 EGL pbuffer 冒烟示例，确认 headless EGL context 可创建：

```sh
./build_board_linux_egl_exe.sh
./build/xge_board_linux_egl
```

0.1 如果目标驱动支持 `EGL_KHR_surfaceless_context`，运行 surfaceless 冒烟示例：

```sh
./build_egl_surfaceless_exe.sh
./build/xge_egl_surfaceless
```

如果目标系统同时具备 pbuffer 和 surfaceless 条件，也可以运行一键脚本：

```sh
./check_board_linux_egl.sh
```

1. 运行基础 hello，确认 EGL context 创建成功。
2. 绘制纯色三角形或矩形。
3. 加载 PNG 纹理并绘制 sprite。
4. 加载 XRF 字体并绘制文本。
5. 创建 render target 并 readback。
6. 测试 touch 或 mouse 输入。
7. 连续运行 10 分钟观察内存和帧稳定性。

## 通过标准

- 启动无崩溃。
- 画面非黑屏。
- readback 得到非空 RGBA 数据。
- 输入事件能进入 XGE。
- debug caps 能输出 EGL/GLES 信息。
- EGL 失败时 `xge_egl_context_t.sLastStage` 能指出失败阶段，`iLastError` 能保留 EGL error code。

`build_board_linux_egl_exe.sh` 当前只验证 EGL pbuffer context 创建和 `eglMakeCurrent`，不代表 DRM/KMS/GBM 显示链路已经完成。
