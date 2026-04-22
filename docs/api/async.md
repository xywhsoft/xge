# Async API

> Async API 负责异步加载 image、texture、font 和 sound，并把完成状态返回给主流程。

[返回 API 索引](README.md) | [异步教程](../guide/async-intro.md) | [Async 范例](../case/async-loading.md)

---

## 模块定位

Async 用于把文件读取、解码和资源准备从主循环中移出去，减少卡顿。它不是通用任务系统，第一版只覆盖引擎资源加载。

异步加载需要特别注意 GPU 资源创建：后台线程可以读文件、解码图片或准备数据，但真正依赖 GL context 的上传必须回到拥有 context 的线程，或通过 XGE 的上传队列/render thread 处理。

## 公共类型

### `xge_async_request_t`

异步请求对象。调用者持有结构体，XGE 写入类型、状态、结果、目标对象、URI、回调和线程状态。

### `xge_async_proc`

```c
typedef void (*xge_async_proc)(xge_async_request pRequest, void* pUser);
```

完成回调。回调中可以检查 `pRequest->iStatus` 和 `pRequest->iResult`。

## API 列表

- `xgeAsyncRequestInit`
- `xgeAsyncRequestFree`
- `xgeAsyncRequestCancel`
- `xgeAsyncThreadingSet`
- `xgeAsyncThreadingGet`
- `xgeAsyncPoll`
- `xgeAsyncImageLoad`
- `xgeAsyncTextureLoad`
- `xgeAsyncFontLoad`
- `xgeAsyncSoundLoad`

## 请求生命周期

### xgeAsyncRequestInit

初始化异步请求对象。

**功能：**

你可以把调用者提供的 `xge_async_request_t` 清零并设置为 pending 状态。

**函数原型：**

```c
XGE_API void xgeAsyncRequestInit(xge_async_request pRequest);
```

**参数：**

- `pRequest`：输入/输出参数，可以为 `NULL`。非 `NULL` 时被初始化。

**返回值：**

无。

**资源归属：**

请求对象由调用者持有。函数不分配长期资源。

**补充说明：**

- 新请求在调用具体 `xgeAsync*Load` 前应先初始化。

**范例代码：**

```c
xge_async_request_t req;
xgeAsyncRequestInit(&req);
```

**相关 API：**

- `xgeAsyncRequestFree`
- `xgeAsyncTextureLoad`
- `xgeAsyncPoll`

---

### xgeAsyncRequestFree

释放异步请求内部资源。

**功能：**

你可以等待并销毁请求内部线程对象，释放 URI 字符串，并把请求恢复到 pending 状态。

**函数原型：**

```c
XGE_API void xgeAsyncRequestFree(xge_async_request pRequest);
```

**参数：**

- `pRequest`：输入/输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放请求内部持有的线程句柄和 URI 内存，不释放目标资源对象。

**补充说明：**

- 如果请求仍有线程，当前实现会等待线程结束。
- 目标 image/texture/font/sound 的释放由调用者负责。

**范例代码：**

```c
xgeAsyncRequestFree(&req);
```

**相关 API：**

- `xgeAsyncRequestCancel`
- `xgeAsyncPoll`
- `xgeTextureFree`

---

### xgeAsyncRequestCancel

请求取消异步任务。

**功能：**

你可以把请求标记为 cancelled，阻止后续正常完成逻辑继续被当成 ready 使用。

**函数原型：**

```c
XGE_API int xgeAsyncRequestCancel(xge_async_request pRequest);
```

**参数：**

- `pRequest`：输入/输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- `pRequest == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 请求已经处于 ready/failed/cancelled 终态时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

不释放请求对象或目标资源。取消后仍应按生命周期调用 `xgeAsyncRequestFree`。

**补充说明：**

- 取消不等于后台线程立刻停止；需要继续 poll 或在 free 时等待。

**范例代码：**

```c
xgeAsyncRequestCancel(&req);
xgeAsyncRequestFree(&req);
```

**相关 API：**

- `xgeAsyncPoll`
- `xgeAsyncRequestFree`
- `xgeAsyncThreadingSet`

---

### xgeAsyncPoll

轮询异步请求状态。

**功能：**

你可以在每帧推进请求状态、检查线程是否完成，并派发待处理的完成回调。

**函数原型：**

```c
XGE_API int xgeAsyncPoll(xge_async_request pRequest);
```

**参数：**

- `pRequest`：输入/输出参数，不能为 `NULL`。

**返回值：**

- 返回当前 `XGE_ASYNC_*` 状态。
- `pRequest == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不转移资源所有权。函数可能销毁已结束的内部线程句柄并派发回调。

