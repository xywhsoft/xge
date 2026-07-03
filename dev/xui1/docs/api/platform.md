# Platform API

> Platform API 负责平台后端、图形后端、运行时能力和跨平台状态查询。

[返回 API 索引](README.md) | [平台教程](../guide/platform-intro.md) | [平台冒烟范例](../case/platform-smoke.md)

---

## 模块定位

XGE 把平台容器和图形能力拆成两层：

- `xge_platform_backend_t`：窗口、事件、输入、present/swap 等平台容器能力。
- `xge_graphics_backend_t`：OpenGL 3.3、OpenGL ES 3.0、WebGL2 或自定义图形后端。

普通游戏通常使用默认后端。只有嵌入宿主、小程序、板卡 Linux、离屏渲染或自定义窗口系统时，才需要在 `xgeInit` 前显式设置后端。

## 标准调用顺序

```text
可选：xgePlatformBackendSet / xgeGraphicsBackendSet
  -> xgeInit
    -> xgePlatformCapsGet / xgeGpuCapsGet / xgePlatformRuntimeGet
  -> xgeUnit
```

后端设置必须发生在 `xgeInit` 之前。初始化后再切换后端属于未定义使用方式，第一版文档不承诺这种场景。

## 常量与宏

| 名称 | 值 | 说明 |
| --- | --- | --- |
| `XGE_PLATFORM_BACKEND_NONE` | `0` | 未指定平台后端。 |
| `XGE_PLATFORM_BACKEND_SOKOL` | `1` | Sokol 平台后端。 |
| `XGE_PLATFORM_BACKEND_MINIPROGRAM` | `2` | 小程序宿主桥接后端。 |
| `XGE_PLATFORM_BACKEND_EGL` | `3` | EGL/pbuffer/surfaceless 后端。 |
| `XGE_PLATFORM_BACKEND_CUSTOM` | `100` | 用户自定义平台后端。 |
| `XGE_GPU_BACKEND_NONE` | `0` | 未指定图形后端。 |
| `XGE_GPU_BACKEND_OPENGL33` | `1` | OpenGL 3.3/Core 风格。 |
| `XGE_GPU_BACKEND_GLES30` | `2` | OpenGL ES 3.0。 |
| `XGE_GPU_BACKEND_WEBGL2` | `3` | WebGL2。 |
| `XGE_GPU_BACKEND_CUSTOM` | `100` | 用户自定义图形后端。 |

## 公共类型

### `xge_platform_backend_t`

平台后端描述结构。XGE 会复制结构体本身，但不会接管 `pUser` 或 `sName` 指向内存的所有权。

| 字段 | 说明 |
| --- | --- |
| `iType` | `XGE_PLATFORM_BACKEND_*`。传入 `XGE_PLATFORM_BACKEND_NONE` 时，`xgePlatformBackendSet` 会按自定义后端处理。 |
| `sName` | 后端名称，主要用于诊断和能力快照。可以为 `NULL`。 |
| `init` | 后端初始化回调，可以为 `NULL`。 |
| `unit` | 后端释放回调，可以为 `NULL`。 |
| `poll` | 事件轮询回调，可以为 `NULL`。 |
| `present` | present/swap 回调，可以为 `NULL`。 |
| `pUser` | 回调透传数据，生命周期由调用者管理。 |

### `xge_graphics_backend_t`

图形后端描述结构。XGE 会复制结构体本身，但不会接管 `pUser` 或 `sName` 指向内存的所有权。

| 字段 | 说明 |
| --- | --- |
| `iType` | `XGE_GPU_BACKEND_*`。传入 `XGE_GPU_BACKEND_NONE` 时，`xgeGraphicsBackendSet` 会按自定义后端处理。 |
| `sName` | 图形后端名称，主要用于诊断和能力快照。可以为 `NULL`。 |
| `init` | 图形后端初始化回调，可以为 `NULL`。 |
| `unit` | 图形后端释放回调，可以为 `NULL`。 |
| `begin` | 开始一帧或一次提交的回调，可以为 `NULL`。 |
| `end` | 结束一帧或一次提交的回调，可以为 `NULL`。 |
| `pUser` | 回调透传数据，生命周期由调用者管理。 |

### `xge_gpu_caps_t`

GPU 能力快照结构。调用者提供结构体，XGE 写入当前后端和 GL 能力信息。

### `xge_graphics_mapping_t`

