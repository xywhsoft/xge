# XGE V2 技术实现方案

本文档用于在实现前固定 XGE V2 的主要技术路径。后续开发时，如果上下文被压缩，应优先以本文档、`XGE_V2_DESIGN.md` 和 `XGE_V2_SPEC.md` 为准。

## 1. 总体架构

XGE V2 的仓库源码按模块组织，单头文件只是一种发布形态，由 `singlehead/` 工具组装生成。根目录 `xge.h` 只保留公开声明，具体实现放在 `src/`。

逻辑分层：

```text
Application
  |
  v
XGE Public API
  |
  +-- Core / Lifecycle
  +-- Scene
  +-- Input
  +-- XUI bridge
  +-- Resource API
  +-- Audio API
  +-- Render2D API
  +-- Advanced Mesh/2.5D API
  |
  v
XGE Internal Systems
  |
  +-- Platform Backend
  +-- Render Command Queue
  +-- GPU Backend
  +-- Resource Loader
  +-- Audio Backend
  +-- Font/Text System
  +-- XUI Incubation / Bridge
  |
  v
xrt / sokol / stb_image / miniaudio / stb_truetype / GL-GLES-WebGL
```

强约束：

- XGE 内核完全依赖 xrt。
- XGE 不实现网络、压缩包、通用容器等外围能力。
- 所有默认渲染走 GPU。
- OpenGL/GLES/WebGL 调用集中在 render thread/context owner。
- 普通 API 可以多线程提交命令，但不能让用户直接跨线程调用 GL。

## 2. 源码与单头文件组织

仓库目录结构：

```text
/
  xge.h                         对外主头文件
  xge.c                         DLL/EXE 构建入口或聚合实现入口
  examples\mvp\build.bat                 Windows EXE 构建脚本
  examples/mvp/build.sh                  Linux/macOS EXE 构建脚本
  build_dll.bat                 Windows DLL 构建脚本
  build_dll.sh                  Linux/macOS shared library 构建脚本
  src/                          引擎模块源码
  lib/                          第三方库，包括 xrt、sokol、stb_image、miniaudio、stb_truetype
  test/                         测试代码
  singlehead/                   单头文件版本、生成器和单头文件测试代码
  docs/                         文档
  examples/                     示例
```

根目录 `xge.h` 和 `xge.c` 用于常规开发与构建。`xge.c` 是聚合编译入口，包含 `src/` 中按功能拆分的实现模块。`singlehead/` 用于生成和验证真正发布用的单头文件版本，可参考 xrt、xpack 等项目的目录结构。

根目录 `xge.h` 结构：

```c
#ifndef XGE_H
#define XGE_H

/* version */
/* config macros */
/* dependency config */
/* public types */
/* public enums */
/* public structs */
/* public API declarations */

#endif /* XGE_H */
```

`src/` 当前模块拆分：

```text
src/xge_impl.c       内部公共上下文、依赖接入、通用 helper、模块聚合
src/xge_core.c       生命周期、窗口、camera、颜色/blend
src/xge_resource.c   资源协议
src/xge_audio.c      miniaudio wrapper
src/xge_font.c       UTF-8、TTF、XRF、文本绘制
src/xge_render.c     image/texture/sprite draw
src/xge_input.c      keyboard/mouse/touch/text input
src/xge_xui.c        XUI 孵化期 context、widget tree、dirty layout、paint 统计
src/xge_gl.h         OpenGL 函数加载
```

单头文件生成工具应按 `xge.h` + `src/` 模块顺序组装发布文件。仓库内不直接维护手写的巨型单头文件实现。内部函数尽量保持模块前缀：

```c
xge_core_*
xge_platform_*
xge_gpu_*
xge_render_*
xge_audio_*
xge_text_*
xge_xui_*
```

## 3. 编译宏

核心宏：

```c
XGE_DEBUG                /* debug 检查与诊断 */
XGE_NO_AUDIO             /* 禁用音频 */
XGE_NO_TEXT              /* 禁用文本 */
XGE_NO_XUI_BRIDGE        /* 禁用 XUI 桥接 */
XGE_NO_MINIPROGRAM       /* 禁用小程序后端 */
XGE_NO_OFFSCREEN         /* 禁用离屏后端 */
XGE_STATIC               /* 静态/内嵌使用 */
XGE_DLL                  /* DLL 导出 */
XGE_API                  /* public symbol visibility */
```

