# Graphics API

> Graphics API 负责帧提交、手动刷新、渲染线程、脏矩形、颜色、blend/depth、camera、viewport 和 clip。

[返回 API 索引](README.md) | [渲染教程](../guide/drawing-intro.md) | [RenderTarget 教程](../guide/render-target-intro.md)

---

## 模块定位

Graphics 是绘制命令之外的渲染控制层。它不直接加载纹理，也不直接管理 shader/material，而是控制一次渲染提交如何开始、结束、刷新，以及全局图形状态如何影响 Drawing、Text、Shape、Material 等模块。

普通游戏多数情况下只需要在帧回调里调用 `xgeClear` 和绘制 API。工具、编辑器、GUI APP、小程序宿主、离屏渲染和专用 render thread 场景会更多使用 `xgeBegin`、`xgeEnd`、`xgeFlush`、dirty rect 和 runtime/caps 查询。

## 标准调用顺序

默认游戏循环：

```text
xgeRun
  -> frame callback
    -> xgeClear
    -> xgeDraw / xgeShape* / xgeTextDraw
```

手动刷新或 APP 模式：

```text
xgeBegin
  -> xgeClear 或 xgeInvalidateRect
  -> 提交绘制命令
xgeEnd
```

`xgeEnd` 当前会调用 `xgeFlush`。需要只刷新命令队列时，也可以直接调用 `xgeFlush`。

## 常量与宏

| 名称 | 值 | 说明 |
| --- | --- | --- |
| `XGE_BLEND_NONE` | `0` | 关闭 blend。 |
| `XGE_BLEND_ALPHA` | `1` | 默认 alpha blend，按 premultiplied alpha 设计。 |
| `XGE_BLEND_ADD` | `2` | 加法 blend，适合光效。 |
| `XGE_BLEND_MULTIPLY` | `3` | 乘法 blend。 |
| `XGE_BLEND_SCREEN` | `4` | screen blend。 |
| `XGE_BLEND_CUSTOM` | `5` | 自定义 blend，后续扩展。 |
| `XGE_COORD_Y_DOWN` | `0` | 默认 y 轴向下。 |
| `XGE_COORD_CENTER` | `1` | 以 viewport 中心作为坐标原点。 |
| `XGE_CAMERA_ORTHO` | `0` | 正交投影。 |
| `XGE_CAMERA_PERSPECTIVE` | `1` | 透视投影，用于 2.5D。 |
| `XGE_DIRTY_RECT_MAX` | `64` | 脏矩形最大数量。 |

## 公共类型

### `xge_rect_t`

矩形结构，字段为 `fX`、`fY`、`fW`、`fH`，单位由调用 API 决定。Graphics API 中通常表示逻辑像素坐标。

### `xge_vec2_t`

二维向量结构，字段为 `fX`、`fY`。

### `xge_color_t`

归一化颜色结构，字段为 `fR`、`fG`、`fB`、`fA`，通常范围为 `0.0f` 到 `1.0f`。

### `xge_camera_t`

Camera 状态结构，包含位置、缩放、旋转、viewport、坐标模式、投影模式和 2.5D 透视参数。

### `xge_render_thread_caps_t`

专用渲染线程能力快照，包含是否支持、是否启用、是否由 worker 消费命令、是否拥有 GL context、是否支持异步 flush。

## API 列表

- `xgeBegin`
- `xgeEnd`
- `xgeFlush`
- `xgeRenderThreadCapsGet`
- `xgeRenderThreadEGLSet`
- `xgeRenderThreadSet`
- `xgeRenderThreadGet`
- `xgeClear`
- `xgePresent`
- `xgeInvalidateRect`
- `xgeDirtyRectCount`
- `xgeDirtyRectGet`
- `xgeDirtyRectClear`
- `xgeColorRGBA`
- `xgeColorUnpack`
- `xgeBlendSet`
- `xgeBlendGet`
- `xgeDepthTestSet`
- `xgeDepthTestGet`
- `xgeCameraDefault`
- `xgeCameraPerspective`
- `xgeCameraSet`
- `xgeCameraGet`
- `xgeWorldToScreen`
- `xgeScreenToWorld`
- `xgeViewportSet`
- `xgeViewportGet`
- `xgeViewportClear`
- `xgeClipSet`
- `xgeClipGet`
- `xgeClipClear`

