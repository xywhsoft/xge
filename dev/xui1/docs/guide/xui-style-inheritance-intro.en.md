# XUI Style Inheritance Intro

This guide explains style, tokens, and `@parent` inheritance in XSON UI. XUI style inheritance uses XValue parent-table chains to share data, avoid field copying, and keep the runtime hot path on a lightweight style cache.

[Guide Index](README.en.md) | [XSON UI](xui-xson-intro.en.md) | [XUI Layout](xui-layout-intro.en.md) | [XUI API](../api/xui.en.md)

## Core Rules

Style resolution runs during page load, refresh, or sync. Layout and paint read the resolved `xge_xui_style_t`; they do not repeatedly inspect XSON string fields.

Field priority:

1. Widget/control defaults.
2. Named style.
3. The style `@parent` chain.
4. Widget inline fields.

The practical result is that a child style writes only its differences, missing fields are looked up from the parent chain, and widget inline fields can always override the style.

## `@parent`

The `@parent` field references another style from the page merged styles:

```json
{
  "styles": {
    "fill": {
      "width": "100%",
      "height": "100%"
    },
    "panel": {
      "@parent": "fill",
      "layout": "column",
      "padding": 12,
      "gap": 6,
      "background": "#101820"
    },
    "toolbar": {
      "@parent": "panel",
      "layout": "row",
      "height": 44,
      "justify": "space-between"
    }
  }
}
```

`toolbar` does not repeat `width/height/padding/gap/background`. If a field is missing from `toolbar`, lookup continues to `panel`, then to `fill`.

Cyclic inheritance fails page load:

```json
{
  "styles": {
    "a": { "@parent": "b" },
    "b": { "@parent": "a" }
  }
}
```

The error includes `style parent cycle`.

## Inline Overrides

A widget can reference a style and override individual fields:

```json
{
  "tree": {
    "type": "row",
    "id": "toolbar",
    "style": "toolbar",
    "padding": [4, 5, 6, 7]
  }
}
```

The widget still inherits layout, height, justify, gap, and other fields, but uses the inline `padding`.

Use inline overrides for small per-instance differences. Extract repeated differences into named styles when the page starts to grow.

## Tokens

Tokens keep colors, spacing, fonts, and textures out of individual style fields:

```json
{
  "tokens": {
    "colors": {
      "panel": "#101820",
      "accent": "#2A7FFF",
      "text": "#F0F4FF"
    },
    "spacing": {
      "pad": 12,
      "gap": 6,
      "toolbarH": 44
    }
  },
  "styles": {
    "panel": {
      "padding": "@spacing.pad",
      "gap": "@spacing.gap",
      "background": "@colors.panel"
    }
  }
}
```

Supported namespaces:

- `tokens.colors`
- `tokens.spacing`
- `tokens.fonts`
- `tokens.textures`

Font and texture tokens refer to externally owned objects. XSON does not create or release font objects; borrowed texture tokens used by images are not released by XSON either.

C code can register context tokens as fallback values:

```c
xgeXuiTokenSetColor(&ui, "accent", XGE_COLOR_RGBA(42, 127, 255, 255));
xgeXuiTokenSetSpacing(&ui, "pad", 12.0f);
xgeXuiTokenSetFont(&ui, "body", &font);
```

XSON/import tokens win first; context tokens are fallback values.

## Imports And Merge Order

Imports merge only `styles/tokens/templates`, never `tree`:

```json
{
  "imports": [ "theme/base.xson", "theme/dark.xson" ],
  "styles": {
    "localPanel": { "@parent": "panel", "padding": 16 }
  }
}
```

Rules:

- Imports are processed in array order.
- Later imports override earlier imports.
- Local page declarations override imports.
- Relative paths resolve from the current XSON file directory.
- URI values with a scheme are passed to `xgeResourceLoad` unchanged.
- Import cycles fail.

This lets a project layer base tokens, shared control styles, and page-local styles.

## Lifetime Constraint

XValue dictionaries and lists store parent-table references; they do not own parent lifetime. If a parent table is released first, a later child lookup can crash.

The first XUI version uses this rule:

- Imported styles, merged styles, and current page styles are owned by `xge_xui_page_t`.
- Do not release an XValue table while it can still be queried as a parent.
- `xgeXuiPageUnload` releases the style/token/template data owned by the page.

Do not set a temporary XValue table as the parent of a long-lived page style.

## Style Cache And Refresh

When a page loads, XSON styles are resolved into each widget's `xge_xui_style_t`. This style cache contains high-frequency layout and visual fields such as layout, size, gap, padding, background, radius, grid, and dock.

After a context token changes, loaded pages do not re-read XSON in the paint/layout hot path. Sync explicitly:

```c
xgeXuiTokenSetColor(&ui, "accent", XGE_COLOR_RGBA(80, 160, 255, 255));
xgeXuiPageSyncStyle(&page);
```

Force a page style recompute with:

```c
xgeXuiPageRefreshStyle(&page);
```

Use the style version for external cache decisions:

```c
uint32_t version = xgeXuiPageStyleVersion(&page);
```

## Current Limits

- `@parent` inherits named styles only; theme style and engine default style are not XValue parents in the first version.
- CSS selectors, class cascade, pseudo classes, and media queries are not supported.
- Expressions are not supported. `@spacing.pad * 2` is not evaluated.
- The runtime hot path does not dynamically parse XSON fields. Sync or refresh a page after token changes.
- Styles are not automatically flattened into full copies. Parent chains share fields and require unified lifetime management.

## Examples

- `examples/xui_xson_style_lab`: tokens, style `@parent`, inline override, and button styling.
- `examples/xui_xson_app_layout_lab`: page-level tokens/styles organization.
- `test/test_main.c`: regression coverage for style inheritance, inline override, token fallback, style cache sync, and parent cycles.

## Next Steps

- Read [XSON UI Intro](xui-xson-intro.en.md).
- Read [XUI Layout Intro](xui-layout-intro.en.md).
