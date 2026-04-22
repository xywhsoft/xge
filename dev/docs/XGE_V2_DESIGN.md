# XGE V2 设计文档

本文档记录 XGE V2 的基础设计方案。

XGE V2 不是 XGE V1 的 1:1 移植。XGE V1 诞生于 DirectDraw 时代，很多功能依赖 CPU bitmap 操作。XGE V2 保留 V1 中有价值的开发目标和使用体验，但基于现代 GPU 渲染、xrt 基础设施和轻量跨平台 C API 重新设计。

## 目标

- 构建一个小巧、灵活、现代化的 2D 游戏引擎。
- 使用 GPU 加速渲染、混合、文字、形状、render target 和图像特效。
- 只提供 C API。
- 公开 API 统一使用 `xge` 前缀。
- 完全依赖 xrt，使用 xrt 提供 allocator、文件系统、平台工具、线程，以及后续 logger。
- xpack 不进入 XGE 内核。XGE 可以通过 VFS/protocol 层从 xpack 加载资源。
- 支持游戏开发，也通过 XUI 默认桥接实现 APP 风格 RMGUI 渲染。
- 引擎定位 2D 优先，同时通过独立高级 API 暴露 2.5D 能力。

## 非目标

- 不兼容 XGE V1 二进制/API。
- 不提供 V1 风格 wrapper。
- 不提供 GDI/GDI+ 后端。
- 不在 XGE 内部实现 IOCP/网络模块。网络属于 xrt。
- 不在 XGE 内部实现 xpack。
- 不保留旧 CPU bitmap blend 管线，例如 AND/OR/XOR/PSET 等 bitmap 操作。
- 不实现完整 3D 引擎能力，例如模型加载、PBR、3D 物理或 3D 场景管理。

## 平台基线

渲染能力基线：

- OpenGL ES 3.0
- WebGL 2.0
- Desktop OpenGL 3.3 Core Profile

macOS/iOS 是最低优先级兼容目标，第一版暂时只走 OpenGL/OpenGL ES。Metal 可以后续重新评估，但不属于第一版基线。

| 目标平台 | 窗口/上下文方案 | 图形 API |
| --- | --- | --- |
| Windows | Sokol | OpenGL 3.3 Core |
| Linux 桌面 | Sokol，支持 X11 与 Wayland | OpenGL 3.3 Core 或 EGL/GLES3 |
| macOS | Sokol | OpenGL Core，低优先级 |
| Android | Sokol | OpenGL ES 3.0+ |
| iOS | Sokol | OpenGL ES 3.0，低优先级 |
| 板卡 Linux | 自研 EGL 后端 | OpenGL ES 3.0+ |
| Web | Sokol/Emscripten | WebGL 2.0 |
| 小程序 | 独立小程序后端 | WebGL 2.0 Canvas |
| 离屏渲染 | EGL pbuffer/surfaceless | OpenGL ES 3.0+ |

板卡 Linux 可以假设为 RK3566 这类近几年较新的板卡。

## 项目形态

- 根目录保留 `xge.h` 和 `xge.c`。
- `xge.h` 只保留公开宏、类型和 API 声明，不直接承载实现代码。
- `xge.c` 是常规构建的聚合入口，包含 `src/` 内的模块实现。
- `src/` 放引擎模块源码，按功能拆分，例如 core、resource、audio、font、render、input。
- `lib/` 放第三方库，包括 xrt、sokol、miniaudio、stb_truetype。
- `test/` 放测试代码。
- `singlehead/` 放单头文件版本、生成器和单头文件测试代码。
- `docs/` 放设计、规格和技术文档。
- `examples/` 放示例。
- 源码交付形态：仓库源码按模块维护，单头文件版本由 `singlehead/` 生成。
- 默认集成方式：常规开发使用 `xge.h + xge.c`；发布集成可使用 `singlehead/` 生成的单头文件。
- 只使用构建脚本，不要求 CMake。
- 构建脚本命名带构建目标，例如：
  - `build_exe.bat`
  - `build_exe.sh`
  - `build_dll.bat`
  - `build_dll.sh`
