# XUI Widget

`xge_xui_widget_t` 是 XUI 的基础节点、布局节点和视觉基础设施。所有控件都挂载在 Widget 上，Widget 负责树结构、盒模型、布局、裁剪、基础绘制、状态样式、焦点、事件路由、owner draw、tooltip 和 XSON 通用属性；具体控件只负责自己的内容语义。

这份文档也是后续控件重构的基础约束：如果能力已经属于 Widget，控件不要重复实现。

## 职责边界

Widget 负责：

- 父子树、兄弟顺序、层级、z 顺序和命名索引。
- 位置、尺寸、margin、border、padding、content rect。
- 常用布局：absolute、row、column、stack、grid、dock。
- 背景、圆角、边框、焦点环、禁用遮罩、调试边框。
- hover、active、focus、disabled、checked 等视觉状态样式。
- visible、enabled、focusable、tabStop、tabIndex、IME、hit test、inputTransparent。
- overflow/clip 和绘制裁剪。
- 自定义 measure、layout、paint、owner draw。
- tooltip、hotkey、默认/取消动作、事件兴趣和事件处理器。

控件负责：

- 控件自己的数据模型和语义，例如 Label 的文字、Image 的纹理区域、Button 的图标混排和选择逻辑、Progress 的进度值。
- 控件内容区域内的绘制。
- 控件自己的交互状态，并通过 `xgeXuiWidgetSetVisualState` 把视觉状态同步给 Widget。

## 树结构

```c
xge_xui_widget root = xgeXuiRoot(&xui);
xge_xui_widget child = xgeXuiWidgetCreate();

xgeXuiWidgetSetName(child, "submit");
xgeXuiWidgetAdd(root, child);

xge_xui_widget found = xgeXuiWidgetFindByName(root, "submit");
xgeXuiWidgetRemove(child);
xgeXuiWidgetFree(child);
```

常用 API：

```c
xge_xui_widget xgeXuiWidgetCreate(void);
void xgeXuiWidgetFree(xge_xui_widget widget);
int xgeXuiWidgetAdd(xge_xui_widget parent, xge_xui_widget child);
void xgeXuiWidgetRemove(xge_xui_widget widget);

void xgeXuiWidgetSetId(xge_xui_widget widget, int id);
int xgeXuiWidgetGetId(xge_xui_widget widget);
void xgeXuiWidgetSetName(xge_xui_widget widget, const char* name);
const char* xgeXuiWidgetGetName(xge_xui_widget widget);
xge_xui_widget xgeXuiWidgetFindById(xge_xui_widget root, int id);
xge_xui_widget xgeXuiWidgetFindByName(xge_xui_widget root, const char* name);
```

## 盒模型

Widget 使用四层矩形：

- `outerRect`：包含 margin 的外边界。
- `borderRect`：背景和边框区域。
- `paddingRect`：扣除 border 后的区域。
- `contentRect`：扣除 padding 后，控件内容绘制和子布局使用的区域。

```c
xgeXuiWidgetSetRect(widget, (xge_rect_t){ 10, 20, 160, 48 });
xgeXuiWidgetSetMarginPx(widget, 4, 4, 4, 4);
xgeXuiWidgetSetPaddingPx(widget, 10, 6, 10, 6);
xgeXuiWidgetSetBorder(widget, 1.0f, XGE_COLOR_RGBA(120, 140, 166, 255));
```

尺寸单位：

```c
xgeXuiSizePx(120.0f);
xgeXuiSizeDip(120.0f);
xgeXuiSizePercent(100.0f);
xgeXuiSizeGrow(1.0f);
xgeXuiSizeContent();
```

`px` 是像素值，`dip` 会受 DIP 缩放影响，`percent` 相对父容器，`grow` 在 row/column 等布局中参与剩余空间分配，`content` 由控件或子节点测量决定。

## 布局

布局类型：

| 常量 | 说明 |
| --- | --- |
| `XGE_XUI_LAYOUT_ABSOLUTE` | 使用局部 rect/anchor 定位。 |
| `XGE_XUI_LAYOUT_ROW` | 子元素水平排列。 |
| `XGE_XUI_LAYOUT_COLUMN` | 子元素垂直排列。 |
| `XGE_XUI_LAYOUT_STACK` | 子元素叠放。 |
| `XGE_XUI_LAYOUT_GRID` | 固定列数网格。 |
| `XGE_XUI_LAYOUT_DOCK` | 按 dock 边停靠布局。 |

