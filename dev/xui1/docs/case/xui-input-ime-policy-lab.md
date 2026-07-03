# XUI Input IME Policy Lab

`examples/xui_input_ime_policy_lab` focuses on the IME path for single-line input. It keeps one main input for `IME_START/UPDATE/END` and candidate rect movement, plus one placeholder input that stays empty and only shows composition text.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetPlaceholder`
- `xgeXuiInputGetCandidateRect`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`

## Run

```bat
examples\xui_input_ime_policy_lab\build.bat
build\xge_xui_input_ime_policy_lab.exe --frames 5
```

## Auto Checks

- Verifies the main input widget is bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Sends `IME_START` and confirms composition storage is initialized.
- Sends `IME_UPDATE` with `"ni hao"` and records the candidate rect.
- Moves the caret to `Home`, sends another `IME_UPDATE`, and confirms the candidate rect shifts left.
- Sends `IME_END` and confirms composition clears.
- Sends a normal text event after IME end and confirms committed text still follows the normal insert path.
- Sends IME composition to an empty placeholder row and confirms the row still has empty real text but non-empty composition text.

## Manual Checks

- The top input should show a yellow candidate rect marker.
- The marker should sit farther right before the caret is moved to `Home`.
- The lower input should visually show composition text instead of its placeholder while composition is active.
- Both rows should remain stable and readable without flicker.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
