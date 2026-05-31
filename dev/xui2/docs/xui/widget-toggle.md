# XUI Toggle

Toggle is a cache-first switch control for binary settings.

## Goals

- keep XUI1-style behavior: pointer click, keyboard Space toggle, checked state, change callback
- use XUI2 widget type, layout, style, event, and cache render paths
- provide a better default visual style without requiring image assets
- keep the switch thumb compact, close to the layui default switch feel
- allow atlas or custom surface indicators for old skins and themed UI
- preserve XUI1-style inner state text, such as `OFF` / `ON`, without replacing the external label text API
- keep XSON deferred

## Default Visual

The default indicator is drawn by the proxy shape API:

- 38x22 rounded pill track
- 14x14 compact circular thumb
- gray unchecked track
- blue checked track
- white thumb
- subtle focus ring
- gray disabled track/thumb

Checked, disabled, hover, active, and focus are separate cache states. A click or Space toggle updates the checked state id immediately, so render uses the matching cached state surface instead of keeping a stale visual.

## Inner State Text

XUI1 allowed a toggle to draw two state titles inside the switch track. XUI2 keeps that behavior separately from the external label:

```c
xuiToggleSetText(widget, "Power mode");
xuiToggleSetInnerText(widget, "OFF", "ON");
```

`xuiToggleSetText` remains the normal text drawn after the switch. `xuiToggleSetInnerText` draws the unchecked text on the right side of the thumb and the checked text on the left side of the thumb, matching the old XUI1 behavior. When inner text is present, the default vector track grows to fit the larger state title. Empty or `NULL` inner text clears the state title and keeps the compact switch size.

When a custom surface or built-in atlas is used, the surface is treated as the whole switch indicator and inner text is not drawn on top of it. This matches XUI1, where textured toggles owned the full visual.

The current default palette:

```text
accent:          #2f80ed
accent hover:    #4092ff
accent active:   #1f6fd6
track:           #d7e1ed
track hover:     #ccdae9
track active:    #becfe2
track border:    #b9c7d8
thumb:           #ffffff
text:            #1f2937
disabled text:   #9ca3af
focus:           #2f80ed
```

## Atlas

Toggle can draw the whole switch indicator from surfaces:

```c
xuiToggleSetIndicatorSurface(widget, uncheckedSurface, uncheckedSrc, checkedSurface, checkedSrc);
```

It can also attempt to use built-in atlas entries:

```c
xuiToggleUseBuiltinAtlas(widget, 1);
```

Expected built-in atlas names, if added later:

- `toggle_unchecked`
- `toggle_checked`

The current migrated XUI1 atlas does not include dedicated toggle artwork. In that case the control falls back to the default vector-drawn style. Custom atlas surfaces are supported now and are used by the example.

## Public API

```c
xuiToggleGetType
xuiToggleCreate
xuiToggleSetChange
xuiToggleSetText
xuiToggleGetText
xuiToggleSetInnerText
xuiToggleGetUncheckedText
xuiToggleGetCheckedText
xuiToggleSetInnerTextColor
xuiToggleSetInnerTextMetrics
xuiToggleSetFont
xuiToggleGetFont
xuiToggleSetChecked
xuiToggleGetChecked
xuiToggleSetTextColor
xuiToggleGetTextColor
xuiToggleSetDisabledTextColor
xuiToggleGetDisabledTextColor
xuiToggleSetTrackSize
xuiToggleGetTrackWidth
xuiToggleGetTrackHeight
xuiToggleSetThumbSize
xuiToggleGetThumbSize
xuiToggleSetGap
xuiToggleGetGap
xuiToggleSetColors
xuiToggleSetIndicatorSurface
xuiToggleUseBuiltinAtlas
xuiToggleGetUseBuiltinAtlas
xuiToggleGetTrackRect
xuiToggleGetThumbRect
xuiToggleGetTextRect
xuiToggleGetInnerTextRect
xuiToggleGetState
```

## Style Properties

Shared choice properties:

```text
choice.text.color
choice.text.disabled_color
choice.indicator.gap
choice.accent.color
choice.accent.hover_color
choice.accent.active_color
choice.focus.color
choice.focus.width
```

Toggle-specific properties:

```text
toggle.track.width
toggle.track.height
toggle.thumb.size
toggle.track.color
toggle.track.hover_color
toggle.track.active_color
toggle.track.border_color
toggle.thumb.color
toggle.thumb.border_color
toggle.inner_text.unchecked_color
toggle.inner_text.checked_color
toggle.inner_text.padding
toggle.inner_text.gap
```

Inherited text/font properties:

```text
text.color
text.disabled_color
text.flags
font.name
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_toggle_test.bat
examples\xui_toggle\build.bat
build\xui_toggle.exe --frames 360
```

The example forwards XGE mouse input into XUI. In `--frames` mode it also performs one synthetic click; the final summary should include `input=1`, `atlas=1`, and `changes=1`.
