# XUI Input Clipboard Policy Lab

`examples/xui_input_clipboard_policy_lab` focuses on keyboard clipboard policy for a single-line input. It keeps one input on screen and auto-replays `Ctrl+A`, `Ctrl+C`, `Ctrl+X`, `Ctrl+V`, and readonly guard in sequence.

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
- `xgeClipboardSetText`
- `xgeClipboardGetText`

## Run

```bat
examples\xui_input_clipboard_policy_lab\build.bat
build\xge_xui_input_clipboard_policy_lab.exe --frames 5
```

## Auto Checks

- Verifies the input widget is bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Sends `Ctrl+A` and checks the full string is selected.
- Sends `Ctrl+C` and checks clipboard text becomes `alpha` when the current machine exposes a readable clipboard.
- Sends `Ctrl+X` and checks `beta` is removed; when clipboard is readable, it also checks clipboard text becomes `beta`.
- Seeds clipboard with `BETA`, sends `Ctrl+V`, and checks paste behavior when clipboard is available.
- Enables readonly, retries cut and paste shortcuts, and checks text plus selection stay unchanged.

## Manual Checks

- You should see a single input with `alpha beta`.
- The final state should still highlight `beta` because the readonly guard phase runs last.
- The status line should show `all=1 copy=1 cut=1 paste=1 readonly=1`.
- `clip=1` means real clipboard read/write was available; `clip=0` means clipboard assertions were skipped on this machine.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
