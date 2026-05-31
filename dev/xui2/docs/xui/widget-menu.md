# XUI Menu

Menu is a popup-backed command list. It is used for context menus, button menus, and nested command menus. The XUI2 implementation follows the XUI1 behavior model but renders the command list as its own cache-first widget and delegates overlay placement, outside close, Escape close, focus restore, and scroll viewport sizing to Popup.

## Goals

- preserve XUI1 menu semantics: item measurement, hover hit testing, keyboard navigation, shortcuts, check/radio state, and submenu opening
- implement Menu as a Popup specialization instead of a ListView wrapper
- keep the menu viewport stable by using Popup's ScrollView / ScrollFrame payload path
- support explicit enabled/disabled item state
- keep XSON deferred

## Structure

`xuiMenuCreate` returns the menu content/controller widget. Internally, it creates a Popup and adds the menu widget to the Popup content widget.

```text
Popup overlay shell
  popup panel
    ScrollView
      ScrollFrame
        viewport widget
          content widget
            Menu content widget
```

Use `xuiMenuGetPopupWidget(menu)` when lower-level Popup inspection is needed. Application code normally opens or closes through `xuiMenuOpenForOwner`, `xuiMenuOpenAt`, and `xuiMenuClose`.

## Items

Menu items are fixed in the menu's internal item array. Text pointers are not copied; the caller must keep strings alive for as long as the menu uses them.

```c
typedef struct xui_menu_item_t {
	const char* sText;
	const char* sShortcut;
	int iType;
	uint32_t iState;
	int iValue;
	int iIcon;
	xui_widget_t* pSubmenu;
	void* pUser;
} xui_menu_item_t;
```

Item types:

- `XUI_MENU_ITEM_NORMAL`
- `XUI_MENU_ITEM_SEPARATOR`
- `XUI_MENU_ITEM_CHECK`
- `XUI_MENU_ITEM_RADIO`
- `XUI_MENU_ITEM_SUBMENU`

Item state flags:

- `XUI_MENU_ITEM_ENABLED`
- `XUI_MENU_ITEM_CHECKED`
- `XUI_MENU_ITEM_DEFAULT`
- `XUI_MENU_ITEM_DANGER`

Enabled is explicit. A command with `iState == 0` is disabled unless it is a separator.

## Behavior

Mouse:

- moving over an enabled item updates the hover index
- moving over a submenu item opens the submenu immediately
- clicking a normal/check/radio item commits the command and closes the root menu
- clicking a disabled item does not select or close the menu

Keyboard:

- Up/Down moves hover between enabled non-separator items
- Right opens the hovered submenu
- Left closes a submenu and restores focus to the parent menu
- Enter/Space commits the hovered item
- Escape closes the root menu
- shortcuts commit through the same path as mouse selection

Shortcut parsing supports exact modifier matching for `Ctrl`, `Alt`, `Shift`, and `Super` plus letters, digits, Space, Tab, Enter, Escape, arrows, Home/End, PageUp/PageDown, Delete, Backspace, and F1-F24 raw key codes.

Check and radio items:

- check items toggle `XUI_MENU_ITEM_CHECKED`
- radio items clear checked state from the contiguous radio segment before selecting the clicked item

Submenus:

- a submenu is another Menu widget assigned through `pSubmenu`
- a submenu is opened with a fixed anchor at the parent item right edge
- root outside-close handling closes the whole chain

## Metrics And Colors

Defaults match the XUI1 menu proportions: 24px command rows, 9px separators, a 22px mark lane, shortcut gap, arrow lane, and a 112px minimum width.

Customize with:

```c
xui_menu_metrics_t metrics;
xui_menu_colors_t colors;
xuiMenuGetMetrics(menu, &metrics);
xuiMenuGetColors(menu, &colors);
xuiMenuSetMetrics(menu, &metrics);
xuiMenuSetColors(menu, &colors);
```

`fMaxHeight` is passed to the Popup as max content viewport height. Popup keeps the requested content size and shows compact scrollbars when needed.

## Public API

```c
xuiMenuGetType
xuiMenuCreate
xuiMenuSetItems
xuiMenuAddItem
xuiMenuAddSeparator
xuiMenuClear
xuiMenuGetItemCount
xuiMenuGetItem
xuiMenuGetItemRect
xuiMenuSetItemState
xuiMenuGetItemState
xuiMenuGetHoverIndex
xuiMenuSetHoverIndex
xuiMenuCommitHover
xuiMenuSetSelect
xuiMenuGetSelectCount
xuiMenuSetFont
xuiMenuGetFont
xuiMenuSetMetrics
xuiMenuGetMetrics
xuiMenuSetColors
xuiMenuGetColors
xuiMenuMeasure
xuiMenuOpenAt
xuiMenuOpenForOwner
xuiMenuClose
xuiMenuIsOpen
xuiMenuGetPopupWidget
xuiMenuGetContentWidget
xuiMenuGetOwner
```

## Example

```c
xui_menu_desc_t desc;
xui_menu_item_t items[3];

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pOwner = fileButton;
desc.pFont = font;
xuiMenuCreate(context, &menu, &desc);

memset(items, 0, sizeof(items));
items[0].sText = "New scene";
items[0].sShortcut = "Ctrl+N";
items[0].iType = XUI_MENU_ITEM_NORMAL;
items[0].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT;
items[0].iValue = 10;
items[1].sText = "Snap to grid";
items[1].sShortcut = "G";
items[1].iType = XUI_MENU_ITEM_CHECK;
items[1].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED;
items[1].iValue = 20;
items[2].iType = XUI_MENU_ITEM_SEPARATOR;

xuiMenuSetItems(menu, items, 3);
xuiMenuSetSelect(menu, onMenuSelect, user);
xuiMenuOpenForOwner(menu, fileButton);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
examples\xui_menu\build.bat
build\xui_menu.exe --frames 360
```

The example summary should include `create=1`, `layout=1`, `select=1`, `check=1`, `submenu=1`, and `close=1`.
