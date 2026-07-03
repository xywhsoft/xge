# XUI TextEdit Standard Lab

`examples/xui_text_edit_standard_lab` validates the standard TextEdit editing policy without opening a window.

## Coverage

- readonly TextEdit still supports `Ctrl+A` selection.
- readonly TextEdit consumes text input without modifying text or recording undo; Backspace leaves text unchanged.
- undo/redo policy records one snapshot per edit operation and preserves/restores selection.
- new edits clear the redo stack.
- `Tab` key inserts a literal `\t` when TextEdit is editable, and the insertion participates in undo.
- find highlight reservation stores external highlight ranges and paints them behind text.
- line number reservation paints a left gutter and restores widget content rect after paint.

## Run

```bat
examples\xui_text_edit_standard_lab\build.bat
build\xge_xui_text_edit_standard_lab.exe
```

Expected summary:

```text
xui-text-edit-standard-lab final-summary create=1 standard=1 readonlyOK=1 undoOK=1 tabOK=1 reserveOK=1 readonly=0 undo=1 redo=1 highlights=2 lineNumbers=1 size=11
```