常规构建通过编译根目录 `xge.c` 启用实现；`xge.h` 不接受 `XGE_IMPL` 这类实现宏。发布用单头文件由 `singlehead/` 生成器把公开声明和 `src/` 模块源码组装到一个文件中，生成文件内部可以使用私有宏隔离实现区。

平台宏由编译器检测和 xrt/sokol 信息共同决定：

```c
XGE_PLATFORM_WINDOWS
XGE_PLATFORM_LINUX
XGE_PLATFORM_MACOS
XGE_PLATFORM_ANDROID
XGE_PLATFORM_IOS
XGE_PLATFORM_WEB
XGE_PLATFORM_MINIPROGRAM
```

图形后端宏：

```c
XGE_GPU_GLCORE
XGE_GPU_GLES3
XGE_GPU_WEBGL2
```

## 3.1 代码风格硬约束

代码风格必须遵守 `docs/代码风格规则.md`。

补充约定：

- 缩进使用 Tab，不使用空格缩进；遇到空格缩进需要转换为 Tab。
- 换行使用 CRLF。
- 类型命名使用 `xge_texture_t` 这种 snake_case 形式。
- 指针 typedef 可按 xrt 风格与结构体 typedef 同行定义。
- 公开函数使用 `xge` 前缀，例如 `xgeTextureLoad`。
- 内部函数添加两条前导下划线，例如 `__xgeRenderFlush`。
- 内部无锁版本使用 `_NoLock` 后缀。
- 注释使用中文。
- 函数定义左花括号另起一行。
- `if`、`for`、`while` 的括号内外按 xrt 风格添加空格。

示例：

```c
// 纹理对象
typedef struct xge_texture_t {
	int iWidth;              // 宽度
	int iHeight;             // 高度
	int iFormat;             // 像素格式
	int iRefCount;           // 引用计数
	uint32_t iFlags;         // 标志
	uint32_t iBackendId;     // 后端ID（ 内部字段 ）
	void* pBackend;          // 后端对象（ 内部字段 ）
} xge_texture_t, *xge_texture;

static int __xgeTextureUpload(xge_texture pTexture)
{
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}
```

## 4. 第一阶段 MVP

第一阶段 MVP 只实现验证引擎基本链路所需的最小功能，不实现完整 spec。

MVP 范围：

```text
1. 建立根目录结构
2. 创建 xge.h / xge.c 骨架
3. 创建 examples\mvp\build.bat / examples/mvp/build.sh
4. 创建最小 example
5. 实现 xgeInit / xgeUnit / xgeRun / xgeQuit
6. 接入 xrt allocator/file/time 的最小路径
7. 接入 Sokol，打开窗口
8. 创建 OpenGL/GLES/WebGL context
9. 实现 clear/present
10. 实现最小输入：键盘与鼠标状态
11. 实现最小纹理加载和 xgeDraw 绘制一张纹理
12. 做人工冒烟测试
13. 更新 XGE_V2_SPEC.md
```

MVP 不包含：

```text
完整场景系统
XUI 桥接接口
完整音频系统
完整异步资源
小程序后端
EGL/离屏后端
XRF
2.5D
高级 material/shader
```

这些功能在 MVP 验证通过后按 spec 分阶段实现。

## 5. 全局状态

XGE 使用一个内部全局状态对象：

```c
typedef struct xge_context_t {
    int initialized;
    int quitting;

    XgeDesc desc;
    XgePlatformState platform;
    XgeGpuState gpu;
    XgeRenderState render;
    XgeInputState input;
    XgeSceneState scene;
    XgeAudioState audio;
    XgeTextState text;
    XgeXuiState xui;
    XgeResourceState resource;

    XgeDebugStats debugStats;
} xge_context_t;
```

第一版可以使用单实例：

```c
static xge_context_t g_xge;
```

多实例不是第一版目标。离屏渲染使用同一个 context 内的 offscreen platform/render target。

## 6. 初始化结构

`XgeDesc` 是入口配置结构，应尽量稳定。

建议：