## 帧提交

### xgeBegin

开始一次显式渲染提交。

**功能：**

你可以在 APP 模式、手动刷新、离屏渲染或宿主驱动的刷新流程中，用它明确打开一次绘制批次。

**函数原型：**

```c
XGE_API int xgeBegin(void);
```

**参数：**

无。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 已经处于渲染提交中时返回 `XGE_ERROR_ALREADY_INITIALIZED`。

**资源归属：**

不创建资源，不转移所有权。函数只修改当前 XGE runtime 的渲染活动状态。

**补充说明：**

- `xgeBegin` 应与 `xgeEnd` 配对。
- 默认 `xgeRun` 流程通常不需要用户手动调用。

**范例代码：**

```c
if (xgeBegin() == XGE_OK) {
	xgeClear(xgeColorRGBA(20, 24, 32, 255));
	xgeEnd();
}
```

**相关 API：**

- `xgeEnd`
- `xgeFlush`
- `xgeRun`

---

### xgeEnd

结束一次显式渲染提交。

**功能：**

你可以在手动刷新流程中用它关闭绘制批次，并把当前命令提交到后端或渲染线程。

**函数原型：**

```c
XGE_API int xgeEnd(void);
```

**参数：**

无。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 当前没有处于 `xgeBegin` 打开的渲染提交中时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内部 `xgeFlush` 失败时返回对应错误码。

**资源归属：**

不创建资源，不转移所有权。函数只修改当前 runtime 的渲染活动状态。

**补充说明：**

- 当前实现会调用 `xgeFlush`。
- 无论 `xgeFlush` 是否成功，当前实现都会关闭渲染活动状态。

**范例代码：**

```c
int ret = xgeBegin();
if (ret == XGE_OK) {
	xgeClear(xgeColorRGBA(0, 0, 0, 255));
	ret = xgeEnd();
}
```

**相关 API：**

- `xgeBegin`
- `xgeFlush`
- `xgePresent`

---

### xgeFlush

刷新当前渲染命令队列。

**功能：**

你可以把已入队的纹理上传和绘制命令提交给当前图形后端。启用专用 render thread 且当前不是 Sokol 窗口运行路径时，命令会走线程化 flush。

**函数原型：**

```c
XGE_API int xgeFlush(void);
```

**参数：**

无。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 纹理上传或命令队列提交失败时返回对应错误码。

**资源归属：**

不转移资源所有权。函数会消费当前已排队的 GPU 上传和绘制命令。

**补充说明：**

- Sokol 窗口运行中，如果 GL 函数可用，当前实现会调用 `glFlush`。
- `xgeEnd` 会自动调用该函数。

**范例代码：**

```c
xgeClear(xgeColorRGBA(12, 16, 24, 255));
if (xgeFlush() != XGE_OK) {
	return 1;
}
```

**相关 API：**

- `xgeBegin`
- `xgeEnd`
- `xgeTextureUploadFlush`

---

### xgeClear

清空当前 render target。

**功能：**

你可以用 RGBA8 颜色清空当前渲染目标，并把整个窗口标记为脏区。

**函数原型：**

```c
XGE_API void xgeClear(uint32_t iColor);
```

**参数：**

- `iColor`：输入参数，RGBA8 打包颜色，通常使用 `xgeColorRGBA` 生成。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 当前实现会记录清屏颜色，并对窗口全区域调用 `xgeInvalidateRect`。
- Sokol 运行路径下会立即调用 GL 清屏。

**范例代码：**

```c
xgeClear(xgeColorRGBA(20, 24, 32, 255));
```

**相关 API：**

- `xgeColorRGBA`
- `xgeInvalidateRect`
- `xgeRenderTargetWindow`

---

### xgePresent

提交当前帧并清空脏矩形。

**功能：**

你可以在宿主驱动的模式中显式请求结束当前显示提交。当前实现会在已初始化时刷新命令，并清空 dirty rect 列表。

**函数原型：**

