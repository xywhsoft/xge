# XUI MenuBar

`MenuBar` 是主菜单栏控件，用于主窗口或 `Window` 控件内部的顶部菜单。它负责顶层菜单项布局、鼠标打开菜单、菜单间切换、键盘导航和 Alt 助记键；弹出的菜单内容仍由 `Menu` 控件负责。

## 设计思路

- 顶层菜单栏属于窗口 chrome 区域，默认读取 `xge_xui_chrome_style_t.tBarColors` 和 `tMenuBarMetrics`。
- 顶层 item 只表达入口语义，不重复实现菜单项绘制细节。
- `&File` 这类文本会显示为 `File`，并把 `F` 作为 Alt 助记键。
- 绑定的 `Menu` 使用同一套 chrome popup menu 配色，打开位置跟随顶层 item。
- `MenuBar` 的事件返回值仍是 XUI 内部 consumed/continue；应用层事件回调应返回 `XGE_OK`，不要把 consumed 当作程序退出码。

## 数据结构

```c
typedef struct xge_xui_menubar_item_t {
    const char* sText;
    int iState;
    int iValue;
    int iMnemonic;
    xge_xui_menu pMenu;
    xge_rect_t tRect;
} xge_xui_menubar_item_t;
```

`iState` 当前支持 `XGE_XUI_MENUBAR_ITEM_ENABLED`。`iMnemonic` 可显式设置；为 0 时会从 `sText` 的 `&` 标记中自动提取。

## API

```c
int xgeXuiMenuBarInit(xge_xui_menubar pMenuBar, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiMenuBarUnit(xge_xui_menubar pMenuBar);
void xgeXuiMenuBarSetItems(xge_xui_menubar pMenuBar, const xge_xui_menubar_item_t* arrItems, int iCount);
int xgeXuiMenuBarAddItem(xge_xui_menubar pMenuBar, const char* sText, xge_xui_menu pMenu, int iValue);
void xgeXuiMenuBarSetItemMenu(xge_xui_menubar pMenuBar, int iIndex, xge_xui_menu pMenu);
void xgeXuiMenuBarSetItemEnabled(xge_xui_menubar pMenuBar, int iIndex, int bEnabled);
void xgeXuiMenuBarSetFont(xge_xui_menubar pMenuBar, xge_font pFont);
void xgeXuiMenuBarSetSelect(xge_xui_menubar pMenuBar, xge_xui_menu_select_proc procSelect, void* pUser);
void xgeXuiMenuBarSetMetrics(xge_xui_menubar pMenuBar, const xge_xui_bar_metrics_t* pMetrics);
void xgeXuiMenuBarSetColors(xge_xui_menubar pMenuBar, const xge_xui_bar_colors_t* pColors);
```

## 交互

- 鼠标按下顶层 item：打开绑定菜单。
- 菜单打开后移动到其他顶层 item：切换到对应菜单。
- `Alt + mnemonic`：打开对应顶层菜单。
- `F10` / `Menu`：聚焦菜单栏。
- `Left` / `Right`：切换顶层 item。
- `Down` / `Enter` / `Space`：打开当前 item。
- `Esc`：关闭当前顶层菜单。

## XSON

```json
{
  "type": "menubar",
  "height": 26,
  "items": [
    {
      "text": "&File",
      "value": 1,
      "items": [
        { "text": "New", "shortcut": "Ctrl+N", "value": 10 },
        { "separator": true },
        { "text": "Exit", "shortcut": "Alt+F4", "danger": true, "value": 99 }
      ]
    },
    {
      "text": "&View",
      "items": [
        { "text": "Show toolbar", "type": "check", "checked": true }
      ]
    }
  ]
}
```

顶层 item 字段：`text`、`value`、`enabled`、`mnemonic`、`items`。其中 `items` 会创建一个页面托管的 `Menu`。

## 范例

- `examples/xui_menubar`
- `examples/xui_menubar_xson`
