# XUI MsgTip Utility

`MsgTip` is a runtime utility for one short centered prompt. It is used for save success, lightweight warning, operation failure, or similar transient feedback. It is not a normal business widget and should not be declared as an XSON node.

## Role

- The caller owns one `xui_msgtip` service object, usually at application or page-root level.
- `xuiMsgTipShow` replaces the current tip text/type/duration and opens the overlay.
- Only the tip body is hit-test visible. Clicking outside does not close the tip and does not consume the event.
- Clicking the tip body closes it immediately.
- `fDuration > 0` closes automatically after that many seconds.
- `fDuration <= 0` keeps the tip open until `xuiMsgTipClose` or a body click.

This preserves the XUI1 behavior while using the XUI2 overlay root and cache-first rendering.

## API

```c
int xuiMsgTipCreate(xui_context pContext, xui_msgtip* ppTip, const xui_msgtip_desc_t* pDesc);
void xuiMsgTipDestroy(xui_msgtip pTip);
int xuiMsgTipShow(xui_msgtip pTip, int iType, const char* sText, float fDuration);
int xuiMsgTipClose(xui_msgtip pTip);
int xuiMsgTipIsOpen(xui_msgtip pTip);
int xuiMsgTipSetText(xui_msgtip pTip, const char* sText);
const char* xuiMsgTipGetText(xui_msgtip pTip);
int xuiMsgTipSetType(xui_msgtip pTip, int iType);
int xuiMsgTipGetType(xui_msgtip pTip);
int xuiMsgTipSetDuration(xui_msgtip pTip, float fDuration);
float xuiMsgTipGetDuration(xui_msgtip pTip);
int xuiMsgTipSetIconSurface(xui_msgtip pTip, xui_surface pSurface, xui_rect_t tSrc);
int xuiMsgTipUseBuiltinIcon(xui_msgtip pTip);
int xuiMsgTipSetMetrics(xui_msgtip pTip, const xui_msgtip_metrics_t* pMetrics);
int xuiMsgTipGetMetrics(xui_msgtip pTip, xui_msgtip_metrics_t* pMetrics);
int xuiMsgTipSetColors(xui_msgtip pTip, const xui_msgtip_colors_t* pColors);
int xuiMsgTipGetColors(xui_msgtip pTip, xui_msgtip_colors_t* pColors);
int xuiMsgTipSetClose(xui_msgtip pTip, xui_msgtip_close_proc onClose, void* pUser);
xui_widget xuiMsgTipGetWidget(xui_msgtip pTip);
xui_rect_t xuiMsgTipGetTipRect(xui_msgtip pTip);
xui_rect_t xuiMsgTipGetIconRect(xui_msgtip pTip);
xui_rect_t xuiMsgTipGetTextRect(xui_msgtip pTip);
```

## Types

```c
XUI_MSGTIP_ICON_NONE
XUI_MSGTIP_ICON_INFO
XUI_MSGTIP_ICON_QUEST
XUI_MSGTIP_ICON_WAR
XUI_MSGTIP_ICON_ERROR
```

The built-in icons reuse the MsgBox atlas entries. `xuiMsgTipSetIconSurface` enables a custom icon without changing the type. `xuiMsgTipUseBuiltinIcon` returns to the built-in atlas.

## Layout And Style

Default metrics are close to XUI1: min width `92`, max width `360`, min height `40`, icon size `20`, and vertical offset `-72`, so the prompt appears centered but slightly above the screen center.

`xui_msgtip_metrics_t` controls:

- `fMinWidth`, `fMaxWidth`, `fMinHeight`
- `fPaddingX`, `fPaddingY`
- `fIconSize`, `fIconGap`
- `fRadius`
- `fOffsetY`

`xui_msgtip_colors_t` controls background, border, text, icon, and shadow colors.

## Validation

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_msgtip_test.bat
examples\xui_msgtip\build.bat
build\xui_msgtip.exe --frames 5
```