```c
typedef struct xge_desc_t {
    int width;
    int height;
    const char* title;

    int flags;
    int runMode;
    int targetFps;

    int platformBackend;
    int gpuBackend;

    int enableAudio;
    int enableText;
    int enableXuiBridge;
    int enableDebug;

    void* nativeWindow;
    void* user;
} xge_desc_t;
```

`flags` 建议：

```c
XGE_INIT_WINDOW
XGE_INIT_FULLSCREEN
XGE_INIT_NOFRAME
XGE_INIT_RESIZABLE
XGE_INIT_HIGHDPI
XGE_INIT_VSYNC
XGE_INIT_OFFSCREEN
```

`platformBackend`：

```c
XGE_PLATFORM_BACKEND_AUTO
XGE_PLATFORM_BACKEND_SOKOL
XGE_PLATFORM_BACKEND_EGL
XGE_PLATFORM_BACKEND_MINIPROGRAM
```

`gpuBackend`：

```c
XGE_GPU_BACKEND_AUTO
XGE_GPU_BACKEND_GLCORE
XGE_GPU_BACKEND_GLES3
XGE_GPU_BACKEND_WEBGL2
```

初始化顺序：

```text
1. 清空 g_xge
2. 保存 XgeDesc，填充默认值
3. 初始化 xrt allocator/file/time/thread 基础能力
4. 初始化 platform backend
5. 创建 GL/GLES/WebGL context
6. 初始化 GPU backend 和 caps
7. 初始化 render command system
8. 初始化 input
9. 初始化 audio
10. 初始化 text/font
11. 初始化 layout
12. 标记 initialized
```

销毁顺序反向执行。

## 7. 错误码

所有返回 `int` 的 API 使用 `XgeResult`。

建议：

```c
typedef enum xge_result_t {
    XGE_OK = 0,
    XGE_ERROR = -1,
    XGE_ERROR_INVALID_ARGUMENT = -2,
    XGE_ERROR_NOT_INITIALIZED = -3,
    XGE_ERROR_ALREADY_INITIALIZED = -4,
    XGE_ERROR_OUT_OF_MEMORY = -5,
    XGE_ERROR_FILE_NOT_FOUND = -6,
    XGE_ERROR_UNSUPPORTED = -7,
    XGE_ERROR_BACKEND_FAILED = -8,
    XGE_ERROR_GPU_FAILED = -9,
    XGE_ERROR_RESOURCE_FAILED = -10,
    XGE_ERROR_AUDIO_FAILED = -11,
    XGE_ERROR_THREAD_FAILED = -12
} xge_result_t;
```

错误信息由 debug log 输出。第一版不需要复杂 error object。

## 8. 平台后端接口

平台后端负责窗口、事件、上下文创建、buffer swap、时间驱动。

内部接口：

```c
typedef struct xge_platform_api_t {
	int  (*init)(xge_context_t* pCtx);
	void (*shutdown)(xge_context_t* pCtx);
	void (*pollEvents)(xge_context_t* pCtx);
	void (*swapBuffers)(xge_context_t* pCtx);
	void* (*getProcAddress)(const char* sName);
	void* (*getNativeHandle)(xge_context_t* pCtx);
} xge_platform_api_t;
```

Sokol 后端覆盖：

- Windows
- Linux X11/Wayland
- macOS
- Android
- iOS
- Web/Emscripten

EGL 后端覆盖：

- board Linux
- offscreen
- pbuffer/surfaceless

Mini Program 后端：

- 独立 JS bridge
- WebGL2 Canvas
- touch/resource/game loop bridge

## 9. GPU 后端接口

GPU 后端提供比 OpenGL 调用更高一级的抽象，但不能过重。

内部接口：

```c
typedef struct xge_gpu_api_t {
	int  (*init)(xge_context_t* pCtx);
	void (*shutdown)(xge_context_t* pCtx);

	int  (*createTexture)(xge_texture pTexture, const xge_texture_desc_t* pDesc, const void* pPixels);
	void (*updateTexture)(xge_texture pTexture, const xge_rect_t* pRect, const void* pPixels);
	void (*destroyTexture)(xge_texture pTexture);

	int  (*createBuffer)(xge_gpu_buffer_t* pBuf, const xge_buffer_desc_t* pDesc);
	void (*updateBuffer)(xge_gpu_buffer_t* pBuf, int iOffset, int iSize, const void* pData);
	void (*destroyBuffer)(xge_gpu_buffer_t* pBuf);

	int  (*createShader)(xge_shader_t* pShader, const xge_shader_desc_t* pDesc);
	void (*destroyShader)(xge_shader_t* pShader);

	int  (*createRenderTarget)(xge_render_target_t* pRT, const xge_render_target_desc_t* pDesc);
	void (*destroyRenderTarget)(xge_render_target_t* pRT);

	void (*beginPass)(const xge_pass_t* pPass);
	void (*endPass)(void);
	void (*draw)(const xge_gpu_draw_call_t* pCall);
} xge_gpu_api_t;
```

