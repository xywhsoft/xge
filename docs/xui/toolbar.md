# XUI Toolbar

`Toolbar` 是窗口 chrome 区域的命令条控件。它用于放置短命令、切换命令、分组分隔符和溢出按钮。

## 设计思路

- 默认背景、边框、hover、active、checked、disabled、文字和分隔线颜色来自 `xge_xui_chrome_style_t.tBarColors`。
- `Toolbar` 本身只负责 item 布局和交互；背景和边框交给 Widget。
- item 可以是普通按钮、toggle、separator。
- 支持水平和垂直方向。
- 支持分组间距、禁用项、tooltip、溢出按钮。

## Item

```c
typedef struct xge_xui_toolbar_item_t {
    const char* sText;
    const char* sTooltip;
    int iType;
    int bEnabled;
    int bChecked;
    int iGroup;
    xge_rect_t tRect;
} xge_xui_toolbar_item_t;
```

`iType` 支持：

- `XGE_XUI_TOOLBAR_ITEM_BUTTON`
- `XGE_XUI_TOOLBAR_ITEM_TOGGLE`
- `XGE_XUI_TOOLBAR_ITEM_SEPARATOR`

## API

```c
int xgeXuiToolbarInit(xge_xui_toolbar pToolbar, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiToolbarSetItems(xge_xui_toolbar pToolbar, const char** arrText, const int* arrTypes, int iCount);
void xgeXuiToolbarSetOrientation(xge_xui_toolbar pToolbar, int iOrientation);
void xgeXuiToolbarSetItemSize(xge_xui_toolbar pToolbar, float fWidth, float fHeight, float fSeparatorSize);
void xgeXuiToolbarSetGroupGap(xge_xui_toolbar pToolbar, float fGap);
void xgeXuiToolbarSetItemGroup(xge_xui_toolbar pToolbar, int iIndex, int iGroup);
void xgeXuiToolbarSetItemTooltip(xge_xui_toolbar pToolbar, int iIndex, const char* sText);
void xgeXuiToolbarSetOverflow(xge_xui_toolbar pToolbar, int bEnabled, float fButtonSize, xge_xui_click_proc procOverflow, void* pUser);
void xgeXuiToolbarSetItemEnabled(xge_xui_toolbar pToolbar, int iIndex, int bEnabled);
void xgeXuiToolbarSetItemChecked(xge_xui_toolbar pToolbar, int iIndex, int bChecked);
void xgeXuiToolbarSetSelect(xge_xui_toolbar pToolbar, xge_xui_select_proc procSelect, void* pUser);
```

## 交互

- `BUTTON`：点击后触发 `procSelect`。
- `TOGGLE`：点击后切换 `bChecked`，再触发 `procSelect`。
- `SEPARATOR`：不参与命中和键盘选择。
- 键盘焦点在 Toolbar 上时，`Enter` / `Space` 会触发当前 hover item；无 hover 时触发第一个可用 item。
- 开启溢出后，放不下的 item 不绘制，右侧或底部绘制溢出按钮。

## XSON

```json
{
  "type": "toolbar",
  "height": 32,
  "itemWidth": 64,
  "itemHeight": 24,
  "separatorSize": 10,
  "items": [
    { "text": "New" },
    { "text": "Pin", "type": "toggle", "checked": true },
    { "type": "separator" },
    { "text": "Disabled", "enabled": false }
  ]
}
```

字段：`orientation`、`itemWidth`、`itemHeight`、`separatorSize`、`backgroundColor`、`itemColor`、`hoverColor`、`activeColor`、`checkedColor`、`focusColor`、`disabledColor`、`separatorColor`、`textColor`、`disabledTextColor`。

## 范例

- `examples/xui_toolbar_lab`
- `examples/xui_menubar`
- `examples/xui_menubar_xson`
