# XUI Cache and APPUI Rendering Design

本文记录 XUI 重构中关于缓存、APPUI 刷新和 GAMEUI 刷新的基础设计。XUI 作为新目录独立推进，不要求兼容现有 XUI 内部实现，但应尽量让未来公开 API 能平滑迁移。

## 1. 背景

现有 XUI 更接近游戏 UI 的即时绘制模型：状态变化后标记 dirty，下一次 paint 仍可能按整棵 widget tree 绘制。这个模型适合 GAMEUI，但不适合普通 APPUI：

- 静态控件反复绘制背景、边框、文字，浪费 CPU。
- GPU 每次接收大量小 draw call，不利于低功耗 APP。
- 脏矩形更像刷新提示，不是完整的 damage/composite 机制。
- 控件绘制和系统级绘制耦合，难以标准化裁剪、缓存和局部 present。

XUI 的核心方向是把控件绘制与 XUI 系统级绘制解耦。控件负责在自身状态或内容变化时更新缓存；XUI 运行时负责 layout、damage、cache dependency、clip、Z order 和 final compositing。

## 2. Goals

- 同时支持 APPUI 和 GAMEUI，并让两者各自走最优路径。
- 控件内容不变时不重新执行控件 draw。
- 静态界面优先通过缓存合成，降低 CPU/GPU 时间。
- 脏矩形、裁剪和重绘标准化，减少控件局部补丁。
- 支持 local cache、state cache、subtree cache、tile cache 和 display-list cache。
- 支持父控件是否吸收子控件缓存的策略选择。
- 保留 full repaint fallback，方便调试、兼容和后端能力不足时回退。
- 为未来动态换肤、样式广播、布局重排和数据控件局部刷新预留清晰接口。

## 3. Non-Goals

- 第一版不实现浏览器级完整 DOM/CSS/rendering engine。
- 第一版不强制每个 widget 都拥有独立 bitmap/texture。
- 第一版不要求所有后端都支持真实 partial present。
- 第一版不把视频、游戏画面、复杂 canvas 等高频动态内容强行纳入静态缓存。

## 4. Core Pipeline

XUI 的渲染管线按阶段拆分：

```text
Widget State
  -> Measure/Layout
  -> Paint Record or Control Raster
  -> Local Cache / State Cache
  -> Subtree Composite Cache
  -> Root Damage Composite
  -> Proxy Composite to Target Surface
```

其中：

- Widget tree 负责状态、事件、语义、布局输入。
- Render node tree 负责可见性、clip、Z order、transform、cache policy。
- Surface cache 负责 bitmap、texture、tile、state variant 的生命周期。
- Damage graph 负责脏区收集、冒泡、裁剪、合并和重绘依赖。
- Compositor 负责把缓存按 Z order 和 clip 合成到目标 surface。
- Proxy 负责 surface 创建、绘制和 surface-to-surface 合成。
- 外部程序负责窗口、系统刷新调度和最终 present。

## 5. Render Modes

### 5.1 GAMEUI Mode

GAMEUI 默认假设每帧都会绘制，适合游戏内嵌 UI、HUD、动态技能栏、频繁动画界面。

推荐策略：

- frame-driven update。
- 可选择直接 compositing，也可跳过复杂 damage。
- 动态控件可使用 local/state cache，但不强制 subtree cache。
- 可用 full-frame composition，减少复杂脏区管理成本。
- cache 更偏向状态切换和文字/图标预渲染。

### 5.2 APPUI Mode

APPUI 默认假设界面大部分时间静止，适合工具、编辑器、普通桌面/移动应用。

推荐策略：

- event/data/style/layout driven update。
- 无 damage 时不绘制。
- 有 damage 时只更新必要 cache 或 tile。
- final draw 尽量只 composite 受影响区域。
- XUI 合成到外部传入的 output surface；外部后端如支持 partial present，可自行把 XUI damage 映射到平台刷新。

## 6. Cache Types

### 6.1 Local Cache

Local cache 表示 widget 自身内容，不包含 children。

适用场景：

- Label 文本。
- Button 本体背景、边框、文字和图标。
- Input 边框、背景、文本。
- Image 控件的缩放结果。

Local cache 的优点是子控件变化不会导致当前 widget 本体重绘。缺点是最终合成时需要继续遍历 children。

### 6.2 State Cache

