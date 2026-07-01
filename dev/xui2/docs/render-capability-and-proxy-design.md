# XUI Render Capability and Proxy Design

本文记录 XUI 渲染代理层的边界。XUI 的定位是 UI runtime：管理 widget、layout、style、event、cache 和渲染输出，但不拥有窗口、不读取系统 DPI、不管理平台消息循环，也不负责调度系统刷新。

当前 XUI 第一版的 `host` 曾经和 XGE 能力混用，后期才逐渐拆出。XUI 从第一版开始应把代理层固化为最小必需依赖层，避免后续返工。

## 1. Design Position

XUI Proxy 不是 `PlatformHost`。更准确的定义是：

```text
XUI Proxy = offscreen rendering abstraction + mandatory system services
```

XUI 不做：

- 不创建窗口。
- 不查询系统 DPI。
- 不接管消息循环。
- 不从平台读取鼠标、键盘、触摸或 IME 消息。
- 不通过 proxy 请求系统刷新。
- 不从 proxy 读取时间。

XUI 依赖：

- `xrt`: 基础库，提供时间等通用能力，是 XUI/XGE 的必选依赖。
- `XuiProxy`: 渲染到 surface 的能力，以及剪贴板、IME 这类没有 proxy 就无法落地的系统服务。
- 外部程序：负责窗口、系统消息、DPI 变化、刷新时机、最终 present。

外部程序可以是 XGE，也可以是 Win32/GDI、OpenGL、Sokol、Direct2D、Web、软件光栅器或其他后端。XUI 只要求它们实现同一套 Proxy 约定。

## 2. Ownership Boundary

| Capability | XUI Owner | Notes |
| --- | --- | --- |
| Widget tree/layout/style/event/cache | XUI core | UI runtime 内部职责 |
| Virtual DPI / scale | XUI core input | 外部程序调用 XUI API 设置，不由 proxy 查询 |
| Viewport/root size | XUI core input | 外部程序调用 XUI API 设置 |
| Mouse/keyboard/touch input | XUI input API | 外部程序翻译后主动传入 |
| IME composition input | XUI input API | 平台 IME 消息由外部程序传给 XUI |
| Refresh / repaint call | XUI render API | 外部程序决定何时调用 XUI 渲染 |
| Time | xrt | 不进 proxy |
| Clipboard | XuiProxy system service | 文本剪贴板为第一版必需能力 |
| IME enable/disable | XuiProxy system service | XUI 根据焦点编辑状态控制 IME 开关 |
| Surface / drawing / text / font | XuiProxy render service | 只渲染到 surface |
| Window present / swapchain | External backend/app | XUI 输出到目标 surface，最终上屏由外部负责 |

这里最关键的边界是：输入数据进入 XUI，不等于由 Proxy 负责输入。Proxy 只做 XUI 必须依赖、没有它就无法工作的事情。

## 3. Current Host Inventory

当前 `xge_xui_host_v2_t` 混合了多类能力。XUI 应拆分如下：

| Current Host Capability | XUI Direction |
| --- | --- |
| draw rect/text/texture/shape | Proxy render service |
| clear rect / damage reject | XUI painter + proxy local clear | proxy 不暴露浏览器式 clip stack |
| texture create/load/destroy | Unified surface resource |
| font create/metrics/destroy | Proxy font/text service |
| clipboard | Proxy system service |
| time | xrt |
| viewport size | XUI viewport input API |
| DPI scale | XUI virtual DPI input API |
| request_refresh | XUI dirty query/render API，外部程序自行调度 |
| flush/present | 外部后端职责，不是 XUI Proxy 必需能力 |

这意味着 XUI 不再设计 `PlatformHost`。如果某个集成层需要同时管理窗口、消息、DPI 和 present，可以在 XUI 外部实现自己的 platform adapter，但它不是 XUI core contract 的一部分。

## 4. Proxy Required Capabilities

### 4.1 System Services

第一版必需：

- Clipboard text read/write。
- IME enable/disable。
- IME candidate rect update。

第一版可选：

- Clipboard 多数据类型，例如 image、file list、custom MIME。
- Cursor shape apply。
- IME language/status query。

