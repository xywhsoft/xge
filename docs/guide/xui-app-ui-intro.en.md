# XUI App UI Intro

This guide shows how to organize an XUI/XSON app page. The goal is a lightweight retained UI for common app screens: header, side rail, scrollable content, status bar, and localized data refresh.

[Guide Index](README.en.md) | [XSON UI](xui-xson-intro.en.md) | [XUI Layout](xui-layout-intro.en.md) | [XUI Style Inheritance](xui-style-inheritance-intro.en.md)

> This guide describes the app/XSON organization model. ScrollView, VirtualList, TreeView, and TableView must follow the ScrollModel / ScrollFrame / ScrollView / VirtualView design; overlay, clip, Z order, and focus restore are handled by the Widget foundation.

## Recommended Structure

A common app page can be split into:

- root: window size and safe area.
- shell: a `dock` container for header, rail, content, and status regions.
- header: a `row` with title, search, and action buttons.
- rail: a `column` with navigation buttons.
- content: `scrollView` or `virtualList`.
- status: a `row` with status text, progress, or sync state.
- overlay: dialogs, menus, tooltips, or debug layers. Usually keep these outside the shell content flow.

Do not model the first version as a complex CSS responsive system. Prefer Dock, Row, Column, Grid, ScrollView, and VirtualList.

## XSON Page Skeleton

```json
{
  "xui": 1,
  "safeArea": [0, 0, 0, 0],
  "tokens": {
    "colors": {
      "shell": "#15191F",
      "panel": "#202631",
      "rail": "#10141A",
      "rule": "#3A4250",
      "accent": "#3277FF",
      "text": "#EEF4FF"
    },
    "spacing": {
      "pad": 10,
      "gap": 8,
      "headerH": 48,
      "railW": 72,
      "statusH": 28,
      "contentW": 480,
      "contentH": 420
    }
  },
  "styles": {
    "shell": { "layout": "dock", "width": "100%", "height": "100%", "padding": "@spacing.pad", "gap": "@spacing.gap", "background": "@colors.shell" },
    "header": { "layout": "row", "dock": "top", "height": "@spacing.headerH", "alignY": "center", "justify": "space-between", "padding": [8, 6], "background": "@colors.panel" },
    "rail": { "layout": "column", "dock": "left", "width": "@spacing.railW", "gap": 6, "padding": 6, "background": "@colors.rail" },
    "content": { "dock": "fill", "width": "grow", "height": "grow", "padding": 6, "contentSize": ["@spacing.contentW", "@spacing.contentH"], "backgroundColor": "@colors.panel", "barColor": "@colors.rule", "thumbColor": "@colors.accent" },
    "status": { "layout": "row", "dock": "bottom", "height": "@spacing.statusH", "alignY": "center", "padding": [8, 4], "background": "@colors.panel" },
    "title": { "textColor": "@colors.text" },
    "action": { "width": 92, "height": 28, "color": "@colors.accent", "textColor": "@colors.text" }
  },
  "tree": {
    "type": "dock",
    "id": "app",
    "style": "shell",
    "children": [
      {
        "type": "row",
        "id": "header",
        "style": "header",
        "children": [
          { "type": "label", "id": "title", "style": "title", "text": "${app.title}" },
          { "type": "button", "id": "refresh", "style": "action", "text": "Refresh", "onClick": "refresh" }
        ]
      },
      {
        "type": "column",
        "id": "rail",
        "style": "rail",
        "children": [
          { "type": "button", "id": "nav-a", "style": "action", "text": "A", "onClick": "navA" },
          { "type": "button", "id": "nav-b", "style": "action", "text": "B", "onClick": "navB" }
        ]
      },
      {
        "type": "scrollView",
        "id": "content",
        "style": "content",
        "children": [
          {
            "type": "grid",
            "id": "cards",
            "gridColumns": 2,
            "rowHeight": 64,
            "gap": 8,
            "children": [
              { "type": "panel", "id": "card-a", "background": "@colors.rail" },
              { "type": "panel", "id": "card-b", "background": "@colors.rail" }
            ]
          }
        ]
      },
      {
        "type": "row",
        "id": "status",
        "style": "status",
        "children": [
          { "type": "label", "id": "status-text", "style": "title", "text": "${app.status}" }
        ]
      }
    ]
  }
}
```

`examples/xui_xson_app_layout_lab` is the buildable validation version of this structure.

