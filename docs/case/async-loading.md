# 异步资源加载

本案例展示在运行时异步加载图片、纹理、字体和音频，并在完成前使用 fallback 资源。

[返回范例解析](README.md) | [异步教程](../guide/async-intro.md) | [Async API](../api/async.md)

## 问题

微端、关卡切换和大型 UI 页面不能把所有资源都同步加载到主线程。XGE 的异步接口允许后台执行 IO/解码，并把 GPU 上传排队到拥有 GL context 的渲染线程。

## 步骤 1：开启异步线程

```c
xgeAsyncThreadingSet(1);
```

在不支持线程的平台上，接口可降级为 poll 驱动，但业务层不应该依赖立即完成。

## 步骤 2：设置 fallback

```c
uint32_t pixel = xgeColorRGBA(255, 0, 255, 255);

xgeTextureFallbackSetRGBA(1, 1, &pixel);
```

加载未完成时可以先使用 fallback，避免画面出现空白。

## 步骤 3：提交加载请求

```c
static xge_async_request_t g_req;
static xge_texture_t g_tex;

static int OnTextureLoaded(xge_async_request pRequest, int iResult, void* pUser)
{
	(void)pRequest;
	(void)pUser;

	if ( iResult != XGE_OK ) {
		xgeLogWrite(XGE_LOG_WARN, "async", "texture load failed");
	}
	return 0;
}

xgeAsyncRequestInit(&g_req);
xgeAsyncTextureLoad(&g_req, &g_tex, "assets/large.png", 0, OnTextureLoaded, NULL);
```

## 步骤 4：轮询状态

```c
int status;

status = xgeAsyncPoll(&g_req);
if ( status == XGE_ASYNC_DONE ) {
	xgeDraw(&g_tex, 40.0f, 40.0f);
}
```

完成回调适合记录状态，实际绘制仍应由主逻辑根据资源状态决定。

## 取消请求

```c
xgeAsyncRequestCancel(&g_req);
xgeAsyncRequestFree(&g_req);
```

取消不是强制终止所有平台线程，而是要求系统尽快停止后续处理。业务层仍需处理回调已经发生的情况。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeAsyncThreadingSet` | 开关异步线程 |
| `xgeAsyncRequestInit` | 初始化请求 |
| `xgeAsyncTextureLoad` | 异步加载纹理 |
| `xgeAsyncFontLoad` | 异步加载字体 |
| `xgeAsyncSoundLoad` | 异步加载音效 |
| `xgeAsyncPoll` | 查询请求状态 |
| `xgeAsyncRequestCancel` | 取消请求 |

## 常见问题

如果异步加载完成但纹理仍为空，检查 `xgeTextureUploadFlush` 是否在渲染线程执行，或者专用 render thread 是否已启用。

如果退出时崩溃，确认先取消未完成请求，再释放目标资源对象。
