# XUI Hyperlink

Hyperlink is a text-only clickable control. It keeps the text layout behavior of Label, but participates in pointer and keyboard input like a lightweight Button.

## Goals

- render as text, not as a button background
- keep three local state caches: normal, hover, active
- expose a click callback and click count
- support Space and Enter activation when focused
- keep Label-style text flags, wrap mode, underline, line gap, paragraph gap, and font styling

## Visual States

Hyperlink owns three cache states:

```text
0                       normal
XUI_WIDGET_STATE_HOVER   hover
XUI_WIDGET_STATE_ACTIVE  pressed / keyboard active
```

The default visual uses accent blue text, darker active text, disabled gray text, and underlines only hover and active states. The control remains transparent; background and focus visuals are left to the containing UI.

## Public API

```c
xuiHyperlinkGetType
xuiHyperlinkCreate
xuiHyperlinkSetClick
xuiHyperlinkSetText
xuiHyperlinkGetText
xuiHyperlinkSetFont
xuiHyperlinkGetFont
xuiHyperlinkSetTextColor
xuiHyperlinkGetTextColor
xuiHyperlinkSetHoverTextColor
xuiHyperlinkGetHoverTextColor
xuiHyperlinkSetActiveTextColor
xuiHyperlinkGetActiveTextColor
xuiHyperlinkSetDisabledTextColor
xuiHyperlinkGetDisabledTextColor
xuiHyperlinkSetTextColors
xuiHyperlinkSetTextFlags
xuiHyperlinkGetTextFlags
xuiHyperlinkSetWrapMode
xuiHyperlinkGetWrapMode
xuiHyperlinkSetUnderline
xuiHyperlinkGetUnderline
xuiHyperlinkGetHoverUnderline
xuiHyperlinkGetActiveUnderline
xuiHyperlinkSetLineGap
xuiHyperlinkGetLineGap
xuiHyperlinkSetParagraphGap
xuiHyperlinkGetParagraphGap
xuiHyperlinkGetState
xuiHyperlinkGetClickCount
```

## Style Properties

Inherited text/font properties:

```text
text.color
text.disabled_color
text.flags
text.wrap
text.underline
text.line_gap
text.paragraph_gap
font.name
```

Hyperlink-specific properties:

```text
hyperlink.text.color
hyperlink.text.hover_color
hyperlink.text.active_color
hyperlink.text.disabled_color
hyperlink.text.hover_underline
hyperlink.text.active_underline
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_hyperlink_test.bat
examples\xui_hyperlink\build.bat
build\xui_hyperlink.exe --frames 3
```

The example synthetic click should produce a final summary with `input=1` and `clicks=1`.