GL/GLES/WebGL 映射结构。用于让纹理、shader 和诊断代码查询当前后端对应的 shader header、纹理格式和特性开关。

### `xge_platform_caps_t`

平台能力快照结构。字段均为输出值，用于判断当前平台是否支持窗口、离屏、触摸、键盘、音频、高 DPI、线程安全提交等能力。

### `xge_platform_runtime_t`

平台运行时快照结构。包含运行状态、逻辑窗口尺寸、framebuffer 尺寸、DPI 缩放和输入事件计数。

## API 列表

- `xgePlatformBackendDefault`
- `xgePlatformBackendSet`
- `xgePlatformBackendGet`
- `xgeGraphicsBackendDefault`
- `xgeGraphicsBackendSet`
- `xgeGraphicsBackendGet`
- `xgeGpuCapsGet`
- `xgePlatformCapsGet`
- `xgePlatformRuntimeGet`
- `xgeGraphicsShaderHeaderGet`
- `xgeGraphicsLibraryNameGet`
- `xgeGraphicsMappingGet`

## 平台后端

### xgePlatformBackendDefault

获取当前编译目标的默认平台后端描述。

**功能：**

你可以用它查看 XGE 默认会选择的窗口和事件容器后端，也可以以默认后端为基础修改少量字段后传给 `xgePlatformBackendSet`。

**函数原型：**

```c
XGE_API xge_platform_backend_t xgePlatformBackendDefault(void);
```

**参数：**

无。

**返回值：**

- 返回 `xge_platform_backend_t` 结构体副本。
- 当前实现默认返回 `XGE_PLATFORM_BACKEND_SOKOL`，名称为 `"sokol"`。

**资源归属：**

返回值是结构体副本，调用者不需要释放。`sName` 是借用指针，调用者不能释放。

**补充说明：**

- 该函数不初始化平台后端。
- 该函数不要求先调用 `xgeInit`。
- 自定义宿主如果只想覆盖部分回调，可以先取默认值再修改。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_platform_backend_t backend = xgePlatformBackendDefault();
	printf("platform backend: %s\n", backend.sName);
	return 0;
}
```

**相关 API：**

- `xgePlatformBackendSet`
- `xgePlatformBackendGet`
- `xgePlatformCapsGet`

---

### xgePlatformBackendSet

设置 XGE 将使用的平台后端。

**功能：**

你可以在初始化前替换默认平台后端，用于小程序桥接、EGL 离屏、自定义宿主或测试环境。

**函数原型：**

```c
XGE_API int xgePlatformBackendSet(const xge_platform_backend_t* pBackend);
```

**参数：**

- `pBackend`：输入参数，不能为 `NULL`。指向平台后端描述结构，函数会复制结构体内容。

**返回值：**

- 成功返回 `XGE_OK`。
- `pBackend == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

XGE 只复制 `xge_platform_backend_t` 结构体本身，不复制 `sName` 指向的字符串，也不接管 `pUser`。如果自定义后端使用 `pUser`，调用者必须保证它至少存活到 `xgeUnit` 后。

**补充说明：**

- 应在 `xgeInit` 前调用。
- 如果 `pBackend->iType` 为 `XGE_PLATFORM_BACKEND_NONE`，当前实现会把内部类型修正为 `XGE_PLATFORM_BACKEND_CUSTOM`。
- 该函数只记录后端配置，不立即调用后端 `init`。

**范例代码：**

```c
#include "xge.h"
#include <string.h>

int main(void) {
	xge_platform_backend_t backend;
	memset(&backend, 0, sizeof(backend));
	backend.iType = XGE_PLATFORM_BACKEND_CUSTOM;
	backend.sName = "host-app";

	if (xgePlatformBackendSet(&backend) != XGE_OK) {
		return 1;
	}
	return 0;
}
```

**相关 API：**

- `xgePlatformBackendDefault`
- `xgePlatformBackendGet`
- `xgeInit`

---

### xgePlatformBackendGet

读取当前平台后端描述。

**功能：**

你可以在诊断、日志或宿主集成代码中读取当前平台后端类型和名称。

**函数原型：**

```c
XGE_API xge_platform_backend_t xgePlatformBackendGet(void);
```

**参数：**

无。

**返回值：**

- 返回当前平台后端结构体副本。
- 如果尚未设置平台后端，返回 `xgePlatformBackendDefault()` 的结果。

**资源归属：**

返回值是结构体副本，调用者不需要释放。返回结构中的 `sName` 和 `pUser` 仍然是借用指针。

