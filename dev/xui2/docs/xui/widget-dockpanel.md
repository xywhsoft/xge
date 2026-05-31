# XUI DockPanel

DockPanel is the XUI2 docking workbench control. It keeps the XUI1 DockPanel design model while replacing the old XUI1 widget infrastructure with a typed XUI2 widget, fixed internal arrays, cache rendering, and normal XUI child widgets for dock window content.

This XUI2 migration is intentionally conservative. It implements the stable core needed by IDE-style tools: dock windows, pane tabs, five docking regions, split nodes, splitter drag, docked tab/caption drag-out, floating-window drag-to-dock preview/commit, hidden/floating/auto-hide states, pane option menus, tab overflow menus, callbacks, hit testing, XSON persistence, examples, and tests. Native OS floating windows remain outside this slice.

## Structure

```text
DockPanel root widget
  DockRegion[document, left, right, top, bottom]
    DockNode split tree
      DockPane
        DockWindow tabs
          host widget
            user client widget
```

The split tree is explicit and binary. A split node stores orientation and ratio. A pane node stores one tab group. A dock window owns a host widget; the user client widget is reparented under that host.

Floating windows remain inside the DockPanel XUI surface. They do not become native child windows.

## Visual Style

The default pane chrome follows the XUI1 VS2005-style DockPanel look:

- docked panes use one flat tab/title strip at the top, not a separate blue caption bar
- the active tab visually merges into the client area and has a 2px orange top accent
- inactive tabs are rectangular, overlap by 1px, and use the normal panel border
- pane option, close, auto-hide, dock, overflow, and drag indicators use the built-in atlas assets when available
- auto-hide strips use flat fills, border feedback, and the DockPanelSuite dock icon

## Regions And Sides

Regions:

```c
XUI_DOCK_PANEL_REGION_DOCUMENT
XUI_DOCK_PANEL_REGION_LEFT
XUI_DOCK_PANEL_REGION_RIGHT
XUI_DOCK_PANEL_REGION_TOP
XUI_DOCK_PANEL_REGION_BOTTOM
```

Dock sides:

```c
XUI_DOCK_PANEL_SIDE_FILL
XUI_DOCK_PANEL_SIDE_LEFT
XUI_DOCK_PANEL_SIDE_RIGHT
XUI_DOCK_PANEL_SIDE_TOP
XUI_DOCK_PANEL_SIDE_BOTTOM
```

`FILL` adds the window as a tab to an existing pane or creates a pane if the region is empty. Split sides create a new pane and split the region root. `xuiDockPanelDockWindowToPaneSide` splits beside a target pane, which matches the XUI1/DockPanelSuite drop behavior more closely than only splitting the top-level region.

## Window States

```c
XUI_DOCK_PANEL_WINDOW_HIDDEN
XUI_DOCK_PANEL_WINDOW_DOCKED
XUI_DOCK_PANEL_WINDOW_FLOATING
XUI_DOCK_PANEL_WINDOW_AUTO_HIDE
```

Hidden, floating, and auto-hide windows keep their title, client, last dock target, last tab index, auto-hide strip region, and last docked pane rectangle. When a hidden or floating window is docked back into a pane, the previous tab index is used as the preferred insertion point so common close/restore flows keep their tab order. The strip region is derived from the last region, or from the last split side when the window came from the document region. Auto-hide is only enabled when that mapping resolves to a left, right, top, or bottom strip; document-center tabs keep the pin affordance disabled. Auto-hide windows expand as an overlay pane without leaving auto-hide state, and the overlay uses the remembered dock size with safe min/max clamps instead of a fixed default width. `xuiDockPanelDockAutoHideWindow` restores an auto-hidden window to its last dock target instead of docking to the strip itself.

`xuiDockPanelSetWindowFlags` follows the XUI1 meaning of `closable` and `dockable`. A non-closable window stays visible but has close commands disabled. A non-dockable window can still be activated in its current pane, but Float, Auto hide, drag-out docking, public dock APIs, and drop-target discovery are disabled for that window.
Inactive non-dockable tabs are drawn with muted text and border feedback so the current layout state is visible without implying that the tab can be dragged out.

## Creating A DockPanel

