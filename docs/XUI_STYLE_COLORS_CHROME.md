# XUI Chrome Color Styles

Scoped implementation and verification notes for the detached `chrome` worktree,
based on `83940262da4ca3787315ffabb51cbb5fbd7a787b`. This is a companion to the
mainline `docs/XUI_COLOR_STYLES.md`, not a claim of whole-repository coverage.

## Resolution Contract

- Color properties use the existing default, type, class, inline and token
  resolution APIs. Clearing a layer reveals the lower-priority style, then the
  control's API/base color or its documented decoration default.
- Styles do not replace the owning control's stored API colors. New decoration
  colors are resolved for painting rather than persisted as API configuration.
- Both `0` and a nonzero RGB value with alpha zero are valid transparent styles.
  Disabled-state alpha adjustment must not turn a transparent override opaque.
- Added properties invalidate `XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER`,
  never layout. Chart retains its existing additional `DIRTY_STYLE` bit.
- Create the control or obtain its type before querying its registered keys.

## Added Color Keys

Defaults below use decimal RGBA components. A fallback to another resolved color
also follows that color's runtime style changes.

| Key | Painted part | Default without this key |
| --- | --- | --- |
| `slider.knob.disabled_color` | Disabled slider knob | `(232, 237, 244, 255)` |
| `rangeslider.knob.disabled_color` | Both disabled range-slider knobs | `(232, 237, 244, 255)` |
| `virtual_joystick.knob.border_color` | Geometric fallback knob outline | `(255, 255, 255, 170)` |
| `toolbar.text.active_color` | Active item text and active overflow dots | White |
| `toolbar.icon.active_color` | Active toolbar icon tint | White |
| `toolbar.separator.highlight_color` | Separator highlight edge | `(255, 255, 255, 112)` |
| `menubar.text.active_color` | Active menu-bar label | White |
| `statusbar.text.active_color` | Active clickable status label | White |
| `tabs.text.disabled_color` | Disabled tab label | Normal text RGB with alpha 120; preserves alpha zero |
| `tabs.close.color` | Normal close glyph | Resolved normal tab text |
| `tabs.close.disabled_color` | Disabled close glyph | Normal text RGB with alpha 100; preserves alpha zero |
| `tabs.close.hover_color` | Hovered close glyph | Resolved normal close glyph |
| `tabs.close.active_color` | Pressed close glyph | Resolved hovered close glyph |
| `tabs.close.hover_background_color` | Hovered close-glyph background | `(220, 232, 246, 255)` |
| `tabs.close.active_background_color` | Pressed close-glyph background | Resolved hovered close background |
| `accordion.indicator.color` | Expand/collapse indicator | Existing border-derived default; default border selects `(47, 125, 215, 255)` |
| `carousel.indicator.background_color` | Indicator group's backing fill | `(0, 0, 0, 62)` |
| `window.close.icon_color` | Window close glyph | `(171, 72, 76, 255)` |
| `window.titlebar.topmost_color` | Topmost-window title strip | `(47, 128, 208, 255)` |
| `chart.legend.hidden_color` | Hidden-series legend marker/text | `(120, 128, 136, 120)` |
| `chart.brush.color` | Brush selection fill | `(42, 124, 221, 38)` |
| `chart.brush.border_color` | Brush selection outline | `(42, 124, 221, 180)` |
| `chart.selection.color` | Selected-point ring | `(30, 40, 52, 220)` |
| `inventory.tooltip.icon_background_color` | Inventory tooltip's icon backing fill | `(245, 249, 253, 255)` |
| `stepbar.arrow.text_color` | Text in arrow-style steps | White |
| `stepbar.check.active_color` | Current-step check in horizontal and vertical dot styles | White |

The toolbar highlight remains part of the separator: a transparent
`toolbar.separator.color` suppresses the entire separator. The joystick outline
key concerns the geometric fallback, not pixels baked into an atlas image.
The Window addition is a topmost title strip, not a new shadow implementation.

## Existing Keys With Behavioral Fixes

