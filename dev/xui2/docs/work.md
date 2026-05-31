# XUI2 Work Context

This document is a handoff snapshot for continuing the XUI2 rewrite from another machine or another conversation context. It records the design decisions, constraints, current implementation state, verification commands, and next-step expectations as of this point.

## Workspace

- Main repo: `D:\git\xge`
- XUI2 workspace: `D:\git\xge\dev\xui2`
- Public XUI2 API header: `D:\git\xge\dev\xui2\xui.h`
- XUI2 implementation files: `D:\git\xge\dev\xui2\src\`
- XUI2 tests: `D:\git\xge\dev\xui2\test_xui\`
- XUI2 examples: `D:\git\xge\dev\xui2\examples\`
- XUI2 docs: `D:\git\xge\dev\xui2\docs\`

The name `xui2` exists only as the development directory/conversation name. Public API names must still use `xui` as the prefix.

API naming style:

```c
xuiDrawBegin
xuiDrawEnd
xuiProgressCreate
xuiWidgetSetRect
```

Do not introduce `xui2` prefixes in public API, type names, constants, source identifiers, or examples unless they refer to the folder or documentation context.

## High-Level Goal

The rewrite exists to make XUI suitable for both:

- GAMEUI: game-embedded UI, frame refresh, frequent animation and interaction.
- APPUI: normal application UI, dirty rectangle refresh, low CPU/GPU cost, stable static UI rendering.

The core architectural target is a cache-first DirectUI system:

- Controls render their own visual content into one or more cache surfaces.
- XUI system rendering composites widget caches, rather than calling every control's draw logic every frame.
- Static widgets should not redraw background, text, border, image, etc. unless their own content/style/layout state changes.
- State-heavy widgets such as Button can have multiple state caches. Switching hover/active/disabled/checked state should ideally select a cache, not redraw.
- Dirty rect repaint should be able to start from root/subtree caches and composite only the required region.
- The rendering model must serve both full-frame game loops and APP-style dirty paint.

## Non-Negotiable Design Constraints

### Public API

- Public API stays in `xui.h`.
- API names use the `xui` prefix plus camel case.
- XSON is explicitly deferred until after all standard controls are migrated.
- Do not add new public proxy capabilities silently.
- If a control or infrastructure feature cannot be implemented with the current proxy contract, stop and report that the proxy must be extended.

### Compatibility Direction

- XUI2 is not a patch on XUI1. It is a clean rewrite under `dev\xui2`.
- XUI1 code and docs are references for behavior and feature coverage.
- Do not copy XUI1 pixel-for-pixel implementation style if XUI2 infrastructure provides a cleaner approach.
- Standard controls should match XUI1 feature expectations unless XUI2 architecture explicitly changes the implementation route.

### Editing And Validation

- Use `apply_patch` for manual file edits.
- Do not revert unrelated dirty worktree changes.
- `git diff --check` should pass before handoff.
- Examples must support `--frames N` and `--seconds N`.
- XUI examples must not auto-exit when run without duration arguments.

## Proxy Layer Design

XUI2 has a proxy layer, not a platform host layer.

XUI does not own:

- windows
- system DPI discovery
- platform event loops
- system time
- native message pumps

XRT is a required base library and provides time/common foundation functions where needed.

The proxy layer provides only capabilities that XUI cannot do without:

System-level proxy capabilities:

- clipboard text get/set
- IME enable/disable
- IME candidate rect

Rendering proxy capabilities:

- surface creation/destruction/update/read
- surface load from file/memory
- surface to surface draw
- surface quad draw
- sampler get/set
- draw context begin/end
- shape draw to surface/draw context
- font load/destroy/metrics
- text measure/draw
- clear full surface or rect

Input is not proxy-owned:

- mouse/key/text/DPI/viewport/refresh data enters XUI through `xuiInput...`, `xuiDispatch...`, `xuiUpdate`, `xuiRender`, etc.
- The backend program or adapter decides how native events map to XUI input calls.

Important proxy rule:

- Future work must not add proxy functions casually.
- If a control requires a missing proxy feature, report it and stop instead of adding an ad hoc backend hook.

Current proxy-specific examples/tests already exist for surface, shape, font, input, etc.

## Surface And Clipping Model

Surface is the central rendering abstraction:

- A surface can be a texture-like resource, render target, or offscreen cache.
- Most XUI rendering should become surface-to-surface composition.
- Nine-patch rendering is surface slicing/composition.
- APP dirty paint is essentially copying a dirty region from a root/subtree cache to the target backend surface/window.

Clip/scissor was intentionally not added as a first-class proxy requirement.

Reasoning:

- Widgets render into finite cache surfaces.
- Overflow outside the cache is naturally clipped.
- Most widget-level clipping can be represented as source/destination rectangle composition.

Known limitation:

- Arbitrary inner text clipping, such as XUI1 Progress drawing a second text color clipped to a partial fill rect, is not exactly expressible without clip/scissor or a more advanced text layer.
- The current Progress implementation avoids adding proxy clip. It draws fill-colored text only when the fill rect fully contains the measured visual text rect. This avoids visibly incorrect centered text in a half-width fill region.
- If exact partial glyph clipping becomes mandatory, it should be solved in the XUI text/cache layer or by formally extending proxy capability, not by local hacks.

Pixel format:

- XUI memory/surface path currently follows `XUI_SURFACE_FORMAT_RGBA8`.
- Proxy caps currently indicate premultiplied internal alpha where applicable.

## Pixel Snapping And Integer Geometry

XUI1 had recurring issues caused by `0.5` coordinates:

- lines could disappear
- 1px lines could become 2px
- borders could start visually at `1,1` instead of `0,0`

XUI2 must keep the pixel snapping fix as a core invariant:

- Layout may use float during measurement/allocation.
- Final widget rect/content rect/cache draw rect should land on integer pixel edges.
- Do not independently round every child width.
- Shared remaining space must be distributed by snapping cumulative boundaries.

Example: `29px / 3`

```text
ideal edges: 0, 9.666, 19.333, 29
pixel edges: 0, 10, 19, 29
widths:      10, 9, 10
```

This avoids overflow and gaps:

- not `10 + 10 + 10 = 30`
- total must remain exactly `29`

The reusable snapping helpers are in `src\xui_internal.h`, including:

- `xuiInternalSnapPixel`
- `xuiInternalSnapSize`
- `xuiInternalSnapRect`
- `xuiInternalSnapRectOut`
- `xuiInternalSnapRectIn`

Controls should not reimplement their own pixel snapping rules unless there is a very specific reason.

## Layout Direction

XUI2 layout should prioritize APP UI, then document UI.

Core layout philosophy:

- APP layout should stay easy and explicit.
- Row and column are kept as separate mental models, not collapsed into one `linear` name.
- Flow/document layout is added for HTML-like document flow, inline/block mixing, and auto wrapping.
- Heavy browser layout features should not be copied wholesale.
- Complex app-level layout controls such as split/dock/viewport may exist outside the widget core as controls.

Current core layout concepts:

- manual
- overlay/stack
- row
- column
- flow/document
- table
- dock
- grid

Important distinction:

- `grid` is a lightweight APP grid for fixed or simple repeated layout.
- `table` is the size-negotiation layout for rows/columns, spans, and min/max/content/star-like negotiation.
- Do not force CSS Grid complexity into the base grid.

Flow/document layout:

- exists because APP layout alone is too rigid for document-like content.
- supports div/span-like mental model through flow modes.
- should be treated as first-class, but scroll/viewport behavior belongs to viewport/scroll controls, not every widget.

Widget-level flow-related modes already defined:

- `XUI_FLOW_BLOCK`
- `XUI_FLOW_INLINE`
- `XUI_FLOW_INLINE_BLOCK`
- `XUI_FLOW_NONE`
- `XUI_FLOW_ABSOLUTE`

## Style System Direction

XUI1 style was fragmented:

- XSON named style inheritance existed.
- Widget runtime did not have CSS-like parent cascade.
- C API style was mostly direct struct copy/state overrides.
- Control-private style fields were scattered.

XUI2 style direction:

- Use XSON/JSON-compatible data later, not a separate CSS parser.
- Build a Style Manager rather than a full browser CSS engine.
- Support style class, inheritance, default style, inline style, state-driven invalidation, and dynamic skin/theme switching.
- Runtime hot path should read resolved/cached style, not repeatedly parse documents or rules.

Current style API/infrastructure exists in `xui_widget.c` and `xui.h`, including:

- style rules
- style classes
- inline styles
- style tokens/default style
- registered style properties
- dirty flags per style property
- inherited properties

Current practical control pattern:

- Register style properties from the control type registration function.
- Resolve style values inside cache render/content measure using `xuiWidgetGetResolvedStyleProperty`.
- Use style property dirty flags to invalidate layout/cache/render correctly.

XSON remains deferred:

- Do not implement XSON per-control while controls are still being migrated.
- After all standard controls are migrated, implement XSON loading in one coherent pass.

## Current Base Infrastructure Status

The following XUI2 infrastructure is considered present enough for control migration:

- Context
- Widget base
- Widget type system
- Layout and content measure callbacks
- Core layout system
- Input layer
- Text layout layer
- Render scheduling
- Cache strategy layer
- Render tree/composition layer
- Painter/proxy drawing support
- Style system
- Default/global style direction
- Resource management foundation
- Tooltip mechanism
- Test proxy used by control tests

Remaining larger systems intentionally deferred:

- XSON loader
- higher-level text-edit integrations such as visible scrollbar modes and model-backed rich editing
- complex controls that depend on text editing/data models
- final complete standard control library

## Control Migration Standard

For each migrated control:

1. Read XUI1 docs/code to understand behavior and feature coverage.
2. Rewrite for XUI2 infrastructure rather than copying old implementation shape.
3. Render into widget cache using proxy draw/surface functions.
4. Register relevant style properties.
5. Add focused test in `test_xui`.
6. Add code-only example in `examples`.
7. Add control document in `docs`.
8. Keep XSON integration deferred.
9. Run the control test, build the example, run `--frames` smoke test, and verify no-args run stays alive.

Controls already migrated or substantially present:

- Label
- Button
- Image
- Separator
- Progress
- Checkbox
- Radio / RadioGroup
- Toggle
- ScrollBar
- Slider
- ScrollModel / ScrollFrame / ScrollView
- Popup
- Menu
- MenuBar
- Toolbar
- StatusBar
- ComboBox
- ColorPicker
- DatePicker
- Panel
- SplitLayout
- Tabs
- Accordion
- Window
- DockPanel
- Input
- NumericInput
- TextEdit
- Tooltip infrastructure and example
- Input event-layer example/infrastructure

## Label Notes

Label was used to validate XUI2 text/layout/cache behavior.

Important points:

- Supports text, font, colors, flags, wrap, underline, line gap, paragraph gap.
- Uses XUI2 text layout.
- Uses cache render.
- Pixel snapping issue was fixed in infrastructure rather than inside Label-specific code.
- Label example was used to confirm edge/border snapping after the integer allocation fix.

## Button Notes

Button migration covered:

- text
- font
- icon and icon placement
- normal/hover/active/focus/disabled visuals
- selectable/checked state
- state cache behavior
- badge
- nine-patch state visuals

Important bug already fixed:

- Button disappeared after click/mouse-out because active/cache state handling was wrong.
- Current tests/examples passed after fix.

## Image Notes

Image migration covered:

- surface ownership boundary: control does not own surface
- source rect
- tint/color modulation
- draw modes: natural/stretch/contain/cover/scale-down/custom
- align
- content measure from source/surface natural size

Example demonstrates:

- from file/memory/RGBA-created surfaces
- source rect
- mixed rendering modes
- offscreen render path

## Separator Notes

Separator migration covered:

- horizontal/vertical
- solid/dot/dash/dash-dot
- color
- thickness
- align
- line rect query

Public API includes:

- `xuiSeparatorCreate`
- `xuiSeparatorSetColor`
- `xuiSeparatorSetThickness`
- `xuiSeparatorSetOrientation`
- `xuiSeparatorSetAlign`
- `xuiSeparatorSetLineStyle`
- `xuiSeparatorGetLineRect`

Files:

- `src\xui_separator.c`
- `test_xui\xui_separator_test.c`
- `examples\xui_separator\main.c`
- `docs\widget-separator.md`

## Progress Notes

Progress migration covered cache-first range/fill rendering and text display.

Public constants in `xui.h`:

- `XUI_PROGRESS_LEFT_TO_RIGHT`
- `XUI_PROGRESS_RIGHT_TO_LEFT`
- `XUI_PROGRESS_BOTTOM_TO_TOP`
- `XUI_PROGRESS_TOP_TO_BOTTOM`
- `XUI_PROGRESS_FILL_STRETCH`
- `XUI_PROGRESS_FILL_REVEAL`

Public descriptor:

- `xui_progress_desc_t`

Public APIs include:

- `xuiProgressGetType`
- `xuiProgressCreate`
- `xuiProgressSetRange`
- `xuiProgressGetRange`
- `xuiProgressSetValue`
- `xuiProgressGetValue`
- `xuiProgressGetRate`
- `xuiProgressSetText`
- `xuiProgressSetTextTemplate`
- `xuiProgressGetTextTemplate`
- `xuiProgressGetDisplayText`
- `xuiProgressSetFont`
- `xuiProgressGetFont`
- `xuiProgressSetTextColor`
- `xuiProgressGetTextColor`
- `xuiProgressSetFillTextColor`
- `xuiProgressGetFillTextColor`
- `xuiProgressSetTextFlags`
- `xuiProgressGetTextFlags`
- `xuiProgressSetColors`
- `xuiProgressGetTrackColor`
- `xuiProgressGetFillColor`
- `xuiProgressSetFillDirection`
- `xuiProgressGetFillDirection`
- `xuiProgressSetTrackPatch`
- `xuiProgressClearTrackPatch`
- `xuiProgressHasTrackPatch`
- `xuiProgressSetFillPatch`
- `xuiProgressClearFillPatch`
- `xuiProgressHasFillPatch`
- `xuiProgressSetFillPatchMode`
- `xuiProgressGetFillPatchMode`
- `xuiProgressGetFillRect`

Implementation file:

- `src\xui_progress.c`

Progress behavior aligned to XUI1:

- default range: `0..1`
- default value: `0`
- swapped range is normalized
- equal range becomes `min..min+1`
- value clamps to range
- dynamic text template detects `%` not escaped by `%%`
- dynamic text formats percent value (`rate * 100`)
- static text is copied as-is
- no text when template is `NULL`
- default horizontal measure: `120x12`
- default vertical measure: `12x120`
- track color default: `RGBA(216,236,248,255)`
- fill color default: `RGBA(46,124,214,255)`
- text color default: theme text or `RGBA(36,52,70,255)`
- fill text color default: white
- text flags default: center/middle/clip
- fill direction default: left-to-right
- fill patch mode default: stretch

Progress style properties:

- `progress.track_color`
- `progress.fill_color`
- `progress.text_color`
- `progress.fill_text_color`
- `progress.text_flags`
- `progress.fill_direction`
- `progress.fill_patch_mode`
- inherited `text.color`
- inherited `text.flags`
- inherited `font.name`

Progress rendering details:

- Draw track patch if present, otherwise track rect if alpha > 0.
- Compute fill rect from content rect and current rate/direction.
- Draw fill patch if present, otherwise fill rect if alpha > 0.
- Draw base text over content rect.
- Draw fill-colored text only when fill rect fully contains the measured text visual rect. This avoids a wrong-looking second centered text when proxy clip is unavailable.
- Simple reveal mode (`slice == 0`) crops source rect according to rate/direction.
- Sliced nine-patch with reveal mode falls back to drawing the patch into current fill rect, since exact source reveal for sliced nine-patch is not well-defined without deeper nine-patch clipping logic.

Progress files:

- `src\xui_progress.c`
- `test_xui\xui_progress_test.c`
- `test_xui\build_progress_test.bat`
- `examples\xui_progress\main.c`
- `examples\xui_progress\build.bat`
- `docs\widget-progress.md`

Progress verification already passed:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_progress_test.bat
examples\xui_progress\build.bat
build\xui_progress.exe --frames 3
```

No-args behavior was checked:

