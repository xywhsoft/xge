# XUI Split Layout Lab

## Goal

Demonstrate the recommended split-resize workflow with layout-owned panes instead of exposing a standalone split layout divider control.

## Coverage

- `xgeXuiSplitLayoutInit/Unit`
- `xgeXuiSplitLayoutSetOrientation`
- `xgeXuiSplitLayoutSetPaneCount`
- `xgeXuiSplitLayoutGetPaneCount`
- `xgeXuiSplitLayoutGetPaneWidget`
- `xgeXuiSplitLayoutSetPaneWeight/GetPaneWeight`
- `xgeXuiSplitLayoutSetPaneMinSize`
- `xgeXuiSplitLayoutGetPaneSize`
- `xgeXuiSplitLayoutSetDividerSize`
- `xgeXuiSplitLayoutSetShadowDrag`
- `xgeXuiSplitLayoutSetColors`

## Scene

The lab renders two cards:

- left: a vertical `SplitLayout` with three panes
- right: a horizontal `SplitLayout` with three panes

Each pane contains regular XUI layout content so pane size changes automatically reflow nested widgets.

## Manual checks

1. Drag the vertical dividers in the left card:
   a shadow line should move first, and pane layout should commit on release.
2. Drag the horizontal dividers in the right card:
   the same shadow-drag behavior should apply.
3. Try to collapse panes past their minimum size:
   movement should clamp before any pane becomes unusably small.
4. Resize the window:
   both split layouts and their nested content should reflow without dead hit regions.

## Auto-check

Run:

```bat
examples\xui_split_layout_lab\build.bat
build\xge_xui_split_layout_lab.exe --frames 5
```

Expected summary shape:

```text
xui-split-layout-lab final-summary frames=5 init=1 vertical=1 horizontal=1 shadow=1 ...
```
