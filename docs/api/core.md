# Core API

> Core API 负责 XGE runtime 的初始化、退出、主循环、时间、窗口基础状态、调试统计和日志。

[返回 API 索引](README.md) | [相关教程](../guide/first-xge-program.md) | [相关案例](../case/minimal-window.md)

---

## 目录

- [模块定位](#模块定位)
- [标准调用顺序](#标准调用顺序)
- [常量与宏](#常量与宏)
- [公共类型](#公共类型)
- [运行时生命周期](#运行时生命周期)
  - [xgeInit](#xgeinit)
  - [xgeUnit](#xgeunit)
  - [xgeMemoryFree](#xgememoryfree)
- [运行循环](#运行循环)
  - [xgeRun](#xgerun)
  - [xgeQuit](#xgequit)
  - [xgeFrame](#xgeframe)
  - [xgeRender](#xgerender)
- [窗口与时间](#窗口与时间)
  - [xgeGetWidth](#xgegetwidth)
  - [xgeGetHeight](#xgegetheight)
  - [xgeGetDelta](#xgegetdelta)
  - [xgeGetFPS](#xgegetfps)
  - [xgeTimer](#xgetimer)
  - [xgeTimeNow](#xgetimenow)
  - [xgeSleep](#xgesleep)
  - [xgeSetTitle](#xgesettitle)
- [调试统计](#调试统计)
  - [xgeFrameStatsReset](#xgeframestatsreset)
  - [xgeFrameStatsGet](#xgeframestatsget)
  - [xgeDebugGetStats](#xgedebuggetstats)
  - [xgeDebugDumpCaps](#xgedebugdumpcaps)
- [日志](#日志)
  - [xgeLogSetLevel](#xgelogsetlevel)
  - [xgeLogGetLevel](#xgeloggetlevel)
  - [xgeLogWrite](#xgelogwrite)
  - [xgeLogFlush](#xgelogflush)
- [常见用法](#常见用法)
- [常见错误](#常见错误)
- [相关示例](#相关示例)

---

## 模块定位

Core 是所有 XGE 程序的入口层。使用者通过 `xgeInit` 创建运行环境，通过 `xgeRun` 或手动的 `xgeFrame` / `xgeRender` 驱动帧更新，最后通过 `xgeUnit` 释放全局资源。

Core 不负责具体资源加载、渲染命令、输入查询或音频播放，这些能力分别放在 Resource、Graphics、Drawing、Input 和 Audio 模块。

---

## 标准调用顺序

最小顺序如下：

1. 在栈上或静态区创建 `xge_desc_t`，并用 `memset` 清零。
2. 填写窗口尺寸、标题、初始化标志和运行模式。
3. 调用 `xgeInit`。
4. 使用 `xgeRun` 进入默认主循环，或在手动模式下循环调用 `xgeFrame` / `xgeRender`。
5. 程序退出前调用 `xgeUnit`。

---

## 常量与宏

| 名称 | 值 | 说明 | 什么时候使用 |
| --- | --- | --- | --- |
| `XGE_VERSION_MAJOR` | `2` | 主版本号 | 编译期版本判断。 |
| `XGE_VERSION_MINOR` | `0` | 次版本号 | 编译期版本判断。 |
| `XGE_VERSION_PATCH` | `0` | 修订号 | 编译期版本判断。 |
| `XGE_INIT_WINDOW` | `0x0000` | 默认窗口模式 | 普通桌面或移动窗口。 |
| `XGE_INIT_FULLSCREEN` | `0x0001` | 请求全屏窗口 | 游戏全屏。 |
| `XGE_INIT_NOFRAME` | `0x0002` | 请求无边框窗口 | 启动画面、工具自绘窗口。 |
| `XGE_INIT_RESIZABLE` | `0x0004` | 窗口可调整大小 | 工具、编辑器和 APP。 |
| `XGE_INIT_HIGHDPI` | `0x0008` | 请求高 DPI framebuffer | Retina、高 DPI 屏幕。 |
| `XGE_INIT_VSYNC` | `0x0010` | 请求垂直同步 | 降低撕裂和空转。 |
| `XGE_INIT_OFFSCREEN` | `0x0020` | 请求离屏初始化 | 工具、测试、无窗口环境。 |
| `XGE_RUN_GAME_LOOP` | `0` | XGE 控制主循环 | 常规游戏。 |
| `XGE_RUN_MANUAL` | `1` | 宿主控制循环 | 工具、APP、小程序和嵌入式宿主。 |
| `XGE_LOG_TRACE` 到 `XGE_LOG_OFF` | `0` 到 `6` | 日志等级 | 控制诊断输出。 |

---

## 公共类型

### `xge_result_t`

多数返回 `int` 的 API 使用 `xge_result_t` 语义。

| 字段 / 值 | 类型 / 值 | 默认值 | 说明 | 所有权 |
| --- | --- | --- | --- | --- |
| `XGE_OK` | `0` | - | 成功。 | 无 |
| `XGE_ERROR` | 负值 | - | 通用失败。 | 无 |
| `XGE_ERROR_INVALID_ARGUMENT` | 负值 | - | 参数非法。 | 无 |
| `XGE_ERROR_NOT_INITIALIZED` | 负值 | - | runtime 尚未初始化。 | 无 |
| `XGE_ERROR_ALREADY_INITIALIZED` | 负值 | - | runtime 已初始化。 | 无 |
| `XGE_ERROR_OUT_OF_MEMORY` | 负值 | - | 内存不足。 | 无 |
| `XGE_ERROR_FILE_NOT_FOUND` | 负值 | - | 文件不存在。 | 无 |
| `XGE_ERROR_UNSUPPORTED` | 负值 | - | 当前后端或平台不支持。 | 无 |
| `XGE_ERROR_BACKEND_FAILED` | 负值 | - | 平台后端失败。 | 无 |
| `XGE_ERROR_GPU_FAILED` | 负值 | - | GPU 后端失败。 | 无 |
| `XGE_ERROR_RESOURCE_FAILED` | 负值 | - | 资源加载或解析失败。 | 无 |
| `XGE_ERROR_AUDIO_FAILED` | 负值 | - | 音频后端失败。 | 无 |
| `XGE_ERROR_THREAD_FAILED` | 负值 | - | 线程创建或同步失败。 | 无 |

### `xge_desc_t`

初始化描述。调用者创建并填写，`xgeInit` 不接管该结构体本身。

| 字段 / 值 | 类型 / 值 | 默认值 | 说明 | 所有权 |
| --- | --- | --- | --- | --- |
| `iWidth` / `iHeight` | `int` | `0` | 初始窗口或离屏尺寸，单位为像素。 | caller-owned |
| `sTitle` | `const char*` | `NULL` | UTF-8 窗口标题。 | borrowed |
| `iFlags` | `uint32_t` | `0` | `XGE_INIT_*` 位标记。 | caller-owned |
| `iRunMode` | `int` | `0` | `XGE_RUN_GAME_LOOP` 或 `XGE_RUN_MANUAL`。 | caller-owned |
| `iTargetFPS` | `int` | `0` | 目标帧率，`0` 表示默认策略。 | caller-owned |
| `pNativeWindow` | `void*` | `NULL` | 宿主原生窗口句柄，语义由平台后端决定。 | borrowed |
| `pUser` | `void*` | `NULL` | 用户数据，供后端或宿主扩展使用。 | borrowed |

### `xge_frame_stats_t`

每帧统计信息，主要用于 debug 和性能观察。

| 字段 / 值 | 类型 / 值 | 默认值 | 说明 | 所有权 |
| --- | --- | --- | --- | --- |
| `iFrameCount` | `int` | `0` | 统计期内帧数。 | snapshot |
| `iDrawCallCount` | `int` | `0` | draw call 数量。 | snapshot |
| `iBatchCount` | `int` | `0` | batch 数量。 | snapshot |
| `iDirtyRectCount` | `int` | `0` | 脏矩形数量。 | snapshot |
| `fFrameTimeMs` | `float` | `0` | 当前帧耗时，毫秒。 | snapshot |
| `fFrameTimeAvgMs` | `float` | `0` | 平均帧耗时，毫秒。 | snapshot |
| `fFrameTimeMaxMs` | `float` | `0` | 最大帧耗时，毫秒。 | snapshot |

### `xge_debug_stats_t`

聚合调试统计。发布版可能只保留有限信息。

| 字段 / 值 | 类型 / 值 | 默认值 | 说明 | 所有权 |
| --- | --- | --- | --- | --- |
| `tFrame` | `xge_frame_stats_t` | 清零 | 帧统计。 | snapshot |
| `iTextureCount` | `int` | `0` | 纹理数量。 | snapshot |
| `iTextureMemoryBytes` | `int` | `0` | 纹理显存估算。 | snapshot |
| `iFontCount` | `int` | `0` | 字体数量。 | snapshot |
| `iAudioCount` | `int` | `0` | 音频对象数量。 | snapshot |
| `iLastGLError` | `int` | `0` | 最近一次 GL error。 | snapshot |

### `xge_scene_proc`

```c
typedef int (*xge_scene_proc)(void* pUser);
```

传给 `xgeRun` 的逐帧回调。返回 `0` 表示继续运行，返回非 `0` 表示请求退出。

---

## 运行时生命周期

### xgeInit

创建 XGE runtime、平台后端和图形后端。

**功能：**

你在程序启动时调用它，建立窗口、离屏环境、平台事件系统、图形后端和 XGE 全局状态。它不加载业务资源，也不接管调用者后续创建的 texture、sound、scene 等对象。

**函数原型：**

```c
XGE_API int xgeInit(const xge_desc_t* pDesc);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `pDesc` | 输入 | 否 | 初始化描述。调用期间只读；结构体、`sTitle`、`pNativeWindow` 和 `pUser` 都不被 XGE 接管。 |

**返回值：**

- `XGE_OK`：初始化成功。
- `XGE_ERROR_INVALID_ARGUMENT`：`pDesc` 非法或尺寸等字段不满足要求。
- `XGE_ERROR_ALREADY_INITIALIZED`：runtime 已经初始化。
- `XGE_ERROR_BACKEND_FAILED`：平台后端初始化失败。
- `XGE_ERROR_GPU_FAILED`：图形后端初始化失败。
- `XGE_ERROR_OUT_OF_MEMORY`：内部状态分配失败。

**资源归属：**

成功后 runtime 由 XGE 内部持有。调用者必须在退出时调用 `xgeUnit` 释放 runtime。`pDesc` 仍归调用者所有。

**补充说明：**

- 后端选择函数必须在 `xgeInit` 前调用。
- 初始化和退出应在同一 runtime 主线程完成。
- 高 DPI 下逻辑窗口尺寸不一定等于 framebuffer 尺寸，应通过 `xgePlatformRuntimeGet` 查询。

**范例代码：**

```c
#include "xge.h"
#include <string.h>

int main(void)
{
	xge_desc_t desc;

	memset(&desc, 0, sizeof(desc));
	desc.iWidth = 800;
	desc.iHeight = 600;
	desc.sTitle = "XGE";
	desc.iRunMode = XGE_RUN_GAME_LOOP;

	if ( xgeInit(&desc) != XGE_OK ) {
		return 1;
	}

	xgeUnit();
	return 0;
}
```

**相关 API：**

- `xgeUnit`
- `xgePlatformBackendSet`
- `xgeGraphicsBackendSet`

---

### xgeUnit

销毁 XGE runtime。

**功能：**

你在程序退出时调用它，释放 XGE 内部持有的平台、图形、输入、统计和后端状态。

**函数原型：**

```c
XGE_API void xgeUnit(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

释放 XGE runtime 内部资源。调用者自己创建并持有的 texture、sound、font、scene、XUI 控件等对象仍应按各模块规则释放。

**补充说明：**

- `xgeUnit` 后不要继续调用依赖 runtime 的 API。
- 不应在另一个线程仍在提交渲染或异步加载时直接退出 runtime。

**范例代码：**

```c
if ( xgeInit(&desc) == XGE_OK ) {
	xgeRun(MainFrame, NULL);
	xgeUnit();
}
```

**相关 API：**

- `xgeInit`
- `xgeQuit`

---

### xgeMemoryFree

释放由 XGE 返回并声明由调用者释放的内存。

**功能：**

用于释放 XGE API 返回给调用者拥有的堆内存，例如后续某些构建/导出类 API 产生的内存块。

**函数原型：**

```c
XGE_API void xgeMemoryFree(void* pData);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `pData` | 输入 | 可为 `NULL` | 由 XGE 分配并明确交给调用者释放的内存指针。 |

**返回值：**

无。

**资源归属：**

调用后 `pData` 指向的内存不再有效。不要用它释放栈内存、静态字符串、第三方库直接分配的内存，或 XGE opaque/透明资源对象本身。

**补充说明：**

- 与 `xgeTextureFree`、`xgeFontFree`、`xgeSoundFree` 这类资源释放函数不同。
- 如果某个 API 文档没有明确要求用 `xgeMemoryFree`，不要猜测使用。

**范例代码：**

```c
void* data = NULL;
int size = 0;

if ( xgeFontBuildXRFMemory(&font, 0x4E00, 128, &data, &size) == XGE_OK ) {
	/* use data */
	xgeMemoryFree(data);
}
```

**相关 API：**

- `xgeFontBuildXRFMemory`

---

## 运行循环

### xgeRun

进入默认主循环。

**功能：**

用于普通游戏或桌面窗口程序，让 XGE 后端接管事件循环、帧推进和渲染调度。

**函数原型：**

```c
XGE_API int xgeRun(xge_scene_proc procFrame, void* pUser);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `procFrame` | 输入 | 可为 `NULL` | 每帧回调。传 `NULL` 时由已安装 scene 或 runtime 默认流程驱动。 |
| `pUser` | 输入 | 可为 `NULL` | 透传给 `procFrame`，XGE 不接管其生命周期。 |

**返回值：**

- `XGE_OK`：主循环正常结束。
- `XGE_ERROR_NOT_INITIALIZED`：尚未调用 `xgeInit`。
- `XGE_ERROR_BACKEND_FAILED`：平台循环或后端执行失败。

**资源归属：**

不分配调用者需要释放的资源。`pUser` 由调用者持有。

**补充说明：**

- 回调返回 `0` 表示继续运行，返回非 `0` 表示请求退出。
- `xgeQuit` 会请求主循环退出，但通常不会中断当前帧。
- 小程序、部分嵌入式宿主和 APP 模式更适合使用手动循环。

**范例代码：**

```c
static int MainFrame(void* pUser)
{
	(void)pUser;
	xgeClear(xgeColorRGBA(20, 24, 32, 255));
	return 0;
}

xgeRun(MainFrame, NULL);
```

**相关 API：**

- `xgeQuit`
- `xgeFrame`
- `xgeRender`

---

### xgeQuit

请求退出主循环。

**功能：**

用于从回调、scene、输入处理或宿主逻辑中结束 `xgeRun`。

**函数原型：**

```c
XGE_API void xgeQuit(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

不分配资源，也不释放 runtime。退出后仍需要调用 `xgeUnit`。

**补充说明：**

- 它是退出请求，不保证立即停止当前回调。
- 不等价于 `exit()`，不会自动释放用户资源。

**范例代码：**

```c
if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
	xgeQuit();
}
```

**相关 API：**

- `xgeRun`
- `xgeUnit`

---

### xgeFrame

手动推进一帧输入、事件和 update。

**功能：**

用于 `XGE_RUN_MANUAL` 模式。宿主程序自己控制循环时，每次需要推进 XGE 状态就调用它。

**函数原型：**

```c
XGE_API int xgeFrame(void);
```

**参数：**

无。

**返回值：**

- `XGE_OK`：推进成功。
- `XGE_ERROR_NOT_INITIALIZED`：runtime 未初始化。
- 其他错误码：平台事件或 update 失败。

**资源归属：**

不分配调用者需要释放的资源。

**补充说明：**

- 通常与 `xgeRender` 配合使用。
- APP 模式可在有 dirty/refresh 请求时才调用渲染。

**范例代码：**

```c
while ( app_is_running() ) {
	xgeFrame();
	if ( needs_render ) {
		xgeRender();
	}
}
```

**相关 API：**

- `xgeRender`
- `xgeBegin`
- `xgeFlush`

---

### xgeRender

手动执行渲染提交。

**功能：**

用于手动模式中主动刷新画面，适合工具、编辑器、APP 模式、小程序宿主或离屏渲染流程。

**函数原型：**

```c
XGE_API int xgeRender(void);
```

**参数：**

无。

**返回值：**

- `XGE_OK`：渲染提交成功。
- `XGE_ERROR_NOT_INITIALIZED`：runtime 未初始化。
- `XGE_ERROR_GPU_FAILED`：图形提交失败。

**资源归属：**

不分配调用者需要释放的资源。

**补充说明：**

- 默认游戏循环中通常不需要手动调用。
- 如果使用显式 `xgeBegin` / `xgeEnd` / `xgeFlush`，应避免和 `xgeRender` 重复提交同一批命令。

**范例代码：**

```c
xgeFrame();
xgeRender();
```

**相关 API：**

- `xgeFrame`
- `xgeBegin`
- `xgeEnd`
- `xgeFlush`

---

## 窗口与时间

### xgeGetWidth

返回当前逻辑窗口宽度。

**功能：**

用于布局、camera、UI 和逻辑坐标计算。

**函数原型：**

```c
XGE_API int xgeGetWidth(void);
```

**参数：**

无。

**返回值：**

- 返回逻辑窗口宽度，单位为像素或逻辑像素，取决于平台后端。
- 未初始化时返回实现定义的安全值。

**资源归属：**

不分配资源。

**补充说明：**

- 高 DPI 下它不一定等于 framebuffer 宽度。
- 需要 framebuffer 尺寸时使用 `xgePlatformRuntimeGet`。

**范例代码：**

```c
float cx = (float)xgeGetWidth() * 0.5f;
```

**相关 API：**

- `xgeGetHeight`
- `xgePlatformRuntimeGet`

---

### xgeGetHeight

返回当前逻辑窗口高度。

**功能：**

用于布局、camera、UI 和逻辑坐标计算。

**函数原型：**

```c
XGE_API int xgeGetHeight(void);
```

**参数：**

无。

**返回值：**

- 返回逻辑窗口高度。
- 未初始化时返回实现定义的安全值。

**资源归属：**

不分配资源。

**补充说明：**

高 DPI 下它不一定等于 framebuffer 高度。

**范例代码：**

```c
xge_camera_t camera = xgeCameraDefault((float)xgeGetWidth(), (float)xgeGetHeight());
```

**相关 API：**

- `xgeGetWidth`
- `xgeCameraDefault`

---

### xgeGetDelta

返回上一帧 delta 时间。

**功能：**

用于 update、动画、补间和非固定步长逻辑。

**函数原型：**

```c
XGE_API float xgeGetDelta(void);
```

**参数：**

无。

**返回值：**

- 返回上一帧耗时，单位秒。
- runtime 未推进过帧时返回实现定义的初始值。

**资源归属：**

不分配资源。

**补充说明：**

固定步长逻辑应优先使用 scene update strategy，而不是直接依赖可变 delta。

**范例代码：**

```c
player.x += player.speed * xgeGetDelta();
```

**相关 API：**

- `xgeSceneUpdateStrategySet`
- `xgeTimer`

---

### xgeGetFPS

返回当前 FPS 估算。

**功能：**

用于 debug overlay、日志和性能观察。

**函数原型：**

```c
XGE_API int xgeGetFPS(void);
```

**参数：**

无。

**返回值：**

返回当前 FPS 估算值。

**资源归属：**

不分配资源。

**补充说明：**

该值用于观察，不建议作为逻辑分支的精确计时来源。

**范例代码：**

```c
int fps = xgeGetFPS();
```

**相关 API：**

- `xgeFrameStatsGet`

---

### xgeTimer

返回高精度运行时间。

**功能：**

用于性能测量、动画时间和工具诊断。

**函数原型：**

```c
XGE_API double xgeTimer(void);
```

**参数：**

无。

**返回值：**

返回时间值，单位秒。时间零点由实现决定，通常不应和墙钟时间混用。

**资源归属：**

不分配资源。

**补充说明：**

用于相对时间差，不用于日期时间显示。

**范例代码：**

```c
double t0 = xgeTimer();
/* work */
double ms = (xgeTimer() - t0) * 1000.0;
```

**相关 API：**

- `xgeTimeNow`

---

### xgeTimeNow

返回当前时间戳。

**功能：**

用于日志、诊断和需要记录“当前时间”的场景。

**函数原型：**

```c
XGE_API int64_t xgeTimeNow(void);
```

**参数：**

无。

**返回值：**

返回时间戳。具体单位和 epoch 由 xrt 时间实现决定。

**资源归属：**

不分配资源。

**补充说明：**

不要和 `xgeTimer` 混用做高精度帧间 delta。

**范例代码：**

```c
int64_t now = xgeTimeNow();
```

**相关 API：**

- `xgeTimer`

---

### xgeSleep

让当前线程休眠指定毫秒。

**功能：**

用于简单等待、测试脚本、低频轮询或非实时工具线程。

**函数原型：**

```c
XGE_API void xgeSleep(uint32_t iMilliseconds);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `iMilliseconds` | 输入 | 不适用 | 休眠时间，单位毫秒。 |

**返回值：**

无。

**资源归属：**

不分配资源。

**补充说明：**

系统调度精度由平台决定，不适合精确帧同步。

**范例代码：**

```c
xgeSleep(16);
```

**相关 API：**

- `xgeTimer`

---

### xgeSetTitle

设置窗口标题。

**功能：**

用于桌面窗口程序在启动、切换工程、显示调试信息时更新标题。

**函数原型：**

```c
XGE_API void xgeSetTitle(const char* sTitle);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `sTitle` | 输入 | 可按后端接受空值 | UTF-8 窗口标题。调用期间只读，不被 XGE 接管。 |

**返回值：**

无。

**资源归属：**

不接管 `sTitle`。调用者可在函数返回后释放或复用原始字符串，除非具体后端文档另有说明。

**补充说明：**

离屏、小程序或无窗口后端可能忽略该调用。

**范例代码：**

```c
xgeSetTitle("XGE Editor");
```

**相关 API：**

- `xgeInit`
- `xgePlatformRuntimeGet`

---

## 调试统计

### xgeFrameStatsReset

重置帧统计。

**功能：**

用于性能采样开始前清空历史统计，避免旧数据影响观察。

**函数原型：**

```c
XGE_API void xgeFrameStatsReset(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

不分配资源。

**补充说明：**

Debug overlay 或性能测试可以在进入测试段时调用。

**范例代码：**

```c
xgeFrameStatsReset();
```

**相关 API：**

- `xgeFrameStatsGet`

---

### xgeFrameStatsGet

读取帧统计快照。

**功能：**

用于显示 draw call、batch、帧耗时等性能信息。

**函数原型：**

```c
XGE_API xge_frame_stats_t xgeFrameStatsGet(void);
```

**参数：**

无。

**返回值：**

返回 `xge_frame_stats_t` 值拷贝。

**资源归属：**

返回值是值类型快照，不需要释放。

**补充说明：**

发布版可能只保留有限统计，具体取决于编译配置。

**范例代码：**

```c
xge_frame_stats_t stats = xgeFrameStatsGet();
```

**相关 API：**

- `xgeFrameStatsReset`
- `xgeDebugGetStats`

---

### xgeDebugGetStats

获取聚合调试统计。

**功能：**

用于 debug overlay、自动化报告和平台冒烟日志。

**函数原型：**

```c
XGE_API int xgeDebugGetStats(xge_debug_stats_t* pStats);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `pStats` | 输出 | 否 | 接收统计快照。调用者提供内存，XGE 不持有该指针。 |

**返回值：**

- `XGE_OK`：写入成功。
- `XGE_ERROR_INVALID_ARGUMENT`：`pStats` 为 `NULL`。
- `XGE_ERROR_NOT_INITIALIZED`：runtime 未初始化。

**资源归属：**

不分配资源。`pStats` 由调用者持有。

**补充说明：**

Debug 统计是诊断能力，不建议作为游戏逻辑依据。

**范例代码：**

```c
xge_debug_stats_t stats;
if ( xgeDebugGetStats(&stats) == XGE_OK ) {
	/* display stats */
}
```

**相关 API：**

- `xgeFrameStatsGet`
- `xgeDebugDumpCaps`

---

### xgeDebugDumpCaps

写入 GPU/platform 能力文本。

**功能：**

用于平台冒烟、日志和问题反馈报告。

**函数原型：**

```c
XGE_API int xgeDebugDumpCaps(char* sBuffer, int iSize);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `sBuffer` | 输出 | 否 | 接收 UTF-8 文本的缓冲区。 |
| `iSize` | 输入 | 不适用 | 缓冲区字节数，必须大于 0。 |

**返回值：**

- 正数或 0：写入的文本长度，具体语义由实现决定。
- 负值：失败，例如参数非法或 runtime 未初始化。

**资源归属：**

调用者持有 `sBuffer`。XGE 不分配新字符串。

**补充说明：**

发布版可能不提供完整 caps dump。

**范例代码：**

```c
char caps[4096];
if ( xgeDebugDumpCaps(caps, sizeof(caps)) > 0 ) {
	xgeLogWrite(XGE_LOG_INFO, "caps", caps);
}
```

**相关 API：**

- `xgeGpuCapsGet`
- `xgePlatformCapsGet`

---

## 日志

### xgeLogSetLevel

设置日志等级。

**功能：**

用于控制 XGE 内部和调用者写入日志的最小输出等级。

**函数原型：**

```c
XGE_API int xgeLogSetLevel(int iLevel);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `iLevel` | 输入 | 不适用 | `XGE_LOG_*` 之一。 |

**返回值：**

- `XGE_OK`：设置成功。
- `XGE_ERROR_INVALID_ARGUMENT`：日志等级非法。

**资源归属：**

不分配资源。

**补充说明：**

日志最终走 xrt logger 适配层。

**范例代码：**

```c
xgeLogSetLevel(XGE_LOG_INFO);
```

**相关 API：**

- `xgeLogGetLevel`
- `xgeLogWrite`

---

### xgeLogGetLevel

读取当前日志等级。

**功能：**

用于诊断 UI、测试和调试配置确认。

**函数原型：**

```c
XGE_API int xgeLogGetLevel(void);
```

**参数：**

无。

**返回值：**

返回当前 `XGE_LOG_*` 等级。

**资源归属：**

不分配资源。

**补充说明：**

该函数只读取 XGE 日志阈值，不代表底层 sink 一定输出该等级。

**范例代码：**

```c
int level = xgeLogGetLevel();
```

**相关 API：**

- `xgeLogSetLevel`

---

### xgeLogWrite

写一条日志。

**功能：**

用于应用层或 XGE 内部模块输出诊断信息。

**函数原型：**

```c
XGE_API int xgeLogWrite(int iLevel, const char* sTag, const char* sMessage);
```

**参数：**

| 参数 | 方向 | 是否可为 `NULL` | 说明 |
| --- | --- | --- | --- |
| `iLevel` | 输入 | 不适用 | `XGE_LOG_*` 日志等级。 |
| `sTag` | 输入 | 可按实现允许 `NULL` | 模块 tag，建议短小稳定，例如 `core`、`render`。 |
| `sMessage` | 输入 | 否 | UTF-8 日志文本。调用期间只读。 |

**返回值：**

- `XGE_OK`：写入或提交成功。
- 非 `XGE_OK`：等级非法、消息非法或底层 logger 失败。

**资源归属：**

不接管 `sTag` 和 `sMessage`，不返回调用者需要释放的资源。

**补充说明：**

- 不要在音频实时回调中写日志。
- 渲染线程中避免复杂格式化。

**范例代码：**

```c
xgeLogWrite(XGE_LOG_INFO, "app", "startup");
```

**相关 API：**

- `xgeLogSetLevel`
- `xgeLogFlush`

---

### xgeLogFlush

刷新日志后端。

**功能：**

用于测试、崩溃前诊断或程序退出前确保日志 sink 收到已提交消息。

**函数原型：**

```c
XGE_API int xgeLogFlush(void);
```

**参数：**

无。

**返回值：**

- `XGE_OK`：刷新成功。
- 非 `XGE_OK`：底层 logger 刷新失败。

**资源归属：**

不分配资源。

**补充说明：**

频繁 flush 可能影响性能，普通每帧日志不应每条都 flush。

**范例代码：**

```c
xgeLogWrite(XGE_LOG_INFO, "app", "shutdown");
xgeLogFlush();
```

**相关 API：**

- `xgeLogWrite`

---

## 常见用法

### 默认游戏循环

```c
xge_desc_t desc;

memset(&desc, 0, sizeof(desc));
desc.iWidth = 800;
desc.iHeight = 600;
desc.sTitle = "XGE Game";
desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
desc.iRunMode = XGE_RUN_GAME_LOOP;

if ( xgeInit(&desc) == XGE_OK ) {
	xgeRun(MainFrame, NULL);
	xgeUnit();
}
```

### 手动循环

```c
desc.iRunMode = XGE_RUN_MANUAL;
xgeInit(&desc);

while ( app_is_running() ) {
	xgeFrame();
	xgeRender();
}

xgeUnit();
```

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| `xgeInit` 返回失败 | 后端不可用、参数非法或 GPU 初始化失败 | 先调用平台检查脚本，再用 `xgeDebugDumpCaps` 获取能力信息。 |
| APP 模式没有刷新 | 只调用了 `xgeFrame`，没有调用 `xgeRender` 或 `xgeFlush` | 手动循环中显式驱动渲染。 |
| 高 DPI 下鼠标坐标和 framebuffer 不一致 | 逻辑尺寸与 framebuffer 尺寸不同 | 使用 `xgePlatformRuntimeGet` 读取 dpi scale 和 framebuffer 尺寸。 |

## 相关示例

- `examples/mvp`
- `examples/platform_smoke`
- `build_exe.bat`
- `check_platform_smoke.bat`
