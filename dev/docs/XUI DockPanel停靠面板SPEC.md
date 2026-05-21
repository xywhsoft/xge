# XUI DockPanel 停靠面板 SPEC

本文档跟踪 XUI DockPanel 停靠面板系统的设计、实现、验证和后续维护进度。目标是获得接近 DockPanelSuite / WeifenLuo DockPanel 的使用手感，同时采用适合 XUI 与 C 结构维护的内部模型。

关联文档：

- `dev/docs/XUI Widget V2基础SPEC.md`
- `dev/docs/XUI控件标准与扩展SPEC.md`
- `dev/docs/XUI声明式界面与样式SPEC.md`
- `dev/docs/XUI轻量控件与布局SPEC.md`

关联资源：

- `res/xui_builtin_atlas.png`
- `res/xui_builtin_atlas.json`
- `src/xge_xui_builtin_assets.inc`
- `res/xui_dockpanel_suite/README.md`
- `res/xui_dockpanel_suite/LICENSE.dockpanelsuite.txt`
- `res/xui_dockpanel_suite/vs2005/`

## 进度维护规则

每次开始 DockPanel 相关开发前，必须先更新本文档进度：

1. 将即将开发的任务从 `[ ]` 改为 `[~]`。
2. 如果任务范围变化，先调整任务描述或新增子任务。
3. 如果发现阻塞，追加到“阻塞与决策记录”。
4. 开发完成、构建通过、示例或测试通过后，才能改为 `[x]`。
5. 如果任务被放弃、替代或暂缓，标记为 `[!]` 并写明原因。
6. 每个可运行切片必须由开发者自行构建、启动、调试、运行测试或 smoke，并在涉及画面时自行截图或做像素/画面检查；不得把首轮验证留给用户统一手动测试。

状态含义：

- `[ ]` 未开始。
- `[~]` 进行中。
- `[x]` 已完成，并通过必要验证。
- `[!]` 放弃、替代或暂缓，必须说明原因。

完成口径：

- 设计任务：文档更新完成，并明确边界、数据结构、生命周期和失败路径。
- API 任务：公开声明、实现、最小调用示例、销毁路径齐备。
- 布局任务：覆盖创建、插入、拆分、合并、最小尺寸、脏布局传播和 splitter 拖动。
- 控件任务：覆盖创建、属性、事件、绘制、销毁、focus/capture/tooltip/disabled/hidden。
- 拖拽任务：覆盖 begin、hover target、indicator、preview rect、commit、cancel、capture lost、Escape。
- Overlay 任务：覆盖 attach/detach、z-order、input transparent、modal 旁路、focus restore。
- 资源任务：atlas 坐标、透明边缘、fallback 绘制、版权说明齐备。
- XSON 任务：字段解析、错误路径、资源释放、失败回滚、最小示例齐备。
- 性能任务：说明热路径不做全树字符串查找、全树重建或大量临时 widget 创建。

## 总体目标

- [x] 提供 `docklayout -> dock_region -> dock_node(split|pane) -> dock_pane -> dockwindow tabs` 的 XUI 内建停靠系统。
- [x] 支持 dockwindow 停靠到已有 docklayout 区域、停靠到 pane 中央成为 tab、停靠到 pane 边缘形成新的 split 区域。
- [x] 支持 XUI root 内部浮动窗口，不支持拖出主窗口成为 OS/native window。
- [x] dockwindow 组合现有 `xge_xui_window_t`，不复制一套普通 window。
- [x] 停靠时隐藏普通 window 外框，由 dockpane 负责 tab/title/chrome。
- [x] 浮动时恢复特殊 dockwindow 外观，但仍限制在 XUI root 内。
- [x] 拖拽过程接入现有事件、pointer capture、overlay root、`XGE_XUI_LAYER_DRAG_ADORNER` 和 z-order。
- [x] 像素级复刻 DockPanelSuite VS2005 的 dock indicator 主要视觉素材。
- [x] 使用 XRT 提供的内存、数组、字典、链表等基础能力，不重复造基础容器。
- [x] 建立示例、回归测试、公开 API 文档和 XSON 声明式入口。

## 硬边界

- [x] 不支持拖出 XUI root 外生成新的原生窗口。说明：后续“floating”均指 XUI root 内 overlay/window。
- [x] 不采用 DockPanelSuite 的 `previousPane + alignment + proportion` 作为内部主模型。说明：XUI 使用显式二叉 split tree，更适合 C 结构维护、调试和序列化。
- [x] 不使用 DockPanelSuite hotspot 图作为命中检测数据。说明：XUI 使用几何命中区域，hotspot 图仅是 WinForms 实现细节。
- [x] 第一版只使用 DockPanelSuite VS2005 风格素材。说明：VS2012 素材当前观感不正确，不进入内建 atlas。
- [x] 不在 layout/paint 热路径解析 XSON、查找字符串 ID 或重建 pane/window 树。
- [x] 不允许普通 widget hit-test 单独决定拖拽落点。说明：拖拽必须经过 dock drag manager 的专用命中顺序。
- [x] 不在 dockpane 内嵌套 UI 卡片式外观。说明：DockPanel 是工具界面基础设施，应保持高密度、清晰边界和低装饰。

## 无人值守开发约束

- [x] DockPanel 开发过程必须默认按无人值守方式推进：开发者自行调试、自行启动程序、自行构建测试用例、自行截图并检查画面是否正常。说明：本轮已执行自构建、自 smoke、自视觉验证约束。
- [x] 每个阶段完成前必须留下可复现验证路径，至少包括构建命令、测试命令、示例启动命令或失败时的精确阻塞原因。说明：本轮记录 `build_dll.bat`、专用 smoke、示例 smoke、XUI 示例全量构建与 `build_test.bat` 阻塞。
- [x] 任何涉及视觉、拖拽、overlay、indicator、preview rect、tab/pane 布局的任务，不能只靠编译通过；必须补充截图、像素检查、debug snapshot 或人工可复现的画面核对记录。说明：已生成 drag preview、tab float preview、float dock preview、global dock preview 与 splitter preview 截图并做像素采样。
- [x] 用户的统一手动测试只作为整体验收，不作为开发阶段首轮验证手段。说明：当前切片首轮验证由开发者本地完成。
- [x] 如果本地环境无法启动示例、无法截图或无法完成画面检查，必须在“阻塞与决策记录”写明具体原因、命令、错误文本和替代验证方式。说明：当前未遇到截图阻塞；已完成本地启动、截图和画面检查。

## 当前资源状态

- [x] 收集 DockPanelSuite VS2005 dock indicator 资源到 `res/xui_dockpanel_suite/vs2005/`。
- [x] 保存 DockPanelSuite 许可说明到 `res/xui_dockpanel_suite/LICENSE.dockpanelsuite.txt`。
- [x] 保存资源来源、用途和选择原则到 `res/xui_dockpanel_suite/README.md`。
- [x] 将 pane diamond 系列外部白色/红色 key 边缘转为 alpha 透明。
- [x] 将 dock pane close/auto-hide/dock/option/overflow 按钮白色 key 背景转为 alpha 透明。
- [x] 将可用素材添加到 `res/xui_builtin_atlas.png`。
- [x] 将 atlas 坐标添加到 `res/xui_builtin_atlas.json`。
- [x] 重新生成 `src/xge_xui_builtin_assets.inc` 内建资源宏。
- [x] 不添加 `DockIndicator_PaneDiamond_Hotspot*.png`。说明：这些图用于 DockPanelSuite 像素命中，不是视觉素材。
- [x] 完整 DLL 链接验证已关闭。说明：2026-05-21 本轮实现后 `build_dll.bat` 通过。

## 资源清单

Dock indicator 资源：

- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_LEFT`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_RIGHT`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_TOP`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_BOTTOM`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_FILL`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_LEFT`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_LEFT_ACTIVE`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_RIGHT`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_RIGHT_ACTIVE`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_TOP`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_TOP_ACTIVE`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_BOTTOM`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_BOTTOM_ACTIVE`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_FILL`
- [x] `XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_FILL_ACTIVE`

Dock pane button 资源：

- [x] `XGE_XUI_ASSET_DOCK_PANE_CLOSE`
- [x] `XGE_XUI_ASSET_DOCK_PANE_AUTO_HIDE`
- [x] `XGE_XUI_ASSET_DOCK_PANE_DOCK`
- [x] `XGE_XUI_ASSET_DOCK_PANE_OPTION`
- [x] `XGE_XUI_ASSET_DOCK_PANE_OPTION_OVERFLOW`

资源使用规则：

