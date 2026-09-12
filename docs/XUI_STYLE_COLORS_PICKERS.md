# Picker Color Styles

This audit covers `src/xui_combobox.c`, `src/xui_cascader.c`,
`src/xui_color_picker.c`, `src/xui_date_picker.c`, and
`src/xui_icon_picker.c`. It adds 91 color properties and completes coverage of
133 picker-owned color properties. Geometry and font properties are not included
in these counts.

| Control | Property prefix | Total colors | Added colors |
| --- | --- | ---: | ---: |
| ComboBox | `combobox` | 22 | 11 |
| Cascader | `cascader` | 26 | 17 |
| ColorPicker | `colorpicker` | 32 | 21 |
| DatePicker | `datepicker` | 32 | 21 |
| IconPicker | `iconpicker` | 21 | 21 |

## Resolution And Compatibility

- All listed properties accept `XUI_STYLE_VALUE_COLOR`, including `0u` and
  nonzero RGB with zero alpha, such as `0xabcdef00u`. A transparent style is an
  explicit override, not a request to fall back to the default.
- Resolution uses the existing default, type, ordered class, inline, and token
  machinery. For a single class the tested precedence is inline > class > type
  > default > API/base value.
- A local resolved copy is used for painting. Picker getters continue to report
  API/base colors. Setters still change those bases while a style is active;
  removing the style reveals the latest base, not a saved older value.
- Existing public descriptors, signatures, and descriptor-zero conventions are
  unchanged. Use styles or the existing setters when an explicit transparent
  value is required.
- Color registration uses exactly `XUI_WIDGET_DIRTY_CACHE |
  XUI_WIDGET_DIRTY_RENDER`, never layout invalidation. Existing font and metric
  properties retain their separate layout behavior.
- Default chrome colors retain their previous values. Legacy alpha-derived
  borders, clear marks, and muted marks retain their normal appearance, but
  never turn an alpha-zero source back into visible ink.

## Common Owner Keys

For each of the five prefixes above, append a dot and each listed suffix.
For example, `combobox.button.color`, `combobox.button.hover_color`, and
`combobox.button.open_color` style the three ComboBox button states.

| Suffixes | Painted part |
| --- | --- |
| `text.color`, `text.disabled_color` | Value text, enabled and disabled |
| `background.color`, `background.hover_color`, `background.open_color`, `background.disabled_color` | Owner background states |
| `border.color`, `border.hover_color`, `border.focus_color` | Owner border states |
| `arrow.color`, `arrow.disabled_color` | Chevron states |
| `button.color`, `button.hover_color`, `button.open_color` | Drop-down button states |

These are 14 keys per control. Existing state priority is preserved. ComboBox,
Cascader, and ColorPicker derive disabled borders from `border.color`.
Cascader's clear mark derives from `arrow.color`; ColorPicker's swatch border
derives from the effective owner border. DatePicker additionally has an explicit
disabled button key below. The other four retain their existing disabled button
behavior.

## ComboBox Keys

In addition to the 14 common keys:

| Full keys | Painted part |
| --- | --- |
| `combobox.popup.panel_color`, `combobox.popup.border_color`, `combobox.popup.shadow_color` | Open menu and popup shell |
| `combobox.popup.hover_color` | Hovered menu row |
| `combobox.popup.text_color`, `combobox.popup.hover_text_color`, `combobox.popup.disabled_text_color` | Menu label states |
| `combobox.popup.separator_color` | Menu separator |

The existing menu API receives the resolved popup colors. Menu shortcut ink
derives from popup text with its existing alpha, danger ink from popup text,
selection marks from `combobox.arrow.color`, and focus from
`combobox.border.focus_color`. Menu-owned styles can override these forwarded
base values. The Menu then forwards its resolved shell palette to Popup, whose
own styles remain applicable.

Editable ComboBox text and disabled text are synchronized into its cached Input
child. Input backgrounds and borders remain transparent so the owner supplies
the chrome. Input placeholder, selection, and cursor base colors are preserved;
their independent styling remains delegated to Input. Color synchronization is
separate from the existing font, visibility, and geometry synchronization.

## Cascader Keys

In addition to the 14 common keys:

| Full keys | Painted part |
| --- | --- |
| `cascader.placeholder.color` | Empty, enabled owner text; disabled owners use `cascader.text.disabled_color` |
| `cascader.popup.panel_color`, `cascader.popup.border_color`, `cascader.popup.shadow_color` | Popup shell and column panel |
| `cascader.popup.text_color`, `cascader.popup.muted_text_color` | Normal and secondary column text and marks |
| `cascader.popup.hover_color`, `cascader.popup.active_color`, `cascader.popup.selected_color` | Hovered, active-path, and selected-path row fills |
| `cascader.popup.active_text_color`, `cascader.popup.disabled_text_color` | Active and disabled row text |
| `cascader.popup.separator_color` | Column separators |

The lightweight column scroll indicator derives from `popup.muted_text_color`
with its existing alpha. Popup shell colors are passed through `xuiPopupSetColors`;
the private column panel is invalidated when its owner's style changes. Selected
values, paths, and labels are not modified by color styling.

## ColorPicker Keys

In addition to the 14 common keys:

| Full keys | Painted part |
| --- | --- |
| `colorpicker.popup.panel_color`, `colorpicker.popup.border_color`, `colorpicker.popup.shadow_color` | Popup shell and panel |
| `colorpicker.popup.text_color`, `colorpicker.popup.muted_text_color` | Editor values and secondary labels |
| `colorpicker.accent.color` | Active controls and palette outlines |
| `colorpicker.field.color`, `colorpicker.field.border_color`, `colorpicker.field.error_border_color` | Channel and hex field fill, normal border, and error border |
| `colorpicker.separator.color` | Palette separator |
| `colorpicker.selection.color`, `colorpicker.selection.text_color` | Channel and hex text selection |
| `colorpicker.track.color`, `colorpicker.knob.color` | Channel slider chrome |
| `colorpicker.marker.color`, `colorpicker.marker.border_color` | SV/hue markers |
| `colorpicker.checker.light_color`, `colorpicker.checker.dark_color` | Transparency checkerboard |

Popup shell colors are forwarded through Popup's API; the private editor panel
is invalidated separately. Checkerboard squares and the color swatch overlay are
painted separately so transparent checker styles are not flattened to opaque
colors.

The selected/current/old RGBA color, user-supplied palette entries, and HSV/hue
gradient colors are content. They are deliberately not replaced by theme keys.
The black and white endpoints inside the SV gradient describe the color domain,
not unthemed chrome. The built-in palette is also content.

## DatePicker Keys

In addition to the 14 common keys:

| Full keys | Painted part |
| --- | --- |
| `datepicker.button.disabled_color` | Disabled drop-down button |
| `datepicker.popup.panel_color`, `datepicker.popup.border_color`, `datepicker.popup.shadow_color` | Popup shell and panel |
| `datepicker.popup.text_color`, `datepicker.popup.muted_text_color` | Main text, weekdays, and secondary labels |
| `datepicker.accent.color` | Selected days/options and active controls |
| `datepicker.field.color`, `datepicker.field.border_color`, `datepicker.field.error_border_color` | Year/time field fill, normal border, and error border |
| `datepicker.day.selected_text_color`, `datepicker.day.disabled_color`, `datepicker.day.range_color` | Selected and disabled day text; range fill |
| `datepicker.separator.color` | Calendar/footer separators |
| `datepicker.calendar.color`, `datepicker.time.color` | Calendar and time-panel backgrounds |
| `datepicker.selection.color` | Year/time text selection |
| `datepicker.option.hover_color` | Hovered year/month option |

Popup shell colors are forwarded through Popup's API; the private calendar/time
panel is invalidated separately. Date values, limits, and ranges are preserved.
The four button defaults intentionally match the previous constant appearance.

## IconPicker Keys

In addition to the 14 common keys:

| Full keys | Painted part |
| --- | --- |
| `iconpicker.placeholder.color` | Empty, enabled owner text |
| `iconpicker.popup.panel_color`, `iconpicker.popup.border_color`, `iconpicker.popup.shadow_color` | Popup shell, ScrollFrame background, and viewport background |
| `iconpicker.item.hover_color`, `iconpicker.item.selected_color`, `iconpicker.item.focus_color` | Grid cell hover/selection fills and focus outline |

The resolved panel color is forwarded to Popup and the nested ScrollFrame. The
private grid viewport is also invalidated, so an already-open grid changes in
the same frame. ScrollFrame and scrollbar chrome remain delegated to those
controls' own APIs/styles, not duplicated as IconPicker properties.