- 构建脚本提供：
  - demo/test EXE 输出
  - 可选 DLL 输出，用于测试、工具或语言绑定
- 不输出 static library。
- 第三方库放在 `lib/` 下。
- 第三方依赖直接复制到 `lib/`，不使用 submodule 作为默认方案。
- 当前计划的第三方依赖：
  - xrt
  - sokol
  - stb_image
  - miniaudio
  - stb_truetype

## API 风格

XGE 使用 C 风格 API，并尽量把模块名放在函数名前部。

示例：

```c
int  xgeInit(const XgeDesc* desc);
void xgeUnit(void);

int  xgeTextureLoad(XgeTexture* texture, const char* path);
void xgeTextureFree(XgeTexture* texture);

void xgeSoundPlay(XgeSound* sound);
void xgeSoundStop(XgeSound* sound);

void xgeScenePush(XgeScene* scene);
void xgeScenePop(void);
```

规则：

- 公开函数使用 `xge` 前缀。
- 属性访问优先使用 `Get` 和 `Set`：例如 `xgeGetWidth`、`xgeCameraSetView`。
- 资源释放使用 `xgeXxxFree`，例如 `xgeTextureFree`。
- 错误返回使用 `int` 枚举码。
- 数据结构尽可能透明。
- 后端/私有状态放入明确标记的 internal 字段。

类型命名使用 snake_case，例如 `xge_texture_t`。公开 API 保持 `xgeTextureLoad` 这种 C 函数命名。内部函数使用两条前导下划线，例如 `__xgeTextureUpload`。

透明对象模式：

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

用户可以读取 `iWidth`、`iHeight`、`iFormat`、`iFlags` 等公开字段。`iBackendId` 和 `pBackend` 属于内部字段，用户代码不能修改或依赖它们。

代码风格必须遵守 `docs/代码风格规则.md`：

- Tab 缩进。
- CRLF 换行。
- 注释使用中文。
- 变量采用 xrt 风格匈牙利前缀。
- 函数定义左花括号另起一行。
- `if`、`for`、`while` 的括号内外按 xrt 风格添加空格。

## 内存

- XGE 的所有内存分配都走 xrt allocator。
- 不支持用户传入 allocator。
- 资源对象支持引用计数。
- 允许用户手动管理对象生命周期。
- 第一版不提供全局资源缓存。

## 资源加载

XGE 支持从以下来源加载资源：

- 文件
- 内存
- 通过 xpack 或其他 VFS 来源加载压缩包资源

路径规则：

- 相对路径基于 xrt 的可执行文件根目录。
- `res://` 是用于 package/VFS 资源的协议。
- xpack 只是 `res://` 的一种 provider，不属于 XGE 内核。

资源对象类型独立：

- `XgeTexture`
- `XgeImage`
- `XgeFont`
- `XgeSound`
- `XgeMusic`
- `XgeStream`

第一版不计划提供统一 asset handle。

## 渲染模型

默认游戏渲染采用 immediate-style 命令提交：

- `xgeDraw*` 将绘制命令写入队列。
- 命令会被批处理。
- draw 阶段结束时统一 flush。

同时支持手动渲染，用于 APP/RMGUI 场景：

```c
xgeBegin();
xgeDraw(...);
xgeFlush();
xgeEnd();
```

场景 draw 回调默认自动包裹 begin/end，因此普通游戏用户通常不需要手动调用它们。

XGE 暴露：

- RenderTarget
- RenderPass
- Viewport
- Camera
- Material
- Shader
- Pipeline 级高级 API

RenderPass 应保持轻量，不做成 Vulkan 式复杂结构：

```c
typedef struct XgePass {
    XgeRenderTarget* target;
    XgeRect viewport;
    unsigned int clearColor;
    int clearFlags;
    XgeCamera* camera;
} XgePass;
```

## 渲染 API

第一版渲染能力：