- [ ] indicator 默认态与 active 态由同一 overlay 绘制路径切换。
- [ ] pane diamond 的 center/fill 命中代表作为 tab 停靠到当前 pane。
- [ ] panel left/right/top/bottom/fill 命中代表停靠到 docklayout 全局区域。
- [x] button 图标只作为 alpha mask 或原图绘制源，颜色/tint 策略后续由主题决定。说明：dock pane button 绘制统一传入状态色 tint，fallback glyph 也使用相同颜色。
- [x] 缺少 texture host 或资源加载失败时，必须提供几何 fallback。说明：内建资源 draw 返回失败时，DockPanel 按钮、strip 图标、pane diamond、panel indicator 改走 bitmap mask/几何 fallback。

## 名词与层次

DockLayout：

- 顶层停靠控件，拥有若干 dock region、floating dockwindow 列表、拖拽状态和 overlay indicator。
- 对外表现为一个 XUI container/viewport 类控件，内部参与 layout/paint/event。

DockRegion：

- DockLayout 内的固定区域：`document`、`left`、`right`、`top`、`bottom`。
- 每个 region 拥有一个 root dock_node。
- side region 可隐藏、可调整比例、可折叠为空。

DockNode：

- 显式二叉 split tree 节点。
- `split` 节点保存 axis、ratio、first、second、splitter rect 和最小尺寸约束。
- `pane` 节点保存一个 dock_pane。
- 空 region 允许 root 为 NULL。

DockPane：

- tab 容器，管理一个或多个 dockwindow。
- 负责 tab strip、active tab、title/chrome、close/pin/dock/options 按钮、client rect。
- dockwindow 停靠后，其普通 window chrome 不参与显示。

DockWindow：

- 可停靠内容单元，组合现有 `xge_xui_window_t`。
- 保存 dock state、owner layout、owner pane、tab title、icon、client widget、last floating rect。
- 停靠、浮动、隐藏只是同一 dockwindow 的状态迁移。

Floating DockWindow：

- XUI root 内部浮动状态。
- 仍使用 `xge_xui_window_t` 的窗口移动、标题栏、边框和关闭按钮能力，但需要 dockwindow 特殊 chrome 和 root clamp。

DockIndicator：

- 拖拽期间显示的放置面板和预览矩形。
- 挂载到 overlay root，使用 drag adorner layer。
- 只在 dock drag manager 处于 dragging 状态时可见。

## 数据模型草案

公开结构保持 opaque 或半 opaque；内部结构可按以下草案落地：

```c
typedef struct xge_xui_dock_layout_t xge_xui_dock_layout_t;
typedef struct xge_xui_dock_region_t xge_xui_dock_region_t;
typedef struct xge_xui_dock_node_t xge_xui_dock_node_t;
typedef struct xge_xui_dock_pane_t xge_xui_dock_pane_t;
typedef struct xge_xui_dock_window_t xge_xui_dock_window_t;
```

核心枚举：

```c
typedef enum xge_xui_dock_region_kind_t {
    XGE_XUI_DOCK_REGION_DOCUMENT,
    XGE_XUI_DOCK_REGION_LEFT,
    XGE_XUI_DOCK_REGION_RIGHT,
    XGE_XUI_DOCK_REGION_TOP,
    XGE_XUI_DOCK_REGION_BOTTOM
} xge_xui_dock_region_kind_t;

typedef enum xge_xui_dock_side_t {
    XGE_XUI_DOCK_SIDE_NONE,
    XGE_XUI_DOCK_SIDE_LEFT,
    XGE_XUI_DOCK_SIDE_RIGHT,
    XGE_XUI_DOCK_SIDE_TOP,
    XGE_XUI_DOCK_SIDE_BOTTOM,
    XGE_XUI_DOCK_SIDE_FILL
} xge_xui_dock_side_t;

typedef enum xge_xui_dock_window_state_t {
    XGE_XUI_DOCK_WINDOW_FLOATING,
    XGE_XUI_DOCK_WINDOW_DOCKED,
    XGE_XUI_DOCK_WINDOW_HIDDEN
} xge_xui_dock_window_state_t;
```

内部字段原则：

- `dock_layout` 持有 region 数组、floating window array、registered window dict、drag state、style cache。
- `dock_region` 持有 kind、root node、rect、visible、portion、min/max size。
- `dock_node` 持有 type、parent、rect、minSize、splitterRect；split 节点持有 axis/ratio/first/second，pane 节点持有 pane。
- `dock_pane` 持有 owner layout、owner node、tab array、active index、rect、tabStripRect、captionRect、clientRect、button rects、hover/active part。
- `dock_window` 持有 owner layout、owner pane、state、xge_xui_window_t、client widget、title/icon、flags、lastFloatRect、lastDockPath。

XRT 复用要求：

- [x] 动态数组优先使用 `xrtArray*` 或项目内已有 XRT array 包装。
- [x] window id/name 索引优先使用 `xrtDict*`。说明：DockLayout load-state 现在为已注册 dockwindow 构建 XRT id/name 字典，windows/tabs/floating 解析均走该索引，并拒绝重复 id/name。
- [ ] 临时 hit path 可使用栈上固定数组；超限时再使用 XRT 分配。
- [x] 所有 dock 对象生命周期统一由 DockLayout 或显式 `Init/Unit` 管理，不允许散落 `malloc/free`。
- [x] 序列化对象不得直接保存裸指针，只保存 region、node path、window id、比例和状态。

## 所有权与生命周期

- [x] `xgeXuiDockLayoutInit` 初始化 DockLayout 控件、region、style 默认值和 drag state。
- [x] `xgeXuiDockLayoutUnit` 释放 region tree、pane、内部数组、overlay indicator 和注册关系。
- [x] `xgeXuiDockWindowInit` 初始化 dockwindow 并创建/绑定内部 `xge_xui_window_t`。
- [x] `xgeXuiDockWindowUnit` 必须先从 pane/floating 列表中移除，再释放内部资源。
- [x] client widget 的所有权默认仍由调用方或 window 管理，DockPanel 只负责 reparent。
- [x] reparent 必须走现有 `xgeXuiWidgetRemove` / internal add 路径，避免一个 widget 同时挂在两个父节点下。
- [x] 停靠时将 dockwindow client widget 挂到 dockpane client slot。
- [x] 浮动时将 client widget 挂回内部 floating window client slot。
- [x] 隐藏时保留 dock model 归属，widget 可从可见树移除或设置 hidden，但不能丢失恢复路径。
- [x] layout 销毁时必须取消进行中的 drag、释放 capture、隐藏 overlay indicator。

## 布局模型

DockLayout 外层区域：

- [x] DockLayout 使用自身 content rect 作为 docking 工作区。
- [x] V1 固定 region 顺序：left、right 先切宽，top、bottom 在剩余矩形切高，document 填满最后区域。
- [x] side region 不可见或 root 为空时不占用空间。
- [x] side region portion 表示相对 DockLayout 当前工作区的比例，必须经过 min/max clamp。
- [x] document region 默认始终可见，但允许为空。
- [x] 后续如需要 DockPanelSuite 更精确的 region 排列，应新增决策记录，不直接改语义。

DockNode split tree：

- [x] split 节点 axis 为 horizontal/vertical，ratio 表示 first child 所占比例。
- [x] ratio clamp 后必须满足 first/second 的最小尺寸。说明：split arrange 与 splitter drag 均使用子节点 axis min 约束 ratio，过窄时按 min 比例降级。
- [x] pane 节点最小尺寸来自 tab strip、caption、button、client min size。说明：pane V1 min 回传 tab/chrome/button 基线，并合并 client/content desired size。
- [x] splitter rect 只存在于 split 节点，并参与 hit-test 与 pointer capture。说明：splitter 命中走 DockLayout 专用 hit-test，优先于 pane/tab 命中，并使用 XUI pointer capture。
- [x] 拖动 splitter 时只修改当前 split ratio，不重排整棵树。说明：拖动过程中只更新被命中的 split node `fRatio`，通过 dirty layout/paint 在下一帧重排。
- [x] pane 被移除到空时，父 split 应被 sibling 提升折叠。
- [x] region root 折叠为空时，side region 自动隐藏或不占用空间。

Pane 布局：

- [x] tab strip 高度、caption 高度、button 尺寸进入 theme/style token。说明：DockLayout 初始化默认 chrome metrics，pane 与 auto-hide expand overlay 均通过同一组度量读取。
- [x] single tab 是否显示 tab strip 作为策略项，V1 默认显示，避免停靠/拖拽入口消失。
- [x] active tab client widget 填充 pane client rect。
- [x] inactive tab client widget hidden 或 detached，但必须保留状态。
- [x] close/pin/dock/options button rect 稳定，不因标题变化挤压到负宽。
- [x] tab overflow 第一版可以显示 overflow button，菜单行为进入后续阶段。

Floating 布局：