Icon assets are explicit content: their SVG fill/stroke or bitmap colors are not
recolored by the picker theme. The existing neutral-white enabled tint and
half-alpha disabled tint are preserved. Selection IDs and category data are
unchanged. `font.name` and `iconpicker.border.width` use the existing style
patterns but are not color keys.

## Same-Frame Preparation

All five types register the private `onPreparePaint` dependency hook. After
layout and before recursive child-cache preparation, it resolves and synchronizes
colors and invalidates dependent caches. It does not paint, call child cache
callbacks, begin manual cache updates, or perform layout. Style-hash checks and
color comparisons avoid repeated work on warm frames. ComboBox also checks owner
cache dirtiness so a public base-color setter reaches its Input child even when
the style hash is unchanged.

Menu/Popup color forwarding is paint-only and updates the active popup. See
[the global color reference](XUI_COLOR_STYLES.md) for the shared dependency
contract and integrated verification results.

## Verification

Entry point: `test_xui/build_style_pickers_test.bat`. With no arguments it runs
all five picker cases and is discovered automatically by the existing
`test_xui/run_all_tests.bat` glob, `build_*_test.bat`.

Run from the repository root in Windows PowerShell with GCC on `PATH`:

```powershell
.\test_xui\build_style_pickers_test.bat
.\test_xui\build_style_pickers_test.bat -Legacy
```

The optional `-OverlayRef` comparison argument compiles only `src/xui_menu.c`
and `src/xui_popup.c` from a specified Git revision via compiler standard input.
All other sources, including core, come from the current checkout. The
no-argument command tests the combined current sources and is the integration
entry point. `-Family combobox` (or another family name from
the source filenames) narrows a local run. `-Legacy` runs the five existing
behavioral suites instead of the new style suites.

The builder compiles core from source, including `src/xui_accessibility.c`, and
does not link a prebuilt XUI DLL. Objects are stored under `build/style_pickers`;
source/common-header timestamp changes rebuild them. Each test includes its
owned control source to inspect base/effective fields and arrange focused states.
Actual production painters execute through the real cache-preparation path and
the test proxy records their colors and target cache surfaces. These are draw
command/cache tests, not GPU pixel or screenshot tests.

Coverage includes:

- All 133 color keys: registration flags, effective resolution, default/type/
  class/inline-token transitions, literal zero and nonzero-RGB alpha zero, and
  clearing each layer back to the unchanged base.
- Actual paints for every color key, including normal/hover/open/focus/disabled
  owner states and already-open popup states, transparent transitions, and base
  restoration. API setters under overrides are exercised as well.
- Menu row states/separators, editable ComboBox Input caches, Cascader active/
  selected/disabled columns, ColorPicker channel/hex errors and selection,
  DatePicker ranges/disabled days/year/month options/time selection, and
  IconPicker grid states and nested ScrollFrame background.
- Direct `xuiRenderPrepare` calls without `xuiUpdate`, with assertions on the
  intended dependent cache surface for default/type/class/inline/token changes
  and clearing. Modern Menu runs additionally require warm preparation to emit
  zero extra draw commands and no dependent layout dirtiness.
- Explicit custom-palette RGBA, transparent selected color, icon SVG path color,
  and selection/range preservation.

Verified with GCC 16.1.0 on Windows in the detached picker worktree, with
picker implementation revision `811c2fd` (not the latest main checkout):

| Suite | ComboBox | Cascader | ColorPicker | DatePicker | IconPicker | Total |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Local baseline Menu, default entry | 1367 | 1527 | 1890 | 1864 | 1267 | 7915 |
| Combined `-OverlayRef cd8278c` | 1380 | 1529 | 1892 | 1866 | 1269 | 7936 |

All checks passed. The combined `-Legacy` run also passed all five existing
picker suites. The baseline omits the assertions specific to Menu's paint-only
setter/warm-cache integration. `git diff --check` passed. Repository-wide
regression is intentionally left to the main integration run.

On 2026-09-12, both style-suite entry commands above were rerun with the same
passing counts. A `-Family combobox -Legacy -OverlayRef cd8278c` invocation from
the `test_xui` directory also passed, verifying argument forwarding and
working-directory independence. An invalid family returned a nonzero exit code
through the BAT wrapper. The runner's discovery glob and documentation coverage
of all 133 resolved color keys were checked without running the global suite.

These implementation and test changes are integrated into mainline; no extra
control or core patch is required. The default command uses current sources.
