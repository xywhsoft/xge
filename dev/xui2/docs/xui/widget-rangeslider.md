# XUI RangeSlider

RangeSlider is a cache-first range value control. It behaves like a slider with two thumbs and exposes a selected start/end interval.

## Goals

- preserve the Slider control conventions: horizontal and vertical orientation, pointer capture drag, wheel, keyboard navigation, change callback, and geometry getters
- prevent the two thumbs from crossing; dragging the start thumb clamps to the end value and dragging the end thumb clamps to the start value
- render through XUI2 widget cache and proxy shape APIs
- expose track/fill/start-thumb/end-thumb geometry for integration and tests
- keep XSON deferred

## Behavior

Default range is `0..1`, start defaults to the minimum, end defaults to the maximum, and default orientation is horizontal.

`xuiRangeSliderSetRange` normalizes reversed ranges. Equal ranges become `min..min+1`. `xuiRangeSliderSetValues` clamps values to the current range and normalizes reversed start/end values. Programmatic setters do not notify the change callback. User interaction does notify.

Input support:

- left-click inside the control: capture and move the nearest thumb to the pointer position
- pointer drag: update the captured thumb continuously
- mouse wheel: small step on the active thumb; if no thumb is active, the nearest thumb is selected
- keyboard: Left/Down decrease, Right/Up increase, PageDown decreases by page step, PageUp increases, Home moves the active thumb to its lower bound, End moves it to its upper bound, Escape cancels capture

Default step fallback is `1%` of the range. Default page step fallback is `10%` of the range.

## Public API

```c
xuiRangeSliderGetType
xuiRangeSliderCreate
xuiRangeSliderSetChange
xuiRangeSliderSetRange
xuiRangeSliderGetRange
xuiRangeSliderSetValues
xuiRangeSliderGetValues
xuiRangeSliderGetStart
xuiRangeSliderGetEnd
xuiRangeSliderGetStartRate
xuiRangeSliderGetEndRate
xuiRangeSliderSetStep
xuiRangeSliderGetStep
xuiRangeSliderSetOrientation
xuiRangeSliderGetOrientation
xuiRangeSliderSetMetrics
xuiRangeSliderGetMetrics
xuiRangeSliderSetColors
xuiRangeSliderSetKnobBorderColor
xuiRangeSliderGetKnobBorderColor
xuiRangeSliderGetTrackRect
xuiRangeSliderGetFillRect
xuiRangeSliderGetStartKnobRect
xuiRangeSliderGetEndKnobRect
xuiRangeSliderGetActiveThumb
xuiRangeSliderGetState
xuiRangeSliderGetChangeCount
```

Thumb identifiers:

```c
XUI_RANGE_SLIDER_THUMB_NONE
XUI_RANGE_SLIDER_THUMB_START
XUI_RANGE_SLIDER_THUMB_END
```

The control uses shared orientation constants:

```c
XUI_ORIENTATION_HORIZONTAL
XUI_ORIENTATION_VERTICAL
```

## Style Properties

```text
rangeslider.track.color
rangeslider.fill.color
rangeslider.fill.hover_color
rangeslider.fill.active_color
rangeslider.knob.color
rangeslider.knob.border_color
rangeslider.focus.color
rangeslider.disabled.color
rangeslider.track.size
rangeslider.knob.size
rangeslider.track.radius
rangeslider.knob.radius
```

## Visual Defaults

- track: neutral light rounded rail
- fill: teal accent with brighter hover and darker active variants
- knobs: white circular thumbs with teal border
- disabled: muted rail/fill with pale knobs

The track is inset by half the knob size so both thumb centers align with the visual rail ends.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_range_slider_test.bat
examples\xui_rangeslider\build.bat
build\xui_rangeslider.exe --frames 360
```

The example summary should include `create=1`, `layout=1`, `default=1`, `vertical=1`, and `input=1`.
