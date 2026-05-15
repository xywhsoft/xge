# XUI Overlay Menu Lab

`examples/xui_overlay_menu_lab` exercises the overlay-style XUI helpers in one auto-exit example. It covers `Popup`, `Tooltip`, `ComboBox`, `Menu`, and `Dialog`, including outside close, ESC close, modal behavior, and disabled menu rows.

## Covered API

- `xgeXuiPopupInit`
- `xgeXuiPopupUnit`
- `xgeXuiPopupSetOwner`
- `xgeXuiPopupSetClose`
- `xgeXuiPopupSetOpen`
- `xgeXuiPopupIsOpen`
- `xgeXuiPopupSetAutoClose`
- `xgeXuiPopupSetBackground`
- `xgeXuiPopupEvent`
- `xgeXuiWidgetSetTooltip`
- `xgeXuiWidgetClearTooltip`
- `xgeXuiWidgetGetTooltip`
- `xgeXuiWidgetTooltipIsOpen`
- `xgeXuiWidgetTooltipGetOwner`
- `xgeXuiWidgetTooltipGetRect`
- `xgeXuiComboBoxInit`
- `xgeXuiComboBoxUnit`
- `xgeXuiComboBoxSetItems`
- `xgeXuiComboBoxSetFont`
- `xgeXuiComboBoxSetSelect`
- `xgeXuiComboBoxSetSelected`
- `xgeXuiComboBoxGetSelected`
- `xgeXuiComboBoxSetPopupHeight`
- `xgeXuiComboBoxSetColors`
- `xgeXuiComboBoxIsOpen`
- `xgeXuiComboBoxGetState`
- `xgeXuiComboBoxEvent`
- `xgeXuiMenuInit`
- `xgeXuiMenuUnit`
- `xgeXuiMenuSetItems`
- `xgeXuiMenuSetFont`
- `xgeXuiMenuSetSelect`
- `xgeXuiMenuSetMetrics`
- `xgeXuiMenuSetColors`
- `xgeXuiMenuOpenForOwner`
- `xgeXuiMenuClose`
- `xgeXuiMenuIsOpen`
- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`
- `xgeXuiDialogSetModal`
- `xgeXuiDialogSetCloseOnEscape`
- `xgeXuiDialogSetShowClose`
- `xgeXuiDialogSetColors`
- `xgeXuiDialogEvent`

## Build And Run

```bat
examples\xui_overlay_menu_lab\build.bat
build\xge_xui_overlay_menu_lab.exe --frames 5
```

## Verification

- Console prints `xui-overlay-menu-lab final-summary`.
- `popup=1` means popup owner wiring, open state, outside close, ESC close, and `autoClose(false, true)` behavior all matched the implementation.
- `tooltip=1` means widget tooltip metadata, offset placement, context-level hover open/close, clear, and non-hooked owner event behavior all worked.
- `combo=1` means combo set/get, dropdown layout, mouse selection, outside close, keyboard open, ESC close, and disabled state reporting all worked.
- `menu=1` means menu sizing, enabled rows, disabled-row rejection, enabled-row selection, outside close, and callback delivery all worked.
- `dialog=1` means dialog title, close callback, show-close toggle, visibility control, and close button flow all worked.
- `policy=1` means the combined high-level rules held: popup outside close, popup ESC close, menu disabled item, dialog modal blocking, and dialog non-modal outside click behavior.
