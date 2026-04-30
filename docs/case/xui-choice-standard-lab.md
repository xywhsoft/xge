# XUI Choice Standard Lab

`examples/xui_choice_standard_lab` validates standard choice-control interaction behavior for CheckBox, Radio, Switch and Toggle.

## Coverage

- CheckBox: focused Space/Enter toggles checked; unfocused keyboard events do not toggle; disabled state blocks change callbacks and reports disabled state.
- Radio: focused Space/Enter selects; Up/Down/Left/Right navigate within the same group; selected value is exclusive; repeating the selected item does not fire group change again; disabled radio is skipped by keyboard navigation.
- Switch: focused Space/Enter toggles checked; unfocused keyboard events do not toggle; disabled state blocks change callbacks and reports disabled state.
- Toggle: aligned with CheckBox checked/change naming; focused Space/Enter toggles checked; disabled state blocks change callbacks; paint uses the same Tech Blue bordered box and bitmap check mark style.
- Focus: CheckBox, Radio, Switch and Toggle remain focusable after initialization.

## Run

```bat
examples\xui_choice_standard_lab\build.bat
build\xge_xui_choice_standard_lab.exe
```

Expected summary:

```text
xui-choice-standard-lab final-summary create=1 checkbox=1 radio=1 switch=1 toggle=1 focus=1 callbacks=2/4/4/2/2
```
