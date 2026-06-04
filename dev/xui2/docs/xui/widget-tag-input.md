# XUI TagInput

TagInput is a composite input control for editing a small list of short text tags. It follows the compact tag + inline input style shown by common web cascaded form controls, while reusing the XUI Input widget for caret, IME, selection, clipboard, and hotkey behavior.

Reference: [layui-vue TagInput](https://www.layui-vue.com/zh-CN/components/tagInput)

## Goals

- display existing tags before an inline input area
- commit the current text by Enter, comma, or semicolon
- remove the last tag with Backspace when the inline input is empty
- remove a specific tag by clicking its close mark
- support wrapping when tags exceed one line
- preserve Input text editing behavior by using an internal `xuiInput` child

## Visual States

TagInput owns four cache states:

```text
0                         normal
XUI_WIDGET_STATE_HOVER     hover
XUI_WIDGET_STATE_FOCUS     child input focused
XUI_WIDGET_STATE_DISABLED  disabled
```

The default style uses a white rectangular field, light border, blue focus border, light gray tag background, and a darker close mark on tag hover. The default radius is `0.0f` to match common form input visuals.

## Public API

```c
xuiTagInputGetType
xuiTagInputCreate
xuiTagInputSetChange
xuiTagInputAddTag
xuiTagInputRemoveTag
xuiTagInputClearTags
xuiTagInputSetTags
xuiTagInputGetTagCount
xuiTagInputGetTag
xuiTagInputSetText
xuiTagInputGetText
xuiTagInputCommit
xuiTagInputSetPlaceholder
xuiTagInputGetPlaceholder
xuiTagInputSetFont
xuiTagInputGetFont
xuiTagInputSetMaxTags
xuiTagInputGetMaxTags
xuiTagInputSetMaxLength
xuiTagInputGetMaxLength
xuiTagInputSetColors
xuiTagInputGetTagRect
xuiTagInputGetCloseRect
xuiTagInputGetInputRect
xuiTagInputGetInputWidget
xuiTagInputGetState
xuiTagInputGetChangeCount
```

`xuiTagInputGetInputWidget` exposes the internal Input widget for tests and advanced integration. Normal code should prefer `xuiTagInputSetText`, `xuiTagInputCommit`, and tag APIs.

## Interaction

- Left click on a tag close mark removes that tag on pointer up if the pointer is still on the same close mark.
- Left click in empty field space focuses the inline input.
- Enter commits the current text as one tag.
- Comma and semicolon commit the current text and do not insert the delimiter.
- Backspace removes the last tag only when the inline input is empty.
- `iMaxTags` defaults to `XUI_TAG_INPUT_TAG_CAPACITY`; `0` in the descriptor means the default capacity.
- `iMaxLength` is forwarded to the internal Input widget.

## Style Properties

```text
taginput.text.color
taginput.placeholder.color
taginput.text.disabled_color
taginput.background.color
taginput.background.hover_color
taginput.background.focus_color
taginput.background.disabled_color
taginput.border.color
taginput.border.hover_color
taginput.border.focus_color
taginput.tag.background_color
taginput.tag.hover_background_color
taginput.tag.text_color
taginput.tag.close_color
taginput.tag.close_hover_color
taginput.radius
taginput.border.width
taginput.tag.height
font.name
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_tag_input_test.bat
examples\xui_taginput\build.bat
build\xui_taginput.exe --frames 3
build_dll.bat
```

The example synthetic run should report `input=1`, `api=1`, `layout=1`, and `state=1`.
