# XUI Window

Window 是 XUI 的浮动风格化容器，用于工具窗口、子窗口、检查器、浮动面板等需要覆盖在普通控件之上的界面。它和 Panel 一样提供标题栏和客户区，但 Window 还承担覆盖层挂载、活动窗口、Z 序、TopMost、移动、调整大小、折叠、最大化、关闭按钮等窗口管理职责。

## 设计边界

- Widget 负责基础盒模型、布局、边框、背景、状态和事件路由。
- Window 是风格化浮动容器，默认挂到 XUI overlay 的 floating 层，而不是普通布局树。
- Window 的业务子控件自动添加到 client widget，用户可以通过 `xgeXuiWindowGetClientWidget` 继续配置客户区布局。
- 普通窗口和 TopMost 窗口分为两个 Z 组；同组内点击窗口会被提升到最前。
- 点击窗口标题栏、边框、客户区子控件都会激活窗口；子控件自身事件不应该被窗口拖拽逻辑抢占。
- Window 使用 XUI chrome style 的默认配色，使 Window、MenuBar、Toolbar、StatusBar、Menu/Popup 的视觉保持一致；单个窗口仍可覆盖颜色。

## 内部结构

```text
window widget                 floating overlay item
  client widget               user children are attached here
  collapse button widget      internal
  maximize button widget      internal
  close button widget         internal
```

标题栏和边框由 Window 自绘，客户区是普通 widget。标题栏按钮是内部 Button 控件，跟随窗口的按钮颜色、hover、active 状态绘制。

## Z 序和活动窗口

Window 初始化时会挂载到 `XGE_XUI_LAYER_FLOATING`。默认窗口使用普通 Z 组，TopMost 使用更高 Z 组。调用 `xgeXuiWindowBringToFront` 或点击窗口时，会更新 overlay tree order，从而在同一 Z 组内提升窗口。

`xgeXuiWindowSetTopMost` 只改变窗口所在 Z 组，不表示独占 modal。TopMost 窗口会压在普通窗口上方，但仍遵循同组内部的前后顺序。

XUI context 保存当前活动窗口：

```c
xge_xui_window xgeXuiWindowGetActive(xge_xui_context pContext);
int xgeXuiWindowIsActive(xge_xui_window pWindow);
```

活动窗口使用更强调的标题栏和边框；非活动窗口使用更轻的标题栏和文字颜色。

## C API

```c
int xgeXuiWindowInit(xge_xui_window pWindow, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiWindowUnit(xge_xui_window pWindow);

xge_xui_widget xgeXuiWindowGetClientWidget(xge_xui_window pWindow);

void xgeXuiWindowSetTitle(xge_xui_window pWindow, xge_font pFont, const char* sTitle);
void xgeXuiWindowSetIcon(xge_xui_window pWindow, xge_texture pTexture, xge_rect_t tSrc);
void xgeXuiWindowSetClose(xge_xui_window pWindow, xge_xui_click_proc procClose, void* pUser);

void xgeXuiWindowSetOpen(xge_xui_window pWindow, int bOpen);
int xgeXuiWindowIsOpen(xge_xui_window pWindow);

void xgeXuiWindowSetShowTitleBar(xge_xui_window pWindow, int bShow);
void xgeXuiWindowSetMovable(xge_xui_window pWindow, int bEnabled);
void xgeXuiWindowSetDragAnywhere(xge_xui_window pWindow, int bEnabled);
void xgeXuiWindowSetResizable(xge_xui_window pWindow, int bEnabled);
void xgeXuiWindowSetResizeEdges(xge_xui_window pWindow, uint32_t iEdges);

void xgeXuiWindowSetShowCollapse(xge_xui_window pWindow, int bShow);
void xgeXuiWindowSetShowMaximize(xge_xui_window pWindow, int bShow);
void xgeXuiWindowSetShowClose(xge_xui_window pWindow, int bShow);

void xgeXuiWindowSetCollapsed(xge_xui_window pWindow, int bCollapsed);
int xgeXuiWindowIsCollapsed(xge_xui_window pWindow);
void xgeXuiWindowSetMaximized(xge_xui_window pWindow, int bMaximized);
int xgeXuiWindowIsMaximized(xge_xui_window pWindow);

void xgeXuiWindowBringToFront(xge_xui_window pWindow);
void xgeXuiWindowSetTopMost(xge_xui_window pWindow, int bTopMost);
int xgeXuiWindowIsTopMost(xge_xui_window pWindow);
int xgeXuiWindowIsActive(xge_xui_window pWindow);
xge_xui_window xgeXuiWindowGetActive(xge_xui_context pContext);

void xgeXuiWindowSetChrome(
    xge_xui_window pWindow,
    float fTitleBarHeight,
    float fBorderWidth,
    float fResizeGrip,
    float fButtonSize);

void xgeXuiWindowSetColors(
    xge_xui_window pWindow,
    uint32_t iBackground,
    uint32_t iTitleBar,
    uint32_t iTitleText,
    uint32_t iBorder,
    uint32_t iButtonNormal,
    uint32_t iButtonHover,
    uint32_t iButtonActive);
```

## XSON

```json
{
  "type": "window",
  "id": "inspector",
  "font": "@fonts.body",
  "anchor": { "left": 320, "top": 120 },
  "width": 360,
  "height": 220,
  "title": "Inspector",
  "icon": "@textures.icon",
  "iconSrc": [0, 0, 16, 16],
  "topMost": true,
  "clientLayout": "column",
  "clientPadding": [12, 10, 12, 10],
  "clientGap": 8,
  "children": [
    { "type": "label", "text": "Children are attached to the client." },
    { "type": "button", "text": "Apply" }
  ]
}
```

常用字段：

- `title` / `text`: 标题栏文字。
- `font`: 标题栏字体。
- `icon` / `iconTexture`, `iconSrc`: 标题栏图标。
- `topMost`: 是否进入 TopMost Z 组。
- `open`, `collapsed`, `maximized`: 初始状态。
- `showTitleBar`, `movable`, `dragAnywhere`, `resizable`: 交互能力。
- `showCollapse`, `showMaximize`, `showClose`: 标题栏按钮可见性。
- `titleBarHeight`, `borderWidth`, `resizeGrip`, `buttonSize`: chrome 尺寸。
- `background` / `backgroundColor`, `clientColor` / `clientBackgroundColor`, `titleBarColor`, `titleTextColor`, `borderColor`, `buttonColor`, `buttonHoverColor`, `buttonActiveColor`: 配色。
- `clientLayout`, `clientPadding`, `clientGap`: 客户区布局。

XSON 中 Window 节点会自动挂载到 overlay floating 层；`children` 会自动添加到 Window 的 client widget。

## 使用建议

- 普通内容分组优先使用 Panel；需要浮动覆盖、拖拽、调整大小或 Z 序管理时使用 Window。
- TopMost 适合工具箱、检查器、调试面板，不应该替代 modal dialog。
- 需要无标题栏浮动工具时，设置 `showTitleBar=false` 和 `dragAnywhere=true`，同时隐藏不需要的标题栏按钮。
- 客户区仍然使用普通 XUI 布局，不要手写窗口内部坐标，除非该客户区本身就是画布类内容。

## 范例

- `examples/xui_window`
- `examples/xui_window_xson`
- `examples/xui_window_lab` 保留为旧策略/回归实验范例。

范例覆盖普通窗口、TopMost、无标题栏拖拽窗口、折叠窗口、标题图标、客户区布局、子按钮事件、XSON 加载、floating layer 和活动窗口状态检查。
