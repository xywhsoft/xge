# XUI MenuBar

MenuBar is a top-level menu strip widget. It owns the horizontal command headers and opens existing `xuiMenu` widgets as Popup-backed dropdowns.

## Goals

- preserve XUI1 menu bar behavior: top-level item layout, `&` mnemonic parsing, Alt shortcuts, F10 focus, arrow-key switching, and click-to-open/click-to-close
- reuse Menu and Popup for dropdown rows, keyboard commit, outside close, Escape close, owner close, focus restore, and scroll sizing
- keep the MenuBar itself cache-first and styleable
- support direct command items without a dropdown menu
- keep XSON deferred

## Structure

`xuiMenuBarCreate` returns the menu strip widget. Each top-level item may reference a `xuiMenu` instance:

```text
MenuBar widget
  File item -> xuiMenu
    Popup
      ScrollView / ScrollFrame
        Menu content rows
  Edit item -> xuiMenu
  Help item -> no menu, direct command callback
```

MenuBar does not create the dropdown Menu widgets by itself. Create menus separately, set their owner to the MenuBar when practical, and attach them through `xui_menubar_item_t.pMenu` or `xuiMenuBarSetItemMenu`.

## Items

```c
typedef struct xui_menubar_item_t {
	const char* sText;
	int iState;
	int iValue;
	int iMnemonic;
	xui_widget_t* pMenu;
	xui_rect_t tRect;
	void* pUser;
} xui_menubar_item_t;
```

Item state:

- `XUI_MENUBAR_ITEM_ENABLED`

Enabled is explicit for `xuiMenuBarSetItems`: an item with `iState == 0` is disabled. `xuiMenuBarAddItem` creates enabled items by default.

Text pointers are not copied; keep item strings alive while the MenuBar uses them.

## Mnemonics

MenuBar follows the XUI1 `&` convention:

- `&File` displays as `File` and registers `Alt+F`
- `E&dit` displays as `Edit` and registers `Alt+D`
- `&&` displays a literal `&`

`iMnemonic` overrides the parsed mnemonic when non-zero. Use `xuiMenuBarSetItemMnemonic` when the displayed text cannot carry a simple ASCII mnemonic.

## Interaction

Pointer:

- moving over an enabled item updates hover
- clicking an item with a menu opens that menu under the item
- clicking an already-open item closes the menu bar stack
- moving to another top-level item while a menu is open switches the open dropdown
- clicking an enabled item without a menu calls the MenuBar select callback with that item's value

Keyboard:

- `Alt+mnemonic` opens the matching top-level menu or commits a direct command
- `F10` and Context Menu focus the MenuBar and select the first enabled top-level item
- Left/Right moves between top-level items; when a menu is open it switches the open dropdown
- Down opens the current top-level menu or moves hover inside the open dropdown
- Up moves hover inside the open dropdown
- Enter/Space opens the current top-level menu, commits a direct top-level command, or commits the hovered dropdown menu item
- Escape closes the open dropdown and restores MenuBar focus

Popup outside-close uses a short open guard to avoid destroying the menu on the same mouse operation that opened it. After that, normal pointer operations outside the menu close it through Popup policy. Because the Popup shield receives pointer events while a dropdown is open, MenuBar also synchronizes top-level hover switching during update from the current pointer position.

## Callback

MenuBar uses the same callback type as Menu:

```c
xuiMenuBarSetSelect(menubar, onSelect, user);
```

When a dropdown menu is opened, the MenuBar propagates its callback to that Menu. This keeps top-level direct commands and dropdown commands on one selection path. The callback sender is the top-level MenuBar for direct commands and the Menu widget for dropdown commands.

## Metrics And Colors

Defaults use the current XUI2 light-blue control style: 26px height, compact rounded top-level items, transparent idle item background, blue active item, and muted disabled text.

Customize with:

```c
xui_menubar_metrics_t metrics;
xui_menubar_colors_t colors;
xuiMenuBarGetMetrics(menubar, &metrics);
xuiMenuBarGetColors(menubar, &colors);
xuiMenuBarSetMetrics(menubar, &metrics);
xuiMenuBarSetColors(menubar, &colors);
```

## Public API

```c
xuiMenuBarGetType
xuiMenuBarCreate
xuiMenuBarSetItems
xuiMenuBarAddItem
xuiMenuBarClear
xuiMenuBarGetItemCount
xuiMenuBarGetItem
xuiMenuBarGetItemRect
xuiMenuBarSetItemMenu
xuiMenuBarSetItemEnabled
xuiMenuBarIsItemEnabled
xuiMenuBarSetItemMnemonic
xuiMenuBarGetHoverIndex
xuiMenuBarSetHoverIndex
xuiMenuBarGetActiveIndex
xuiMenuBarGetOpenIndex
xuiMenuBarOpenItem
xuiMenuBarClose
xuiMenuBarIsOpen
xuiMenuBarGetOpenMenu
xuiMenuBarSetSelect
xuiMenuBarSetFont
xuiMenuBarGetFont
xuiMenuBarSetMetrics
xuiMenuBarGetMetrics
xuiMenuBarSetColors
xuiMenuBarGetColors
xuiMenuBarGetState
xuiMenuBarGetChangeCount
```

## Constants

```c
XUI_MENUBAR_ITEM_CAPACITY
XUI_MENUBAR_ITEM_ENABLED
XUI_MENUBAR_STATE_OPEN
XUI_KEY_F10
```

## Style Properties

```text
font.name
menubar.background.color
menubar.border.color
menubar.item.color
menubar.item.hover_color
menubar.item.active_color
menubar.text.color
menubar.text.disabled_color
menubar.focus.color
menubar.height
menubar.padding.x
menubar.padding.y
menubar.item.padding.x
menubar.item.gap
menubar.radius
menubar.border.width
```

Color properties affect cache rendering. Height, padding, item gap, and item padding participate in layout and hit testing.

## Example

```c
xui_menubar_item_t items[3];

memset(items, 0, sizeof(items));
items[0].sText = "&File";
items[0].iState = XUI_MENUBAR_ITEM_ENABLED;
items[0].iValue = 1;
items[0].pMenu = fileMenu;
items[1].sText = "&Edit";
items[1].iState = XUI_MENUBAR_ITEM_ENABLED;
items[1].iValue = 2;
items[1].pMenu = editMenu;
items[2].sText = "&Help";
items[2].iState = XUI_MENUBAR_ITEM_ENABLED;
items[2].iValue = 99;

xuiMenuBarSetItems(menubar, items, 3);
xuiMenuBarSetSelect(menubar, onMenuCommand, user);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menubar_test.bat
examples\xui_menubar\build.bat
build\xui_menubar.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `open=1`, `switch=1`, `mnemonic=1`, `select=1`, and `close=1`.
