# XUI Input Blink Focus Lab

`examples/xui_input_blink_focus_lab` focuses on focus ownership and caret blink for single-line inputs. It keeps two inputs on screen so the example can verify blink toggle, unfocused reset, and focus handoff in one place.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`
- `xgeXuiSetFocus`

## Run

```bat
examples\xui_input_blink_focus_lab\build.bat
build\xge_xui_input_blink_focus_lab.exe --frames 5
```

## Auto Checks

- Verifies both input widgets are bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Focuses the primary input and checks `UpdateProc` toggles caret visibility after `0.51s`.
- Clears focus and checks unfocused update resets caret visibility to `1`.
- Switches focus to the secondary input and checks the new focus owner takes over blink state.
- Sends a mouse-down into the primary input and checks the event path restores focus there.

## Manual Checks

- You should see two stacked inputs with different caret colors.
- The focused row should own the visible caret; the inactive row should look idle.
- Clicking the upper row should bring focus back to it cleanly.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
