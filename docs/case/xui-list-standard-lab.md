# XUI List Standard Lab

`examples/xui_list_standard_lab` validates the maintained ListView behavior.

## Coverage

- Disabled item: clicking a disabled row does not change selection or emit select callback.
- Keyboard navigation: Arrow Up/Down, Home, End, PageUp and PageDown skip disabled rows and ensure the selected row remains visible.
- Enter/Space: emits select callback for the current enabled row.
- Focus policy: keyboard input is ignored when the widget is blurred or disabled.
- Selection mode: validates `single`, `multi` and `range` selection using an external selection buffer.
- Item renderer reservation: validates that the per-row renderer callback receives visible row state flags.

## Run

```bat
examples\xui_list_standard_lab\build.bat
build\xge_xui_list_standard_lab.exe
```

Expected summary:

```text
xui-list-standard-lab final-summary create=1 keyboard=1 disabled=1 focus=1 selection=1 renderer=1 selected=3 scroll=60.00 callbacks=11 last=3 renderCalls=5
```
