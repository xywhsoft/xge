# XUI Input Policy Lab

`examples/xui_input_policy_lab` focuses on single-line input policy behavior. It keeps five inputs on screen at once so the example can double as a compact visual checklist for placeholder, horizontal scroll, password mask, readonly guards, and disabled-state focus rules.

## Covered API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetPlaceholder`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetPassword`
- `xgeXuiInputSetReadonly`
- `xgeXuiInputSetDisabled`
- `xgeXuiInputGetCandidateRect`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`
- `xgeXuiWidgetIsEnabled`

## Run

```bat
examples\xui_input_policy_lab\build.bat
build\xge_xui_input_policy_lab.exe --frames 5
```

## Auto Checks

- Verifies the input widget bindings point to `EventProc`, `UpdateProc`, and `PaintProc`.
- Verifies the placeholder row keeps empty text and a non-empty placeholder string.
- Moves the long-text row to `End` and checks that horizontal scroll becomes non-zero while the candidate rect stays inside the content box.
- Uses `Ctrl+A` and `Ctrl+C` on the password row and confirms the stored text is unchanged.
- Uses `Ctrl+A`, text input, and `Backspace` on the readonly row and confirms the text stays unchanged while selection still works.
- Clicks the disabled row through the XUI dispatcher and confirms focus stays on the previous input.

## Manual Checks

- The first row should show placeholder text instead of real content.
- The second row should show a long line already scrolled toward the end.
- The third row should display password bullets, not raw text.
- The fourth row should stay editable-looking but block changes.
- The fifth row should look disabled and should not take focus.

## Skip Conditions

- If no usable Windows font can be loaded, initialization fails before the example starts.