第一版只需要一个 GL-family backend，内部根据平台选择 GL Core/GLES3/WebGL2 差异。

## 10. 渲染命令队列

公开 `xgeDraw*` 不直接调用 GL，而是写入命令队列。

命令类型：

```c
typedef enum xge_render_cmd_type_t {
    XGE_RENDER_CMD_DRAW_SPRITE,
    XGE_RENDER_CMD_DRAW_SHAPE,
    XGE_RENDER_CMD_DRAW_TEXT,
    XGE_RENDER_CMD_DRAW_MESH,
    XGE_RENDER_CMD_SET_PASS,
    XGE_RENDER_CMD_CUSTOM
} xge_render_cmd_type_t;
```

命令数据应避免频繁小分配。建议：

- 每帧 arena 分配命令。
- 每个线程一个 thread-local command chunk。
- `xgeFlush` 时合并到 render thread。

第一版可先实现简单锁保护队列，后续优化为多 producer queue。

提交路径：

```text
xgeDraw*
  -> validate
  -> copy command data into frame arena
  -> append command to current thread queue

xgeFlush
  -> merge queues
  -> sort/group commands if needed
  -> build batches
  -> execute GPU draw calls on render thread
```

排序原则：

- 默认保持提交顺序，避免意外改变 UI/游戏绘制结果。
- 可选启用 material/texture 排序，用于特定 pass。
- UI/layout 默认保持顺序。

## 11. 批处理策略

Sprite batch key：

```text
render target
shader/material
texture
blend mode
sampler
clip/scissor
vertex format
```

批处理顶点格式：

```c
typedef struct xge_sprite_vertex_t {
	float fX, fY, fZ;
	float fU, fV;
	unsigned int iColor;
} xge_sprite_vertex_t;
```

默认顶点颜色使用 `0xRRGGBBAA`，提交 GPU 前可转换为 shader 需要的格式。

Shape 可以共用简单 colored vertex batch。

Text 通过 glyph atlas 走 sprite batch。

## 12. Texture 与 Image

`XgeImage` 是 CPU 图像：

```c
typedef struct xge_image_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iStride;
	void* pPixels;
	unsigned int iFlags;
} xge_image_t, *xge_image;
```

`XgeTexture` 是 GPU 图像：

```c
typedef struct xge_texture_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iRefCount;
	unsigned int iFlags;
	unsigned int iBackendId;
	void* pBackend;
} xge_texture_t, *xge_texture;
```

加载路径：

```text
xgeTextureLoad(path)
  -> xrt file/resource read
  -> image decode to RGBA bytes
  -> optional premultiply alpha
  -> GPU upload on render thread
  -> free CPU pixels unless caller requested XgeImage
```

异步加载路径：

```text
worker thread:
  read + decode + premultiply

render thread:
  upload to GPU
  swap fallback with real texture
```

## 13. RenderTarget 与 Readback

RenderTarget：

```c
typedef struct xge_render_target_t {
	int iWidth;
	int iHeight;
	int iFormat;
	xge_texture_t objColor;
	unsigned int iBackendId;
	void* pBackend;
} xge_render_target_t, *xge_render_target;
```

Readback 是慢路径：

- screenshot
- tests
- tools
- offscreen output

不要让常规游戏逻辑依赖频繁 readback。

## 14. Material 与 Shader

Material 是普通用户入口：

```c
typedef struct xge_material_t {
	xge_shader_t* pShader;
	xge_texture arrTextures[8];
	unsigned int iColor;
	int iBlendMode;
	unsigned int iFlags;
	void* pUniforms;
} xge_material_t, *xge_material;
```

Shader 是高级接口：

