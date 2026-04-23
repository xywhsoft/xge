# Mini Program API

> Mini Program API 负责小程序宿主桥接、Canvas/WebGL2 生命周期、touch/text 输入和音频命令转发。

[返回 API 索引](README.md) | [小程序教程](../guide/miniprogram-intro.md) | [小程序范例](../case/miniprogram-hello.md)

---

## 模块定位

小程序是 XGE 第一版核心目标之一。由于小程序环境不是标准桌面窗口，XGE 通过 host bridge 接收 Canvas/WebGL context、资源加载、request frame、touch/text 和音频命令。

本 API 描述 C 侧桥接边界。JS 宿主层负责平台 API 适配、Canvas 获取、资源读写和音频命令落地。

## 标准调用顺序

```text
xgeMiniProgramSetBridge 可选
  -> xgeMiniProgramInit / xgeMiniProgramInitSimple
  -> 宿主每帧调用 xgeMiniProgramFrame
  -> resize/touch/text 事件转发到 XGE
  -> xgeMiniProgramUnit
```

如果需要使用 `res://` 资源桥，建议先设置 bridge，再初始化小程序后端。初始化后设置 bridge 也可用，XGE 会补注册资源 provider。

## 公共类型

### `xge_miniprogram_desc_t`

| 字段 | 说明 |
| --- | --- |
| `pCanvas` | 小程序 Canvas 对象或宿主句柄。 |
| `pWebGLContext` | WebGL context 或宿主句柄。 |
| `iWidth` / `iHeight` | 逻辑尺寸。 |
| `fDevicePixelRatio` | DPR，小于等于 0 时按 1 处理。 |
| `pUser` | 用户数据。 |

### `xge_miniprogram_touch_t`

| 字段 | 说明 |
| --- | --- |
| `iId` | 小程序触点 ID。 |
| `fX` / `fY` | 坐标。 |
| `fForce` | 压力值，平台不支持时可为默认值。 |

### `xge_miniprogram_bridge_t`

| 字段 | 说明 |
| --- | --- |
| `request_frame` | 请求下一帧。 |
| `load_resource` | 宿主资源加载。 |
| `free_resource` | 宿主资源释放。 |
| `audio_command` | 宿主音频命令。 |
| `pUser` | 用户数据。 |

## API 参考

### xgeMiniProgramInit

使用完整描述初始化小程序后端。

**功能：**

你可以用它把当前平台后端设置为小程序，把图形后端设置为 WebGL2，并保存 Canvas、WebGL context、尺寸和 DPR 信息。

**函数原型：**

```c
XGE_API int xgeMiniProgramInit(const xge_miniprogram_desc_t* pDesc);
```

**参数：**

- `pDesc`：小程序初始化描述，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- `pDesc` 为 `NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

XGE 复制 `pDesc` 的值，不接管 `pCanvas`、`pWebGLContext` 或 `pUser` 指针所有权。

**补充说明：**

- `fDevicePixelRatio <= 0` 时会被修正为 `1.0f`。
- `iWidth` / `iHeight` 大于 0 时会同步到 XGE 当前尺寸。
- 如果 bridge 已设置 `load_resource`，会注册 `res://` 资源 provider。

**范例代码：**

```c
xge_miniprogram_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iWidth = 750;
desc.iHeight = 1334;
desc.fDevicePixelRatio = 2.0f;
xgeMiniProgramInit(&desc);
```

**相关 API：**

- `xgeMiniProgramInitSimple`
- `xgeMiniProgramSetBridge`
- `xgeMiniProgramUnit`

---

### xgeMiniProgramInitSimple

使用宽高和 DPR 简化初始化。

**功能：**

你可以在不需要传入 Canvas/context 句柄时，用最少参数初始化小程序后端。

**函数原型：**

```c
XGE_API int xgeMiniProgramInitSimple(int iWidth, int iHeight, float fDevicePixelRatio);
```

**参数：**

- `iWidth`：逻辑宽度，必须大于 0。
- `iHeight`：逻辑高度，必须大于 0。
- `fDevicePixelRatio`：DPR，小于等于 0 时由完整初始化路径按 1 处理。

**返回值：**

- 成功返回 `XGE_OK`。
- 宽高非法返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数内部构造临时描述并调用 `xgeMiniProgramInit`，不持有调用者资源。

**补充说明：**

该函数适合测试、简单宿主或 Canvas/context 已由外部 glue 管理的情况。

**范例代码：**

```c
xgeMiniProgramInitSimple(750, 1334, 2.0f);
```

**相关 API：**

- `xgeMiniProgramInit`

---

### xgeMiniProgramUnit

释放小程序后端状态。

**功能：**

你可以用它清空 XGE 保存的小程序描述，并标记小程序后端未初始化。

**函数原型：**