**补充说明：**

- 该函数不触发后端初始化。
- 该函数可用于 `xgeInit` 前后的诊断。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_platform_backend_t backend = xgePlatformBackendGet();
	printf("platform type: %d\n", backend.iType);
	return 0;
}
```

**相关 API：**

- `xgePlatformBackendDefault`
- `xgePlatformBackendSet`
- `xgePlatformCapsGet`

---

## 图形后端

### xgeGraphicsBackendDefault

获取当前编译目标的默认图形后端描述。

**功能：**

你可以用它查看 XGE 默认的图形 API 基线，并在需要时基于默认描述创建自定义后端配置。

**函数原型：**

```c
XGE_API xge_graphics_backend_t xgeGraphicsBackendDefault(void);
```

**参数：**

无。

**返回值：**

- 返回 `xge_graphics_backend_t` 结构体副本。
- 当前实现默认返回 `XGE_GPU_BACKEND_OPENGL33`，名称为 `"opengl33"`。

**资源归属：**

返回值是结构体副本，调用者不需要释放。`sName` 是借用指针，调用者不能释放。

**补充说明：**

- 默认值表达 XGE 的桌面开发基线，不表示所有平台都使用 OpenGL 3.3。
- 小程序、Web、Android、板卡 Linux 等路径会根据后端切换到 WebGL2 或 GLES3 风格。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_graphics_backend_t backend = xgeGraphicsBackendDefault();
	printf("graphics backend: %s\n", backend.sName);
	return 0;
}
```

**相关 API：**

- `xgeGraphicsBackendSet`
- `xgeGraphicsBackendGet`
- `xgeGraphicsMappingGet`

---

### xgeGraphicsBackendSet

设置 XGE 将使用的图形后端。

**功能：**

你可以在初始化前指定 OpenGL 3.3、OpenGL ES 3.0、WebGL2 或自定义图形后端。该选择会影响 shader header、纹理格式映射、GPU 能力快照和渲染后端行为。

**函数原型：**

```c
XGE_API int xgeGraphicsBackendSet(const xge_graphics_backend_t* pBackend);
```

**参数：**

- `pBackend`：输入参数，不能为 `NULL`。指向图形后端描述结构，函数会复制结构体内容。

**返回值：**

- 成功返回 `XGE_OK`。
- `pBackend == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

XGE 只复制 `xge_graphics_backend_t` 结构体本身，不复制 `sName` 指向的字符串，也不接管 `pUser`。如果自定义图形后端使用 `pUser`，调用者必须保证它至少存活到 `xgeUnit` 后。

**补充说明：**

- 应在 `xgeInit` 前调用。
- 如果 `pBackend->iType` 为 `XGE_GPU_BACKEND_NONE`，当前实现会把内部类型修正为 `XGE_GPU_BACKEND_CUSTOM`。
- 调用成功后，内部 GPU caps 的 `iBackend` 会同步为该图形后端类型。

**范例代码：**

```c
#include "xge.h"
#include <string.h>

int main(void) {
	xge_graphics_backend_t backend;
	memset(&backend, 0, sizeof(backend));
	backend.iType = XGE_GPU_BACKEND_GLES30;
	backend.sName = "gles30";

	return xgeGraphicsBackendSet(&backend) == XGE_OK ? 0 : 1;
}
```

**相关 API：**

- `xgeGraphicsBackendDefault`
- `xgeGraphicsBackendGet`
- `xgeGraphicsShaderHeaderGet`
- `xgeGraphicsMappingGet`

---

### xgeGraphicsBackendGet

读取当前图形后端描述。

**功能：**

你可以在日志、shader 构建、平台诊断或兼容性分支中读取当前图形后端类型和名称。

**函数原型：**

```c
XGE_API xge_graphics_backend_t xgeGraphicsBackendGet(void);
```

**参数：**

无。

**返回值：**

- 返回当前图形后端结构体副本。
- 如果尚未设置图形后端，返回 `xgeGraphicsBackendDefault()` 的结果。

**资源归属：**

返回值是结构体副本，调用者不需要释放。返回结构中的 `sName` 和 `pUser` 仍然是借用指针。

**补充说明：**

- 该函数不创建 GL context。
- 该函数可用于 `xgeInit` 前后的诊断。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_graphics_backend_t backend = xgeGraphicsBackendGet();
	printf("graphics type: %d\n", backend.iType);
	return 0;
}
```

**相关 API：**

