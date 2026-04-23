# Render Target Lab

`examples/render_target_lab` 是 render target 和 pass API 的覆盖型范例。它创建离屏 render target，resize 后通过 pass 清屏和绘制，再 readback 左上角像素验证颜色，最后把 render target texture 绘制回窗口。

## 覆盖 API

- `xgeRenderTargetWindow`
- `xgeRenderTargetCreate`
- `xgeRenderTargetResize`
- `xgeRenderTargetReadPixels`
- `xgeRenderTargetTexture`
- `xgeRenderTargetFree`
- `xgePassInit`
- `xgePassBegin`
- `xgePassEnd`

## 构建和运行

```bat
examples\render_target_lab\build.bat
build\xge_render_target_lab.exe --frames 120
```

## 验证点

- 控制台输出 `render-target-lab init`。
- `window=1 create=1 resize=1 texture=1 pass=1 readback=1` 表示所有路径通过。
- `rgba=32,64,96,255` 附近表示 readback 读到 pass clear color。
- 窗口中间能看到离屏 target 被绘制回窗口。
