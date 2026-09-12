# Basic Control Color Styles

This document covers Checkbox, Radio, Toggle, Input, NumericInput, Canvas,
Button, CheckCard, Label, Hyperlink, Image, and the Icon drawing resource.
The CheckCard implementation filename is `src/xui_check_card.c`.

## Color Contract

- Colors use XUI's `0xRRGGBBAA` representation and `XUI_STYLE_VALUE_COLOR`.
- The listed keys use the existing default, type, class, inline, and token
  resolution APIs. Color property dirty flags are
  `XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER`, without layout dirtiness.
- Styles are resolved into temporary paint values. They do not replace stored
  descriptor/API base colors. Removing a style exposes the lower-priority
  style, then the stored base value when all overrides are removed.
- Both `0` and a value such as `0x12345600` are transparent style colors, not
  missing-property sentinels. Semitransparent styles retain their exact alpha.
  Constructor fields that historically treat zero as unspecified keep that
  constructor convention; it does not apply to stylesheet values.
- Different alias names remain distinct properties. The alias priorities
  below apply after each individual key resolves through the cascade, preserving
  the controls' existing conventions rather than inventing a merged alias
  cascade.

In the following tables, append each suffix to the exact prefix. For example,
`choice.background.` plus `active_color` is
`choice.background.active_color`. Lists do not imply unlisted state keys.

## Checkbox, Radio, and Toggle

| Controls | Prefix | Color suffixes |
| --- | --- | --- |
| All three | `choice.text.` | `color`, `disabled_color` |
| All three | `text.` | `color`, `disabled_color` |
| All three | `choice.accent.` | `color`, `hover_color`, `active_color` |
| All three | `choice.focus.` | `color` |
| Checkbox, Radio | `choice.background.` | `color`, `hover_color`, `active_color`, `disabled_color` |
| Checkbox, Radio | `choice.border.` | `color`, `hover_color`, `disabled_color` |
| Checkbox | `checkbox.check.` | `color`, `disabled_color` |
| Radio | `radio.dot.` | `color`, `disabled_color` |
| Toggle | `toggle.track.` | `color`, `hover_color`, `active_color`, `border_color`, `disabled_color`, `disabled_border_color` |
| Toggle | `toggle.thumb.` | `color`, `border_color`, `disabled_color`, `disabled_border_color` |
| Toggle | `toggle.inner_text.` | `unchecked_color`, `checked_color` |

`text.color` and `text.disabled_color` take precedence over their corresponding
`choice.text.*` aliases. The shared `choice.*` names are preserved.
`choice.background.*` is the unchecked indicator fill; checked enabled visuals
use `choice.accent.*`. Disabled indicator fills and marks have their own keys.
Toggle checked tracks use the shared accent colors. Disabled Toggle inner text
uses the resolved disabled text color, including when prebuilding a disabled
cache slot while the live widget is enabled.

The former unchecked-active fill and white check/dot paint constants are now
resolved colors. Choice focus rings retain the default effective alpha of 160,
but no longer overwrite a supplied style alpha while painting.

Explicit state surfaces and opt-in atlas skins remain image content. Their
white draw tint is an identity multiplier, not an unthemed chrome color. The
procedural fill, border, and mark keys do not recolor those supplied skins.

## Input and NumericInput

### Input Keys

| Prefix | Color suffixes |
| --- | --- |
| `text.` | `color`, `disabled_color` |
| `input.text.` | `color`, `disabled_color` |
| `input.placeholder.` | `color` |
| `input.background.` | `color`, `hover_color`, `disabled_color` |
| `input.border.` | `color`, `hover_color`, `focus_color`, `disabled_color` |
| `input.error.` | `background_color`, `border_color` |
| `input.selection.` | `color` |
| `input.cursor.` | `color` |
| `input.decoration.` | `color`, `hover_color`, `active_color`, `disabled_color` |

`input.text.*` takes precedence over the corresponding generic `text.*` key.
Without `input.border.disabled_color`, the disabled border derives from the
resolved normal border, capping its alpha at 120. An already lower alpha,
including zero, is preserved. An explicit disabled-border style is used exactly.
This is the shared `input.border.disabled_color` key also used as a fallback by
NumericInput and by the separately maintained TextEdit implementation.

Decoration keys theme the default colors of text, texture, vector-icon,
atlas-icon, and clear-button rendering paths. A nonzero color supplied for the
decoration's current state takes precedence over the style. Zero descriptor
state colors retain their existing unspecified/default meaning. The normal,
hover, active, and disabled defaults remain unchanged. Custom decoration paint
callbacks own their drawing colors and are not rewritten by these keys.