- `xgeGraphicsBackendDefault`
- `xgeGraphicsBackendSet`
- `xgeGpuCapsGet`

---

## 能力与运行时查询

### xgeGpuCapsGet

获取当前 GPU 能力快照。

**功能：**

你可以把 GPU vendor、renderer、version、shading language 和最大纹理尺寸写入日志或诊断报告，用于排查平台兼容问题。

**函数原型：**

```c
XGE_API int xgeGpuCapsGet(xge_gpu_caps_t* pCaps);
```

**参数：**

- `pCaps`：输出参数，不能为 `NULL`。函数会先清零结构体，再写入能力快照。

**返回值：**

- 成功返回 `XGE_OK`。
- `pCaps == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用者持有 `pCaps` 指向的结构体。XGE 不保存该指针，输出字符串写入结构体内部定长数组，不需要释放。

**补充说明：**

- 未创建或未运行 GL context 时，当前实现仍会返回后端基线版本，但 vendor、renderer 等 GL 字符串可能为空。
- `XGE_GPU_BACKEND_OPENGL33` 对应版本 `3.3`。
- `XGE_GPU_BACKEND_GLES30` 对应版本 `3.0`。
- `XGE_GPU_BACKEND_WEBGL2` 对应版本 `2.0`。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_gpu_caps_t caps;
	if (xgeGpuCapsGet(&caps) != XGE_OK) {
		return 1;
	}
	printf("gpu backend=%d version=%d.%d\n",
		caps.iBackend,
		caps.iMajorVersion,
		caps.iMinorVersion);
	return 0;
}
```

**相关 API：**

- `xgeGraphicsBackendGet`
- `xgePlatformCapsGet`
- `xgeDebugDumpCaps`

---

### xgePlatformCapsGet

获取当前平台能力快照。

**功能：**

你可以查询当前平台是否支持窗口、离屏渲染、pbuffer、surfaceless、触摸、鼠标、键盘、文本输入、手柄、音频、高 DPI 和线程安全提交。

**函数原型：**

```c
XGE_API int xgePlatformCapsGet(xge_platform_caps_t* pCaps);
```

**参数：**

- `pCaps`：输出参数，不能为 `NULL`。函数会先清零结构体，再写入能力快照。

**返回值：**

- 成功返回 `XGE_OK`。
- `pCaps == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用者持有 `pCaps` 指向的结构体。XGE 不保存该指针，输出字符串写入结构体内部定长数组，不需要释放。

**补充说明：**

- Sokol 后端会根据编译目标标记 Windows、Linux X11/Wayland、macOS、Android、iOS 或 Web 能力。
- 小程序后端会标记 `bMiniProgram` 和 `bWASM`。
- EGL 后端会根据 `xgeEGLCapsGet` 结果标记离屏、pbuffer、surfaceless 和板卡 Linux 能力。
- 该函数描述的是当前编译和后端路径的能力，不等同于所有目标平台都已经实机验证。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_platform_caps_t caps;
	if (xgePlatformCapsGet(&caps) != XGE_OK) {
		return 1;
	}
	printf("window=%d offscreen=%d touch=%d\n",
		caps.bWindow,
		caps.bOffscreen,
		caps.bTouch);
	return 0;
}
```

**相关 API：**

- `xgeGpuCapsGet`
- `xgePlatformRuntimeGet`
- `xgeEGLCapsGet`

---

### xgePlatformRuntimeGet

获取当前平台运行时状态快照。

**功能：**

你可以查询窗口尺寸、framebuffer 尺寸、DPI 缩放和输入事件计数，用于布局、渲染尺寸选择、调试面板或平台冒烟测试。

**函数原型：**

```c
XGE_API int xgePlatformRuntimeGet(xge_platform_runtime_t* pRuntime);
```

**参数：**

- `pRuntime`：输出参数，不能为 `NULL`。函数会先清零结构体，再写入运行时快照。

**返回值：**

- 成功返回 `XGE_OK`。
- `pRuntime == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。

**资源归属：**

调用者持有 `pRuntime` 指向的结构体。XGE 不保存该指针。

**补充说明：**

- `iWindowWidth` / `iWindowHeight` 是逻辑窗口尺寸。
- `iFramebufferWidth` / `iFramebufferHeight` 是实际 framebuffer 尺寸，高 DPI 环境中可能大于逻辑窗口尺寸。
- `fDpiScale` 在未知时回退为 `1.0f`。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_platform_runtime_t runtime;
	if (xgePlatformRuntimeGet(&runtime) != XGE_OK) {
		return 1;
	}
	printf("window=%dx%d framebuffer=%dx%d dpi=%.2f\n",
		runtime.iWindowWidth,
		runtime.iWindowHeight,
		runtime.iFramebufferWidth,
		runtime.iFramebufferHeight,
		runtime.fDpiScale);
	return 0;
}
```