- Clear 和 present。
- Texture 加载与绘制。
- Sprite 绘制。
- Source/destination rect 绘制。
- Color modulation。
- Alpha 控制。
- Flip/mirror。
- Rotation 和 scale。
- RenderTarget/FBO。
- Screenshot/readback 慢路径。
- Shape 绘制。
- Text 绘制。
- 自定义 material/shader。
- 高级 mesh/2.5D API。

CPU bitmap 操作不是渲染模型。CPU 像素只通过 `XgeImage` 服务于加载、处理、工具和 readback 路径。

## 坐标

XGE 同时支持逻辑坐标和像素坐标。

- 核心内部渲染使用 float 逻辑坐标。
- 像素 API 是 int 参数包装。
- 默认逻辑坐标等于屏幕像素坐标。
- 默认 Y 轴向下。
- 支持其他坐标系，包括居中原点。
- Camera 与 viewport 相互独立，但默认重合。

建议 API 区分：

```c
void xgeDrawRect(float x, float y, float w, float h, unsigned int color);
void xgeDrawRectPx(int x, int y, int w, int h, unsigned int color);
```

`xgeDrawRect` 使用 world/logical 坐标。`xgeDrawRectPx` 使用屏幕像素坐标，并绕过 camera transform。

## 颜色与 Alpha

- 立即颜色值使用 `0xRRGGBBAA`。
- 图像像素内存使用 RGBA byte 顺序。
- 默认纹理格式是 RGBA8。
- 默认 alpha 模型是 premultiplied alpha。
- 纹理加载可以选择 premultiplied 或 straight alpha。

Premultiplied alpha blend 使用：

```text
src = ONE
dst = ONE_MINUS_SRC_ALPHA
```

第一版 blend modes：

```c
typedef enum XgeBlendMode {
    XGE_BLEND_NONE,
    XGE_BLEND_ALPHA,
    XGE_BLEND_ADD,
    XGE_BLEND_MULTIPLY,
    XGE_BLEND_SCREEN,
    XGE_BLEND_CUSTOM
} XgeBlendMode;
```

后续可选模式：

- subtract
- lighten
- darken

V1 的 AND、OR、XOR、PSET 等 bitmap blend mode 不保留。

## Image 与 CPU 像素

CPU-side pixels 通过 `XgeImage` 支持。

- `XgeImage` 拥有 CPU pixel data。
- `XgeTexture` 拥有 GPU texture data。
- `XgeTexture` 默认不保留 CPU 副本。
- RenderTarget/Texture readback 作为慢路径支持。

建议 API：

```c
int   xgeImageLoad(XgeImage* image, const char* path);
void* xgeImageGetPixels(XgeImage* image);
int   xgeTextureCreateFromImage(XgeTexture* texture, const XgeImage* image);
int   xgeTextureReadPixels(XgeTexture* texture, XgeImage* outImage);
```

## 2.5D

XGE 仍然是 2D 引擎，但高级渲染 API 可以暴露 2.5D 能力。

支持的 2.5D 范围：

- 自定义 vertex/index mesh
- z/order
- orthographic camera
- 可选 perspective camera
- depth test 开关
- 图像透视/扭曲效果
- isometric/depth-sorted 场景
- sprite/card/UI 3D transform
- 粒子深度

不作为核心目标：

- 3D 模型资源管线
- 3D scene graph
- PBR/lighting system
- 3D physics

## 场景系统

XGE 使用现代 lifecycle scene model，不使用 V1 的消息回调模型。

场景支持：

- user data
- enter/leave
- pause/resume
- event
- update
- draw
- free

建议结构：

```c
typedef struct XgeScene {
    void* user;
    int (*onEnter)(struct XgeScene* scene);
    int (*onLeave)(struct XgeScene* scene);
    int (*onPause)(struct XgeScene* scene);
    int (*onResume)(struct XgeScene* scene);
    int (*onEvent)(struct XgeScene* scene, const XgeEvent* event);
    int (*onUpdate)(struct XgeScene* scene, float dt);
    int (*onDraw)(struct XgeScene* scene);
    int (*onFree)(struct XgeScene* scene);
} XgeScene;
```

