# XUI Global Color Styles

## Contract

Built-in widget chrome and default content colors use the existing stylesheet
resolver. The normal priority is the widget's API/descriptor base, then the
default, inherited, type, named, class, state-class and inline stylesheet values.
Only properties registered as inherited pass down the widget tree. Tokens remain usable
wherever a color property is accepted. An absent property falls back to the
current API base; a COLOR property with value zero is an explicit transparent
color, not an absent override. Clearing the stylesheet restores the API base.

Styles do not rewrite an owner's stored API palette or user content. Compound
controls can forward resolved colors to their owned children's APIs; those child
getters then report the forwarded base, while the owner's getters keep reporting
the original API palette. In particular,
explicit rich-document formatting, TableView cell/row/column colors, chart
series, timeline item colors, image pixels, custom painters, and terminal SGR
truecolor are content rather than widget chrome. Their existing explicit-color
precedence is retained. A renderer's default/fallback colors are themeable.

Color-only changes invalidate paint caches, not layout, row indexes, document
paragraphs or virtual content measurement. Composite controls synchronize their
dependent paint state before the framework prepares child caches. Already-open
popups and cached viewports do not need to be recreated for a theme change.

## Global Configuration

For example, remove the separately colored ComboBox expand-button surface in
normal, hover and open states:

```c
#include "xui.h"

#define COLOR_PROP(name, rgba) { \
    .iSize = sizeof(xui_style_property_t), .sName = name, \
    .tValue = { .iSize = sizeof(xui_style_value_t), \
        .iType = XUI_STYLE_VALUE_COLOR, .iColor = rgba } }

static int apply_colors(xui_context context)
{
    static const xui_style_property_t colors[] = {
        COLOR_PROP("combobox.background.color", XUI_COLOR_RGBA(32, 53, 61, 255)),
        COLOR_PROP("combobox.text.color", XUI_COLOR_RGBA(224, 236, 240, 255)),
        COLOR_PROP("combobox.button.color", 0),
        COLOR_PROP("combobox.button.hover_color", 0),
        COLOR_PROP("combobox.button.open_color", 0),
        COLOR_PROP("combobox.arrow.color", XUI_COLOR_RGBA(100, 214, 213, 255))
    };
    if (xuiComboBoxGetType(context) == NULL) return XUI_ERROR_INVALID_STATE;
    return xuiStyleSetDefault(context, colors,
        (int)(sizeof(colors) / sizeof(colors[0])));
}
```

`xuiStyleSetDefault` replaces the complete default stylesheet; add these entries
to the application's existing property array rather than issuing one call per
property. Register the relevant widget type (or create the control) before
configuring its sheet so that property metadata is available.

The same property names work with `xuiStyleSetType`, named/class styles,
`xuiWidgetSetInlineStyle`, and color tokens. Global defaults also reach the
internal widgets of compound controls. The generic child widget's stylesheet
can override colors forwarded through its API by its owner; for example,
`menu.*` controls the Menu in a ComboBox, and `scrollbar.*` controls scrollbars
inside scroll containers. Avoid conflicting owner and child global rules unless
that override is intentional. PropertyGrid has a role-aware paint adapter:
child inline `tableview.*` overrides win, then `propertygrid.*` role colors,
then generic `tableview.*` theme colors, then API bases. This rule does not
change the color precedence of an ordinary TableView.

## Coverage Map

| Controls | Style families / detailed reference |
| --- | --- |
| ComboBox, Cascader, ColorPicker, DatePicker, IconPicker | [Picker colors](XUI_STYLE_COLORS_PICKERS.md) |
| CheckBox, Radio, Toggle, Input, NumericInput, Canvas and basic content widgets | [Basic colors](XUI_STYLE_COLORS_BASIC.md) |
| Popup, Menu, MsgTip, Toast, MsgBox, FileDialog | [Overlay colors](XUI_STYLE_COLORS_OVERLAYS.md) |
| Shared Tooltip | `tooltip.background.color`, `tooltip.border.color`, `tooltip.text.color`; absent properties use `xuiSetChromeStyle` values |
| ListView, TableView, TreeView, MessageList, TimeLineView | [Collection colors](XUI_STYLE_COLORS_COLLECTIONS.md) |
| ScrollFrame, ScrollView, DockPanel, TableGrid, PropertyGrid | [Container colors](XUI_STYLE_COLORS_CONTAINERS.md) |
| Slider, RangeSlider, ScrollBar, VirtualJoystick, MenuBar, Toolbar, StatusBar, Tabs, Window, Accordion, Carousel, Chart, InventoryGrid and other chrome | [Chrome colors](XUI_STYLE_COLORS_CHROME.md) |
| RichEdit and CodeEdit | Editor additions below; existing `codeedit.*` and `textedit.*` properties remain available |
| Terminal | `background.color`, `foreground.color`, `cursor.color`, `selection.color`, `selection.text_color`, `search.highlight_color`, `focus.color`, `focus.disabled_color`, `link.hover_color`; prefix every name with `terminal.` |
| Terminal ANSI palette | `terminal.palette.0` through `terminal.palette.15` (no `.color` suffix) |
| FlowGraph and Workflow | FlowGraph additions below; Workflow uses the FlowGraph renderer |

Nonpainting layout/model
objects have no extra surface colors. Their visible child controls are styled
through their own families.

## Editor Additions

Add `richedit.` to each name in this table:

