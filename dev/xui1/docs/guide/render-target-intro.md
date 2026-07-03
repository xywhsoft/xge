# RenderTarget 与离屏渲染入门

> 状态：中文初稿已生成，待审阅。

RenderTarget 让你把画面先画到一张纹理，再把这张纹理画回窗口或读回 CPU。它常用于后处理、动态贴图、小地图、UI 缓存、截图和自动化测试。

## 先区分 3 个概念

| 概念 | 解决什么问题 |
| --- | --- |
| RenderTarget | 绘制输出到哪里。 |
| RenderPass | 什么时候切换到某个 RenderTarget，并保存/恢复状态。 |
| EGL Offscreen | 在没有普通窗口时如何创建 GL/GLES context。 |

普通后处理只需要 RenderTarget/Pass。无窗口渲染才需要 Offscreen/EGL。

## 创建纹理 RenderTarget

```c
xge_render_target_t target;
xge_pass_t pass;
memset(&target, 0, sizeof(target));
memset(&pass, 0, sizeof(pass));

if ( xgeRenderTargetCreate(&target, 512, 512) != XGE_OK ) {
	return 1;
}

xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, xgeColorRGBA(0, 0, 0, 0));
```

## 绘制到 RenderTarget

```c
xgePassBegin(&pass);
xgeShapeCircleFill(256.0f, 256.0f, 120.0f, xgeColorRGBA(255, 128, 64, 255));
xgePassEnd(&pass);
```

`xgePassBegin` 和 `xgePassEnd` 必须配对。Pass 会切换 framebuffer，并在结束时恢复之前的状态。

## 把 RenderTarget 画回窗口

```c
xge_texture rt_tex = xgeRenderTargetTexture(&target);
xgeDraw(rt_tex, 40.0f, 40.0f);
```

这就是很多后处理和 UI 缓存的基础：先画到 RT，再把 RT 当普通纹理使用。

## 读回像素

```c
uint8_t pixels[512 * 512 * 4];
xgeRenderTargetReadPixels(&target, pixels, 512 * 4);
```

读回会触发 GPU 同步，不适合每帧大量调用。它适合截图、工具导出、自动化测试和离屏生成资源。

## 无窗口离屏渲染

无窗口环境需要 Offscreen/EGL。

```c
xge_offscreen_t offscreen;
memset(&offscreen, 0, sizeof(offscreen));

if ( xgeOffscreenInit(&offscreen, 512, 512) == XGE_OK ) {
	xge_render_target target = xgeOffscreenRenderTarget(&offscreen);
	/* pass draw and readback */
	xgeOffscreenUnit(&offscreen);
}
```

EGL 的 pbuffer/surfaceless 能否使用取决于目标机器驱动。板卡 Linux 要以实际验证为准。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| RT 纹理为空 | 没有 begin/end，或绘制命令提交到了窗口 | 用 `xgePassBegin` / `xgePassEnd` 包住绘制。 |
| readback 很慢 | GPU 同步 | 降低频率，只在工具和测试路径读回。 |
| 无窗口环境初始化失败 | EGL surface/context 模式不受驱动支持 | 先查 `xgeEGLCapsGet`，在 pbuffer/surfaceless 间切换。 |

## 下一步

- 查 RT/Pass 细节看 [Render Target API](../api/render-target.md)。
- 查 EGL/Offscreen 看 [Offscreen / EGL API](../api/offscreen.md)。

[返回教程入口](README.md)