State cache 是同一个 widget 的多状态缓存。状态 ID 决定当前使用哪份缓存。

典型状态：

- normal
- hover
- active
- pressed
- disabled
- focused
- checked
- selected
- error

Button 这类控件可以在标题、字体、颜色或尺寸变化时更新多个状态缓存。运行时 hover/pressed 切换只切换 cache handle，不重新绘制控件内容。

State cache 不应强制 eager 生成。推荐策略：

```text
LAZY          first use raster
EAGER         init/update time raster all configured states
IDLE_PREWARM  first use required state, prewarm other states during idle
PINNED        generated cache cannot be evicted unless resource/device reset
```

### 6.3 Subtree Cache

Subtree cache 表示父 widget 缓存包含自身和 children 的合成结果。

适用场景：

- 静态 Panel。
- Toolbar。
- StatusBar。
- Sidebar。
- 静态表单区域。
- 不频繁变化的 DockPanel pane。

优点：

- 父级 composite 时只需要画一个矩形缓存。
- 子树不需要收到系统级 draw。
- 对静态 APPUI 性能收益大。

缺点：

- 子控件变化可能触发父级 subtree cache 更新。
- 父级很大时整张缓存重绘和上传成本高。
- 嵌套过深时可能造成 cache propagation 过重。

### 6.4 Tiled Subtree Cache

大型 subtree 不应使用单张大 surface。应拆成 tile，例如 256x256 或 512x512。

适用场景：

- Root surface。
- ScrollView content。
- 大型文档视图。
- 大表格。
- 复杂编辑器工作区。

优点：

- 单个控件变化只更新相交 tile。
- 可以按 viewport 加载、回收、预热 tile。
- 避免大纹理更新拖垮 GPU bandwidth。

### 6.5 Display-List Cache

Display-list cache 保存绘制命令，而不是立即 raster 成 bitmap/texture。

适用场景：

- 后端切换。
- DPI 或缩放变化频繁。
- 内存预算紧张，不希望所有内容都保留 bitmap。
- 控件 draw command 很少，但 raster surface 很大。

Display list 可在需要时 raster 到 CPU bitmap 或 GPU render target。

## 7. Cache Policy

每个 render node 可以选择缓存策略：

```c
typedef enum xui_cache_policy_t {
    XUI_CACHE_NONE = 0,
    XUI_CACHE_LOCAL,
    XUI_CACHE_STATE,
    XUI_CACHE_SUBTREE,
    XUI_CACHE_SUBTREE_TILED,
    XUI_CACHE_DISPLAY_LIST,
    XUI_CACHE_AUTO
} xui_cache_policy_t;
```

建议默认：

- Leaf control: `LOCAL` or `STATE`
- Button/CheckBox/Tab/Toolbar item: `STATE`
- Panel/Toolbar/StatusBar/Sidebar: `SUBTREE`
- Root/large ScrollView/document: `SUBTREE_TILED`
- High-frequency canvas/game/video: `NONE` or explicit custom surface
- Unknown custom owner draw: conservative `LOCAL`, optionally full repaint fallback

`AUTO` 由运行时根据面积、变更频率、透明度、children 数量、内存预算和后端能力决定。

## 8. Surface Abstraction

缓存不应固定为 CPU ARGB bitmap，也不应固定为 GPU texture。统一抽象为 surface：

```c
typedef enum xui_surface_kind_t {
    XUI_SURFACE_CPU_ARGB,
    XUI_SURFACE_GPU_TEXTURE,
    XUI_SURFACE_GPU_RENDER_TARGET,
    XUI_SURFACE_OUTPUT
} xui_surface_kind_t;
```

CPU ARGB 适合 Win32/GDI、软件后端、离线测试和截图。

GPU texture/render target 适合 OpenGL/Sokol/Metal/Vulkan/WebGL，避免每次从 CPU bitmap 上传 GPU。

所有 surface 必须明确：

- width / height
- virtual dpi scale
- pixel format
- premultiplied alpha policy
- color space
- proxy/resource generation
- resource generation
- last used frame/time
- memory cost

## 9. Cache Key

Cache key 至少包含：

```text
widget id or stable node id
cache policy
state id
width / height
virtual dpi scale
theme version
style version
content version
layout version
font version
resource version
proxy/surface kind
pixel format
```

任一版本变化都应使对应 cache 失效或进入 pending update。

## 10. Damage Model