- [x] floating dockwindow 使用 overlay root 或 floating layer 挂载。
- [x] floating rect 必须 clamp 到 XUI root 工作区。
- [x] floating dockwindow 可拖动、resize、bring-to-front。
- [x] floating window 被拖向 docklayout 时进入 dock drag loop。
- [x] floating 状态仍参与 DockLayout 注册，不能成为普通独立 window。

## 公开 API 草案

DockLayout：

```c
XGE_API int xgeXuiDockLayoutInit(xge_xui_dock_layout_t* pLayout, xge_xui_context_t* pContext, xge_xui_widget_t* pWidget);
XGE_API void xgeXuiDockLayoutUnit(xge_xui_dock_layout_t* pLayout);
XGE_API xge_xui_widget_t* xgeXuiDockLayoutWidget(xge_xui_dock_layout_t* pLayout);
XGE_API xge_xui_dock_pane_t* xgeXuiDockLayoutDockWindow(
    xge_xui_dock_layout_t* pLayout,
    xge_xui_dock_window_t* pWindow,
    xge_xui_dock_region_kind_t eRegion,
    xge_xui_dock_side_t eSide,
    float fProportion);
XGE_API int xgeXuiDockLayoutFloatWindow(
    xge_xui_dock_layout_t* pLayout,
    xge_xui_dock_window_t* pWindow,
    xge_rect_t tRect);
XGE_API int xgeXuiDockLayoutHideWindow(
    xge_xui_dock_layout_t* pLayout,
    xge_xui_dock_window_t* pWindow);
XGE_API void xgeXuiDockLayoutSetRegionPortion(
    xge_xui_dock_layout_t* pLayout,
    xge_xui_dock_region_kind_t eRegion,
    float fPortion);
```

DockWindow：

```c
XGE_API int xgeXuiDockWindowInit(xge_xui_dock_window_t* pWindow, xge_xui_context_t* pContext);
XGE_API void xgeXuiDockWindowUnit(xge_xui_dock_window_t* pWindow);
XGE_API xge_xui_window_t* xgeXuiDockWindowBaseWindow(xge_xui_dock_window_t* pWindow);
XGE_API xge_xui_widget_t* xgeXuiDockWindowClientWidget(xge_xui_dock_window_t* pWindow);
XGE_API void xgeXuiDockWindowSetClientWidget(xge_xui_dock_window_t* pWindow, xge_xui_widget_t* pClient);
XGE_API void xgeXuiDockWindowSetTitle(xge_xui_dock_window_t* pWindow, const char* szTitle);
XGE_API void xgeXuiDockWindowSetIcon(xge_xui_dock_window_t* pWindow, xge_texture_t* pIcon);
XGE_API void xgeXuiDockWindowSetClosable(xge_xui_dock_window_t* pWindow, int bClosable);
XGE_API void xgeXuiDockWindowSetDockable(xge_xui_dock_window_t* pWindow, int bDockable);
XGE_API xge_xui_dock_window_state_t xgeXuiDockWindowGetState(const xge_xui_dock_window_t* pWindow);
```

DockPane：

```c
XGE_API int xgeXuiDockPaneGetWindowCount(const xge_xui_dock_pane_t* pPane);
XGE_API xge_xui_dock_window_t* xgeXuiDockPaneGetWindow(const xge_xui_dock_pane_t* pPane, int iIndex);
XGE_API xge_xui_dock_window_t* xgeXuiDockPaneGetActiveWindow(const xge_xui_dock_pane_t* pPane);
XGE_API void xgeXuiDockPaneSetActiveIndex(xge_xui_dock_pane_t* pPane, int iIndex);
XGE_API int xgeXuiDockPaneGetActiveIndex(const xge_xui_dock_pane_t* pPane);
```

XUI Builtin Assets：

```c
XGE_API int xgeXuiBuiltinAssetGetCount(void);
XGE_API int xgeXuiBuiltinAssetGetRect(const char* sName, xge_rect_t* pRect);
```

API 落地规则：

- [ ] 初始公开 API 保持最小可用，避免一次性暴露所有内部 node 操作。
- [ ] 需要调试/测试的内部操作先放 internal helper，不急于导出。
- [ ] 所有 setter 必须明确 dirty layout、dirty paint 或无副作用。
- [ ] 所有 API 对 NULL 输入的策略必须与现有 XUI 控件一致。
- [ ] API 文档必须说明 dockwindow 与普通 `xge_xui_window_t` 的组合关系。

## 绘制与主题

DockLayout 绘制：

- [ ] region 背景跟随工具界面主题，不做强装饰。
- [x] split gutter 与 splitter 绘制复用 SplitLayout 风格或 dock 专用 token。说明：V1 使用 dock 专用浅蓝 splitter 色与边线，后续主题 token 化进入打磨阶段。
- [ ] document region 空态可显示轻量占位背景，但不得成为落地依赖。

DockPane 绘制：

- [ ] tab strip 支持 normal/hover/active/disabled。
- [ ] active tab 与 pane client 边框视觉相连。
- [ ] caption/title 区域与 tab strip 的关系必须固定，避免标题与按钮重叠。
- [ ] close/pin/dock/options 使用 atlas 资源。
- [ ] button hover/active 使用 XUI theme 状态色，不改原始资源。
- [ ] tab 过多时先压缩宽度到最小，再显示 overflow button。

DockIndicator 绘制：

- [ ] pane diamond 使用 VS2005 pane diamond 系列资源。
- [x] 全局 panel indicator 使用 VS2005 panel 系列资源。说明：drag overlay 根据 docklayout 全局 side/fill 绘制 panel indicator 资源。
- [ ] active 指示由 active 资源或 overlay highlight 切换。
- [ ] preview rect 使用半透明填充和边框，默认接近 DockPanelSuite 蓝色反馈。
- [ ] indicator 挂载到 overlay root，layer 使用 drag adorner。
- [ ] indicator 自身 input transparent，避免吞掉拖拽事件。

Fallback：

- [x] 缺少 atlas 资源时，pane diamond fallback 用几何菱形/箭头绘制。说明：pane indicator draw 失败时绘制固定五块几何目标，并按当前 side 高亮。
- [x] 缺少 dock pane button 资源时，close/pin/dock/options 使用已有内建 icon 或几何绘制。说明：close/pin/dock/options/overflow 均有 bitmap-mask fallback。
- [x] fallback 必须不影响命中逻辑。说明：fallback 只在 paint path 生效，hit-test 仍使用既有 rect 与 side 计算。

## 事件与拖拽状态机

拖拽入口：

- [ ] floating dockwindow 标题栏拖动可进入 dock drag。
- [ ] dockpane tab 拖动可进入 dock drag。
- [ ] dockpane caption/title 拖动 active tab 可进入 dock drag。
- [x] splitter 拖动不进入 dock drag，只调整 split ratio。说明：splitter mouse/touch down 优先进入 splitter drag state，不创建 dock drag overlay。
- [x] 非 dockable window 不显示 indicator，不允许 dock drop。说明：`bDockable=0` 时 tab/title drag 不进入 pending/dragging，专用 smoke 验证不会显示 overlay、preview 或改变 dock 状态。

拖拽状态：

```c
typedef enum xge_xui_dock_drag_phase_t {
    XGE_XUI_DOCK_DRAG_IDLE,
    XGE_XUI_DOCK_DRAG_PENDING,
    XGE_XUI_DOCK_DRAG_DRAGGING,
    XGE_XUI_DOCK_DRAG_COMMITTING,
    XGE_XUI_DOCK_DRAG_CANCELING
} xge_xui_dock_drag_phase_t;
```

拖拽状态字段：

- `dragWindow`
- `sourcePane`
- `sourceTabIndex`
- `sourceState`
- `hoverKind`
- `hoverPane`
- `hoverRegion`
- `dockSide`
- `previewRect`
- `indicatorRect`
- `indicatorAsset`
- `startMouse`
- `lastMouse`
- `dragOffset`
- `pointerId`
- `modifier`

状态机：

- [ ] begin drag：记录 source、设置 pointer capture、bring floating/adorner to front、初始化 overlay indicator。
- [ ] update hover target：使用专用 dock hit-test，不依赖普通 widget target。
- [x] show indicator：根据 hover pane/region 显示 pane diamond 或 panel indicator。说明：pane hover 显示 diamond，docklayout region hover 显示 panel indicator。
- [x] update preview rect：根据目标 side/fill 计算最终停靠区域。说明：pane side/fill 与 docklayout 全局 side/fill 均有 preview rect。
- [x] commit drop：释放 capture、隐藏 indicator、执行一次模型变更、dirty layout/paint。说明：pane fill、pane side split、global region drop 已进入专用 smoke。
- [x] cancel：释放 capture、隐藏 indicator、恢复 source 状态和原 rect。说明：Escape、capture lost/cancel 和 drop failure 均通过专用 smoke 覆盖 source 保持。
- [x] capture lost：按 cancel 处理。说明：capture cancel/lost 进入统一 cancel 路径并释放 pointer capture。
- [x] Escape：按 cancel 处理。说明：Escape 通过 XUI capture 释放路径专测通过。
- [x] Ctrl 或配置项可临时禁用 docking，仅保持 floating drag。说明：mouse/touch move 带 `XGE_KEY_MOD_CTRL` 时清空 pane/global hover target，只保留 floating preview rect，mouse up 提交为 floating。

