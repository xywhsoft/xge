# XUI TextEdit

TextEdit is a cache-first multi-line text editing widget. It extends the Input editing model to multi-line content while reusing the existing XUI2 widget, input, clipboard, IME, Popup, and Menu infrastructure.

## Goals

- provide a real multi-line editing widget for application UI
- support text, placeholder, selection, caret, max length, readonly mode, word wrap, optional line numbers, internal scrolling, undo/redo, and change callback
- reuse Input context-menu command constants and Chinese menu titles
- keep clipboard and IME behavior on the existing proxy contract
- keep XSON deferred

## Default Visual

The default style matches Input:

- white rounded rectangle background
- neutral border
- blue hover/focus border
- pale blue selection fill
- blue caret
- gray placeholder and disabled text
- optional left line-number gutter with a muted background and divider

TextEdit renders its own background, border, text, selection, and caret into one widget cache. Text, selection, caret, scroll, focus, hover, disabled, readonly, font, and style changes invalidate the normal XUI2 cache/render path.

## Text Editing

Supported editing behavior:

- pointer click places the caret
- pointer drag extends selection
- double-click selects the word under the pointer
- text input inserts UTF-8 text at the caret or replaces the selection
- Enter inserts a newline
- Backspace/Delete remove selection or one UTF-8 codepoint around the caret
- Left/Right/Up/Down/PageUp/PageDown/Home/End move the caret, with Shift extending selection
- Ctrl+A, Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+Z, and Ctrl+Y map to select all, copy, cut, paste, undo, and redo
- mouse wheel scrolls internal content
- right-click preserves the current selection while the context menu is focused

`iMaxLength` is currently a UTF-8 byte limit. Insertions are clamped at a valid codepoint boundary.

Readonly mode keeps focus, selection, and copy enabled, but disables text mutation, cut, paste, and delete.

## Line Numbers

Set `bLineNumbers` in `xui_text_edit_desc_t` or call `xuiTextEditSetLineNumbers(textEdit, 1, width)` to enable a left gutter. The gutter is part of TextEdit's own cache and does not scroll horizontally. Text layout, caret hit testing, selection rectangles, scroll clamping, and IME candidate positioning all use the text rect after the line-number gutter, so the editing surface does not overlap the numbers.

Line numbers are physical-line based. Wrapped continuation rows keep the gutter but do not repeat a number, matching the expected editor behavior.

## Context Menu

Each TextEdit owns an internal Menu widget. The menu is opened by right-click, the context-menu key, or `xuiTextEditOpenMenu`.

TextEdit deliberately reuses Input command ids:

```c
XUI_INPUT_MENU_UNDO
XUI_INPUT_MENU_CUT
XUI_INPUT_MENU_COPY
XUI_INPUT_MENU_PASTE
XUI_INPUT_MENU_DELETE
XUI_INPUT_MENU_SELECT_ALL
```

The default order follows the Windows Notepad grouping:

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

Default titles are Chinese and match Input:

| Command | Default title |
| --- | --- |
| `XUI_INPUT_MENU_UNDO` | `撤销` |
| `XUI_INPUT_MENU_CUT` | `剪切` |
| `XUI_INPUT_MENU_COPY` | `复制` |
| `XUI_INPUT_MENU_PASTE` | `粘贴` |
| `XUI_INPUT_MENU_DELETE` | `删除` |
| `XUI_INPUT_MENU_SELECT_ALL` | `全选` |

Use `xuiTextEditSetMenuTitle(textEdit, command, title)` to override one title. Passing `NULL` or an empty string resets that title to the default.

## Public API

```c
xuiTextEditGetType
xuiTextEditCreate
xuiTextEditSetChange
xuiTextEditSetText
xuiTextEditGetText
xuiTextEditSetPlaceholder
xuiTextEditGetPlaceholder
xuiTextEditSetFont
xuiTextEditGetFont
xuiTextEditSetMaxLength
xuiTextEditGetMaxLength
xuiTextEditSetReadonly
xuiTextEditIsReadonly
xuiTextEditSetWordWrap
xuiTextEditGetWordWrap
xuiTextEditSetLineNumbers
xuiTextEditGetLineNumbers
xuiTextEditGetLineNumberWidth
xuiTextEditSetLineNumberColors
xuiTextEditSetSelection
xuiTextEditGetSelection
xuiTextEditSelectAll
xuiTextEditHasSelection
xuiTextEditCopy
xuiTextEditCut
xuiTextEditPaste
xuiTextEditDeleteSelection
xuiTextEditUndo
xuiTextEditRedo
xuiTextEditCanUndo
xuiTextEditCanRedo
xuiTextEditSetScroll
xuiTextEditGetScroll
xuiTextEditScrollBy
xuiTextEditSetColors
xuiTextEditSetMenuTitle
xuiTextEditGetMenuTitle
xuiTextEditOpenMenu
xuiTextEditGetMenuWidget
xuiTextEditGetTextRect
xuiTextEditGetCursorRect
xuiTextEditGetLineCount
xuiTextEditGetState
xuiTextEditGetChangeCount
```

## Style Properties

```text
textedit.text.color
textedit.placeholder.color
textedit.text.disabled_color
textedit.background.color
textedit.background.hover_color
textedit.background.disabled_color
textedit.border.color
textedit.border.hover_color
textedit.border.focus_color
textedit.selection.color
textedit.cursor.color
textedit.line_number.color
textedit.line_number.background_color
textedit.line_number.border_color
textedit.line_number.width
textedit.radius
textedit.border.width
textedit.line_gap
font.name
```

TextEdit also falls back to the corresponding `input.*` style property where that is appropriate, so shared input themes can style both widgets.

## Example

```c
xui_text_edit_desc_t desc;
xui_widget textEdit;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.sText = "XUI2 TextEdit\nmulti-line text";
desc.sPlaceholder = "请输入多行文本";
desc.pFont = font;
desc.iMaxLength = 4096;
desc.bWordWrap = 1;
desc.bLineNumbers = 1;
desc.fLineNumberWidth = 46.0f;

xuiTextEditCreate(context, &textEdit, &desc);
xuiTextEditSetMenuTitle(textEdit, XUI_INPUT_MENU_COPY, "复制文本");
xuiWidgetAddChild(root, textEdit);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_text_edit_test.bat
examples\xui_textedit\build.bat
build\xui_textedit.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `state=1`, `menu=1`, and `input=1`. `state=1` also verifies that line numbers are enabled.

## Current Scope

This V1 has internal scrolling but does not yet draw visible TextEdit-owned scrollbars. If XUI1-style scrollbar modes are required, the next slice should wire TextEdit to ScrollFrame/ScrollBar presentation without changing the text-editing core.
