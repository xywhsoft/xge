# XUI Toolbar Lab

`examples/xui_toolbar_lab` validates the Toolbar control.

## Coverage

- Button, toggle and separator item types.
- Disabled item ignores pointer input.
- Keyboard activation with Space/Enter focus path.
- Horizontal and vertical layout.
- Group metadata and group gap layout.
- Item tooltip metadata and hover tooltip query.
- Overflow button layout, overflow item query and overflow callback.

## Run

```bat
examples\xui_toolbar_lab\build.bat
build\xge_xui_toolbar_lab.exe --frames 2
```

Expected summary:

```text
xui-toolbar-lab final-summary frames=2 init=1 toggle=1 disabled=1 keyboard=1 vertical=1 group=1 tooltip=1 overflow=1 last=1 cb=2 of=1
```
