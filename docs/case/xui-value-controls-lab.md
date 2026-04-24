# XUI Value Controls Lab

`examples/xui_value_controls_lab` exercises the value-style XUI controls in one auto-exit example. It covers `Slider`, `Progress`, `Splitter`, and `ScrollBar`, including init/unit, range/value/page/orientation setters, color updates, state queries, and the mouse or wheel paths that actually mutate values.

## Covered API

- `xgeXuiSliderInit`
- `xgeXuiSliderUnit`
- `xgeXuiSliderSetChange`
- `xgeXuiSliderSetRange`
- `xgeXuiSliderSetValue`
- `xgeXuiSliderGetValue`
- `xgeXuiSliderSetColors`
- `xgeXuiSliderGetState`
- `xgeXuiSliderEvent`
- `xgeXuiProgressInit`
- `xgeXuiProgressUnit`
- `xgeXuiProgressSetRange`
- `xgeXuiProgressSetValue`
- `xgeXuiProgressGetValue`
- `xgeXuiProgressSetText`
- `xgeXuiProgressSetTextColor`
- `xgeXuiProgressSetColors`
- `xgeXuiSplitterInit`
- `xgeXuiSplitterUnit`
- `xgeXuiSplitterSetChange`
- `xgeXuiSplitterSetRange`
- `xgeXuiSplitterSetValue`
- `xgeXuiSplitterGetValue`
- `xgeXuiSplitterSetOrientation`
- `xgeXuiSplitterSetColors`
- `xgeXuiSplitterGetState`
- `xgeXuiSplitterEvent`
- `xgeXuiScrollBarInit`
- `xgeXuiScrollBarUnit`
- `xgeXuiScrollBarSetChange`
- `xgeXuiScrollBarSetRange`
- `xgeXuiScrollBarSetPage`
- `xgeXuiScrollBarSetValue`
- `xgeXuiScrollBarGetValue`
- `xgeXuiScrollBarSetOrientation`
- `xgeXuiScrollBarSetColors`
- `xgeXuiScrollBarGetState`
- `xgeXuiScrollBarEvent`

## Build And Run

```bat
examples\xui_value_controls_lab\build.bat
build\xge_xui_value_controls_lab.exe --frames 5
```

## Verification

- Console prints `xui-value-controls-lab final-summary`.
- `slider=1` means slider default values matched the current theme, reversed range inputs were normalized, color overrides stuck, and drag interaction changed value through the callback path.
- `progress=1` means progress range normalization, value clamping, text/font assignment, text color, and fill colors all updated as expected.
- `splitter=1` means splitter default orientation matched implementation, custom range and colors applied, the horizontal separator responded to vertical drag, and focus plus hover state were observable through `GetState`.
- `scrollbar=1` means range/page/orientation updates worked, negative page input clamped to `0`, track click, thumb drag, and wheel scrolling all changed the value, and the thumb drag state was released correctly.
- `state=1` means slider, splitter, and scrollbar all reported disabled state through their `GetState` APIs after the widgets were disabled.