**相关 API：**

- `xgePlatformCapsGet`
- `xgeGetWidth`
- `xgeGetHeight`

---

## 图形映射辅助

### xgeGraphicsShaderHeaderGet

获取指定图形后端推荐的 shader header。

**功能：**

你可以在自定义 shader 或 material 系统中生成与当前后端匹配的 GLSL 版本声明和精度声明。

**函数原型：**

```c
XGE_API int xgeGraphicsShaderHeaderGet(int iBackend, char* sBuffer, int iSize);
```

**参数：**

- `iBackend`：输入参数，取 `XGE_GPU_BACKEND_*`。传入 `XGE_GPU_BACKEND_NONE` 时使用当前图形后端。
- `sBuffer`：输出缓冲区，不能为 `NULL`。
- `iSize`：输出缓冲区字节数，必须大于 `0`。

**返回值：**

- 成功返回写入长度，语义与 `snprintf` 一致。
- `sBuffer == NULL` 或 `iSize <= 0` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用者持有 `sBuffer`。XGE 只写入字符串，不保存该指针。

**补充说明：**

- GLES3 和 WebGL2 当前返回 `#version 300 es` 加默认 precision。
- OpenGL 3.3 当前返回 `#version 330 core`。
- 返回值可能等于或大于 `iSize`，调用者应按 `snprintf` 规则判断是否被截断。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	char header[128];
	int len = xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, header, sizeof(header));
	if (len < 0) {
		return 1;
	}
	printf("%s", header);
	return 0;
}
```

**相关 API：**

- `xgeGraphicsBackendGet`
- `xgeGraphicsMappingGet`
- `xgeShaderCreate`

---

### xgeGraphicsLibraryNameGet

获取指定图形后端的候选动态库名称。

**功能：**

你可以在自定义平台后端或诊断工具中枚举当前后端可能需要加载的 GL/GLES 动态库名称。

**函数原型：**

```c
XGE_API int xgeGraphicsLibraryNameGet(int iBackend, int iIndex, char* sBuffer, int iSize);
```

**参数：**

- `iBackend`：输入参数，取 `XGE_GPU_BACKEND_*`。传入 `XGE_GPU_BACKEND_NONE` 时使用当前图形后端。
- `iIndex`：输入参数，候选库序号，从 `0` 开始，不能小于 `0`。
- `sBuffer`：输出缓冲区，不能为 `NULL`。
- `iSize`：输出缓冲区字节数，必须大于 `0`。

**返回值：**

- 成功返回写入长度，语义与 `snprintf` 一致。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 指定序号没有候选库名称时返回 `XGE_ERROR_FILE_NOT_FOUND`，并把 `sBuffer[0]` 置为 `0`。

**资源归属：**

调用者持有 `sBuffer`。XGE 只写入字符串，不保存该指针。

**补充说明：**

- 该函数只提供候选名称，不负责加载动态库。
- 不同操作系统上的候选库数量和名称不同。
- 普通 XGE 用户不需要直接调用该函数。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	char name[128];
	int i;
	for (i = 0; ; i++) {
		int ret = xgeGraphicsLibraryNameGet(XGE_GPU_BACKEND_NONE, i, name, sizeof(name));
		if (ret == XGE_ERROR_FILE_NOT_FOUND) {
			break;
		}
		if (ret < 0) {
			return 1;
		}
		printf("library[%d]=%s\n", i, name);
	}
	return 0;
}
```

**相关 API：**

- `xgeGraphicsBackendGet`
- `xgeGraphicsMappingGet`
- `xgeDebugDumpCaps`

---

### xgeGraphicsMappingGet

获取指定图形后端的 GL/GLES/WebGL 映射信息。

**功能：**

你可以查询当前后端是否是 OpenGL Core、GLES 或 WebGL，以及 VAO、FBO、depth texture、RGBA8 格式和 shader header 等映射信息。

**函数原型：**

```c
XGE_API int xgeGraphicsMappingGet(int iBackend, xge_graphics_mapping_t* pMapping);
```