**补充说明：**

- 如果线程仍在运行，返回 `XGE_ASYNC_LOADING`。
- `onComplete` 在 poll 发现 `bCallbackPending` 时调用。

**范例代码：**

```c
int status = xgeAsyncPoll(&req);
if (status == XGE_ASYNC_READY) {
	/* use resource */
}
```

**相关 API：**

- `xgeAsyncRequestInit`
- `xgeAsyncRequestFree`
- `xgeAsyncTextureLoad`

---

## 线程控制

### xgeAsyncThreadingSet

开关异步后台线程。

**功能：**

你可以控制 Async API 是否尝试创建后台线程。关闭后仍可用统一请求状态机执行同步加载。

**函数原型：**

```c
XGE_API int xgeAsyncThreadingSet(int bEnabled);
```

**参数：**

- `bEnabled`：输入参数，`0` 表示关闭，非 `0` 表示开启。

**返回值：**

- 返回 `XGE_OK`。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- Web、小程序和部分嵌入环境可关闭线程模式。

**范例代码：**

```c
xgeAsyncThreadingSet(0);
```

**相关 API：**

- `xgeAsyncThreadingGet`
- `xgeAsyncImageLoad`
- `xgeAsyncPoll`

---

### xgeAsyncThreadingGet

查询异步后台线程开关。

**功能：**

你可以读取当前 Async 是否启用线程模式。

**函数原型：**

```c
XGE_API int xgeAsyncThreadingGet(void);
```

**参数：**

无。

**返回值：**

- 返回 `0` 表示关闭。
- 返回非 `0` 表示开启。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 该函数只读运行时开关，不代表平台一定能成功创建线程。

**范例代码：**

```c
if (xgeAsyncThreadingGet()) {
	/* threaded mode */
}
```

**相关 API：**

- `xgeAsyncThreadingSet`
- `xgePlatformCapsGet`
- `xgeAsyncPoll`

---

## 异步资源加载

### xgeAsyncImageLoad

异步加载 image。

**功能：**

你可以在请求中加载 CPU image，并在完成后使用 `xge_image_t`。

**函数原型：**

```c
XGE_API int xgeAsyncImageLoad(xge_async_request pRequest, xge_image pImage, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
```

**参数：**

- `pRequest`：输入/输出参数，不能为 `NULL`。
- `pImage`：输出目标，不能为 `NULL`。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。
- `iFlags`：输入参数，`XGE_IMAGE_*` 标志。
- `onComplete`：完成回调，可以为 `NULL`。
- `pUser`：用户数据，原样透传给回调。

**返回值：**

- 成功启动或同步完成返回 `XGE_OK`。
- 参数非法、URI 复制失败、线程启动失败或加载失败时返回对应错误码。

**资源归属：**

目标 image 由调用者持有，成功后调用者用 `xgeImageFree` 释放。XGE 不接管 `pUser`。

**补充说明：**

- 线程开启时，后台执行加载；否则当前调用同步完成并设置请求状态。

**范例代码：**

```c
xgeAsyncImageLoad(&req, &image, "assets/player.png", XGE_IMAGE_PREMULTIPLIED, OnDone, NULL);
```

**相关 API：**

- `xgeImageLoadEx`
- `xgeAsyncPoll`
- `xgeAsyncRequestFree`

---

### xgeAsyncTextureLoad

异步加载 texture。

**功能：**

你可以异步读取和解码图片，并准备纹理对象。涉及 GL context 的上传应通过 XGE 上传队列或 render thread 完成。

**函数原型：**

```c
XGE_API int xgeAsyncTextureLoad(xge_async_request pRequest, xge_texture pTexture, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
```

**参数：**

- `pRequest`：输入/输出参数，不能为 `NULL`。
- `pTexture`：输出目标，不能为 `NULL`。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。
- `iFlags`：输入参数，`XGE_IMAGE_*` 标志。
- `onComplete`：完成回调，可以为 `NULL`。
- `pUser`：用户数据，原样透传给回调。

**返回值：**

- 成功启动或同步完成返回 `XGE_OK`。
- 参数、线程、资源或纹理创建失败时返回对应错误码。

**资源归属：**

目标 texture 由调用者持有，成功后调用者用 `xgeTextureFree` 释放。