场景栈语义：

- `xgeSceneSet`：清空栈并进入新场景。
- `xgeScenePush`：暂停当前场景并进入子场景。
- `xgeScenePop`：离开当前场景并恢复上一个场景。
- `xgeSceneReplace`：替换当前栈顶场景。

Update 和 draw 分离。允许多种 update/draw 策略：

- 一次 update 对应一次 draw
- fixed update + variable draw
- 多次 update 对应一次 draw
- manual app-mode rendering

## 运行模式

XGE 支持游戏循环模式和手动模式。

```c
typedef enum XgeRunMode {
    XGE_RUN_GAME_LOOP,
    XGE_RUN_MANUAL
} XgeRunMode;
```

游戏循环模式自动驱动事件轮询、update、draw 和 present。

手动模式用于 APP/RMGUI 工作流，允许显式控制 frame/render/present。

建议 API：

```c
int  xgeFrame(void);
void xgeRender(void);
void xgePresent(void);
void xgeInvalidateRect(const XgeRect* rect);
```

## 输入

第一版支持：

- keyboard
- mouse
- touch
- gamepad
- text input
- IME

Key code 映射 Sokol key code。

多点触控设计：

```c
#define XGE_TOUCH_MAX 16

typedef struct XgeTouchPoint {
    unsigned long long id;
    int phase;
    float x;
    float y;
    float dx;
    float dy;
    float pressure;
    double time;
} XgeTouchPoint;

typedef struct XgeTouchEvent {
    int count;
    XgeTouchPoint points[XGE_TOUCH_MAX];
} XgeTouchEvent;
```

Touch phase：

- begin
- move
- stationary
- end
- cancel

建议 API：

```c
int xgeTouchGetCount(void);
const XgeTouchPoint* xgeTouchGet(int index);
const XgeTouchPoint* xgeTouchFind(unsigned long long id);
```

## 线程

XGE API 在可行范围内设计为支持多线程调用。

OpenGL 约束：

- 实际 GL 调用在 render thread/context owner 上执行。
- Worker threads 只提交 command 和资源上传请求。

规则：

- `xgeDraw*` 可以从多个线程调用，并写入 command queue。
- 资源加载/解码可以在 worker threads 执行。
- GPU upload 排队到 render thread。
- `xgeFlush`、`xgePresent` 和 context 操作在 render thread 执行。

## 异步资源

资源加载支持异步工作流。

- 后台线程解码 image/font/audio data。
- GPU resource creation 排队执行。
- 支持微端/流式场景所需 fallback resources。
- 用户可以在真实资源加载完成前显示 fallback texture/font/sound。

## 音频

音频后端：miniaudio。

原因：

- single-file friendly
- C API
- 许可证宽松
- 支持 decoding、streaming、resource management、engine-level playback 和 3D spatialization
- 比 C++ 音频引擎更适合 XGE 的轻量设计

XGE 音频概念：

- `XgeSound`：短音效
- `XgeMusic`：音乐
- `XgeStream`：流式音频
- `XgeAudioGroup`：音量/混音组
- `XgeAudioListener`：3D audio listener transform

第一版音频能力：

- 真实音频播放
- 区分 sound/music/stream
- mixer/groups
- group volume
- loop
- fade
- 3D coordinate playback

## 文本与字体

字体栅格化后端：stb_truetype。

文本编码：

- 公开 API 只使用 UTF-8。
- 支持中文 UCS2 范围。
- 第一版不支持 emoji。
- 支持 fallback font chain。

第一版文本能力：

- font load
- glyph atlas
- glyph metrics
- text measurement
- 单行绘制
- 多行绘制
- 矩形内绘制
- alignment
- clipping/scissor support
- 给 layout/UI 使用的 line break hooks

复杂富文本不是第一版 text renderer 目标，可以基于 layout/RMGUI 系统构建。

