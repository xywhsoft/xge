# XUI Layout Intro

This guide describes the current XUI layout system. XUI is a retained-mode GUI layer, not an HTML/CSS clone. It favors predictable layout, bounded memory use, and stable runtime cost for app UI, in-game UI, and tool panels.

[Guide Index](README.en.md) | [XUI API](../api/xui.en.md) | [XUI Controls](xui-controls-intro.en.md) | [XUI Rendering](xui-render-intro.en.md)

## Layout Model

XUI uses a widget tree. Each widget has a `rect`, a padding-adjusted `contentRect`, and a lightweight `xge_xui_style_t`. Layout runs only when dirty. Reuse widgets and update state instead of rebuilding the tree every frame.

```c
xge_xui_context_t ui;
xge_xui_widget root;

xgeXuiInit(&ui);
root = xgeXuiRoot(&ui);
xgeXuiWidgetSetLayout(root, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetPaddingPx(root, 16.0f, 16.0f, 16.0f, 16.0f);
xgeXuiUpdate(&ui, delta);
xgeXuiPaint(&ui);
xgeXuiUnit(&ui);
```

The root widget follows the window size. App and full-screen UI can inset the root content area with safe area padding:

```c
xgeXuiSetSafeAreaPx(&ui, left, top, right, bottom);
```

## Size Types

XUI sizes are explicit typed values. The layout engine does not run a CSS-like constraint solver.

```c
xgeXuiSizePx(120.0f);       /* fixed pixels */
xgeXuiSizeDip(48.0f);       /* device-independent size */
xgeXuiSizePercent(100.0f);  /* percent of parent content size */
xgeXuiSizeGrow(1.0f);       /* share remaining main-axis space */
xgeXuiSizeContent();        /* measured by content or control */
```

`minWidth/minHeight/maxWidth/maxHeight` clamp the assigned size. Row and Column grow distribution handles min/max redistribution. If there is not enough space, overflow is allowed; choose clip, ScrollView, or VirtualList explicitly.

## Layout Types

`Absolute` is useful for dialogs, draggable panels, HUD, and exact positioning. Children can use fixed rects or anchors.

```c
xgeXuiWidgetSetLayout(panel, XGE_XUI_LAYOUT_ABSOLUTE);
xgeXuiWidgetSetAnchorPx(dialog, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP, 210.0f, 140.0f, 0.0f, 0.0f);
```

`Row` places children left to right. `gap` controls spacing, `justify` distributes remaining space, and `alignY` controls cross-axis alignment.

```c
xgeXuiWidgetSetLayout(toolbar, XGE_XUI_LAYOUT_ROW);
xgeXuiWidgetSetGap(toolbar, 8.0f);
xgeXuiWidgetSetJustify(toolbar, XGE_XUI_JUSTIFY_SPACE_BETWEEN);
```

`Column` places children top to bottom. It is the common choice for settings pages, menus, forms, and panel bodies.

`Stack` places children in the same slot. It is useful for backgrounds, masks, badges, and layered state. Children can use `alignX/alignY`.

`Grid` uses a fixed column count and row height, keeping placement O(N). The first version supports `columnSpan`, but not row span, named areas, or auto backfill.

```c
xgeXuiWidgetSetLayout(grid, XGE_XUI_LAYOUT_GRID);
xgeXuiWidgetSetGrid(grid, 4, 40.0f, 8.0f, 8.0f);
xgeXuiWidgetSetGridColumnSpan(card, 2);
```

`Dock` is intended for app shells: header, footer, left/right rails, and fill content. Children declare a dock role.

```c
xgeXuiWidgetSetLayout(shell, XGE_XUI_LAYOUT_DOCK);
xgeXuiWidgetSetDock(header, XGE_XUI_DOCK_TOP);
xgeXuiWidgetSetDock(content, XGE_XUI_DOCK_FILL);
```

## Scrolling And Long Lists

Overflow does not automatically become scrollable. Use ScrollView or VirtualList explicitly.

ScrollView is for medium-sized content trees. It applies a scroll offset to its subtree and hit-tests within the content rect:

```c
xgeXuiScrollViewInit(&scroll, widget);
xgeXuiScrollViewSetContentSize(&scroll, 800.0f, 1200.0f);
xgeXuiScrollViewSetOffset(&scroll, 0.0f, 160.0f);
```

VirtualList is for large fixed-height lists. It reuses visible slots instead of creating widgets for all items:

```c
xgeXuiVirtualListInit(&list, widget);
xgeXuiVirtualListSetItemCount(&list, 10000);
xgeXuiVirtualListSetItemHeight(&list, 28.0f);
```

## XSON Declarative Layout

XUI pages can be described with XSON without a UI designer. XSON creates a retained widget tree. Styles are resolved into a lightweight style cache during load/refresh/sync, so the hot path does not repeatedly scan string fields.

```json
{
  "xui": 1,
  "tokens": {
    "colors": { "panel": "#202631", "accent": "#3277FF" },
    "spacing": { "pad": 10, "gap": 8, "headerH": 48 }
  },
  "styles": {
    "shell": { "layout": "dock", "width": "100%", "height": "100%", "padding": "@spacing.pad", "gap": "@spacing.gap" },
    "header": { "dock": "top", "height": "@spacing.headerH", "background": "@colors.panel" },
    "content": { "dock": "fill", "width": "grow", "height": "grow" }
  },
  "tree": {
    "type": "dock",
    "id": "app",
    "style": "shell",
    "children": [
      { "type": "row", "id": "header", "style": "header" },
      { "type": "column", "id": "content", "style": "content" }
    ]
  }
}
```

Styles support `@parent` inheritance, token references, and widget inline overrides. XValue parent tables share data but do not own parent lifetime. `xge_xui_page_t` owns imported styles, merged styles, and page styles, then releases them in `xgeXuiPageUnload`.

## Dirty And Batch Updates

Layout, style, and paint all use dirty flags. Use layout batching when changing many widgets:

```c
xgeXuiLayoutBatchBegin(&ui);
/* add/remove/update many widgets */
xgeXuiLayoutBatchEnd(&ui);
```

The batch suppresses intermediate refresh work. Ending the batch marks the affected tree for layout.

## Debugging

The release `xge` library does not expose debug APIs. Debug features live in `xgedbg` and are isolated by the `XGE_DEBUGMODE` compile macro. The debug build can emit layout snapshots, overlays, and page traces with widget tree, dirty flags, rect/content, margin/padding, layout type, and XSON load details.

## Common Pitfalls

- Do not rebuild the widget tree every frame to simulate IMGUI.
- Do not expect Row or Column overflow to create scrollbars automatically. Choose clip, ScrollView, or VirtualList.
- Do not build very large lists from thousands of normal child widgets. Use VirtualList.
- Do not call xgedbg APIs from release code. Guard debug calls with `#if XGE_DEBUGMODE`.
- `content` size depends on a measure callback or control-owned measurement. Custom controls should set `xgeXuiWidgetSetMeasure`.

## Examples

- `examples/xui_layout_gallery`: handwritten C API layout gallery.
- `examples/xui_xson_layout_gallery_lab`: XSON migration of the layout gallery.
- `examples/xui_xson_app_layout_lab`: app shell with Dock, ScrollView, and Grid.
- `examples/xui_xson_virtual_list_lab`: XSON VirtualList and itemTemplate.
- `examples/xui_xson_style_lab`: tokens, style `@parent`, and inline override.

## Next Steps

- Read [XUI Controls Intro](xui-controls-intro.en.md).
- Read [XUI Rendering Intro](xui-render-intro.en.md) for app-mode refresh behavior.
