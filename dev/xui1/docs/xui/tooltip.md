# XUI Tooltip

Tooltip 是 widget 的内置提示机制，不再作为独立控件存在。任何 widget 都可以挂载 tooltip；它只负责在鼠标悬停时显示说明、状态提示或自定义浮层内容。

## 设计思路

- Tooltip 属于 widget 基础设施，避免每个控件重复实现提示逻辑。
- 默认形态是轻量文本提示，适合按钮、输入框、菜单项等控件的短说明。
- 复杂提示使用 custom tooltip，通过 measure/paint 回调绘制，例如游戏装备 tooltip。
- XSON 只支持声明式文本 tooltip；自定义绘制需要 C 代码回调。
- Tooltip 不参与布局树尺寸计算，它由 widget tooltip 管理器放入 tooltip layer。

## API

```c
void xgeXuiWidgetSetTooltipText(xge_xui_widget pWidget, const char* sText);
void xgeXuiWidgetSetTooltip(xge_xui_widget pWidget, const xge_xui_tooltip_desc_t* pDesc);
void xgeXuiWidgetSetTooltipResolver(xge_xui_widget pWidget, xge_xui_tooltip_resolve_proc procResolve, void* pUser);
void xgeXuiWidgetClearTooltip(xge_xui_widget pWidget);
const xge_xui_tooltip_desc_t* xgeXuiWidgetGetTooltip(xge_xui_widget pWidget);

int xgeXuiWidgetTooltipIsOpen(xge_xui_context pContext);
xge_xui_widget xgeXuiWidgetTooltipGetOwner(xge_xui_context pContext);
xge_rect_t xgeXuiWidgetTooltipGetRect(xge_xui_context pContext);
```

`xge_xui_tooltip_desc_t` 关键字段：

- `iType`: `XGE_XUI_TOOLTIP_TEXT` 或 `XGE_XUI_TOOLTIP_CUSTOM`。
- `sText`: 文本提示内容。
- `iAnchor`: `bottom/top/right/left/cursor` 对应的锚点。
- `fOffsetX/fOffsetY`: tooltip 相对锚点的偏移。
- `fDelay`: 悬停多久后显示。
- `bFollowCursor`: 光标锚点时是否跟随鼠标移动。
- `procMeasure/procPaint/pUser`: 自定义 tooltip 的尺寸计算和绘制回调。

`xge_xui_tooltip_resolve_proc` 使用布尔返回值：返回非 0 表示 `pDesc` 中已经填好本次要显示的 tooltip；返回 0 表示当前没有 tooltip。它会在鼠标移动到 widget 上，以及 tooltip 更新时被调用，适合根据鼠标位置或控件内部状态动态切换提示内容。

## XSON

字符串简写：

```json
{
  "type": "button",
  "text": "Apply",
  "tooltip": "Apply changes"
}
```

对象写法：

```json
{
  "type": "label",
  "text": "Item",
  "tooltip": {
    "text": "Open item details",
    "anchor": "right",
    "offsetX": 8,
    "offsetY": 0,
    "delay": 0,
    "followCursor": false
  }
}
```

禁用：

```json
{
  "tooltip": { "enabled": false }
}
```

XSON 中不存在 `type:"tooltip"`。Tooltip 只能作为 widget 的通用属性出现。

## 范例

- `examples/xui_tooltip`: 展示静态文本、锚点、延迟、光标跟随、动态 resolver，以及游戏装备自定义 tooltip。
- `examples/xui_tooltip_xson`: 展示 XSON 的 `tooltip` 字符串简写和对象属性写法。