命中顺序：

- [ ] 当前 pointer 位于 pane diamond active hit 区时，优先 pane-level drop。
- [x] 其次命中 dockpane tab strip，可作为 tab reorder 或 tab insert。说明：同 pane tab strip hover 会进入 reorder preview，优先于 pane/global docking drop。
- [x] 其次命中 DockLayout 全局 panel indicator，可作为 region-level drop。说明：docklayout content 外沿 side 命中会映射到 left/right/top/bottom 独立 region。
- [ ] 其次命中已有 pane 边缘，可作为 pane split drop。
- [x] 无命中时保持 floating preview 或仅移动 floating window。说明：docked tab 拖出 docklayout 无命中时显示 root-clamped floating preview，mouse up 后成为 floating dockwindow。
- [ ] modal overlay 存在时，只有属于当前 DockLayout 或 drag adorner 的命中可参与 docking。

几何 hit-test：

- [ ] pane diamond left/right/top/bottom/fill 使用固定五块几何区域。
- [x] panel indicator left/right/top/bottom/fill 使用各自资源 rect。说明：V1 使用固定 31px panel indicator 与 side/fill 资源切换；精确位置手感在阶段 E 调整。
- [x] tab hit-test 使用 tab rect array，不扫字符串。说明：tab click 与 reorder insertion 均基于已计算 tab rect。
- [ ] pane split hit-test 使用 pane content/caption rect 的边缘阈值。
- [ ] region hit-test 使用 region rect 与 docklayout content rect。

Drop 行为：

- [ ] fill drop 到 pane：加入目标 pane tab。
- [ ] left/right/top/bottom drop 到 pane：创建 split 节点，目标 pane 与新 pane 成为 siblings。
- [ ] left/right/top/bottom drop 到 region：插入或创建 region root。
- [ ] document fill drop：加入 document region 的当前 pane 或创建 document pane。
- [ ] 从 source pane 移出最后一个 tab 时，折叠 source pane。
- [ ] commit 过程必须防止 source 与 target 相同导致重复插入。
- [x] drop 失败必须回滚到拖拽前状态。说明：commit 前 dockwindow 变为 non-dockable 时取消 drag、隐藏 overlay、释放 capture，source pane/tab/floating 状态保持不变；内存分配失败注入后续统一测试设施再补。

## Tab 行为

- [x] 鼠标点击 tab 设置 active window。说明：专用 smoke 已覆盖点击 Output tab 激活。
- [x] tab close button 关闭对应 dockwindow，若不可关闭则 disabled。说明：pane close 点击会隐藏 active dockwindow；active window 不可关闭时 V1 隐藏 close rect，专用 smoke 覆盖。
- [x] 中键关闭 tab 可评估，V1 非必需。说明：已实现 DockPanelSuite 风格中键关闭可关闭 tab；右键 tab 不进入 dock drag，不抢 pointer capture，专用 smoke 覆盖。
- [x] tab 拖拽开始阈值沿用 Widget DragBegin 阈值。说明：复用 DockPanel drag pending -> dragging 阈值。
- [x] tab reorder 支持同 pane 内部重新排序。说明：同 pane tab strip drop 会重排 `arrWindows` 并保持 moved tab active。
- [x] tab 从 pane 拖出但无 docking 命中时变为 floating dockwindow。说明：专用 smoke 覆盖 docked tab 拖出到 docklayout 外部、commit 为 floating、再 dock 回原 pane。
- [x] tab title 过长时 ellipsis，不撑开 pane。说明：tab 绘制使用本地省略截断，pane 宽度不足时仍裁剪在 tab rect 内。
- [x] tab tooltip 显示完整 title。说明：DockLayout tooltip resolver 对 tab/caption 返回完整 dockwindow title，专用 smoke 已覆盖 `Output`。
- [x] active window 隐藏/关闭后选择邻近 tab。说明：关闭 Output 后 active 回到 Document；关闭最后一个 tab 时已有空 pane collapse 路径。

## Pane 按钮行为

- [x] close：关闭 active dockwindow 或关闭 pane 中当前 tab。说明：pane close 按钮关闭当前 active dockwindow；完整 tab 逐项 close 菜单进入后续 option/overflow。
- [x] auto-hide：V1 可以显示 disabled 或标记为后续阶段；实现 auto-hide 前不得误导用户。说明：side/split pane 的 active dockwindow 可通过 auto-hide button 收入边缘 strip；document fill pane 仍 disabled，不参与 hit-test/capture。
- [x] dock/pin：在 auto-hide 与 docked 状态之间切换；V1 如未实现 auto-hide，应隐藏或 disabled。说明：auto-hide strip item 点击打开临时 overlay pane；overlay 内 dock/pin button 可恢复到保存的 region/side。
- [x] option：打开 pane/window 菜单，第一版可提供 close/close all/float/dock 等最小命令。说明：pane option button 现在打开 XUI menu，提供 Float、Dock 占位、Close、Close Others、Close All，并覆盖启用/禁用状态与键盘选择。
- [x] overflow：tab 溢出时打开 tab 列表菜单。说明：pane 宽度不足时显示 overflow button，打开 XUI menu 列出当前 pane 全部 tab，并以 checked 状态标出 active tab，选择项可切换 active window。
- [~] 所有按钮支持 hover/active/disabled、tooltip、keyboard/focus 策略。说明：close/option/overflow/auto-hide 支持 hover/active、tooltip 与 pointer capture；auto-hide expand 已覆盖 focus/capture/close，完整 focus ring 与 DockPanelSuite 级 hover 延迟调优待补。

## Auto-Hide 规划

Auto-hide 不阻塞 V1 完成，但必须预留结构：

- [x] dockwindow flags 预留 auto-hide state。说明：新增 `XGE_XUI_DOCK_WINDOW_AUTO_HIDE`、`iAutoHideRegion` 与 strip rect。
- [x] DockLayout 预留 left/right/top/bottom auto-hide strip。说明：layout arrange 会为存在 auto-hide dockwindow 的 side 预留 23px strip，并从 docking content rect 扣除。
- [x] auto-hide strip item 复用 dockwindow title/icon。说明：strip item 绘制 dock icon、title、hover/active 背景；left/right strip 先以 clipped title 表现。
- [x] hover/click expand 使用 overlay 或临时 pane。说明：strip click 打开 `XGE_XUI_LAYER_DRAG_ADORNER` overlay pane，client widget 临时 reparent 到 overlay。
- [x] expand pane 不修改原 split tree，收起后恢复 strip。说明：expand 只记录 `pAutoHideExpandWindow` 和临时 rect，close/outside/Escape 收起后仍保持 `AUTO_HIDE`。
- [x] pin/dock button 可将 auto-hide window 恢复为 docked pane。说明：auto-hide button 收起 active window，overlay dock button 调用 `xgeXuiDockLayoutDockAutoHideWindow` 恢复原 region/side。
- [x] Auto-hide 阶段需要单独补充验收口径。说明：专用 smoke 覆盖 auto-hide button、strip layout、strip expand、overlay close、overlay dock restore、focus/capture；lab `--auto-hide-preview` 提供截图路径。

## 序列化规划

序列化目标：

- [x] 保存 DockLayout region 顺序、portion、visible。说明：`xgeXuiDockLayoutSaveState` 输出固定顺序 `regions[]`，包含 `kind/portion/visible/root`。
- [x] 保存每个 region 的 binary split tree。说明：region `root` 递归保存 `split` / `pane` 节点。
- [x] 保存 split axis、ratio、child path。说明：split 节点保存 `axis/ratio/first/second`。
- [x] 保存 pane tab window id 列表和 active tab id。说明：pane 节点保存 `tabs[]` 和 `active`。
- [x] 保存 floating dockwindow rect、z-order、visible state。说明：`floating[]` 保存 `id/rect/z`，`windows[]` 保存 `state/visible`。
- [~] 保存 dockwindow lastDockPath 和 lastFloatRect。说明：已保存 `lastRegion/lastSide/lastTabIndex/lastFloatRect`；完整 node path 等 load 需要的路径结构留到下一阶段。
- [x] 不保存裸指针、runtime rect、hover/drag 状态。说明：save-state 只输出 id、region、split、tab、rect 和状态字段。

建议格式：