## XRF 字体格式

XRF 重新设计为快速 bitmap font/cache 格式。

用途：

- 快速加载
- 快速渲染
- 预烘焙 UCS2 中文位图字体
- TTF 渲染缓存后端
- 适合板卡 Linux、小程序和较低性能设备

第一版 XRF 能力：

- UTF-8 输入映射到 glyph。
- UCS2 glyph ranges。
- 多 atlas pages。
- A8 或 RGBA8 atlas。
- glyph metrics。
- ascent/descent/line height。
- 可选 kerning。
- 不内建压缩要求；压缩属于 xpack/xrt 资源层。

## XUI 与 RMGUI

布局系统和控件系统归属 XUI，不进入 XGE 内核。游戏通常只有 GUI 需要布局，XGE 内核应保持轻量，专注窗口、输入、资源、渲染、字体、音频、RenderTarget、离屏和平台后端。

XGE 与 XUI 的最终关系：

- XGE 不依赖 XUI。
- XUI core 依赖 xrt，不直接依赖 XGE。
- XUI backend for XGE 依赖 XUI 和 XGE，是官方默认接入层。
- XGE 向 XUI 提供时间、输入事件、资源加载、字体测量、纹理和绘制能力。
- XUI 负责 retained tree、布局、控件状态、事件消费、响应逻辑，并把最终绘制命令提交到 XGE。
- XUI 也可以接入其他框架，只要实现对应 backend 接口。

开发期允许采用“XGE 内部孵化 XUI”的临时流程：

- 初期可以在 XGE 仓库内实现 `xge_xui_*` 模块，用于快速调试布局、控件、事件和渲染。
- 孵化代码必须从第一天按 XUI core/backend 边界书写，避免直接散落进 XGE core。
- 孵化阶段允许直接调用 XGE 内部高性能能力，减少空中搭桥成本。
- API 和测试稳定后，将 XUI core/layout/widget/event/paint 迁移到 XUI 仓库。
- 迁移后 XGE 中只保留 `xui_backend_xge` 或等价桥接层。
- 剥离前必须跑通 XGE 侧和 XUI 侧回归测试。

这个流程是开发效率策略，不改变最终依赖边界。最终发布形态仍然是 XGE 与 XUI 两个配合良好的库。

XGE 提供给 XUI 的底层能力：

- pointer/key/text/IME 基础事件。
- frame delta 和 lifecycle。
- texture/image/font/text measure。
- draw primitives、clip/scissor、RenderTarget。
- 手动刷新和 dirty rect 需要的 app-mode frame 控制。

XUI 提供：

- layout tree。
- widget tree。
- style/theme。
- focus/navigation。
- pointer/key/text event dispatch。
- dirty layout 和 dirty paint。
- DUI/RMGUI 控件库。

第一版 XGE 只实现 XUI bridge 所需的基础接口和示例，不在 XGE 内核实现完整布局和控件库。

## Material 与 Shader

暴露两层接口：

- Material API 面向常规用户。
- Shader/Pipeline API 面向高级用户。

Material 负责：

- textures
- color
- blend mode
- uniforms
- effect parameters

Shader/Pipeline 负责：

- 自定义 GLSL 源码或生成 variant。
- vertex layout。
- render state。
- custom mesh draw。

Gray、shade、distortion 等效果应作为 shader/material effect 实现，而不是 bitmap blend mode。

## Debug 与诊断

Debug 模式：

- xrt logger 可用后接入日志。
- 检查 GL errors。
- 提供 GPU caps dump。
- 暴露 frame stats。
- 暴露 draw call count。
- 暴露 batch count。

Release 模式：

- 默认关闭诊断。
- GPU caps dump 和 frame stats 不属于常规 runtime API。

建议 debug API：

```c
void xgeDebugDumpCaps(void);
int  xgeDebugGetStats(XgeDebugStats* outStats);
```

## 第一版功能集

第一版应实现：

