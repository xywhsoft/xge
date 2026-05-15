# XUI Popup

## 设计定位

Popup 是 XUI 的通用弹层基础设施。它不是普通业务控件，而是让某个 widget 以 overlay 身份出现的控制器。

Widget 负责节点、盒模型、布局、裁剪、基础绘制和事件路由。Popup 负责弹层特有行为：

- 挂载到 `overlayRoot`。
- 绑定 owner 或自定义 anchor。
- 打开、关闭、置顶和可见性。
- 根据 anchor 和弹出方向计算位置。
- 管理 outside click、Escape、owner 区域点击、modal 消费策略。
- 打开时处理焦点，关闭时恢复焦点。
- owner hidden 或 disabled 时自动关闭。
- 当窗口显示不下完整内容时，提供 viewport + scroll 的通用承载策略。

Popup 不负责业务内容。ComboBox 的列表、Menu 的菜单项、ColorPicker 的颜色面板、DatePicker 的日历格子都由具体控件绘制和处理。

## 核心原则

Popup 的大小策略遵循一个硬规则：

**Popup 不缩放内容。**

控件申请多大，代表业务布局按这个尺寸设计。Popup 不替控件决定内容应该变小。窗口放不下时，缩小的是可见 viewport，不是内容尺寸；内容通过滚动查看和操作。

这条规则用于降低控件开发复杂度，避免控件作者在不同窗口大小下同时处理缩放布局、重排和交互命中。

## 尺寸模型

Popup 使用两个尺寸概念：

- `contentSize`：弹层内容申请大小，永远不被 Popup 缩放。
- `viewportSize`：实际可见容器大小，最大不超过窗口大小。

规则：

```text
if windowSize >= contentSize:
    viewportSize = contentSize
    不启用滚动容器
else:
    viewportSize = min(contentSize, windowSize)
    启用内部滚动容器
    contentSize 保持不变
```

实现上，overlay root 下的 popup widget 表示 viewport。内容 widget 保持 `contentSize`，当内容超出 viewport 时由 Popup 内部滚动能力显示滚动条并处理滚动偏移。

```text
overlayRoot
  popupWidget          // viewport，负责背景、边框、裁剪、outside 命中
    contentWidget      // contentSize，不被缩放
```

如果内容不超出 viewport，可以直接让内容 widget 填满 popup widget，滚动能力不显示。

## 锚点和方向

Popup 的位置由两个参数决定：

- `anchorPoint`：以 owner 或 anchor rect 的哪个点作为锚点。
- `popupDirection`：popup 从锚点向哪个方向展开。

默认行为：

```text
anchorPoint = bottomLeft
popupDirection = rightDown
```

也就是常见下拉框：以 owner 左下角为锚点，向右下展开。

常用组合：

| anchorPoint | popupDirection | 用途 |
| --- | --- | --- |
| `bottomLeft` | `rightDown` | ComboBox、普通下拉菜单 |
| `bottomRight` | `leftDown` | 右对齐下拉 |
| `topLeft` | `rightUp` | 向上弹出 |
| `topRight` | `leftUp` | 右对齐向上弹出 |
| `cursor` | `rightDown` | 右键菜单、上下文菜单 |
| `fixed` | `rightDown` 或指定方向 | 固定坐标浮层 |

旧的 `BOTTOM_LEFT`、`TOP_RIGHT` 等 placement 常量可以映射到这套模型。后续新 API 应优先表达 `anchorPoint + popupDirection`，旧 API 保留兼容。

## 回退策略

Popup 先用 `anchorPoint + popupDirection` 计算理想位置。如果理想位置无法完整放进窗口，则按方向回退。

垂直空间不足时，优先垂直对称回退：

| 原方向 | 垂直回退 |
| --- | --- |
| `rightDown` | `rightUp` |
| `rightUp` | `rightDown` |
| `leftDown` | `leftUp` |
| `leftUp` | `leftDown` |

水平空间不足时，优先水平对称回退：

| 原方向 | 水平回退 |
| --- | --- |
| `rightDown` | `leftDown` |
| `rightUp` | `leftUp` |
| `leftDown` | `rightDown` |
| `leftUp` | `rightUp` |

水平和垂直都不足时，尝试对角回退：

| 原方向 | 对角回退 |
| --- | --- |
| `rightDown` | `leftUp` |
| `rightUp` | `leftDown` |
| `leftDown` | `rightUp` |
| `leftUp` | `rightDown` |