剪贴板至少要支持文本。多数据类型可以晚一些做，但 API 应避免只能表达纯文本的死角。

IME 的原则是：当 XUI 没有编辑控件需要激活时，XUI 应通过 proxy 保持 IME 关闭，避免中文输入法影响普通快捷键、游戏操作和非文本控件交互。IME 的 composition/key/text 消息不是 proxy 读取，而是由外部程序通过 XUI input API 传入。

### 4.2 Render Services

第一版必需：

- Surface create/destroy/query。
- Surface create from file/memory/RGBA pixels。
- Surface update from RGBA pixels。
- Surface clear，默认清透明色。
- Begin/end drawing to surface。
- Draw surface to surface, rect-to-rect。
- Draw surface to surface, quad-to-quad。
- Fill/stroke basic shapes to surface。
- Font load/free/metrics。
- Text measure。
- Text render to surface。
- Clear rect。Proxy 可以内部使用 scissor 实现局部覆盖清空，但不把 clip stack 作为 MVP 必选 API。

第一版可选：

- Surface map/readback。
- Rounded rect/native line/circle/path。
- GPU render target surface。
- Partial surface update acceleration。
- Text layout object and glyph run cache。
- External surface import。

所有绘制都以 surface 为目标。Proxy 不接收 widget/control，也不理解 layout、style selector、popup、scroll、event route。

## 5. Surface as the Unified Resource

XUI 应把 `texture` 统一进 `surface` 概念。Surface 是可被采样、可被绘制、可作为缓存、可作为最终输出目标的资源抽象。

```text
XuiSurface
  CPU RGBA bitmap
  GPU texture
  GPU render target
  decoded image
  root retained cache
  widget local/state cache
  external output target
```

Surface 通过 usage/caps 区分能力：

| Usage | Meaning |
| --- | --- |
| `SAMPLED` | 可作为源被 draw 到其他 surface |
| `RENDER_TARGET` | 可作为目标被绘制 |
| `CPU_READ` | 可读回像素 |
| `CPU_WRITE` | 可从 CPU 更新 |
| `IMMUTABLE` | 图片资源，不经常更新 |
| `CACHE` | Widget/root/tile cache |
| `OUTPUT` | 外部程序传入的最终输出目标 |

这样做的好处：

- 图片加载、控件缓存、根缓存、九宫格源图和输出目标都使用同一类 handle。
- XUI 内部逻辑更自洽：大部分绘制都是 surface 到 surface。
- 软件光栅器和 GPU 后端都能实现同一套 API。
- 后续如果需要保留 `texture` 名称，也只是 `surface` 的 sampled-only 别名。

Baseline pixel format：

```text
RGBA8 premultiplied alpha
top-left origin
integer physical backing size
logical size + virtual dpi scale stored in XUI metadata
```

Proxy 必须把后端纹理坐标差异隐藏掉。无论 surface 来自图片、CPU bitmap，还是 GPU render target，XUI 看到的 source rect 和 readback 都统一为 top-left origin；render target 被当作源 surface 再采样时，后端适配层负责处理 Y 方向差异。

Alpha 策略：XUI 内部 surface 统一使用 premultiplied alpha。`XUI_SURFACE_ALPHA_STRAIGHT` 只表示输入像素是 straight alpha，proxy 需要在创建/加载阶段转换为 premultiplied。图片加载默认解码为 RGBA 后转 premultiplied。默认 sampler 为 linear + clamp，控件缓存可以按需要改为 nearest。

## 6. Surface Blit Modes

缓存合成和图片绘制主要依赖两类 blit：

```text
rect-to-rect
  src rect -> dst rect

quad-to-quad
  src quad -> dst quad
```

`rect-to-rect` 是 APPUI 最常见路径，适合根缓存局部刷新、普通控件缓存、九宫格切片、图标绘制。

`quad-to-quad` 用于旋转、透视或非轴对齐变换。软件光栅器实现会更复杂，但 API 层应从第一版保留这个能力，否则未来动画、变换和复杂控件会被锁死。

