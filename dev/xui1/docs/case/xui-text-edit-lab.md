# XUI Text Edit Lab

## Build

Windows:

```bat
examples\xui_text_edit_lab\build.bat
```

Linux/macOS:

```sh
./examples/xui_text_edit_lab/build.sh
```

## Run

Windows:

```bat
build\xge_xui_text_edit_lab.exe --frames 180
```

Linux/macOS:

```sh
./build/xge_xui_text_edit_lab --frames 180
```

## Coverage

- `xgeXuiInputInit/Unit`
- `xgeXuiInputSetText/GetText`
- `xgeXuiInputSetFont`
- `xgeXuiInputSetColors`
- `xgeXuiInputSetPlaceholder`
- `xgeXuiInputSetPassword`
- `xgeXuiInputSetReadonly`
- `xgeXuiInputSetDisabled`
- `xgeXuiInputSetSelection/GetSelection`
- `xgeXuiInputGetCandidateRect`
- `xgeXuiInputEvent`
- `xgeXuiTextEditInit/Unit`
- `xgeXuiTextEditSetText/GetText`
- `xgeXuiTextEditSetFont`
- `xgeXuiTextEditSetColors`
- `xgeXuiTextEditSetReadonly`
- `xgeXuiTextEditSetWordWrap`
- `xgeXuiTextEditSetScroll`
- `xgeXuiTextEditUndo/Redo`
- `xgeXuiTextEditGetCandidateRect`
- `xgeXuiTextEditEvent`
- default clipboard and context-menu paths

## Expected

- Top status line should report `input=1 edit=1 cursor=1 ro=1 clipmenu=1 undo=1 wrap=1 ime=1`.
- Single-line `Input` shows placeholder or text, supports selection, IME, and right-click menu.
- Multi-line `TextEdit` shows several lines plus one long wrapped line.
- Caret, selection highlight, wrap mode, page navigation, undo/redo, and IME candidate placement should behave consistently.

## Final Summary

Program exits automatically and prints:

```text
xui-text-edit-lab final-summary frames=... input=... edit=... cursor=... ro=... clipmenu=... undo=... wrap=... ime=...
```

Focus on:

- `input/edit/cursor/ro/clipmenu/undo/wrap/ime`
- `menus`
- `input(len/cursor/select/password/disabled)`
- `edit(len/lines/visual/undo/redo/scroll/wrap)`
