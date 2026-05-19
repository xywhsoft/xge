# XUI Accordion

Accordion 是一个风格化的动态容器控件，用于把多组内容折叠到同一个纵向区域中。它不再自己绘制段落文本，而是基于 Widget、Button 和布局系统拼装：每个 section 由 header button 和 client widget 组成，业务控件全部添加到 client widget。

## 设计边界

- Widget 负责盒模型、边框、背景、可见性、启用状态和布局。
- Accordion 负责 section 管理、展开/折叠状态、单展开/多展开策略、header 样式和 client 入口。
- Header 使用 Button 基础设施，因此天然具备 hover、pressed、selected、disabled、缓存和文本裁剪能力。
- Client 是普通 widget，可继续使用 column、row、grid、stack 等布局。
- 折叠时 client widget 会设置为不可见；不可见子节点不参与布局测量和排列，因此父容器会随折叠状态重新计算尺寸。
- Accordion 不再内建内容文本。需要文本时，在 client 中添加 Label；需要操作时，在 client 中添加 Button/Input 等控件。

## 内部结构

```text
accordion widget
  section widget[0]
    header widget
      button control
    client widget
      user children...
  section widget[1]
    header widget
      button control
    client widget
      user children...
```

这种结构的重点是便利性：用户通过 API 创建 section，然后直接拿到 client widget 分配布局和添加子控件。

## C API

```c
int xgeXuiAccordionInit(xge_xui_accordion pAccordion, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiAccordionUnit(xge_xui_accordion pAccordion);
void xgeXuiAccordionClear(xge_xui_accordion pAccordion);

int xgeXuiAccordionAddSection(xge_xui_accordion pAccordion, const char* sTitle, int bExpanded, int iId);
int xgeXuiAccordionGetSectionCount(xge_xui_accordion pAccordion);

xge_xui_widget xgeXuiAccordionGetSectionWidget(xge_xui_accordion pAccordion, int iIndex);
xge_xui_widget xgeXuiAccordionGetHeaderWidget(xge_xui_accordion pAccordion, int iIndex);
xge_xui_widget xgeXuiAccordionGetButtonWidget(xge_xui_accordion pAccordion, int iIndex);
xge_xui_widget xgeXuiAccordionGetClientWidget(xge_xui_accordion pAccordion, int iIndex);

int xgeXuiAccordionIsExpanded(xge_xui_accordion pAccordion, int iIndex);
void xgeXuiAccordionSetExpanded(xge_xui_accordion pAccordion, int iIndex, int bExpanded);
void xgeXuiAccordionSetSectionEnabled(xge_xui_accordion pAccordion, int iIndex, int bEnabled);
void xgeXuiAccordionSetMode(xge_xui_accordion pAccordion, int iMode);

void xgeXuiAccordionSetFont(xge_xui_accordion pAccordion, xge_font pFont);
void xgeXuiAccordionSetMetrics(xge_xui_accordion pAccordion, float fHeaderHeight, float fSpacing, float fContentPadding);
void xgeXuiAccordionSetSelect(xge_xui_accordion pAccordion, xge_xui_select_proc procSelect, void* pUser);
void xgeXuiAccordionSetColors(
    xge_xui_accordion pAccordion,
    uint32_t iBackground,
    uint32_t iHeader,
    uint32_t iHover,
    uint32_t iExpanded,
    uint32_t iContent,
    uint32_t iBorder,
    uint32_t iText);
float xgeXuiAccordionGetContentHeight(xge_xui_accordion pAccordion);
```

`xgeXuiAccordionAddSection` 返回 section index。常规用法是立刻调用 `xgeXuiAccordionGetClientWidget`，然后给 client 设置布局并添加子控件。

```c
int iSection = xgeXuiAccordionAddSection(&accordion, "Audio", 1, 100);
xge_xui_widget client = xgeXuiAccordionGetClientWidget(&accordion, iSection);
xgeXuiWidgetSetLayout(client, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetGap(client, 6.0f);
xgeXuiWidgetAdd(client, volumeSliderWidget);
```

## XSON

```json
{
  "type": "accordion",
  "font": "@fonts.body",
  "mode": "single",
  "headerHeight": 28,
  "spacing": 4,
  "contentPadding": 8,
  "background": "#F6FBFFFF",
  "headerColor": "#E8EEF7FF",
  "hoverColor": "#DAE6F6FF",
  "expandedColor": "#2E7CD6FF",
  "contentColor": "#F8FCFFFF",
  "borderColor": "#7FC4E5FF",
  "textColor": "#242A34FF",
  "sections": [
    {
      "title": "General",
      "expanded": true,
      "clientLayout": "column",
      "clientGap": 6,
      "children": [
        { "type": "label", "text": "Children are added to the client widget." }
      ]
    },
    {
      "title": "Advanced",
      "expanded": false,
      "enabled": false,
      "children": [
        { "type": "button", "text": "Disabled header keeps its client definition." }
      ]
    }
  ]
}
```

Section 支持：

- `title`: header 标题。
- `id`: section 业务 ID。
- `expanded`: 初始展开状态。
- `enabled`: header 是否可点击。
- `clientLayout` / `layout`: client widget 布局。
- `clientPadding`: client widget 内边距。
- `clientGap` / `gap`: client widget 子节点间距。
- `children`: 添加到 client widget 的子控件数组。

## 范例

- `examples/xui_accordion`
- `examples/xui_accordion_xson`

范例覆盖多展开、单展开、折叠布局收缩、禁用 header、自定义配色、自定义 padding、client 子布局、按钮子控件和 XSON 子节点挂载。