- `build\xui_progress.exe` without duration stays alive and does not auto-exit.

Regression tests also passed after shared test proxy changes:

```bat
test_xui\build_button_test.bat
test_xui\build_image_test.bat
test_xui\build_separator_test.bat
```

## Shared Test Proxy Notes

Shared test proxy files:

- `test_xui\xui_test_proxy.h`
- `test_xui\xui_test_proxy.c`

Recent enhancement:

- Added simple test font creation through proxy `fontLoadFile/fontLoadMemory`.
- Added basic text measure using `strlen * font_size * 0.5`.
- Added text draw counting and last text rect/color tracking.

New helper APIs:

- `xuiTestSurfaceGetTextDrawCount`
- `xuiTestSurfaceGetLastTextRect`
- `xuiTestSurfaceGetLastTextColor`

This change was regression-tested with Button/Image/Separator tests.

## Current Verification Snapshot

Commands recently run successfully:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_slider_test.bat
examples\xui_slider\build.bat
build\xui_slider.exe --frames 3
test_xui\build_scrollbar_test.bat
test_xui\build_toggle_test.bat
test_xui\build_radio_test.bat
test_xui\build_checkbox_test.bat
test_xui\build_builtin_asset_test.bat
test_xui\build_button_test.bat
test_xui\build_progress_test.bat
examples\xui_progress\build.bat
build\xui_progress.exe --frames 3
test_xui\build_image_test.bat
test_xui\build_separator_test.bat
test_xui\build_menubar_test.bat
examples\xui_menubar\build.bat
build\xui_menubar.exe --frames 3
test_xui\build_toolbar_test.bat
examples\xui_toolbar\build.bat
build\xui_toolbar.exe --frames 3
test_xui\build_statusbar_test.bat
examples\xui_statusbar\build.bat
build\xui_statusbar.exe --frames 3
test_xui\build_msgbox_test.bat
examples\xui_msgbox\build.bat
build\xui_msgbox.exe --frames 3
test_xui\build_msgtip_test.bat
examples\xui_msgtip\build.bat
build\xui_msgtip.exe --frames 5
test_xui\build_toast_test.bat
examples\xui_toast\build.bat
build\xui_toast.exe --frames 5
```

No-args Slider run was checked via hidden process start and stayed alive after 2 seconds.
No-args MenuBar run was checked via hidden process start and stayed alive after 2 seconds.
No-args Toolbar run was checked via hidden process start and stayed alive after 2 seconds.
No-args StatusBar run was checked via hidden process start and stayed alive after 2 seconds.
No-args MsgBox run was checked via hidden process start and stayed alive after 2 seconds.
No-args MsgTip run was checked via hidden process start and stayed alive after 2 seconds.

Known benign example warning:

- `build\xui_slider.exe --frames 3` may emit `libpng warning: iCCP: cHRM chunk does not match sRGB` from existing icon/resource loading; it exits successfully.
- `build\xui_menu.exe --frames 3` may emit the same libpng warning from existing icon/resource loading; it exits successfully.
- `build\xui_menubar.exe --frames 3` may emit the same libpng warning from existing icon/resource loading; it exits successfully.
- `build\xui_toolbar.exe --frames 3` may emit the same libpng warning from existing icon/resource loading; it exits successfully.
- `build\xui_statusbar.exe --frames 3` may emit the same libpng warning from existing icon/resource loading; it exits successfully.
- `build\xui_msgbox.exe --frames 3` may emit the same libpng warning from existing icon/resource loading; it exits successfully.
- `build\xui_msgtip.exe --frames 5` may emit the same libpng warning from existing icon/resource loading; it exits successfully.
- `build\xui_toast.exe --frames 5` may emit the same libpng warning from existing icon/resource loading; it exits successfully.
- `build\xui_combobox.exe --frames 3` may emit the same libpng warning from existing icon/resource loading; it exits successfully.

Formatting:

```bat
git diff --check
```

Result:

- passed
- only line-ending warnings for tracked files:
  - `dev/xui2/README.md`
  - `dev/xui2/docs/README.md`
  - `dev/xui2/docs/work.md`
  - `dev/xui2/src/xui_internal.h`
  - `dev/xui2/src/xui_widget.c`
  - `dev/xui2/test_xui/xui_test_proxy.c`
  - `dev/xui2/test_xui/xui_test_proxy.h`
  - `dev/xui2/tools/xui_asset_atlas.ps1`
  - `dev/xui2/xui.h`

Whitespace scan for recently edited files produced no trailing whitespace matches.

## Git Working Tree Caveat

The XUI2 worktree contains many untracked files from the ongoing rewrite. This is expected in the current phase.

Do not treat the untracked files as accidental. Do not delete or revert them.

Observed status includes many untracked XUI2 files such as:

- `src\xui_core.c`
- `src\xui_widget.c`
- `src\xui_text.c`
- `src\xui_label.c`
- `src\xui_button.c`
- `src\xui_image.c`
- `src\xui_separator.c`
- `src\xui_progress.c`
- many `test_xui\*.c`
- many `examples\xui_*`
- docs for migrated widgets

Tracked modified files include:

- `dev/xui2/README.md`
- `dev/xui2/docs/README.md`
- `dev/xui2/src/xui_proxy_xge.c`
- `dev/xui2/xui.h`

The modified `xui_proxy_xge.c` existed before the Progress work in this context. Do not revert it without explicit instruction.

## Important Known Tradeoffs

### Progress Fill Text Clipping

XUI1 used host clip to draw fill-colored text clipped to the fill rect.

XUI2 does not currently expose clip/scissor in proxy by design.

Current Progress behavior:

- base text always draws normally
- fill-colored text draws only if the fill rect fully contains the measured text visual rect

This is intentionally conservative:

- avoids adding proxy clip
- avoids visibly wrong centered text inside partial fill rect
- keeps Progress usable and efficient

If exact clipped two-color text is required later, solve it with:

- a text-layer offscreen cache/clip strategy, or
- a formally designed proxy extension

Do not add one-off clip functions in the control.

### Nine-Patch Reveal

Simple unsliced fill patch reveal is implemented by cropping source rect.

Sliced nine-patch reveal is not exactly equivalent to XUI1 `bEasyMode` because XUI2 `xui_nine_patch_t` does not carry the old `bEasyMode` field. Current behavior:

- if `slice == 0`, reveal crops source
- if sliced, draw nine-patch into current fill rect

This preserves common visual behavior without adding complexity to the proxy.

### XSON

XSON remains deferred.

Reason:

- per-control XSON during control migration creates churn and poor code quality
- after all controls are migrated, implement one coherent loader and property mapping pass

## Built-In Assets

XUI1 built-in assets have been inspected and migrated into XUI2 as an atlas-backed service.

Source material:

- `res/xui_builtin_atlas.json`
- `res/xui_builtin_atlas.png`
- `res/msgbox_*.png`
- `res/xui_*.png`
- `res/xui_*.bmp`
- `res/xui_dockpanel_suite/`

XUI2 implementation:

- `src/xui_assets.c`
- `src/xui_builtin_assets.inc`
- `tools/xui_asset_atlas.ps1`
- public APIs in `xui.h`:
  - `xuiBuiltinAssetGetCount`
  - `xuiBuiltinAssetGetName`
  - `xuiBuiltinAssetGetAtlasSize`
  - `xuiBuiltinAssetGetRect`
  - `xuiBuiltinAssetGetRectByIndex`
  - `xuiBuiltinAssetGetAtlas`

Design decisions:

- atlas data is XUI-owned, not XGE-owned
- generated symbols use `XUI_*` names, not `XGE_XUI_*`
- atlas surface is loaded through the active proxy using `surfaceLoadMemory`
- atlas lifetime is owned by the XUI context resource table
- controls should request the shared atlas from context instead of loading duplicate surfaces
- public API exposes names/rects/atlas; per-control drawing remains control-owned

Current verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_builtin_asset_test.bat
test_xui\build_button_test.bat
test_xui\build_image_test.bat
```

Known migrated asset groups:

- message box/tip icons
- checkbox/radio/menu marks
- button badge
- chevrons/triangles
- input/search/user/lock/eye/clear icons
- property check
- dock panel indicators and pane buttons

## Radio Notes

Radio and RadioGroup migration covered:

- cache-first radio widget type
- default modern flat circular visual
- checked/unchecked state
- pointer click select
- keyboard Space select
- radio-level change callback
- group-level selected-index change callback
- vertical and horizontal group orientation
- one-selected-at-a-time sibling synchronization
- complete cache states for checked, disabled, hover, active, and focus
- custom indicator surfaces
- optional built-in atlas indicator
- interactive XGE example input forwarding with auto-click smoke
- test, example, and doc

Public constants:

- `XUI_RADIO_STATE_CHECKED`
- `XUI_RADIO_GROUP_VERTICAL`
- `XUI_RADIO_GROUP_HORIZONTAL`

Files:

- `src\xui_radio.c`
- `test_xui\xui_radio_test.c`
- `test_xui\build_radio_test.bat`
- `examples\xui_radio\main.c`
- `examples\xui_radio\build.bat`
- `docs\xui\widget-radio.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_radio_test.bat
examples\xui_radio\build.bat
build\xui_radio.exe --frames 3
```

## Toggle Notes

Toggle migration covered:

- cache-first toggle widget type
- default modern flat switch visual
- compact 14px thumb on a 38x22 track, matching the smaller layui-style direction
- checked/unchecked state
- pointer click toggle
- keyboard Space toggle
- change callback
- complete cache states for checked, disabled, hover, active, and focus
- track/thumb/text geometry query APIs
- custom whole-indicator atlas surface
- future-compatible built-in atlas lookup for `toggle_unchecked` / `toggle_checked`
- interactive XGE example input forwarding with auto-click smoke
- test, example, and doc

Public constant:

- `XUI_TOGGLE_STATE_CHECKED`

Files:

- `src\xui_toggle.c`
- `test_xui\xui_toggle_test.c`
- `test_xui\build_toggle_test.bat`
- `examples\xui_toggle\main.c`
- `examples\xui_toggle\build.bat`
- `docs\xui\widget-toggle.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_toggle_test.bat
examples\xui_toggle\build.bat
build\xui_toggle.exe --frames 3
```

## ScrollBar Notes

ScrollBar migration covered:

- cache-first scrollbar widget type
- full mode with decrease/increase buttons
- compact mode with centered thin track/thumb
- horizontal and vertical orientation
- normalized range/page/value handling
- pointer thumb drag with capture
- track click page step
- button click small step
- mouse wheel large/page step
- keyboard navigation: Up/Left, Down/Right, PageUp, PageDown, Home, End, Escape
- change callback for user interaction
- geometry query APIs for track, thumb, and end buttons
- style properties for track/thumb/button colors and metrics
- interactive XGE example input forwarding with auto-click smoke
- test, example, and doc

Public constants:

- `XUI_SCROLLBAR_MODE_FULL`
- `XUI_SCROLLBAR_MODE_COMPACT`
- `XUI_SCROLLBAR_BUTTONS_AUTO`
- `XUI_SCROLLBAR_BUTTONS_ON`
- `XUI_SCROLLBAR_BUTTONS_OFF`
- `XUI_SCROLLBAR_PART_NONE`
- `XUI_SCROLLBAR_PART_TRACK`
- `XUI_SCROLLBAR_PART_THUMB`
- `XUI_SCROLLBAR_PART_DECREASE`
- `XUI_SCROLLBAR_PART_INCREASE`

Files:

- `src\xui_scrollbar.c`
- `test_xui\xui_scrollbar_test.c`
- `test_xui\build_scrollbar_test.bat`
- `examples\xui_scrollbar\main.c`
- `examples\xui_scrollbar\build.bat`
- `docs\xui\widget-scrollbar.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_scrollbar_test.bat
examples\xui_scrollbar\build.bat
build\xui_scrollbar.exe --frames 3
```

## Slider Notes

Slider migration covered:

- cache-first slider widget type
- horizontal and vertical orientation
- normalized range/value handling
- pointer click-to-position and drag with capture
- mouse wheel small-step adjustment
- keyboard navigation: Left/Down, Right/Up, PageDown, PageUp, Home, End, Escape
- change callback for user interaction
- geometry query APIs for track, fill, and knob
- modern flat default visual: light rail, blue accent fill, white circular knob
- style properties for track/fill/knob colors and metrics
- interactive XGE example input forwarding with auto-click smoke
- test, example, and doc

Files:

- `src\xui_slider.c`
- `test_xui\xui_slider_test.c`
- `test_xui\build_slider_test.bat`
- `examples\xui_slider\main.c`
- `examples\xui_slider\build.bat`
- `docs\xui\widget-slider.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_slider_test.bat
examples\xui_slider\build.bat
build\xui_slider.exe --frames 3
```

## ScrollModel / ScrollFrame / ScrollView Notes

Scroll mechanism migration covered:

- pure `xui_scroll_model_t` state object for viewport, content size, clamped offset, coordinate transforms, and ensure-rect-visible
- `scrollframe` widget type with internal viewport widget, horizontal/vertical ScrollBar widgets, and optional corner block
- `scrollview` widget type with a ScrollFrame and a content widget; business children are added to the content widget
- automatic/always/hidden scrollbar policy per axis
- full and compact scrollbar modes backed by the migrated ScrollBar control
- iterative auto-scrollbar layout that reserves visible bars and avoids the XUI1 chained-scrollbar case where a size-content parent can grow by the scrollbar reserve
- fixed-rect fallback that shrinks the viewport and only shows the opposite axis when content still overflows after reserve
- wheel axis policy, content drag, model/bar synchronization, and user change callbacks
- viewport clipping for rendering and hit testing through the widget parent chain
- test, example, and separated docs

New constants:

- `XUI_SCROLLBAR_POLICY_AUTO`
- `XUI_SCROLLBAR_POLICY_ALWAYS`
- `XUI_SCROLLBAR_POLICY_HIDDEN`
- `XUI_WHEEL_AXIS_VERTICAL`
- `XUI_WHEEL_AXIS_HORIZONTAL`
- `XUI_WHEEL_AXIS_BOTH`
- `XUI_SCROLL_FRAME_CORNER_NONE`
- `XUI_SCROLL_FRAME_CORNER_AUTO`
- `XUI_SCROLL_FRAME_CORNER_GRIP`

Files:

- `src\xui_scroll_model.c`
- `src\xui_scroll_frame.c`
- `src\xui_scroll_view.c`
- `test_xui\xui_scroll_model_test.c`
- `test_xui\build_scroll_model_test.bat`
- `test_xui\xui_scroll_frame_test.c`
- `test_xui\build_scroll_frame_test.bat`
- `test_xui\xui_scroll_view_test.c`
- `test_xui\build_scroll_view_test.bat`
- `examples\xui_scrollview\main.c`
- `examples\xui_scrollview\build.bat`
- `docs\xui\scroll-model.md`
- `docs\xui\widget-scrollframe.md`
- `docs\xui\widget-scrollview.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_scroll_model_test.bat
test_xui\build_scroll_frame_test.bat
test_xui\build_scroll_view_test.bat
examples\xui_scrollview\build.bat
build\xui_scrollview.exe --frames 3
```

## Next Recommended Work

Continue standard control migration.

Latest migrated control:

- Toast

Checkbox status:

- cache-first widget type
- default modern flat visual
- checked/unchecked state
- pointer and keyboard toggle
- change callback
- complete cache states for checked, disabled, hover, active, and focus
- interactive XGE example input forwarding with auto-click smoke
- custom indicator surfaces
- optional built-in atlas indicator
- test, example, and doc

Radio / RadioGroup status:

- cache-first widget type
- default modern flat visual
- checked/unchecked state
- group selection synchronization
- pointer and keyboard select
- radio and group change callbacks
- vertical/horizontal layout
- custom indicator surfaces
- optional built-in atlas indicator
- test, example, and doc

Toggle status:

- cache-first widget type
- default modern flat visual
- compact thumb style
- checked/unchecked state
- pointer and keyboard toggle
- change callback
- custom atlas surface support
- future-compatible built-in atlas lookup
- test, example, and doc

ScrollBar status:

- cache-first widget type
- full and compact modes
- horizontal and vertical orientation
- pointer drag, track click, button click, wheel, and keyboard input
- range/page/value normalization
- change callback
- geometry query APIs
- style properties
- test, example, and doc

