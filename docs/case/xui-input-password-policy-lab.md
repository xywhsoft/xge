# XUI Input Password Policy Lab

`examples/xui_input_password_policy_lab` focuses on password semantics for a single-line input. It keeps one masked password input plus one plain probe input on screen, and auto-replays copy/cut guard, paste, readonly guard, IME block, and candidate rect movement without printing plaintext back out.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`
- `xgeXuiInputSetPassword`
- `xgeXuiInputSetText`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiInputGetCandidateRect`
- `xgeXuiInputSetReadonly`
- `xgeClipboardSetText`
- `xgeClipboardGetText`

## Run

```bat
examples\xui_input_password_policy_lab\build.bat
build\xge_xui_input_password_policy_lab.exe --frames 5
```

## Auto Checks

- Verifies the input widget is bound to `EventProc`, `UpdateProc`, and `PaintProc`.
- Enables password mode and checks the backing text length stays `8`.
- Seeds clipboard and sends `Ctrl+C`; when clipboard is readable, checks the clipboard text does not change.
- Sends `Ctrl+X` on a selection and checks password text plus selection stay unchanged.
- Sends `Ctrl+V`; when clipboard is readable, checks paste still works for password input.
- Enables readonly and checks paste is blocked while text plus selection stay unchanged.
- Sends `IME_START/UPDATE/END` plus a non-ASCII text commit and checks password mode keeps composition empty while the backing text stays unchanged.
- Moves the cursor from start to end and checks `xgeXuiInputGetCandidateRect` moves right.

## Manual Checks

- You should see a single masked input rather than plaintext.
- A lower plain input is present only for manual focus-out verification.
- The final state should still select the full password text.
- Typing through a normal English keyboard should still work, but IME composition text should not appear in the password field.
- Clicking the lower plain input should move focus away from the password field and let the system IME recover.
- The status line should show `mask=1 copy=1 cut=1 paste=1 readonly=1 ime=1 cand=1`.
- `clip=1` means real clipboard read/write was available; `clip=0` means clipboard-specific assertions were skipped on this machine.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
