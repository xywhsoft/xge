# XSON UI Intro

This guide explains how to describe XUI pages with XSON. An XSON page is loaded into a retained widget tree, making it useful for app UI and in-game UI built directly from layout descriptions without a UI designer.

[Guide Index](README.en.md) | [XUI Layout](xui-layout-intro.en.md) | [XUI Controls](xui-controls-intro.en.md) | [XUI API](../api/xui.en.md)

> In current XSON, `type` maps to Control, Container, Viewport, or Overlay; Control rejects normal `children` by default. `scroll` / `scrollView` has been restored on top of ScrollModel + ScrollFrame, and its `children` attach to the internal content widget. `popup`, `listView`, `treeView`, and `tableView` have also been restored.
> `virtualList`, `propertyGrid`, `menu`, `comboBox`, and `textEdit` are still quarantined for the viewport rebuild and temporarily fail with an unavailable error; `colorPicker` is restored. Common stacking fields use `layer` plus `zIndex`/`z` with `layer > z > treeOrder` ordering, common hit-test fields support `hitTestVisible` and `inputTransparent`, common focus fields support `tabStop`, `tabIndex`, and `imeMode`, and common base paint fields support `borderColor`, `borderWidth`, `focusRingColor`, `focusRingWidth`, `disabledOverlay`, `debugOutlineColor`, and `debugOutlineWidth`.

## Scope

The first XSON UI version covers structured pages, not scripted UI:

- Containers: `panel/absolute/row/column/stack/grid/dock`; normal `children` are allowed.
- Viewports: `scroll/scrollView` has been restored with normal `children`, `listView` has been restored for fixed-height rows, `treeView` has been restored for hierarchical data, and `tableView` has been restored for static tables; the other viewport types are still being rebuilt under the new VirtualView boundary.
- Controls: `label/button/image/input/numericInput/colorPicker/datePicker/checkbox/radio/switch/slider/progress/tabs/toolbar/statusBar/comboBox/accordion/separator`; normal `children` are rejected by default.
- Overlays: `popup/tooltip/menu/msgBox/toast`.
- Styles: `styles`, `@parent`, tokens, and inline overrides.
- Events: `onClick` can bind to a C-side registered name.
- Data: label/input/image support simple `${key}` model binding.
- Debugging: detailed page trace is available only in `xgedbg`.

XSON does not execute scripts, evaluate expressions, or repeatedly parse string fields in the layout/paint hot path.

## Minimal Load Flow

```c
xge_xui_context_t ui;
xge_xui_page_t page;
xge_xui_binder_t binder;

xgeXuiInit(&ui);
xgeXuiBinderInit(&binder);

if ( xgeXuiPageLoad(&ui, "ui/main.xson", &binder, &page) != XGE_OK ) {
	printf("page load failed: %s\n", xgeXuiPageGetError(&page));
}

xgeXuiUpdate(&ui, delta);
xgeXuiPaint(&ui);

xgeXuiPageUnload(&page);
xgeXuiUnit(&ui);
```

Use `xgeXuiPageLoadMemory` for in-memory strings. See `examples/xui_xson_page_lab`.

## Page Structure

A page usually contains `tokens`, `styles`, and `tree`:

```json
{
  "xui": 1,
  "tokens": {
    "colors": { "panel": "#202631", "text": "#EEF4FF", "accent": "#3277FF" },
    "spacing": { "pad": 10, "gap": 8, "headerH": 48 }
  },
  "styles": {
    "screen": { "layout": "dock", "width": "100%", "height": "100%", "padding": "@spacing.pad", "gap": "@spacing.gap" },
    "header": { "dock": "top", "height": "@spacing.headerH", "background": "@colors.panel" },
    "title": { "textColor": "@colors.text" },
    "action": { "width": 92, "height": 28, "color": "@colors.accent", "textColor": "@colors.text" }
  },
  "tree": {
    "type": "dock",
    "id": "app",
    "style": "screen",
    "children": [
      {
        "type": "row",
        "id": "header",
        "style": "header",
        "children": [
          { "type": "label", "id": "title", "style": "title", "text": "Dashboard" },
          { "type": "button", "id": "refresh", "style": "action", "text": "Refresh", "onClick": "refresh" }
        ]
      },
      { "type": "column", "id": "content", "dock": "fill", "width": "grow", "height": "grow" }
    ]
  }
}
```

String `id` values are stored as widget names and indexed by the page:

```c
xge_xui_widget refresh = xgeXuiPageFind(&page, "refresh");
```

## Styles And Override Order

Fields are resolved in this order:

1. Control defaults.
2. Named style.
3. The style `@parent` chain.
4. Widget inline fields.

```json
{
  "styles": {
    "fill": { "width": "100%", "height": "100%" },
    "panel": { "@parent": "fill", "layout": "column", "padding": 12, "gap": 6 },
    "toolbar": { "@parent": "panel", "layout": "row", "height": 44, "justify": "space-between" }
  },
  "tree": {
    "type": "row",
    "id": "toolbar",
    "style": "toolbar",
    "padding": [4, 5, 6, 7]
  }
}
```

The `toolbar` style inherits from `panel` and `fill`, while the inline `padding` overrides the style padding.

## Imports

`imports` reuse tokens, styles, and templates:

```json
{
  "imports": [ "common.xson", "theme/dark.xson" ],
  "styles": {
    "localPanel": { "@parent": "panel", "padding": 16 }
  },
  "tree": { "type": "column", "id": "root", "style": "localPanel" }
}
```

Rules:

- URI values with a scheme are passed to `xgeResourceLoad` unchanged.
- Relative paths resolve from the current XSON file directory.
- Imports merge only `styles/tokens/templates`, never `tree`.
- Later imports override earlier imports; local page declarations override imports.
- Cyclic imports fail page load with an error.

## Event Binding

XSON events refer only to C-side registered names. Missing names fail page load instead of silently dropping events.

```c
static void OnRefresh(xge_xui_widget widget, void* user)
{
	(void)widget;
	(void)user;
}

xge_xui_binder_t binder;
xgeXuiBinderInit(&binder);
xgeXuiBinderSetClick(&binder, "refresh", OnRefresh, app);
xgeXuiPageLoad(&ui, "ui/main.xson", &binder, &page);
```

```json
{ "type": "button", "id": "refresh", "text": "Refresh", "onClick": "refresh" }
```

`script` and `onClickScript` are rejected.

## Data Binding

The first model-binding version recognizes only whole-field `${key}` values. It does not support interpolation or expressions:

```json
{
  "tree": {
    "type": "column",
    "children": [
      { "type": "label", "id": "nameLabel", "text": "${player.name}" },
      { "type": "input", "id": "nameInput", "value": "${player.name}" },
      { "type": "image", "id": "avatar", "src": "${player.avatar}" }
    ]
  }
}
```

```c
xge_xui_model_t model;
xgeXuiModelInit(&model);
xgeXuiModelSetText(&model, "player.name", "Ada");
xgeXuiModelSetText(&model, "player.avatar", "assets/avatar.png");
xgeXuiPageApplyModel(&page, &model);
```

`Hello ${name}` and `${a+b}` are not recognized as bindings.

## VirtualList Templates

`virtualList` can use an item template from top-level `templates`. It creates only visible slots at runtime.

```json
{
  "templates": {
    "row": {
      "type": "row",
      "height": 24,
      "children": [
        { "type": "label", "id": "row-label", "text": "Item" }
      ]
    }
  },
  "tree": {
    "type": "virtualList",
    "id": "inventory",
    "itemCount": 1000,
    "itemHeight": 24,
    "itemTemplate": "row"
  }
}
```

## Errors And Debugging

Read the page error string after load failure:

```c
if ( xgeXuiPageLoad(&ui, "ui/bad.xson", NULL, &page) != XGE_OK ) {
	printf("%s\n", xgeXuiPageGetError(&page));
}
```

Errors include the resource URI and field path, for example `ui/bad.xson: tree.children: expected array`.

Detailed XSON page trace belongs to `xgedbg`; release `xge` does not declare or export it:

```c
#if XGE_DEBUGMODE
xgedbgXuiPageTrace(&page, buffer, sizeof(buffer));
#endif
```

## Examples

- `examples/xui_xson_page_lab`: minimal page loading.
- `examples/xui_xson_style_lab`: tokens, `@parent`, and inline override.
- `examples/xui_xson_app_layout_lab`: Dock shell, ScrollView, and Grid.
- `examples/xui_xson_virtual_list_lab`: VirtualList and itemTemplate.
- `examples/xui_xson_layout_gallery_lab`: XSON migration of the handwritten layout gallery.

## Next Steps

- Read [XUI Layout Intro](xui-layout-intro.en.md).
- Read the style inheritance guide once it is added.
