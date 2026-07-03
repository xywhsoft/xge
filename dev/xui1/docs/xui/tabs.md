# XUI Tabs

Tabs 是一个风格化页面容器，不再是旧式的独立选项卡条。它由选项卡按钮栏和页面客户区组成，负责管理页面的选中、禁用、提醒状态，以及页面 widget 的显示和隐藏。

## 设计边界

- Tabs 是容器控件，适合管理多个互斥页面。
- Tab 按钮使用 Button 基础设施，获得 hover、active、selected、disabled 和 badge 能力。
- 每个页面都是一个普通 widget，可以继续使用 XUI 布局系统。
- Tabs 只显示当前选中的页面，其他页面隐藏。
- 复杂页面内容不应该放在 tab 按钮里，而应放到对应 page widget 中。
- 默认视觉由 tabbar 分隔线和 client 三边框共同组成；选中的 tab 与页面客户区连成一个整体，不在相邻边重复画线。选中 tab 不改变按钮尺寸，只在靠外侧绘制 3px 强调条。

## 内部结构

```text
tabs widget
  tabbar widget
    button widget[0]
    button widget[1]
    ...
  client widget
    page widget[0]
    page widget[1]
    ...
```

TabBar 负责按钮排列，client 使用 stack 布局承载页面。用户可以通过 API 获取 tabbar、client、page 或 button widget 做进一步定制。

## C API

```c
int xgeXuiTabsInit(xge_xui_tabs pTabs, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiTabsUnit(xge_xui_tabs pTabs);

int xgeXuiTabsAddPage(xge_xui_tabs pTabs, const char* sTitle);
xge_xui_widget xgeXuiTabsGetTabBarWidget(xge_xui_tabs pTabs);
xge_xui_widget xgeXuiTabsGetClientWidget(xge_xui_tabs pTabs);
xge_xui_widget xgeXuiTabsGetPageWidget(xge_xui_tabs pTabs, int iIndex);
xge_xui_widget xgeXuiTabsGetButtonWidget(xge_xui_tabs pTabs, int iIndex);

void xgeXuiTabsSetSelected(xge_xui_tabs pTabs, int iIndex);
int xgeXuiTabsGetSelected(xge_xui_tabs pTabs);
void xgeXuiTabsSetEnabledItems(xge_xui_tabs pTabs, const int* arrEnabled, int iCount);
void xgeXuiTabsSetDirtyItems(xge_xui_tabs pTabs, const int* arrDirty, int iCount);
void xgeXuiTabsSetItems(xge_xui_tabs pTabs, const char** arrItems, int iCount);
void xgeXuiTabsSetFont(xge_xui_tabs pTabs, xge_font pFont);
void xgeXuiTabsSetTabSize(xge_xui_tabs pTabs, float fWidth, float fHeight);
void xgeXuiTabsSetColors(
    xge_xui_tabs pTabs,
    uint32_t iBackground,
    uint32_t iTab,
    uint32_t iHover,
    uint32_t iActive,
    uint32_t iFocus,
    uint32_t iDisabled,
    uint32_t iText,
    uint32_t iActiveText);
```

`SetItems` 保留为快速创建页面的入口；新代码更推荐 `AddPage + GetPageWidget`，语义更明确。

## XSON

```json
{
  "type": "tabs",
  "font": "@fonts.body",
  "selected": 1,
  "pages": [
    {
      "title": "Overview",
      "children": [
        { "type": "label", "text": "First page" }
      ]
    },
    {
      "title": "Inventory",
      "dirty": true,
      "selected": true,
      "children": [
        { "type": "button", "text": "Equip" }
      ]
    },
    {
      "title": "Disabled",
      "enabled": false,
      "children": [
        { "type": "label", "text": "Cannot select this page." }
      ]
    }
  ]
}
```

`pages[].children` 会添加到对应 page widget。`dirty` 会显示 tab badge，`enabled:false` 会禁用对应 tab 按钮，`selected:true` 可以指定初始页。

## 范例

- `examples/xui_tabs`
- `examples/xui_tabs_xson`

范例覆盖默认页面、选中页、禁用页、dirty badge、自定义配色、页面客户区布局和 XSON 页面描述。