XUI 应建立正式 damage region，而不是只记录调试 dirty rect。

Damage 来源：

- Widget 内容变化。
- Widget 状态 ID 变化。
- Style/theme 变化。
- Layout/size/position 变化。
- Visibility/enabled/focus 变化。
- Child cache 更新。
- Scroll offset 变化。
- Overlay open/close/move。
- Proxy surface lost/reset。

Damage region 应支持：

- add rect
- add old rect + new rect
- union
- intersect clip
- expand for visual outsets
- tile mapping
- merge threshold
- full damage fallback

Layout/paint/cache damage 应分层：

```text
layout dirty
paint dirty
local cache dirty
state cache dirty
subtree cache dirty
tile dirty
screen damage
```

## 11. Cache Update Contract

控件不直接响应系统 draw。控件在需要更新自身缓存时进入 update transaction：

```c
xuiUpdateBegin(widget, state_id, flags);
// control draws into its selected cache surface or display list
xuiUpdateEnd(widget);
```

`xuiUpdateBegin` 负责：

- 选择或创建目标 cache。
- 根据 state id、size、style version 等检查 cache 是否可复用。
- 绑定 CPU bitmap 或 GPU render target。
- 设置 local coordinate 和 initial clip。

`xuiUpdateEnd` 负责：

- 提交 cache version。
- 计算 local damage。
- 通知父节点 cache dependency changed。
- 根据 parent cache policy 触发 subtree/tile dirty。
- 请求 APPUI refresh 或 GAMEUI frame composite。

控件自己决定是否 clear 缓存：

- Opaque widget 通常 clear。
- Incremental canvas 可以保留旧内容并局部更新。
- Text/label/button 这类 deterministic 控件建议完整重绘 local cache。

## 12. Parent Cache Propagation

子控件 cache 更新后，父控件有两种主要路径：

### 12.1 Parent Absorbs Children

父控件的 subtree cache 包含子控件合成结果。

流程：

```text
child local/state cache updated
  -> parent subtree cache dirty in child's bounds
  -> update parent cache or parent tiles
  -> propagate to next cache boundary
```

适合静态 APPUI。最终 root composite 可以很快。

风险：

- 大父控件可能因小变化频繁重合成。
- 深层 cache boundary 过多会增加传播成本。

应通过 tile 和 cache boundary 控制。

### 12.2 Parent Does Not Absorb Children

父控件只缓存自身 local 内容。最终绘制时逐级 composite children。

流程：

```text
child cache updated
  -> screen damage changed
  -> compositor traverses visible affected subtree
```

适合动态内容、多层交互或高频变化区域。

风险：

- 每次 composite 需要更多节点遍历和 clip 计算。
- 静态界面收益不如 subtree cache。

### 12.3 Recommended Default

XUI 不应固定单一策略。推荐：

- APP shell 默认偏 subtree/tiled subtree。
- 普通 leaf control 默认 local/state。
- 大型内容区默认 tiled subtree。
- 高频动态控件默认 local or none。
- Runtime 根据变更频率可调整 `AUTO` 策略。

## 13. Clipping

矩形缓存使标准裁剪大幅简化：

```text
dst rect intersect clip rect
src uv adjusted from intersection
draw clipped quad
```

但以下情况必须设计为 visual bounds：

- rounded corner
- shadow
- glow
- focus ring
- thick border
- text overflow
- transformed child
- popup/tooltip/drag adorner

基础结构应区分：

```text
layout bounds
hit bounds
paint bounds
cache bounds
screen damage bounds
```

## 14. Z Order and Overlap

Composite 必须统一使用：

```text
layer > zIndex > treeOrder
```

当子控件更新导致父 cache 需要更新时，必须考虑同一 cache boundary 内的覆盖关系：

- 若父 cache 吸收 children，更新 dirty area 时需要重合成 dirty area 内所有相交 child，按 Z order 绘制。
- 若父 cache 不吸收 children，compositor 在 screen damage 内按 render tree 绘制相交节点。

合理 APPUI 不应有大量深度重叠控件；但系统不能依赖这个假设来保证正确性。

## 15. Memory Budget and Eviction

缓存系统必须有预算，否则多状态缓存和 subtree cache 很容易失控。

建议 budget 维度：

- CPU bitmap bytes
- GPU texture bytes
- render target bytes
- tile count
- pinned cache bytes
- per-widget maximum cache bytes

