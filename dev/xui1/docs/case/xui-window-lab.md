# XUI Window Lab

## Goal

Demonstrate the first formal `xgeXuiWindow*` control with two usage styles:

- a chrome window with title bar, icon, collapse/maximize/close buttons
- a frameless window that can still be dragged and resized

## Coverage

- `xgeXuiWindowInit/Unit`
- `xgeXuiWindowGetClientWidget`
- `xgeXuiWindowSetTitle`
- `xgeXuiWindowSetIcon`
- `xgeXuiWindowSetOpen/IsOpen`
- `xgeXuiWindowSetShowTitleBar`
- `xgeXuiWindowSetMovable`
- `xgeXuiWindowSetDragAnywhere`
- `xgeXuiWindowSetResizable`
- `xgeXuiWindowSetResizeEdges`
- `xgeXuiWindowSetShowCollapse`
- `xgeXuiWindowSetShowMaximize`
- `xgeXuiWindowSetShowClose`
- `xgeXuiWindowSetCollapsed/IsCollapsed`
- `xgeXuiWindowSetMaximized/IsMaximized`
- `xgeXuiWindowSetChrome`
- `xgeXuiWindowSetColors`
- `xgeXuiWindowSetClose`
- `xgeXuiWindowEvent`
- `xgeXuiWindowEventProc`
- `xgeXuiWindowPaintProc`

## Scene

The lab shows two floating windows on the root XUI surface:

- `XUI Window`: title bar, icon, caption buttons, title-bar drag, and four-edge resize
- frameless tool window: no title bar, drag-anywhere enabled, and edge resize still available

Both windows place normal XUI content inside the internal client widget returned by `xgeXuiWindowGetClientWidget`.

## Manual checks

1. Drag the main window from the title bar:
   only the chrome area should move the window.
2. Click the collapse, maximize, and close buttons:
   layout and button ordering should stay stable.
3. Resize the main window from all four edges:
   the client content should reflow inside the frame.
4. Drag the frameless tool window from its background:
   it should move even without a title bar.
5. Resize the frameless tool window from the edges:
   edge hit regions should still work when the title bar is hidden.

## Auto-check

Run:

```bat
examples\xui_window_lab\build.bat
build\xge_xui_window_lab.exe --frames 5
```

Expected summary shape:

```text
xui-window-lab final-summary frames=5 init=1 main=1 tool=1 ...
```
