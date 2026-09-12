# Container stylesheet colors

Color properties use the existing default, type, named, class, state-class,
inline and token cascade. They invalidate style/cache/render only. A color of
zero is a present transparent override, not a request for the default. Public
color setters/getters retain API/base colors; clearing stylesheet overrides
restores the latest base colors.

## ScrollFrame and ScrollView

- `scrollframe.background.color`, `scrollframe.corner.color`, `scrollframe.grip.color`
- `scrollview.background.color`, `scrollview.corner.color`, `scrollview.grip.color`

ScrollView colors override the corresponding colors of its owned ScrollFrame.
The frame resolves them while painting, without changing its base palette.
The ScrollView properties are inherited by its backing frame, so normal style
invalidation and cache preparation handle runtime changes without invoking
child rendering callbacks or clearing their dirty flags manually.

The private `onPreparePaint` hook resolves the backing frame when the owner's
resolved style changes, including local class/inline changes. This happens
before child cache preparation, without an explicit `xuiStyleRefresh`, manual
child drawing, or layout invalidation. The shared hook is part of the core;
see [the global color reference](XUI_COLOR_STYLES.md) for its contract.

Scrollbars delegate to the actual ScrollBar widgets returned by
`xuiScrollFrameGetHScrollBarWidget` / `xuiScrollFrameGetVScrollBarWidget` (or
the ScrollView accessors). Use `scrollbar.track.color`, `scrollbar.thumb.color`,
`scrollbar.thumb.hover_color`, `scrollbar.thumb.active_color`,
`scrollbar.focus.color`, `scrollbar.disabled.color`, `scrollbar.button.color`
and `scrollbar.button.icon_color` on the ScrollBar type or these child widgets.
No parent aliases rewrite child inline styles or base colors.

## DockPanel

The `dockpanel.*` properties cover the full API palette: `background.color`,
`pane.color`, `client.color`, `caption.color`, `caption.active_color`,
`caption.text_color`, `caption.active_text_color`, `tab.color`, `tab.hover_color`,
`tab.active_color`, `tab.text_color`, `tab.active_text_color`, `border.color`,
`focus.color`, `splitter.color`, `splitter.hover_color`, `splitter.active_color`,
`button.color`, `button.hover_color`, `button.active_color`, `auto_hide.color`,
`auto_hide.hover_color`, `float.title_color`, and `float.border_color`.

Additional chrome keys are `button.icon_color`, `button.disabled_color`,
`button.close_icon_color`, `auto_hide.border_color`, `tab.disabled_border_color`,
`tab.disabled_text_color`, `tab.indicator_color`, `drag.indicator_color`,
`drag.insert_border_color`, `drag.preview_color`, `drag.preview_border_color`,
and `drag.preview_inner_border_color`, all with the `dockpanel.` prefix.

The owner's paint palette is separate from `xuiDockPanelGetColors`. The private
paint preparation hook invalidates floating/expanded hosts, their button state
caches, and active detached drag visuals before framework cache traversal.
Content icons keep their original white modulation; built-in chrome icon tints
are themeable. Menus delegate to their actual Menu widgets and `menu.*` styles.

## TableGrid

TableGrid is a composite, not a TableView subtype. Use the real TableView
returned by `xuiTableGridGetTableView` for named/class/inline `tableview.*`
styles. Global defaults and TableView type styles apply directly to that child.
No duplicate `tablegrid.*` keys or injected child inline styles are needed.
The TableView paint preparation hook refreshes its viewport in the same pass.

Editors retain their own `input.*`, `numeric_input.*`, `combobox.*`, picker,
`textedit.*`, `popup.*`, and `button.*` style cascades. The TableGrid API palette
provides Input/ComboBox base colors instead of fixed white/blue editing chrome;
stylesheet changes never rewrite those bases. Explicit cell colors and picker
values remain content, not theme defaults. TableGrid itself has no painted
surface or paint-color dependency to synchronize.

## PropertyGrid

PropertyGrid also composes a TableGrid and TableView. Its adapter palette uses
`propertygrid.background.color`, `propertygrid.grid.color`,
`propertygrid.category.background_color`, `propertygrid.category.hover_color`,
`propertygrid.category.text_color`, `propertygrid.category.icon_color`,
`propertygrid.name.background_color`, `propertygrid.name.text_color`,
`propertygrid.name.hover_color`, `propertygrid.value.background_color`,
`propertygrid.value.text_color`, `propertygrid.selected.color`,
`propertygrid.readonly.text_color`, `propertygrid.invalid.color`, and
`propertygrid.dirty.color`.