| Family | Fix and verification |
| --- | --- |
| `slider.disabled.color`, `slider.knob.border_color`, `rangeslider.disabled.color`, `rangeslider.knob.border_color` | Keep alpha-zero values transparent when deriving disabled-state paints |
| `scrollbar.disabled.color`, `virtual_joystick.disabled.color` | Do not restore opacity in derived disabled chrome |
| `toolbar.focus.color`, `statusbar.item.active_color` | Preserve alpha zero through existing alpha adjustments |
| `inventory.text.color`, `inventory.text.muted_color`, `inventory.hotkey.color`, `inventory.slot.quality_color` | Resolve the inventory owner's current palette inside the already-open custom tooltip; transparent default quality no longer falls back to a visible border |
| `taginput.text.color`, `taginput.placeholder.color`, `taginput.text.disabled_color` | Synchronize the child Input's base palette before its cache is prepared; preserve the child selection base color |
| `text.color`, `text.disabled_color` on Panel | Refresh inherited Header/Title styles and synchronize title Label colors before its cache is prepared |
| `panel.background.color`, `panel.header.color`, `panel.client.color`, `panel.border.color` | Existing render keys verified through override, transparency and clearing transitions |

## Cached Children And Delegation

The implementation requires the private `xui_widget_type_t::onPreparePaint`
hook from core commit `4ca291ea2976c8aba061b661255e64b8b42dcd3f`. The hook runs
before recursive child-cache preparation. Controls synchronize dependencies and
invalidate paint only; they do not draw, run layout, manually call a child's
`onCacheRender`, or manage its `UpdateBegin/UpdateEnd/ClearDirty` lifecycle.

| Owner | Dependency synchronized before painting |
| --- | --- |
| Tabs | Owned tab buttons using the parent palette |
| Accordion | Owned section/header/client chrome |
| Carousel | Indicator/arrow overlay; autoplay remains in `onUpdate` |
| Window | Client, collapse, maximize and close button caches |
| InventoryGrid | The active core tooltip widget, only when this grid owns it |
| TagInput | Child Input colors, compared before calling paint-only setters |
| Panel | Header/Title inherited style resolution and title Label colors; layout synchronization remains in the layout path |

These style transitions work with a direct `xuiRender` call after changing the
style; an intervening `xuiUpdate` is not required. Stable frames reuse the caches.

Delegated controls keep their existing style namespaces:

- TagInput selection uses `input.selection.color`, including tokens and alpha
  zero. There is deliberately no redundant `taginput.selection.color`.
  Child `input.text.color` can override the text base supplied by TagInput.
  TagInput still owns its embedded Input's base text/placeholder/disabled colors;
  it no longer resets the selection base to a hardcoded blue.
- MenuBar menus and toolbar/tab overflow menus use their Menu/Popup styles.
  Inventory split-dialog Input/NumericInput/Button/Label/Popup visuals likewise
  remain delegated. Their descriptor defaults are not new parent-level keys.
- Panel title uses the existing Label `text.color` and `text.disabled_color`.
  Its Image child keeps image-specific styling; arbitrary client content is not
  recolored by the panel chrome keys.
- Core `tooltip.background.color`, `tooltip.border.color`, and
  `tooltip.text.color` belong to the separate mainline Tooltip work. They govern
  the core frame/default text, not the Inventory custom `onPaint` content palette.
  This branch neither implements nor verifies those three core additions.

## Explicit Content Exceptions

- Chart series colors and per-point colors remain data colors. Tests retain
  explicit series `0x812345ff` and point `0xb72345ff` while chrome is overridden.
- Inventory slot quality colors explicitly supplied by the item override the
  default quality palette; item icon tint is preserved. Tests retain
  `0xb93658ff` quality and `0xa82547ff` icon tint.
- Neutral white sampling tints for supplied image surfaces are not hardcoded
  chrome. Tab/window/QRCode icons, Panel images and explicit Progress nine-patch
  content are not repainted with unrelated parent chrome colors.
- User-provided client widgets, carousel pages and custom paint callbacks retain
  their content ownership. Parent cache invalidation does not rewrite their data.

## Other Files Reviewed

The following files were read but not modified by this branch. The listed keys
are a source inventory, not additional assertions in `xui_style_chrome_test`.

| Source | Existing color entry points |
| --- | --- |
| `src/xui_breadcrumb.c` | `breadcrumb.text.color`, `breadcrumb.text.hover_color`, `breadcrumb.text.active_color`, `breadcrumb.text.disabled_color`, `breadcrumb.separator.color`, `breadcrumb.background.color` |
| `src/xui_page.c` | `page.background.color`, `page.border.color`, `page.text.color`, `page.hover.color`, `page.active.color`, `page.current.color`, `page.current_text.color`, `page.disabled_text.color`, `page.focus.color` |
| `src/xui_progress.c` | `text.color`, `progress.track_color`, `progress.fill_color`, `progress.text_color`, `progress.fill_text_color` |
| `src/xui_qrcode.c` | `qrcode.foreground_color`, `qrcode.background_color` |
| `src/xui_separator.c` | `separator.color` |
| `src/xui_split_layout.c` | `splitlayout.divider.color`, `splitlayout.divider.hover_color`, `splitlayout.divider.active_color`, `splitlayout.shadow.color` |
| `src/xui_window_frame.c` | Geometry helper; no independent chrome painter or color namespace |