如果所有回退方向都无法完整显示 viewport，则进入最终回退。

## 最终回退

最终回退不再改变内容尺寸，也不再强制保持 owner 和 popup 的理想相对关系。

规则：

- 如果下方空间不足，上方空间也不足，按原始方向计算，再把 viewport 坐标向窗口内浮动。
- 如果右侧空间不足，左侧空间也不足，按方向约定向反方向浮动。
- 最差情况下，viewport 填满整个窗口。
- 允许遮盖 owner，因为此时窗口空间已经无法满足理想弹出关系。

最终 clamp 的对象是 `viewportRect`，不是 `contentRect`。

## 定位算法

完整流程：

```text
1. 获取窗口 rect。
2. 获取 owner borderRect、显式 anchorRect、cursor 坐标或 fixed 坐标。
3. 获取 contentSize。
4. 计算 viewportSize：
   - contentSize 能放进窗口：viewportSize = contentSize。
   - contentSize 放不进窗口：viewportSize = min(contentSize, windowSize)，启用滚动容器。
5. 按 anchorPoint + popupDirection 计算 preferredRect。
6. 如果 preferredRect 完整在窗口内，使用它。
7. 否则按垂直、水平、对角回退方向依次尝试。
8. 仍不行，按原方向计算后把 viewportRect clamp 到窗口内。
9. 设置 popup widget 为 viewportRect。
10. 设置 content widget 为 contentSize，并按滚动偏移绘制和命中。
```

## 关闭策略

Popup 需要显式区分三个区域：

- `inside`：点击 popup viewport 内部。
- `owner`：点击 owner 区域。
- `outside`：既不在 popup，也不在 owner。

建议策略：

| 策略 | 说明 |
| --- | --- |
| `outsideClose` | outside click 关闭 Popup，并消费事件。 |
| `outsideIgnore` | outside click 不关闭，事件继续传播。 |
| `outsideConsume` | outside click 不关闭，但消费事件，常用于 modal。 |
| `ownerPassthrough` | owner click 不由 Popup 处理，交给 owner 自己决定。 |
| `ownerClose` | owner click 关闭 Popup。 |
| `ownerToggle` | owner click 关闭 Popup，并继续把本次事件交给 owner；适合 owner 自己实现再次点击 toggle。 |
| `ownerConsume` | owner click 不关闭，但消费事件。 |
| `escapeClose` | Escape 关闭 Popup 并消费事件。 |
| `escapeIgnore` | Escape 不处理，继续传播。 |

Popup 内部区域是否拦截由 `consumeInside` 控制：

- `false`：默认。子控件未消费时，事件可继续传播，适合 tooltip、非交互说明层。
- `true`：内部区域消费 pointer down/up，适合 menu、dropdown、picker 等交互弹层，避免点穿到背后控件。

兼容 API `xgeXuiPopupSetAutoClose(outside, escape)` 映射为：

- `outside=true`：`outsideClose`
- `outside=false`：`outsideIgnore`
- `escape=true`：`escapeClose`
- `escape=false`：`escapeIgnore`

## 焦点策略

Popup 打开时不一定总要把焦点给 popup widget。不同控件需求不同：

| 焦点策略 | 说明 |
| --- | --- |
| `NONE` | 不改变焦点，适合 tooltip 或非交互提示。 |
| `POPUP` | 焦点给 popup widget，兼容当前行为。 |
| `FIRST_CHILD` | 焦点给第一个可聚焦子项，适合菜单、列表。 |
| `CUSTOM` | 调用方指定 focus widget，适合 ComboBox 内部 ListView、ColorPicker 内部编辑区。 |

关闭时恢复焦点：

- 如果显式设置 `focusRestore`，关闭时优先恢复到它。
- 如果未显式设置，打开时记录当前焦点，关闭时恢复。
- 如果 restore widget hidden 或 disabled，不恢复。

## 视觉口径

Popup 默认视觉应贴近 XUI Tech Blue：

- 背景：浅色 panel。
- 边框：清晰蓝灰边框。
- 圆角：默认不超过 4px，工具型控件保持克制。
- 不使用模糊阴影或大面积装饰。
- 内容裁剪由 viewport 控制。

具体控件可覆盖 popup surface 样式，但不应绕开 Popup 的位置、关闭和焦点策略。

