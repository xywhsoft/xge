# XUI DatePicker

DatePicker is a cache-first date and time selection widget. The XUI2 implementation keeps the XUI1 owner-plus-popup model, and uses the XUI2 Popup layer for placement, outside close, Escape close, and focus restore.

## Goals

- preserve XUI1 modes: date, time, datetime, date range, time range, and datetime range
- keep typed input out of scope for V1; the owner field displays the value and opens the popup
- keep popup edits in a draft state until OK commits them
- support nullable values, min/max limits, first-day-of-week calendar layout, seconds visibility, custom format text, and custom range separators
- expose owner and popup panel geometry for tests and advanced integration
- keep XSON deferred

## Structure

`xuiDatePickerCreate` returns the owner widget. Internally, it creates a Popup and a panel widget used as the popup content.

```text
DatePicker owner widget
  internal Popup
    content panel
      one or two calendar panels
      optional time fields
      Today/Now, Clear, Cancel, OK footer buttons
```

Use `xuiDatePickerGetPopupWidget` and `xuiDatePickerGetPanelWidget` only for tests, diagnostics, or advanced integration. Normal application code should use the DatePicker open/close and value APIs.

## Modes

```c
XUI_DATE_PICKER_MODE_DATE
XUI_DATE_PICKER_MODE_TIME
XUI_DATE_PICKER_MODE_DATETIME
XUI_DATE_PICKER_MODE_DATE_RANGE
XUI_DATE_PICKER_MODE_TIME_RANGE
XUI_DATE_PICKER_MODE_DATETIME_RANGE
```

Single-value modes use `xuiDatePickerSetValue` and `xuiDatePickerGetValue`. Range modes use `xuiDatePickerSetRangeValue` and `xuiDatePickerGetRangeValue`.

Programmatic setters update stored values without notifying callbacks. User edits notify callbacks through the popup lifecycle.

## Value Model

Values use the existing `xtime` representation. Date modes normalize to the date part. Time modes normalize to the time part. When seconds are hidden with `xuiDatePickerSetShowSecond(picker, 0)`, seconds are committed as `0`.

`xtime == 0` is a valid value. Empty state is represented separately by `xuiDatePickerHasValue` or `xuiDatePickerHasRangeValue`.

If a range is committed with only one endpoint, the missing endpoint is filled from `xuiDatePickerSetDefaultRangeSpan`. If the end value is less than the start value, DatePicker normalizes by swapping the endpoints.

Limits are configured with `xuiDatePickerSetLimits`. Date and datetime modes compare full values; time modes compare the time part.

## Popup Behavior

Open paths:

- left mouse down on the owner toggles the popup
- Enter and Space toggle from keyboard focus

Close paths:

- OK commits the draft and fires `onChange` only when the effective value changed, then fires `onCommit`
- Cancel closes without committing and fires `onCancel`
- Escape and outside press close through Popup and cancel the draft
- Clear clears only when nullable, fires `onClear`, and fires `onChange` if a value existed
- disabling or hiding the owner closes the popup

Placement modes:

- `XUI_DATE_PICKER_POPUP_AUTO`: bottom-left first; Popup may flip if needed
- `XUI_DATE_PICKER_POPUP_BOTTOM`: bottom-left first
- `XUI_DATE_PICKER_POPUP_TOP`: top-left first

The popup size defaults to a mode-aware size and can be overridden with `xuiDatePickerSetPopupSize`. Panel rect APIs are panel-local; owner rect APIs are owner-local.

## Formatting

DatePicker supports token formatting for owner text:

```text
yyyy  four-digit year
yy    two-digit year
mm    two-digit month
dd    two-digit day
hh    two-digit hour
nn    two-digit minute
ss    two-digit second
```

Use `xuiDatePickerSetFormat` to override the default mode format, and `xuiDatePickerSetRangeSeparator` to override the range separator.

## Public API

```c
xuiDatePickerGetType
xuiDatePickerCreate
xuiDatePickerSetChanging
xuiDatePickerSetChange
xuiDatePickerSetCommit
xuiDatePickerSetCancel
xuiDatePickerSetClear
xuiDatePickerSetMode
xuiDatePickerGetMode
xuiDatePickerSetNullable
xuiDatePickerGetNullable
xuiDatePickerSetValue
xuiDatePickerGetValue
xuiDatePickerHasValue
xuiDatePickerClearValue
xuiDatePickerSetRangeValue
xuiDatePickerGetRangeValue
xuiDatePickerHasRangeValue
xuiDatePickerSetLimits
xuiDatePickerClearLimits
xuiDatePickerSetFormat
xuiDatePickerGetFormat
xuiDatePickerSetRangeSeparator
xuiDatePickerGetRangeSeparator
xuiDatePickerSetShowSecond
xuiDatePickerGetShowSecond
xuiDatePickerSetFirstDayOfWeek
xuiDatePickerGetFirstDayOfWeek
xuiDatePickerSetDefaultRangeSpan
xuiDatePickerGetDefaultRangeSpan
xuiDatePickerOpen
xuiDatePickerClose
xuiDatePickerToggle
xuiDatePickerIsOpen
xuiDatePickerSetPopupSize
xuiDatePickerGetPopupSize
xuiDatePickerSetPopupPlacement
xuiDatePickerGetPopupPlacement
xuiDatePickerSetMetrics
xuiDatePickerGetMetrics
xuiDatePickerSetColors
xuiDatePickerSetBorderColors
xuiDatePickerSetPopupColors
xuiDatePickerSetFont
xuiDatePickerGetFont
xuiDatePickerGetText
xuiDatePickerGetPopupWidget
xuiDatePickerGetPanelWidget
xuiDatePickerGetButtonRect
xuiDatePickerGetTextRect
xuiDatePickerGetCalendarPanelRect
xuiDatePickerGetTimePanelRect
xuiDatePickerGetPrevRect
xuiDatePickerGetNextRect
xuiDatePickerGetDayRect
xuiDatePickerGetDayValue
xuiDatePickerGetTimeRect
xuiDatePickerGetFooterRect
xuiDatePickerGetHoverPart
xuiDatePickerGetActivePart
xuiDatePickerGetState
xuiDatePickerGetChangingCount
xuiDatePickerGetChangeCount
xuiDatePickerGetCommitCount
xuiDatePickerGetCancelCount
xuiDatePickerGetClearCount
```

## Style Properties

```text
datepicker.text.color
datepicker.text.disabled_color
datepicker.background.color
datepicker.background.hover_color
datepicker.background.open_color
datepicker.background.disabled_color
datepicker.border.color
datepicker.border.hover_color
datepicker.border.focus_color
datepicker.arrow.color
datepicker.arrow.disabled_color
datepicker.radius
datepicker.border.width
font.name
```

Popup panel colors are configured through `xuiDatePickerSetPopupColors` or the descriptor fields.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_date_picker_test.bat
examples\xui_datepicker\build.bat
build\xui_datepicker.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `mode=1`, `value=1`, `range=1`, `limit=1`, `popup=1`, `clear=1`, and `disabled=1`.