Slider status:

- cache-first widget type
- horizontal and vertical orientation
- pointer click-to-position and drag
- wheel and keyboard input
- range/value normalization
- change callback
- geometry query APIs
- style properties
- test, example, and doc

ScrollModel / ScrollFrame / ScrollView status:

- pure model state
- automatic scrollbar policy
- full and compact frame modes
- viewport clipping and content widget ownership
- wheel, content drag, and ensure-child-visible
- anti-chained-scrollbar measure/arrange rule
- separated docs for model, frame, and view
- tests and XGE example

Popup status:

- overlay-layer shell with ScrollView / ScrollFrame payload
- owner binding and explicit anchor rect
- bottom/top and left/right anchor directions with flip fallback and viewport clamp
- outside, owner, Escape, modal, consume-inside, and focus-restore policies
- automatic popup viewport sizing with anti-chained-scrollbar reserve
- style metrics and color APIs for panel, border, shadow, and backdrop
- test, example, and doc

Menu status:

- Popup-backed command menu widget
- item measurement, cached item rects, hover hit testing, and cache-first painting
- normal, separator, check, radio, and submenu item types
- explicit enabled/disabled item state plus default and danger presentation
- check toggling and contiguous radio segment synchronization
- keyboard navigation, Escape close, Enter/Space commit, and shortcut commit path
- hover-open submenu chain with root close behavior
- metrics/color customization APIs
- test, example, and doc

Files:

- `src\xui_menu.c`
- `test_xui\xui_menu_test.c`
- `test_xui\build_menu_test.bat`
- `examples\xui_menu\main.c`
- `examples\xui_menu\build.bat`
- `docs\xui\widget-menu.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
examples\xui_menu\build.bat
build\xui_menu.exe --frames 3
```

MenuBar status:

- cache-first top-level menu strip widget
- XUI1-style `&` mnemonic parsing with `Alt+letter` hotkey registration and `&&` escaping
- Menu-backed dropdown items with Popup outside close, owner close, Escape close, and focus restore
- top-level hover, active, and open item tracking with pointer switching while a dropdown is open
- F10/context-menu focus entry, Left/Right top-level navigation, Up/Down dropdown hover movement, Enter/Space commit, and Escape close
- direct top-level command items without dropdown menus
- explicit enabled/disabled item state, item rect queries, state/change queries, metrics/color APIs, and style properties
- public `xuiMenuCommitHover` helper added for MenuBar keyboard commit into open dropdown menus
- test, example, and doc

Files:

- `src\xui_menubar.c`
- `src\xui_menu.c`
- `test_xui\xui_menubar_test.c`
- `test_xui\build_menubar_test.bat`
- `examples\xui_menubar\main.c`
- `examples\xui_menubar\build.bat`
- `docs\xui\widget-menubar.md`
- `docs\xui\widget-menu.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menubar_test.bat
examples\xui_menubar\build.bat
build\xui_menubar.exe --frames 3
```

Toolbar status:

- cache-first command strip widget
- button, toggle, and separator item types
- horizontal and vertical orientation
- explicit enabled/disabled state, checked toggle state, item rect queries, hover/active state, and change counters
- pointer capture on press and same-item release commit
- keyboard hover movement and Space/Enter activation
- tooltip metadata through the core tooltip resolver
- group gaps and overflow button reporting with first/count callback
- metrics/color customization APIs and style properties
- test, example, and doc

Files:

- `src\xui_toolbar.c`
- `test_xui\xui_toolbar_test.c`
- `test_xui\build_toolbar_test.bat`
- `examples\xui_toolbar\main.c`
- `examples\xui_toolbar\build.bat`
- `docs\xui\widget-toolbar.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_toolbar_test.bat
examples\xui_toolbar\build.bat
build\xui_toolbar.exe --frames 3
```

StatusBar status:

- cache-first bottom chrome/status strip widget
- text, progress, fixed spacer, and flexible spacer item types
- left, center, and right independent section layout
- explicit enabled/clickable state, progress clamping, item rect queries, hover/active state, and change counters
- pointer capture on press and same-item release commit
- Space/Enter keyboard activation for focused status bar
- metrics/color customization APIs and style properties
- test, example, and doc

Files:

- `src\xui_statusbar.c`
- `test_xui\xui_statusbar_test.c`
- `test_xui\build_statusbar_test.bat`
- `examples\xui_statusbar\main.c`
- `examples\xui_statusbar\build.bat`
- `docs\xui\widget-statusbar.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_statusbar_test.bat
examples\xui_statusbar\build.bat
build\xui_statusbar.exe --frames 3
```

Input status:

- cache-first single-line text input widget
- text, placeholder, selection, caret, password, readonly, error state, max length, and change callback
- pointer click/drag selection, double-click select-all, text input, Backspace/Delete, arrows, Home/End, and Ctrl editing shortcuts
- proxy clipboard integration for copy/cut/paste
- IME candidate rect integration through the existing proxy hook
- XUI1-style generic input decorations for leading/trailing icon, text, texture, clear, and custom paint nodes
- decoration visibility modes reserve text padding automatically, so selection/caret/IME layout avoids decoration areas
- built-in right-click/context-menu-key menu backed by `xuiMenu`
- Windows Notepad-style menu grouping with separators
- Chinese default menu titles with `xuiInputSetMenuTitle` overrides
- style properties for text, placeholder, background, border, error, selection, cursor, radius, and font
- test, example, and doc

Files:

- `src\xui_input_widget.c`
- `test_xui\xui_input_widget_test.c`
- `test_xui\build_input_widget_test.bat`
- `examples\xui_input\main.c`
- `examples\xui_input\build.bat`
- `docs\xui\widget-input.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_input_widget_test.bat
examples\xui_input\build.bat
build\xui_input.exe --frames 3
```

TextEdit status:

- cache-first multi-line text editing widget
- text, placeholder, selection, caret, readonly, word wrap, max length, internal scroll, and change callback
- pointer click/drag selection, double-click select-all, text input, Enter, Backspace/Delete, arrows, PageUp/PageDown, Home/End, and Ctrl editing shortcuts
- proxy clipboard integration for multi-line copy/cut/paste
- undo/redo stacks for editing operations
- IME candidate rect integration through the existing proxy hook
- built-in right-click/context-menu-key menu backed by `xuiMenu`
- reuses `XUI_INPUT_MENU_*` command ids and the Input default Chinese menu titles
- style properties for text, placeholder, background, border, selection, cursor, radius, line gap, and font
- test, example, and doc

Files:

- `src\xui_text_edit.c`
- `test_xui\xui_text_edit_test.c`
- `test_xui\build_text_edit_test.bat`
- `examples\xui_textedit\main.c`
- `examples\xui_textedit\build.bat`
- `docs\xui\widget-textedit.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_text_edit_test.bat
examples\xui_textedit\build.bat
build\xui_textedit.exe --frames 3
```

Current V1 note:

- TextEdit has internal scrolling but does not yet draw visible TextEdit-owned scrollbars. If XUI1-style scrollbar modes are required, wire the text editing core to ScrollFrame/ScrollBar presentation in the next slice.

NumericInput status:

- cache-first numeric input widget backed by an internal `xuiInput`
- preserves XUI1 split: Input owns text editing/selection/IME/menu, NumericInput owns range/value/step/commit/spinner semantics
- default range `0..100`, step `1`, precision `3`, right-aligned text, visible spinner, invalid text error state
- supports integer mode, formatter callback, change callback, error callback, readonly mode, hidden spinner, spinner width/colors, wheel, Up/Down keys, Enter/focus-loss commit
- reuses Input right-click menu and exposes menu-title passthrough APIs
- exposes spinner/button rects, hover/active buttons, button enabled state, input child, state, error, and change count
- XSON remains deferred

Files:

- `src\xui_numeric_input.c`
- `test_xui\xui_numeric_input_test.c`
- `test_xui\build_numeric_input_test.bat`
- `examples\xui_numericinput\main.c`
- `examples\xui_numericinput\build.bat`
- `docs\xui\widget-numeric-input.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_numeric_input_test.bat
examples\xui_numericinput\build.bat
build\xui_numericinput.exe --frames 3
```

ComboBox status:

- cache-first single-select dropdown owner widget backed by an internal `xuiMenu`
- preserves XUI1 data model: simple string arrays plus structured items with value, enabled, separator, icon, and user data
- Popup/Menu-backed dropdown handles outside close, Escape close, focus restore, keyboard navigation, compact scrollbar, and owner-width matching
- supports selected index, selected value lookup, enabled item map, explicit popup height/max height, top/bottom/auto popup placement, and selection callback
- exposes internal Menu/Popup widgets, button/text rects, state, metrics, colors, font, and change count for tests/integration
- XSON remains deferred

Files:

- `src\xui_combobox.c`
- `test_xui\xui_combobox_test.c`
- `test_xui\build_combobox_test.bat`
- `examples\xui_combobox\main.c`
- `examples\xui_combobox\build.bat`
- `docs\xui\widget-combobox.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_combobox_test.bat
examples\xui_combobox\build.bat
build\xui_combobox.exe --frames 3
```

ColorPicker status:

- cache-first color picker owner widget backed by an internal `xuiPopup`
- preserves XUI1 split: compact owner surface plus popup editor panel
- supports RGB and RGBA modes, `#RRGGBB` / `#RRGGBBAA` hex text, SV area, hue bar, alpha slider, RGB/A sliders, old/new swatches, and a 16-entry palette
- palette/SV/hue/channel/alpha/old-swatch edits apply immediately and notify the change callback only when the effective color changes
- disabled/hidden owner blocks or closes the popup
- exposes internal Popup/panel widgets, owner-local rects, panel-local rects, hover/active part, state, metrics, colors, font, and change count for tests/integration
- XSON remains deferred

Files:

- `src\xui_color_picker.c`
- `test_xui\xui_color_picker_test.c`
- `test_xui\build_color_picker_test.bat`
- `examples\xui_colorpicker\main.c`
- `examples\xui_colorpicker\build.bat`
- `docs\xui\widget-color-picker.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_color_picker_test.bat
examples\xui_colorpicker\build.bat
build\xui_colorpicker.exe --frames 3
```

DatePicker status:

- cache-first date/time picker owner widget backed by an internal `xuiPopup`
- preserves XUI1 split: compact owner surface plus popup draft editor
- supports date, time, datetime, date range, time range, and datetime range modes
- supports nullable values, min/max limits, first-day-of-week calendar layout, hidden seconds, custom formats, custom range separators, and default range spans
- popup edits stay in a draft until OK; Cancel, Escape, and outside close cancel the draft; Clear works only when nullable
- exposes internal Popup/panel widgets, owner-local rects, panel-local rects, hover/active part, state, metrics, colors, font, text, and callback counters for tests/integration
- XSON remains deferred

Files:

- `src\xui_date_picker.c`
- `test_xui\xui_date_picker_test.c`
- `test_xui\build_date_picker_test.bat`
- `examples\xui_datepicker\main.c`
- `examples\xui_datepicker\build.bat`
- `docs\xui\widget-date-picker.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_date_picker_test.bat
examples\xui_datepicker\build.bat
build\xui_datepicker.exe --frames 3
```

Panel status:

- cache-first styled container widget with root/header/icon/title/client internal structure
- preserves XUI1 split: Panel owns presentation, application children belong to the client widget
- supports title text, title font/color/alignment, optional icon surface/source/size, hidden header, custom background/header/client colors, border, radius, header gap, and client clipping
- client widget remains a normal XUI container and can use column/row/grid/table/flow/dock/manual layout APIs
- exposes internal widgets, panel-local header/icon/title/client rects, state, and metrics for tests/integration
- XSON remains deferred

Files:

- `src\xui_panel.c`
- `test_xui\xui_panel_test.c`
- `test_xui\build_panel_test.bat`
- `examples\xui_panel\main.c`
- `examples\xui_panel\build.bat`
- `docs\xui\widget-panel.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_panel_test.bat
examples\xui_panel\build.bat
build\xui_panel.exe --frames 3
```

## SplitLayout Migration Status

SplitLayout is migrated as a layout widget under the public type name `splitlayout`.

Implemented:

- pane count, internal pane widgets, and `xuiSplitLayoutAddPaneChild`
- vertical orientation for left-to-right panes and horizontal orientation for top-to-bottom panes
- fixed/grow pane modes with weights, fixed sizes, min/max sizes, resolved size queries, and oversubscribed proportional fallback
- divider layout/visual/hit rectangles with separate metrics
- shadow drag preview and live drag
- adjacent-pane drag commit rules from XUI1
- divider hover/active/focus state, pointer capture, capture-loss and Escape cancel handling
- style properties for divider colors, shadow color, and divider metrics
- geometry/state/change-count APIs for tests and integrations
- focused test, code-only example, and widget document
- XSON remains deferred

Files:

- `src\xui_split_layout.c`
- `test_xui\xui_split_layout_test.c`
- `test_xui\build_split_layout_test.bat`
- `examples\xui_split_layout\main.c`
- `examples\xui_split_layout\build.bat`
- `docs\xui\widget-splitlayout.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_split_layout_test.bat
examples\xui_split_layout\build.bat
build\xui_split_layout.exe --frames 3
```

## Tabs Migration Status

Tabs is migrated as a page-container widget under the public type name `tabs`.

Implemented:

- internal tabbar widget, client widget, tab button widgets, and normal page widgets
- top/bottom/left/right placement, including vertical label drawing for side tabs
- selected-page visibility and public page/tabbar/client/button widget accessors
- disabled item handling: pointer selection ignored and keyboard navigation skips disabled tabs
- close buttons with a dedicated close callback that does not trigger selection
- scrollable tabbar with wheel scrolling, clamped `scrollX`, max-scroll query, and selected-tab auto reveal
- dirty badge and icon metadata rectangles, with text space reservation
- XUI1-style visual connection between the selected tab and the client border, avoiding a duplicate separator line
- style properties for tab colors, text colors, client/border colors, and tab metrics
- focused test, code-only example, and widget document
- XSON remains deferred

Files:

- `src\xui_tabs.c`
- `test_xui\xui_tabs_test.c`
- `test_xui\build_tabs_test.bat`
- `examples\xui_tabs\main.c`
- `examples\xui_tabs\build.bat`
- `docs\xui\widget-tabs.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_tabs_test.bat
examples\xui_tabs\build.bat
build\xui_tabs.exe --frames 3
```

## Accordion Migration Status

Accordion is migrated as a collapsible section-container widget under the public type name `accordion`.

Implemented:

- internal section widgets, header hit widgets, and client widgets
- multiple-expanded and single-expanded modes
- collapsed clients become invisible and are assigned zero layout height
- disabled headers keep visual state but ignore pointer selection
- select callback with section index and id
- keyboard navigation with Up/Down/Home/End and Space/Enter toggle
- metrics, colors, font, section title/id/state APIs, and geometry query APIs
- style properties for header/content colors and layout metrics

Files:

- `src\xui_accordion.c`
- `test_xui\xui_accordion_test.c`
- `test_xui\build_accordion_test.bat`
- `examples\xui_accordion\main.c`
- `examples\xui_accordion\build.bat`
- `docs\xui\widget-accordion.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_accordion_test.bat
examples\xui_accordion\build.bat
build\xui_accordion.exe --frames 3
```

## Window Migration Status

Window is migrated as a floating overlay container widget under the public type name `window`.

Implemented:

- automatic overlay-root attachment on the floating layer
- normal/top-most z groups with activation and bring-to-front behavior
- internal client widget plus collapse, maximize, and close button widgets
- title text, optional icon, titlebar visibility, button visibility, and close callback
- titlebar drag, optional drag-anywhere frameless mode, edge resize, min-size clamp, and maximize/restore
- collapse/expand that hides the client widget and restores the previous expanded height
- active/inactive chrome colors, button hover/active colors, close colors, metrics, state, change-count, and geometry query APIs
- style properties for chrome colors, font, titlebar height, border width, resize grip, button/icon size, radius, and min size
- focused test, code-only example, and widget document
- XSON remains deferred

Files:

- `src\xui_window.c`
- `test_xui\xui_window_test.c`
- `test_xui\build_window_test.bat`
- `examples\xui_window\main.c`
- `examples\xui_window\build.bat`
- `docs\xui\widget-window.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_window_test.bat
examples\xui_window\build.bat
build\xui_window.exe --frames 3
```

## MsgBox Migration Status

MsgBox is migrated as a Window-backed utility object under the public handle type `xui_msgbox`.

Implemented:

- opaque utility object that owns an internal Window, content widget, button widgets, and modal backdrop widget
- default modal behavior with backdrop event consumption
- XUI1 icon types, result constants, preset buttons, custom buttons, and result callback
- built-in atlas icons for info/question/warning/error, plus custom icon surface support
- automatic message wrapping and dialog sizing capped by the root width ratio
- Enter commits the first button result, Escape and titlebar close return close result
- metrics, colors, modal/open/text/type/button APIs, internal widget getters, geometry queries, and counters
- focused test, code-only example, and utility document
- XSON remains deferred

Files:

- `src\xui_msgbox.c`
- `test_xui\xui_msgbox_test.c`
- `test_xui\build_msgbox_test.bat`
- `examples\xui_msgbox\main.c`
- `examples\xui_msgbox\build.bat`
- `docs\xui\widget-msgbox.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_msgbox_test.bat
examples\xui_msgbox\build.bat
build\xui_msgbox.exe --frames 3
```

## MsgTip Migration Status

MsgTip is migrated as a cache-rendered overlay utility object under the public handle type `xui_msgtip`.

Implemented:

- opaque utility object that owns one internal overlay widget on `XUI_LAYER_TOOLTIP`
- XUI1 single-tip behavior: `Show` replaces current content, duration auto-closes, `duration <= 0` stays open
- tip-body hit test only: clicking the tip closes it, outside clicks pass through without closing
- built-in atlas icons for info/question/warning/error, plus custom icon surface support
- automatic text wrapping, centered layout with configurable vertical offset, and root/viewport bounds clamping
- metrics, colors, duration/text/type APIs, close callback, widget/geometry getters, and counters
- focused test, interactive example with auto-run checks, and utility document
- XSON remains deferred because MsgTip is a runtime service, not page structure

Files:

- `src\xui_msgtip.c`
- `test_xui\xui_msgtip_test.c`
- `test_xui\build_msgtip_test.bat`
- `examples\xui_msgtip\main.c`
- `examples\xui_msgtip\build.bat`
- `docs\xui\widget-msgtip.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_msgtip_test.bat
examples\xui_msgtip\build.bat
build\xui_msgtip.exe --frames 5
```

No-args `build\xui_msgtip.exe` was checked via hidden process start and stayed alive after 2 seconds. `--frames 5` may emit the existing libpng iCCP warning from atlas loading and still exits successfully.

## Toast Migration Status

Toast is migrated as a queue-based runtime notification utility object under the public handle type `xui_toast`.

Implemented:

- opaque utility object that owns independent overlay item widgets on `XUI_LAYER_TOOLTIP`
- XUI1 active/pending queue behavior with stable `toastId`
- visible item limit and pending queue capacity, including `CAPACITY_DROP` close reason
- duration starts when an item becomes visible; `duration <= 0` uses the XUI1 default duration
- body click callback, close button close, API close, clear, timeout, and close reason callback
- pointer close requests are deferred until `xuiUpdate` so callbacks can safely call show/close/clear
- transparent space outside each toast does not block the page below
- built-in atlas icons for info/success/warning/error and close button
- automatic text wrapping, stack placement, direction, metrics, colors, font APIs, item geometry getters, and counters
- focused test, interactive example with auto-run checks, and utility document
- XSON remains deferred because Toast is a runtime service, not page structure

Files:

- `src\xui_toast.c`
- `test_xui\xui_toast_test.c`
- `test_xui\build_toast_test.bat`
- `examples\xui_toast\main.c`
- `examples\xui_toast\build.bat`
- `docs\xui\widget-toast.md`

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_toast_test.bat
examples\xui_toast\build.bat
build\xui_toast.exe --frames 5
```

`build\xui_toast.exe --frames 5` may emit the existing libpng iCCP warning from atlas loading and still exits successfully.

Likely next XUI1 controls to migrate, depending on priority:

- next requested XUI1 control or higher-level menu integration layer

For each control, follow the migration standard above:

- behavior from XUI1
- XUI2 cache/render/style/layout integration
- test
- example
- doc
- no XSON yet

Higher-level text editing integrations should build on the new TextEdit core instead of reintroducing ad hoc editing state.

## 2026-05-30 ListView Migration

Status: implemented for XUI2.

Files:

- `src/xui_list_view.c`
- `test_xui/xui_list_view_test.c`
- `test_xui/build_list_view_test.bat`
- `examples/xui_listview/main.c`
- `examples/xui_listview/build.bat`
- `docs/xui/widget-listview.md`

Design notes:

- ListView embeds ScrollFrame directly.
- Horizontal scrollbar is hidden by default; vertical scrollbar is auto and compact by default.
- Rows are cache-rendered inside the viewport rather than represented as child widgets.
- The migration preserves XUI1 selection modes, disabled-row rejection, hover, keyboard navigation, ensure-visible, external selection buffer, and repeat-select notification.
- Custom row rendering is adapted for XUI2 by passing `xui_draw_context` into `xui_list_view_item_proc`.

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_list_view_test.bat
examples\xui_listview\build.bat
build\xui_listview.exe --frames 5
```

## 2026-05-30 TreeView Migration

Status: implemented for XUI2.

Files:

- `src/xui_tree_view.c`
- `test_xui/xui_tree_view_test.c`
- `test_xui/build_tree_view_test.bat`
- `examples/xui_treeview/main.c`
- `examples/xui_treeview/build.bat`
- `docs/xui/widget-treeview.md`

Design notes:

- TreeView embeds ScrollFrame directly.
- Horizontal scrollbar is hidden by default; vertical scrollbar is auto and compact by default.
- Nodes are stored as an id/parent array, and the visible row array is rebuilt from expanded branches.
- The migration preserves XUI1 expand/collapse, checkbox, disabled-node rejection, selection fallback on collapse, hover, keyboard navigation, ensure-visible, adapter refresh, and custom row rendering.
- Custom row rendering receives `xui_draw_context`, node id, visible index, node state, row rect, and state flags.
- XSON remains deferred.

Verification:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_tree_view_test.bat
examples\xui_treeview\build.bat
build\xui_treeview.exe --frames 5
```

`build\xui_treeview.exe --frames 5` may emit the existing libpng iCCP warning from resource loading and still exits successfully.

## Quick Command Reference

Build and run Progress:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_progress_test.bat
examples\xui_progress\build.bat
build\xui_progress.exe --frames 360
```

Run migrated control tests:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_label_test.bat
test_xui\build_builtin_asset_test.bat
test_xui\build_checkbox_test.bat
test_xui\build_radio_test.bat
test_xui\build_toggle_test.bat
test_xui\build_scrollbar_test.bat
test_xui\build_slider_test.bat
test_xui\build_scroll_model_test.bat
test_xui\build_scroll_frame_test.bat
test_xui\build_scroll_view_test.bat
test_xui\build_list_view_test.bat
test_xui\build_tree_view_test.bat
test_xui\build_table_grid_test.bat
test_xui\build_property_grid_test.bat
test_xui\build_timeline_view_test.bat
test_xui\build_popup_test.bat
test_xui\build_menu_test.bat
test_xui\build_menubar_test.bat
test_xui\build_toolbar_test.bat
test_xui\build_statusbar_test.bat
test_xui\build_msgbox_test.bat
test_xui\build_msgtip_test.bat
test_xui\build_toast_test.bat
test_xui\build_combobox_test.bat
test_xui\build_color_picker_test.bat
test_xui\build_date_picker_test.bat
test_xui\build_panel_test.bat
test_xui\build_split_layout_test.bat
test_xui\build_tabs_test.bat
test_xui\build_accordion_test.bat
test_xui\build_window_test.bat
test_xui\build_input_widget_test.bat
test_xui\build_numeric_input_test.bat
test_xui\build_text_edit_test.bat
test_xui\build_button_test.bat
test_xui\build_image_test.bat
test_xui\build_separator_test.bat
test_xui\build_progress_test.bat
```

Run examples:

```bat
cd /d D:\git\xge\dev\xui2
examples\xui_label\build.bat
examples\xui_button\build.bat
examples\xui_checkbox\build.bat
examples\xui_radio\build.bat
examples\xui_toggle\build.bat
examples\xui_scrollbar\build.bat
examples\xui_slider\build.bat
examples\xui_scrollview\build.bat
examples\xui_listview\build.bat
examples\xui_treeview\build.bat
examples\xui_tablegrid\build.bat
examples\xui_propertygrid\build.bat
examples\xui_timelineview\build.bat
examples\xui_popup\build.bat
examples\xui_menu\build.bat
examples\xui_menubar\build.bat
examples\xui_toolbar\build.bat
examples\xui_statusbar\build.bat
examples\xui_msgbox\build.bat
examples\xui_msgtip\build.bat
examples\xui_toast\build.bat
examples\xui_combobox\build.bat
examples\xui_colorpicker\build.bat
examples\xui_datepicker\build.bat
examples\xui_panel\build.bat
examples\xui_split_layout\build.bat
examples\xui_tabs\build.bat
examples\xui_accordion\build.bat
examples\xui_window\build.bat
examples\xui_input\build.bat
examples\xui_numericinput\build.bat
examples\xui_textedit\build.bat
examples\xui_image\build.bat
examples\xui_separator\build.bat
examples\xui_progress\build.bat
build\xui_label.exe --frames 360
build\xui_button.exe --frames 360
build\xui_checkbox.exe --frames 360
build\xui_radio.exe --frames 360
build\xui_toggle.exe --frames 360
build\xui_scrollbar.exe --frames 360
build\xui_slider.exe --frames 360
build\xui_scrollview.exe --frames 360
build\xui_listview.exe --frames 360
build\xui_treeview.exe --frames 360
build\xui_tablegrid.exe --frames 360
build\xui_propertygrid.exe --frames 360
build\xui_timelineview.exe --frames 360
build\xui_popup.exe --frames 360
build\xui_menu.exe --frames 360
build\xui_menubar.exe --frames 360
build\xui_toolbar.exe --frames 360
build\xui_statusbar.exe --frames 360
build\xui_msgbox.exe --frames 360
build\xui_msgtip.exe --frames 360
build\xui_toast.exe --frames 360
build\xui_combobox.exe --frames 360
build\xui_colorpicker.exe --frames 360
build\xui_datepicker.exe --frames 360
build\xui_panel.exe --frames 360
build\xui_split_layout.exe --frames 360
build\xui_tabs.exe --frames 360
build\xui_accordion.exe --frames 360
build\xui_window.exe --frames 360
build\xui_input.exe --frames 360
build\xui_numericinput.exe --frames 360
build\xui_textedit.exe --frames 360
build\xui_image.exe --frames 360
build\xui_separator.exe --frames 360
build\xui_progress.exe --frames 360
```

Formatting check:

```bat
cd /d D:\git\xge
git diff --check
```

## Summary For Resume

If resuming from this document, assume:

- XUI2 infrastructure is mature enough to continue standard control migration.
- XSON should still be postponed.
- Do not alter proxy capabilities unless a missing capability is explicitly reported and approved.
- Maintain cache-first rendering.
- Maintain integer pixel snapping through shared helpers.
- Keep API naming as `xuiCamelCase`.
- Continue with controls using XUI1 behavior as reference, XUI2 architecture as implementation target.

## 2026-05-30 TableView Migration

- Added `xuiTableView` typed widget with copied columns/rows, adapter callbacks, cell formatter/renderer hooks, header renderer, merge provider, tooltip resolver, selection callbacks, hover callbacks, sort callbacks, and column-resize callbacks.
- The widget uses a fixed cache-rendered header plus an internal `ScrollFrame` viewport. Horizontal and vertical scrollbars are automatic by default, compact by default, and expose the frame/viewport/model for integration.
- Preserved the XUI1 interaction model: row/cell selection modes, disabled row/cell rejection, keyboard navigation, header sort, header-edge column resize, merged-cell owner hit testing, and ensure-cell-visible scrolling.
- Added cell visuals for text, bool checkbox, color swatch, picker-style cells, invalid/dirty/editing markers, focus stroke, row stripes, hover, selection, and custom cell/header renderers.
- Added tests and example:
  - `test_xui\build_table_view_test.bat`
  - `examples\xui_tableview\build.bat`
  - `build\xui_tableview.exe --frames 5`
- Added docs:
  - `docs\xui\widget-tableview.md`

Verification on 2026-05-30:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_table_view_test.bat
examples\xui_tableview\build.bat
build\xui_tableview.exe --frames 5
```

The example summary reached `create=1 layout=1 select=1 sort=1 scroll=1 custom=1`. The run still prints the existing libpng iCCP warning from bundled resources.

## 2026-05-30 TableGrid Migration

- Added `xuiTableGrid` typed widget as an editable layer over `xuiTableView`; TableGrid reuses TableView for header, scrollbars, selection, sorting, column resize, merge, formatters, renderers, and cell painting.
- Preserved the XUI1 ownership contract: the adapter/model owns table data, TableGrid reads through `onCount/onCell`, validates edits through `onValidate`, and writes accepted values through `onSet` plus `onChange`.
- Added editor dispatch for text, int, float, bool, enum, color, date/time/datetime, textarea, picker/file/image, and custom cells. Inline editors are clipped to the TableView viewport; textarea uses Popup + TextEdit + OK/Cancel.
- Added display edit mode, quick edit mode, reserved immediate mode, Enter/Escape handling, outside pointer/wheel commit, validation reject state, editing-cell marker, and commit/reject/cancel/picker/change counters.
- Added tests and example:
  - `test_xui\build_table_grid_test.bat`
  - `examples\xui_tablegrid\build.bat`
  - `build\xui_tablegrid.exe --frames 5`
- Added docs:
  - `docs\xui\widget-tablegrid.md`

Verification on 2026-05-30:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_table_grid_test.bat
examples\xui_tablegrid\build.bat
build\xui_tablegrid.exe --frames 5
```

The example summary reached `create=1 layout=1 edit=1 validate=1 picker=1 scroll=1 quick=1`. The run still prints the existing libpng iCCP warning from bundled resources.

## 2026-05-30 PropertyGrid Migration

- Added `xuiPropertyGrid` typed widget as an inspector-style property editor built on `xuiTableGrid` and `xuiTableView`.
- Preserved the XUI1 behavior that matters for user experience: one-level categories, merged category rows, collapse/expand, hidden properties, readonly/disabled flags, dirty/invalid markers, quick edit, description tooltip/panel, value-column editing, and picker/file/image/custom callbacks.
- The control owns a compact category/property model, while editor widgets remain delegated to TableGrid. Enum/numeric/date/color editor config can be stored per property.
- Added selected-property APIs, typed value helpers, per-property and global action/render callbacks, style/color APIs, scroll APIs, and select/toggle counters for diagnostics.
- Added tests and example:
  - `test_xui\build_property_grid_test.bat`
  - `examples\xui_propertygrid\build.bat`
  - `build\xui_propertygrid.exe --frames 5`
- Added docs:
  - `docs\xui\widget-propertygrid.md`

Verification on 2026-05-30:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_property_grid_test.bat
examples\xui_propertygrid\build.bat
build\xui_propertygrid.exe --frames 5
```

The example summary reached `create=1 layout=1 edit=1 validate=1 action=1 toggle=1 scroll=1 render=1`. The run still prints the existing libpng iCCP warning from bundled resources.

## 2026-05-30 TimeLineView Migration

- Added `xuiTimeLineView` typed widget with a compact internal layer/frame/span/selection model, XUI2 cache rendering, and an internal `ScrollFrame` for automatic horizontal and vertical scrolling.
- Preserved the XUI1 timeline interaction model: four-region layout, ruler, layer list, timeline grid, current-frame playhead, sparse key/blank/normal frames, spans, selection separate from current frame, layer visible/lock toggles, hit testing, keyboard navigation, drag selection, and Ctrl+wheel zoom.
- Added callbacks for current-frame, layer, frame, span, layer-selected, context opening/command, frame click/double-click, selection, plus custom renderer hooks for layer/ruler/frame/span painting.
- Added public APIs for frame count/rate/current frame, metrics, frame width range, layers, sparse frames, spans, selection, colors, offsets, ensure visible, hit testing, and diagnostic counters.
- Added tests and example:
  - `test_xui\build_timeline_view_test.bat`
  - `examples\xui_timelineview\build.bat`
  - `build\xui_timelineview.exe --frames 5`