```c
XGE_API void xgePresent(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 当前实现不会直接调用平台后端 `present` 回调；实际 swap/present 仍由平台后端和运行循环负责。
- 调用后会执行 `xgeDirtyRectClear`。

**范例代码：**

```c
xgeClear(xgeColorRGBA(0, 0, 0, 255));
xgePresent();
```

**相关 API：**

- `xgeFlush`
- `xgeDirtyRectClear`
- `xgePlatformBackendSet`

---

## 专用渲染线程

### xgeRenderThreadCapsGet

查询专用渲染线程能力。

**功能：**

你可以在启用 render thread 前判断当前 runtime 是否支持线程化命令消费，以及是否能与当前 GL context 组合使用。

**函数原型：**

```c
XGE_API int xgeRenderThreadCapsGet(xge_render_thread_caps_t* pCaps);
```

**参数：**

- `pCaps`：输出参数，不能为 `NULL`。函数会先清零结构体，再写入能力快照。

**返回值：**

- 成功返回 `XGE_OK`。
- `pCaps == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用者持有 `pCaps` 指向的结构体。XGE 不保存该指针。

**补充说明：**

- 当前实现标记 `bSupported = 1`、`bWorkerDrain = 1`。
- Sokol 正在运行时，`bCanUseWithCurrentContext` 当前为 `0`，表示不建议与当前窗口 GL context 组合使用。

**范例代码：**

```c
xge_render_thread_caps_t caps;
if (xgeRenderThreadCapsGet(&caps) == XGE_OK && caps.bCanUseWithCurrentContext) {
	xgeRenderThreadSet(1);
}
```

**相关 API：**

- `xgeRenderThreadSet`
- `xgeRenderThreadGet`
- `xgeRenderThreadEGLSet`

---

### xgeRenderThreadEGLSet

配置专用渲染线程使用的 EGL context 描述。

**功能：**

你可以为离屏渲染、板卡 Linux 或无窗口环境配置由 render thread 拥有的 EGL context。

**函数原型：**

```c
XGE_API int xgeRenderThreadEGLSet(const xge_egl_desc_t* pDesc);
```

**参数：**

- `pDesc`：输入参数。为 `NULL` 时清除 render thread EGL 配置；非 `NULL` 时必须提供有效尺寸和 surface 策略。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- render thread 已启用时返回 `XGE_ERROR_ALREADY_INITIALIZED`。
- 描述无效时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 当前平台或 EGL 能力不支持该描述时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

XGE 会复制 `xge_egl_desc_t` 结构体。调用者可以在函数返回后释放或复用传入结构体。描述中的宿主指针所有权仍属于调用者。

**补充说明：**

- 配置成功后，当前实现会把平台后端切换为 EGL，图形后端切换为 GLES3。
- Sokol 窗口运行中不支持该配置。

**范例代码：**

```c
xge_egl_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iWidth = 640;
desc.iHeight = 480;
desc.bPBuffer = 1;
xgeRenderThreadEGLSet(&desc);
```

**相关 API：**

- `xgeEGLCapsGet`
- `xgeRenderThreadSet`
- `xgeOffscreenInit`

---

### xgeRenderThreadSet

启用或关闭专用渲染线程。

**功能：**

你可以让渲染命令由专门线程消费，使拥有 GL context 的线程集中执行 GPU 操作。

**函数原型：**

```c
XGE_API int xgeRenderThreadSet(int bEnabled);
```

**参数：**

- `bEnabled`：输入参数。`0` 表示关闭，非 `0` 表示启用。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 当前上下文组合不支持启用时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

不转移用户资源所有权。关闭时会等待并回收内部 render thread 状态。

**补充说明：**

- 不建议在渲染进行中频繁切换。
- 启用前应先调用 `xgeRenderThreadCapsGet`。

**范例代码：**

```c
if (xgeRenderThreadSet(1) != XGE_OK) {
	xgeRenderThreadSet(0);
}
```

**相关 API：**

- `xgeRenderThreadCapsGet`
- `xgeRenderThreadGet`
- `xgeFlush`

---

### xgeRenderThreadGet

查询专用渲染线程是否启用。

**功能：**

你可以在日志、调试面板或运行时策略中读取当前 render thread 开关状态。

**函数原型：**

```c
XGE_API int xgeRenderThreadGet(void);
```

**参数：**

无。

**返回值：**

