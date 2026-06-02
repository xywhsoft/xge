# XUI Input

Input is a cache-first single-line text editing widget. It keeps the XUI1 behavior surface for normal application input fields while using XUI2 widget type, layout, cache render, style, event, clipboard, IME, Popup, and Menu infrastructure.

## Goals

- provide a real single-line text input widget, separate from the lower-level `xuiInput...` event ingress API
- support text, placeholder, selection, caret, max length, password display, readonly mode, error state, decoration areas, and change callback
- use proxy clipboard and IME candidate-rect hooks without adding new proxy capabilities
- include an integrated right-click menu backed by `xuiMenu`
- keep XSON deferred

## Default Visual

The default style is drawn by the proxy shape/text APIs:

- white rounded rectangle background
- light neutral border
- blue hover/focus border
- pale blue selection fill
- blue caret
- gray placeholder and disabled text
- red error background/border when `bError` is enabled

Input uses one cache for its own content. Text, selection, caret, focus, hover, disabled, readonly, and error changes invalidate the widget cache/render state through the normal XUI2 invalidation path.

## Text Editing

Supported editing behavior:

- pointer click places the caret
- pointer drag extends selection
- double-click selects the word under the pointer; `Ctrl+A` remains the select-all command
- text input inserts UTF-8 text at the caret or replaces the selection
- Backspace/Delete remove selection or one UTF-8 codepoint around the caret
- Left/Right/Home/End move the caret, with Shift extending selection
- Ctrl+A, Ctrl+C, Ctrl+X, Ctrl+V, and Ctrl+Z map to select all, copy, cut, paste, and undo

`iMaxLength` is currently a UTF-8 byte limit. Insertions are clamped at a valid codepoint boundary.

Password mode masks the rendered string and blocks copy/cut clipboard export.

Readonly mode keeps focus, selection, and copy enabled, but disables text mutation, cut, paste, and delete.

## Decorations

Input decorations preserve the XUI1 generic decoration model instead of adding one-off fields such as `searchIcon` or `clearButton`.

A decoration is a runtime node attached to the leading or trailing side of the input. Visible decorations reserve layout width by increasing the widget padding, so text, selection, caret hit testing, scrolling, and IME candidate placement automatically avoid the decoration area.

Decoration kinds:

| Kind | Behavior |
| --- | --- |
| `XUI_INPUT_DECORATION_ICON` | Draws a built-in mask icon from the atlas. |
| `XUI_INPUT_DECORATION_TEXT` | Draws centered text, useful for suffixes or compact action labels. |
| `XUI_INPUT_DECORATION_TEXTURE` | Draws a caller-provided surface region. |
| `XUI_INPUT_DECORATION_CLEAR` | Shows a built-in clear icon and clears text on click. |
| `XUI_INPUT_DECORATION_CUSTOM_PAINT` | Reserves width and delegates painting to `onPaint`. |

Built-in icons:

```text
XUI_INPUT_ICON_SEARCH
XUI_INPUT_ICON_USER
XUI_INPUT_ICON_LOCK
XUI_INPUT_ICON_EYE
```

Visibility modes:

```text
XUI_INPUT_DECORATION_VISIBLE_ALWAYS
XUI_INPUT_DECORATION_VISIBLE_NOT_EMPTY
XUI_INPUT_DECORATION_VISIBLE_FOCUSED
XUI_INPUT_DECORATION_VISIBLE_FOCUSED_NOT_EMPTY
```

The default width is 22 px for icons, clear buttons, textures, and custom paint nodes. Text decorations use measured text width plus `fPadding * 2` unless `fWidth` is set explicitly.

Pointer behavior matches XUI1: hover and active state are tracked per decoration, a decoration click captures the pointer, and the click fires only when the pointer is released over the same decoration. Readonly and disabled inputs do not trigger decoration actions.

The `examples\xui_input` demo covers the XUI1-style decoration patterns: search icon, clear button, password lock/eye toggle, right-side text action, unit suffix on right-aligned text, caller-provided texture, and custom paint.

## Context Menu

Each Input owns an internal Menu widget. The menu is opened by right-click, the context-menu key, or `xuiInputOpenMenu`.

