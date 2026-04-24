# XUI Choice Controls Lab

`examples/xui_choice_controls_lab` exercises the choice-style XUI controls in one auto-exit example. It covers `Toggle`, `CheckBox`, `RadioGroup`, `Radio`, and `Switch`, including init/unit, setter/getter, state queries, and both keyboard plus mouse interaction paths.

## Covered API

- `xgeXuiToggleInit`
- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`
- `xgeXuiToggleSetText`
- `xgeXuiToggleSetChecked`
- `xgeXuiToggleGetChecked`
- `xgeXuiToggleSetTextColor`
- `xgeXuiToggleSetColors`
- `xgeXuiToggleGetState`
- `xgeXuiToggleEvent`
- `xgeXuiCheckBoxInit`
- `xgeXuiCheckBoxUnit`
- `xgeXuiCheckBoxSetChange`
- `xgeXuiCheckBoxSetText`
- `xgeXuiCheckBoxSetChecked`
- `xgeXuiCheckBoxGetChecked`
- `xgeXuiCheckBoxSetTextColor`
- `xgeXuiCheckBoxSetColors`
- `xgeXuiCheckBoxGetState`
- `xgeXuiCheckBoxEvent`
- `xgeXuiRadioGroupInit`
- `xgeXuiRadioGroupUnit`
- `xgeXuiRadioGroupSetChange`
- `xgeXuiRadioGroupSetSelected`
- `xgeXuiRadioGroupGetSelected`
- `xgeXuiRadioInit`
- `xgeXuiRadioUnit`
- `xgeXuiRadioSetGroup`
- `xgeXuiRadioSetChange`
- `xgeXuiRadioSetText`
- `xgeXuiRadioSetChecked`
- `xgeXuiRadioGetChecked`
- `xgeXuiRadioSetTextColor`
- `xgeXuiRadioSetColors`
- `xgeXuiRadioGetState`
- `xgeXuiRadioEvent`
- `xgeXuiSwitchInit`
- `xgeXuiSwitchUnit`
- `xgeXuiSwitchSetChange`
- `xgeXuiSwitchSetText`
- `xgeXuiSwitchSetChecked`
- `xgeXuiSwitchGetChecked`
- `xgeXuiSwitchSetTextColor`
- `xgeXuiSwitchSetColors`
- `xgeXuiSwitchGetState`
- `xgeXuiSwitchEvent`

## Build And Run

```bat
examples\xui_choice_controls_lab\build.bat
build\xge_xui_choice_controls_lab.exe --frames 5
```

## Verification

- Console prints `xui-choice-controls-lab final-summary`.
- `toggle=1` means toggle default values matched implementation, setters applied, and both keyboard plus mouse toggles incremented callback/change counters.
- `checkbox=1` means checkbox default values matched implementation, setters applied, and both keyboard plus mouse toggles worked.
- `group=1` means `RadioGroup` set/get and callback flow worked, and selection moved correctly between the two radios.
- `radio=1` means radio item callbacks, checked state, focus state, and grouped interaction all behaved as expected.
- `switch=1` means switch default colors matched implementation, setters applied, and both keyboard plus mouse toggles worked.
- `state=1` means toggle, checkbox, and switch all reported disabled state through their `GetState` APIs after the widgets were disabled.