- 返回 `0` 表示未启用。
- 返回非 `0` 表示已启用。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 该函数只读当前状态，不检查平台是否支持。

**范例代码：**

```c
if (xgeRenderThreadGet()) {
	xgeLogWrite(XGE_LOG_INFO, "render", "render thread enabled");
}
```

**相关 API：**

- `xgeRenderThreadSet`
- `xgeRenderThreadCapsGet`
- `xgeDebugDumpCaps`

---

## 脏矩形

### xgeInvalidateRect

标记需要刷新的矩形区域。

**功能：**

你可以在 GUI/APP 模式中标记局部刷新区域，减少无意义重绘。

**函数原型：**

```c
XGE_API void xgeInvalidateRect(xge_rect_t tRect);
```

**参数：**

- `tRect`：输入参数，逻辑窗口坐标矩形。宽高小于等于 `0` 的区域会被忽略。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。函数会把矩形复制进内部 dirty rect 列表。

**补充说明：**

- XGE 未初始化时调用会被忽略。
- 矩形会被裁剪到窗口范围。
- dirty rect 超过 `XGE_DIRTY_RECT_MAX` 后，当前实现会合并为一个联合区域。

**范例代码：**

```c
xge_rect_t rect = { 0.0f, 0.0f, 320.0f, 200.0f };
xgeInvalidateRect(rect);
```

**相关 API：**

- `xgeDirtyRectCount`
- `xgeDirtyRectGet`
- `xgeDirtyRectClear`

---

### xgeDirtyRectCount

获取当前 dirty rect 数量。

**功能：**

你可以在 APP 模式或测试代码中判断当前是否存在待刷新的区域。

**函数原型：**

```c
XGE_API int xgeDirtyRectCount(void);
```

**参数：**

无。

**返回值：**

- 返回当前 dirty rect 数量。
- XGE 尚未初始化时返回 `0`。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 返回值最大不超过 `XGE_DIRTY_RECT_MAX`，溢出时实现会合并区域。

**范例代码：**

```c
if (xgeDirtyRectCount() > 0) {
	xgeFlush();
}
```

**相关 API：**

- `xgeInvalidateRect`
- `xgeDirtyRectGet`
- `xgeDirtyRectClear`

---

### xgeDirtyRectGet

读取指定 dirty rect。

**功能：**

你可以遍历 dirty rect 列表，用于 GUI 局部绘制、调试显示或冒烟测试。

**函数原型：**

```c
XGE_API int xgeDirtyRectGet(int iIndex, xge_rect_t* pRect);
```

**参数：**

- `iIndex`：输入参数，dirty rect 索引，从 `0` 开始。
- `pRect`：输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化、`pRect == NULL`、索引小于 `0` 或索引越界时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用者持有 `pRect` 指向的结构体。XGE 不保存该指针。

**补充说明：**

- 返回的是 dirty rect 快照副本。

**范例代码：**

```c
int i;
for (i = 0; i < xgeDirtyRectCount(); i++) {
	xge_rect_t rect;
	if (xgeDirtyRectGet(i, &rect) == XGE_OK) {
		/* redraw rect */
	}
}
```

**相关 API：**

- `xgeDirtyRectCount`
- `xgeInvalidateRect`
- `xgeDirtyRectClear`

---

### xgeDirtyRectClear

清空 dirty rect 列表。

**功能：**

你可以在局部刷新完成后清除所有脏区标记。

**函数原型：**

