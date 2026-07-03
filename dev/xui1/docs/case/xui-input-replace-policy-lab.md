# XUI Input Replace Policy Lab

`examples/xui_input_replace_policy_lab` focuses on how a single-line input handles replacing an active selection. It keeps one input on screen and auto-replays typing replace, `Ctrl+V` replace, `Delete` replace, and readonly guard in sequence.

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
examples\xui_input_replace_policy_lab\build.bat
build\xge_xui_input_replace_policy_lab.exe --frames 5
```

## Auto Checks

- Verifies the input widget is bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Selects `beta`, sends a text event, and checks the selection is replaced by `X`.
- Selects `beta`, prefers `Ctrl+V` with clipboard text `Z`, and falls back to the same replace shape if the current machine does not expose a writable system clipboard.
- Selects ` beta`, sends `Delete`, and checks the selection is removed cleanly.
- Enables readonly, retries text, delete, and paste paths, and checks the original text plus selection stay unchanged.

## Manual Checks

- You should see a single input with `alpha beta`.
- The final state should still highlight `beta` because the readonly guard phase runs last.
- The status line should show `text=1 paste=1 delete=1 readonly=1`; `clip=1` means real clipboard paste ran, `clip=0` means fallback was used.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
