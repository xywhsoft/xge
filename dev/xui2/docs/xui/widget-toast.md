# XUI Toast Utility

`Toast` is a runtime notification service for non-blocking messages such as saved state, background task status, warnings, and operation failures. It is not a normal business widget and should not be declared as an XSON node.

## Role

- The caller owns one `xui_toast` service object, usually at application or page-root level.
- `xuiToastShow` creates a notification and returns a stable `toastId`.
- Visible notifications are placed on the tooltip overlay layer as separate hit widgets, so transparent space around the stack does not block the page below.
- If the visible limit is full, new items enter the pending queue.
- Duration starts when an item becomes visible, not when it enters the pending queue.
- Clicking the close button only closes the notification and does not call the click callback.
- Clicking the notification body calls the optional click callback and closes the notification.
- Pointer close requests are processed during the next `xuiUpdate`, which keeps event dispatch stable when callbacks call `show`, `close`, or `clear`.

This preserves the XUI1 queue and event-stability behavior while using XUI2 overlay widgets and cache-first rendering.

## API

```c
int xuiToastCreate(xui_context pContext, xui_toast* ppToast, const xui_toast_desc_t* pDesc);
void xuiToastDestroy(xui_toast pToast);
int xuiToastShow(xui_toast pToast, int iType, const char* sTitle, const char* sMessage, float fDuration, xui_toast_click_proc onClick, void* pUser);
int xuiToastClose(xui_toast pToast, int iToastId);
int xuiToastClear(xui_toast pToast);
int xuiToastGetActiveCount(xui_toast pToast);
int xuiToastGetPendingCount(xui_toast pToast);
int xuiToastSetPlacement(xui_toast pToast, int iPlacement);
int xuiToastGetPlacement(xui_toast pToast);
int xuiToastSetDirection(xui_toast pToast, int iDirection);
int xuiToastGetDirection(xui_toast pToast);
int xuiToastSetMetrics(xui_toast pToast, const xui_toast_metrics_t* pMetrics);
int xuiToastGetMetrics(xui_toast pToast, xui_toast_metrics_t* pMetrics);
int xuiToastSetColors(xui_toast pToast, const xui_toast_colors_t* pColors);
int xuiToastGetColors(xui_toast pToast, xui_toast_colors_t* pColors);
int xuiToastSetFont(xui_toast pToast, xui_font pFont);
xui_font xuiToastGetFont(xui_toast pToast);
int xuiToastSetClose(xui_toast pToast, xui_toast_close_proc onClose, void* pUser);
```

Diagnostics:

```c
xui_widget xuiToastGetItemWidget(xui_toast pToast, int iSlot);
xui_rect_t xuiToastGetItemRect(xui_toast pToast, int iSlot);
xui_rect_t xuiToastGetIconRect(xui_toast pToast, int iSlot);
xui_rect_t xuiToastGetCloseRect(xui_toast pToast, int iSlot);
int xuiToastGetShowCount(xui_toast pToast);
int xuiToastGetCloseCount(xui_toast pToast);
int xuiToastGetExpireCount(xui_toast pToast);
int xuiToastGetDropCount(xui_toast pToast);
int xuiToastGetChangeCount(xui_toast pToast);
```

## Types And Reasons

Notification types:

```c
XUI_TOAST_TYPE_INFO
XUI_TOAST_TYPE_SUCCESS
XUI_TOAST_TYPE_WARNING
XUI_TOAST_TYPE_ERROR
```

Close reasons:

```c
XUI_TOAST_CLOSE_TIMEOUT
XUI_TOAST_CLOSE_CLICK
XUI_TOAST_CLOSE_BUTTON
XUI_TOAST_CLOSE_CLEAR
XUI_TOAST_CLOSE_CAPACITY_DROP
XUI_TOAST_CLOSE_API
```

`fDuration <= 0` uses the default `3.0` seconds, matching the XUI1 toast service.

## Layout

`xuiToastSetPlacement` controls the stack anchor:

```c
XUI_TOAST_PLACEMENT_TOP_RIGHT
XUI_TOAST_PLACEMENT_TOP_LEFT
XUI_TOAST_PLACEMENT_BOTTOM_RIGHT
XUI_TOAST_PLACEMENT_BOTTOM_LEFT
XUI_TOAST_PLACEMENT_TOP_CENTER
XUI_TOAST_PLACEMENT_BOTTOM_CENTER
```

`xuiToastSetDirection` controls stack growth:

```c
XUI_TOAST_DIRECTION_AUTO
XUI_TOAST_DIRECTION_DOWN
XUI_TOAST_DIRECTION_UP
```

`AUTO` grows downward for top placements and upward for bottom placements.

`xui_toast_metrics_t` controls width, minimum width and height, margin, gap, padding, icon size, close button size, progress height, radius, and `iMaxVisible`. When `iMaxVisible` is `0`, Toast estimates the visible limit from the viewport height and clamps it to `XUI_TOAST_VISIBLE_CAPACITY`.

The pending queue capacity is `XUI_TOAST_QUEUE_CAPACITY`. When it overflows, the oldest pending item is dropped and the close callback receives `XUI_TOAST_CLOSE_CAPACITY_DROP`.

## Styling

Default styling uses the same light-blue notification language as XUI1: pale background, blue border, left accent band, readable 28 px icon block, close button, and a bottom progress line.

Built-in icons reuse the atlas entries:

- `msgbox_info`
- `property_check_12`
- `msgbox_war`
- `msgbox_error`
- `clear_10`

## Validation

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_toast_test.bat
examples\xui_toast\build.bat
build\xui_toast.exe --frames 5
```
