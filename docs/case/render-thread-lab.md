# Render Thread Lab

`examples/render_thread_lab` 先在 manual / no-window 模式下验证 worker drain render thread，再尝试配置 EGL-owned render thread，最后开一个普通窗口展示当前 Sokol window context 为什么应该拒绝直接启用 render thread。

## 覆盖 API

- `xgeRenderThreadCapsGet`
- `xgeRenderThreadEGLSet`
- `xgeRenderThreadSet`
- `xgeRenderThreadGet`
- `xgeDraw`
- `xgeFlush`

## 构建和运行

```bat
examples\render_thread_lab\build.bat
build\xge_render_thread_lab.exe --frames 120
```

## 验证点

- 控制台先输出 `render-thread-lab init`。
- `worker=1` 路径表示 manual context 下支持 worker drain render thread。
- `queue=1` 且 `refs=2/1` 表示 draw 命令入队后纹理引用增加，`xgeFlush` 后被正确消费并释放。
- `enable=1`、`flush=1`、`disable=1` 表示 `xgeRenderThreadSet(1/0)` 与 `xgeRenderThreadGet` 流程正常。
- `egl_caps=1` 表示 EGL capability 查询成功。
- 如果 `skip=1`，则必须带有明确 `reason`，如 `egl-not-compiled` 或 `pbuffer-not-available`。
- 如果 `config=1 enable=1 owned=1`，表示 EGL render thread 配置成功，且 `bGLContextOwned=1`。
- 窗口阶段 `window=1/1 current=0` 表示当前 Sokol window context 能正确报告“不可与当前 context 组合使用”，并返回拒绝结果。
- 退出时打印 `render-thread-lab final-summary`。

## 可跳过条件

- 当前构建未编译 EGL，或运行环境不支持 pbuffer / EGL render thread 时，EGL 路径允许 skip，但必须输出明确原因。
- 如果本机没有可用系统字体，窗口阶段可以无文字展示；不影响 worker/EGL 路径通过。