```c
XGE_API void xgeMiniProgramUnit(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

函数不释放宿主 Canvas、WebGL context 或 bridge 用户数据。

**补充说明：**

该函数只清理 XGE 小程序 runtime 状态。

**范例代码：**

```c
xgeMiniProgramUnit();
```

**相关 API：**

- `xgeMiniProgramInit`

---

### xgeMiniProgramSetBridge

设置宿主桥接回调。

**功能：**

你可以用它把 request frame、资源加载、资源释放和音频命令转发交给小程序宿主层实现。

**函数原型：**

```c
XGE_API int xgeMiniProgramSetBridge(const xge_miniprogram_bridge_t* pBridge);
```

**参数：**

- `pBridge`：bridge 描述，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- `pBridge` 为 `NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

XGE 复制 bridge 结构体，不接管回调函数或 `pUser` 生命周期。

**补充说明：**

- 初始化后设置 bridge 时，如果存在 `load_resource`，XGE 会注册 `res://` provider。
- 资源加载回调返回的数据由 `free_resource` 回调释放；未提供 `free_resource` 时，XGE 默认用 `xrtFree` 释放。

**范例代码：**

```c
xge_miniprogram_bridge_t bridge;
memset(&bridge, 0, sizeof(bridge));
bridge.request_frame = host_request_frame;
bridge.load_resource = host_load_resource;
xgeMiniProgramSetBridge(&bridge);
```

**相关 API：**

- `xgeMiniProgramRequestFrame`
- `xgeMiniProgramAudioCommand`

---

### xgeMiniProgramFrame

推进一帧。

**功能：**

宿主每次收到 requestAnimationFrame 或等价调度时，可以调用该函数驱动 XGE 帧流程。

**函数原型：**

```c
XGE_API int xgeMiniProgramFrame(double fTimeSeconds);
```

**参数：**

- `fTimeSeconds`：宿主时间，单位秒。当前实现暂未使用该参数。

**返回值：**

- 成功返回 `xgeFrame` 的结果。
- 小程序后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

帧内 update/draw 由 XGE 当前 scene 和生命周期系统处理。

**范例代码：**

```c
xgeMiniProgramFrame(time_seconds);
```

**相关 API：**

- `xgeMiniProgramRequestFrame`
- `xgeFrame`

---

### xgeMiniProgramResize

更新小程序尺寸和 DPR。

**功能：**

宿主 Canvas 尺寸或 DPR 变化时，可以用它同步 XGE 逻辑尺寸和默认 camera viewport。

**函数原型：**

```c
XGE_API int xgeMiniProgramResize(int iWidth, int iHeight, float fDevicePixelRatio);
```

**参数：**

- `iWidth`：新逻辑宽度，必须大于 0。
- `iHeight`：新逻辑高度，必须大于 0。
- `fDevicePixelRatio`：DPR，小于等于 0 时按 1 处理。

**返回值：**

- 成功返回 `XGE_OK`。
- 宽高非法返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不分配资源。

**补充说明：**

该函数会更新 `g_xge.iWidth`、`g_xge.iHeight` 和当前 camera viewport 宽高。

**范例代码：**

```c
xgeMiniProgramResize(width, height, dpr);
```

**相关 API：**

- `xgeMiniProgramInit`

---

### xgeMiniProgramTouch

批量转发多点触控。

**功能：**

宿主可以把小程序 touch 事件转换为 XGE touch event，并派发给当前 scene。

**函数原型：**

```c
XGE_API int xgeMiniProgramTouch(int iPhase, const xge_miniprogram_touch_t* pTouches, int iCount);
```

**参数：**

- `iPhase`：触控阶段，必须是 `XGE_TOUCH_BEGIN`、`XGE_TOUCH_MOVE`、`XGE_TOUCH_END` 或 `XGE_TOUCH_CANCEL`。
- `pTouches`：触点数组，不能为 `NULL`。
- `iCount`：触点数量，范围为 0 到 `XGE_TOUCH_MAX`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或 phase 非法返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数只在调用期间读取 `pTouches`，不会持有数组指针。

**补充说明：**

- XGE 会按触点 ID 维护内部 touch 列表和 delta。
- 函数会构造 `XGE_EVENT_TOUCH_*` 并调用 `xgeSceneDispatchEvent`。

**范例代码：**

```c
xge_miniprogram_touch_t touches[1];
touches[0].iId = 1;
touches[0].fX = 120.0f;
touches[0].fY = 240.0f;
xgeMiniProgramTouch(XGE_TOUCH_BEGIN, touches, 1);
```

**相关 API：**

- `xgeMiniProgramTouchOne`
- `xgeSceneDispatchEvent`

---

### xgeMiniProgramTouchOne

转发单个触点。

**功能：**

你可以用它简化单指事件转发，不需要手动构造触点数组。

