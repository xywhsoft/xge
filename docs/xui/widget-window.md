# XUI Window

Window is a cache-first floating container widget. It preserves the XUI1 idea that a window is an overlay layer with chrome, activation, z-order, drag/resize, and a client area for application content.

## Goals

- keep Window as a floating overlay, not a normal layout child
- preserve XUI1 chrome behavior: title bar, optional icon, collapse/maximize/close buttons, drag, resize, active state, and top-most grouping
- keep application content inside the client widget
- support frameless and drag-anywhere tool surfaces
- expose internal widgets and geometry for tests and advanced integrations
- keep XSON deferred

## Structure

`xuiWindowCreate` returns the Window root widget. It is automatically attached to the context overlay root in `XUI_LAYER_FLOATING`.

```text
Window root widget
  client widget
    user children
  collapse button widget
  maximize button widget
  close button widget
```

Use `xuiWindowAddChild` or `xuiWindowGetClientWidget` for application content. Raw `xuiWidgetAddChild(window, child)` bypasses the client-area convention and can interfere with chrome hit testing.

## Layering

Windows use two z groups:

```c
XUI_WINDOW_Z_NORMAL
XUI_WINDOW_Z_TOPMOST
```

Normal windows can be raised among normal windows. Top-most windows remain in the top-most z group; they are not modal and do not block input outside themselves. `xuiWindowGetActive` returns the active open window from the overlay/root scan.

## Interaction

- pointer down activates the window and raises it inside its z group
- dragging the title bar moves the window when movable
- frameless windows can opt into client drag with `xuiWindowSetDragAnywhere`
- frameless drag-anywhere windows omit the top resize edge by default so the top strip drags instead of resizing; explicit `iResizeEdges` or `xuiWindowSetResizeEdges` overrides this
- resizing is edge-based and constrained by min size and parent viewport
- collapse hides the client widget and stores the previous expanded height
- maximize stores the restore rect, uncollapses the window, and fills the overlay parent
- close hides the window and calls `xui_window_close_proc`
- capture loss and pointer-up clear active drag/resize/button state

## Public API

```c
xuiWindowGetType
xuiWindowCreate
xuiWindowSetClose
xuiWindowGetClientWidget
xuiWindowGetCollapseButtonWidget
xuiWindowGetMaximizeButtonWidget
xuiWindowGetCloseButtonWidget
xuiWindowAddChild
xuiWindowInsertBefore
xuiWindowSetTitle
xuiWindowGetTitle
xuiWindowSetFont
xuiWindowGetFont
xuiWindowSetIcon
xuiWindowGetIconSurface
xuiWindowGetIconSource
xuiWindowSetOpen
xuiWindowIsOpen
xuiWindowSetShowTitleBar
xuiWindowGetShowTitleBar
xuiWindowSetMovable
xuiWindowIsMovable
xuiWindowSetDragAnywhere
xuiWindowIsDragAnywhere
xuiWindowSetResizable
xuiWindowIsResizable
xuiWindowSetResizeEdges
xuiWindowGetResizeEdges
xuiWindowSetShowCollapse
xuiWindowGetShowCollapse
xuiWindowSetShowMaximize
xuiWindowGetShowMaximize
xuiWindowSetShowClose
xuiWindowGetShowClose
xuiWindowSetCollapsed
xuiWindowIsCollapsed
xuiWindowSetMaximized
xuiWindowIsMaximized
xuiWindowBringToFront
xuiWindowSetTopMost
xuiWindowIsTopMost
xuiWindowIsActive
xuiWindowGetActive
xuiWindowSetChrome
xuiWindowGetChrome
xuiWindowSetMinSize
xuiWindowGetMinSize
xuiWindowSetColors
xuiWindowGetColors
xuiWindowGetTitleBarRect
xuiWindowGetClientRect
xuiWindowGetCollapseButtonRect
xuiWindowGetMaximizeButtonRect
xuiWindowGetCloseButtonRect
xuiWindowGetResizeEdgesAt
xuiWindowGetHoverPart
xuiWindowGetActivePart
xuiWindowGetState
xuiWindowGetChangeCount
xuiWindowGetCloseCount
```

## Constants

```c
XUI_WINDOW_TITLE_CAPACITY
XUI_WINDOW_EDGE_LEFT
XUI_WINDOW_EDGE_TOP
XUI_WINDOW_EDGE_RIGHT
XUI_WINDOW_EDGE_BOTTOM
XUI_WINDOW_EDGE_ALL
XUI_WINDOW_Z_NORMAL
XUI_WINDOW_Z_TOPMOST
XUI_WINDOW_PART_NONE
XUI_WINDOW_PART_BODY
XUI_WINDOW_PART_TITLE_BAR
XUI_WINDOW_PART_RESIZE
XUI_WINDOW_PART_COLLAPSE
XUI_WINDOW_PART_MAXIMIZE
XUI_WINDOW_PART_CLOSE
XUI_WINDOW_STATE_OPEN
XUI_WINDOW_STATE_COLLAPSED
XUI_WINDOW_STATE_MAXIMIZED
XUI_WINDOW_STATE_ACTIVE
XUI_WINDOW_STATE_TOPMOST
XUI_WINDOW_STATE_DRAGGING
XUI_WINDOW_STATE_RESIZING
```

## Style Properties

```text
font.name
window.background.color
window.client.color
window.titlebar.color
window.titlebar.inactive_color
window.title.text_color
window.title.inactive_text_color
window.border.color
window.border.active_color
window.button.color
window.button.hover_color
window.button.active_color
window.close.hover_color
window.close.active_color
window.titlebar.height
window.border.width
window.resize_grip
window.button.size
window.icon.size
window.radius
window.min_width
window.min_height
```

Color properties affect cache rendering. Titlebar height, border width, resize grip, button size, icon size, and minimum size participate in layout and hit testing.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_window_test.bat
examples\xui_window\build.bat
build\xui_window.exe --frames 3
```

The example summary should include `create=1`, `layer=1`, `drag=1`, `resize=1`, `collapse=1`, `maximize=1`, and `close=1`.