常用 API：

```c
xgeXuiWidgetSetLayout(parent, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetGap(parent, 8.0f);
xgeXuiWidgetSetAlign(parent, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_START);
xgeXuiWidgetSetJustify(parent, XGE_XUI_JUSTIFY_START);

xgeXuiWidgetSetGrid(parent, 3, 64.0f, 8.0f, 8.0f);
xgeXuiWidgetSetGridColumnSpan(child, 2);
xgeXuiWidgetSetDock(child, XGE_XUI_DOCK_TOP);
```

对齐：

- `XGE_XUI_ALIGN_START`
- `XGE_XUI_ALIGN_CENTER`
- `XGE_XUI_ALIGN_END`
- `XGE_XUI_ALIGN_STRETCH`

分布：

- `XGE_XUI_JUSTIFY_START`
- `XGE_XUI_JUSTIFY_CENTER`
- `XGE_XUI_JUSTIFY_END`
- `XGE_XUI_JUSTIFY_SPACE_BETWEEN`

## 基础绘制

Widget 在控件内容之前绘制背景和边框，在内容和子节点之后绘制禁用遮罩、焦点环和调试边框。

```c
xgeXuiWidgetSetBackground(widget, XGE_COLOR_RGBA(248, 250, 253, 255));
xgeXuiWidgetSetRadius(widget, 4.0f);
xgeXuiWidgetSetBorder(widget, 1.0f, XGE_COLOR_RGBA(150, 168, 190, 255));
xgeXuiWidgetSetFocusRing(widget, 2.0f, XGE_COLOR_RGBA(58, 132, 220, 180));
xgeXuiWidgetSetDisabledOverlay(widget, XGE_COLOR_RGBA(240, 244, 248, 120));
xgeXuiWidgetSetDebugOutline(widget, 1.0f, XGE_COLOR_RGBA(255, 0, 0, 180));
```

控件重构原则：

- Label、Image、Separator、Progress、Button 等控件不要自己重复绘制背景和边框。
- 控件绘制默认应限制在 `contentRect`。
- 如果控件需要全控件素材背景，例如图片按钮，使用控件自己的图片/九宫格机制；但普通背景和边框仍优先走 Widget。

## 状态样式

Widget 支持按视觉状态覆盖以下样式：

- background
- border color
- border width
- focus ring color
- focus ring width
- disabled overlay

状态样式结构：

```c
typedef struct xge_xui_state_style_t {
    uint32_t iMask;
    uint32_t iBackgroundColor;
    uint32_t iBorderColor;
    float fBorderWidth;
    uint32_t iFocusRingColor;
    float fFocusRingWidth;
    uint32_t iDisabledOverlayColor;
} xge_xui_state_style_t;
```

便捷 API：

```c
xgeXuiWidgetSetStateBackground(widget, XGE_XUI_STATE_HOVER, hoverColor);
xgeXuiWidgetSetStateBorder(widget, XGE_XUI_STATE_ACTIVE, 2.0f, activeBorder);
xgeXuiWidgetSetStateFocusRing(widget, XGE_XUI_STATE_FOCUS, 2.0f, focusColor);
xgeXuiWidgetSetStateDisabledOverlay(widget, XGE_XUI_STATE_DISABLED, overlayColor);

xgeXuiWidgetSetVisualState(widget, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_FOCUS);
```

完整 API：

```c
void xgeXuiWidgetSetStateStyle(xge_xui_widget widget, int state, const xge_xui_state_style_t* style);
const xge_xui_state_style_t* xgeXuiWidgetGetStateStyle(xge_xui_widget widget, int state);
void xgeXuiWidgetClearStateStyle(xge_xui_widget widget, int state);
void xgeXuiWidgetSetVisualState(xge_xui_widget widget, int state);
int xgeXuiWidgetGetVisualState(xge_xui_widget widget);
```

状态合成顺序：

1. base style
2. normal state style
3. checked
4. hover
5. active
6. focus
7. disabled

后面的状态覆盖前面的状态。比如按钮同时处于 selected 和 hover 时，hover 的背景色会覆盖 selected 背景色；如果 hover 没设置背景，则保留 selected 背景。

## 裁剪与 Overflow

```c
xgeXuiWidgetSetOverflow(widget, XGE_XUI_OVERFLOW_CLIP);
xgeXuiWidgetSetClip(widget, 1);
```

模式：

