# XUI ScrollBar

ScrollBar 是滚动值控制控件，适合独立表达一个滚动范围，也可作为后续 ScrollView/ListView 等容器的视觉和交互基础。它和 Slider 的区别在于 ScrollBar 有 page 概念、thumb 尺寸由可见比例决定，并可选择显示两端按钮。

## 设计原则

- Widget 负责通用状态、布局、背景、边框和启用/禁用；ScrollBar 只负责轨道、thumb、两端按钮和按钮图标。
- 支持水平和垂直方向。水平从左到右递增，垂直从上到下递增，符合滚动条使用习惯。
- 支持 full 与 compact 两种模式。full 用于传统滚动条，compact 用于轻量悬浮或窄条场景。
- 两端按钮使用 buttonMode 控制：auto、off、on。auto 会根据控件尺寸和模式决定是否展示。
- 颜色、尺寸、按钮颜色、thumb 最小尺寸均可配置。
- 内置渲染缓存。状态、数值、尺寸、颜色、模式变化会自动使缓存失效。

## API

```c
int xgeXuiScrollBarInit(xge_xui_scrollbar pScrollBar, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiScrollBarUnit(xge_xui_scrollbar pScrollBar);
void xgeXuiScrollBarSetChange(xge_xui_scrollbar pScrollBar, xge_xui_slider_proc procChange, void* pUser);
void xgeXuiScrollBarSetRange(xge_xui_scrollbar pScrollBar, float fMin, float fMax, float fPage);
void xgeXuiScrollBarSetPage(xge_xui_scrollbar pScrollBar, float fPage);
void xgeXuiScrollBarSetValue(xge_xui_scrollbar pScrollBar, float fValue);
float xgeXuiScrollBarGetValue(xge_xui_scrollbar pScrollBar);
void xgeXuiScrollBarSetOrientation(xge_xui_scrollbar pScrollBar, int iOrientation);
void xgeXuiScrollBarSetMode(xge_xui_scrollbar pScrollBar, int iMode);
void xgeXuiScrollBarSetButtonMode(xge_xui_scrollbar pScrollBar, int iMode);
void xgeXuiScrollBarSetMetrics(xge_xui_scrollbar pScrollBar, float fTrackSize, float fMinThumbSize, float fThumbRadius, float fButtonSize);
void xgeXuiScrollBarSetColors(xge_xui_scrollbar pScrollBar, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
void xgeXuiScrollBarSetButtonColors(xge_xui_scrollbar pScrollBar, uint32_t iButton, uint32_t iIcon);
void xgeXuiScrollBarSetCacheMode(xge_xui_scrollbar pScrollBar, int iMode);
```

`iMode` 使用 `XGE_XUI_SCROLLBAR_MODE_FULL` 或 `XGE_XUI_SCROLLBAR_MODE_COMPACT`。

`iButtonMode` 使用：

- `XGE_XUI_SCROLLBAR_BUTTONS_AUTO`
- `XGE_XUI_SCROLLBAR_BUTTONS_OFF`
- `XGE_XUI_SCROLLBAR_BUTTONS_ON`

`xgeXuiScrollBarSetMetrics` 中 `fThumbRadius < 0` 表示自动半径，`fButtonSize <= 0` 表示按钮尺寸取控件短边。

## XSON

```json
{
  "type": "scrollbar",
  "min": 0,
  "max": 100,
  "page": 24,
  "value": 40,
  "orientation": "vertical",
  "mode": "full",
  "buttons": "auto",
  "trackSize": 8,
  "minThumbSize": 18,
  "thumbRadius": -1,
  "buttonSize": 0,
  "trackColor": "#FFFFFFFF",
  "thumbColor": "#B9D0E2F5",
  "hoverColor": "#9EB9D0FA",
  "activeColor": "#84A0BCFF",
  "buttonColor": "#F8FBFFFF",
  "buttonIconColor": "#68849EFF",
  "disabledColor": "#B4BAC4FF",
  "cacheMode": "auto"
}
```

`orientation` 可为 `horizontal` 或 `vertical`。`mode` 可为 `full` 或 `compact`。`buttons` 可为 `auto`、`off`、`on`，也可用布尔值映射为 on/off。`cacheMode` 可为 `auto`、`off`、`force`。

## 测试覆盖

专属范例：

- `examples/xui_scrollbar`
- `examples/xui_scrollbar_xson`

范例覆盖水平 full、水平 compact、按钮 on/off、自定义配色、自定义尺寸、禁用状态、垂直 full、垂直 compact、cache force，以及 XSON 声明式加载。