- single-header XGE core
- xrt integration
- Sokol platform backend
- EGL/offscreen platform backend baseline
- EGL 编译期能力报告，用于明确当前构建是否真正启用 pbuffer/surfaceless/native window 路径
- mini program backend baseline
- GLES3/WebGL2/GL3 render backend
- command batching
- texture/image loading
- sprite drawing
- shape drawing
- text drawing
- render target/pass
- material/shader advanced entry point
- input: keyboard/mouse/touch/gamepad/text/IME
- scene lifecycle and scene stack
- miniaudio backend
- sound/music/stream/group/fade/3D audio
- XUI incubation and default bridge baseline
- XRF bitmap font format baseline
- async resource loading
- fallback resources
- debug diagnostics

## 当前实现状态

截至当前开发阶段，第一阶段 MVP 的核心代码、测试和示例已经基本完成：

- 核心生命周期、manual frame、game loop、scene stack、app-mode refresh 已具备自动测试。
- 资源、纹理、render target、command queue、shape、sprite、shader/material、2.5D、async fallback 已具备自动测试或示例。
- 文本、TTF、XRF、中文文本、XRF cache 已具备自动测试。
- 音频 API、group、fallback、listener、3D/fade 接口已具备 API 级测试和示例。
- XUI 已独立成 XUI 仓库主线，XGE 侧保留孵化代码和 bridge 示例作为真实接入参考。
- 小程序 hello scaffold、JS bridge、浏览器调试入口和构建复制脚本已具备。
- Sokol 后端已提供编译目标能力报告，可诊断 Windows、Linux X11、Linux Wayland、macOS、Android、iOS、Web/Emscripten 目标。
- Sokol 后端已补充编译图形后端能力报告，可诊断当前二进制使用 GLCore、GLES3、D3D11、Metal 还是 dummy backend。
- 平台后端已补充运行态快照 API，可在冒烟示例中输出窗口尺寸、framebuffer 尺寸、DPI scale 和输入/resize/quit 事件计数。
- Platform smoke 已支持按 frame/seconds 自动退出，并提供日志检查脚本用于目标机结果采集。
- 已补充 platform smoke 一键验证脚本，把构建、自动退出运行和日志检查串联到同一流程。
- 当前 vendored Sokol 的 Linux 桌面实现基于 X11，不包含原生 Wayland 后端；Wayland 需要升级平台库或独立 Wayland/EGL 后端。
- macOS 已补充 Sokol + OpenGL MVP 构建脚本；Android 已补充 NativeActivity/CMake scaffold，并修正 Android 不支持 `SOKOL_NO_ENTRY` 的入口差异。
- iOS 已补充 Sokol + OpenGL ES 3 simulator app scaffold，并与 Android 一样走 Sokol `sokol_main` 入口路径，不由 XGE 主动调用 `sapp_run`。
- Android 已补充 NDK Windows/Unix 构建脚本、Gradle debug APK scaffold 和 adb 安装启动脚本，便于从 native library 验证推进到设备安装验证。
- iOS 已补充 simulator 安装启动脚本，便于在 macOS/Xcode 环境中完成 app bundle 冒烟。
- Web/Emscripten 已补充 WebGL2 构建脚本、serve 脚本和 `platform/web/shell.html` 浏览器 shell，当前仍需 emsdk 和浏览器 WebGL2 冒烟验证。
- 已补充通用 `examples/platform_smoke` 示例，用于平台冒烟时输出 `xgeDebugDumpCaps` 并绘制基础 shape/texture；Web、macOS、Linux X11/EGL 脚本已切换到该示例。
- 已补充 `check_platform_backends.*` 工具链检查脚本，用于在目标机器上快速确认 gcc/clang/cmake/emcc/xcrun/NDK 等基础条件。
- 已补充 `docs/平台后端验证结果.md`，后续所有平台冒烟结果都必须落盘，作为关闭平台 spec 项的依据。
- 已补充 `check_platform_results.*`，用于读取验证结果并输出哪些平台 spec 项可关闭、哪些仍被阻塞。
- 已补充 `record_platform_result.*`，用于在目标平台执行后把状态、环境、脚本、摘要和日志追加到验证结果文档。
- 已补充 `check_platform_scaffold.*`，用于在没有目标 SDK 的机器上检查 Android/iOS/Web/Linux/macOS scaffold 文件和关键配置是否完整。
- 已补充 `check_platform_all.*`，用于把 scaffold、工具链和验证结果状态检查串成一个平台准备检查入口。
- EGL/offscreen/board Linux 已具备能力报告、初始化失败阶段诊断、pbuffer 初始化路径、CPU fallback 标记、pbuffer smoke 示例和构建脚本，但真实板卡和 Linux EGL 路径仍需要目标环境人工验证。
- 板卡 Linux 已补充 `check_board_linux_egl.sh` 一键冒烟脚本，用于目标设备上顺序验证 pbuffer 和 surfaceless EGL 示例。
- EGL surfaceless 已补充独立冒烟示例和构建脚本，目标设备仍需暴露 `EGL_KHR_surfaceless_context` 后才能关闭 surfaceless 任务。
- 已补齐 `docs/人工冒烟测试流程.md`，后续人工验证按该文档逐项反馈，再关闭 spec 中的人工冒烟测试项。

