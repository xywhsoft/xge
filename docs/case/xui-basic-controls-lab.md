# XUI Basic Controls Lab

`examples/xui_basic_controls_lab` exercises the basic display/control widgets in one auto-exit example. It covers `Label`, `Image`, `Button`, Button icon mode, `Panel`, and `Separator`, plus their init/unit, setter, and state paths.

## Covered API

- `xgeXuiLabelInit`
- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`
- `xgeXuiLabelSetFont`
- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`
- `xgeXuiLabelMeasure`
- `xgeXuiImageInit`
- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`
- `xgeXuiImageSetSource`
- `xgeXuiImageSetColor`
- `xgeXuiImageSetMode`
- `xgeXuiButtonInit`
- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`
- `xgeXuiButtonEvent`
- `xgeXuiButtonSetIcon`
- `xgeXuiButtonSetIconColor`
- `xgeXuiButtonSetIconLayout`
- `xgeXuiPanelInit`
- `xgeXuiPanelUnit`
- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`
- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`
- `xgeXuiPanelSetClip`
- `xgeXuiSeparatorInit`
- `xgeXuiSeparatorUnit`
- `xgeXuiSeparatorSetColor`
- `xgeXuiSeparatorSetThickness`
- `xgeXuiSeparatorSetOrientation`

## Build And Run

```bat
examples\xui_basic_controls_lab\build.bat
build\xge_xui_basic_controls_lab.exe --frames 5
```

## Verification

- Console prints `xui-basic-controls-lab final-summary`.
- `label=1` means the label default values, text/font/color/alignment setters, and `xgeXuiLabelMeasure` all matched expectations.
- `image=1` means the image default texture/color/mode values were observed and texture/source/color/mode setters all applied.
- `button=1` means the button text/color/state palette setters worked and both mouse click plus focused Enter key paths incremented click counters and callback counters.
- `icon=1` means Button icon texture/source/iconColor/layout/palette setters worked and both focused Space key plus mouse click paths fired.
- `panel=1` means the panel default background/title state matched implementation and title/background/clip setters all applied.
- `separator=1` means the separator default color/thickness/orientation matched implementation and thickness clamped to `1.0f` when set below the minimum.
- `state=1` means both button instances reported disabled state through `GetState` after the widgets were disabled.