- Added docs:
  - `docs\xui\widget-timelineview.md`

Verification on 2026-05-30:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_timeline_view_test.bat
examples\xui_timelineview\build.bat
build\xui_timelineview.exe --frames 5
```

The example summary reached `create=1 layout=1 interaction=1 scroll=1 paint=1`. The run still prints the existing libpng iCCP warning from bundled resources.

### 2026-05-31 TimeLineView Selection Follow-up

- Fixed grid selection input so Shift-click, Ctrl-click, plain drag range selection, and Ctrl-drag range add/remove behave like the XUI1 timeline selection model.
- TimeLineView now keeps a drag-start selection snapshot and applies drag selection live while the mouse moves instead of waiting until pointer-up.
- The timeline example now forwards Shift/Ctrl/Alt/Super modifier state into the XUI input context, so mouse selection and Shift keyboard navigation receive the correct modifiers.
- Expanded the TimeLineView unit test coverage for Shift range selection, Ctrl toggle, drag range selection, and Ctrl-drag range add.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_timeline_view_test.bat
examples\xui_timelineview\build.bat
build\xui_timelineview.exe --frames 3
```

## 2026-05-31 Chart V1 Initial Implementation

- Added `xuiChart` as a typed XUI2 widget in `src\xui_chart.c`.
- Added public chart constants, data structs, hit-test structs, descriptor, and API declarations in `xui.h`.
- V1 renders line, grouped vertical bar, pie/donut, and scatter series through the existing draw proxy primitives; no new proxy capability was added.
- Added copied point-array data storage, value/category axis modes, plot rect/range calculation, legend rendering, legend click visibility toggle, default tooltip overlay, selection/hover state, and hit testing for series/legend parts.
- Added focused chart unit test and four-panel chart example.
- Kept ECharts as a product-reference only; no option/API compatibility was added.

Files:

- `src\xui_chart.c`
- `test_xui\xui_chart_test.c`
- `test_xui\build_chart_test.bat`
- `examples\xui_chart\main.c`
- `examples\xui_chart\build.bat`
- `docs\chart-control-design.md`
- `docs\chart-control-spec.md`
- `docs\xui\widget-chart.md`

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/test_xui/build_chart_test.bat dev/xui2/examples/xui_chart dev/xui2/docs dev/xui2/README.md
```

Observed run summary:

```text
xui_chart final-summary frames=5 create=1 layout=1 updatedCaches=0 drawnCaches=4
```

Known V1 gaps:

- implementation is currently concentrated in `src\xui_chart.c`; planned split files are deferred
- custom tooltip callback is implemented; richer formatter context is deferred
- category axis labels are implemented; advanced tick formatting is deferred
- chart cache layers are represented by normal widget cache invalidation rather than separate static/plot/overlay cache objects
- example run reports existing libpng iCCP warnings from loaded image/profile data outside chart rendering

### 2026-05-31 Chart SPEC Follow-up

- Added `xuiChartSetTooltipCallback`.
- Added `xuiChartDataToPixel` and `xuiChartPixelToData`.
- Added basic y-axis value labels and x-axis category/value labels.
- Expanded `xui_chart_test` coverage for coordinate round-trip, legend hit testing, and custom tooltip rendering.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
```

Observed run summary remains:

```text
xui_chart final-summary frames=5 create=1 layout=1 updatedCaches=0 drawnCaches=4
```

### 2026-05-31 Vector Rendering Capability Planning

- Added `docs\vector-render-capability-spec.md` as the tracked XGE/XUI proxy plan for high-quality vector rendering.
- Recorded the design decision that smooth curves, dashed curves, high-quality area fills, SVG path subsets, complex icons, and similar vector-heavy UI should be built on shared XGE/proxy path or mesh capabilities instead of chart-local or widget-local rasterization.
- Updated `docs\render-capability-and-proxy-design.md` with the medium-term path/mesh direction.
- Updated `docs\chart-control-spec.md` so deferred smooth curves, area fill, and richer chart vector shapes depend on the shared vector rendering route.

Current direction:

- XGE owns efficient path/mesh primitives.
- XUI proxy exposes backend-neutral mesh/path capabilities and caps.
- XUI painter wraps those capabilities for controls.
- XUI widgets can use CPU tessellation fallback only as shared infrastructure, not private per-control code.

### 2026-05-31 Chart V1 SPEC Closure Slice

- Added chart padding APIs: `xuiChartSetPadding` and `xuiChartGetPadding`.
- Added chart-local dirty diagnostics: `XUI_CHART_DIRTY_STATIC`, `XUI_CHART_DIRTY_PLOT`, `XUI_CHART_DIRTY_OVERLAY`, and `xuiChartGetDirtyFlags`.
- Routed chart setters through chart-local dirty flags while preserving normal XUI widget cache invalidation.
- Expanded `xui_chart_test` so line, grouped bar, pie/donut, and scatter all have explicit hit-test coverage.
- Verified hidden bar series are excluded from hit testing.

Verification:

```bat
test_xui\build_chart_test.bat
```

### 2026-05-31 Chart Scatter Visual Mapping Slice

- Added scatter value-to-radius APIs: `xuiChartSetSeriesValueRadius`, `xuiChartGetSeriesValueRadius`, and `xuiChartClearSeriesValueRadius`.
- Added scatter value-to-color APIs: `xuiChartSetSeriesValueColor`, `xuiChartGetSeriesValueColor`, and `xuiChartClearSeriesValueColor`.
- Scatter render now maps `point.value` to symbol size and blended color when enabled; per-point `color` remains the highest-priority override.
- Updated `examples\xui_chart` so the scatter panel shows value-driven symbol size and color.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/examples/xui_chart/main.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Stacked Bar Slice

- Added bar mode constants: `XUI_CHART_BAR_GROUPED` and `XUI_CHART_BAR_STACKED`.
- Added chart-level bar mode APIs: `xuiChartSetBarMode` and `xuiChartGetBarMode`.
- Stacked bar mode now stacks visible bar series by item index, with positive and negative values accumulating separately from the zero baseline.
- Updated bar range calculation, rendering, and hit testing for stacked mode while preserving grouped mode as the default.
- Updated `examples\xui_chart` so the bar panel demonstrates stacked bars.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/examples/xui_chart/main.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Horizontal Bar Slice

- Added bar direction constants: `XUI_CHART_BAR_VERTICAL` and `XUI_CHART_BAR_HORIZONTAL`.
- Added chart-level direction APIs: `xuiChartSetBarDirection` and `xuiChartGetBarDirection`.
- Horizontal bars reuse `point.y` as the numeric value, mapping value to X and item/category index to Y.
- Horizontal grouped and horizontal stacked bars share the existing bar data path, range calculation, rendering, and hit-testing infrastructure.
- Updated `examples\xui_chart` so the bar panel demonstrates horizontal stacked bars.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/examples/xui_chart/main.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Multi-Ring Pie Slice

- Multiple visible pie series now render as concentric rings inside one plot area.
- Pie rendering now emits annular wedge triangles for ring slices instead of drawing full wedges and masking the center.
- Series order maps from inner ring to outer ring; `xuiChartSetPieInnerRadius` controls the hole inside the innermost ring.
- Pie hit testing now respects each ring's inner and outer radius and returns the matching series and slice.
- Updated `examples\xui_chart` so the pie panel demonstrates a two-ring pie chart.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/examples/xui_chart/main.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Rose Pie Slice

- Added pie mode constants: `XUI_CHART_PIE_NORMAL` and `XUI_CHART_PIE_ROSE`.
- Added chart-level pie mode APIs: `xuiChartSetPieMode` and `xuiChartGetPieMode`.
- Rose mode uses equal slice angles and maps each positive `point.value` to radius within the series ring.
- Rose mode shares the existing multi-ring pie rendering path, so multiple visible pie series produce multi-ring rose charts.
- Updated pie hit testing so rose slices respect each slice's value-mapped outer radius.
- Updated `examples\xui_chart` so the pie panel demonstrates a multi-ring rose chart.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/examples/xui_chart/main.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Zoom And Pan Slice

- Added manual cartesian view range APIs: `xuiChartSetViewRange`, `xuiChartGetViewRange`, and `xuiChartResetViewRange`.
- Cartesian charts still use automatic data ranges by default; wheel zoom or drag pan activates a manual view range.
- Mouse wheel over the plot zooms around the cursor.
- Pointer drag inside the plot pans the current cartesian view using pointer capture.
- Pie-only charts ignore wheel zoom and drag pan.
- Expanded `xui_chart_test` to cover manual range set/get/reset, wheel zoom range reduction, and drag pan range shift.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Brush Range Slice

- Added cartesian brush range APIs: `xuiChartSetBrushRange`, `xuiChartGetBrushRange`, and `xuiChartClearBrushRange`.
- Shift+drag inside the plot creates or updates the brush range; normal drag remains pan.
- Brush overlay renders as a translucent plot-local rectangle with an outline using existing rect fill/stroke proxy primitives.
- Brush ranges are stored in data coordinates and normalized on set.
- Pie-only charts ignore brush drag.
- Expanded `xui_chart_test` to cover programmatic set/get/clear and Shift+drag brush creation.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Large Data LOD Slice

- Added line/scatter LOD APIs: `xuiChartSetLodThreshold`, `xuiChartGetLodThreshold`, and `xuiChartGetLastLodStride`.
- A threshold of `0` disables LOD.
- When enabled, line and scatter rendering keep first/last points and draw every Nth intermediate point so draw volume stays near the threshold.
- Hit testing still uses the full copied data.
- Expanded `xui_chart_test` to render a 32-point line with threshold 8 and verify an active LOD stride.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

### 2026-05-31 Chart Animation Transition Slice

- Added opt-in transition APIs: `xuiChartSetAnimation`, `xuiChartGetAnimation`, and `xuiChartStepAnimation`.
- Data and view-range changes start a transition when animation is enabled.
- Rendering applies transition progress as a series alpha fade for line, bar, scatter, and pie/rose series.
- The control does not own a timer; callers advance animation per frame with `xuiChartStepAnimation`.
- Expanded `xui_chart_test` to cover transition activation, half-progress stepping, and completion.
- Marked the tracked deferred SPEC item complete.

Verification:

```bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
git diff --check -- dev/xui2/xui.h dev/xui2/src/xui_chart.c dev/xui2/test_xui/xui_chart_test.c dev/xui2/docs/chart-control-spec.md dev/xui2/docs/xui/widget-chart.md dev/xui2/docs/work.md
```

## 2026-05-30 DockPanel Migration

- Added `xuiDockPanel` typed widget with the XUI1 DockPanel core model: five fixed regions, explicit binary split tree, pane tab groups, and dock windows that host normal XUI widgets.
- Preserved the important XUI1 UX constraints for this first conservative slice: floating stays inside the XUI root, dock windows can move between docked/hidden/floating/auto-hide states, pane tabs activate content, pane close hides content, pin moves to auto-hide, and splitters drag continuously.
- Added public APIs for adding windows, setting clients/titles/flags/user data, docking to regions or panes, floating, hiding, auto-hide restore, pane activation, region sizing, metrics/colors, callbacks, hit testing, info queries, and diagnostic counters.
- Added files:
  - `src\xui_dock_panel.c`
  - `test_xui\xui_dock_panel_test.c`
  - `test_xui\build_dock_panel_test.bat`
  - `examples\xui_dockpanel\main.c`
  - `examples\xui_dockpanel\build.bat`
  - `docs\xui\widget-dockpanel.md`
- Current deferred items are native OS floating windows, XSON save/load, DockPanelSuite-style drag indicator overlays, drag-to-dock preview/commit, and tab overflow popup menus.

Verification on 2026-05-30:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_dock_panel_test.bat
examples\xui_dockpanel\build.bat
build\xui_dockpanel.exe --frames 3
```

The dedicated test passed. The example build passed and the `--frames` smoke exited successfully; the run still prints the existing bundled-resource libpng iCCP warning.

## 2026-05-30 DockPanel Second Slice

- Added pane option menus and tab overflow menus on top of the first conservative DockPanel core.
- Preserved the XUI1 user-facing behavior that matters here: a pane menu can activate tabs and issue Float / Auto hide / Close commands, while an overflowed tab strip exposes all tabs through a popup menu.
- Added overflow layout state to `xui_dock_pane_info_t`: visible tab range, overflow flag, option rect, and overflow rect.
- Added public menu APIs:
  - `xuiDockPanelGetOptionMenu`
  - `xuiDockPanelGetOverflowMenu`
  - `xuiDockPanelOpenPaneMenu`
  - `xuiDockPanelOpenOverflowMenu`
- Updated the DockPanel example to include enough document tabs to show overflow and to smoke-test both menus.

Verification on 2026-05-30:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_dock_panel_test.bat
examples\xui_dockpanel\build.bat
build\xui_dockpanel.exe --frames 3
```

The dedicated test passed. The example summary reached `create=1 layout=1 tab=1 menu=1 float=1 autohide=1 paint=1`; the run still prints the existing bundled-resource libpng iCCP warning.

## 2026-05-30 DockPanel Auto-Hide Overlay Slice

- Changed auto-hide strip interaction to match the XUI1/IDE behavior more closely: clicking an auto-hide strip now expands an overlay pane instead of immediately docking the window back.
- Added overlay caption rendering with pin and close affordances. The overlay keeps the dock window in `XUI_DOCK_PANEL_WINDOW_AUTO_HIDE` state; pin restores it to the dock tree and close hides it.
- Added public APIs for host tools and tests:
  - `xuiDockPanelExpandAutoHideWindow`
  - `xuiDockPanelCollapseAutoHide`
  - `xuiDockPanelGetAutoHideExpandedWindow`
  - `xuiDockPanelGetAutoHideExpandRect`
- Added `tAutoHideRect` to `xui_dock_window_info_t` so callers can inspect the strip rectangle without relying on internal state.
- Updated the dedicated test and example smoke to verify strip expansion, overlay visibility, collapse, and dock restore.

Verification on 2026-05-30:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_dock_panel_test.bat
examples\xui_dockpanel\build.bat
build\xui_dockpanel.exe --frames 3
```

The dedicated test passed. The example summary still reaches `create=1 layout=1 tab=1 menu=1 float=1 autohide=1 paint=1`; the run still prints the existing bundled-resource libpng iCCP warning.

## 2026-05-31 DockPanel Drag-Dock Slice

- Added the next XUI1 maturity step for DockPanel: floating-window drag-to-dock preview and drop commit.
- Drop targeting now separates panel-edge docking from pane targeting. Pane center drops tab into the pane; pane edge drops create a split beside the target pane.
- Added `xuiDockPanelDockWindowToPaneSide`, `xuiDockPanelFindDropTarget`, `xuiDockPanelGetDragPreview`, and `xui_dock_drop_info_t`.
- Added an XUI-drawn preview rectangle during drag. Bitmap/mask DockPanelSuite indicator art remains a later visual layer, not a blocker for the drop model.
- Updated the dedicated test and example smoke to cover target lookup, live drag preview, side split commit, and summary reporting.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_dock_panel_test.bat
examples\xui_dockpanel\build.bat
build\xui_dockpanel.exe --frames 3
```

The dedicated test passed. The example summary reached `create=1 layout=1 tab=1 menu=1 float=1 dragdock=1 autohide=1 paint=1`; the run still prints the existing bundled-resource libpng iCCP warning.

## 2026-05-31 DockPanel Dockable Flags Slice

