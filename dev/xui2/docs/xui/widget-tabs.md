# XUI Tabs

Tabs is a cache-first page container widget. It keeps the mature XUI1 behavior while moving the implementation to XUI2 typed widgets, cache rendering, style properties, and normal child containers.

## Goals

- preserve the XUI1 model: tab bar, tab button widgets, client widget, and page widgets
- selected page is visible; unselected pages stay hidden
- disabled tabs cannot be selected and are skipped by keyboard navigation
- close hit targets notify a close callback without changing selection
- scrollable tab bars support wheel scrolling and selected-tab auto reveal
- dirty and icon metadata reserve tab text space
- selected tabs overlap the client border by 1px, and tab buttons skip the shared edge to avoid a double separator line
- keep XSON deferred

## Structure

`xuiTabsCreate` returns the Tabs root widget:

```text
Tabs root widget
  tabbar widget
    tab button widget 0
    tab button widget 1
    ...
  client widget
    page widget 0
      user children
    page widget 1
      user children
    ...
```

Use `xuiTabsGetPageWidget` or `xuiTabsAddPageChild` to attach application content. Tab button widgets are internal but can be queried for advanced tests and integration.

## Interaction

- `xuiTabsSetSelected` ignores disabled items.
- Pointer click on a disabled tab does not select or notify.
- Left/Right and Up/Down keys cycle through enabled tabs and skip disabled tabs.
- Home and End select the first or last enabled tab.
- If close buttons are enabled, clicking the close rect calls `xui_tabs_close_proc` and does not call the select callback.
- Mouse wheel on the tab bar changes `scrollX` when scrolling is enabled.
- Selecting a tab calls `xuiTabsEnsureVisible` internally so hidden tabs scroll into view.

## Placement

Tabs supports:

```c
XUI_TABS_PLACEMENT_TOP
XUI_TABS_PLACEMENT_BOTTOM
XUI_TABS_PLACEMENT_LEFT
XUI_TABS_PLACEMENT_RIGHT
```

For left and right placement, tab labels are drawn vertically and the tab width/height axes are swapped.

## Public API

```c
xuiTabsGetType
xuiTabsCreate
xuiTabsSetSelect
xuiTabsSetClose
xuiTabsAddPage
xuiTabsSetItems
xuiTabsGetItemCount
xuiTabsGetItemText
xuiTabsGetTabBarWidget
xuiTabsGetClientWidget
xuiTabsGetPageWidget
xuiTabsGetButtonWidget
xuiTabsAddPageChild
xuiTabsSetEnabledItems
xuiTabsSetDirtyItems
xuiTabsSetIcons
xuiTabsSetFont
xuiTabsGetFont
xuiTabsSetSelected
xuiTabsGetSelected
xuiTabsSetTabSize
xuiTabsGetTabSize
xuiTabsSetPlacement
xuiTabsGetPlacement
xuiTabsSetScrollable
xuiTabsIsScrollable
xuiTabsSetScroll
xuiTabsGetScroll
xuiTabsGetMaxScroll
xuiTabsEnsureVisible
xuiTabsSetColors
xuiTabsGetColors
xuiTabsGetTabBarRect
xuiTabsGetClientRect
xuiTabsGetTabRect
xuiTabsGetTextRect
xuiTabsGetIconRect
xuiTabsGetDirtyRect
xuiTabsGetCloseRect
xuiTabsGetHoverIndex
xuiTabsGetActiveIndex
xuiTabsGetCloseHoverIndex
xuiTabsGetCloseActiveIndex
xuiTabsGetState
xuiTabsGetChangeCount
xuiTabsGetCloseCount
```

## Constants

```c
XUI_TABS_PAGE_CAPACITY
XUI_TABS_TITLE_CAPACITY
XUI_TABS_PLACEMENT_TOP
XUI_TABS_PLACEMENT_BOTTOM
XUI_TABS_PLACEMENT_LEFT
XUI_TABS_PLACEMENT_RIGHT
XUI_TABS_STATE_OPEN
```

## Style Properties

```text
tabs.background.color
tabs.tab.color
tabs.tab.hover_color
tabs.tab.active_color
tabs.tab.focus_color
tabs.tab.disabled_color
tabs.text.color
tabs.text.active_color
tabs.border.color
tabs.client.color
tabs.tab.width
tabs.tab.height
```

Color properties affect cache rendering. Tab width and height participate in layout and invalidate tab geometry.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_tabs_test.bat
examples\xui_tabs\build.bat
build\xui_tabs.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `disabled=1`, `keyboard=1`, `close=1`, `scroll=1`, and `meta=1`.
