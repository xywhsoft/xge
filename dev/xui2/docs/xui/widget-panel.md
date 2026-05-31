# XUI Panel

Panel is a cache-first styled container widget. It keeps the XUI1 model of a root panel with a header, optional icon, title text, and a client area for application content.

## Goals

- preserve the XUI1 internal structure: root, header, icon, title, and client
- keep Panel as a style/container primitive for grouped forms, settings blocks, and tool surfaces
- attach application children to the client widget instead of mixing them with Panel internals
- support optional icon, hidden header, custom colors, border, radius, header metrics, and client clipping
- keep root, header, title, icon, and client widgets accessible for advanced layout and tests
- keep XSON deferred

## Structure

`xuiPanelCreate` returns the root panel widget. Internally, it creates four child widgets:

```text
Panel root widget
  header widget
    icon widget
    title widget
  client widget
    user children
```

Use `xuiPanelAddChild` or `xuiPanelGetClientWidget` for application content. Raw `xuiWidgetAddChild(panel, child)` attaches to the panel root and bypasses the client-area convention.

## Layout

The Panel root owns a custom layout:

- header is arranged at the top with `xuiPanelSetHeaderHeight`
- setting header height to `0` hides the header and lets the client fill the full panel
- icon and title are arranged by the header row layout
- client uses a normal XUI widget layout, defaulting to column layout, 8px padding, 6px gap, and clipping enabled

The client widget is intentionally ordinary. Callers can set its layout, padding, gap, table tracks, grid metrics, or overflow through the normal widget APIs.

## Public API

```c
xuiPanelGetType
xuiPanelCreate
xuiPanelGetHeaderWidget
xuiPanelGetIconWidget
xuiPanelGetTitleWidget
xuiPanelGetClientWidget
xuiPanelAddChild
xuiPanelInsertBefore
xuiPanelSetTitle
xuiPanelGetTitle
xuiPanelSetFont
xuiPanelGetFont
xuiPanelSetTitleColor
xuiPanelGetTitleColor
xuiPanelSetDisabledTitleColor
xuiPanelGetDisabledTitleColor
xuiPanelSetTitleAlign
xuiPanelGetTitleAlign
xuiPanelSetIcon
xuiPanelGetIconSurface
xuiPanelGetIconSource
xuiPanelSetIconSize
xuiPanelGetIconSize
xuiPanelSetBackgroundColor
xuiPanelGetBackgroundColor
xuiPanelSetHeaderColor
xuiPanelGetHeaderColor
xuiPanelSetClientColor
xuiPanelGetClientColor
xuiPanelSetBorder
xuiPanelGetBorder
xuiPanelSetRadius
xuiPanelGetRadius
xuiPanelSetHeaderHeight
xuiPanelGetHeaderHeight
xuiPanelSetHeaderGap
xuiPanelGetHeaderGap
xuiPanelSetClientClip
xuiPanelGetClientClip
xuiPanelGetHeaderRect
xuiPanelGetIconRect
xuiPanelGetTitleRect
xuiPanelGetClientRect
xuiPanelGetState
```

## Style Properties

```text
text.color
text.disabled_color
text.flags
font.name
panel.background.color
panel.header.color
panel.client.color
panel.border.color
panel.radius
panel.border.width
panel.header.height
panel.header.gap
panel.icon.size
panel.client.clip
```

Text and font properties are inherited so the title can follow surrounding style rules. Panel-specific color and metric properties apply to the root panel and internal layout.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_panel_test.bat
examples\xui_panel\build.bat
build\xui_panel.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `title=1`, `icon=1`, `style=1`, `client=1`, and `clip=1`.