- `XGE_XUI_OVERFLOW_VISIBLE`：不裁剪。
- `XGE_XUI_OVERFLOW_CLIP`：裁剪到 widget。
- `XGE_XUI_OVERFLOW_HIDDEN`：隐藏溢出，语义上等价裁剪。
- `XGE_XUI_OVERFLOW_SCROLL`：为滚动视图保留的裁剪语义。

`XGE_XUI_WIDGET_ROLE_CONTROL` 默认会启用 clip，避免控件内容溢出破坏布局。

## 层级与命中

```c
xgeXuiWidgetSetZ(widget, 10);
xgeXuiWidgetSetLayer(widget, XGE_XUI_LAYER_POPUP);
xgeXuiWidgetSetHitTestVisible(widget, 1);
xgeXuiWidgetSetInputTransparent(widget, 0);
```

层级：

- `XGE_XUI_LAYER_NORMAL`
- `XGE_XUI_LAYER_FLOATING`
- `XGE_XUI_LAYER_POPUP`
- `XGE_XUI_LAYER_MODAL`
- `XGE_XUI_LAYER_TOOLTIP`
- `XGE_XUI_LAYER_DRAG_ADORNER`
- `XGE_XUI_LAYER_DEBUG`

`z` 只在同层内排序，`layer` 用于 overlay、popup、modal、tooltip 等跨普通树的绘制和命中顺序。

## 焦点与键盘

```c
xgeXuiWidgetSetFocusable(widget, 1);
xgeXuiWidgetSetTabStop(widget, 1);
xgeXuiWidgetSetTabIndex(widget, 10);
xgeXuiWidgetSetFocusScope(widget, 1);
xgeXuiWidgetSetDefaultAction(widget, OnDefault, user);
xgeXuiWidgetSetCancelAction(widget, OnCancel, user);
```

约定：

- 能被键盘直接操作的控件应设置 focusable。
- 需要参与 Tab 顺序的控件设置 tabStop。
- 控件的 focus 视觉应尽量使用 Widget 的 focus ring，而不是控件自己额外画一套。
- `defaultAction` 和 `cancelAction` 用于 Enter/Escape 等通用动作场景。

IME：

```c
xgeXuiWidgetSetImeMode(widget, XGE_XUI_IME_ENABLED);
xgeXuiWidgetSetImeCandidateRect(widget, ResolveCandidateRect, user);
```

只有输入类控件应启用 IME。

## 事件

Widget 提供通用事件入口、捕获事件、按类型事件处理器和事件兴趣掩码。

```c
xgeXuiWidgetSetEvent(widget, OnEvent, user);
xgeXuiWidgetSetCaptureEvent(widget, OnCaptureEvent);
xgeXuiWidgetSetEventHandler(widget, XGE_EVENT_MOUSE_DOWN, OnMouseDown, user);
xgeXuiWidgetSetEventInterest(widget, XGE_XUI_EVENT_MASK_MOUSE_MOVE, 1);
```

控件实现建议：

- 控件可以使用 `procEvent` 实现核心交互，也可以用 typed event handler 做扩展。
- hover、active、checked、disabled 等控件状态变化后，应调用 `xgeXuiWidgetSetVisualState`。
- 输入透明的装饰节点应设置 `inputTransparent`，不要拦截交互。

## Measure、Layout 与 Paint Hook

```c
xgeXuiWidgetSetMeasure(widget, MeasureProc);
xgeXuiWidgetSetMeasureUser(widget, MeasureProc, user);
xgeXuiWidgetSetLayoutProc(widget, LayoutProc, user);

xgeXuiWidgetSetPaintBefore(widget, PaintBefore, user);
xgeXuiWidgetSetPaint(widget, PaintContent, user);
xgeXuiWidgetSetPaintAfter(widget, PaintAfter, user);
```

绘制顺序：

1. `paintBefore`
2. Widget 背景和边框
3. 控件内容 `paint`
4. 子节点
5. `paintAfter`
6. disabled overlay
7. focus ring
8. debug outline

控件默认应只设置 `paint`，特殊控件才使用 before/after。

## Owner Draw

Owner draw 用于用户完全或部分接管绘制。

```c
xgeXuiWidgetSetOwnerDraw(widget, XGE_XUI_OWNER_DRAW_CONTENT, DrawProc, user);
xgeXuiWidgetSetOwnerDrawControl(widget, controlPointer);
```

模式：