- Tightened `bDockable` to match the XUI1 behavior contract instead of treating it as a mostly visual flag.
- Non-dockable windows now keep activation in their current pane, but public float/dock APIs, auto-hide restore, drag-out docking, and drop-target lookup reject them.
- Pane option menus now keep Float and Auto hide unavailable for non-dockable active windows, and menu command execution also guards against stale/manual invocation.
- Inactive non-dockable tabs now use muted text and border feedback, preserving the XUI1 hint that the tab is selectable but not dock-movable.
- Updated the dedicated test and example smoke so this behavior is verified together with the existing drag-dock and auto-hide coverage.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_dock_panel_test.bat
examples\xui_dockpanel\build.bat
build\xui_dockpanel.exe --frames 3
```

The dedicated test passed. The example summary reached `create=1 layout=1 tab=1 menu=1 dockable=1 float=1 dragdock=1 autohide=1 paint=1`; the run still prints the existing bundled-resource libpng iCCP warning.

## 2026-05-31 DockPanel Active Fallback Slice

- Aligned DockPanel active-change notifications with XUI1's pane active/focus fallback behavior.
- Pane add, remove, and reorder paths now notify when the active window changes implicitly, including hide/float/auto-hide flows that remove the active tab and select a sibling.
- Added a dedicated test that hides the active scratch tab, verifies fallback to the next sibling, verifies the active callback, and restores the original tab order.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_dock_panel_test.bat
examples\xui_dockpanel\build.bat
build\xui_dockpanel.exe --frames 3
```

The dedicated test passed. The example summary reached `create=1 layout=1 tab=1 menu=1 dockable=1 float=1 dragdock=1 autohide=1 paint=1`; the run still prints the existing bundled-resource libpng iCCP warning.

## 2026-05-31 CodeEdit Theme And Command Slice

- Continued the SPEC-driven CodeEdit implementation under `docs/xui-codeedit-spec.md`.
- Added the standalone CodeEdit theme module:
  - `src/xui_code_theme.c`
  - `xuiCodeThemeCreate`
  - `xuiCodeThemeSetDefault`
  - `xuiCodeThemeSetStyle`
  - `xuiCodeThemeGetStyle`
  - `xuiCodeThemeMapTokenKind`
  - `xuiCodeThemeGetTokenStyle`
- Added the standalone CodeEdit command map module:
  - `src/xui_code_command.c`
  - `xuiCodeCommandMapCreate`
  - `xuiCodeCommandMapLoadDefaults`
  - `xuiCodeCommandMapBind`
  - `xuiCodeCommandMapUnbind`
  - `xuiCodeCommandMapFind`
  - `xuiCodeCommandMapGetCount`
  - `xuiCodeCommandMapGetBinding`
- Added dedicated tests for both modules:
  - `test_xui/xui_code_theme_test.c`
  - `test_xui/build_code_theme_test.bat`
  - `test_xui/xui_code_command_test.c`
  - `test_xui/build_code_command_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
build_dll.bat
```

All six CodeEdit unit tests passed. `build_dll.bat` initially failed because an existing `build\xge.dll` could not be overwritten; after the stale DLL was moved aside, the DLL and import library rebuilt successfully.

## 2026-05-31 CodeEdit Search Slice

- Added the standalone CodeEdit search module:
  - `src/xui_code_search.c`
  - `xuiCodeSearchFindPlain`
  - `xuiCodeSearchFindRegex`
  - `xuiCodeSearchReplaceAllPlain`
  - `xuiCodeSearchReplaceAllRegex`
- Added search constants and result structs to `xui.h`, including case mode,
  whole-word, backward, wrap, multiline, dot-newline, and capture ranges.
- Regex search and replacement reuse XRT regex. Capture replacement supports
  `$0`-style and `\0`-style numbered captures.
- Added dedicated tests:
  - `test_xui/xui_code_search_test.c`
  - `test_xui/build_code_search_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
build_dll.bat
```

All seven CodeEdit unit tests passed, and `build_dll.bat` rebuilt `build\xge.dll`
and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Annotation Slice

- Added the standalone CodeEdit annotation store:
  - `src/xui_code_annotation.c`
  - `xuiCodeAnnotationStoreCreate`
  - `xuiCodeAnnotationSetMarker`
  - `xuiCodeAnnotationSetIndicator`
  - `xuiCodeAnnotationSetDiagnostics`
  - `xuiCodeAnnotationTrackEdit`
- Added marker, indicator, diagnostic constants and structs to `xui.h`.
- Marker data supports line/type uniqueness, tooltip copy, user data, line
  queries, and clearing.
- Indicator data supports range decorations, style type, user data, overlap
  clearing, offset queries, and basic edit tracking.
- Diagnostic data supports severity, range, code/message/source string copy,
  indexed lookup, offset lookup, and clearing.
- Added dedicated tests:
  - `test_xui/xui_code_annotation_test.c`
  - `test_xui/build_code_annotation_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
build_dll.bat
```

All eight CodeEdit unit tests passed, and `build_dll.bat` rebuilt `build\xge.dll`
and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Language And Token Buffer Slice

- Added the standalone CodeEdit language registry:
  - `src/xui_code_language.c`
  - `xuiCodeLanguageRegistryCreate`
  - `xuiCodeLanguageRegistryLoadDefaults`
  - `xuiCodeLanguageRegistryRegister`
  - `xuiCodeLanguageRegistryFind`
  - `xuiCodeLanguageRegistryFindByExtension`
  - `xuiCodeLanguageLex`
  - `xuiCodeLanguageFold`
- The default registry now exposes the built-in C language with `c` and `h`
  extensions, line/block comment metadata, C lexer dispatch, and C fold
  dispatch.
- Custom language registration supports callback lexer providers and regex
  lexer providers through the same registry API.
- Added the standalone external token buffer:
  - `src/xui_code_token_buffer.c`
  - `xuiCodeTokenBufferSet`
  - `xuiCodeTokenBufferGetTokens`
  - `xuiCodeTokenBufferGetTokensInRange`
- External tokens are version-gated so stale parser/LSP tokens are rejected
  before rendering.
- Added dedicated tests:
  - `test_xui/xui_code_language_test.c`
  - `test_xui/build_code_language_test.bat`
  - `test_xui/xui_code_token_buffer_test.c`
  - `test_xui/build_code_token_buffer_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
build_dll.bat
```

All ten CodeEdit unit tests passed, and `build_dll.bat` rebuilt `build\xge.dll`
and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Fold State Slice

- Added the standalone CodeEdit fold state module:
  - `src/xui_code_fold_state.c`
  - `xuiCodeFoldStateSetRanges`
  - `xuiCodeFoldStateBuildFromProvider`
  - `xuiCodeFoldStateToggleLine`
  - `xuiCodeFoldStateFoldAll`
  - `xuiCodeFoldStateUnfoldAll`
  - `xuiCodeFoldStateIsLineVisible`
  - `xuiCodeFoldStateBuildVisibleLines`
- Custom fold providers now have a document-level execution path before the
  widget exists.
- Fold commands are represented as state operations, so the future fold margin
  and command map can reuse this module directly.
- Added dedicated tests:
  - `test_xui/xui_code_fold_state_test.c`
  - `test_xui/build_code_fold_state_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
build_dll.bat
```

All eleven CodeEdit unit tests passed, and `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Provider Hook Slice

- Added the standalone CodeEdit provider module:
  - `src/xui_code_provider.c`
  - `xuiCodeProviderSetCompletion`
  - `xuiCodeProviderSetHover`
  - `xuiCodeProviderSetSignature`
  - `xuiCodeProviderSetCommand`
  - `xuiCodeProviderRequestCompletion`
  - `xuiCodeProviderRequestHover`
  - `xuiCodeProviderRequestSignature`
  - `xuiCodeProviderExecuteCommand`
- Added public provider structs and callback types for completion items, hover
  payloads, signature help, and command interception.
- Provider UI presentation is intentionally left to the future CodeEdit widget
  layer; this slice closes storage and dispatch behavior.
- Added dedicated tests:
  - `test_xui/xui_code_provider_test.c`
  - `test_xui/build_code_provider_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
build_dll.bat
```

All twelve CodeEdit unit tests passed, and `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Margin Model Slice

- Added the standalone CodeEdit margin model:
  - `src/xui_code_margin.c`
  - `xuiCodeMarginModelLoadDefaults`
  - `xuiCodeMarginModelAdd`
  - `xuiCodeMarginModelRemove`
  - `xuiCodeMarginModelSetWidth`
  - `xuiCodeMarginModelSetVisible`
  - `xuiCodeMarginModelLayout`
  - `xuiCodeMarginModelHitTest`
- Added public margin constants, descriptors, layout info, hit-test info, and
  custom render/event callback fields to `xui.h`.
- The default margin order is now represented as data: change, marker, fold,
  line number, diagnostic, then text viewport.
- This closes margin storage, width, visibility, layout, and hit geometry. The
  actual drawing and event dispatch remain in the future CodeEdit widget layer.
- Added dedicated tests:
  - `test_xui/xui_code_margin_test.c`
  - `test_xui/build_code_margin_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
build_dll.bat
```

All thirteen CodeEdit unit tests passed, and `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Search Indicator Slice

- Added search-to-indicator bridge APIs:
  - `xuiCodeSearchClearResultIndicators`
  - `xuiCodeSearchMarkAllPlain`
  - `xuiCodeSearchMarkAllRegex`
- Added `XUI_CODE_INDICATOR_SEARCH_RESULT` as the default indicator id for
  highlighted search matches.
- Plain and regex search results now write into the annotation indicator store,
  preserving unrelated indicator ids and allowing targeted clearing.
- Updated `test_xui/build_code_search_test.bat` to link `src/xui_code_annotation.c`
  because the search module now exposes annotation-backed indicator APIs.
- Added dedicated tests:
  - `test_xui/xui_code_search_indicator_test.c`
  - `test_xui/build_code_search_indicator_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
build_dll.bat
```

All fourteen CodeEdit unit tests passed, and `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Selection And Caret Slice

- Added the standalone CodeEdit selection model:
  - `src/xui_code_selection.c`
  - `xuiCodeSelectionSetRange`
  - `xuiCodeSelectionGetRange`
  - `xuiCodeSelectionGetState`
  - `xuiCodeSelectionSelectAll`
  - `xuiCodeSelectionGotoOffset`
  - `xuiCodeSelectionGotoLineColumn`
  - `xuiCodeSelectionMove`
- Added public selection flags, selection state struct, selection model handle,
  and selection/navigation APIs to `xui.h`.
- The model supports one primary selection today while keeping the public API
  selection-count based for future multi-selection.
- Implemented navigation state for left/right, word left/right, line start/end,
  document start/end, up/down with preferred column, and shift-extension
  selection commands. Page navigation remains widget-dependent because it needs
  viewport metrics.
- Added dedicated tests:
  - `test_xui/xui_code_selection_test.c`
  - `test_xui/build_code_selection_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
build_dll.bat
```

All fifteen CodeEdit unit tests passed, and `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Editing Command Slice

- Added the standalone CodeEdit editing module:
  - `src/xui_code_editing.c`
  - `xuiCodeEditingInsertText`
  - `xuiCodeEditingDeleteBackward`
  - `xuiCodeEditingDeleteForward`
  - `xuiCodeEditingIndentSelection`
  - `xuiCodeEditingOutdentSelection`
  - `xuiCodeEditingToggleLineComment`
- Editing commands operate on `xui_code_document` and
  `xui_code_selection_model`, leaving widget input/event routing separate.
- Implemented document-level insertion, replace-selection, Backspace, Delete,
  readonly rejection, indent/outdent, and line-comment toggling.
- Added dedicated tests:
  - `test_xui/xui_code_editing_test.c`
  - `test_xui/build_code_editing_test.bat`

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
build_dll.bat
```

All sixteen CodeEdit unit tests passed, and `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 Chart Vector Rendering Phase 0 Contract

- Closed Phase 0 of `docs/vector-render-capability-spec.md` with a repo-backed
  audit of current XGE and XUI proxy rendering capabilities.
- Confirmed current XUI proxy exposes primitive drawing and surface quad calls,
  but no generic mesh or path API.
- Confirmed current XGE has immediate shape primitives, `xgeShapeBatch...`
  same-color triangle batching, `xgeDrawQuad3D`, and internal dynamic
  vertex/index upload paths suitable for a future colored mesh primitive.
- Decided the next durable route is mesh first, then painter-level path
  tessellation over mesh, with a later native XGE path API only if profiling or
  quality requires it.
- Defined proxy capability flags, fallback behavior, coordinate/winding/fill
  rule/stroke/dash/AA expectations for the future mesh/path implementation.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
```

The chart example rebuilt and ran for five frames successfully. The run still
prints two existing libpng iCCP warnings from image metadata.

## 2026-05-31 XUI Mesh Proxy Slice

- Added the XUI mesh contract:
  - `xui_mesh_vertex_t`
  - `XUI_PROXY_CAP_MESH_TRIANGLES`
  - `xui_draw_mesh_triangles_proc`
  - `xui_proxy_t.drawMeshTriangles`
  - `xuiPainterDrawMeshTriangles`
- Added the planned path capability flags for future fill/stroke/dash/AA
  reporting.
- Updated the shared XUI test proxy to advertise mesh support and record mesh
  call counts and payload sizes.
- Added `xge_shape_vertex_t`, `xgeShapeMeshFill`, and `xgeShapeMeshFillPx` on
  the dev XGE layer, backed by the existing shape auto-batch vertex/index path.
- Updated the XUI XGE proxy to advertise mesh support and forward
  `drawMeshTriangles` to `xgeShapeMeshFillPx`.
- Added a mesh smoke assertion to `test_xui/xui_chart_test.c`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
build_dll.bat
test_xui\build_chart_test.bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
```

`build_dll.bat`, the chart test, the chart example build, and the five-frame
chart example smoke all passed. The example still prints two existing libpng
iCCP warnings from image metadata.

## 2026-05-31 XUI Path Builder And Fill Mesh Slice

- Added the XUI path data model:
  - `xui_path`
  - `xui_path_command_t`
  - `XUI_PATH_CMD_MOVE`
  - `XUI_PATH_CMD_LINE`
  - `XUI_PATH_CMD_QUAD`
  - `XUI_PATH_CMD_CUBIC`
  - `XUI_PATH_CMD_CLOSE`
- Added path lifecycle and builder APIs:
  - `xuiPathCreate`
  - `xuiPathDestroy`
  - `xuiPathClear`
  - `xuiPathMoveTo`
  - `xuiPathLineTo`
  - `xuiPathQuadTo`
  - `xuiPathCubicTo`
  - `xuiPathClose`
  - `xuiPathGetCommandCount`
  - `xuiPathGetCommand`
- Added `xuiPathFlatten` for quadratic/cubic flattening and
  `xuiPathBuildFillMesh` for simple filled path meshes.
- Extended the chart test mesh smoke to build a closed mixed line/quadratic/
  cubic path, flatten it, generate a fill mesh, and submit it through
  `xuiPainterDrawMeshTriangles`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 Vector Smoke Regression Slice

- Added `test_xui/xui_vector_smoke_test.c` as a focused vector-heavy
  regression entrypoint.
- Added `test_xui/build_vector_smoke_test.bat`.
- The smoke test covers:
  - filled cubic/quadratic path mesh generation
  - round-cap and round-join stroke rendering
  - dashed square-cap stroke rendering
  - mesh proxy call count and payload-size validation
- This gives later SVG path, vector icon, and anti-aliasing work a small
  repeatable validation target independent of the chart test.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_vector_smoke_test.bat
```

The vector smoke test rebuilt and passed.

## 2026-05-31 CodeEdit Multi-Selection Foundation Slice

- Continued from the tracked XUI2 spec backlog after chart/vector reached the
  current dependency boundary.
- Extended `xui_code_selection_model_t` from a primary-only selection to a
  primary selection plus XRT-managed additional inactive selections.
- Implemented `xuiCodeSelectionAdd`, multi-selection counts, indexed selection
  access, and reset behavior when primary caret/range commands replace the
  active selection.
- Updated CodeEdit rendering so selected ranges and collapsed carets are drawn
  for every selection in the model.
