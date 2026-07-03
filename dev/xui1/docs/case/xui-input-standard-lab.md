# XUI Input Standard Lab

`examples/xui_input_standard_lab` validates the standard Input callback and length-limit contract.

## Coverage

- `Change` callback fires only when visible text actually changes.
- `Submit` callback fires on focused Enter and receives current text.
- `Filter` callback receives old/new text, rejects invalid candidate text, and suppresses `Change` on rejection.
- `MaxLength` clamps typed and programmatic text on UTF-8 byte boundaries.
- error state stores error text, paints an error border/background, and records error text rect.
- clear button computes a right-side hit rect, clears text through the same `Change` path, and respects readonly.
- prefix/suffix icon reservation computes icon rects, adjusts padding, and shifts suffix icon when clear button is visible.
- readonly input consumes text events, Backspace and `Ctrl+X` without modifying text or firing `Change`.
- readonly input still supports `Ctrl+A` selection.
- password input consumes copy/cut shortcuts without exposing text, rejects non-ASCII text input, and clears IME composition.
- IME candidate rect is clamped to the input content rect and keeps a 1px cursor-width anchor.
- Backspace mutation participates in the same `Change` path.

## Run

```bat
examples\xui_input_standard_lab\build.bat
build\xge_xui_input_standard_lab.exe
```

Expected summary:

```text
xui-input-standard-lab final-summary create=1 standard=1 change=9/9 submit=1/1 filter=2/1 clear=2 icons=1/3 max=0 error=0 text=abcd lastChange=abcd lastSubmit=Z
```
