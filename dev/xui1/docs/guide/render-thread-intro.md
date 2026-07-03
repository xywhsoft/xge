# 专用 Render Thread 入门

本教程说明 XGE 中拥有 GL context 的专用 render thread 的使用边界。

[返回教程索引](README.md) | [Graphics API](../api/graphics.md) | [平台教程](platform-intro.md)

## 设计目标

OpenGL/GLES/WebGL 的 GPU 操作必须在拥有当前 context 的线程执行。XGE 的专用 render thread 用于集中执行这些操作，让资源加载和业务线程只提交请求。

```text
main / game thread
  -> submit draw and resource commands
render thread
  -> owns GL context
  -> drains commands
  -> uploads GPU resources
  -> presents
```

## 查询能力

```c
xge_render_thread_caps_t caps;

if ( xgeRenderThreadCapsGet(&caps) == XGE_OK && caps.bSupported ) {
	/* render thread is available in this build */
}
```

需要同时关注：

- `bSupported`：当前构建是否支持。
- `bCanUseWithCurrentContext`：当前平台 context 是否允许迁移或独占。
- `bGLContextOwned`：启用后是否由 render thread 拥有 context。

## EGL 场景配置

板卡 Linux、离屏和部分嵌入式环境可以在启用前配置 EGL 描述：

```c
xge_egl_desc_t egl_desc;

memset(&egl_desc, 0, sizeof(egl_desc));
egl_desc.iWidth = 1280;
egl_desc.iHeight = 720;
egl_desc.iFlags = XGE_EGL_PBUFFER;

xgeRenderThreadEGLSet(&egl_desc);
```

## 启用

```c
if ( xgeRenderThreadSet(1) != XGE_OK ) {
	/* fallback to main-thread rendering */
}
```

启用应尽早发生，避免已经创建的 GL 资源需要跨线程迁移。

## 资源上传

后台线程可以加载文件和解码图片，但不能直接调用 GL 上传。推荐路径：

```c
xgeTextureUploadQueue(&texture);
/* render thread drains queue */
xgeTextureUploadFlush();
```

`xgeTextureUploadFlush` 应在拥有 context 的线程执行。启用专用 render thread 后，由渲染线程负责处理队列。

## 常见错误

不要在多个线程同时调用底层 GL。所有 GPU 命令必须进入 XGE 渲染队列或在 render thread 执行。

不要在初始化后频繁开关 render thread。这个能力是 runtime 级策略，不是每帧状态。

不要假设所有平台都支持。Web、小程序和部分移动平台的线程模型会限制 GL context 迁移。

## 下一步

- 资源异步加载读 [异步资源加载入门](async-intro.md)。
- 离屏/板卡场景读 [离屏渲染入门](offscreen-intro.md)。
