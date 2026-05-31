# XUI Checkbox

Checkbox is a cache-first choice control for binary options.

## Goals

- keep XUI1 behavior: pointer click, keyboard Space toggle, checked state, change callback
- use XUI2 widget type, layout, style, event, and cache render paths
- provide a better default visual style without requiring image assets
- allow atlas or custom surface indicators for old skins and themed UI
- keep XSON deferred

## Default Visual

The default indicator is drawn by the proxy shape API:

- 18x18 rounded square
- white unchecked fill
- light blue hover fill
- blue checked fill
- white check mark
- subtle focus ring
- gray disabled fill/border/check

Checked, disabled, hover, active, and focus are separate cache states. The renderer draws the indicator from the cache state id, so a click or Space toggle updates the checked visual on the next render instead of reusing a stale state surface.

The current default palette:

```text
accent:          #2f80ed
accent hover:    #4092ff
accent active:   #1f6fd6
text:            #1f2937
disabled text:   #9ca3af
border:          #b8c4d2
hover border:    #7fb5f3
focus:           #2f80ed
unchecked fill:  #ffffff
hover fill:      #f7fbff
disabled fill:   #f3f5f8
```

## Atlas

Checkbox can draw its indicator from surfaces:

```c
xuiCheckBoxSetIndicatorSurface(widget, uncheckedSurface, uncheckedSrc, checkedSurface, checkedSrc);
```

It can also use the migrated built-in atlas:

```c
xuiCheckBoxUseBuiltinAtlas(widget, 1);
```

Built-in atlas entries:

- `checkbox_unchecked`
- `checkbox_checked`

The default is `bUseBuiltinAtlas = 0`, because the new programmatic visual is the preferred XUI2 style. Atlas support is for compatibility and skinning.

## Public API

```c
xuiCheckBoxGetType
xuiCheckBoxCreate
xuiCheckBoxSetChange
xuiCheckBoxSetText
xuiCheckBoxGetText
xuiCheckBoxSetFont
xuiCheckBoxGetFont
xuiCheckBoxSetChecked
xuiCheckBoxGetChecked
xuiCheckBoxSetTextColor
xuiCheckBoxGetTextColor
xuiCheckBoxSetDisabledTextColor
xuiCheckBoxGetDisabledTextColor
xuiCheckBoxSetIndicatorSize
xuiCheckBoxGetIndicatorSize
xuiCheckBoxSetGap
xuiCheckBoxGetGap
xuiCheckBoxSetColors
xuiCheckBoxSetIndicatorSurface
xuiCheckBoxUseBuiltinAtlas
xuiCheckBoxGetUseBuiltinAtlas
xuiCheckBoxGetIndicatorRect
xuiCheckBoxGetTextRect
xuiCheckBoxGetState
```

## Style Properties

Shared choice properties:

```text
choice.text.color
choice.text.disabled_color
choice.indicator.size
choice.indicator.gap
choice.accent.color
choice.accent.hover_color
choice.accent.active_color
choice.border.color
choice.border.hover_color
choice.focus.color
choice.focus.width
```

Checkbox-specific properties:

```text
checkbox.radius
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
test_xui\build_checkbox_test.bat
examples\xui_checkbox\build.bat
build\xui_checkbox.exe --frames 360
```

The example forwards XGE mouse input into XUI. In `--frames` mode it also performs one synthetic click; the final summary should include `input=1` and `changes=1`.