- [x] C API 提供 save/load 到 XValue 或 XSON object 的内部能力。说明：已提供 `xgeXuiDockLayoutSaveState` / `xgeXuiDockLayoutLoadState` / `StateFree` / `StateGetCounts`；LoadState 先构建临时 split tree 与 floating 列表，验证 window id 后一次性提交。
- [x] XSON 页面声明可创建初始 dock layout。说明：`type:"dockLayout"` 已接入 page loader，并由 `examples\xui_dockpanel_xson` 覆盖 docked/floating/hidden 初始状态。
- [x] 运行时 layout 持久化可由应用保存到独立配置。说明：运行时 XValue save/load 已闭环，应用层可自行将 XValue/XSON 写入配置文件。
- [x] load 失败必须保持原 layout 不变或完全回滚。说明：LoadState 在提交前完成格式、region、node、tab id、floating id 与状态一致性校验；NULL/错误 state 返回错误且不修改当前布局。

## XSON 声明式规划

V1 XSON 能力：

- [x] 支持 `dockLayout` 控件类型。说明：XSON `type:"dockLayout"` 已接入 page loader，创建 `xge_xui_dock_layout_t` 并纳入 page unload。
- [x] 支持 region 初始比例和可见性。说明：`regions.left/right/top/bottom` 初始 portion 可声明；region 可见性由初始 dockwindow placement 驱动。
- [x] 支持 pane 初始 tab 列表。说明：通过 `dockWindows[]` 的声明顺序、`region` 和 `side:"fill"` 初始化 pane tabs；显式 pane/node XSON roundtrip 留到持久化阶段。
- [x] 支持 dockwindow title/id/closable/dockable。说明：`dockWindows[]` 支持 `id`、`title`、`closable`、`dockable`，非 dockable 初始 docked window 会先创建停靠状态再关闭后续拖拽能力。
- [x] 支持将已有声明式控件挂入 dockwindow client。说明：`dockWindows[].children` 使用常规 XSON widget builder，并通过 dockwindow client slot 承载。
- [x] 支持错误路径指向具体 region/node/window 字段。说明：类型、children、state、rect 和 placement 错误会带 `dockWindows[i]` 或字段路径。
- [x] 不在 XSON 中声明拖拽运行态。说明：XSON 只声明初始 docked/floating/hidden 状态，不接受 hover/drag/capture 运行态。

示例形态草案：

```json
{
  "type": "dockLayout",
  "regions": {
    "left": {
      "portion": 0.22,
      "root": {
        "type": "pane",
        "tabs": ["project", "assets"],
        "active": "project"
      }
    },
    "document": {
      "root": {
        "type": "pane",
        "tabs": ["editor"]
      }
    }
  },
  "windows": {
    "project": { "title": "Project", "dockable": true, "closable": true },
    "assets": { "title": "Assets", "dockable": true, "closable": true },
    "editor": { "title": "Editor", "dockable": true, "closable": false }
  }
}
```

## 示例与验证目标

示例：

- [x] 新增 `examples/xui_dockpanel_lab/`，展示 document、left、right、bottom 四类区域。说明：示例已纳入 `build_examples_xui.bat`。
- [x] 示例包含至少 5 个 dockwindow：project、properties、output、editor、preview。说明：当前为 Document.c、Output、Toolbox、Properties、Preview。
- [x] 示例支持 tab 切换、pane split、floating、dock back、close。说明：静态 tab/split/close、close preview、splitter ratio preview、tab drag preview、floating title dock-back preview 均已实现并通过 smoke。
- [x] 示例显示 DockPanelSuite 风格 indicator 与 preview rect。说明：`build\xui_dockpanel_lab.exe --drag-preview`、`--float-dock-preview` 与 `--global-dock-preview` 会保持拖拽 overlay 可见，截图已验证。
- [x] 示例提供 smoke 参数，能自动创建布局并跑若干帧退出。说明：`build\xui_dockpanel_lab.exe --frames 3` 与各 preview 模式均输出 `create=1 layout=1 state=1 labels=17`。
- [x] 示例手动启动时不自动退出。说明：截图验证使用无 `--frames` 启动示例，进程保持窗口直到截图脚本主动结束。

测试：

- [x] 单元测试覆盖 split tree 插入、移除、折叠、ratio clamp。说明：专用 smoke 覆盖 fill 插入、pane-local side split、splitter ratio drag、ratio min/max clamp、source pane 折叠与 float/hide 移除，并补充二级 split 中 nested pane/root sibling 折叠。
- [x] 单元测试覆盖 dockwindow dock/float/hide 状态迁移。说明：`test/xui_dockpanel_smoke.c` 已覆盖并通过。
- [x] 单元测试覆盖 source pane 最后一个 tab 移出后的树折叠。说明：专用 smoke 统计 document region pane 数量，验证最后 tab float 后从 3 个 pane 折叠回 2 个 pane。
- [x] 单元测试覆盖 tab active index 变化。说明：`test/xui_dockpanel_smoke.c` 覆盖 setter 与鼠标点击激活。
- [x] 单元测试覆盖 drag cancel 后 source 状态恢复。说明：专用 smoke 通过 Escape 释放 capture 并验证 dockwindow 仍在原 pane、floating 列表未变化。
- [x] 单元测试覆盖 atlas asset rect 能查询到。说明：新增 `xgeXuiBuiltinAssetGetRect` / `xgeXuiBuiltinAssetGetCount`，专用 smoke 覆盖 dock indicator 与 dock pane button 资源 rect。
- [x] 如测试框架不适合 UI drag，全流程用示例 smoke + debug trace 验证。说明：当前已有专用 smoke、示例 smoke、non-dockable drag 负向覆盖、tab float preview/drag preview/float dock preview/global dock preview/splitter preview 截图与像素检查。

人工验证：

- [ ] Windows 手动验证拖拽到 pane diamond 五个方向。
- [ ] Windows 手动验证拖拽到全局 left/right/top/bottom/fill indicator。
- [ ] Windows 手动验证 tab 拖出浮动、浮动窗口再停靠。
- [ ] Windows 手动验证关闭 active tab 后 active 选择正确。
- [ ] Windows 手动验证 DLL 未被占用时 `build_dll.bat` 通过。
- [ ] Windows 手动验证 `build_examples_all.bat --xui` 通过。

## 阶段 0：SPEC 与资源基线

- [x] 确认 DockPanelSuite 数据模型。说明：其核心为 DockPanel、DockWindow、NestedPaneCollection、DockPane，分割关系通过 previousPane/alignment/proportion 表达。
- [x] 确认 XUI 内部采用显式二叉 split tree。
- [x] 确认 dockwindow 组合现有 `xge_xui_window_t`。
- [x] 确认不支持 OS/native 外部窗口。
- [x] 确认 VS2005 素材进入项目资源目录。
- [x] 确认 hotspot 图不用加入 atlas。
- [x] 完成 atlas 资源合并与宏生成。
- [x] 新增本文档作为 tracked SPEC。
- [ ] 重新验证 DLL 构建与 atlas 生成脚本，在 DLL 未被占用时关闭资源基线。

## 阶段 A：类型、API 与内部骨架

- [x] 在 `xge.h` 声明 docklayout/dockwindow/dockpane 基础类型和最小公开 API。说明：已加入 DockLayout/DockRegion/DockNode/DockPane/DockWindow 类型、常量和 Phase A API。
- [x] 新增 dockpanel 实现文件，接入 `src/xge_impl.c` 编译聚合。说明：已新增 `src/xge_xui_dockpanel.c` 并纳入单翻译单元构建。
- [x] 定义 region/node/pane/window/drag state 内部结构。说明：已落地结构字段和 drag phase 基础字段；拖拽完整状态机进入阶段 D。
- [x] 接入 XRT array/dict/list 和统一内存释放路径。说明：pane/layout window 列表使用 `xarray_struct`，node/pane 使用 XRT 分配释放。
- [x] 实现 DockLayout Init/Unit、Widget 获取、默认 region 初始化。说明：document region 默认可见，side region 有默认比例与最小尺寸。
- [x] 实现 DockWindow Init/Unit、title/client/flags 基础 setter。说明：dockwindow 组合 `xge_xui_window_t`，默认隐藏，显式 dock/float 后进入对应状态。
- [x] 建立 dockwindow 注册表，防止同一 dockwindow 重复注册到多个 layout。说明：不同 layout 重复注册返回 invalid argument。
- [x] 添加最小 smoke 测试：创建 layout、window、unit 不泄漏不崩溃。说明：已加入 `test/test_main.c`，并新增可独立构建的 `test/xui_dockpanel_smoke.c`；`test/build_xui_dockpanel_smoke.bat` 和 `build\xge_xui_dockpanel_smoke.exe` 验证通过。全量 `build_test.bat` 被既有测试/API 漂移阻断，阻塞另记。
- [x] 更新公开 API 文档。说明：`docs/api/xui.md` 与 `docs/api/xui.en.md` 已补 DockPanel / DockLayout 公开 API、调用顺序、状态/所有权和交互边界说明。

