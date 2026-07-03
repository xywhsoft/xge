# XUI Input Delete Policy Lab

`examples/xui_input_delete_policy_lab` focuses on delete behavior for a single-line input. It keeps one input on screen and auto-replays plain `Backspace`, plain `Delete`, selection delete, and readonly guard in sequence.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`
- `xgeXuiInputSetText`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiInputSetReadonly`

## Run

```bat
examples\xui_input_delete_policy_lab\build.bat
build\xge_xui_input_delete_policy_lab.exe --frames 5
```

## Auto Checks

- Verifies the input widget is bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Sets cursor at the end of `alpha`, sends `Backspace`, and checks the last character is removed.
- Sets cursor at the start of `alpha`, sends `Delete`, and checks the first character is removed.
- Selects `beta`, sends `Backspace`, and checks the selection is deleted as one range.
- Enables readonly, retries both delete keys on a selection, and checks text plus selection stay unchanged.

## Manual Checks

- You should see a single input with `alpha beta`.
- The final state should still highlight `beta` because the readonly guard phase runs last.
- The status line should show `back=1 delete=1 select=1 readonly=1`.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
