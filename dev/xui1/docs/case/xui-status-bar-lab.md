# XUI Status Bar Lab

`examples/xui_status_bar_lab` validates the StatusBar control.

## Coverage

- Text item, progress item, fixed spacer and flexible spacer layout.
- Left, center and right status sections.
- Progress value clamping.
- Clickable item select callback.
- Disabled clickable item ignores pointer input.

## Run

```bat
examples\xui_status_bar_lab\build.bat
build\xge_xui_status_bar_lab.exe --frames 2
```

Expected summary:

```text
xui-status-bar-lab final-summary frames=2 init=1 layout=1 flex=1 progress=1 click=1 disabled=1 last=0 cb=1
```
