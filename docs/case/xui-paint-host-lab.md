# XUI Paint Host Lab

`examples/xui_paint_host_lab` installs a custom XUI host that forwards drawing to the default engine path while counting paint callbacks. It covers host registration, paint-command style routing, clip set/clear, a custom paint callback, and manual refresh requests in one auto-exit example.

## Covered API

- `xgeXuiSetHost`
- `xgeXuiGetHost`
- `xgeXuiRefreshRequest`
- `xgeXuiRefreshNeeded`
- `xgeXuiWidgetSetPaint`
- `xgeXuiWidgetSetClip`
- `xgeXuiUpdate`
- `xgeXuiPaint`
- `xgeXuiPanelInit`
- `xgeXuiPanelSetTitle`
- `xgeXuiImageInit`
- `xgeXuiLabelInit`

## Build And Run

```bat
examples\xui_paint_host_lab\build.bat
build\xge_xui_paint_host_lab.exe --frames 5
```

## Verification

- Console prints `xui-paint-host-lab final-summary`.
- `host=1` means `xgeXuiSetHost` took effect, `xgeXuiGetHost` returned the custom host, and host-side text measurement ran.
- `paint=1` means paint-command routing produced non-zero command counts and host draw callbacks saw rect, image, and text traffic.
- `clip=1` means a clipped widget triggered both `clip_set` and `clip_clear`.
- `custom=1` means the custom `xgeXuiWidgetSetPaint` callback executed.
- `refresh=1` means `xgeXuiRefreshRequest` raised `xgeXuiRefreshNeeded`, hit the host refresh callback, and cleared again after the next paint.