九宫格不需要作为 backend 原生能力。XUI 可以把九宫格拆成 9 次 rect-to-rect surface draw；如果后端提供 batch draw，可以合并提交。

## 7. Virtual DPI and Size

XUI 只存在内部虚拟 DPI。外部程序负责把真实系统 DPI、窗口 scale 或游戏 viewport scale 转换后告诉 XUI。

建议 API 方向：

```c
void xuiSetVirtualDpi(xui_context ctx, float dpi_scale);
void xuiSetViewportSize(xui_context ctx, float w, float h);
void xuiNotifyDpiChanged(xui_context ctx, float dpi_scale);
```

DPI/scale 变化是 XUI 输入事件，会触发：

- layout invalidation。
- text measure/layout cache invalidation。
- size-dependent surface cache invalidation。
- root damage。

Proxy 不提供 `get_dpi_scale()`。Surface desc 中可以包含 XUI 传入的 scale metadata，但它不是由 proxy 自行读取系统得来。

## 8. Refresh and Rendering Entry

刷新不属于 Proxy。XUI 可以维护 dirty/damage 状态，并提供查询和渲染函数；外部程序决定何时调用。

建议 APPUI 路径：

```text
platform receives paint/damage rect
  -> app calls xuiSetViewportSize / xuiSetVirtualDpi when needed
  -> app calls xuiInput* for pending events
  -> app calls xuiRender(ctx, output_surface, dirty_rects)
  -> XUI updates dirty widget caches
  -> XUI composites root cache to output_surface
  -> app presents output_surface to window
```

建议 GAMEUI 路径：

```text
game frame
  -> app calls xuiUpdate(ctx, dt) or XUI reads xrt time where needed
  -> app calls xuiRender(ctx, frame_surface, full_or_damage_rect)
  -> app composites frame_surface with the game frame or presents it
```

可提供的 XUI API：

```c
int  xuiHasDamage(xui_context ctx);
int  xuiGetDamageRects(xui_context ctx, xui_rect_i* rects, int capacity);
void xuiClearDamage(xui_context ctx);
int  xuiRender(xui_context ctx, xui_surface target, const xui_rect_i* rects, int rect_count);
```

这里没有 `request_refresh` proxy callback。需要主动唤醒窗口时，应由外部程序根据 XUI dirty 查询结果或自己的应用状态决定。

## 9. Input Boundary

消息输入由 XUI API 提供，不由 Proxy 提供。

建议输入 API 家族：

```c
void xuiInputPointerMove(xui_context ctx, uint64_t pointer_id, float x, float y, uint32_t mods);
void xuiInputPointerDown(xui_context ctx, uint64_t pointer_id, int button, float x, float y, uint32_t mods);
void xuiInputPointerUp(xui_context ctx, uint64_t pointer_id, int button, float x, float y, uint32_t mods);
void xuiInputWheel(xui_context ctx, float x, float y, float dx, float dy, uint32_t mods);
void xuiInputKeyDown(xui_context ctx, int key, uint32_t mods, uint32_t native_code);
void xuiInputKeyUp(xui_context ctx, int key, uint32_t mods, uint32_t native_code);
void xuiInputText(xui_context ctx, const char* utf8);
void xuiInputImeBegin(xui_context ctx);
void xuiInputImeUpdate(xui_context ctx, const char* composition_utf8, int cursor);
void xuiInputImeEnd(xui_context ctx, const char* commit_utf8);
```

Proxy 的 IME 职责只在 XUI 需要开启/关闭 IME 或更新候选框位置时被调用。

## 10. Primitive Drawing

XUI 控件绘制到自身 cache surface 时，面对的是 `Painter`。Painter 可以记录命令、做 damage early reject，再通过 Proxy 落地。

第一版 Painter 必需命令：

- clear target rect。
- fill rect。
- stroke rect。
- draw surface rect。
- draw surface quad。
- draw text。

可选命令：

- rounded rect。
- line。
- triangle。
- circle/circle stroke。
- painter-level clip stack。第一版不要求 proxy 暴露 clip stack；多数裁剪由 surface 边界、source/destination rect 和缓存合成自然完成。
- polygon/path。
- alpha mask draw。

