# XUI Focus Capture Event Lab

`examples/xui_focus_capture_event_lab` uses two XUI buttons and a self-driven event replay to cover hit-test, direct dispatch, queued dispatch, focus changes, pointer enter/leave, capture lost, keyboard activation, and right-click context events in one auto-exit example.

## Covered API

- `xgeXuiHitTest`
- `xgeXuiSetFocus`
- `xgeXuiSetCapture`
- `xgeXuiDispatchEvent`
- `xgeXuiEventPush`
- `xgeXuiEventPop`
- `xgeXuiEventCount`
- `xgeXuiDispatchQueuedEvents`
- `xgeXuiWidgetSetCaptureEvent`
- `xgeXuiWidgetSetCaptureEventUser`
- `xgeXuiButtonInit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonEvent`
- `xgeXuiUpdate`
- `xgeXuiPaint`

## Build And Run

```bat
examples\xui_focus_capture_event_lab\build.bat
build\xge_xui_focus_capture_event_lab.exe --frames 5
```

## Verification

- Console prints `xui-focus-capture-event-lab final-summary`.
- `hit=1` means `xgeXuiHitTest` resolves both buttons correctly.
- `dispatch=1` means direct dispatch ran and `ESC` cleared capture through the event path.
- `queue=1` means `event push/pop/count/dispatchQueuedEvents` all completed and the queue drained back to zero.
- `focus=1` means direct focus set plus `Tab/Shift+Tab` generated both focus-in and focus-out transitions.
- `pointer=1` means pointer move generated enter/leave between the two buttons.
- `capture=1` means both capture replacement and `ESC` produced `capture lost`.
- `keynav=1` means `Enter` and `Space` activated the focused button.
- `context=1` means right mouse down/up produced context begin/end on the target widget.
- `hooks=1` means both `xgeXuiWidgetSetCaptureEvent` and `xgeXuiWidgetSetCaptureEventUser` paths received callbacks.
