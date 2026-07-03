# XUI MsgBox

MsgBox is a Window-backed utility object for short decisions and alerts. It preserves the XUI1 design that message boxes are convenient popup windows, not normal layout widgets and not XSON page nodes.

## Goals

- keep MsgBox as a utility object built from XUI2 Window, Button, overlay, text layout, and built-in atlas assets
- preserve XUI1 behavior: title bar, close button, modal shield, icon types, automatic message wrapping, preset/custom buttons, Enter/Escape results
- expose internal widgets and geometry for tests and advanced integrations
- keep XSON deferred

## Structure

`xuiMsgBoxCreate` returns an opaque `xui_msgbox` handle. Internally it owns:

```text
Window root widget
  client widget
    content widget
    button widgets
Backdrop widget, only attached while modal and open
```

The Window is used for title, close, activation, moving, and overlay ownership. The client widget is switched to manual layout because MsgBox owns its message and button geometry.

## Icons

Preset types map to built-in atlas entries:

```c
XUI_MSGBOX_ICON_NONE
XUI_MSGBOX_ICON_INFO
XUI_MSGBOX_ICON_QUEST
XUI_MSGBOX_ICON_WAR
XUI_MSGBOX_ICON_ERROR
```

`xuiMsgBoxSetIconSurface` replaces the built-in atlas icon with an application surface. `xuiMsgBoxUseBuiltinIcon` switches back to the atlas icon for the current type.

## Buttons And Results

Preset buttons:

```c
XUI_MSGBOX_BUTTON_OK
XUI_MSGBOX_BUTTON_OK_CANCEL
XUI_MSGBOX_BUTTON_YES_NO
XUI_MSGBOX_BUTTON_YES_NO_CANCEL
XUI_MSGBOX_BUTTON_CUSTOM
```

Preset results:

```c
XUI_MSGBOX_RESULT_CLOSE   /* -1 */
XUI_MSGBOX_RESULT_OK      /* 0 */
XUI_MSGBOX_RESULT_CANCEL  /* 1 */
XUI_MSGBOX_RESULT_YES     /* 0 */
XUI_MSGBOX_RESULT_NO      /* 2 */
```

Custom buttons use `xui_msgbox_button_t` with text, result id, and `XUI_BUTTON_SEMANTIC_*`.

## Modal Behavior

MsgBox is modal by default. The modal backdrop consumes pointer and keyboard events outside the window but does not close the box. Closing happens through:

- title bar close button: `XUI_MSGBOX_RESULT_CLOSE`
- Escape: `XUI_MSGBOX_RESULT_CLOSE`
- Enter while the window has focus: first button result
- button click: that button result

Use `xuiMsgBoxSetModal(box, 0)` for a modeless floating message window.

## Public API

```c
xuiMsgBoxCreate
xuiMsgBoxDestroy
xuiMsgBoxSetText
xuiMsgBoxSetTitle
xuiMsgBoxGetTitle
xuiMsgBoxSetMessage
xuiMsgBoxGetMessage
xuiMsgBoxSetType
xuiMsgBoxGetType
xuiMsgBoxSetIconSurface
xuiMsgBoxUseBuiltinIcon
xuiMsgBoxSetButtons
xuiMsgBoxSetCustomButtons
xuiMsgBoxSetResult
xuiMsgBoxSetModal
xuiMsgBoxIsModal
xuiMsgBoxSetOpen
xuiMsgBoxIsOpen
xuiMsgBoxGetResult
xuiMsgBoxSetMetrics
xuiMsgBoxGetMetrics
xuiMsgBoxSetColors
xuiMsgBoxGetColors
xuiMsgBoxGetWindowWidget
xuiMsgBoxGetContentWidget
xuiMsgBoxGetBackdropWidget
xuiMsgBoxGetButtonWidget
xuiMsgBoxGetButtonCount
xuiMsgBoxGetIconRect
xuiMsgBoxGetMessageRect
xuiMsgBoxGetButtonRect
xuiMsgBoxGetBackdropRect
xuiMsgBoxGetWrapLineCount
xuiMsgBoxGetResultCount
xuiMsgBoxGetChangeCount
```

## Example

```c
xui_msgbox_desc_t desc;
xui_msgbox box;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.sTitle = "Project";
desc.sMessage = "The scene has unsaved changes. Save it before opening another project?";
desc.pFont = font;
desc.iType = XUI_MSGBOX_ICON_QUEST;
desc.iButtons = XUI_MSGBOX_BUTTON_YES_NO_CANCEL;

xuiMsgBoxCreate(context, &box, &desc);
xuiMsgBoxSetResult(box, onResult, user);
xuiMsgBoxSetOpen(box, 1);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_msgbox_test.bat
examples\xui_msgbox\build.bat
build\xui_msgbox.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `enter=1`, and `custom=1`.