Mainline also synchronizes SplitLayout divider caches and active SplitLayout/
Window drag previews in the prepare-paint stage. The existing color keys work
after caches are warm, including owner-only inline styles and API base changes.
The private adorner color setter changes only paint state, retaining geometry,
input capture and overlay ordering. DockPanel uses this same color-only path.
`build_style_drag_test.bat` covers both split orientations and window move/resize
previews, transparency, removal of styles, warm reuse and unchanged geometry.

## Verification

From Windows PowerShell in the repository/worktree root:

```powershell
.\test_xui\build_style_chrome_test.bat
```

The wrapper can also be called from `test_xui` as
`.\build_style_chrome_test.bat`. It resolves its own root with `pushd`, restores
the caller's directory with `popd`, preserves compiler/test exit codes, and does
not run an older executable if compilation fails.

- Latest detached-worktree result: `xui_style_chrome_test: 5194 checks, 0 failures`.
- Compiler configuration: GCC, `-O1 -g -Wall -Wextra -Werror`, with existing
  unused-parameter, unused-function and cast-function-type exclusions.
- The test includes 16 owned control implementations exactly once and links 23
  supporting XUI sources separately, including `xui_accessibility.c` and the
  core prepare-paint hook. It does not consume a prebuilt mainline XGE DLL.
- All 26 added color keys have `chrome_key` tests. They execute installed paint
  callbacks and record colors submitted to the renderer for default/type/class/
  inline/token transitions, live token replacement, zero and RGB-with-zero-alpha,
  clearing each layer, and paint-only dirty flags. These are draw-submission
  assertions, not a GPU framebuffer/screenshot comparison.
- Cached-child tests first create real cache surfaces, change styles, render
  without Update, check the new paint, check a stable cache-reuse frame, switch
  the token again, make it transparent, and clear the override. They also check
  that owner and child do not acquire layout dirty flags.
- Inventory tooltip tests open the real tooltip using pointer input and its
  initial timer; subsequent runtime color changes need no Update.
- TagInput/Panel disabled-color tests disable the actual Input/Label, whose
  painters read enabled state rather than an artificial cache-state argument.
- Wrapper failure path: an invalid process-local `GCC_EXEC_PREFIX` produced
  compiler exit `1`; no `xui_style_chrome_test:` output followed, proving that the
  stale executable was not run. The environment was restored after the check.
- `git diff --check` is part of the closeout. Whole-mainline regression remains
  with the parent integration task and is not claimed by this focused test.

## Integration Order

The parent reports all control changes through `86b0b0c` already integrated into
mainline `583d7d2b74df8fae246890431032e2d915b8db93`. Do not apply them twice.
Only the final wrapper/documentation closeout commit needs adding to that state.
The complete prior detached-worktree sequence is retained here for traceability:

| Commit | Scope |
| --- | --- |
| `c9051fe2a7363faa018be9651964e16d0e5c7d72` | Slider, RangeSlider, ScrollBar, VirtualJoystick; focused test/wrapper |
| `e951283633e606f4191cf9859f5215df4301d4e6` | Toolbar, MenuBar, StatusBar |
| `d79ad80882b23cc74dd8d558a09b3188e4a51367` | Tabs, Accordion, Carousel, Window decorations and initial dependency refresh |
| `bf3bef05a6e6c3585c41b4d22b85eec04b565d52` | Chart |
| `6b2e7fb36467b31a6e367739cbebb0216e4278ce` | Inventory tooltip palette and refresh |
| `f47261442509016d22e59c547d848f6fee23f9ea` | Imported core hook; equivalent to parent `4ca291ea2976c8aba061b661255e64b8b42dcd3f`, not a second core change |
| `6d72252f0453aaef39351bb17b2868d0433c552f` | Replace initial Update-based color dependency refresh with prepare-paint hooks |
| `8ee214654dbb2dd78f6896305e6c9aaa060efaa3` | StepBar |
| `ba3f62d35921457559fd9f375f9a286411fc9c34` | TagInput child color synchronization and selection base preservation |
| `86b0b0cf8c4f3e3600b703014834ba694daa746b` | Panel title palette synchronization and focused assertions |

The final delivery response supplies the new closeout commit hash, which cannot
be embedded in the document created by that same commit. No changes are pushed.