## 兼容策略

下面这些 API 属于历史入口或便捷入口，仍保持可用，但新代码应优先使用 `anchorPoint + direction + closePolicy + focusPolicy` 表达完整语义：

- `xgeXuiPopupSetOpen`
- `xgeXuiPopupSetOwner`
- `xgeXuiPopupSetAutoClose`
- `xgeXuiPopupSetPlacement`
- `xgeXuiPopupSetAnchorRect`
- `xgeXuiPopupSetOffset`
- `xgeXuiPopupSetFocusRestore`
- `XGE_XUI_OVERLAY_PLACEMENT_MANUAL`

`MANUAL` 模式不参与自动尺寸、自动方向和自动回退。调用方完全负责 rect。

ComboBox、ColorPicker 等复合控件应复用 Popup 的尺寸、锚点、方向回退、关闭和焦点恢复机制；控件自身只负责业务内容和内部交互。

## 当前 API

```c
void xgeXuiPopupSetContentWidget(xge_xui_popup popup, xge_xui_widget content);
void xgeXuiPopupSetContentSize(xge_xui_popup popup, float width, float height);
void xgeXuiPopupSetAnchorPoint(xge_xui_popup popup, int anchorPoint);
void xgeXuiPopupSetDirection(xge_xui_popup popup, int direction);
void xgeXuiPopupSetGap(xge_xui_popup popup, float gap);
void xgeXuiPopupSetMatchOwnerWidth(xge_xui_popup popup, int enabled);
void xgeXuiPopupSetConsumeInside(xge_xui_popup popup, int enabled);
void xgeXuiPopupSetClosePolicy(xge_xui_popup popup, int outsidePolicy, int ownerPolicy, int escapePolicy);
void xgeXuiPopupSetFocusPolicy(xge_xui_popup popup, int focusPolicy, xge_xui_widget customFocus);
xge_rect_t xgeXuiPopupGetViewportRect(xge_xui_popup popup);
xge_rect_t xgeXuiPopupGetContentRect(xge_xui_popup popup);
void xgeXuiPopupSetScroll(xge_xui_popup popup, float x, float y);
void xgeXuiPopupGetScroll(xge_xui_popup popup, float* x, float* y);
```

常量：

```c
XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT
XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT
XGE_XUI_POPUP_ANCHOR_TOP_LEFT
XGE_XUI_POPUP_ANCHOR_TOP_RIGHT
XGE_XUI_POPUP_ANCHOR_CURSOR
XGE_XUI_POPUP_ANCHOR_FIXED

XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN
XGE_XUI_POPUP_DIRECTION_RIGHT_UP
XGE_XUI_POPUP_DIRECTION_LEFT_DOWN
XGE_XUI_POPUP_DIRECTION_LEFT_UP

XGE_XUI_POPUP_OUTSIDE_CLOSE
XGE_XUI_POPUP_OUTSIDE_IGNORE
XGE_XUI_POPUP_OUTSIDE_CONSUME

XGE_XUI_POPUP_OWNER_PASSTHROUGH
XGE_XUI_POPUP_OWNER_CLOSE
XGE_XUI_POPUP_OWNER_TOGGLE
XGE_XUI_POPUP_OWNER_CONSUME

XGE_XUI_POPUP_ESCAPE_CLOSE
XGE_XUI_POPUP_ESCAPE_IGNORE

XGE_XUI_POPUP_FOCUS_NONE
XGE_XUI_POPUP_FOCUS_POPUP
XGE_XUI_POPUP_FOCUS_FIRST_CHILD
XGE_XUI_POPUP_FOCUS_CUSTOM
```

## 范例要求

当前新增：

- `examples/xui_popup`
- `examples/xui_popup_xson`

覆盖：

- 默认 owner bottom-left + right-down。
- `xui_popup` 覆盖四个角点 anchor 与四个 direction 的 16 种组合，包括侧边弹出组合。
- 垂直、水平、对角回退。
- 窗口小于 contentSize 时 viewport + scroll。

后续逐控件迁移时继续补：

- 固定坐标。
- cursor anchor。
- outside/owner/Escape 策略的专门范例。
- modal consume。
- focus restore 和 custom focus。

旧 `xui_popup_owner_lab`、`xui_popup_policy_lab`、`xui_overlay_policy_lab` 保留为历史回归，文档应指向本文件作为新设计口径。