```c
typedef struct xge_shader_desc_t {
	const char* sVertexSource;
	const char* sFragmentSource;
	const char* sLabel;
} xge_shader_desc_t;
```

Shader 版本策略：

- 内置 shader 使用模板生成 GL Core/GLES3/WebGL2 版本。
- 用户 shader 第一版要求用户提供目标平台可用源码。
- 后续可以提供简易 shader transpile/prelude。

## 15. 2.5D 实现路径

2.5D 使用高级 mesh API，不污染普通 2D API。

建议：

```c
typedef struct xge_vertex_t {
	float fX, fY, fZ;
	float fU, fV;
	unsigned int iColor;
} xge_vertex_t;

typedef struct xge_mesh_t {
	xge_vertex_t* pVertices;
	unsigned short* pIndices;
	int iVertexCount;
	int iIndexCount;
	xge_material pMaterial;
} xge_mesh_t, *xge_mesh;
```

第一版使用 16-bit indices。需要时再增加 32-bit index capability。

Depth test 默认关闭。高级 API 可打开：

```c
xgeRenderSetDepthTest(1);
```

2.5D camera：

- 默认 orthographic。
- optional perspective 只用于高级 pass。

## 16. Scene 实现

内部状态：

```c
typedef struct xge_scene_state_t {
	xge_scene arrStack[32];
	int iTop;
	xge_scene pPendingScene;
	int iPendingOp;
	int iUpdateMode;
	float fFixedStep;
	float fAccumulator;
} xge_scene_state_t;
```

场景切换不应在回调中立即修改栈，而是设置 pending op，在安全点处理。

安全点：

```text
frame start
after event dispatch
before update
after draw
```

推荐在 frame start 处理 pending scene op。

Scene stack 规则：

- push：当前 scene 调 `onPause`，新 scene 调 `onEnter`。
- pop：当前 scene 调 `onLeave` 和可选 `onFree`，上一个 scene 调 `onResume`。
- set：清空全部旧 scene，进入新 scene。
- replace：leave 当前 scene，enter 新 scene。

## 17. 输入实现

输入状态应同时支持事件和查询。

```c
typedef struct xge_input_state_t {
	unsigned char arrKeyDown[512];
	unsigned char arrKeyPressed[512];
	unsigned char arrKeyReleased[512];

	float fMouseX;
	float fMouseY;
	float fMouseDX;
	float fMouseDY;
	float fMouseWheel;
	unsigned int iMouseButtons;

	xge_touch_point_t arrTouches[XGE_TOUCH_MAX];
	int iTouchCount;

	xge_gamepad_state_t arrGamepads[4];
} xge_input_state_t;
```

每帧开始清空 transient state：

- keyPressed
- keyReleased
- mouseDx
- mouseDy
- mouseWheel
- touch phase transient flags

IME/text input：

- 以事件形式发送 UTF-8 文本。
- UI/RMGUI 消费文本事件。
- 游戏也可订阅文本输入事件。

## 18. 资源协议

资源加载统一经过 resolver：

```text
plain/path.png
  -> xrt exepath + relative path

res://ui/button.png
  -> registered provider
```

Provider 接口：

```c
typedef struct xge_resource_provider_t {
	const char* sScheme;
	int (*load)(const char* sURI, void** ppData, int* pSize, void* pUser);
	void (*free)(void* pData, void* pUser);
	void* pUser;
} xge_resource_provider_t;
```

第一版内置：

- file provider
- memory provider
- provider registration hook

xpack provider 由 xpack 集成层注册。

## 19. 异步资源状态机

状态：

```c
typedef enum xge_async_state_t {
    XGE_ASYNC_EMPTY,
    XGE_ASYNC_QUEUED,
    XGE_ASYNC_LOADING,
    XGE_ASYNC_DECODED,
    XGE_ASYNC_UPLOADING,
    XGE_ASYNC_READY,
    XGE_ASYNC_FAILED,
    XGE_ASYNC_CANCELLED
} xge_async_state_t;
```

请求对象：

```c
typedef struct xge_async_request_t {
	int iType;
	int iState;
	const char* sURI;
	void* pResult;
	void* pFallback;
	void (*procCallback)(struct xge_async_request_t* pReq, void* pUser);
	void* pUser;
} xge_async_request_t;
```

实现路径：