### NumericInput Keys

| Prefix | Color suffixes |
| --- | --- |
| `numeric_input.text.` | `color`, `disabled_color` |
| `numeric_input.placeholder.` | `color` |
| `numeric_input.selection.` | `color` |
| `numeric_input.cursor.` | `color` |
| `numeric_input.background.` | `color`, `hover_color`, `disabled_color` |
| `numeric_input.border.` | `color`, `hover_color`, `focus_color`, `disabled_color` |
| `numeric_input.error.` | `background_color`, `border_color` |
| `numeric_input.spinner.` | `color`, `hover_color`, `active_color`, `border_color`, `icon_color`, `icon_disabled_color` |

The five text, placeholder, selection, and cursor colors are painted by the
private Input child. That child reads the NumericInput owner's resolved keys,
so owner type, class, inline, and token changes reach the actual text cache.
NumericInput-specific text colors override the child's generic/Input defaults.
The child's background, error background, and all borders remain transparent;
NumericInput paints the outer chrome. Global Input background styles therefore
do not create an opaque rectangle inside NumericInput.

Disabled NumericInput border priority is:

1. `numeric_input.border.disabled_color`.
2. `input.border.disabled_color`, resolved on the NumericInput owner.
3. The resolved normal NumericInput border with alpha capped at 120.

Spinner fills use `numeric_input.spinner.color` when disabled, readonly, or at
a value limit; the unavailable arrow uses `icon_disabled_color`. The divider
uses `border_color`. There is no separate disabled spinner fill field.

Base text colors are synchronized to the private Input through the existing
API setup path; resolved stylesheet colors are never baked into those bases.
The owner's private `onPreparePaint` hook compares the style hash and marks the
Input cache dirty before child-cache preparation. It does not draw, lay out,
manually update cache slots, or clear dirty flags.

## Canvas

| Key | Paint target |
| --- | --- |
| `canvas.background.color` | Viewport background behind the drawing surface |
| `canvas.border.color` | Viewport border |
| `canvas.pen.color` | Default interactive pen, unless an explicit pen color was supplied |

The viewport painter resolves background and border on the Canvas owner. Its
private `onPreparePaint` hook compares those effective colors and invalidates
the viewport before recursive cache preparation. Default, type, class, inline,
and token transitions therefore affect cached viewport visuals in the same
render call. Unchanged effective colors do not cause an extra viewport paint.
All cache states and draw-context cleanup remain the framework's responsibility.

Canvas contains a ScrollFrame and its scrollbars. Frame background, corner,
grip, scrollbar tracks/thumbs, and arrow-button chrome are delegated to those
controls, not additional `canvas.*` keys. In particular, a transparent Canvas
viewport exposes the containing frame/background; it does not clear the frame
or the drawing surface. The frame is initially given the Canvas base background
by the constructor. Canvas viewport tests distinguish that separate frame paint
from the viewport's three-state cache paints. Theme the frame through its own
control family when the entire composed surface must be transparent.

The drawing surface is persistent content, not a theme cache. Its clear color,
explicit draw-call colors, per-vertex colors, and existing pixels are not changed
when Canvas styles change. Surface composition uses white as an identity tint.
An explicit descriptor pen color, `xuiCanvasSetPen`, or `xuiCanvasSetColors`
opts the pen out of `canvas.pen.color`; API-specified transparent pen colors
remain transparent. When no explicit pen color exists, the interactive pointer
drawing path resolves the stylesheet pen at the time of each stroke operation.

## Button and CheckCard

| Control | Prefix | Color suffixes |
| --- | --- | --- |
| Button | `button.` | `text_color`, `disabled_text_color`, `icon_color` |
| Button | `text.` | `color`, `disabled_color` |
| Button | `button.` | `normal_color`, `hover_color`, `active_color`, `disabled_color`, `checked_color` |
| Button | `button.` | `border_color`, `hover_border_color`, `active_border_color`, `disabled_border_color`, `checked_border_color`, `focus_color` |
| Button | `button.badge.` | `color`, `border_color` |
| CheckCard | `checkcard.background.` | `color`, `hover_color`, `active_color`, `checked_color` |
| CheckCard | `checkcard.border.` | `color`, `hover_color`, `checked_color`, `disabled_color` |
| CheckCard | `checkcard.corner.` | `color` |
| CheckCard | `checkcard.check.` | `color` |
| CheckCard | `checkcard.focus.` | `color` |

