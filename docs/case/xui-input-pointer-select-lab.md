# XUI Input Pointer Select Lab

`examples/xui_input_pointer_select_lab` focuses on pointer-driven selection behavior for a single-line input. It keeps one short input on screen and auto-replays click, drag, and double-click so the final state is easy to read.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`
- `xgeXuiInputGetSelection`
- `xgeXuiSetFocus`
- `xgeXuiDispatchEvent`
- `xgeTextMeasure`

## Run

```bat
examples\xui_input_pointer_select_lab\build.bat
build\xge_xui_input_pointer_select_lab.exe --frames 5
```

## Auto Checks

- Verifies the input widget is bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Clears focus and sends a click into the input to verify pointer routing restores focus ownership.
- Sends a click near `gamma` and checks the caret lands in the later part of the string with no active selection.
- Sends a left-to-right drag from after `alpha ` into `gamma` and checks the selection becomes `6..13` and capture is released on mouse-up.
- Seeds a double-click at `beta` and checks word selection becomes `6..10`.

## Manual Checks

- You should see a single input with `alpha beta gamma`.
- The final state should highlight `beta` in the input.
- The status line should end with `capture=0`.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
