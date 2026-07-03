# XUI StatusBar

`StatusBar` 是窗口 chrome 区域底部的信息条控件。它用于展示状态文本、进度、固定空白、弹性空白和可点击状态项。

## 设计思路

- 默认背景、边框、item 状态色和文字色来自 `xge_xui_chrome_style_t.tBarColors`。
- `StatusBar` 只绘制顶部边线、状态项、文本和进度条；背景由 Widget 绘制。
- item 分为 left、center、right 三个 section，各 section 独立布局。
- spacer 可以是固定宽度，也可以是弹性权重。

## Item

```c
typedef struct xge_xui_status_bar_item_t {
    const char* sText;
    int iType;
    int iSection;
    int bClickable;
    int bEnabled;
    float fWidth;
    float fFlex;
    float fMin;
    float fMax;
    float fValue;
    xge_rect_t tRect;
} xge_xui_status_bar_item_t;
```

`iType` 支持：

- `XGE_XUI_STATUS_BAR_ITEM_TEXT`
- `XGE_XUI_STATUS_BAR_ITEM_PROGRESS`
- `XGE_XUI_STATUS_BAR_ITEM_SPACER`

`iSection` 支持：

- `XGE_XUI_STATUS_BAR_SECTION_LEFT`
- `XGE_XUI_STATUS_BAR_SECTION_CENTER`
- `XGE_XUI_STATUS_BAR_SECTION_RIGHT`

## API

```c
int xgeXuiStatusBarInit(xge_xui_status_bar pStatusBar, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiStatusBarClear(xge_xui_status_bar pStatusBar);
int xgeXuiStatusBarAddText(xge_xui_status_bar pStatusBar, int iSection, const char* sText, float fWidth, int bClickable);
int xgeXuiStatusBarAddProgress(xge_xui_status_bar pStatusBar, int iSection, float fMin, float fMax, float fValue, float fWidth);
int xgeXuiStatusBarAddSpacer(xge_xui_status_bar pStatusBar, int iSection, float fWidth);
int xgeXuiStatusBarAddFlexibleSpacer(xge_xui_status_bar pStatusBar, int iSection, float fWeight);
void xgeXuiStatusBarSetSelect(xge_xui_status_bar pStatusBar, xge_xui_select_proc procSelect, void* pUser);
void xgeXuiStatusBarSetItemEnabled(xge_xui_status_bar pStatusBar, int iIndex, int bEnabled);
void xgeXuiStatusBarSetItemText(xge_xui_status_bar pStatusBar, int iIndex, const char* sText);
void xgeXuiStatusBarSetProgress(xge_xui_status_bar pStatusBar, int iIndex, float fValue);
void xgeXuiStatusBarSetMetrics(xge_xui_status_bar pStatusBar, float fHeight, float fGap, float fItemPadding);
```

## 交互

只有 `bClickable != 0` 且 enabled 的非 spacer item 参与鼠标命中。点击释放在同一个 item 上时触发 `procSelect`。

## XSON

```json
{
  "type": "statusBar",
  "height": 26,
  "items": [
    { "text": "Ready", "width": 80, "clickable": true },
    { "type": "progress", "min": 0, "max": 100, "value": 62, "width": 120 },
    { "type": "spacer", "width": 260 },
    { "text": "XUI", "section": "right", "width": 58 }
  ]
}
```

字段：`section`、`type`、`text`、`width`、`clickable`、`enabled`、`min`、`max`、`value`、`barHeight/statusHeight`、`itemGap/gap`、`itemPadding` 以及颜色字段。

## 范例

- `examples/xui_status_bar_lab`
- `examples/xui_menubar`
- `examples/xui_menubar_xson`
