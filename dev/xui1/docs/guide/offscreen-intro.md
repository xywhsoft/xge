# 离屏渲染入门

本教程介绍 XGE 的离屏渲染路径，包括无窗口 EGL、RenderTarget 和像素读回。

[返回教程索引](README.md) | [Offscreen API](../api/offscreen.md) | [离屏范例](../case/offscreen-egl.md)

## 适用场景

离屏渲染适合：

- 服务端或工具进程生成图片。
- 板卡 Linux 无窗口环境。
- 自动化截图和渲染回归测试。
- 把 XGE 作为嵌入式渲染模块。

## 初始化 EGL

```c
xge_egl_context_t egl;
xge_egl_desc_t desc;

memset(&desc, 0, sizeof(desc));
desc.iWidth = 512;
desc.iHeight = 512;
desc.iFlags = XGE_EGL_PBUFFER;

if ( xgeEGLInit(&egl, &desc) != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}

xgeEGLMakeCurrent(&egl);
```

桌面窗口路径通常不需要直接调用 EGL API；离屏、板卡和嵌入式环境才需要。

## 创建离屏目标

```c
xge_offscreen_t offscreen;

if ( xgeOffscreenInit(&offscreen, 512, 512) != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}
```

`xgeOffscreenRenderTarget` 可以把 offscreen 包装成 RenderTarget，之后按普通 pass 绘制。

## 读回像素

```c
uint32_t pixels[512 * 512];

if ( xgeOffscreenReadPixels(&offscreen, pixels, 512 * 4) == XGE_OK ) {
	/* encode png or compare pixels */
}
```

像素读回会造成 GPU/CPU 同步，不应作为每帧常规路径。

## 清理

```c
xgeOffscreenUnit(&offscreen);
xgeEGLUnit(&egl);
```

## 常见错误

不要在没有 current context 的线程里创建 GPU 资源。离屏模式也需要明确 GL context 所属线程。

不要把离屏读回当成 UI 截图的实时路径。需要实时显示时，应直接绘制 RenderTarget 纹理。

## 下一步

- 看完整例子用 [离屏与板卡 EGL 范例](../case/offscreen-egl.md)。
- 需要窗口内离屏合成时读 [RenderTarget 入门](render-target-intro.md)。