- Kept editing commands primary-selection based for this slice; multi-range
  edit application remains tracked in `docs\xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_selection_test.bat
test_xui\build_code_edit_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 Chart XSON Deferral Status

- Rechecked `docs\chart-control-spec.md` and
  `docs\vector-render-capability-spec.md`; vector rendering tasks are closed
  and the only remaining chart item is the optional XSON/chart description.
- Marked the XSON/chart description item as `[~]` because it is intentionally
  dependency-blocked by standard control migration and the shared XSON loader
  pass.
- No chart-only parser or schema loader was added. Chart should consume the
  shared XRT/XSON route when that system is available.

## 2026-05-31 Path Fill Rule Slice

- `xuiPainterDrawPath` now consumes `xui_path_style_t.iFillRule` for fill
  styles instead of ignoring it.
- Added fill-rule validation for:
  - `XUI_PATH_FILL_NON_ZERO`
  - `XUI_PATH_FILL_EVEN_ODD`
- V1 behavior supports both rules on simple filled paths through the existing
  triangle-fan mesh route. Complex compound paths with holes remain a future
  tessellator improvement, not a widget-local workaround.
- Extended `test_xui/xui_vector_smoke_test.c` to exercise even-odd fill style
  through `xuiPainterDrawPath`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_vector_smoke_test.bat
```

The vector smoke test rebuilt and passed.

## 2026-05-31 Input Vector Icon Adoption Slice

- Extended the built-in vector icon catalog with input-oriented icons:
  - `user`
  - `lock`
  - `eye`
- Updated input decoration rendering so search, user, lock, eye, and clear
  decorations prefer vector icon drawing when `drawMeshTriangles` is available.
- Kept the existing atlas-backed bitmap assets and clear-icon line fallback for
  proxies without mesh/path support.
- Extended `test_xui/xui_input_widget_test.c` to assert that decoration
  rendering submits vector icon mesh draws.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_input_widget_test.bat
```

The input widget test rebuilt and passed.

## 2026-05-31 Vector Anti-Alias Strategy Slice

- Closed the tracked anti-alias strategy item in
  `docs/vector-render-capability-spec.md`.
- The strategy is explicit:
  - current mesh fallback is deterministic and may be aliased
  - `XUI_PROXY_CAP_PATH_AA` is a quality capability, not a correctness
    requirement
  - medium-term AA should be shared geometry-fringe tessellation for fills and
    strokes with premultiplied alpha falloff
  - future native XGE path AA can replace painter-level fringe generation
    without changing widget APIs
  - visual AA validation belongs in backend-specific visual regression once an
    AA-capable backend path exists
- No runtime behavior changed in this slice.

## 2026-05-31 Vector Icon Path API Slice

- Added a small built-in vector icon catalog backed by SVG-subset path data:
  - `check`
  - `close`
  - `chevron_down`
  - `search`
- Added public vector icon APIs:
  - `xuiVectorIconGetCount`
  - `xuiVectorIconGetName`
  - `xuiPainterDrawVectorIcon`
- `xuiPainterDrawVectorIcon` maps the icon viewBox into the requested rect,
  imports the icon path through `xuiPathParseSvg`, and renders it through
  the shared `xuiPainterDrawPath` mesh route.
- Extended `test_xui/xui_vector_smoke_test.c` to draw vector icons and verify
  mesh submissions.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_vector_smoke_test.bat
```

The vector smoke test rebuilt and passed.

## 2026-05-31 SVG Path Subset Import Slice

- Added `xuiPathParseSvg` to import a practical SVG path subset into the
  existing `xui_path` builder.
- Supported commands in this slice:
  - absolute and relative `M`
  - absolute and relative `L`
  - absolute and relative `H`
  - absolute and relative `V`
  - absolute and relative `Q`
  - absolute and relative `C`
  - `Z`
- The parser accepts repeated command parameter groups and comma/whitespace
  separators, then routes everything through the existing path builder rather
  than adding a parallel vector representation.
- Extended `test_xui/xui_vector_smoke_test.c` to draw SVG-subset imported
  paths through `xuiPainterDrawPath`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_vector_smoke_test.bat
```

The vector smoke test rebuilt and passed.

## 2026-05-31 CodeEdit Integration Readiness Slice

- Extended `test_xui/xui_code_edit_test.c` so CodeEdit is verified as an
  embedded child in Panel, Tabs, and DockPanel containers.
- Added a read-only debugger-source scenario with breakpoint marker,
  execution-line marker, diagnostic data, copy command dispatch, and blocked
  text mutation.
- Added an IDE command integration check where a host command is bound through
  the CodeEdit command map and handled by the provider command callback.
- Updated `test_xui/build_code_edit_test.bat` with the extra container control
  sources required by this integration smoke.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget integration test rebuilt and passed.

## 2026-05-31 CodeDocument Piece Table Slice

- Reworked `src/xui_code_document.c` so the document owns an original buffer,
  append buffer, and piece list internally.
- `SetText` now seeds the original buffer and piece list; insert, delete, and
  replace append new text into the add buffer and splice the piece list before
  exporting the current contiguous text view for existing CodeEdit callers.
- Undo/redo snapshots restore through the same piece-table storage path, so
  existing version, dirty, line-index, UTF-8, and transaction behavior is kept.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_edit_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 CodeEdit Font Metrics Fallback Slice

- Added CodeEdit metric helpers that derive line height from
  `fontGetMetrics` and column width from `textMeasure("M")`, with existing
  fixed metrics retained as the lower-bound fallback.
- IME candidate placement, pointer hit testing, token spans, whitespace/EOL
  markers, selection rectangles, margin row rectangles, text rows, and caret
  rendering now consume the resolved metrics instead of hard-coded row/column
  values.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Syntax Style Properties Slice

- Added `codeedit.syntax.*.color` style properties for keyword, type, number,
  string, char, comment, preprocessor, operator, brace, and error token kinds.
- Token rendering now resolves syntax style properties before falling back to
  the CodeEdit theme table.
- Extended the CodeEdit widget test to verify inline syntax style override on
  rendered token output.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Public API Surface Closure

- Closed the public constants, structs, handles/callbacks, and function
  declaration spec items after the CodeEdit widget test and example continued
  compiling against `xui.h`.
- The public surface now includes the widget-level theme override APIs:
  `xuiCodeEditSetTheme` and `xuiCodeEditSetStyle`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed against the public header.

## 2026-05-31 CodeEdit Error String API Closure

- Extended `test_xui/xui_code_edit_test.c` to verify widget-level invalid
  UTF-8 error propagation through `xuiCodeEditGetLastError`.
- Verified the widget error string clears after a successful text update.
- Closed the error string API spec item alongside the existing document and
  regex search error coverage.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 Code Annotation Edit Tracking Closure

- Extended `test_xui/xui_code_annotation_test.c` to cover overlapping delete
  tracking after an earlier insertion shift.
- Marker lines collapse to the edited line, while indicators and diagnostics
  shrink or collapse according to the existing annotation range policy.
- Closed the CodeEdit spec marker/indicator edit-tracking item against the
  annotation-store behavior and tests.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_annotation_test.bat
```

The annotation test rebuilt and passed.

## 2026-05-31 CodeEdit Theme Override API Slice

- Added `xuiCodeThemeCopy`.
- Added widget-level `xuiCodeEditSetTheme` and `xuiCodeEditSetStyle` APIs.
- Both CodeEdit theme override paths invalidate style/cache/render state and
  repaint token spans without rebuilding the document.
- Extended the CodeEdit widget test to verify theme-copy override and direct
  widget style override on rendered C token output.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Indent Guide Display Slice

- Added indentation guide rendering under
  `XUI_CODE_EDIT_SHOW_INDENT_GUIDES`.
- Indent guides share the `codeedit.whitespace.color` style property with
  whitespace and EOL markers.
- Extended the CodeEdit widget test to enable whitespace, EOL, and indent
  guide display together.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Fold-Visible Line Mapping Slice

- CodeEdit caret placement, pointer hit testing, margin hit dispatch, and
  rendering now map between document lines and fold-visible rows.
- Collapsed fold ranges suppress hidden rows from text and line-number margin
  painting.
- The line-number margin now highlights the active caret line.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Custom Margin Event Slice

- Added CodeEdit margin hit dispatch for pointer down/up events.
- Custom margin `onEvent` callbacks now receive the widget, margin id, line,
  original XUI event id, line-local margin rectangle, and user data.
- Default marker and diagnostic margin clicks are consumed before text
  selection, and fold margin pointer down toggles the clicked fold header.
- Extended `test_xui/xui_code_edit_test.c` to verify custom margin render and
  pointer-event callbacks plus default fold/marker margin hit routing.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Visible-Line Virtualization Slice

- Updated CodeEdit cache rendering to derive the first visible row from
  `scrollY / lineHeight`, including a one-line lookbehind buffer.
- Rendering no longer walks from line 0 for deeply scrolled documents before
  reaching visible rows.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Widget Event Wiring Slice

- Wired CodeEdit widget focus/blur, key-down, text, and command events through
  `xuiWidgetSetEventHandler`.
- CodeEdit now routes default key bindings through `xuiCodeCommandMapFind` and
  `xuiCodeCommandExecute`, using its owned document, selection, providers,
  fold state, readonly flag, and indent settings.
- Text events insert into the document through `xuiCodeEditingInsertText` and
  respect readonly/Control-modified input.
- Added default Backspace/Delete command bindings to the CodeEdit command map.
- Extended `test_xui/xui_code_edit_test.c` to verify focus, text input,
  key-command deletion, Tab insertion, command-dispatch select-all, and
  readonly blocking.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_command_test.bat
test_xui\build_code_edit_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 CodeEdit IME Foundation Slice

- CodeEdit now enables IME mode during widget initialization.
- Added a candidate-rectangle callback based on the current caret line/column
  and the widget world rectangle.
- Routed `XUI_EVENT_IME_COMPOSITION` through the existing CodeEdit editing path
  so composition text respects readonly and selection replacement behavior.
- Extended `test_xui/xui_code_edit_test.c` to verify IME mode, candidate rect
  availability, and composition text insertion.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

The CodeEdit related test matrix rebuilt and passed, and `build_dll.bat`
completed successfully.

## 2026-05-31 CodeEdit Token Matrix Rerun

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

The menu test, all CodeEdit tests, and `build_dll.bat` passed.

## 2026-05-31 CodeEdit Rendering Matrix Rerun

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

The menu test, all CodeEdit tests, and `build_dll.bat` passed.

## 2026-05-31 CodeEdit Token Rendering Slice

- CodeEdit now refreshes its token buffer from the built-in C lexer during
  cache rendering when the document version changes.
- Visible token spans are overlaid with colors resolved through
  `xuiCodeThemeGetTokenStyle`.
- The CodeEdit widget test now verifies token-buffer population and a
  theme-colored token draw after changing the type token style.
- The CodeEdit widget build script now includes `src\xui_code_lexer_c.c`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_style_test.bat
test_xui\build_code_edit_test.bat
examples\xui_codeedit\build.bat
build_dll.bat
build\xui_codeedit.exe --frames 3
```

The style system test, CodeEdit widget test, CodeEdit example build/run, and
DLL build passed.

## 2026-05-31 CodeEdit Scroll Matrix Rerun

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

The menu test, all CodeEdit tests, and `build_dll.bat` passed.

## 2026-05-31 CodeEdit Base Rendering Slice

- Added CodeEdit default layout and cache policy registration.
- Added a CodeEdit cache render callback that paints:
  - base background
  - a divider
  - visible text lines
  - selection range fill
  - primary caret
- Rendering accounts for CodeEdit scroll offsets.
- Extended `test_xui/xui_code_edit_test.c` to create a font and target surface,
  render CodeEdit, and verify cache-surface rectangle/text drawing plus
  selection fill rendering.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
examples\xui_codeedit\build.bat
build_dll.bat
build\xui_codeedit.exe --frames 3
```

The CodeEdit widget test, CodeEdit example build/run, and DLL build passed.

## 2026-05-31 CodeEdit Menu Matrix Rerun

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

The menu test, all CodeEdit tests, and `build_dll.bat` passed.

## 2026-05-31 CodeEdit Scroll State Slice

- Added public CodeEdit scroll helpers:
  - `xuiCodeEditSetScroll`
  - `xuiCodeEditGetScroll`
- CodeEdit now stores horizontal and vertical scroll offsets, clamps negative
  values, and invalidates layout/cache/render when scroll changes.
- Pointer wheel events update scroll offsets.
- Caret hit testing now accounts for scroll offsets.
- Extended `test_xui/xui_code_edit_test.c` to verify scroll get/set, clamping,
  wheel updates, and scroll-aware pointer hit testing.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Matrix Rerun

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

All CodeEdit tests rebuilt and passed; `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Context Menu Slice

- Added an owned XUI Menu to CodeEdit and destroy handling for its popup/menu
  widget.
- Added public helpers:
  - `xuiCodeEditGetMenuWidget`
  - `xuiCodeEditOpenMenu`
- Context-menu events now focus CodeEdit, place the caret when there is no
  active selection, and open the menu.
- Menu items cover undo, redo, cut, copy, paste, delete, select all, find,
  replace, go to line, toggle comment, and toggle fold.
- Menu selection routes through the existing CodeEdit command execution path.
- Extended `test_xui/xui_code_edit_test.c` and its build script to verify menu
  creation, API opening, item state, readonly enable state, right-click opening,
  and delete execution.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Pointer Selection Slice

- Added left-button pointer down/move/up handling to CodeEdit.
- Pointer down focuses the widget, captures the pointer, and places the caret
  using a monospace line/column hit path.
- Pointer drag extends the primary selection through `xuiCodeSelectionSetRange`.
- Pointer up releases capture; capture-lost clears the internal drag flag.
- Extended `test_xui/xui_code_edit_test.c` to verify pointer focus, capture,
  caret placement, drag range selection, and capture release.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 Chart Pie Sector Mesh Slice

- Updated pie and rose chart rendering to use the shared triangle mesh proxy
  path when `drawMeshTriangles` is available.
- Each positive pie/rose slice now submits one deterministic mesh:
  - full pie slices use a center fan
  - donut and multi-ring slices use connected outer/inner arc quads
- Backends without mesh support keep the previous per-segment
  `drawTriangleFill` fallback.
- Extended `test_xui/xui_chart_test.c` to assert that pie rendering produces
  mesh draw calls.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 Path Stroke Join Slice

- Added join geometry to the shared solid path stroke mesh route used by
  `xuiPainterDrawPath`.
- `xui_path_style_t.iLineJoin` now supports:
  - `XUI_PATH_JOIN_MITER`
  - `XUI_PATH_JOIN_BEVEL`
  - `XUI_PATH_JOIN_ROUND`
- Miter and bevel joins add corner connector triangles; round joins add a
  deterministic triangle-fan arc at the outer corner.
- Dashed strokes intentionally keep independent dash segments for now, so
  visible dashes are not connected across gaps.
- Extended `test_xui/xui_chart_test.c` with bevel-join and round-join painter
  stroke smoke checks.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 Path Stroke Cap Slice

- Added stroke cap handling to the shared path stroke mesh route used by
  `xuiPainterDrawPath`.
- `xui_path_style_t.iLineCap` now supports:
  - `XUI_PATH_CAP_BUTT`
  - `XUI_PATH_CAP_SQUARE`
  - `XUI_PATH_CAP_ROUND`
- Round caps are generated as small triangle-fan semicircles, so widgets and
  charts get the behavior through the same proxy mesh primitive.
- Extended `test_xui/xui_chart_test.c` with square-cap and round-cap painter
  stroke smoke checks.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 Path Dash And Chart Dashed Line Slice

- Added `xuiPathBuildDashedStrokeMesh` for dash-aware width stroke
  tessellation over flattened paths.
- `xuiPainterDrawPath` now honors `xui_path_style_t.pDashPattern`,
  `iDashCount`, and `fDashOffset`.
- Added line-series dash APIs:
  - `xuiChartSetSeriesDash`
  - `xuiChartClearSeriesDash`
  - `xuiChartGetSeriesDash`
- Dashed chart lines use the same path/mesh stroke route as smooth lines.
- Updated the chart example to render the second line series with a dash
  pattern.
- Updated chart/vector docs and tests.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 CodeEdit Command Dispatch Slice

- Added `xui_code_command_context_t` and `xuiCodeCommandExecute` so command ids
  can route through provider interception and then default editor behavior.
- Routed default commands for navigation, delete/backspace, word delete,
  newline/tab insertion, indent/outdent, line comment toggle, undo/redo, and
  fold toggle/fold-all/unfold-all.
- Added command-context clipboard routing through the existing XUI proxy
  contract for copy, cut, and paste, including multiline paste normalization via
  the document editing path.
- Kept find/replace UI, goto dialog, completion popup, and signature popup
  commands explicitly unsupported until widget/popup UI routes land.
- Added word-delete editing APIs with editor-specific word segment behavior so
  forward delete does not consume following fold/newline structure.
- Extended the command test to cover provider interception, command-driven
  navigation/editing/folding, clipboard copy/cut/paste, and unsupported
  UI-dependent command reporting.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
build_dll.bat
```