## C-Side Loading

```c
typedef struct app_ui_t {
	xge_xui_context_t ui;
	xge_xui_page_t page;
	xge_xui_binder_t binder;
	xge_xui_model_t model;
} app_ui_t;

static void OnRefresh(xge_xui_widget widget, void* user)
{
	app_ui_t* app = (app_ui_t*)user;
	(void)widget;
	xgeXuiModelSetText(&app->model, "app.status", "refreshing");
	xgeXuiPageApplyModel(&app->page, &app->model);
}

static int LoadAppUi(app_ui_t* app)
{
	xgeXuiInit(&app->ui);
	xgeXuiBinderInit(&app->binder);
	xgeXuiModelInit(&app->model);
	xgeXuiBinderSetClick(&app->binder, "refresh", OnRefresh, app);
	xgeXuiModelSetText(&app->model, "app.title", "Dashboard");
	xgeXuiModelSetText(&app->model, "app.status", "ready");
	if ( xgeXuiPageLoad(&app->ui, "ui/app.xson", &app->binder, &app->page) != XGE_OK ) {
		return XGE_ERROR;
	}
	return xgeXuiPageApplyModel(&app->page, &app->model);
}
```

Events bind only to C-side registered names. Missing names fail page load, so configuration mistakes are found during startup.

## Safe Area

App and full-screen UI need to account for notches, gesture areas, and platform status bars. Declare safe area at the XSON top level:

```json
{ "safeArea": [0, 24, 0, 0] }
```

Or set it from platform code:

```c
xgeXuiSetSafeAreaPx(&ui, left, top, right, bottom);
```

XUI represents safe area as root padding. The overlay root remains full-window for dialogs, menus, and debug layers.

## Choosing The Content Area

Use ScrollView when:

- The content count is moderate.
- Children need a full retained tree.
- The content area contains forms, cards, or nested layout.

Use VirtualList when:

- The list is long.
- Item height is fixed.
- Only visible slot widget trees are needed.

Do not put thousands of rows directly into a normal Column. That increases memory use and layout cost.

## Data Refresh

The first model-binding version uses explicit apply:

```c
xgeXuiModelSetText(&model, "app.status", "loaded");
xgeXuiPageApplyModel(&page, &model);
```

It supports only whole-field `${key}` values. It does not interpolate strings or evaluate expressions. Format composed text in C before writing it to the model.

After changing style tokens, explicitly sync the page style cache:

```c
xgeXuiTokenSetColor(&ui, "accent", XGE_COLOR_RGBA(64, 160, 255, 255));
xgeXuiPageSyncStyle(&page);
```

## Frame Loop

App mode still uses the retained XUI update and paint flow:

```c
static int AppFrame(void* user)
{
	app_ui_t* app = (app_ui_t*)user;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiUpdate(&app->ui, xgeGetDelta());
	xgeXuiPaint(&app->ui);
	xgeEnd();
	xgePresent();
	return 0;
}
```

If there is no continuous animation, pair this with app-mode refresh behavior and redraw only after input, data changes, or dirty state changes.

## Debug Boundary

The release `xge` library does not include XUI debug APIs. Use `xgedbg` during app development for snapshot, overlay, or page trace, and guard calls with macros:

```c
#if XGE_DEBUGMODE
xgedbgXuiPageTrace(&page, buffer, sizeof(buffer));
#endif
```

Do not keep debug placeholder functions in release code.

## Common Issues

- Use `dock` for the shell root instead of recalculating header/rail/content rects by hand on window resize.
- Content overflow does not automatically create scrollbars. Choose ScrollView or VirtualList explicitly.
- Put global colors and spacing in tokens instead of hardcoding them on every widget.
- Use inline overrides for local differences; extract repeated differences into named styles.
- Use VirtualList for large lists.
- Keep all debug API calls inside `#if XGE_DEBUGMODE`.

## Examples

- `examples/xui_xson_app_layout_lab`: declarative app shell layout validation.
- `examples/xui_xson_layout_gallery_lab`: more layout geometry combinations.
- `examples/xui_xson_virtual_list_lab`: long-list template and slot reuse.
- `examples/app_mode`: basic app run mode.

## Next Steps

- Read [XSON UI Intro](xui-xson-intro.en.md).
- Read [XUI Rendering Intro](xui-render-intro.en.md).