```text
main thread:
  create request, assign fallback, enqueue job

worker:
  load bytes, decode data

render thread:
  upload GPU resources

main thread:
  publish READY/FAILED and call callback
```

## 20. Audio 实现路径

使用 miniaudio。

内部对象：

```c
typedef struct xge_sound_t {
	int iRefCount;
	unsigned int iFlags;
	void* pMaSound;
} xge_sound_t, *xge_sound;

typedef struct xge_audio_group_t {
	float fVolume;
	void* pMaGroup;
} xge_audio_group_t, *xge_audio_group;
```

第一版文件格式策略需要单独确认。建议：

- WAV 必须支持。
- OGG 建议支持。
- MP3 视 miniaudio 配置和平台成本决定。

3D audio：

- `XgeAudioListener` 映射 miniaudio listener。
- `xgeSoundSetPosition` 设置声音位置。
- 2D 游戏默认 z=0。

Fade：

- 优先使用 miniaudio 支持的 fade/volume ramp。
- 若不足，在 XGE 层每帧更新音量。

## 21. Text 与 Font 实现路径

字体系统分三层：

```text
Font face
  -> TTF/XRF source

Glyph cache
  -> glyph metrics + atlas location

Text draw
  -> emits sprite commands
```

UTF-8 解码后得到 codepoint。第一版只承诺 UCS2 范围：

```text
0x0000 - 0xFFFF
```

Fallback font：

```text
primary font missing glyph
  -> fallback chain lookup
  -> missing glyph placeholder
```

Atlas：

- 第一版可以固定 atlas 尺寸，例如 1024x1024 或 2048x2048。
- atlas 满时创建新 page。
- 每个 page 是一张 `XgeTexture`。

Text draw：

```text
measure
layout simple lines
emit glyph quads
batch with atlas texture
```

## 22. XRF 格式实现路径

XRF 是字体缓存格式，不是复杂排版格式。

建议结构：

```text
Header
GlyphRange[]
GlyphRecord[]
AtlasPage[]
PixelData[]
```

Header 字段：

- magic
- version
- flags
- glyphCount
- pageCount
- lineHeight
- ascent
- descent
- atlasFormat

GlyphRecord：

- codepoint
- pageIndex
- x/y/w/h
- offsetX/offsetY
- advanceX

PixelData：

- A8 或 RGBA8。
- 压缩不在 XRF 内部处理，可由 xpack 外层处理。

TTF-to-XRF cache：

```text
load TTF
select glyph range
rasterize
pack atlas
write XRF
runtime load XRF fast path
```

## 23. XUI 桥接实现路径

> 2026-05-07 口径更新：本节保留 XUI 第一版桥接和孵化路线。当前基础层设计以 `XUI Widget V2基础设计.md` 与 `XUI Widget V2基础SPEC.md` 为准；剥离到独立仓库必须等 Widget V2 完成并重验后再执行。

布局和控件系统属于 XUI，不进入 XGE 内核。XGE 只提供 XUI 需要的宿主能力，XUI 通过 backend 接口接入。默认 backend 是 XGE，但 XUI core 不能依赖 XGE。

开发流程采用“XGE 内部孵化，成熟后剥离”的路线。原因是 GUI 系统高度依赖渲染、输入、字体、clip、app-mode 和调试反馈，一开始完全分仓实现会让调试成本过高。

孵化期目录建议：

```text
src/xge_xui_core.c       XUI context、生命周期、公共容器
src/xge_xui_event.c      XUI event queue、hit test、dispatch
src/xge_xui_layout.c     XUI layout
src/xge_xui_widget.c     XUI widget tree
src/xge_xui_paint.c      XUI paint commands -> XGE draw
src/xge_xui_controls.c   基础控件
```

孵化期约束：

- 文件和 API 使用 `xge_xui_` / `__xgeXui` 前缀，方便后续整体迁移和重命名。
- 不把 XUI 状态塞进 XGE core context，使用独立 `xge_xui_context_t`。
- XUI 孵化模块可以调用 XGE 内部绘制和输入能力，但要集中在 paint/backend 适配层。
- layout/widget/event 逻辑不能直接调用 GL，也不能直接依赖 sokol。
- 每完成一个 XUI 能力，需要补 XGE 侧测试；剥离到 XUI 仓库后同一批测试要迁移或复用。
- 剥离前先形成 XUI public API，再从 XGE 内部调用切换到 XUI backend 调用。