高层形状可以 fallback：

- border rect -> 4 个 fill rect。
- line -> tessellated quad。
- triangle -> draw surface/mesh or software raster。
- rounded rect/circle -> tessellation or software raster。
- alpha mask icon -> generated surface + tinted draw。

XUI 的约束是：这些形状只画到 surface，不直接画到窗口。

## 11. Text and Font

文本是 APPUI 性能重点。Proxy 需要提供 font 和 text 的最低能力，但复杂 text layout cache 可以由 XUI 管理。

Proxy 必需能力：

- font load from file/memory。
- font release。
- font metrics。
- simple text measure。
- simple text render to surface。

XUI text layer 负责：

- text version。
- font generation。
- width constraint。
- wrap/align/ellipsis/password flags。
- cursor rect。
- selection rect。
- hit test。
- IME candidate rect。
- glyph/text layout cache。

复杂 shaping、fallback font、emoji、BiDi 可以分阶段实现。但 API 不能被 ASCII-only `measure_text` 锁死。

## 12. Proxy VTable Direction

以下只是方向，不是最终 API：

```c
typedef struct xui_proxy_t {
    uint32_t size;
    uint32_t version;
    void* user;

    int  (*getCaps)(xui_proxy proxy, xui_proxy_caps_t* caps);

    int  (*clipboardSetText)(xui_proxy proxy, const char* text);
    int  (*clipboardGetText)(xui_proxy proxy, char* text, int capacity);

    void (*imeSetEnabled)(xui_proxy proxy, int enabled);
    void (*imeSetCandidateRect)(xui_proxy proxy, xui_rect_t rect);

    int  (*surfaceCreate)(xui_proxy proxy, xui_surface* out, const xui_surface_desc_t* desc);
    int  (*surfaceCreateRGBA)(xui_proxy proxy, xui_surface* out, int w, int h, const void* pixels, int stride, uint32_t flags);
    int  (*surfaceLoadFile)(xui_proxy proxy, xui_surface* out, const char* path, uint32_t flags);
    int  (*surfaceLoadMemory)(xui_proxy proxy, xui_surface* out, const void* data, int size, uint32_t flags);
    int  (*surfaceUpdateRGBA)(xui_proxy proxy, xui_surface surface, xui_rect_i rect, const void* pixels, int stride);
    int  (*surfaceReadRGBA)(xui_proxy proxy, xui_surface surface, void* pixels, int stride);
    int  (*surfaceGetDesc)(xui_proxy proxy, xui_surface surface, xui_surface_desc_t* desc);
    int  (*surfaceDraw)(xui_proxy proxy, xui_surface surface, xui_rect_t src_rect, xui_rect_t dst_rect, uint32_t tint, uint32_t flags);
    int  (*surfaceClear)(xui_proxy proxy, xui_surface target, uint32_t color);
    int  (*surfaceClearRect)(xui_proxy proxy, xui_surface target, xui_rect_t rect, uint32_t color);
    int  (*surfaceDrawTo)(xui_proxy proxy, xui_surface target, xui_surface surface, xui_rect_t src_rect, xui_rect_t dst_rect, uint32_t tint, uint32_t flags);
    int  (*surfaceDrawQuad)(xui_proxy proxy, xui_surface surface, const xui_surface_vertex_t* vertices, uint32_t flags);
    int  (*surfaceDrawQuadTo)(xui_proxy proxy, xui_surface target, xui_surface surface, const xui_surface_vertex_t* vertices, uint32_t flags);
    int  (*surfaceGetSampler)(xui_proxy proxy, xui_surface surface, xui_surface_sampler_t* sampler);
    int  (*surfaceSetSampler)(xui_proxy proxy, xui_surface surface, const xui_surface_sampler_t* sampler);
    int  (*surfaceGetGeneration)(xui_proxy proxy, xui_surface surface, uint32_t* generation);
    void (*surfaceDestroy)(xui_proxy proxy, xui_surface surface);

    int  (*shapePoint)(xui_proxy proxy, xui_surface target, float x, float y, float size, uint32_t color);
    int  (*shapeLine)(xui_proxy proxy, xui_surface target, float x0, float y0, float x1, float y1, float width, uint32_t color);
    int  (*shapeTriangleFill)(xui_proxy proxy, xui_surface target, xui_vec2_t a, xui_vec2_t b, xui_vec2_t c, uint32_t color);
    int  (*shapeTriangleStroke)(xui_proxy proxy, xui_surface target, xui_vec2_t a, xui_vec2_t b, xui_vec2_t c, float width, uint32_t color);
    int  (*shapeRectFill)(xui_proxy proxy, xui_surface target, xui_rect_t rect, uint32_t color);
    int  (*shapeRectStroke)(xui_proxy proxy, xui_surface target, xui_rect_t rect, float width, uint32_t color);
    int  (*shapeCircleFill)(xui_proxy proxy, xui_surface target, float x, float y, float radius, uint32_t color);
    int  (*shapeCircleStroke)(xui_proxy proxy, xui_surface target, float x, float y, float radius, float width, uint32_t color);
    int  (*shapeRectFill)(xui_proxy proxy, xui_surface target, xui_rect_t rect, float radius, uint32_t color);
    int  (*shapeRectStroke)(xui_proxy proxy, xui_surface target, xui_rect_t rect, float radius, float width, uint32_t color);

    int  (*fontLoadFile)(xui_proxy proxy, xui_font* out, const char* path, float size, uint32_t flags);
    int  (*fontLoadMemory)(xui_proxy proxy, xui_font* out, const void* data, int size, float size_px, uint32_t flags);
    int  (*fontGetMetrics)(xui_proxy proxy, xui_font font, xui_font_metrics_t* out);
    void (*fontDestroy)(xui_proxy proxy, xui_font font);
    int  (*textMeasure)(xui_proxy proxy, xui_font font, const char* utf8, xui_vec2_t* out);
    int  (*textDraw)(xui_proxy proxy, xui_surface target, xui_font font, const char* utf8, xui_rect_t rect, uint32_t color, uint32_t flags);

    int  (*drawBegin)(xui_proxy proxy, xui_draw_context* out, xui_surface target);
    int  (*drawEnd)(xui_proxy proxy, xui_draw_context dc);
    int  (*drawClearRect)(xui_proxy proxy, xui_draw_context dc, xui_rect_t rect, uint32_t color);
    int  (*drawSurface)(xui_proxy proxy, xui_draw_context dc, xui_surface src, xui_rect_t src_rect, xui_rect_t dst_rect, uint32_t tint, uint32_t flags);
    int  (*drawSurfaceQuad)(xui_proxy proxy, xui_draw_context dc, xui_surface src, const xui_surface_vertex_t* vertices, uint32_t flags);
    int  (*drawRectFill)(xui_proxy proxy, xui_draw_context dc, xui_rect_t rect, uint32_t color);
    int  (*drawRectStroke)(xui_proxy proxy, xui_draw_context dc, xui_rect_t rect, float width, uint32_t color);
    int  (*drawRectFill)(xui_proxy proxy, xui_draw_context dc, xui_rect_t rect, float radius, uint32_t color);
    int  (*drawRectStroke)(xui_proxy proxy, xui_draw_context dc, xui_rect_t rect, float radius, float width, uint32_t color);
    int  (*drawText)(xui_proxy proxy, xui_draw_context dc, xui_font font, const char* utf8, xui_rect_t rect, uint32_t color, uint32_t flags);
} xui_proxy_t;
```