```c
XGE_API void xgeDirtyRectClear(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- `xgePresent` 当前会调用该函数。
- 游戏连续渲染一般不需要手动管理 dirty rect。

**范例代码：**

```c
xgeDirtyRectClear();
```

**相关 API：**

- `xgePresent`
- `xgeInvalidateRect`
- `xgeDirtyRectCount`

---

## 颜色、Blend 与 Depth

### xgeColorRGBA

打包 RGBA8 颜色。

**功能：**

你可以用 0 到 255 的 RGBA 分量生成 XGE 绘制 API 使用的颜色值。

**函数原型：**

```c
XGE_API uint32_t xgeColorRGBA(int iR, int iG, int iB, int iA);
```

**参数：**

- `iR`：输入参数，红色分量，建议范围 `0` 到 `255`。
- `iG`：输入参数，绿色分量，建议范围 `0` 到 `255`。
- `iB`：输入参数，蓝色分量，建议范围 `0` 到 `255`。
- `iA`：输入参数，alpha 分量，建议范围 `0` 到 `255`。

**返回值：**

- 返回 RGBA8 打包颜色。

**资源归属：**

返回值是普通整数，不涉及资源所有权。

**补充说明：**

- XGE 颜色约定尽量贴近 OpenGL 使用方式，默认纹理格式为 RGBA8。
- 当前宏实现不负责运行时 clamp，调用者应传入合法分量。

**范例代码：**

```c
uint32_t white = xgeColorRGBA(255, 255, 255, 255);
```

**相关 API：**

- `xgeColorUnpack`
- `xgeClear`
- `xgeMaterialSetColor`

---

### xgeColorUnpack

把 RGBA8 颜色解包为 float 分量。

**功能：**

你可以把 XGE 打包颜色转换成 shader、调试面板或自定义渲染代码更容易使用的归一化颜色。

**函数原型：**

```c
XGE_API xge_color_t xgeColorUnpack(uint32_t iColor);
```

**参数：**

- `iColor`：输入参数，RGBA8 打包颜色。

**返回值：**

- 返回 `xge_color_t`，每个分量通常在 `0.0f` 到 `1.0f` 范围。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 该函数不判断颜色是否来自 `xgeColorRGBA`。

**范例代码：**

```c
xge_color_t c = xgeColorUnpack(xgeColorRGBA(255, 128, 0, 255));
```

**相关 API：**

- `xgeColorRGBA`
- `xgeClear`
- `xgeBlendSet`

---

### xgeBlendSet

设置全局 blend 模式。

**功能：**

你可以控制后续绘制命令使用的混合方式，例如普通 alpha、加法发光、乘法阴影或 screen。

**函数原型：**

```c
XGE_API void xgeBlendSet(int iBlend);
```

**参数：**

- `iBlend`：输入参数，取 `XGE_BLEND_*`。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 默认 alpha 路径按 premultiplied alpha 设计。
- 传入未知值时当前实现仍会记录该值，实际 GL 状态应用以内部实现为准。

**范例代码：**

```c
xgeBlendSet(XGE_BLEND_ADD);
/* draw glow */
xgeBlendSet(XGE_BLEND_ALPHA);
```

**相关 API：**

- `xgeBlendGet`
- `xgeColorRGBA`
- `xgeMaterialSetBlend`

---

### xgeBlendGet

获取当前全局 blend 模式。

**功能：**

你可以在临时修改 blend 前保存当前状态，并在绘制后恢复。

**函数原型：**

```c
XGE_API int xgeBlendGet(void);
```

**参数：**

无。

**返回值：**

- 返回当前 blend 模式，通常为 `XGE_BLEND_*`。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 返回的是 XGE runtime 中记录的状态。

**范例代码：**

```c
int old_blend = xgeBlendGet();
xgeBlendSet(XGE_BLEND_MULTIPLY);
xgeBlendSet(old_blend);
```

**相关 API：**

- `xgeBlendSet`
- `xgeMaterialSetBlend`
- `xgeClear`

---

### xgeDepthTestSet

开关 depth test。

**功能：**

你可以为 2.5D 绘制启用深度测试，让带 Z 值的图元按深度参与遮挡。

**函数原型：**

```c
XGE_API void xgeDepthTestSet(int bEnabled);
```

**参数：**

- `bEnabled`：输入参数，`0` 表示关闭，非 `0` 表示开启。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 纯 2D 默认不需要开启 depth test。
- 2.5D 顶点和 camera 参数应统一设计，否则可能出现遮挡异常。

**范例代码：**

```c
xgeDepthTestSet(1);
/* draw 2.5D */
xgeDepthTestSet(0);
```

**相关 API：**

- `xgeDepthTestGet`
- `xgeDrawQuad3D`
- `xgeCameraPerspective`

---

### xgeDepthTestGet

查询 depth test 当前状态。

**功能：**

你可以在临时开启或关闭 depth test 前保存当前状态。

**函数原型：**

```c
XGE_API int xgeDepthTestGet(void);
```

**参数：**

无。

**返回值：**

- 返回 `0` 表示关闭。
- 返回非 `0` 表示开启。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 返回的是 XGE runtime 中记录的状态。

**范例代码：**

```c
int old_depth = xgeDepthTestGet();
xgeDepthTestSet(1);
xgeDepthTestSet(old_depth);
```

**相关 API：**

- `xgeDepthTestSet`
- `xgeDrawQuad3D`
- `xgeCameraPerspective`

---

## Camera 与坐标变换

### xgeCameraDefault

创建默认正交 camera。

**功能：**

你可以根据逻辑尺寸创建一个 y 轴向下、缩放为 1、viewport 覆盖指定尺寸的 2D camera。

**函数原型：**

```c
XGE_API xge_camera_t xgeCameraDefault(float fWidth, float fHeight);
```

**参数：**

- `fWidth`：输入参数，camera viewport 宽度，单位为逻辑像素。
- `fHeight`：输入参数，camera viewport 高度，单位为逻辑像素。

**返回值：**

- 返回 `xge_camera_t` 结构体副本。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 默认 `iCoordinateMode` 为 `XGE_COORD_Y_DOWN`。
- 默认 `iProjectionMode` 为 `XGE_CAMERA_ORTHO`。

**范例代码：**

```c
xge_camera_t camera = xgeCameraDefault(1280.0f, 720.0f);
xgeCameraSet(&camera);
```

**相关 API：**

- `xgeCameraSet`
- `xgeCameraGet`
- `xgeWorldToScreen`

---

### xgeCameraPerspective

创建透视 camera。

**功能：**

你可以创建用于 2.5D 绘制的透视 camera，让带 Z 值的四边形或 mesh 产生透视缩放效果。

**函数原型：**

```c
XGE_API xge_camera_t xgeCameraPerspective(float fWidth, float fHeight, float fFovY, float fNearZ, float fFarZ);
```

**参数：**

- `fWidth`：输入参数，camera viewport 宽度，单位为逻辑像素。
- `fHeight`：输入参数，camera viewport 高度，单位为逻辑像素。
- `fFovY`：输入参数，垂直视场角，单位为度。小于等于 `0` 时回退为 `60.0f`。
- `fNearZ`：输入参数，近裁剪面。小于等于 `0` 时回退为 `0.1f`。
- `fFarZ`：输入参数，远裁剪面。小于等于 `fNearZ` 时回退为 `1000.0f`。

**返回值：**

- 返回 `xge_camera_t` 结构体副本。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 当前实现会根据 `fHeight` 和 `fFovY` 计算 `fPerspectiveDistance`。
- 透视 camera 主要服务 2.5D，不代表 XGE 变成通用 3D 引擎。

**范例代码：**

```c
xge_camera_t camera = xgeCameraPerspective(1280.0f, 720.0f, 60.0f, 0.1f, 1000.0f);
xgeCameraSet(&camera);
```

**相关 API：**

- `xgeCameraSet`
- `xgeDepthTestSet`
- `xgeDrawQuad3D`

---

### xgeCameraSet

设置当前 camera。

**功能：**

你可以改变世界坐标到屏幕坐标的映射方式，包括位置、缩放、viewport、坐标模式和投影模式。

**函数原型：**

```c
XGE_API void xgeCameraSet(const xge_camera_t* pCamera);
```

**参数：**

- `pCamera`：输入参数，可以为 `NULL`。为 `NULL` 时当前实现直接忽略。

**返回值：**

无。

**资源归属：**

XGE 会复制 camera 结构体，不保存 `pCamera` 指针。

**补充说明：**

- 缩放分量为 `0` 时当前实现会修正为 `1.0f`。
- 透视 camera 的 FOV、near、far 和 perspective distance 无效时会被修正。

**范例代码：**

```c
xge_camera_t camera = xgeCameraDefault(800.0f, 600.0f);
camera.tPosition.fX = 100.0f;
xgeCameraSet(&camera);
```

**相关 API：**

- `xgeCameraDefault`
- `xgeCameraPerspective`
- `xgeCameraGet`

---

### xgeCameraGet

获取当前 camera。

**功能：**

你可以保存当前 camera，临时切换绘制坐标系后再恢复。

**函数原型：**

```c
XGE_API xge_camera_t xgeCameraGet(void);
```

**参数：**

无。

**返回值：**

- 返回当前 camera 结构体副本。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 返回的是当前 runtime 记录的 camera 状态。

**范例代码：**

```c
xge_camera_t old_camera = xgeCameraGet();
xge_camera_t ui_camera = xgeCameraDefault(800.0f, 600.0f);
xgeCameraSet(&ui_camera);
xgeCameraSet(&old_camera);
```

**相关 API：**

- `xgeCameraSet`
- `xgeWorldToScreen`
- `xgeScreenToWorld`

---

### xgeWorldToScreen

将世界坐标转换为屏幕坐标。

**功能：**

你可以把游戏世界中的点映射到当前 camera 的 viewport 空间，用于拾取、UI 标记或调试绘制。

**函数原型：**

```c
XGE_API xge_vec2_t xgeWorldToScreen(xge_vec2_t tPoint);
```

**参数：**

- `tPoint`：输入参数，世界坐标点。

**返回值：**

- 返回屏幕坐标点。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 当前实现应用 camera 位置、缩放、viewport 和坐标模式。
- 该函数不应用 2.5D Z 透视投影。

**范例代码：**

```c
xge_vec2_t world = { 10.0f, 20.0f };
xge_vec2_t screen = xgeWorldToScreen(world);
```

**相关 API：**

- `xgeScreenToWorld`
- `xgeCameraSet`
- `xgeCameraGet`

---

### xgeScreenToWorld

将屏幕坐标转换为世界坐标。

**功能：**

你可以把鼠标、触摸或 UI 输入坐标转换到当前 camera 对应的世界空间。

**函数原型：**

```c
XGE_API xge_vec2_t xgeScreenToWorld(xge_vec2_t tPoint);
```

**参数：**

- `tPoint`：输入参数，屏幕坐标点。

**返回值：**

- 返回世界坐标点。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 当前实现应用 camera viewport、坐标模式、缩放和位置。
- 如果 camera 缩放异常，结果也会异常；`xgeCameraSet` 会把 0 缩放修正为 1。

**范例代码：**

```c
xge_vec2_t mouse = { 100.0f, 120.0f };
xge_vec2_t world = xgeScreenToWorld(mouse);
```

**相关 API：**

- `xgeWorldToScreen`
- `xgeCameraSet`
- `xgeMouseGet`

---

## Viewport 与 Clip

### xgeViewportSet

设置当前 viewport。

**功能：**

你可以限制后续渲染输出区域，用于分屏、编辑器面板、离屏预览或局部绘制。

**函数原型：**

```c
XGE_API void xgeViewportSet(xge_rect_t tRect);
```

**参数：**

- `tRect`：输入参数，viewport 矩形，单位为逻辑像素。宽高小于 `0` 时会修正为 `0`。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- XGE 使用 y 轴向下的逻辑坐标；实际 GL viewport 会转换为 GL 需要的 y 轴向上坐标。
- 没有 GL context 或 GL viewport 函数不可用时，当前实现只记录状态。

**范例代码：**

```c
xge_rect_t viewport = { 0.0f, 0.0f, 640.0f, 480.0f };
xgeViewportSet(viewport);
```

**相关 API：**

- `xgeViewportGet`
- `xgeViewportClear`
- `xgeCameraSet`

---

### xgeViewportGet

获取当前 viewport。

**功能：**

你可以保存当前 viewport，临时切换绘制区域后再恢复。

**函数原型：**

```c
XGE_API xge_rect_t xgeViewportGet(void);
```

**参数：**

无。

**返回值：**

- 返回当前 viewport。
- 未显式设置 viewport 时，返回覆盖当前窗口尺寸的默认矩形。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 默认矩形宽高来自当前 XGE runtime 的窗口尺寸。

**范例代码：**

```c
xge_rect_t old_viewport = xgeViewportGet();
```

**相关 API：**

- `xgeViewportSet`
- `xgeViewportClear`
- `xgePlatformRuntimeGet`

---

### xgeViewportClear

清除显式 viewport。

**功能：**

你可以恢复默认全窗口 viewport。

**函数原型：**

```c
XGE_API void xgeViewportClear(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 当前实现会清除内部 viewport 标记，并在 GL 可用时恢复 GL viewport。

**范例代码：**

```c
xgeViewportClear();
```

**相关 API：**

- `xgeViewportSet`
- `xgeViewportGet`
- `xgeClipClear`

---

### xgeClipSet

设置裁剪矩形。

**功能：**

你可以限制后续绘制只出现在指定矩形内，用于 GUI 滚动区域、列表、面板和局部重绘。

**函数原型：**

```c
XGE_API void xgeClipSet(xge_rect_t tRect);
```

**参数：**

- `tRect`：输入参数，裁剪矩形，单位为逻辑像素。宽高小于 `0` 时会修正为 `0`。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- GL 路径下当前实现使用 scissor test。
- XGE 会把 y 轴向下坐标转换为 GL scissor 使用的坐标。

**范例代码：**

```c
xge_rect_t clip = { 10.0f, 10.0f, 200.0f, 120.0f };
xgeClipSet(clip);
```

**相关 API：**

- `xgeClipGet`
- `xgeClipClear`
- `xgeViewportSet`

---

### xgeClipGet

获取当前裁剪矩形。

**功能：**

你可以保存当前 clip，临时设置子区域裁剪后再恢复。

**函数原型：**

```c
XGE_API xge_rect_t xgeClipGet(void);
```

**参数：**

无。

**返回值：**

- 返回当前裁剪矩形。
- 未设置 clip 时返回全 0 矩形。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 当前 API 没有单独返回 clip 是否启用的函数，调用方需要结合自己的状态管理判断。

**范例代码：**

```c
xge_rect_t old_clip = xgeClipGet();
```

**相关 API：**

- `xgeClipSet`
- `xgeClipClear`
- `xgeViewportGet`

---

### xgeClipClear

清除当前裁剪矩形。

**功能：**

你可以恢复不裁剪的绘制状态。

**函数原型：**

```c
XGE_API void xgeClipClear(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- GL 路径下当前实现会关闭 scissor test。

**范例代码：**

```c
xgeClipClear();
```

**相关 API：**

- `xgeClipSet`
- `xgeClipGet`
- `xgeViewportClear`

---

## 生命周期与所有权

Graphics API 控制当前 runtime 的渲染状态，不创建长期业务对象。Camera、viewport、clip、blend、depth 和 dirty rect 状态都属于当前 XGE runtime。

所有结构体输出都是副本或写入调用者提供的结构体。除 render thread 内部状态外，本页 API 不产生需要用户释放的资源。

## 线程约束

默认窗口路径下，`xgeBegin`、`xgeEnd`、`xgeFlush`、`xgeClear`、`xgePresent` 和绘制状态修改应在渲染 owner 线程调用。启用专用 render thread 后，普通线程提交命令，实际 GL 操作由 render thread 执行。

Dirty rect、camera、viewport、clip、blend 和 depth 都是 runtime 全局状态。多线程调用时，应由上层保证同一时间只有一个逻辑写入者。

## 后端差异

OpenGL 3.3、GLES3 和 WebGL2 的状态能力接近但不完全一致。线宽、shader 版本、readback、present、scissor、viewport 和 context 线程模型都可能存在平台差异。

Web、小程序和部分移动平台不一定支持完整的专用 render thread。使用 `xgeRenderThreadCapsGet` 查询真实能力。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 手动模式下没有画面 | 没有调用 `xgeEnd` 或 `xgeFlush` | 使用完整的 begin/end/flush 顺序。 |
| GUI 局部刷新残影 | dirty rect 没有清理或 redraw 覆盖不完整 | 刷新后调用 `xgeDirtyRectClear`，并保证 dirty 区域完整重绘。 |
| 2.5D 图像深度异常 | 没开启 depth test 或顶点 Z 设计不一致 | 使用 `xgeDepthTestSet` 并统一 camera 透视参数。 |
| 高 DPI 下 viewport 错位 | 使用逻辑窗口尺寸替代 framebuffer 尺寸 | 从 `xgePlatformRuntimeGet` 读取 framebuffer 尺寸。 |

## 相关示例

- `examples/mvp`
- `examples/render_target`
- `examples/perspective_quad`
- `examples/debug_overlay`
