# XUI Theme Lab

`examples/xui_theme_lab` exercises the Tech Blue theme defaults, context theme set/get, DIP scale, `xgeXuiStyleFromTheme`, and light/dark theme switching in one auto-exit example.

## Covered API

- `xgeXuiThemeDefault`
- `xgeXuiSetTheme`
- `xgeXuiGetTheme`
- `xgeXuiSetDipScale`
- `xgeXuiGetDipScale`
- `xgeXuiStyleFromTheme`
- `xgeXuiRefreshNeeded`
- `xgeXuiRefreshClear`
- `xgeXuiButtonInit`
- `xgeXuiButtonSetText`
- `xgeXuiWidgetSetSize`
- `xgeXuiSizeDip`

## Theme Coverage

- Tech Blue default palette: text, background, panel, border, accent, selection, and state colors.
- Size tokens: radius, padding, spacing, and border width.
- Default font inheritance through context theme.
- State color inheritance for button normal, hover, active, focus, and disabled.
- Visual state swatches for normal, hover, active, focus, and disabled.
- Geometry/bitmap-mask fallback policy for built-in symbols is documented in `docs/api/xui.md`; this lab verifies theme values that those draw paths consume.

## Build And Run

```bat
examples\xui_theme_lab\build.bat
build\xge_xui_theme_lab.exe --frames 5
```

## Verification

- Console prints `xui-theme-lab final-summary`.
- `default=1` means the context starts with the documented Tech Blue default theme values and DIP scale `1.0`.
- `setget=1` means `xgeXuiSetTheme` applied the dark theme, `xgeXuiGetTheme` returned it, and refresh was requested.
- `dip=1` means a DIP-sized probe widget changed from `24x12` to `48x24` after `xgeXuiSetDipScale(..., 2.0f)`.
- `style=1` means `xgeXuiStyleFromTheme` copied panel color, radius, padding, and spacing from the active theme.
- `lightdark=1` means the lab switched from dark to light theme and verified that the resulting palette differs.
- `button=1` means a button initialized under the dark theme inherited its font and state colors from that theme.
- `states=1` means the state swatch widgets display the documented normal, hover, active, focus, and disabled colors.