重点：

- 没有 window。
- 没有 get DPI。
- 没有 get time。
- 没有 request refresh。
- 没有 input polling。
- 没有 present。

外部程序如果希望把窗口 backbuffer 包装成 `XuiSurface OUTPUT`，可以通过 `surfaceCreate` 或未来的 import API 完成。但 XUI 只把它当 surface 画，不知道它背后是不是窗口。

## 13. Cache Integration Rules

XUI cache 与 Proxy 的关系：

- CacheManager 决定 cache policy。
- Proxy 创建、清空、更新 surface。
- Painter 只绘制到 surface。
- Compositor 只把 surface 合成到目标 surface。
- 外部程序负责目标 surface 最终上屏。

必须建立以下规则：

- 每个 surface 有 resource generation。
- 每个 widget cache 有 content generation。
- style/theme/font/image 变化推进相关 generation。
- surface lost 后，Proxy 返回错误或 generation change，XUI 标记依赖它的 cache dirty。
- memory pressure 时，CacheManager 决定释放哪些 cache，Proxy 只负责销毁 surface。

State cache key 至少包含：

```text
widget stable id
state id
size in physical pixels
virtual dpi scale
style generation
font generation
surface/resource generation
```

## 14. Dirty and Bound Rules

Dirty rect 数据应使用 integer physical pixel rect。逻辑坐标转换到物理像素时需要统一 round 策略：