Button generic `text.*` aliases take precedence over the corresponding
`button.*text_color` keys; their previously missing registrations are now
present even when no other text control has been created. `button.focus_color`
is the focus border color. The new badge keys affect the built-in circular
badge, not an explicitly supplied badge surface. Button icon tint remains
controlled by `button.icon_color`, including disabled rendering.

CheckCard paints only its own chrome; child labels/images retain their own
control style keys. Disabled CheckCard retains its normal or checked background
as appropriate and uses the disabled border, with checked corner/check visuals
remaining independently styled.
CheckCard keeps the legacy base focus ring's effective alpha of 150, but a
`checkcard.focus.color` style now retains its exact alpha, including zero.
The focus ring also requires a positive focus width and a focused enabled state.

## Label, Hyperlink, Image, and Icon

| Control | Prefix | Color suffixes |
| --- | --- | --- |
| Label | `text.` | `color`, `disabled_color` |
| Hyperlink | `text.` | `color`, `disabled_color` |
| Hyperlink | `hyperlink.text.` | `color`, `hover_color`, `active_color`, `disabled_color` |
| Image | `image.` | `color`, `tint` |

Hyperlink-specific text keys override the generic aliases. Underlines use the
selected text color. `image.tint` takes precedence over `image.color`. Image
has no separate disabled palette; the effective tint applies in either state.
These controls already resolved their actual paint colors; the audit adds
rendering, cascade, transparent-color, restoration, and cached-transition tests
without unnecessary source changes.

Icon (`src/xui_icon.c`) is a drawing resource/category API, not an Image widget
and not an independently styled widget. `xuiIconDraw` uses the explicit tint in
its draw descriptor, including through icon aliases. SVG path fill/stroke
colors are content multiplied by that tint; surface icons receive the tint
directly. No redundant, ineffective `icon.color` or `icon.tint` property is
registered. A global Image style cannot override an explicit Icon draw tint.

## Verification

Run from a Windows PowerShell checkout with GCC available:

```powershell
test_xui\build_style_basic_choices_test.bat
test_xui\build_style_basic_input_test.bat
test_xui\build_style_basic_canvas_test.bat
test_xui\build_style_basic_buttons_test.bat
test_xui\build_style_basic_content_test.bat
test_xui\build_prepare_paint_test.bat
```

The five family wrappers match `test_xui/run_all_tests.bat`'s dynamic
`build_*_test.bat` glob. Their shared helper is named `build_style_basic.bat`
so it is not executed again as a standalone test. The helper compiles core and
control sources directly, including the required `src/xui_accessibility.c`,
instead of relying on an old DLL. It enables `-Wall -Wextra -Werror` with the
existing targeted unused-parameter/function and cast-function-type exceptions.

Tests instrument actual installed painters and drawing-proxy calls, and use
`xuiRender` for cached runtime transitions. They are effective paint-color
tests against the test proxy, not GPU screenshots or property-registration-only
tests. Coverage includes:

- Relevant normal, hover, active, focused, checked, error, and disabled colors.
- Default/type/class/inline cascade, live tokens, exact semitransparent alpha,
  both transparent-zero encodings, and exact base restoration after clearing.
- Paint-only property dirty flags and preserved API base getters.
- NumericInput owner-to-Input text cache refresh in the same render call.
- Canvas default/class/inline changes across all three test cache slots,
  unchanged warm frames without another paint, and restoration on clearing.
- Canvas destruction during the first child paint stops remaining cache-slot
  paints and leaves every successful `drawBegin` paired with `drawEnd`.
- Input text and vector-icon decoration paths, explicit decoration colors,
  custom badge surfaces, explicit Icon tint, and persistent Canvas content.

Verified on 2026-09-12 in the basic detached worktree:

| Test wrapper family | Checks | Failures |
| --- | ---: | ---: |
| `style_basic_choices` | 3837 | 0 |
| `style_basic_input` | 3345 | 0 |
| `style_basic_canvas` | 265 | 0 |
| `style_basic_buttons` | 2402 | 0 |
| `style_basic_content` | 861 | 0 |
| `prepare_paint` | Not counted by that runner | 0 |

The expanded tables contain all 105 unique color keys registered by the scoped
control sources, with no missing or extra keys. `git diff --check` also passes.
The prepare-paint build reports the existing DatePicker `snprintf`
format-truncation warning at this baseline; that unrelated source was not
changed. Full-project results are recorded in the global color reference.

NumericInput and Canvas use the private prepare-paint hook. They do not manually
repaint child caches. See [the global color reference](XUI_COLOR_STYLES.md) for
the shared dependency contract and integrated verification results.