## 阶段 B：静态 DockLayout、Region 与 Split Tree

- [x] 实现 DockLayout measure/arrange/paint 基础路径。说明：DockLayout 注册 measure/layout/paint/event proc；`build\xui_dockpanel_lab_window.png` 已截图核对。
- [x] 实现五个 region 的矩形分配与 portion clamp。说明：left/right/top/bottom/document 按固定顺序切分，side portion 经过 min/max clamp。
- [x] 实现 dock_node split/pane 创建、销毁、折叠。说明：支持 pane 节点、split 节点、空 pane 折叠与 region root 置空。
- [x] 实现 split tree arrange，计算 pane rect 和 splitter rect。说明：已计算 pane rect/client rect/tab strip rect/splitter rect，后续 splitter 拖动另列。
- [x] 实现 pane 最小尺寸回传到 split tree。说明：pane 递归更新 `fMinWidth/fMinHeight`，split 节点按 axis 聚合子树 min，并约束 arrange/drag ratio。
- [x] 实现 splitter pointer capture 与 ratio 调整。说明：splitter drag 已接入 mouse/touch move/up/cancel/capture lost，专用 smoke 与 `--splitter-preview` 截图验证通过。
- [x] 实现 pane 创建、tab array、active tab、client rect。说明：active tab client 填充 pane client，inactive client hidden。
- [x] 实现 DockLayoutDockWindow 到空 region、已有 pane fill、已有 pane side。说明：fill 合并为 tab，side 在 region root 上拆分出新的 split；任意深度 pane-side 命中进入阶段 D drop target。
- [x] 添加 split tree 单元测试。说明：`test/xui_dockpanel_smoke.c` 覆盖 fill tab、right side split、paint、float/hide；`test/build_xui_dockpanel_smoke.bat` 通过。

## 阶段 C：DockWindow 状态迁移与 Pane Tabs

- [x] 实现 dockwindow docked/floating/hidden 三态迁移。说明：专用 smoke 覆盖 dock -> float -> hide。
- [x] 实现停靠时隐藏普通 window chrome，由 pane 绘制 tab/title/chrome。说明：停靠时关闭 base window，pane 绘制 tab/title/button chrome。
- [~] 实现浮动时恢复 dockwindow 特殊 window 外观。说明：已恢复 base window、client reparent、root clamp 和 bring-to-front；DockPanelSuite 级特殊外观细节进入阶段 E。
- [x] 实现 client widget 在 dockpane client slot 与 window client slot 间 reparent。说明：停靠挂到 DockLayout widget，浮动/隐藏挂回 dockwindow base window client。
- [x] 实现 tab 绘制、active/hover/pressed 状态。说明：已绘制 active/hover/pressed 基础状态和 atlas pane buttons。
- [x] 实现 tab click 激活。说明：事件路径覆盖 mouse down/up，专用 smoke 已验证。
- [x] 实现 close button 关闭 active dockwindow。说明：pane close 命中调用 `xgeXuiDockLayoutHideWindow`；关闭后 active 选择邻近 tab，非 closable active 不显示 close hit rect。
- [x] 实现 option/overflow button 的最小 disabled 或菜单占位策略。说明：option 已升级为 pane menu 入口；auto-hide 以 disabled 态绘制且不参与 hover/press/capture；overflow 行为进入阶段 E。
- [x] 实现 floating rect root clamp 与 bring-to-front。说明：`xgeXuiDockLayoutFloatWindow` clamp 到 root rect 并调用 `xgeXuiWindowBringToFront`。
- [x] 添加 dock/float/hide 状态迁移测试。说明：`test/xui_dockpanel_smoke.c` 已覆盖，并通过 `build\xge_xui_dockpanel_smoke.exe`。

## 阶段 D：拖拽、Indicator 与 Drop Commit

- [~] 实现 dock drag manager。说明：已落地 docked tab/caption 与 floating title 的 pending/dragging/commit/cancel 基础状态，并补齐同 pane tab reorder 与 Ctrl 抑制停靠；DockPanelSuite 级全局 indicator 细节继续补。
- [~] 接入 Widget DragBegin/Move/End/Cancel 和 pointer capture。说明：当前直接接入 XUI pointer capture 与 mouse/touch down/move/up/cancel；是否复用 XUI synthetic drag event 另做评估。
- [x] 实现从 floating title/caption/tab 发起拖拽。说明：docked tab、docked caption 空白区 active tab、floating title 均已进入 dock drag；V1 没有独立 floating tab strip。
- [x] 实现 overlay indicator widget 创建、显示、隐藏和资源绘制。说明：drag overlay 挂到 `XGE_XUI_LAYER_DRAG_ADORNER`，input transparent，绘制 VS2005 pane diamond / panel indicator。
- [x] 实现 pane diamond 几何 hit-test。说明：基于 pane rect 边缘区和中心 fill 几何命中，不依赖 hotspot 图。
- [x] 实现 panel indicator 几何 hit-test。说明：layout 全局 side/fill 命中已实现，left/right/top/bottom 映射到独立 region；DockPanelSuite 级边缘位置微调转入阶段 E。
- [x] 实现 preview rect 计算和绘制。说明：overlay 绘制半透明蓝色 drop preview。
- [x] 实现 commit drop 到 pane fill。说明：fill 复用 `xgeXuiDockLayoutDockWindow(..., FILL, ...)` 合并 tab。
- [x] 实现 commit drop 到 pane side split。说明：hover pane 边缘拖放现在提交到 pane-local binary split，不再统一拆 document region root。
- [x] 实现 commit drop 到 global region。说明：floating dockwindow 拖到 docklayout 全局 left side 可提交到 left region，专用 smoke 已覆盖。
- [x] 实现 cancel、Escape、capture lost 回滚。说明：touch cancel/capture lost 已取消；Escape 通过 XUI capture 释放路径专测通过。
- [x] 实现 source pane 移出最后 tab 后折叠。说明：空 pane collapse 路径已有，专用 smoke 已覆盖拖拽/float 后的树折叠。
- [x] 添加拖拽状态机测试或 smoke 验证。说明：`test/xui_dockpanel_smoke.c` 覆盖 pending、drag overlay、pane-local right preview/drop commit、floating title dock-back、global left region drop、Escape cancel。

## 阶段 E：DockPanelSuite 手感补齐

- [ ] 调整 indicator 位置、尺寸、透明度和 active feedback 到接近 DockPanelSuite VS2005。
- [ ] 调整 preview rect 填充/边框颜色到接近 DockPanelSuite。
- [x] 实现 tab reorder。说明：已支持同 pane 内部 tab reorder，包含插入线 preview、专用 smoke 和 `--tab-reorder-preview` 示例截图。
- [x] 实现 tab overflow 菜单。说明：tab overflow button 按需显示，菜单列出 pane tab，active tab 使用 radio checked 状态，选择后同步 active/focus。
- [x] 实现 pane option 菜单：float、dock、close、close all、close others。说明：复用 XUI menu，Float 可将 active dockwindow 转为 floating，Close/Close Others/Close All 走 hide/collapse 路径，Dock 在 pane 菜单中作为 disabled 状态占位。
- [x] 实现 dockwindow focus 与 active tab 同步。说明：DockPane active 变更、close fallback、dock/float commit 通过 pending focus window 同步到 active dockwindow 的 focusable content/client；专用 smoke 验证 Document/Output 切换与关闭恢复。
- [x] 实现 keyboard 行为：Esc cancel drag、Ctrl 策略、Tab focus 不穿透 overlay。说明：Esc 显式取消 DockLayout drag/splitter 并释放 capture；XUI core 在 capture 存在时消费 Tab，避免 overlay 期间 focus 穿透；Ctrl move 抑制 docking 仅保留 floating preview。
- [x] 实现 tooltip：tab title、pane buttons、indicator 可选说明。说明：DockLayout tooltip resolver 根据鼠标位置解析 tab title、close、auto-hide、options；drag/capture 期间 tooltip 由 XUI core 自动关闭，indicator tooltip V1 不启用。
- [~] 补齐 disabled/non-dockable/readonly 状态表现。说明：non-dockable drag 禁用、auto-hide disabled 绘制与 hit-test 排除、option menu item disabled/danger 状态、overflow active checked 状态已覆盖；readonly 和 auto-hide 高级状态待补。
- [ ] 手动对照 DockPanelSuite 常见操作路径调手感。

## 阶段 F：持久化与 XSON