**参数：**

- `iBackend`：输入参数，取 `XGE_GPU_BACKEND_*`。传入 `XGE_GPU_BACKEND_NONE` 时使用当前图形后端。
- `pMapping`：输出参数，不能为 `NULL`。函数会先清零结构体，再写入映射信息。

**返回值：**

- 成功返回写入 shader header 的长度，语义来自内部 `xgeGraphicsShaderHeaderGet`。
- `pMapping == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- shader header 写入失败时返回对应错误码。

**资源归属：**

调用者持有 `pMapping` 指向的结构体。XGE 不保存该指针。

**补充说明：**

- OpenGL 3.3 路径会设置 `bOpenGLCore`。
- GLES3 路径会设置 `bGLES`。
- WebGL2 路径会同时设置 `bWebGL` 和 `bGLES`。
- `iRGBA8InternalFormat`、`iRGBAFormat`、`iUnsignedByteType` 是供内部纹理上传和高级用户参考的 GL 枚举值。

**范例代码：**

```c
#include "xge.h"
#include <stdio.h>

int main(void) {
	xge_graphics_mapping_t mapping;
	if (xgeGraphicsMappingGet(XGE_GPU_BACKEND_NONE, &mapping) < 0) {
		return 1;
	}
	printf("glcore=%d gles=%d webgl=%d\n",
		mapping.bOpenGLCore,
		mapping.bGLES,
		mapping.bWebGL);
	return 0;
}
```

**相关 API：**

- `xgeGraphicsShaderHeaderGet`
- `xgeGraphicsBackendGet`
- `xgeTextureCreateRGBA`

---

## 平台支持状态说明

正式文档只描述策略，不把未实机验证的平台写成“已支持”。实测结果以 [dev/docs/平台后端验证结果.md](../../dev/docs/平台后端验证结果.md) 为准。

| 目标 | 计划后端 | 图形基线 | 文档表述 |
| --- | --- | --- | --- |
| Windows | Sokol | OpenGL 3.3 | 可开发验证。 |
| Linux 桌面 | Sokol X11/Wayland 计划 | OpenGL 3.3 或 GLES3/EGL | X11/EGL 路径可验证，Wayland 建议支持。 |
| macOS | Sokol | OpenGL | 低优先级兼容路径。 |
| Android | Sokol/NativeActivity/APK | GLES3 | 脚手架与实机验证分开记录。 |
| iOS | Sokol | OpenGL ES | 低优先级兼容路径。 |
| Web | Emscripten/Sokol | WebGL2 | WebGL2 基线。 |
| 小程序 | 自定义宿主桥接 | WebGL2 风格 | 第一版核心目标。 |
| 板卡 Linux | EGL pbuffer/surfaceless，后续 GBM/KMS | GLES3 | 优先考虑近年新板卡。 |
| 离屏渲染 | EGL/offscreen RT | GLES3/OpenGL | 用于测试、服务端和工具链。 |

## 生命周期与所有权

`xgePlatformBackendSet` 和 `xgeGraphicsBackendSet` 必须在 `xgeInit` 之前调用。初始化后再切换后端不属于第一版支持范围。

所有 `Get` 函数返回结构体副本或写入调用者提供的结构体。XGE 不会因为这些查询把内部对象所有权交给调用者。

用户自定义后端中的 `pUser` 生命周期由调用者负责，必须保证至少覆盖 `xgeInit` 到 `xgeUnit`。

## 线程约束

平台后端设置、初始化和释放建议在主线程执行。运行时查询用于诊断和工具面板，如果平台后端不提供线程安全保证，应在主线程读取。

图形库名称、shader header 和 mapping 查询是只读辅助函数，可以作为 shader 构建或诊断工具调用。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 初始化后切换后端行为异常 | 后端选择应早于 `xgeInit` | 在 `xgeInit` 前调用 `xgePlatformBackendSet` / `xgeGraphicsBackendSet`。 |
| 高 DPI 渲染尺寸不对 | 逻辑窗口和 framebuffer 不一致 | 使用 `xgePlatformRuntimeGet` 的 framebuffer 字段。 |
| 文档说可接入但目标机跑不通 | 脚手架存在不等于实机通过 | 更新 `dev/docs/平台后端验证结果.md`，正式文档标记待验证。 |

## 相关示例

- `examples/platform_smoke`
- `examples/egl_surfaceless`
- `check_platform_smoke.bat`
- `check_platform_all.bat`
