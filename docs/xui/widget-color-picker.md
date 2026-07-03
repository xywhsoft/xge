# XUI ColorPicker

ColorPicker is a cache-first color selection widget. It keeps the XUI1 split between a compact owner control and a popup editor, while using the XUI2 Popup layer for placement, outside close, Escape close, and focus restore.

## Goals

- preserve XUI1 RGB/RGBA color editing, `#RRGGBB` / `#RRGGBBAA` hex values, old/new swatches, and a 16-entry palette
- keep color changes immediate: palette clicks, SV/hue/alpha/channel edits, and old-color restore notify the change callback as soon as the value changes
- reuse Popup instead of introducing a separate overlay mechanism
- expose owner and popup panel geometry for tests and advanced integration
- keep the owner and popup panel cache-first and styleable
- keep XSON deferred

## Structure

`xuiColorPickerCreate` returns the owner widget. Internally, the widget creates a Popup and a panel widget used as the popup content.

```text
ColorPicker owner widget
  internal Popup
    content panel
      SV area
      hue bar
      old/new swatches
      RGB/A fields and sliders
      hex field
      palette row
```

Use `xuiColorPickerGetPopupWidget` and `xuiColorPickerGetPanelWidget` only for tests, diagnostics, or advanced integration. Normal application code should use the ColorPicker open/close and value APIs.

## Value Model

The stored color uses the existing XUI `XUI_COLOR_RGBA(r, g, b, a)` layout. When alpha is disabled, the alpha channel is normalized to `255`, and the hex text is formatted as `#RRGGBB`. When alpha is enabled, the alpha channel is preserved and the hex text is formatted as `#RRGGBBAA`.

Programmatic setters update the value without notifying the change callback:

```c
xuiColorPickerSetColor(picker, XUI_COLOR_RGBA(46, 124, 214, 255));
xuiColorPickerSetRGBA(picker, 46, 124, 214, 180);
xuiColorPickerSetHex(picker, "#2E7CD6");
```

User edits notify `xui_color_picker_change_proc` only when the effective color changes.

## Popup Behavior

Open paths:

- left mouse down on the owner toggles the popup
- Enter and Space toggle from keyboard focus

Close paths:

- Escape closes through Popup
- outside press closes through Popup
- disabling or hiding the owner closes the popup

Placement modes:

- `XUI_COLOR_PICKER_POPUP_AUTO`: bottom-left first; Popup may flip if needed
- `XUI_COLOR_PICKER_POPUP_BOTTOM`: bottom-left first
- `XUI_COLOR_PICKER_POPUP_TOP`: top-left first

The popup size defaults to `432x300`. `xuiColorPickerSetPopupSize` can override it. The panel rect APIs are panel-local; owner rect APIs are owner-local.

## Palette

The palette capacity is `XUI_COLOR_PICKER_PALETTE_CAPACITY` entries. `xuiColorPickerSetPalette` copies colors into the widget, so the caller does not need to keep the input array alive after the call.

Palette clicks apply the clicked color immediately. In RGB mode, palette matching ignores alpha by comparing opaque RGB values.

## Public API

```c
xuiColorPickerGetType
xuiColorPickerCreate
xuiColorPickerSetChange
xuiColorPickerSetColor
xuiColorPickerGetColor
xuiColorPickerSetRGBA
xuiColorPickerGetRGBA
xuiColorPickerSetAlphaEnabled
xuiColorPickerGetAlphaEnabled
xuiColorPickerSetHex
xuiColorPickerGetHex
xuiColorPickerSetPalette
xuiColorPickerGetPaletteCount
xuiColorPickerGetPaletteColor
xuiColorPickerOpen
xuiColorPickerClose
xuiColorPickerToggle
xuiColorPickerIsOpen
xuiColorPickerSetPopupSize
xuiColorPickerGetPopupSize
xuiColorPickerSetPopupPlacement
xuiColorPickerGetPopupPlacement
xuiColorPickerSetMetrics
xuiColorPickerGetMetrics
xuiColorPickerSetColors
xuiColorPickerSetBorderColors
xuiColorPickerSetPopupColors
xuiColorPickerSetFont
xuiColorPickerGetFont
xuiColorPickerGetPopupWidget
xuiColorPickerGetPanelWidget
xuiColorPickerGetSwatchRect
xuiColorPickerGetButtonRect
xuiColorPickerGetTextRect
xuiColorPickerGetSvRect
xuiColorPickerGetHueRect
xuiColorPickerGetAlphaRect
xuiColorPickerGetHexRect
xuiColorPickerGetOldRect
xuiColorPickerGetNewRect
xuiColorPickerGetPaletteRect
xuiColorPickerGetHoverPart
xuiColorPickerGetActivePart
xuiColorPickerGetState
xuiColorPickerGetChangeCount
```

## Style Properties

```text
colorpicker.text.color
colorpicker.text.disabled_color
colorpicker.background.color
colorpicker.background.hover_color
colorpicker.background.open_color
colorpicker.background.disabled_color
colorpicker.border.color
colorpicker.border.hover_color
colorpicker.border.focus_color
colorpicker.arrow.color
colorpicker.arrow.disabled_color
colorpicker.radius
colorpicker.border.width
font.name
```

Popup panel colors are configured through `xuiColorPickerSetPopupColors` or the descriptor fields. The descriptor also supports arrow, button, popup, field, separator, metric, font, palette, alpha, popup-size, and placement overrides.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_color_picker_test.bat
examples\xui_colorpicker\build.bat
build\xui_colorpicker.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `hex=1`, `palette=1`, `alpha=1`, and `disabled=1`.