仍未收口的部分主要受外部条件限制：

- Linux X11/Wayland、macOS、Android、iOS、Web/Emscripten 的 Sokol 启动路径需要对应平台构建和运行环境。
- Web/Emscripten 已补充 `build_web_exe.*` 和 WebGL proc loader 分支，但仍需要 Emscripten 工具链与浏览器 WebGL2 冒烟验证。
- Sokol 非 Windows 平台的实机验证步骤已经固化到 `docs/Sokol跨平台冒烟测试计划.md`。
- Sokol 后端实际覆盖边界已经固化到 `docs/Sokol后端边界说明.md`。
- EGL surfaceless、板卡 Linux EGL、离屏 EGL init 需要目标驱动和设备验证。
- Render thread 命令执行已实现无活动 GL context 时的 worker drain 路径、能力报告，以及 EGL/offscreen context owner 配置路径；EGL 配置成功后 worker 可在线程内创建 GLES3 context 并同步 flush。
- 专用渲染线程实现边界已经固化到 `docs/专用渲染线程设计.md`，后续需要在真实 EGL/offscreen/板卡环境验证 context owner 模型，并继续把 GPU resource create/upload/destroy/present 完整迁移到 render queue，再评估 Sokol 窗口后端。
- xrt logger 已接入，XGE 通过 `xgeLogSetLevel`、`xgeLogWrite`、`xgeLogFlush` 和内部 `__xgeLog*` 适配层记录 core/platform/graphics/resource/audio 关键失败路径；详细边界见 `docs/日志接入设计.md`。
- 人工冒烟测试需要用户在真实窗口、输入法、音频设备、小程序/板卡环境中执行并反馈。

## 待设计项

以下细节在实现前还需要单独设计：

- `XgeDesc` 初始化结构。
- 错误码枚举。
- 公开结构体字段边界。
- 具体 render command queue 模型。
- 小程序 bridge API。
- XUI 孵化期结构、API 和剥离边界。
- XRF binary layout。
- miniaudio 文件格式支持策略。
- async resource state machine。
- shader variant 生成策略。

## 开发过程规则

- 每完成一个功能，必须更新 `docs/XGE_V2_SPEC.md`。
- 如果实现需要偏离已确认设计，必须先向用户发送警报。
- 偏离警报需要在后续多个回复中持续提醒，防止用户提前编排“继续任务”时忽略偏离。
- 用户明确认可偏离后，先更新 `XGE_V2_DESIGN.md`、`XGE_V2_TECH_PLAN.md` 或 `XGE_V2_SPEC.md`，再修改代码。
- 前期测试采用人机协作：Codex 给出操作指令，用户执行并反馈结果，再循环修正。
