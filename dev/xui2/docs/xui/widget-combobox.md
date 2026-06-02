# XUI ComboBox

ComboBox is a single-select dropdown widget. The XUI2 implementation keeps the XUI1 split between a compact owner control and a popup option list, but builds the dropdown on the existing XUI2 Menu/Popup stack instead of introducing a separate list layer.

ComboBox now supports two interaction modes:

- `XUI_COMBOBOX_MODE_SELECT`: pure selection, matching the original XUI2 ComboBox behavior.
- `XUI_COMBOBOX_MODE_EDIT`: VB-style dropdown edit mode. The user can type arbitrary text or choose an item from the dropdown.

## Goals

- preserve XUI1 single-select behavior, structured item data, disabled items, separators, and value lookup
- reuse Popup for overlay placement, outside close, Escape close, focus restore, and scroll viewport sizing
- reuse Menu for option row measurement, hover hit testing, keyboard navigation, disabled item handling, and row rendering
- reuse Input for editable text, caret, IME, selection, clipboard, hotkeys, and the right-click menu in edit mode
- keep the owner widget cache-first and styleable
- keep XSON deferred

## Structure

`xuiComboBoxCreate` returns the owner widget. Internally, it creates a Menu and uses that Menu's Popup as the dropdown.

```text
ComboBox owner widget
  internal Input widget, visible only in edit mode
  internal Menu widget
    internal Popup
      ScrollView
        ScrollFrame
          viewport
            Menu content rows
```

Use `xuiComboBoxGetInputWidget`, `xuiComboBoxGetMenuWidget`, and `xuiComboBoxGetPopupWidget` only for tests, diagnostics, or advanced integration. Normal application code should use the ComboBox text, open/close, and selection APIs.

## Items

ComboBox supports simple string arrays:

```c
const char* items[] = {"Low", "Balanced", "High"};

xui_combobox_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.arrItems = items;
desc.iItemCount = 3;
desc.iSelected = 1;
xuiComboBoxCreate(context, &combo, &desc);
```

It also supports structured item data:

```c
typedef struct xui_combobox_item_t {
	const char* sText;
	int iValue;
	int bEnabled;
	int bSeparator;
	int iIcon;
	void* pUser;
} xui_combobox_item_t;
```

For simple string items, `iValue` defaults to the item index and items are enabled by default. For structured items, `bEnabled` must be non-zero for selectable rows. Separators are never selectable.

Text pointers are not copied; the caller must keep strings alive while the ComboBox uses them.

## Selection

`xuiComboBoxSetSelected` and `xuiComboBoxSetSelectedValue` update selection without notifying the callback. User commit through mouse or keyboard notifies `xui_combobox_select_proc` only when the selected index changes.

Disabled items and separators cannot become selected. If an enabled map disables the current selection, the selected index is cleared to `-1`.

## Editable Text

`XUI_COMBOBOX_MODE_SELECT` is the default. In this mode the displayed text is always the selected item's text, and `xuiComboBoxSetText` can only select an enabled item whose text matches the supplied string.

`XUI_COMBOBOX_MODE_EDIT` embeds an Input child inside the owner control. The arrow button still opens the Menu/Popup dropdown, but the text area behaves like a normal Input:

- typed text, caret movement, selection, IME, clipboard, hotkeys, and the Input right-click menu are handled by the internal Input
- choosing an enabled dropdown item updates both `selected` and editable text
- typing text that no longer equals the selected item's text clears `selected` to `-1`
- `xuiComboBoxSetText` updates editable text without calling `xui_combobox_text_proc`
- user typing calls the callback registered through `xuiComboBoxSetTextChange`

Create an editable ComboBox:

```c
xui_combobox_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.arrItems = items;
desc.iItemCount = itemCount;
desc.iSelected = 0;
desc.iMode = XUI_COMBOBOX_MODE_EDIT;
desc.sPlaceholder = "type or choose";
xuiComboBoxCreate(context, &combo, &desc);
```

## Popup Behavior

Open paths:

- left mouse down on the owner toggles the dropdown
- Enter and Space toggle from keyboard focus
- Down opens and moves hover to the next enabled item
- Up opens and moves hover to the previous enabled item

Close paths:

- selecting an enabled item commits and closes
- Escape closes through Popup/Menu
- outside press closes through Popup
- disabling or hiding the owner closes the dropdown

Placement modes:

- `XUI_COMBOBOX_POPUP_AUTO`: bottom-left first; Popup may flip if needed
- `XUI_COMBOBOX_POPUP_BOTTOM`: bottom-left first
- `XUI_COMBOBOX_POPUP_TOP`: top-left first

The dropdown asks Popup to match owner width. `fPopupHeight` is an explicit dropdown viewport cap; otherwise `fPopupMaxHeight` is used, defaulting to 168px. If content is taller, Popup shows its compact scrollbar.

## Public API

```c
xuiComboBoxGetType
xuiComboBoxCreate
xuiComboBoxSetSelect
xuiComboBoxSetTextChange
xuiComboBoxSetItems
xuiComboBoxSetItemData
xuiComboBoxSetEnabledItems
xuiComboBoxGetItemCount
xuiComboBoxGetItem
xuiComboBoxGetItemText
xuiComboBoxGetItemValue
xuiComboBoxIsItemEnabled
xuiComboBoxSetSelected
xuiComboBoxGetSelected
xuiComboBoxSetSelectedValue
xuiComboBoxGetSelectedValue
xuiComboBoxSetMode
xuiComboBoxGetMode
xuiComboBoxSetText
xuiComboBoxGetText
xuiComboBoxOpen
xuiComboBoxClose
xuiComboBoxToggle
xuiComboBoxIsOpen
xuiComboBoxSetPopupHeight
xuiComboBoxGetPopupHeight
xuiComboBoxSetPopupMaxHeight
xuiComboBoxGetPopupMaxHeight
xuiComboBoxSetPopupPlacement
xuiComboBoxGetPopupPlacement
xuiComboBoxSetMetrics
xuiComboBoxGetMetrics
xuiComboBoxSetColors
xuiComboBoxSetBorderColors
xuiComboBoxSetArrowColors
xuiComboBoxSetPopupColors
xuiComboBoxSetFont
xuiComboBoxGetFont
xuiComboBoxGetInputWidget
xuiComboBoxGetMenuWidget
xuiComboBoxGetPopupWidget
xuiComboBoxGetButtonRect
xuiComboBoxGetTextRect
xuiComboBoxGetState
xuiComboBoxGetChangeCount
```

## Style Properties

```text
combobox.text.color
combobox.text.disabled_color
combobox.background.color
combobox.background.hover_color
combobox.background.open_color
combobox.background.disabled_color
combobox.border.color
combobox.border.hover_color
combobox.border.focus_color
combobox.arrow.color
combobox.arrow.disabled_color
combobox.radius
combobox.border.width
font.name
```

Popup row colors are configured through `xuiComboBoxSetPopupColors`, which forwards to the internal Menu.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_combobox_test.bat
examples\xui_combobox\build.bat
build\xui_combobox.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `select=1`, `disabled=1`, `key=1`, and `placement=1`.
It should also include `edit=1` after the editable ComboBox checks pass.
