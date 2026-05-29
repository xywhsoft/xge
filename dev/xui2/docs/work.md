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
- full text edit stack
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
- Tooltip infrastructure and example
- Input example/infrastructure

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

Progress was the most recent migrated control.

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
test_xui\build_progress_test.bat
examples\xui_progress\build.bat
build\xui_progress.exe --frames 3
test_xui\build_button_test.bat
test_xui\build_image_test.bat
test_xui\build_separator_test.bat
```

No-args Progress run was checked via hidden process start and stayed alive after 2 seconds.

Formatting:

```bat
git diff --check
```

Result:

- passed
- only line-ending warnings for existing tracked files:
  - `dev/xui2/README.md`
  - `dev/xui2/docs/README.md`
  - `dev/xui2/src/xui_proxy_xge.c`
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

## Next Recommended Work

Continue standard control migration.

Likely next XUI1 controls to migrate, depending on priority:

- Slider
- Checkbox
- Radio
- Toggle
- Panel
- ScrollBar / ScrollView
- Popup / Menu-related controls
- Input/TextEdit later, after text editing infrastructure is ready

For each control, follow the migration standard above:

- behavior from XUI1
- XUI2 cache/render/style/layout integration
- test
- example
- doc
- no XSON yet

Controls involving text editing should wait until text editing foundation is intentionally designed and implemented.

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
examples\xui_image\build.bat
examples\xui_separator\build.bat
examples\xui_progress\build.bat
build\xui_label.exe --frames 360
build\xui_button.exe --frames 360
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
