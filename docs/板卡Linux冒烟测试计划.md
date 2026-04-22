# 板卡 Linux 冒烟测试计划

目标板卡：近几年支持 GLES3/EGL 的 Linux 板卡，例如 RK3566。

## 环境检查

- `libEGL.so` 可加载。
- `libGLESv2.so` 可加载。
- `/dev/dri/card*` 或窗口系统可用。
- 输入设备权限已配置。

## 测试步骤

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