| 模式 | 说明 |
| --- | --- |
| `XGE_XUI_OWNER_DRAW_NONE` | 不启用 owner draw。 |
| `XGE_XUI_OWNER_DRAW_CONTENT` | 接管内容绘制，仍保留 Widget 背景、子节点、遮罩、焦点环。 |
| `XGE_XUI_OWNER_DRAW_CONTENT_AND_CHILDREN` | 接管内容和子节点绘制。 |
| `XGE_XUI_OWNER_DRAW_FULL` | 接管整个 widget 绘制，Widget 不再自动画背景、边框和子节点。 |

控件设计建议：

- 普通自定义外观优先通过 Widget 样式或控件 API 提供，不要让用户必须 owner draw。
- owner draw 是高级出口，用于无法用标准控件能力表达的特殊需求。

## Tooltip

```c
xgeXuiWidgetSetTooltipText(widget, "保存");

xge_xui_tooltip_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iType = XGE_XUI_TOOLTIP_TEXT;
desc.sText = "保存当前内容";
desc.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
desc.fOffsetY = 6.0f;
xgeXuiWidgetSetTooltip(widget, &desc);
```

XSON 可使用字符串或对象：

```json
{
  "tooltip": "保存当前内容"
}
```

```json
{
  "tooltip": {
    "text": "保存当前内容",
    "anchor": "bottom",
    "offsetY": 6,
    "delay": 0.35,
    "followCursor": false
  }
}
```

## Dirty 标记

```c
xgeXuiWidgetMarkLayout(widget);
xgeXuiWidgetMarkPaint(widget);
xgeXuiWidgetMarkStyle(widget);
```

规则：

- 尺寸、margin、padding、layout、children 变化标记 layout。
- 背景、边框、文字、纹理、状态变化标记 paint。
- 会影响布局和绘制的综合样式变化标记 style。

控件 setter 必须主动标记脏状态，不能依赖下一帧偶然刷新。

## XSON 通用字段

所有 XSON 控件节点都支持 Widget 通用字段：

```json
{
  "type": "button",
  "id": "ok",
  "name": "ok",
  "width": "100%",
  "height": 42,
  "minWidth": 80,
  "maxWidth": 240,
  "margin": [4, 4, 4, 4],
  "padding": [10, 6, 10, 6],
  "layout": "row",
  "gap": 8,
  "align": "stretch",
  "justify": "center",
  "background": "#F8FAFDFF",
  "radius": 4,
  "borderColor": "#96A8BEFF",
  "borderWidth": 1,
  "focusRingColor": "#3A84DCB0",
  "focusRingWidth": 2,
  "disabledOverlay": "#F0F4F878",
  "overflow": "clip",
  "visible": true,
  "enabled": true,
  "tabStop": true,
  "tabIndex": 10,
  "tooltip": "确认操作"
}
```

布局字段：

- `layout`: `absolute` / `row` / `column` / `stack` / `grid` / `dock`
- `dock`: `left` / `top` / `right` / `bottom` / `fill`
- `grid`: `{ "columns": 3, "rowHeight": 64, "columnGap": 8, "rowGap": 8, "columnSpan": 2 }`
- `columns` / `gridColumns` / `rowHeight` / `columnGap` / `rowGap` 也可直接写在节点上。

裁剪和层级字段：

- `overflow`: `visible` / `clip` / `hidden` / `scroll`
- `clip`: bool
- `layer`: `normal` / `floating` / `popup` / `modal` / `tooltip` / `dragAdorner` / `debug`
- `z` / `zIndex`: 同层排序值。

交互字段：

- `visible`
- `enabled`
- `hitTestVisible`
- `inputTransparent`
- `tabStop`
- `tabIndex`
- `imeMode`

## 控件重构检查清单

每个控件重构时都要确认：

- 背景、边框、圆角、padding、裁剪、focus ring、disabled overlay 是否复用 Widget。
- hover、active、focus、disabled、checked 视觉状态是否通过 Widget 状态样式表达。
- 控件自己的绘制是否只覆盖控件语义内容。
- setter 是否语义明确，并正确标记 layout/paint/style。
- XSON 字段是否与 C API 一一对应。
- 专属测试是否覆盖正常、禁用、交互态、自定义绘制/样式、XSON 加载。

对于简单静态控件，优先保持模型简单；对于复杂视觉需求，优先抽象为可复用基础设施，例如九宫格对象，而不是把一次性逻辑写进单个控件。