依赖方向：

```text
xrt
  -> xge
  -> xui_core

xge + xui_core
  -> xui_backend_xge
```

实际含义：

- `xge` 依赖 xrt。
- `xui_core` 依赖 xrt。
- `xui_backend_xge` 同时依赖 xui core 和 xge。
- `xge` 不能 include `xui.h`，避免 XGE 内核被 GUI 复杂度拖重。

上面的依赖方向是最终形态。孵化期允许 XGE 仓库内存在 XUI 源码，但这些源码必须保持可剥离边界。

XGE 提供给 XUI backend 的能力：

```c
typedef struct xge_xui_host_t {
	float (*getDelta)(void);
	int (*getSize)(int* pW, int* pH);
	int (*pollEvent)(xge_event_t* pEvent);

	int (*textureCreate)(xge_texture pTexture, int iW, int iH, const void* pPixels);
	void (*textureFree)(xge_texture pTexture);
	xge_vec2_t (*textMeasure)(xge_font pFont, const char* sText);

	void (*drawTexture)(const xge_draw_t* pDraw);
	void (*drawRect)(const xge_rect_t* pRect, uint32_t iColor);
	void (*setClip)(const xge_rect_t* pRect);
	void (*clearClip)(void);
} xge_xui_host_t;
```

上面是概念接口，最终实现时可以按当前 C API 风格调整命名和结构，但边界不变。

事件流：

```text
platform/sokol/miniprogram
  -> XGE input event normalization
  -> XUI backend event adapter
  -> XUI event queue
  -> hit test / capture / target / bubble
  -> consumed ? stop : return to game
```

渲染流：

```text
XGE frame/app-mode refresh
  -> xuiUpdate(delta)
  -> xuiLayoutFlush()
  -> xuiPaint()
  -> XGE draw batch / clip / texture / text
  -> XGE flush/present
```

XGE 第一版 bridge 范围：

- pointer/key/text 基础事件。
- 系统 IME text/composition 事件通道，不实现内建 IME。
- XUI 使用 XGE 字体测量和文本绘制。
- XUI 使用 XGE texture 和 draw primitive。
- 支持 UI 未消费事件返回给游戏逻辑。
- 支持 app-mode 手动刷新。

## 25. Mini Program 实现路径

小程序是第一版核心需求，但独立于 Sokol。

组成：

```text
xge_platform_miniprogram.h
xge_miniprogram.js
```

C 侧：

- 提供初始化 hook。
- 提供 WebGL2 context 接入。
- 接收 JS event。
- 请求 JS resource load。

JS 侧：

- 创建 Canvas/WebGL2。
- 转发 touch/input/lifecycle。
- 提供文件加载。
- 驱动 requestAnimationFrame/game loop。

需要谨慎处理：

- WASM 是否可用。
- 多线程是否可用。
- miniaudio 是否能直接工作。
- 平台音频是否需要 JS bridge。
- 文件系统是否需要缓存层。

第一版小程序目标：

- WebGL2 Canvas 渲染。
- Touch input。
- 基础音频策略。
- 资源加载。
- game loop。

## 26. Offscreen 与 Board Linux 实现路径

Offscreen：

- 优先 EGL surfaceless。
- 不支持时使用 EGL pbuffer。
- 所有绘制进入 RenderTarget。
- readback 输出 RGBA bytes。

Board Linux：

- 第一阶段实现 EGL context。
- 显示输出后续按目标系统选择 DRM/KMS/GBM、Wayland 或 X11/EGL。
- 输入按目标系统单独设计，不强塞进第一阶段。

## 27. Debug 实现路径

Debug stats：

```c
typedef struct xge_debug_stats_t {
	int iDrawCalls;
	int iBatches;
	int iVertices;
	int iIndices;
	int iTextureBinds;
	int iShaderBinds;
	int iRenderTargetSwitches;
	float fFrameMS;
	float fUpdateMS;
	float fDrawMS;
} xge_debug_stats_t;
```

Debug 模式执行：

- GL error check。
- shader compile log。
- GPU caps dump。
- frame stats collect。
- resource count collect。

Release 模式默认编译掉或关闭这些逻辑。