- cache backing size 向上取整。
- damage rect outward round。
- text/layout logical size 可保留 float。
- surface draw 的最终采样区域必须可裁剪。

Painter/Proxy 应支持早期剔除：

- command rect 不与 surface dirty rect 相交，跳过。
- compositor draw surface 不与 target damage 相交，跳过。
- text/surface draw 必须可传入明确 bounds；文字溢出优先通过绘制到控件缓存时的 surface 边界自然裁剪。

## 15. What Not to Put in Proxy

不要放进 Proxy：

- window。
- system DPI query。
- request_refresh callback。
- platform message polling。
- time query。
- widget tree。
- control type。
- layout algorithm。
- style selector。
- popup placement。
- scroll model。
- event routing。
- cache policy。
- XSON resource import。
- business callback。

Proxy 只认识 system service、surface、draw context、font、text。

## 16. Proxy Capability Discipline

XUI2 上层实现必须严格依赖已经声明的 Proxy 能力。后续开发中，如果 Painter、Cache、Compositor、Widget、Layout、Text 或 Style 发现现有 Proxy 无法完成某个必需行为，流程必须立即停止，并向项目负责人汇报：

- 缺失的 Proxy 能力是什么。
- 当前功能为什么无法用现有 Proxy contract 实现。
- 建议新增的最小 Proxy API 或 caps 是什么。
- 对已有 proxy 实现、测试和兼容性的影响。

在完成确认前，不允许在上层私自绕过 Proxy 边界，不允许直接调用后端图形 API，不允许临时把平台、窗口、输入、布局、样式或控件语义塞进 Proxy，也不允许在单个实现里添加未进入 `xui.h` contract 的隐式能力。

Proxy contract 的变更必须同步更新：

- `xui.h`。
- XGE/GPU proxy。
- Null/Record proxy。
- Software RGBA proxy。
- proxy smoke/pixel tests。
- 本设计文档。

## 17. Implementations to Prepare

建议第一阶段至少准备三类 Proxy 实现：

| Implementation | Purpose |
| --- | --- |
| Null/Record Proxy | 单元测试、命令统计、debug trace |
| Software RGBA Proxy | APPUI fallback、截图、验证 cache 语义 |
| XGE/GPU Proxy | GAMEUI 和现有引擎集成 |

外部 platform adapter 不属于 XUI core contract，但可以在 examples/tools 中提供：

- Win32 adapter：窗口、WM_PAINT、DPI、剪贴板、IME 消息转 XUI API。
- XGE adapter：ProcFrame、游戏输入、backbuffer surface、XRT 时间集成。
- Tool adapter：UIDesign 的多窗口、多文档、预览 surface。

Null/Record Proxy 必须作为一等实现保留。它能让 layout/style/event/cache 测试不依赖图形环境。

## 18. Migration from Current Host

当前 `xge_xui_host_t` 和 `xge_xui_host_v2_t` 可以这样迁移：