```c
xui_dock_panel_desc_t desc;
xui_widget dock;
int doc;
int preview;
int docPane;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pFont = font;
xuiDockPanelCreate(ctx, &dock, &desc);

xuiDockPanelAddWindow(dock, "Document.c", documentClient, &doc);
xuiDockPanelAddWindow(dock, "Preview.h", previewClient, &preview);

xuiDockPanelDockWindow(dock, doc,
    XUI_DOCK_PANEL_REGION_DOCUMENT,
    XUI_DOCK_PANEL_SIDE_FILL,
    0.0f,
    &docPane);

xuiDockPanelDockWindowToPane(dock, preview, docPane);
```

Split panes:

```c
xuiDockPanelDockWindow(dock, toolbox,
    XUI_DOCK_PANEL_REGION_DOCUMENT,
    XUI_DOCK_PANEL_SIDE_LEFT,
    0.22f,
    NULL);

xuiDockPanelDockWindowToPaneSide(dock, output,
    docPane,
    XUI_DOCK_PANEL_SIDE_BOTTOM,
    0.25f,
    NULL);
```

Floating:

```c
xuiDockPanelFloatWindow(dock, properties, (xui_rect_t){420, 80, 260, 180});
```

Auto-hide:

```c
xuiDockPanelAutoHideWindow(dock, toolbox);
xuiDockPanelDockAutoHideWindow(dock, toolbox);
```

## Persistence

DockPanel can save and restore its layout as an XSON-backed `xvalue` tree. The state keeps the five regions, split tree, pane tab order and active tab, window state, hidden/floating/auto-hide metadata, last tab indices, auto-hide strip regions, floating rectangles, last dock rectangles, and window flags. Registered window ids are resolved against the current DockPanel on load, so host tools should add the same windows before loading a saved layout.

```c
xvalue state;

if (xuiDockPanelSaveState(dock, &state) == XUI_OK) {
    xuiDockPanelLoadState(dock, state);
    xuiDockPanelStateFree(state);
}

xuiDockPanelSaveXSONFile(dock, "layout.xson");
xuiDockPanelLoadXSONFile(dock, "layout.xson");
```

The state format is versioned (`version = 1`) and is intentionally close to the mature XUI1 DockPanel format: `regions`, `windows`, and `floating` are separate sections, while each region stores a binary node tree with `split` and `pane` nodes.

## Interaction

Implemented interaction:

- clicking pane tabs activates a dock window
- dragging a pane tab within its own tab strip reorders that pane's tabs
- middle-clicking a pane tab closes that dock window when it is closable
- overflowing tab strips show a drop-down menu for all tabs
- pane option buttons show a menu with tab activation, Float, Auto hide, Close, Close Others, and Close All
- pane close button hides the active dock window
- pane pin button moves the active dock window into auto-hide state when the active window can map to an auto-hide strip
- clicking an auto-hide strip expands that window as an overlay pane sized from the last docked pane rectangle
- the auto-hide overlay pin restores the window to the dock tree
- the auto-hide overlay close button hides the window
- dragging splitters updates split ratios
- dragging outer region splitters resizes the left, right, top, or bottom dock region
- dragging a docked pane tab or pane caption past the drag threshold turns that dock window into an internal floating window
- releasing that drag outside the DockPanel drop target area keeps the window floating inside the DockPanel surface
- releasing that same drag over a pane or region docks the window into the target, so docked tabs can move directly between layouts
- dragging a floating title bar moves the floating window inside the DockPanel bounds
- dragging a floating window edge or corner resizes the floating window inside the DockPanel bounds
- clicking or floating a dock window brings that internal floating window to the front
- dragging a floating title bar over a DockPanel target shows a drop preview
- holding Ctrl while dragging suppresses the docking preview and keeps the window floating on release
- releasing over a pane center docks as a tab; releasing over a pane edge creates a split beside that pane
- releasing over the DockPanel outer edge docks into the corresponding fixed region

The content client widget keeps its own event handling inside the pane client rect.

## Callbacks

```c
xuiDockPanelSetWindowStateChanged
xuiDockPanelSetActiveChanged
xuiDockPanelSetWindowClose
```

