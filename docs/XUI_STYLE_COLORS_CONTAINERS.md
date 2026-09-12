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
child drawing, or layout invalidation. It requires the shared core commit
`4ca291e` (cherry-picked into this worktree as `883874b`).

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

Editors retain their own `input.*`, `numericinput.*`, `combobox.*`, picker,
`textedit.*`, `popup.*`, and `button.*` style cascades. The TableGrid API palette
provides Input/ComboBox base colors instead of fixed white/blue editing chrome;
stylesheet changes never rewrite those bases. Explicit cell colors and picker
values remain content, not theme defaults. TableGrid itself has no painted
surface or paint-color dependency to synchronize.

## Verification

From the detached workspace in Windows PowerShell:

```powershell
cmd /c test_xui\build_style_containers_test.bat
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

The standalone build uses current project sources, including
`src/xui_accessibility.c`, rather than a prebuilt XGE DLL. Tests observe colors
sent to the drawing proxy after a single cache preparation pass, including
cached descendants, token changes, transparent overrides and clearing styles.
