# XUI Page

Page is a cache-first pagination control migrated from XUI1 Pager. It displays page buttons, navigation buttons, and ellipsis markers for large page ranges.

## Goals

- preserve XUI1 Pager behavior: page count/current page, total/page-size conversion, windowed page numbers, first/previous/next/last buttons, hover/active/focus/disabled states, click activation, keyboard navigation, and change callback
- render through XUI2 widget cache and proxy shape/text APIs
- expose item geometry and metadata for integration and tests
- keep XSON deferred

## Behavior

Default page count is `1`, current page is `1`, and default page window size is `5`. Window size is normalized to an odd positive value and capped by the internal item capacity.

`xuiPageSetPageCount` clamps the page count to at least `1` and clamps the current page to the new range. `xuiPageSetCurrent` clamps the target page. Programmatic changes notify only when `bNotify` is non-zero. User interaction always notifies when the page changes.

Input support:

- left-click an enabled item: capture, show active state, and activate on pointer-up when released over the same item
- keyboard while focused: Left/Up moves to previous page, Right/Down moves to next page, Home moves to page `1`, End moves to the last page, Escape cancels capture

The rendered item order matches XUI1:

```text
Prev First [ellipsis] page-window [ellipsis] Last Next
```

## Public API

```c
xuiPageGetType
xuiPageCreate
xuiPageSetChange
xuiPageSetPageCount
xuiPageGetPageCount
xuiPageSetCurrent
xuiPageGetCurrent
xuiPageSetTotal
xuiPageSetWindowSize
xuiPageGetWindowSize
xuiPageSetText
xuiPageGetText
xuiPageSetFont
xuiPageGetFont
xuiPageSetMetrics
xuiPageGetMetrics
xuiPageSetColors
xuiPageSetFocusColor
xuiPageGetItemCount
xuiPageGetItemInfo
xuiPageGetHoverItem
xuiPageGetActiveItem
xuiPageGetState
xuiPageGetChangeCount
```

Item types:

```c
XUI_PAGE_ITEM_PREV
XUI_PAGE_ITEM_FIRST
XUI_PAGE_ITEM_ELLIPSIS
XUI_PAGE_ITEM_PAGE
XUI_PAGE_ITEM_LAST
XUI_PAGE_ITEM_NEXT
```

## Style Properties

```text
page.background.color
page.border.color
page.text.color
page.hover.color
page.active.color
page.current.color
page.current_text.color
page.disabled_text.color
page.focus.color
page.item.height
page.width.page
page.width.text
page.width.nav
page.width.ellipsis
font.name
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_page_test.bat
examples\xui_page\build.bat
build\xui_page.exe --frames 360
```

The example summary should include `create=1`, `layout=1`, and `input=1`.