**补充说明：**

- 后台线程不应直接操作没有 context 的 GL。若实现创建 CPU shadow 后排队上传，最终上传在 render owner 线程完成。

**范例代码：**

```c
xgeAsyncTextureLoad(&req, &tex, "assets/player.png", XGE_IMAGE_PREMULTIPLIED, OnDone, NULL);
```

**相关 API：**

- `xgeTextureLoadEx`
- `xgeTextureUploadQueue`
- `xgeAsyncPoll`

---

### xgeAsyncFontLoad

异步加载 font。

**功能：**

你可以把字体文件加载和初始化移出主帧流程。

**函数原型：**

```c
XGE_API int xgeAsyncFontLoad(xge_async_request pRequest, xge_font pFont, const char* sPath, float fSize, xge_async_proc onComplete, void* pUser);
```

**参数：**

- `pRequest`：输入/输出参数，不能为 `NULL`。
- `pFont`：输出目标，不能为 `NULL`。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。
- `fSize`：输入参数，字号，单位与 Font API 一致。
- `onComplete`：完成回调，可以为 `NULL`。
- `pUser`：用户数据，原样透传给回调。

**返回值：**

- 成功启动或同步完成返回 `XGE_OK`。
- 参数、线程、资源或字体加载失败时返回对应错误码。

**资源归属：**

目标 font 由调用者持有，成功后调用者用 `xgeFontFree` 释放。

**补充说明：**

- 字体 atlas 或 GPU 缓存如果依赖 GL，应回到 render owner 线程处理。

**范例代码：**

```c
xgeAsyncFontLoad(&req, &font, "assets/ui.ttf", 18.0f, OnDone, NULL);
```

**相关 API：**

- `xgeFontLoad`
- `xgeFontFree`
- `xgeAsyncPoll`

---

### xgeAsyncSoundLoad

异步加载 sound。

**功能：**

你可以异步加载短音效，避免音频解码阻塞主循环。

**函数原型：**

```c
XGE_API int xgeAsyncSoundLoad(xge_async_request pRequest, xge_sound pSound, const char* sPath, xge_async_proc onComplete, void* pUser);
```

**参数：**

- `pRequest`：输入/输出参数，不能为 `NULL`。
- `pSound`：输出目标，不能为 `NULL`。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。
- `onComplete`：完成回调，可以为 `NULL`。
- `pUser`：用户数据，原样透传给回调。

**返回值：**

- 成功启动或同步完成返回 `XGE_OK`。
- 参数、线程、资源或音效加载失败时返回对应错误码。

**资源归属：**

目标 sound 由调用者持有，成功后调用者用 `xgeSoundFree` 释放。

**补充说明：**

- 适合短音效。长音乐或流式音频优先使用 Music/Stream API。

**范例代码：**

```c
xgeAsyncSoundLoad(&req, &sound, "assets/click.wav", OnDone, NULL);
```

**相关 API：**

- `xgeSoundLoad`
- `xgeSoundFree`
- `xgeAsyncPoll`

---

## 生命周期与所有权

`xge_async_request_t` 由调用者持有，使用 `xgeAsyncRequestInit` 初始化，结束后调用 `xgeAsyncRequestFree`。取消请求后，目标资源对象仍由调用者负责释放。

异步加载目标对象例如 texture、font、sound 必须在请求完成前保持有效。如果对象可能提前销毁，应先取消请求并处理回调竞态。

## 线程约束

异步线程只负责 IO、解码和 CPU 侧准备。GPU 上传、纹理创建和 atlas 更新必须排队到 render thread 或当前 context owner。

回调由 `xgeAsyncPoll` 派发时通常在调用 poll 的线程执行。业务代码仍建议只记录状态，再由主流程消费。

## 后端差异

不支持线程的平台会降级到轮询或同步路径。Web、小程序和部分移动平台的资源加载由宿主控制，异步语义可能由 provider 或桥接层实现。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 异步完成后资源对象已失效 | 目标资源在栈上提前返回或被释放 | 目标资源生命周期必须长于 request。 |
| 后台加载导致 GL 错误 | 后台线程直接做 GPU 上传 | 使用上传队列或 render thread。 |
| 取消后立即 free 卡住 | 任务尚未进入终态，free 会等待线程 | 取消后继续 poll 或接受 free 等待成本。 |

## 相关示例

- `examples/async`
- `build_async_exe.bat`