**函数原型：**

```c
XGE_API int xgeMiniProgramTouchOne(int iPhase, int iId, float fX, float fY, float fForce);
```

**参数：**

- `iPhase`：触控阶段。
- `iId`：触点 ID，必须大于等于 0。
- `fX`：触点 X 坐标。
- `fY`：触点 Y 坐标。
- `fForce`：压力值。

**返回值：**

- 成功返回 `XGE_OK`。
- ID 或 phase 非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不分配调用者需要释放的资源。

**补充说明：**

内部会构造一个 `xge_miniprogram_touch_t` 并调用 `xgeMiniProgramTouch`。

**范例代码：**

```c
xgeMiniProgramTouchOne(XGE_TOUCH_MOVE, 1, x, y, 1.0f);
```

**相关 API：**

- `xgeMiniProgramTouch`

---

### xgeMiniProgramText

转发文本 codepoint。

**功能：**

宿主系统 IME 提交文本后，可以把 Unicode codepoint 传给 XGE 文本输入事件。

**函数原型：**

```c
XGE_API int xgeMiniProgramText(uint32_t iCodepoint);
```

**参数：**

- `iCodepoint`：Unicode codepoint，不能为 0。

**返回值：**

- 成功返回 `XGE_OK`。
- `iCodepoint` 为 0 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不分配资源。

**补充说明：**

函数会设置当前文本 codepoint，并派发 `XGE_EVENT_TEXT`。

**范例代码：**

```c
xgeMiniProgramText(0x4E2D);
```

**相关 API：**

- `xgeSceneDispatchEvent`
- `xgeXuiTextInputEvent`

---

### xgeMiniProgramRequestFrame

请求宿主调度下一帧。

**功能：**

XGE 或上层系统需要继续刷新时，可以通过该函数调用 bridge 的 `request_frame`。

**函数原型：**

```c
XGE_API int xgeMiniProgramRequestFrame(void);
```

**参数：**

无。

**返回值：**

- bridge 成功时返回宿主回调返回值。
- 未设置 `request_frame` 时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

宿主回调应避免阻塞，只负责安排下一次帧调度。

**范例代码：**

```c
xgeMiniProgramRequestFrame();
```

**相关 API：**

- `xgeMiniProgramSetBridge`
- `xgeMiniProgramFrame`

---

### xgeMiniProgramAudioCommand

向宿主发送音频命令。

**功能：**

你可以用它把 C 侧音频意图转发给小程序宿主，由宿主调用平台音频 API。

**函数原型：**

```c
XGE_API int xgeMiniProgramAudioCommand(int iCommand, int iHandle, const void* pData, int iSize);
```

**参数：**

- `iCommand`：命令 ID，由小程序后端和宿主协议约定。
- `iHandle`：宿主音频对象句柄或 ID。
- `pData`：命令数据，可以为 `NULL`。
- `iSize`：命令数据字节数，必须大于等于 0。

**返回值：**

- 成功时返回宿主回调返回值。
- `iSize < 0` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 未设置 `audio_command` 时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

`pData` 只在调用期间有效。如果宿主异步处理，bridge 需要自行复制数据。

**补充说明：**

音频命令协议不写死在 C 核心中，便于适配不同小程序平台。

**范例代码：**

```c
xgeMiniProgramAudioCommand(1, handle, NULL, 0);
```

**相关 API：**

- `xgeMiniProgramSetBridge`

---

## 生命周期与所有权

`xgeMiniProgramInit` / `xgeMiniProgramUnit` 管理小程序 runtime。Bridge 回调和 `pUser` 由宿主持有，XGE 不接管其内存。

触控数组和音频命令数据只在调用期间有效，XGE 如需保留必须内部复制。

## 线程约束

小程序环境通常由宿主 JS 或平台主线程驱动。`xgeMiniProgramFrame`、resize、touch、text 和 audio command 应按宿主事件顺序调用。

不要假设小程序支持 native 线程或 GL context 迁移。

## 后端差异

小程序不是标准浏览器，也不是桌面窗口。文件系统、音频、canvas、WebGL 和输入都依赖宿主桥接。

不同小程序平台 API 差异较大，XGE 只定义桥接边界和统一事件语义。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 画面尺寸模糊 | DPR 没传或 framebuffer 尺寸错误 | 调用 `xgeMiniProgramResize` 时传入正确 DPR。 |
| 触摸 ID 混乱 | 宿主没有保持触点 ID 稳定 | 从 touch begin 到 end 保持同一 `iId`。 |
| 资源加载失败 | 没设置 `load_resource` bridge | 初始化前或初始化后调用 `xgeMiniProgramSetBridge`。 |

## 相关示例

- `examples/miniprogram_hello`
- `examples\miniprogram_hello\build.bat`
