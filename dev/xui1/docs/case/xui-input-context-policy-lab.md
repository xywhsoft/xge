# XUI Input Context Policy Lab

`examples/xui_input_context_policy_lab` focuses on the default context menu for a single-line input. It isolates the enabled-item rules and the `Select All / Cut / Copy / Paste / Delete` action path without bringing in `TextEdit`.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiInputSetReadonly`
- `xgeXuiInputSetPassword`
- `xgeXuiInputSetDisabled`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`

## Run

```bat
examples\xui_input_context_policy_lab\build.bat
build\xge_xui_input_context_policy_lab.exe --frames 5
```

## Auto Checks

- Opens the default menu on a normal input and verifies enabled flags for all five entries.
- Drives `Copy`, `Cut`, `Paste`, `Delete`, and `Select All` through the menu callback path.
- Reopens the menu in readonly mode and verifies `Cut/Paste/Delete` are disabled while `Copy` stays enabled.
- Verifies password mode blocks opening the default menu.
- Verifies disabled mode stops normal text insertion and keeps the widget disabled.

## Manual Checks

- The single input row should remain visible and readable.
- Final text should end in `" beta"` after the delete-path check.
- If you manually right-click while password mode is active, the default menu should not appear.

## Skip Conditions

- If the system clipboard is unavailable, paste-specific checks degrade to the non-paste path.
- If no usable Windows font can be loaded, initialization fails before the example starts.
