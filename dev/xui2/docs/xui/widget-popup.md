# XUI Popup

Popup is the shared overlay mechanism for temporary UI such as menus, pickers, and command palettes. It is not a normal layout container. The content path is always backed by a ScrollView so oversized popup content keeps a stable viewport and scrolls instead of scaling.

## Goals

- preserve the XUI1 popup behavior model: owner binding, anchor placement, fallback, outside close, Escape close, and focus restore
- implement Popup as an XUI2 overlay-layer widget
- use ScrollView / ScrollFrame for all popup payload content
- avoid chained scrollbars when one scrollbar reduces the available viewport
- keep XSON deferred

## Structure

```text
Popup overlay shell
  popup panel
    ScrollView
      ScrollFrame
        viewport widget
          content widget
            application children
        horizontal ScrollBar
        vertical ScrollBar
        corner
```

Applications add popup content to the content widget:

```c
xui_widget content = xuiPopupGetContentWidget(popup);
xuiWidgetAddChild(content, child);
```

The Popup widget itself is the overlay shell. When close policies need outside or owner click handling, the shell spans the viewport and the panel is arranged inside it. When outside handling is ignored and the popup is not modal, the shell can shrink to the panel rect so hit testing can pass through to normal widgets.

## Placement

Popup placement is resolved from:

- owner widget world rect, or an explicit anchor rect
- anchor point: bottom-left, bottom-right, top-left, top-right, cursor, or fixed
- direction: right-down, right-up, left-down, or left-up
- gap and offset
- viewport margin

The fallback order follows XUI1:

1. requested direction
2. vertical flip
3. horizontal flip
4. diagonal flip
5. clamp to the viewport margin

`xuiPopupSetMatchOwnerWidth` makes the content viewport match the owner width before scrollbar reserve is applied.

## Scroll Sizing

Popup never scales content. It resolves an outer panel size from the content size, max size, border, padding, and scrollbar reserve.

When only one axis overflows, Popup expands the outer panel by the scrollbar reserve on that axis if the window still has room. This preserves the original content viewport width or height and avoids creating a second scrollbar only because the first scrollbar consumed space. A second-axis scrollbar is allowed only when the hard viewport or max-size limit cannot fit content plus the first scrollbar reserve.

## Close And Focus Policy

Close policy:

- `XUI_POPUP_OUTSIDE_CLOSE`
- `XUI_POPUP_OUTSIDE_IGNORE`
- `XUI_POPUP_OUTSIDE_CONSUME`
- `XUI_POPUP_OWNER_PASSTHROUGH`
- `XUI_POPUP_OWNER_CLOSE`
- `XUI_POPUP_OWNER_TOGGLE`
- `XUI_POPUP_OWNER_CONSUME`
- `XUI_POPUP_ESCAPE_CLOSE`
- `XUI_POPUP_ESCAPE_IGNORE`

Focus policy:

- `XUI_POPUP_FOCUS_POPUP`
- `XUI_POPUP_FOCUS_NONE`
- `XUI_POPUP_FOCUS_FIRST_CHILD`
- `XUI_POPUP_FOCUS_CUSTOM`

Opening stores the current focus unless an explicit restore target was set. Closing restores focus if the target is still visible, enabled, and focusable. If the owner is hidden, disabled, or destroyed, `xuiPopupApplyPlacement` and event handling close the popup.

## Public API

```c
xuiPopupGetType
xuiPopupCreate
xuiPopupSetChange
xuiPopupSetOpen
xuiPopupIsOpen
xuiPopupToggle
xuiPopupApplyPlacement
xuiPopupSetOwner
xuiPopupGetOwner
xuiPopupSetAnchorRect
xuiPopupClearAnchorRect
xuiPopupGetAnchorRect
xuiPopupSetAnchor
xuiPopupSetDirection
xuiPopupSetGap
xuiPopupSetOffset
xuiPopupSetContentSize
xuiPopupGetContentSize
xuiPopupSetMaxSize
xuiPopupSetMatchOwnerWidth
xuiPopupSetClosePolicy
xuiPopupSetModal
xuiPopupSetConsumeInside
xuiPopupSetFocusPolicy
xuiPopupSetFocusRestore
xuiPopupSetScroll
xuiPopupGetScroll
xuiPopupGetPanelWidget
xuiPopupGetScrollViewWidget
xuiPopupGetFrameWidget
xuiPopupGetViewportWidget
xuiPopupGetContentWidget
xuiPopupGetModel
xuiPopupGetPopupRect
xuiPopupGetViewportRect
xuiPopupGetContentRect
xuiPopupSetColors
xuiPopupSetMetrics
xuiPopupGetChangeCount
```

## Example

```c
xui_popup_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pOwner = button;
desc.fContentWidth = 280.0f;
desc.fContentHeight = 420.0f;
desc.fMaxHeight = 220.0f;
desc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
desc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
desc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
desc.iOwnerPolicy = XUI_POPUP_OWNER_TOGGLE;
desc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
desc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;

xuiPopupCreate(context, &popup, &desc);
xuiWidgetAddChild(xuiPopupGetContentWidget(popup), menuItem);
xuiPopupSetOpen(popup, 1);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_popup_test.bat
examples\xui_popup\build.bat
build\xui_popup.exe --frames 360
```

The example summary should include `create=1`, `layout=1`, `scroll=1`, and `close=1`.