All listed CodeEdit tests rebuilt and passed. `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Widget Lifecycle Slice

- Added `src/xui_code_edit.c` with the `codeedit` widget type.
- `xuiCodeEditGetType` and `xuiCodeEditCreate` now register/create a widget
  that owns the core editor state:
  - document
  - selection
  - theme
  - fold state
  - annotation store
  - token buffer
  - provider set
  - margin model
  - command map
- Added public widget accessors plus text, readonly, and widget error APIs.
- The widget currently exposes lifecycle/state ownership only; rendering,
  input routing, scroll frame integration, popup/menu integration, and IME are
  still tracked as pending widget tasks.
- Added `test_xui/xui_code_edit_test.c` and
  `test_xui/build_code_edit_test.bat` to verify type registration/reuse,
  create/destroy, root embedding, owned handles, initial text, readonly state,
  default margins, default command map, text reset, and document edit range.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

All listed CodeEdit tests rebuilt and passed. `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Document Affected Range Slice

- Added `xuiCodeDocumentGetLastEditRange` so consumers can query the latest
  affected document range after set/insert/delete/replace/undo/redo.
- Transaction edits now aggregate affected ranges from all edits between
  `xuiCodeDocumentBeginEdit` and `xuiCodeDocumentEndEdit`.
- Single edits replace the previous affected range, so incremental consumers
  can treat the value as the latest invalidation span.
- Extended `test_xui/xui_code_document_test.c` with insert, delete, replace,
  and grouped transaction affected range coverage.
- Updated the CodeEdit SPEC edit-transaction status to record affected range
  tracking; widget callback notification remains pending.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
build_dll.bat
```

All listed CodeEdit tests rebuilt and passed. `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Monospaced Layout Fast Path Slice

- Made `xui_code_layout.c` use cached `fCharWidth` and `fLineHeight` as the
  explicit monospaced fast path for content width, visible line rectangles,
  caret rectangles, and hit testing.
- Added tab-aware visual column expansion using `iTabColumns`, defaulting to
  four columns when unspecified.
- Hit testing now maps from visual columns back to document columns through the
  same tab policy used by caret placement.
- Extended `test_xui/xui_code_layout_test.c` with tab width/content size,
  caret, and hit-test coverage.
- Updated the CodeEdit SPEC so monospaced fast path is now `[X]`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
build_dll.bat
```

All listed CodeEdit tests rebuilt and passed. `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Word and Line Selection Slice

- Added `xuiCodeSelectionSelectWord` for state-level word selection based on
  identifier characters (`isalnum` plus underscore).
- Added `xuiCodeSelectionSelectLine` with optional line-break inclusion so
  future line-number clicks and triple-click selection can share the same
  selection logic.
- Separator offsets intentionally produce an empty word selection; pointer
  hit-testing can pass the actual word-character offset when a word is hit.
- Extended `test_xui/xui_code_selection_test.c` to cover word selection, empty
  separator selection, line text selection, and line-with-break selection.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
build_dll.bat
```

All listed CodeEdit tests rebuilt and passed. `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 CodeEdit Plain Search Range Slice

- Added `xuiCodeSearchFindPlainRange` as the public range-limited plain search
  API used by future selection-only search wiring.
- The range API supports forward, backward, wrap, case-sensitive, and
  whole-word matching while clamping and normalizing reversed ranges.
- Extended `test_xui/xui_code_search_test.c` with range forward/backward/wrap
  coverage and an exclusion check to ensure matches outside the range are not
  returned.
- Updated the CodeEdit SPEC so plain-text search is now `[X]`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
build_dll.bat
```

All listed CodeEdit tests rebuilt and passed. `build_dll.bat` rebuilt
`build\xge.dll` and `build\xge.lib` successfully.

## 2026-05-31 Chart Smooth Line Adoption

- Added line-series smooth APIs:
  - `xuiChartSetSeriesSmooth`
  - `xuiChartGetSeriesSmooth`
- Smooth line rendering builds a cubic path from the sampled line points and
  renders it through `xuiPathBuildStrokeMesh` and proxy mesh drawing.
- Backends without mesh support continue to use the existing straight-line
  rendering path.
- Updated the chart example to enable both smooth line and area fill on the
  first line series.
- Updated the chart user docs and tracking specs.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 CodeEdit Whitespace/EOL Display Slice

- Added CodeEdit display option flags for whitespace, EOL marks, and tracked
  indentation guides.
- Added `xuiCodeEditSetDisplayOptions` and `xuiCodeEditGetDisplayOptions`.
- The widget cache renderer now draws ASCII markers for spaces, tabs, and EOL
  when enabled.
- Extended the CodeEdit widget test to verify display option defaults, toggling,
  marker rendering, and marker color.
- Updated `xui-codeedit-spec.md`; indentation guides and style-property
  customization remain tracked.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_menu_test.bat
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
```

The CodeEdit related test matrix rebuilt and passed, and `build_dll.bat`
completed successfully.

## 2026-05-31 CodeEdit User Documentation Slice

- Added `docs/xui/widget-codeedit.md` with widget goals, creation, public API,
  editing behavior, display options, menu behavior, syntax/token model,
  search/annotation/provider/theme notes, verification commands, and current
  scope.
- Linked the widget document from `docs/README.md`.
- Updated `xui-codeedit-spec.md` to mark user documentation and docs index
  tracking complete.

## 2026-05-31 CodeEdit Example Slice

- Added `examples/xui_codeedit/main.c`, a headless smoke example that creates
  a CodeEdit with C source, display options, menu, marker, indicator, fold,
  search, command dispatch, input, selection, render, and token checks.
- Added `examples/xui_codeedit/build.bat`.
- Updated `xui-codeedit-spec.md` and `docs/xui/widget-codeedit.md` for example
  verification.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
examples\xui_codeedit\build.bat
build\xui_codeedit.exe --frames 3
```

Result:

```text
xui_codeedit summary: document=1 lexer=1 render=1 input=1 selection=1 margin=1 marker=1 indicator=1 fold=1 search=1 command=1
```

## 2026-05-31 CodeEdit Style Property Slice

- Registered core `codeedit.*` style properties for text, readonly text,
  background, divider, selection, caret, whitespace markers, and inherited
  `font.name`.
- CodeEdit cache rendering now resolves core color properties through the XUI2
  style system.
- Extended the CodeEdit widget test to set an inline
  `codeedit.whitespace.color` property and verify that marker rendering uses
  the styled color.
- Updated the CodeEdit SPEC and widget user document with the current property
  list and remaining style coverage.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Margin Rendering Slice

- CodeEdit cache rendering now lays out the margin model and shifts the text
  origin after visible margins.
- Hit testing and IME candidate rectangles account for the total margin width.
- Widget paint now draws line-number text, marker margin blocks, fold header
  glyphs, diagnostic severity blocks, and custom margin render callbacks.
- Extended the CodeEdit widget test with marker, diagnostic, fold state, margin
  render count, and margin-aware pointer hit testing coverage.
- Updated the CodeEdit SPEC and user documentation.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Language Metadata Command Slice

- CodeEdit now owns an `xui_code_language_registry`, loads default C language
  metadata, and exposes it with `xuiCodeEditGetLanguageRegistry`.
- The widget command context resolves `sLineComment` from the active
  `sLanguage` value instead of hardcoding `//`.
- Extended the CodeEdit widget test with a custom `toy` language registered on
  the widget registry; `XUI_CODE_COMMAND_TOGGLE_LINE_COMMENT` now inserts `#`
  for that language.
- Updated the CodeEdit SPEC and widget user document.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_language_test.bat
test_xui\build_code_edit_test.bat
examples\xui_codeedit\build.bat
build_dll.bat
build\xui_codeedit.exe --frames 3
```

All commands passed.

## 2026-05-31 CodeEdit Replace-All Integration Slice

- Added CodeEdit-level replace-all APIs:
  - `xuiCodeEditReplaceAllPlain`
  - `xuiCodeEditReplaceAllRegex`
- The widget APIs reuse the document/search layer, reject readonly edits, update the primary selection from the document edit range, and refresh search-result indicators for replacements.
- Updated `test_xui/build_code_edit_test.bat` so the widget test links the search module now required by CodeEdit.
- Extended the widget test to cover plain whole-word replace-all, regex capture replace-all, selection updates, and replacement indicators.
- Marked replace and replace-all complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_search_test.bat
test_xui\build_code_edit_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 CodeEdit Language Comment Slice

- Added `xuiCodeEditSetLanguage` and `xuiCodeEditGetLanguage`.
- `xuiCodeEditSetLanguage` validates registered language ids and reports a widget error for missing languages.
- Extended the CodeEdit widget test to register a custom language, switch to it at runtime, and verify toggle-line-comment uses its line comment metadata.
- Marked line comment commands and focus/capture/event wiring complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit ScrollModel Integration Slice

- Added CodeEdit scroll APIs:
  - `xuiCodeEditGetScrollModel`
  - `xuiCodeEditEnsureCaretVisible`
- CodeEdit now owns an `xui_scroll_model_t`.
- Scroll state is clamped through XUI ScrollModel using content size derived
  from visible line count and maximum line width.
- Rendering refreshes the scroll model before painting.
- Command execution calls ensure-caret-visible after handled editor commands.
- Extended the CodeEdit widget test to verify scroll-model offset exposure and
  caret ensure-visible behavior while preserving scroll-aware hit testing.
- Kept the scroll integration item `[~]` because visible internal ScrollBar
  composition is still pending.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Context Menu Command State Slice

- Added `xui_code_command_enabled_proc`.
- Added provider APIs:
  - `xuiCodeProviderSetCommandEnabled`
  - `xuiCodeProviderCanExecuteCommand`
- CodeEdit context menu now queries provider command enabled state for
  host-owned commands such as Find, Replace, and Go To Line.
- Menu selection still routes through the existing CodeEdit command path, so
  provider-handled host commands share keyboard and menu behavior.
- Extended provider tests for command-enabled queries and clear behavior.
- Extended CodeEdit widget tests for Find/Replace/Goto menu enable state and
  Find menu routing through the provider command callback.
- Marked context menu complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_provider_test.bat
test_xui\build_code_edit_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 CodeEdit IME Composition Slice

- Added CodeEdit-owned IME preedit state.
- IME composition events with active composition text now update and render
  preedit text at the caret without modifying the document.
- Empty IME composition events cancel preedit state without modifying the
  document.
- Final IME text commits through one CodeDocument transaction.
- Added `codeedit.ime.color` for composition text/underline rendering.
- Extended the CodeEdit widget test to verify preedit, cancel, commit, and
  composition rendering behavior.
- Marked IME composition complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed without warnings.

## 2026-05-31 CodeEdit Edit Transaction Notification Slice

- Added `xui_code_document_change_proc` and
  `xuiCodeDocumentSetChangeCallback`.
- CodeDocument now sends change notifications with affected range and version.
- Single edits, `SetText`, undo, and redo notify immediately.
- Grouped `BeginEdit` / `EndEdit` transactions coalesce nested edits into one
  callback at the outer `EndEdit`.
- Extended the document test to verify insert notification, grouped transaction
  range notification, and undo notification.
- Marked edit transactions complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_edit_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 CodeEdit Base Rendering Slice

- Added style properties for `codeedit.current_line.color` and
  `codeedit.readonly.background_color`.
- CodeEdit cache rendering now uses the readonly background color when the
  widget is readonly.
- Visible active line rows now draw a current-line highlight before selection
  and text layers.
- Extended the test proxy with color-specific rect-fill counting.
- Extended the CodeEdit widget test to verify style-driven current-line and
  readonly-background rendering.
- Marked base rendering layers complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Indentation Policy Slice

- Added `XUI_CODE_EDIT_INDENT_WITH_TABS` for tab-character indentation policy.
- CodeEdit command execution now builds either spaces or a tab character from the widget configuration.
- Extended the widget test to cover command outdent and tab-character indent policy.
- Re-ran the editing helper test to keep document-level indent/outdent coverage intact.
- Marked indentation commands complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
test_xui\build_code_editing_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 CodeEdit Selection Gesture Slice

- Added CodeEdit handling for `XUI_EVENT_POINTER_DOUBLE_CLICK`.
- Double-click now selects the word under the pointer.
- Ctrl+double-click selects the full logical line under the pointer.
- Extended the CodeEdit widget test to cover both word and line pointer selection gestures.
- Marked selection behavior complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit widget test rebuilt and passed.

## 2026-05-31 CodeEdit Caret Page Navigation Slice

- Added public CodeEdit commands for PageUp/PageDown and Shift+PageUp/Shift+PageDown selection.
- Bound the new commands in the default command map.
- CodeEdit now supplies command contexts with a page line count derived from the widget viewport and line height.
- Extended command and widget tests to cover default bindings, command-level page movement, and viewport-based PageDown behavior.
- Extended editing tests for backward and forward word deletion.
- Marked token rendering, caret navigation, and text insertion/deletion complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_command_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
```

Both tests rebuilt and passed.

## 2026-05-31 CodeEdit Internal ScrollBar Slice

- Added internal horizontal and vertical XUI ScrollBar children to CodeEdit.
- CodeEdit now computes auto scrollbar visibility, viewport size, scrollbar
  rectangles, range/page/steps, and value synchronization from its ScrollModel.
- Rendering and hit testing now use the active viewport instead of the full
  widget content rect, so scrollbar reserve space is excluded from editor text
  interaction.
- Added public scrollbar getters:
  - `xuiCodeEditGetHScrollBarWidget`
  - `xuiCodeEditGetVScrollBarWidget`
- Extended CodeEdit tests to verify internal scrollbar parentage, visibility,
  rectangles, and synchronized values.
- Marked CodeEdit widget implementation and internal scroll integration
  complete in `docs/xui-codeedit-spec.md`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_edit_test.bat
```

The CodeEdit test rebuilt and passed.

## 2026-05-31 XUI Path Stroke Mesh Slice

- Added `xuiPathBuildStrokeMesh` for width-based stroke tessellation over
  flattened path segments.
- The first stroke implementation emits independent quad segments; join, cap,
  dash, fill-rule, and anti-alias quality remain tracked in the vector spec.
- Updated `xuiPainterDrawPath` so stroke styles now render through the same
  mesh route instead of returning unsupported.
- Extended `test_xui/xui_chart_test.c` to verify direct stroke mesh generation
  and styled stroke rendering through `xuiPainterDrawPath`.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 Chart Area Fill Adoption

- Added line-series area fill APIs:
  - `xuiChartSetSeriesAreaFill`
  - `xuiChartGetSeriesAreaFill`
- Line area fill uses the XUI path builder and `xuiPathBuildFillMesh`, then
  submits the filled area through the proxy mesh draw route.
- Backends without mesh support keep the existing straight-line chart rendering
  and skip area fill instead of failing the chart.
- Extended `test_xui/xui_chart_test.c` to enable line area fill and verify that
  chart rendering submits a mesh draw.

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.

## 2026-05-31 XUI Painter Path API Slice

- Added `xui_path_style_t` with fill, stroke, fill-rule, join, cap, and dash
  fields.
- Added painter path helpers:
  - `xuiPainterFillPath`
  - `xuiPainterDrawPath`
- The current path rendering route is painter-level path-to-mesh over
  `drawMeshTriangles`; no backend path callback is required for this slice.
- `xuiPainterDrawPath` supports fill now and returns
  `XUI_ERROR_UNSUPPORTED` for stroke styles until stroke tessellation lands.
- Extended the chart test to verify:
  - direct fill path rendering
  - styled fill path rendering
  - unsupported fallback when a proxy has no mesh draw function

Verification on 2026-05-31:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_chart_test.bat
```

The chart test rebuilt and passed.