Right-clicking while text is selected preserves the selection and keeps the selection fill visible while the menu has focus, matching the XUI1/Windows input behavior.

The default order follows the Windows Notepad style grouping:

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

Default titles are Chinese:

| Command | Default title |
| --- | --- |
| `XUI_INPUT_MENU_UNDO` | `撤销` |
| `XUI_INPUT_MENU_CUT` | `剪切` |
| `XUI_INPUT_MENU_COPY` | `复制` |
| `XUI_INPUT_MENU_PASTE` | `粘贴` |
| `XUI_INPUT_MENU_DELETE` | `删除` |
| `XUI_INPUT_MENU_SELECT_ALL` | `全选` |

Use `xuiInputSetMenuTitle(input, command, title)` to override one title. Passing `NULL` or an empty string resets that title to the default.

Menu item enabled state is rebuilt on every open:

- Undo requires undo state
- Cut/Delete require selection and non-readonly
- Copy requires selection
- Paste requires non-readonly
- Select All requires non-empty text that is not already fully selected

## Public API

```c
xuiInputGetType
xuiInputCreate
xuiInputSetChange
xuiInputSetText
xuiInputGetText
xuiInputSetPlaceholder
xuiInputGetPlaceholder
xuiInputSetFont
xuiInputGetFont
xuiInputSetMaxLength
xuiInputGetMaxLength
xuiInputSetTextAlign
xuiInputGetTextAlign
xuiInputSetPassword
xuiInputIsPassword
xuiInputSetReadonly
xuiInputIsReadonly
xuiInputSetError
xuiInputGetError
xuiInputSetSelection
xuiInputGetSelection
xuiInputSelectAll
xuiInputHasSelection
xuiInputCopy
xuiInputCut
xuiInputPaste
xuiInputDeleteSelection
xuiInputUndo
xuiInputCanUndo
xuiInputSetColors
xuiInputSetErrorColors
xuiInputDecorationAdd
xuiInputDecorationSet
xuiInputDecorationRemove
xuiInputDecorationClear
xuiInputDecorationGetRect
xuiInputSetMenuTitle
xuiInputGetMenuTitle
xuiInputOpenMenu
xuiInputGetMenuWidget
xuiInputGetTextRect
xuiInputGetCursorRect
xuiInputGetState
xuiInputGetChangeCount
```

## Style Properties

```text
input.text.color
input.placeholder.color
input.text.disabled_color
input.background.color
input.background.hover_color
input.background.disabled_color
input.border.color
input.border.hover_color
input.border.focus_color
input.error.background_color
input.error.border_color
input.selection.color
input.cursor.color
input.radius
input.border.width
font.name
```

## Example

```c
xui_input_desc_t desc;
xui_widget input;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.sText = "XUI2 Input";
desc.sPlaceholder = "请输入文本";
desc.pFont = font;
desc.iMaxLength = 64;
desc.iTextAlign = XUI_INPUT_ALIGN_LEFT;

xuiInputCreate(context, &input, &desc);
xui_input_decoration_desc_t deco;
memset(&deco, 0, sizeof(deco));
deco.iSize = sizeof(deco);
deco.iKind = XUI_INPUT_DECORATION_ICON;
deco.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
deco.iIcon = XUI_INPUT_ICON_SEARCH;
xuiInputDecorationAdd(input, XUI_INPUT_DECORATION_SIDE_LEADING, NULL, &deco);

memset(&deco, 0, sizeof(deco));
deco.iSize = sizeof(deco);
deco.iKind = XUI_INPUT_DECORATION_CLEAR;
deco.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_NOT_EMPTY;
xuiInputDecorationAdd(input, XUI_INPUT_DECORATION_SIDE_TRAILING, NULL, &deco);
xuiInputSetMenuTitle(input, XUI_INPUT_MENU_COPY, "复制文本");
xuiWidgetAddChild(root, input);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_input_widget_test.bat
examples\xui_input\build.bat
build\xui_input.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `state=1`, `menu=1`, `input=1`, `decoration=1`, and `customPaint=1`.