Eviction 策略：

- LRU by last used time/frame.
- Prefer evict lazy state variants before current state.
- Prefer evict offscreen tiles before visible tiles.
- Do not evict pinned cache unless device reset or explicit purge.
- On memory pressure, degrade subtree cache to local cache or full repaint.

## 16. Proxy and Output Strategy

不同 Proxy/外部后端能力不同：

### 16.1 CPU/Win32 Bitmap Path

- 控件 raster 到 CPU ARGB bitmap。
- WM_PAINT region 映射到 XUI damage。
- Root 或 subtree cache blit 到外部传入的 output surface。
- 适合普通桌面 APP 和早期验证。

### 16.2 GPU Texture Path

- 控件 raster 到 GPU render target 或 texture。
- Composite 用 textured quads。
- 支持 scissor 和 tile damage。
- 避免 CPU->GPU 频繁上传。

### 16.3 Retained Root Surface

如果 swapchain/backbuffer 不保证保留上一帧，APPUI partial repaint 不能直接只画 dirty rect 到 backbuffer。

应使用 retained root surface：

```text
update dirty widgets/tiles into retained root surface
draw root surface to output surface
external backend presents output surface
```

外部后端支持真实 partial present 时，再把 XUI screen damage 映射到平台 partial present。

## 17. Correctness Risks

需要提前约束：

- 透明 surface 统一使用 premultiplied alpha。
- 文本亚像素 AA 在透明缓存上可能有彩边，应支持 grayscale AA 或后端专用策略。
- virtual DPI/scale 变化必须 invalid all size-dependent caches。
- Font/resource/theme 变化必须通过 version 广播失效。
- Render target lost/device reset 必须重建 GPU cache。
- 自绘控件必须声明 paint bounds；不声明时保守扩大到 widget bounds 或 full subtree damage。
- Backdrop blur/filter 依赖背后内容，不能当普通 local cache 处理。

## 18. Initial Implementation Phases

### Phase A: Core Types and Software Reference

- 定义 render node、surface、cache key、damage region、cache policy。
- 实现 CPU ARGB software surface。
- 实现 local/state cache。
- 实现 full repaint fallback。
- 建立 correctness tests。

### Phase B: APPUI Damage Composite

- 实现 screen damage。
- 实现 clipped quad blit。
- 实现 subtree cache boundary。
- 支持 parent absorbs children。
- 实现 tiled subtree cache 的最小版本。

### Phase C: GPU Proxy

- 增加 GPU texture/render target surface。
- 增加 texture quad compositor。
- 增加 scissor/tile upload/retained root surface。
- 处理 device reset 和 resource generation。

### Phase D: Auto Policy and Performance

- 加入 memory budget 和 LRU。
- 加入 idle prewarm。
- 加入 cache policy auto heuristic。
- 建立 APPUI 和 GAMEUI benchmark。

### Phase E: Integration with Layout and Style

- Layout dirty 与 cache dirty 分离。
- Style class/theme/token version 广播到 cache system。
- 动态换肤时按 style dependency 局部失效。

## 19. Design Decisions So Far

- XUI 从新目录推进，不直接重构现有 XUI。
- 控件 draw 与系统 draw 解耦。
- XUI draw/compositor 默认不调用控件 draw，只 composite 已更新 cache。
- 控件通过 update transaction 更新自身 cache。
- 父控件是否吸收子控件缓存由 cache policy 决定。
- APPUI 优先静态缓存和 damage composite。
- GAMEUI 保留 frame-driven 快路径。
- 多状态缓存是 widget 基础能力，但生成策略可 lazy/eager/idle/pinned。
- 大 surface 必须支持 tile 化。
- full repaint fallback 必须长期存在。

## 20. Open Questions

- 第一版是否先以 CPU ARGB software compositor 验证，还是直接做 GPU texture path。
- 默认 tile size 选择 256 还是 512，是否允许后端覆盖。
- State id 是否使用整数 bitmask、枚举组合，还是 hash key。
- Subtree cache 的自动启发式阈值如何设定。
- 自绘控件 paint bounds API 如何设计。
- 文本缓存是进入 widget local cache，还是单独建立 glyph/text run cache。
- APPUI 的 retained root surface 是否第一版必须实现。
- XUI 是否保留 C API 风格，还是内部用更强的模块边界后再导出 C API。



