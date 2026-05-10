# XUI Button Standard Lab

`examples/xui_button_standard_lab` validates Button standard interaction behavior, including icon-only and icon+text configurations.

## Coverage

- Button Mouse, Touch, Enter and Space click paths.
- Button blur and disabled policies.
- Button optional checked state.
- Button loading state consumes activation without firing click.
- Button semantic styles: default, primary and danger.
- Button icon + text left/right layout rect calculation.
- Icon-only Button Mouse, Enter and Space click paths.
- Icon-only Button shares the same normal, hover, active, focus and disabled state color contract.
- Button instances remain focusable after initialization.

## Run

```bat
examples\xui_button_standard_lab\build.bat
build\xge_xui_button_standard_lab.exe
```

Expected summary:

```text
xui-button-standard-lab final-summary create=1 button=1 icon=1 focus=1 options=1 clicks=5/3 callbacks=5/3 checked=1 loading=0 semantic=2 iconText=1
```