- [x] 定义 DockPanel layout save/load 数据结构。说明：save-state XValue table 包含 `version/regions/windows/floating`。
- [x] 实现 split tree 序列化。说明：`split` 节点递归保存 axis/ratio/first/second。
- [x] 实现 pane tabs 和 active tab 序列化。说明：`pane` 节点保存 `tabs[]` 和 active index。
- [x] 实现 floating rect/state 序列化。说明：`floating[]` 保存 rect/z，`windows[]` 保存 state/visible/lastFloatRect。
- [x] 实现 load 失败回滚。说明：`xgeXuiDockLayoutLoadState` 使用临时 root/floating 结构，校验成功后才替换当前 layout；专用 smoke 覆盖无效输入不改变 floating 状态。
- [x] 实现 `dockLayout` XSON 控件。说明：page loader 支持 `type:"dockLayout"` 并创建 DockLayout 控件实例。
- [~] 实现 XSON region/node/window 字段解析。说明：已支持 region portion 与 dockwindow id/title/state/region/side/portion/rect/closable/dockable/children；显式 binary node/split tree 字段留给持久化 roundtrip。
- [x] 实现 XSON 错误路径与资源释放。说明：DockLayout/DockWindow 纳入 page control arena unload，声明式 client children 随 page unload 释放。
- [x] 添加 XSON dockpanel 示例。说明：新增 `examples/xui_dockpanel_xson` 并接入 `build_examples_xui.bat`。
- [x] 添加持久化 roundtrip 测试。说明：专用 smoke 保存 docked+floating 混合状态，扰动布局后 LoadState 恢复 floating/docked/hidden 与 client parent。

## 阶段 G：Auto-Hide 与高级停靠

- [x] 实现 auto-hide strip 数据结构和布局。说明：dockwindow 保存 auto-hide region/strip rect，DockLayout arrange 为 left/right/top/bottom strip 预留空间并绘制 item。
- [x] 实现 dockwindow pin/dock 状态切换。说明：`xgeXuiDockLayoutAutoHideWindow` / `xgeXuiDockLayoutDockAutoHideWindow` 已接入 pane button、overlay dock button、XSON 和 smoke。
- [x] 实现 hover/click expand overlay pane。说明：strip click 展开 overlay pane，不重建或改写原 split tree；DockPanelSuite 级 hover 延迟进入视觉/手感调优。
- [x] 实现 auto-hide expand 的 focus/capture/close 策略。说明：展开后 focus 切到 dockwindow content，button press 使用 overlay capture，close/outside click/Escape 收回 strip。
- [x] 实现 auto-hide 与原 split tree 的恢复关系。说明：auto-hide 保留 `lastRegion/lastSide`，恢复时按保存的 region/side 重新 dock；document fill pane 不进入 auto-hide。
- [x] 实现 auto-hide 状态序列化。说明：save/load 保存 `state:"autoHide"` 与 `autoHideRegion`，LoadState 可恢复 auto-hide 状态并等待下一帧 layout 计算 strip rect。
- [x] 添加 auto-hide 示例和人工验证记录。说明：`examples\xui_dockpanel_xson` 新增 `state:"autoHide"` 窗口；`examples\xui_dockpanel_lab --auto-hide-preview` 自动打开 expand overlay；专用 smoke 覆盖 pane auto-hide button、strip layout、strip expand、overlay close/dock。

## 阻塞与决策记录

