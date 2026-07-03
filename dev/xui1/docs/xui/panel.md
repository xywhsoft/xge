# XUI Panel

Panel 是一个风格化容器控件，用于表达“带标题的明确区域”。Widget 负责基础盒模型、布局、边框、背景和状态；Panel 在 Widget 之上提供标题栏、图标、标题文本和客户区的组合能力。

## 设计边界

- Widget 是基础设施，不表达业务风格。
- Panel 是最基础的风格化容器，用于分组内容、表单区域、设置区块等场景。
- Panel 不直接承载业务子控件，业务子控件应添加到 client widget。
- Panel 的标题栏由 header、icon、title 三个内部 widget 组成。
- Panel 的客户区是普通 widget，具备完整布局能力，可以设置 row、column、grid、stack 等布局。

## 内部结构

```text
panel widget
  header widget
    icon widget
    title widget
  client widget
    user children...
```

这种结构让 Panel 可以通过属性快速设置标题和图标，同时让用户通过 `xgeXuiPanelGetClientWidget` 直接拿到客户区继续布局。

## C API

```c
int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget);
void xgeXuiPanelUnit(xge_xui_panel pPanel);

xge_xui_widget xgeXuiPanelGetHeaderWidget(xge_xui_panel pPanel);
xge_xui_widget xgeXuiPanelGetIconWidget(xge_xui_panel pPanel);
xge_xui_widget xgeXuiPanelGetTitleWidget(xge_xui_panel pPanel);
xge_xui_widget xgeXuiPanelGetClientWidget(xge_xui_panel pPanel);

void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle);
void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor);
void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags);
void xgeXuiPanelSetIcon(xge_xui_panel pPanel, xge_texture pTexture, xge_rect_t tSrc);
void xgeXuiPanelSetIconSize(xge_xui_panel pPanel, float fSize);

void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor);
void xgeXuiPanelSetHeaderColor(xge_xui_panel pPanel, uint32_t iColor);
void xgeXuiPanelSetClientColor(xge_xui_panel pPanel, uint32_t iColor);
void xgeXuiPanelSetHeaderHeight(xge_xui_panel pPanel, float fHeight);
void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip);
```

## XSON

```json
{
  "type": "panel",
  "title": "Settings",
  "font": "@fonts.body",
  "icon": "@textures.gear",
  "iconSrc": [0, 0, 18, 18],
  "headerColor": "#E8F3FBFF",
  "clientColor": "#F8FCFFFF",
  "clientLayout": "column",
  "clientPadding": [8, 8, 8, 8],
  "clientGap": 6,
  "children": [
    { "type": "label", "text": "Panel children are attached to client." }
  ]
}
```

XSON 中 `children` 会自动添加到 Panel 的 client widget，而不是直接添加到 Panel 根 widget。

## 范例

- `examples/xui_panel`
- `examples/xui_panel_xson`

范例覆盖默认标题、图标标题、自定义配色、客户区布局、禁用状态、客户区裁剪和 XSON 子节点挂载。