`xuiPropertyGridGetStyle` returns the unchanged API palette. Effective colors
are stored separately, and the private paint hook resolves inherited owner
colors before the backing table is prepared. It invalidates paint caches only;
color setters no longer rebuild visible rows or trigger layout. Category row
data does not snapshot colors, so an existing viewport cache sees live changes.

The private TableView paint adapter supplies resolved background/grid colors and
explicit cell-color presence without changing public base colors. PropertyGrid
draws role backgrounds (including selected values) and invalid/dirty markers;
the table still draws ordinary content. Transparent cells therefore do not
reveal a second, stale cell background. Tables without a paint adapter retain
their existing partial-color and explicit row/column/cell precedence. The
generic adapter knows no PropertyGrid types or property names, and guards both
chrome callbacks against destruction and callback replacement.

Child inline `tableview.*` styles on `xuiPropertyGridGetTableView` take precedence
over matching role colors. Otherwise `propertygrid.*` role colors override
generic `tableview.*` colors, including simultaneous global defaults; clearing
the role restores the generic table color, then the API base if neither is set.
The adapter palette is applied after generic TableView color resolution.
Category roles map to TableView
header colors; name/value roles map to row/text colors. PropertyGrid does not
write child inline styles or replace cell content colors, including color
swatches. Its editors use the independent child style cascades described above.

## Verification

From the repository root in Windows PowerShell:

```powershell
cmd /c test_xui\build_style_containers_test.bat
cmd /c test_xui\build_style_containers_adapter_test.bat
cmd /c test_xui\build_style_containers_adapter_test.bat collection
cmd /c test_xui\build_style_containers_test.bat --regression table_view
cmd /c test_xui\build_style_containers_test.bat --regression table_view_index
cmd /c test_xui\build_style_containers_test.bat --regression table_provider_lifetime
cmd /c test_xui\build_style_containers_test.bat --regression property_grid
cmd /c test_xui\build_style_containers_test.bat --regression table_grid
```

The `scroll` test selection passes 113 checks, including one child render per
changed frame, zero child renders on warm frames, and stable layout versions.
The `dock` selection passes 261 checks, including cached floating/auto-hide
chrome, all three button states, active drag overlays, transparent/clear
transitions, original icon modulation, and unchanged layout versions.
The existing `dock_panel` regression passes; `dock_panel_pixel` passes 1688
checks with no failures when rebuilt through `--regression <name>`.
The `table` selection passes 179 checks: default/type/class/token/inline colors
reach the actual TableView viewport; Input and an open ComboBox editor react
to live styles, transparent overrides, and clearing. Raw palettes and explicit
cell colors are preserved. Warm table frames perform zero viewport redraws and
paint-only style changes leave the owner/table layout versions unchanged.
The existing `table_grid` regression also passes when rebuilt from source.

The `property` selection passes 350 checks. Each of its 15 keys is asserted
against actual drawing calls, including selected/hover states, category icons,
read-only text, generic and enum invalid/dirty markers. Tests cover same-pass
default/type/class/token/inline transitions, transparent selection and cells,
clearing to updated API bases, child inline precedence, and simultaneous
generic/role global background and grid colors. Global PropertyGrid keys do
not color an ordinary TableView. Warm frames perform zero viewport redraws;
owner/table/viewport rectangles and layout versions remain unchanged.

The full containers selection passes 903 checks with zero failures. The new
private adapter test passes 288 checks across 20 callback lifetime cases,
including self/parent/root/context destruction and adapter replacement. It
also checks unchanged API colors, row-index storage, merge generation, layout
versions, and zero warm-frame repaint. The existing TableView, index,
PropertyGrid and TableGrid regressions pass. The original provider lifetime
test passes 996 cases / 11335 checks; the collection TableView test passes all
29 color keys and its render/cache checks.

These standalone builds can report the pre-existing DatePicker `snprintf`
truncation warning in `src/xui_date_picker.c`.

The standalone build uses current project sources, including
`src/xui_accessibility.c`, rather than a prebuilt XGE DLL. Tests observe colors
sent to the drawing proxy after a single cache preparation pass, including
cached descendants, token changes, transparent overrides and clearing styles.
