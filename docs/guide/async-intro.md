# 异步资源加载入门

> 状态：中文初稿已生成，待审阅。

异步加载用于减少主循环卡顿。XGE 第一版的 Async API 面向资源加载，不是通用任务系统。

## 适合异步加载的资源

| 资源 | API |
| --- | --- |
| 图片 | `xgeAsyncImageLoad` |
| 纹理 | `xgeAsyncTextureLoad` |
| 字体 | `xgeAsyncFontLoad` |
| 音效 | `xgeAsyncSoundLoad` |

大型关卡、微端资源、远程下载后的本地缓存，都适合用异步加载配合 fallback。

## 最小调用顺序

```text
xgeAsyncRequestInit
  -> xgeAsyncTextureLoad
  -> 每帧 xgeAsyncPoll
  -> 使用资源
  -> xgeAsyncRequestFree
```

## 异步加载纹理

```c
static void OnTextureReady(xge_async_request req, void* user)
{
	xge_texture tex = (xge_texture)user;
	if ( req->iStatus == XGE_ASYNC_READY && req->iResult == XGE_OK ) {
		/* tex ready */
	}
}

xge_async_request_t req;
xge_texture_t tex;
memset(&req, 0, sizeof(req));
memset(&tex, 0, sizeof(tex));

xgeAsyncRequestInit(&req);
xgeAsyncTextureLoad(&req, &tex, "assets/player.png", XGE_IMAGE_PREMULTIPLIED, OnTextureReady, &tex);
```

每帧轮询：

```c
xgeAsyncPoll(&req);
```

结束时：

```c
xgeAsyncRequestFree(&req);
xgeTextureFree(&tex);
```

## Fallback 资源

异步加载期间，建议给用户一个可见 fallback。

```c
uint32_t color = xgeColorRGBA(255, 0, 255, 255);
xgeTextureFallbackSetRGBA(1, 1, &color);
xgeTextureFallbackGet(&tex);
```

真实纹理完成后再替换。

## GPU 上传边界

后台线程可以做：

- 文件读取。
- 图片解码。
- 字体数据准备。
- 音频数据准备。

后台线程不应该直接做：

- OpenGL 纹理创建。
- buffer 上传。
- 依赖当前 GL context 的操作。

这些操作应该由拥有 GL context 的线程执行，或通过 XGE 的上传队列/render thread。

## 取消请求

```c
xgeAsyncRequestCancel(&req);
```

取消不是立即停止。安全做法是继续 poll 到终态，再释放 request 和目标资源。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 完成回调里资源已失效 | 目标资源生命周期太短 | request 完成前保持目标对象有效。 |
| 后台线程 GL error | 后台线程没有 GL context | 只在 render thread 或主渲染线程上传 GPU 资源。 |
| 取消后崩溃 | 取消后立即释放 request | 继续 poll 到终态再释放。 |

## 下一步

- 查完整函数看 [Async API](../api/async.md)。
- 纹理加载细节看 [资源与纹理入门](texture-intro.md)。

[返回教程入口](README.md)
