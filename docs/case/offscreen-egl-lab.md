# Offscreen EGL Lab

`examples/offscreen_egl_lab` 先探测一次独立 EGL pbuffer 路径，再在普通窗口上下文里执行 `xgeOffscreenInit`、offscreen render target、clear 和 readback。没有 EGL 的环境会输出明确 skip 原因，但 offscreen fallback render target 仍会继续验证。

## 覆盖 API

- `xgeEGLCapsGet`
- `xgeEGLInit`
- `xgeEGLMakeCurrent`
- `xgeEGLUnit`
- `xgeOffscreenInit`
- `xgeOffscreenRenderTarget`
- `xgeOffscreenReadPixels`
- `xgeOffscreenUnit`

## 构建和运行

```bat
examples\offscreen_egl_lab\build.bat
build\xge_offscreen_egl_lab.exe --frames 120
```

## 验证点

- 控制台输出 `offscreen-egl-lab init`。
- `caps=1` 表示 EGL capability 查询成功。
- `skip=1` 且 `reason=egl-not-compiled`、`pbuffer-not-available` 或类似原因时，表示当前机器无 EGL，属于明确跳过。
- `offscreen=1 target=1 readback=1` 表示 offscreen 初始化、target 获取和 readback 都成功。
- `fallback=1` 表示当前环境走了普通窗口 OpenGL fallback render target。
- `rgba=34,68,102,255` 表示 readback 读到了 offscreen clear color。

## 可跳过条件

- 没有 EGL 编译支持、没有 pbuffer、或者目标平台未提供可用 EGL 驱动时，EGL 探测阶段允许 skip，但必须输出明确原因。
