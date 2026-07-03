# XUI Slider

Slider is a cache-first value control for volume, numeric settings, tool options, and game/app UI parameters.

## Goals

- preserve XUI1 behavior: range/value, horizontal and vertical orientation, pointer capture drag, wheel, keyboard navigation, and change callback
- render through XUI2 widget cache and proxy shape APIs
- provide a cleaner light flat default style without requiring atlas assets
- expose track/fill/knob geometry for integration and tests
- keep XSON deferred

## Behavior

Default range is `0..1`, value starts at the minimum, and default orientation is horizontal.

`xuiSliderSetRange` normalizes reversed ranges. Equal ranges become `min..min+1`. `xuiSliderSetValue` clamps values to the current range and does not notify the change callback. User interaction does notify.

Input support:

- left-click inside the control: capture and move the value to the pointer position
- pointer drag: update continuously while captured
- mouse wheel: small step; positive wheel increases the value
- keyboard: Left/Down decrease, Right/Up increase, PageDown decreases by page step, PageUp increases by page step, Home sets minimum, End sets maximum, Escape cancels capture

Default step fallback is `1%` of the range. Default page step fallback is `10%` of the range.

## Public API

```c
xuiSliderGetType
xuiSliderCreate
xuiSliderSetChange
xuiSliderSetRange
xuiSliderGetRange
xuiSliderSetValue
xuiSliderGetValue
xuiSliderGetRate
xuiSliderSetStep
xuiSliderGetStep
xuiSliderSetOrientation
xuiSliderGetOrientation
xuiSliderSetMetrics
xuiSliderGetMetrics
xuiSliderSetColors
xuiSliderSetKnobBorderColor
xuiSliderGetKnobBorderColor
xuiSliderGetTrackRect
xuiSliderGetFillRect
xuiSliderGetKnobRect
xuiSliderGetState
xuiSliderGetChangeCount
```

The control uses shared orientation constants:

```c
XUI_ORIENTATION_HORIZONTAL
XUI_ORIENTATION_VERTICAL
```

## Style Properties

```text
slider.track.color
slider.fill.color
slider.fill.hover_color
slider.fill.active_color
slider.knob.color
slider.knob.border_color
slider.focus.color
slider.disabled.color
slider.track.size
slider.knob.size
slider.track.radius
slider.knob.radius
```

## Visual Defaults

- track: light blue-gray rounded rail
- fill: blue accent with brighter hover and darker active variants
- knob: white circular thumb with subtle blue-gray border
- disabled: muted rail/fill with pale knob

The track is inset by half the knob size so the knob center aligns with both ends of the visual rail.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_slider_test.bat
examples\xui_slider\build.bat
build\xui_slider.exe --frames 360
```

The example summary should include `create=1`, `layout=1`, `default=1`, `vertical=1`, and `input=1`.