| Current Concept | XUI Target |
| --- | --- |
| `draw_rect` | Painter command + proxy fill_rect |
| `draw_border_rect` | Painter stroke_rect, fallback to rects |
| `draw_rounded_rect` | Painter rounded_rect helper, fallback mesh/software |
| `draw_line/triangle/circle` | Painter shape helpers, proxy optional/native |
| `draw_texture/draw_image` | Painter draw_surface |
| `draw_text/measure_text/fontGetMetrics` | XUI text layer + proxy text/font service |
| `clip_push/clip_clear/clip_get` | Painter 可选能力；proxy MVP 不暴露 clip stack |
| `texture_create_*` | surface load/create/update |
| `font_create_*` | proxy font service |
| `clipboard` | proxy clipboard service |
| `time` | xrt |
| `viewport size` | `xuiSetViewportSize()` |
| `dpi scale` | `xuiSetVirtualDpi()` |
| `request_refresh` | XUI damage query + external scheduling |
| `flush/present` | external backend/app |

`xge_xui_paint_command_t` 的方向仍有价值，但 XUI command 应明确 command type、clip id、target surface、source surface、blend、opacity、dirty bounds 和 debug owner。

## 19. First Version Boundary

第一版建议强制完成：

- XuiProxy 最小依赖层。
- Unified surface abstraction。
- CPU RGBA software proxy。
- Null/Record proxy。
- Draw surface rect-to-rect。
- Draw surface quad-to-quad API。
- Clear surface to transparent。
- Clear surface rect。
- Draw context begin/end。
- Surface sampler and generation query。
- Basic shape drawing to surface。
- Simple text + font metrics。
- Clipboard text。
- IME enable/disable + candidate rect。
- XUI input API for pointer/key/text/IME/DPI/viewport。
- XUI render API to target surface with damage rects。
- Render stats/debug dump。

第一版可以暂缓：

- Clipboard 多数据类型。
- Cursor apply。
- Painter clip stack。
- Complex CSS-like filters。
- Arbitrary path clip。
- Full text shaping engine。
- GPU atlas management API。
- Retained vector scene graph。
- Animation compositor。
- Subpixel-perfect browser text layout。

## 20. Open Decisions

仍需后续确认：

- Surface image loading由 Proxy 直接实现，还是由 XUI/xrt image codec 解码后调用 `surfaceCreateRGBA`。
- Clipboard 多数据类型的 API 形状。
- Fallback font 放在 XUI text layer；shaping、BiDi、emoji 第一版暂缓。
- Output surface import API 是否第一版需要。
- Quad-to-quad 是否要求 Tier 0 必须硬实现，还是允许 software fallback。
- Surface tile size 默认值和后端覆盖策略。

## 21. Design Conclusion

## 22. Medium-Term Path And Mesh Direction

High-quality vector UI should not be solved by adding one-off curve renderers inside individual controls. The medium-term direction is:

- XGE owns the efficient primitive implementation for generic triangle mesh and path rendering.
- XUI proxy exposes backend-neutral mesh/path capabilities and capability flags.
- XUI painter provides control-friendly wrappers for fill, stroke, dash, cap, join, fill rule, and anti-alias behavior.
- XUI controls such as Chart, icon widgets, and future vector-heavy widgets consume those painter/proxy capabilities.

This is required for anti-aliased Bezier curves, stroked curves, dashed curves, curve fill, SVG path subsets, high-quality area charts, and complex icon/shape rendering.

XUI can still keep CPU tessellation fallback for simple cases or test backends, but that fallback must be shared infrastructure, not private chart/control code. The tracked plan lives in `vector-render-capability-spec.md`.

## 23. Existing Boundary Summary

XUI 的代理层应从一开始保持窄边界：

1. XUI 没有窗口能力，只渲染到 surface。
2. XUI 不读取系统 DPI，只接受外部输入的虚拟 DPI。
3. XUI 不从 proxy 获取时间，时间来自 xrt。
4. XUI 不通过 proxy 请求刷新，外部程序调用 XUI render API。
5. 输入消息通过 XUI input API 传入，不由 proxy polling。
6. Proxy 只提供必须依赖的系统服务和 surface 渲染能力。
7. Surface 是统一资源，图片、缓存、根节点输出和最终目标都走同一抽象。

这个边界能让 XUI 脱离具体图形 API，也能让 APPUI 缓存系统、GAMEUI 集成和软件光栅器路径使用同一套内部逻辑。