- [x] 决策：XUI 使用显式二叉 split tree，不照搬 DockPanelSuite previousPane/alignment/proportion。
- [x] 决策：dockwindow 组合 `xge_xui_window_t`，停靠/浮动切换 chrome 和 parent，不复制 window 系统。
- [x] 决策：拖拽必须接入 capture、overlay root 和 drag adorner layer，不依赖普通 widget hit-test。
- [x] 决策：DockPanelSuite hotspot 图不进入 atlas，XUI 使用几何命中。
- [x] 决策：VS2012 素材不进入第一版内建资源。
- [x] 已解决：`build_dll.bat` 链接 `build\xge.dll` 曾因 `Permission denied` 失败。处理：2026-05-21 重新运行 `build_dll.bat` 通过。
- [x] 验证：2026-05-21 `test/build_xui_dockpanel_smoke.bat` 通过，`build\xge_xui_dockpanel_smoke.exe` 输出 `xui_dockpanel_smoke passed tabs=1 drawRect=37 drawText=2 drawImage=6`。
- [x] 验证：2026-05-21 `build\xui_dockpanel_lab.exe --frames 3` 通过，输出 `create=1 layout=1 state=1 labels=17`。
- [x] 验证：2026-05-21 `build_examples_all.bat --xui` 通过，包含 `XUI DockPanel lab`。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab` 截图到 `build\xui_dockpanel_lab_window.png`，像素检查 `size=1040x679 nonWhite=1099 blueChrome=480 darkText=44 samples=4959`，画面包含 left/document/right/bottom pane、tab、pane buttons 和 client 内容。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab --drag-preview` 截图到 `build\xui_dockpanel_lab_drag_preview.png`，像素检查 `sample=78769 nonWhite=18679 blueOverlay=7445 leftBlue=1023 darkText=509`，画面包含 pane diamond、active right drop 和 preview rect。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab --float-dock-preview` 截图到 `build\xui_dockpanel_lab_float_dock_preview.png`，像素检查 `sample=78769 nonWhite=15069 blueOverlay=7029 leftBlue=1023 darkText=497`，画面包含 floating dockwindow、pane diamond、active fill drop 和 preview rect。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab --global-dock-preview` 截图到 `build\xui_dockpanel_lab_global_dock_preview.png`，像素检查 `sample=78769 nonWhite=15884 blueOverlay=6489 leftBlue=1147 darkText=574`，画面包含 floating dockwindow、全局 left panel indicator 和 preview rect。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab --tab-reorder-preview` 截图到 `build\xui_dockpanel_lab_tab_reorder_preview.png`，像素检查 `sample=57263 nonWhite=11149 blueOverlay=4872 tabBlue=3965 darkText=1280`，画面包含同 pane tab reorder 插入线 preview。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab --tab-float-preview` 截图到 `build\xui_dockpanel_lab_tab_float_preview.png`，像素检查 `sample=78769 nonWhite=17790 blueOverlay=7188 leftBlue=1320 darkText=513`，画面包含 docked tab 拖出后的 floating preview rect。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab --splitter-preview` 截图到 `build\xui_dockpanel_lab_splitter_preview.png`，像素检查 `sample=78769 nonWhite=17675 blueChrome=6756 darkText=807 splitterTone=822`，画面包含 splitter ratio 调整后的 left/document/right/bottom pane。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab --close-preview` 截图到 `build\xui_dockpanel_lab_close_preview.png`，像素检查 `sample=78769 nonWhite=17989 blueChrome=7046 darkText=515 tabBlue=10674 outputTabRegionNonWhite=235`，画面显示 Output tab 已关闭且 Document 保持 active。
- [x] 验证：2026-05-21 复跑 `build_dll.bat`、`test\build_xui_dockpanel_smoke.bat`、`build\xge_xui_dockpanel_smoke.exe`、`examples\xui_dockpanel_lab\build.bat`、八种 `xui_dockpanel_lab --frames 3` 模式与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 non-dockable tab drag 负向路径；`Output` 设置 `dockable=false` 后 mouse down/move/up 不进入 dock drag、不显示 overlay/preview、不改变 docked 状态。
- [x] 验证：2026-05-21 专用 smoke 增加内建 atlas rect 查询覆盖；校验 pane diamond、global panel indicator active 资源和 close/auto-hide/dock/option/overflow pane button 资源尺寸。
- [x] 验证：2026-05-21 专用 smoke 增加 drop failure rollback；拖拽已进入 pane fill hover 后将 `Output` 改为 `dockable=false`，mouse up 取消 commit 并保持原 document pane 两个 tab、无 floating、无 split。
- [x] 验证：2026-05-21 新增 atlas 查询 API 与 drop failure rollback 后复跑 `test\build_xui_dockpanel_smoke.bat`、`build\xge_xui_dockpanel_smoke.exe`、`build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 自启动 `XUI DockPanel Lab` 截图到 `build\xui_dockpanel_lab_disabled_buttons.png`，像素检查 `sample=78769 nonWhite=17989 blueChrome=14052 darkText=489 disabledGray=181`，画面包含 pane disabled button 灰色占位；八种 `xui_dockpanel_lab --frames 3` 模式均保持 `create=1 layout=1 state=1 labels=17`。
- [x] 验证：2026-05-21 disabled pane button 改动后复跑 `build_examples_all.bat --xui` 通过。
- [x] 验证：2026-05-21 专用 smoke 增加 active tab focus 同步覆盖；`Document` / `Output` active 切换、关闭 `Output` 后 fallback、重新 dock `Output` 后恢复均验证 `tXui.pFocus` 指向对应 focusable content widget。
- [x] 验证：2026-05-21 focus 同步改动后复跑 `examples\xui_dockpanel_lab\build.bat`、八种 `xui_dockpanel_lab --frames 3` 模式与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 split tree pane min-size 回传与 ratio min/max clamp 覆盖；`pSplit->fRatio=0.01/0.99` 后 document/tool pane 均满足 axis min，截图 `build\xui_dockpanel_lab_min_size.png` 像素检查 `sample=78769 nonWhite=18035 blueChrome=10015 darkText=726 splitterTone=697`；`test\build_xui_dockpanel_smoke.bat`、`build\xge_xui_dockpanel_smoke.exe`、八种 lab `--frames 3` 与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 drag 期间 `Tab` 被消费且不改变 focus、`Escape` 取消 drag 并释放 pointer capture 覆盖；修改后复跑 `test\build_xui_dockpanel_smoke.bat`、`build\xge_xui_dockpanel_smoke.exe`、八种 lab `--frames 3` 与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 DockPanel tab/close tooltip 覆盖；示例新增 `--tooltip-preview`，截图 `build\xui_dockpanel_lab_tooltip.png` 可见 `Output` tooltip，像素检查 `sample=78769 nonWhite=17976 tooltipBack=60769 tooltipBorder=38 darkText=491`；九种 lab `--frames 3` 与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 pane option menu 覆盖；检查 Float/Dock/Close/Close Others/Close All 文本和启用状态、Escape 关闭、Enter 执行 Float、Close All 隐藏可关闭 tab；示例新增 `--option-menu-preview`，截图 `build\xui_dockpanel_lab_option_menu.png` 可见 pane option 菜单，像素检查 `sample=78769 nonWhite=18301 menuBlue=283 dangerText=0 menuBorder=46`；十种 lab `--frames 3` 与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 tab overflow menu 覆盖；临时压窄 pane 后确认 overflow button 出现、菜单列出 `Document`/`Output`、active tab checked、Enter 可切回 `Document`；示例新增 `--overflow-menu-preview`，截图 `build\xui_dockpanel_lab_overflow_menu.png` 可见 tab overflow 菜单和省略标题，像素检查 `sample=78769 nonWhite=18332 menuBlue=518 menuBorder=1275 darkText=744 overflowButton=983`；十一种 lab `--frames 3` 模式与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 Ctrl 抑制 docking 覆盖；drag move 带 `XGE_KEY_MOD_CTRL` 时无 pane/global hover、无 indicator、保留 floating preview，mouse up 后 `Output` 进入 floating；示例新增 `--ctrl-drag-preview`，截图 `build\xui_dockpanel_lab_ctrl_drag_preview.png` 可见 floating preview rect 且没有 dock indicator，像素检查 `sample=78769 nonWhite=18289 blueOverlay=335 indicatorBlue=820 darkText=503 whiteLike=60474`；十二种 lab `--frames 3` 模式与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 专用 smoke 增加 deep split collapse 覆盖；构造 document region 二级 split，隐藏内层 pane 验证 sibling 提升，隐藏 root sibling 验证 document root 回到单 pane；`test\build_xui_dockpanel_smoke.bat` 与 `build\xge_xui_dockpanel_smoke.exe` 均通过。
- [x] 验证：2026-05-21 `docs/api/xui.md`、`docs/api/xui.en.md` 增加 DockPanel / DockLayout API 文档，覆盖 dockwindow 组合 `xge_xui_window_t`、dock/float/hide 状态、client widget ownership、drag/capture/overlay 边界与当前 XSON/auto-hide 限制。
- [x] 验证：2026-05-21 专用 smoke 增加 tab 中键关闭与右键负向覆盖；中键 down/up 关闭 `Output` 并释放 capture，右键 tab 不启动 dock drag。复跑 `build\xge_xui_dockpanel_smoke.exe`、十二种 `xui_dockpanel_lab --frames 3` 模式与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 新增 DockPanel XSON 声明式入口与示例；`examples\xui_dockpanel_xson` 覆盖 docked/floating/hidden/autoHide、region portion、dockable=false 与 client children，`build\xui_dockpanel_xson.exe --frames 3` 输出 `create=1 state=1 layouts=1 windows=7 labels=7`，随后 `build_examples_all.bat --xui` 通过并包含 `XUI DockPanel XSON`。
- [x] 验证：2026-05-21 新增 `xgeXuiDockLayoutSaveState` save-state 序列化，专用 smoke 覆盖 regions/windows/floating 计数并释放 XValue；`test\build_xui_dockpanel_smoke.bat`、`build\xge_xui_dockpanel_smoke.exe` 与 `build_examples_all.bat --xui` 均通过。
- [x] 验证：2026-05-21 新增 `xgeXuiDockLayoutLoadState` roundtrip；专用 smoke 覆盖 NULL state 失败不修改布局、保存后扰动 dock/hidden 状态、LoadState 恢复 floating/docked 状态与 client reparent；`test\build_xui_dockpanel_smoke.bat` 与 `build\xge_xui_dockpanel_smoke.exe` 均通过。
- [x] 验证：2026-05-21 新增 auto-hide 基础闭环；专用 smoke 覆盖 side/split pane auto-hide button down/up、`XGE_XUI_DOCK_WINDOW_AUTO_HIDE` 状态、strip rect layout、strip item click expand、overlay close、overlay dock restore；`examples\xui_dockpanel_xson` 覆盖 `state:"autoHide"`。
- [x] 验证：2026-05-21 新增 auto-hide expand 视觉验证；`examples\xui_dockpanel_lab --auto-hide-preview --frames 3` 输出 `create=1 layout=1 state=1 labels=17`，自启动截图到 `build\xui_dockpanel_autohide_preview.png`，画面可见右侧 auto-hide strip、展开 overlay pane、dock/close buttons 和 Properties client 内容。
- [!] 阻塞：`build_test.bat` 当前被既有测试与公开 API 漂移阻断，尚未运行到 DockPanel 新 smoke。2026-05-21 复跑仍失败，前置错误包括 `xgeXuiCheckBoxSetColors` 参数数量不匹配、`xgeXuiSwitch*` API 缺失、TreeView/TableView/Menu 测试访问旧字段。处理：DockPanel 本轮使用 `test/xui_dockpanel_smoke.c` 独立验证；全量测试基线需单独修复。

## V1 验收标准

V1 完成必须同时满足：

- [x] `xge.h` 有稳定的 DockLayout/DockWindow 最小公开 API。说明：Phase A/B/C 最小 API 已编译进 DLL。
- [x] 静态 docklayout 可创建 document/left/right/bottom 等区域。说明：`xui_dockpanel_lab` 已显示 document、left、right、bottom。
- [x] dockwindow 可停靠、浮动、隐藏并保持同一 client widget。说明：专用 smoke 覆盖 client slot reparent。
- [x] pane tabs 可显示、切换、关闭 active tab。说明：显示、点击切换、close hide 与 close 后 active fallback 已由专用 smoke 和 `--close-preview` 验证。
- [x] pane side drop 可拆分出新的 dock region node。说明：API/static side split 与 tab drag side commit 已验证。
- [x] pane fill drop 可合并为 tab。说明：`Document.c` 与 `Output` 合并为同一 pane tab。
- [x] floating dockwindow 可拖回 docklayout。说明：floating dockwindow 标题栏进入 dock drag，`test/xui_dockpanel_smoke.c` 和 `xui_dockpanel_lab --float-dock-preview` 已验证。
- [x] 拖拽过程中显示 DockPanelSuite VS2005 风格 indicator 和 preview rect。说明：`--drag-preview`、`--float-dock-preview`、`--global-dock-preview` 截图与像素采样已验证。
- [x] Escape/capture lost/drop failure 均能回滚。说明：Escape、capture lost/cancel 路径已接入并专测；drop failure 通过 mid-drag `dockable=false` 负向 commit 验证 source 状态不变。
- [x] 示例 `xui_dockpanel_lab` 可手动操作并可 smoke 跑通。说明：普通 smoke、tab reorder preview、tab float preview、tab drag preview、floating dock preview、global dock preview、截图和像素检查均通过；用户后续统一手动验收。
- [x] 资源 license/source 说明保留。
- [x] `build_dll.bat` 和 `build_examples_all.bat --xui` 在 DLL 未被占用时通过。说明：2026-05-21 已验证 `build_dll.bat`、`build_examples_xui.bat` 与 `build_examples_all.bat --xui`。

V1 不要求：

- [!] OS/native 外部浮动窗口。原因：产品边界明确禁止。
- [!] DockPanelSuite 级 auto-hide hover 延迟、动画与最终视觉手感调优。原因：click expand overlay、focus/capture/close 与 dock restore 已完成，后续只保留手感调优，不阻塞基础功能闭环。
- [!] 完整 DockPanelSuite persistence 兼容格式。原因：XUI 使用自己的 split tree 序列化。
- [!] VS2012 风格资源。原因：当前观感不符合目标。
