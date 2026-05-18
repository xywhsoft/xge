# XUI Menu

Menu 是弹出式命令菜单控件，用于右键菜单、按钮菜单和多级菜单。新实现不再基于 ListView；Menu 自己负责菜单项测量、绘制、命中测试、悬停、键盘导航和子菜单，Popup 只负责弹层、定位、回退、滚动视口、关闭策略和焦点。

## 设计口径

- 不保留旧的 `SetEnabledItems` / `SetSize` / ListView 内部访问口径。
- 菜单项在添加、删除、修改时重新测量；打开菜单时只使用已缓存的尺寸。
- 菜单内容大小由菜单项数量、最长文本、快捷键文本、图标/勾选/箭头区域共同决定。
- Popup 不改变菜单申请的内容大小；窗口空间不足时由 Popup 提供滚动视口。
- 子菜单悬停立即打开。
- 菜单事件坐标使用 Popup 的内容 Widget 坐标，不需要控件自己扣除滚动偏移。

## 菜单项

`xge_xui_menu_item_t` 字段：

- `sText`: 主文本。
- `sShortcut`: 快捷键文本，例如 `Ctrl+S`。
- `iType`: `NORMAL`、`SEPARATOR`、`CHECK`、`RADIO`、`SUBMENU`。
- `iState`: `ENABLED`、`CHECKED`、`DEFAULT`、`DANGER`。
- `iValue`: 回调值，优先用它表达业务语义。
- `iIcon`: 图标占位 ID。当前绘制轻量图标标记，后续可接入统一 icon atlas。
- `pSubmenu`: 子菜单指针。
- `pUser`: 菜单项自定义数据。

## 热键

菜单打开后会响应菜单项的 `sShortcut`。支持 `Ctrl`、`Alt`、`Shift`、`Super/Win/Cmd` 组合，以及字母、数字、`Del/Delete`、`Enter/Return`、`Esc/Escape`、`Space`、`Tab`、`Backspace`、方向键、`PageUp/PgUp`、`PageDown/PgDn`、`Home`、`End`、`F1`-`F24`。

匹配要求修饰键完全一致，例如 `Ctrl+S` 不会被 `Ctrl+Shift+S` 触发。热键触发时和鼠标点击菜单项走同一条提交路径：`CHECK` 会切换状态，连续 `RADIO` 段会自动互斥，然后调用 `xge_xui_menu_select_proc` 并关闭根菜单。

## API

```c
int xgeXuiMenuInit(xge_xui_menu pMenu, xge_xui_context pContext);
void xgeXuiMenuSetItems(xge_xui_menu pMenu, const xge_xui_menu_item_t* arrItems, int iCount);
int xgeXuiMenuAddItem(xge_xui_menu pMenu, const xge_xui_menu_item_t* pItem);
int xgeXuiMenuAddSeparator(xge_xui_menu pMenu);
void xgeXuiMenuSetItemState(xge_xui_menu pMenu, int iIndex, int iState);
void xgeXuiMenuSetSelect(xge_xui_menu pMenu, xge_xui_menu_select_proc procSelect, void* pUser);
void xgeXuiMenuOpenAt(xge_xui_menu pMenu, xge_xui_widget pOwner, float fX, float fY);
void xgeXuiMenuOpenForOwner(xge_xui_menu pMenu, xge_xui_widget pOwner);
void xgeXuiMenuClose(xge_xui_menu pMenu);
```

## XSON

`items` 支持字符串和对象：

```json
{
  "type": "menu",
  "owner": "button",
  "items": [
    "Open",
    { "text": "Save", "shortcut": "Ctrl+S", "value": 10 },
    { "type": "separator" },
    { "text": "Danger", "danger": true, "enabled": true }
  ]
}
```

对象字段：`text`、`shortcut`、`type`、`separator`、`enabled`、`checked`、`danger`、`value`、`icon`。

## 当前重构状态

Menu 旧实现已从编译入口隔离。恢复时必须复用新的 Popup/ScrollView 路径，菜单本身只负责测量、绘制、命中、悬停、键盘导航、提交和子菜单关系。