The close callback is a notification. The default close behavior hides the window.

The active callback fires for explicit tab activation and for implicit pane fallback. Hiding, floating, auto-hiding, docking, or reordering a window can therefore notify when the pane active window changes.

## Hit Testing

`xuiDockPanelHitTest` takes local DockPanel coordinates and fills `xui_dock_hit_t`.

Hit types:

```c
XUI_DOCK_PANEL_HIT_PANE_CAPTION
XUI_DOCK_PANEL_HIT_PANE_TAB
XUI_DOCK_PANEL_HIT_PANE_CLOSE
XUI_DOCK_PANEL_HIT_PANE_PIN
XUI_DOCK_PANEL_HIT_PANE_OPTION
XUI_DOCK_PANEL_HIT_PANE_OVERFLOW
XUI_DOCK_PANEL_HIT_SPLITTER
XUI_DOCK_PANEL_HIT_AUTO_HIDE
XUI_DOCK_PANEL_HIT_FLOAT_TITLE
XUI_DOCK_PANEL_HIT_FLOAT_CLOSE
XUI_DOCK_PANEL_HIT_AUTO_HIDE_PANEL
XUI_DOCK_PANEL_HIT_AUTO_HIDE_PIN
XUI_DOCK_PANEL_HIT_AUTO_HIDE_CLOSE
```

Floating title and close hits are handled by the internal host widget.

## Public API

Core APIs:

```c
xuiDockPanelGetType
xuiDockPanelCreate
xuiDockPanelClear
xuiDockPanelAddWindow
xuiDockPanelSetWindowClient
xuiDockPanelSetWindowFlags
xuiDockPanelDockWindow
xuiDockPanelDockWindowToPane
xuiDockPanelDockWindowToPaneSide
xuiDockPanelFloatWindow
xuiDockPanelHideWindow
xuiDockPanelAutoHideWindow
xuiDockPanelExpandAutoHideWindow
xuiDockPanelCollapseAutoHide
xuiDockPanelGetAutoHideExpandedWindow
xuiDockPanelGetAutoHideExpandRect
xuiDockPanelDockAutoHideWindow
xuiDockPanelSetPaneActiveWindow
xuiDockPanelGetWindowInfo
xuiDockPanelGetPaneInfo
xuiDockPanelSetRegionSize
xuiDockPanelSetMetrics
xuiDockPanelSetColors
xuiDockPanelHitTest
xuiDockPanelFindDropTarget
xuiDockPanelGetDragPreview
xuiDockPanelGetOptionMenu
xuiDockPanelGetOverflowMenu
xuiDockPanelOpenPaneMenu
xuiDockPanelOpenOverflowMenu
xuiDockPanelSaveState
xuiDockPanelLoadState
xuiDockPanelStateFree
xuiDockPanelStateGetCounts
xuiDockPanelSaveXSONFile
xuiDockPanelLoadXSONFile
```

The option and overflow menus reuse the standard XUI Menu/Popup stack. `xuiDockPanelOpenPaneMenu` and `xuiDockPanelOpenOverflowMenu` are public so tests or host tools can open them programmatically; pointer interaction opens them from the pane option button and overflow button.

Menu item values use these command IDs:

```c
XUI_DOCK_PANEL_MENU_FLOAT
XUI_DOCK_PANEL_MENU_AUTO_HIDE
XUI_DOCK_PANEL_MENU_CLOSE
XUI_DOCK_PANEL_MENU_CLOSE_OTHERS
XUI_DOCK_PANEL_MENU_CLOSE_ALL
XUI_DOCK_PANEL_MENU_WINDOW_BASE + windowIndex
```

Diagnostic counters:

```c
xuiDockPanelGetChangeCount
xuiDockPanelGetLayoutChangeCount
xuiDockPanelGetWindowChangeCount
```

## Current Limits

- no native OS floating windows

The drag preview keeps the translucent target rectangle and also draws the built-in DockPanelSuite pane/panel indicators when the atlas is available. If the atlas cannot be loaded, the target rectangle remains sufficient for interaction.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_dock_panel_test.bat
examples\xui_dockpanel\build.bat
build\xui_dockpanel.exe --frames 3
```