## 28. 测试策略

测试框架使用纯 C 实现，不引入复杂外部测试依赖。

测试方式：

```text
1. Codex 编写测试/示例/冒烟检查指令
2. 用户在本机执行需要人工观察或交互的步骤
3. 用户反馈结果
4. Codex 根据反馈修正代码或测试
5. 循环直到通过
```

阶段策略：

- 前期以开发和冒烟测试为主。
- GPU/窗口/音频/小程序等依赖真实环境的功能采用人机协作测试。
- 稳定后再集中补齐系统化测试。
- 能离屏运行的逻辑测试优先自动化。

测试目录：

```text
test/
  test_main.c
  test_core.c
  test_math.c
  test_layout.c
  test_resource.c
```

## 29. 文档与偏离处理规则

每完成一个功能，必须更新 `docs/XGE_V2_SPEC.md`。

如果实现需要偏离已确认设计，必须先发送警报，再修改文档，再修改代码。

警报规则：

```text
1. 在发现偏离时立即发送明确警报。
2. 如果用户后续发送“继续任务”这类提前编排命令，仍需在接下来的多个回复中持续提醒该偏离。
3. 未获得用户明确认可前，不直接按偏离方案实现。
4. 用户认可后，先更新 XGE_V2_DESIGN.md / XGE_V2_TECH_PLAN.md / XGE_V2_SPEC.md。
5. 文档更新完成后再改代码。
```

警报格式：

```text
警报：当前实现需要偏离已确认设计。
偏离点：...
原因：...
建议处理：...
等待确认后再继续。
```

## 30. 实现顺序建议

推荐开发顺序：

```text
1. 项目结构和单头文件骨架
2. XgeDesc / error code / core lifecycle
3. xrt allocator/file/time 接入
4. Sokol window + GL context
5. GL backend 最小能力：clear/present/shader/buffer/texture
6. render command queue
7. sprite batch + xgeDraw
8. image load + texture load
9. scene lifecycle
10. input keyboard/mouse/touch
11. render target/pass
12. shape draw
13. text/stb_truetype/glyph atlas
14. miniaudio basic sound/music
15. XUI incubation baseline
16. async resources/fallback
17. material/shader advanced API
18. 2.5D mesh API
19. offscreen/EGL
20. mini program backend
21. XRF format
22. XUI bridge / extraction
```

每完成一个阶段，应同步更新：

- `XGE_V2_SPEC.md`
- 示例
- 测试
- 必要时更新 `XGE_V2_DESIGN.md`

## 31. 关键风险

需要持续关注：

- WebGL2 与 GLES3/GLCore shader 差异。
- 小程序平台限制。
- OpenGL 多线程上下文误用。
- 单头文件体积膨胀。
- XUI 孵化代码边界失控，难以剥离。
- 字体 atlas 内存占用。
- premultiplied alpha 资源管线一致性。
- release 模式中 debug 逻辑未完全关闭。
- xrt logger/allocator/thread 能力是否按 XGE 需求及时补齐。

## 32. 固定结论

以下结论除非明确重新讨论，否则不应改变：

- XGE V2 不兼容 XGE V1。
- C API only。
- 完全依赖 xrt。
- 单头文件交付。
- 渲染基线 GLES3/WebGL2/GL3 Core。
- Sokol 是默认窗口后端，但不是唯一后端。
- 小程序是第一版核心目标。
- miniaudio 是音频后端。
- stb_truetype 是 TTF 后端。
- XRF 是快速 bitmap font/cache 格式。
- 布局系统属于 XUI；XGE 第一版只负责 XUI 孵化和默认桥接。
- CPU bitmap blend 不回归。
- 2.5D 是独立高级 API。
- 目录结构采用根目录 `xge.h/xge.c`，`src/` 放引擎代码，`lib/` 放三方库，`test/` 放测试，`singlehead/` 放单头文件版本与生成器。
- 构建脚本命名带目标后缀，例如 `build_dll.bat`、`examples/mvp/build.sh`。
- 三方依赖直接复制到 `lib/`。
- 代码风格遵守 `docs/代码风格规则.md`。
- 类型命名使用 `xge_texture_t` 形式。
- 内部函数使用 `__xge` 前缀。
- 实现偏离设计时必须先警报、再更新文档、最后改代码。
