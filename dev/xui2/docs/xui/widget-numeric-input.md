# XUI NumericInput

NumericInput is a cache-first numeric editing widget. It keeps the XUI1 split between text editing and numeric semantics: an internal `xuiInput` child owns text, selection, IME, clipboard, undo, and the context menu, while the outer NumericInput owns value/range/step validation and the spinner surface.

## Goals

- reuse Input behavior instead of reimplementing single-line editing
- support min/max range, step, precision, integer mode, readonly mode, error state, wheel, Up/Down keys, and spinner buttons
- preserve invalid text until the next valid commit
- expose spinner/button geometry for tests and integration
- reuse the Input right-click menu with configurable menu titles
- keep XSON deferred

## Behavior

Default range is `0..100`, step is `1`, precision is `3`, and the spinner is visible. Reversed ranges are normalized. Equal ranges become `min..min+1`.

`xuiNumericInputSetValue` clamps to the range, applies integer rounding when enabled, updates the displayed text, and does not notify the change callback. User actions notify when the numeric value changes.

Commit behavior:

- valid text parses with `strtod`, clamps to range, clears error state, and reformats text
- invalid or empty text sets error state and keeps the user's text unchanged
- Enter and focus loss commit the current text

Input behavior:

- Up increases by step
- Down decreases by step
- mouse wheel changes by step; positive wheel increases the value
- spinner mouse down captures the pointer and only commits on mouse up over the same enabled button
- spinner up is disabled at max; spinner down is disabled at min
- readonly and disabled widgets block user stepping/spinner/wheel changes

## Internal Input

The internal Input is available through `xuiNumericInputGetInputWidget`. It is arranged inside the NumericInput content rect with the spinner width reserved on the right.

The context menu is the same Input menu:

```text
Undo
---
Cut
Copy
Paste
Delete
---
Select All
```

Use `xuiNumericInputSetMenuTitle` to override the Input menu labels for one NumericInput.

## Spinner Visibility

The descriptor defaults to visible even when zero-initialized. Use `xuiNumericInputSetSpinnerVisible(widget, 0)` after creation for normal hidden-spinner setup. A negative `bSpinnerVisible` descriptor value also creates a hidden spinner.

## Public API

```c
xuiNumericInputGetType
xuiNumericInputCreate
xuiNumericInputSetChange
xuiNumericInputSetErrorChange
xuiNumericInputSetFormatter
xuiNumericInputSetRange
xuiNumericInputGetRange
xuiNumericInputSetStep
xuiNumericInputGetStep
xuiNumericInputSetInteger
xuiNumericInputIsInteger
xuiNumericInputSetPrecision
xuiNumericInputGetPrecision
xuiNumericInputSetReadonly
xuiNumericInputIsReadonly
xuiNumericInputSetSpinnerVisible
xuiNumericInputGetSpinnerVisible
xuiNumericInputSetSpinnerWidth
xuiNumericInputGetSpinnerWidth
xuiNumericInputSetSpinnerColors
xuiNumericInputSetValue
xuiNumericInputGetValue
xuiNumericInputCommit
xuiNumericInputStep
xuiNumericInputSetText
xuiNumericInputGetText
xuiNumericInputSetMenuTitle
xuiNumericInputGetMenuTitle
xuiNumericInputOpenMenu
xuiNumericInputGetMenuWidget
xuiNumericInputGetInputWidget
xuiNumericInputGetSpinnerRect
xuiNumericInputGetButtonRect
xuiNumericInputGetHoverButton
xuiNumericInputGetActiveButton
xuiNumericInputIsButtonEnabled
xuiNumericInputGetError
xuiNumericInputGetState
xuiNumericInputGetChangeCount
```

## Style Properties

```text
numeric_input.background.color
numeric_input.background.hover_color
numeric_input.background.disabled_color
numeric_input.border.color
numeric_input.border.hover_color
numeric_input.border.focus_color
numeric_input.error.background_color
numeric_input.error.border_color
numeric_input.spinner.color
numeric_input.spinner.hover_color
numeric_input.spinner.active_color
numeric_input.spinner.border_color
numeric_input.spinner.icon_color
numeric_input.spinner.icon_disabled_color
numeric_input.radius
numeric_input.border.width
numeric_input.spinner.width
font.name
```

Text, selection, caret, clipboard, IME, and menu styling are still provided by the internal Input widget.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_numeric_input_test.bat
examples\xui_numericinput\build.bat
build\xui_numericinput.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `state=1`, `key=1`, `spin=1`, `wheel=1`, `error=1`, `noSpin=1`, and `menu=1`.