| Area | Keys |
| --- | --- |
| Surface and text | `text.color`, `background.color`, `border.color`, `border.focus_color` |
| Interaction | `selection.color`, `cursor.color`, `find.result_color`, `find.active_color` |
| Structured content defaults | `link.color`, `quote.border_color`, `rule.color`, `paragraph.background_color` |
| Tables | `table.border_color`, `table.header_color`, `table.cell_color` |
| Missing-image placeholder | `image.placeholder_color`, `image.border_color`, `image.text_color` |

Add `codeedit.` to these additional keys:

| Area | Keys |
| --- | --- |
| Margin | `margin.background_color`, `line_number.color`, `line_number.active_color`, `line_number.active_background_color` |
| Decorations | `marker.color`, `fold.color` |
| Signature help | `assist.active_color`, `assist.text.color`, `assist.documentation.color` |

Diagnostic margins use the same severity palette as diagnostics in the editor,
including information and hint colors. TextEdit adds
`textedit.border.disabled_color`, falling back to `input.border.disabled_color`
when the specific key is absent. Terminal adds `terminal.focus.disabled_color`.

Existing CodeEdit syntax colors remain available as `codeedit.syntax.<kind>.color`,
where `<kind>` is `keyword`, `type`, `number`, `string`, `char`, `comment`,
`preprocessor`, `operator`, `brace`, or `error`.

## FlowGraph Additions

Add `flowgraph.` to every name below. The existing background, grid, node,
selection, edge and port base colors continue to work with retained paint caches.

| Area | Keys |
| --- | --- |
| Nodes | `node.hover_color`, `node.running_color`, `node.success_color`, `node.failed_color`, `node.warning_color`, `node.diagnostic_border_color`, `node.disabled_border_color`, `node.disabled_color` |
| Run state | `run.queued_color`, `run.running_color`, `run.success_color`, `run.failed_color`, `run.skipped_color`, `run.warning_color` |
| Badges | `badge.color`, `badge.text_color`, `badge.success_color`, `badge.failed_color`, `badge.skipped_color`, `badge.warning_color`, `badge.warning_text_color` |
| Edges | `edge.active_color`, `edge.taken_color`, `edge.skipped_color`, `edge.invalid_color`, `edge.hover_color` |
| Connection preview | `connection.valid_color`, `connection.invalid_color` |
| Marquee | `selection.color`, `selection.border_color` |
| Ports | `port.disabled_color`, `port.hover_color` |
| Text and diagnostics | `text.color`, `text.disabled_color`, `text.summary_color`, `diagnostic.badge_color`, `diagnostic.text_color` |

## Maintenance

A new widget color must be registered as `XUI_STYLE_VALUE_COLOR`, resolved at
the paint boundary, and consumed by the actual painter. Testing registration
alone is insufficient. Test live changes after a warm cache, removal of the
override, alpha-zero values, explicit content precedence, and all applicable
normal/hover/active/selected/disabled/open states.

If children depend on an owner's palette, use inherited properties where that
matches the widget contract, or the private type `onPreparePaint` hook to sync
dependent colors and invalidate caches before the child walk. Do not call child
cache callbacks, `xuiWidgetUpdateBegin/End`, or clear child dirty flags manually
from an owner's cache renderer. Geometry and content indexes must not be rebuilt
to propagate a color-only change.

The framework selects the nearest type's `onPreparePaint` implementation. A
derived type that replaces this hook must explicitly preserve the base type's
paint dependencies, as MsgBox and FileDialog do for Window. Color-only backdrop
changes must not change input shielding or allocate full-viewport caches for an
invisible, alpha-zero surface.

New widgets and detached overlays may not have participated in the last global
style refresh. Their stale style generation is resolved before paint-dependency
hooks run, including on the first visible frame. Warm widgets retain their
resolved palette without repeating this resolution.

Run the new `build_style_*_test.bat` suites, the cache/lifetime tests, and
`test_xui/run_all_tests.bat` for integration verification.

## Integrated Verification

Verified on 2026-09-12 with GCC 16.1.0 on Windows x86-64, using the committed
source snapshot `3fb97b2e1ca2af34c3de4269d5d18972f3169b19`. The build and tests
ran in an isolated worktree with identical tracked contents, excluding the
main workspace's concurrent, uncommitted texture-storage changes.

| Verification | Result |
| --- | --- |
| `test_xui/run_all_tests.bat` | 167 groups passed, 0 failed, no retries |
| Native state pixel matrix | 960 verified images, result 0 |
| Native popup pixel matrix | 1280 verified images, result 0 |
| Render lifetime regression | 2092 checks, 0 failures |
| C configuration example and packaged public headers | Declaration/syntax checks passed with `-Wall -Wextra -Werror` |

The native matrices exercise DPI 1.0, 1.25, 1.5 and 2.0 with different geometry,
cache and border configurations. They are offscreen GPU/readback tests, not a
claim of manual visual inspection of every control or browser-platform testing.

The integration run also caught and resolved two issues:

- `214831b`: new and detached widgets could miss the first-frame stylesheet
  resolution. The core now resolves stale generations before prepare-paint
  hooks; CodeEdit's popup test still requires zero repaint on warm frames.
- `3fb97b2`: RichEdit's scale/fractional reference renderer still expected
  inherited colors to be baked into geometry. Its paint palette now follows
  the late-resolution contract while retaining independent geometry traversal,
  draw comparisons, explicit-content checks and layout/index performance checks.

The local build is in `build/color_styles_3fb97b2/`, with the DLL, import library,
matching public headers, SHA256 values and verification logs. The complete
runner log is also retained as `build/xui_color_styles_full_final_20260912.log`.
