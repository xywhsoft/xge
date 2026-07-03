# XUI Accordion

Accordion is a cache-first collapsible section container. It preserves the XUI1 section/header/client model while moving rendering, layout, events, and style hooks to XUI2 typed widgets.

## Goals

- keep the XUI1 internal shape: Accordion root, section widgets, header widgets, and client widgets
- collapsed clients are hidden and do not contribute height
- multiple mode allows several expanded sections
- single mode keeps only one expanded section when a section opens
- disabled headers keep their visual state but ignore pointer selection
- expose section/header/client widgets so application content remains normal XUI children
- keep XSON deferred

## Structure

`xuiAccordionCreate` returns the Accordion root widget:

```text
Accordion root widget
  section widget 0
    header widget
    client widget
      user children
  section widget 1
    header widget
    client widget
      user children
```

Use `xuiAccordionGetClientWidget` or `xuiAccordionAddSectionChild` to attach application content. `xuiAccordionGetButtonWidget` returns the header hit widget; XUI2 keeps the button-like behavior in the header widget instead of reusing an XUI1 control struct.

## Modes

```c
XUI_ACCORDION_MODE_MULTIPLE
XUI_ACCORDION_MODE_SINGLE
```

In single mode, opening one section collapses the other expanded sections. Clicking an expanded section still toggles it closed, matching the XUI1 behavior.

## Interaction

- pointer click on an enabled header toggles the expanded state and calls `xui_accordion_select_proc`
- disabled headers do not toggle and do not notify
- Up/Down/Home/End move the focused header selection
- Space/Enter toggles the selected enabled header
- Escape cancels active pointer capture

## Public API

```c
xuiAccordionGetType
xuiAccordionCreate
xuiAccordionSetSelect
xuiAccordionClear
xuiAccordionAddSection
xuiAccordionGetSectionCount
xuiAccordionGetSectionWidget
xuiAccordionGetHeaderWidget
xuiAccordionGetButtonWidget
xuiAccordionGetClientWidget
xuiAccordionAddSectionChild
xuiAccordionSetSectionTitle
xuiAccordionGetSectionTitle
xuiAccordionSetSectionId
xuiAccordionGetSectionId
xuiAccordionSetExpanded
xuiAccordionIsExpanded
xuiAccordionSetSectionEnabled
xuiAccordionIsSectionEnabled
xuiAccordionSetMode
xuiAccordionGetMode
xuiAccordionSetFont
xuiAccordionGetFont
xuiAccordionSetMetrics
xuiAccordionGetMetrics
xuiAccordionSetColors
xuiAccordionGetColors
xuiAccordionGetContentHeight
xuiAccordionGetSectionRect
xuiAccordionGetHeaderRect
xuiAccordionGetClientRect
xuiAccordionGetArrowRect
xuiAccordionGetSelected
xuiAccordionGetHoverIndex
xuiAccordionGetActiveIndex
xuiAccordionGetState
xuiAccordionGetChangeCount
```

## Constants

```c
XUI_ACCORDION_SECTION_CAPACITY
XUI_ACCORDION_TITLE_CAPACITY
XUI_ACCORDION_MODE_MULTIPLE
XUI_ACCORDION_MODE_SINGLE
```

## Style Properties

```text
accordion.background.color
accordion.header.color
accordion.header.hover_color
accordion.header.expanded_color
accordion.content.color
accordion.border.color
accordion.text.color
accordion.text.active_color
accordion.text.disabled_color
accordion.header.height
accordion.spacing
accordion.content.padding
```

Color properties affect cache rendering. Header height, spacing, and content padding participate in layout.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_accordion_test.bat
examples\xui_accordion\build.bat
build\xui_accordion.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `multi=1`, `single=1`, `disabled=1`, and `style=1`.
