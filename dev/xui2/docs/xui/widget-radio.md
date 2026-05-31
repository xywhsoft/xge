# XUI Radio

Radio is a cache-first choice control for mutually exclusive options.

## Goals

- keep XUI1 behavior: pointer click, keyboard Space select, checked state, change callback
- provide `radiogroup` for one-selected-at-a-time behavior
- use XUI2 widget type, layout, style, event, and cache render paths
- provide a better default visual style without requiring image assets
- allow atlas or custom surface indicators for old skins and themed UI
- keep XSON deferred

## Default Visual

The default indicator is drawn by the proxy shape API:

- 18x18 circle
- white unchecked fill
- light blue hover fill
- blue checked fill
- white inner dot
- subtle focus ring
- gray disabled fill/border/dot

Checked, disabled, hover, active, and focus are separate cache states. A click or Space select updates the checked state id immediately, so render uses the matching cached state surface instead of keeping a stale visual.

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

## Radio Group

`radiogroup` is a lightweight container that owns selection coordination. It accepts existing radio widgets or creates options directly:

```c
xuiRadioGroupAddRadio(group, radio);
xuiRadioGroupAddOption(group, &radio, &desc);
```

The group can be vertical or horizontal:

```c
xuiRadioGroupSetOrientation(group, XUI_RADIO_GROUP_VERTICAL);
xuiRadioGroupSetOrientation(group, XUI_RADIO_GROUP_HORIZONTAL);
```

When one child radio is selected, sibling radios in the same group are unchecked. The group exposes selected index and selected widget getters, plus a group-level change callback.

## Atlas

Radio can draw its indicator from surfaces:

```c
xuiRadioSetIndicatorSurface(widget, uncheckedSurface, uncheckedSrc, checkedSurface, checkedSrc);
```

It can also use the migrated built-in atlas:

```c
xuiRadioUseBuiltinAtlas(widget, 1);
```

Built-in atlas entries:

- `radio_unchecked`
- `radio_checked`

The default is `bUseBuiltinAtlas = 0`, because the new programmatic visual is the preferred XUI2 style. Atlas support is for compatibility and skinning.

## Public API

```c
xuiRadioGetType
xuiRadioCreate
xuiRadioSetChange
xuiRadioSetText
xuiRadioGetText
xuiRadioSetFont
xuiRadioGetFont
xuiRadioSetChecked
xuiRadioGetChecked
xuiRadioSetTextColor
xuiRadioGetTextColor
xuiRadioSetDisabledTextColor
xuiRadioGetDisabledTextColor
xuiRadioSetIndicatorSize
xuiRadioGetIndicatorSize
xuiRadioSetGap
xuiRadioGetGap
xuiRadioSetColors
xuiRadioSetIndicatorSurface
xuiRadioUseBuiltinAtlas
xuiRadioGetUseBuiltinAtlas
xuiRadioGetIndicatorRect
xuiRadioGetTextRect
xuiRadioGetState
xuiRadioGroupGetType
xuiRadioGroupCreate
xuiRadioGroupSetChange
xuiRadioGroupAddRadio
xuiRadioGroupAddOption
xuiRadioGroupSetSelectedIndex
xuiRadioGroupGetSelectedIndex
xuiRadioGroupGetSelectedRadio
xuiRadioGroupSetOrientation
xuiRadioGroupGetOrientation
xuiRadioGroupSetGap
xuiRadioGroupGetGap
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

Radio-specific properties:

```text
radio.dot.scale
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
test_xui\build_radio_test.bat
examples\xui_radio\build.bat
build\xui_radio.exe --frames 360
```

The example forwards XGE mouse input into XUI. In `--frames` mode it also performs one synthetic click; the final summary should include `input=1`, `groupChanges=1`, and `selected=0`.
