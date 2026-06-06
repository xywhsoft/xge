# XUI2 Vector Rendering Capability SPEC

This SPEC tracks the medium-term rendering capabilities required by high-quality XUI2 controls. It covers XGE engine work, XUI proxy contract work, and XUI control adoption.

Status markers:

- `[ ]` not started
- `[~]` in progress
- `[X]` completed

## Decision

XUI2 should not solve high-quality Bezier/path rendering by adding one-off curve code inside individual widgets. The durable direction is:

- XGE provides reusable path and/or triangle mesh rendering primitives.
- XUI proxy exposes those primitives as backend-neutral render services.
- XUI controls consume the proxy services through painter/widget rendering.
- XUI may keep CPU tessellation fallbacks for simple cases, tests, and backends that do not support accelerated path rendering.

This keeps chart, icon, rich vector UI, and future SVG-like rendering from duplicating geometry/rasterization code in each control.

## Required Product Outcomes

The combined XGE/XUI capability should support:

- anti-aliased Bezier curves
- stroked curves with width
- dashed curve strokes
- curve/path fill
- SVG-style path subsets
- high-quality area charts
- complex icons and shapes
- future vector-rich widgets without per-widget raster hacks

## Ownership Boundary

| Layer | Responsibility |
| --- | --- |
| XGE | Efficient rendering backend: path tessellation/rasterization, mesh draw, anti-alias strategy, batching where possible |
| XUI Proxy | Stable backend-neutral API for path/mesh capabilities, capability flags, software fallback hooks where needed |
| XUI Painter | Convenience API, command validation, style/color integration, cache invalidation integration |
| XUI Widgets | Use painter/proxy capabilities; do not implement independent rasterizers unless explicitly scoped as fallback |

## Phase 0: Design Contract

- [X] Record that high-quality vector rendering is a XGE/proxy capability, not a chart-only workaround.
- [X] Audit current XGE shape/mesh APIs and decide whether path should build on an existing mesh path or a new vector subsystem.
- [X] Define proxy capability flags for path and mesh rendering.
- [X] Define fallback rules for backends without path support.
- [X] Define coordinate, winding, fill-rule, stroke-join, stroke-cap, dash, and anti-alias expectations.

Completion criteria:

- The XGE/XUI contract can be implemented by OpenGL, software test proxy, and future backends without widget-specific exceptions.

Phase 0 audit and contract:

- Current XUI proxy exposes point, line, single filled/stroked triangle, rect, circle, round rect, text, and surface quad draw calls. It does not expose a generic triangle mesh or path draw call.
- Current XGE exposes immediate shape primitives and `xgeShapeBatch...` for same-color triangle batches, plus `xgeDrawQuad3D` for textured quads. XGE internals already have dynamic vertex/index upload paths and a shape renderer with vertex color support, so the first implementation target should extend/expose a colored triangle mesh path rather than create a chart-only renderer.
- Mesh should be the first stable cross-layer primitive: XGE renders indexed or non-indexed colored triangle lists; XUI proxy forwards mesh payloads; XUI painter can tessellate paths into mesh where a backend does not provide native path rendering.
- Path should initially be a painter-level builder plus tessellation pipeline over mesh. A later native XGE path API can be added only if profiling or backend quality requires it.
- Proposed proxy capability flags:
  - `XUI_PROXY_CAP_MESH_TRIANGLES`: backend accepts colored triangle mesh draws.
  - `XUI_PROXY_CAP_PATH_FILL`: backend or painter route can fill paths.
  - `XUI_PROXY_CAP_PATH_STROKE`: backend or painter route can stroke paths with width/cap/join.
  - `XUI_PROXY_CAP_PATH_DASH`: backend or painter route can apply dash patterns.
  - `XUI_PROXY_CAP_PATH_AA`: backend can provide anti-aliased vector edges.
- Fallback rules:
  - If path is requested and mesh is available, XUI painter tessellates path to mesh.
  - If mesh is unavailable, controls keep existing primitive rendering where possible and return `XGE_ERROR_NOT_SUPPORTED` for features that require path quality.
  - Chart keeps straight-line/primitive rendering as fallback for smooth line, area fill, dashed line, and high-quality sector edges.
  - Test/software proxies may record mesh/path commands without accelerated rasterization as long as payload validation remains strict.
- Coordinate and geometry contract:
  - Coordinates are XUI logical surface coordinates with top-left origin; proxy backends convert to their native render coordinates.
  - Mesh vertices are submitted in draw order with per-vertex premultiplied RGBA color.
  - Default winding is counter-clockwise for front-facing triangles; tessellators should emit deterministic winding.
  - Fill rules: non-zero is default; even-odd is supported by path style when implemented.
  - Stroke defaults: miter join, butt cap, miter limit 4.0, solid line, width in logical pixels.
  - Dash pattern units are logical pixels and follow the path distance after flattening.
  - Anti-aliasing is a capability, not a correctness requirement; fallback rendering may be aliased but must preserve geometry bounds and color semantics.

## Phase 1: Triangle Mesh Primitive

- [X] Add or expose a generic colored triangle mesh draw path in XGE.
- [X] Add XUI proxy mesh capability flag.
- [X] Add XUI proxy mesh draw function.
- [X] Add software/test proxy implementation that records mesh calls.
- [X] Add XGE proxy implementation.
- [X] Add painter-level wrapper.
- [X] Add focused mesh smoke test.

Completion criteria:

- XUI can render arbitrary colored triangle lists into a surface through the proxy.
- The test proxy can validate mesh call counts and vertex/index payload bounds.

Phase 1 partial implementation note:

- XUI now defines `xui_mesh_vertex_t`, `XUI_PROXY_CAP_MESH_TRIANGLES`,
  `xui_draw_mesh_triangles_proc`, `xui_proxy_t.drawMeshTriangles`, and
  `xuiPainterDrawMeshTriangles`.
- The shared XUI test proxy advertises mesh support and records mesh draw count,
  last vertex count, and last index count.
- XGE now exposes `xge_shape_vertex_t`, `xgeShapeMeshFill`, and
  `xgeShapeMeshFillPx`, implemented over the existing shape auto-batch
  vertex/index upload path.
- The XUI XGE proxy advertises `XUI_PROXY_CAP_MESH_TRIANGLES` and forwards
  `drawMeshTriangles` to `xgeShapeMeshFillPx`.
- `test_xui/xui_chart_test.c` includes a focused painter mesh smoke check using
  a four-vertex, six-index colored quad mesh.

## Phase 2: Path Builder And Tessellation

- [X] Add path builder data model: move, line, quadratic, cubic, close.
- [X] Add path flattening/tessellation for fill.
- [X] Add path stroke tessellation with width.
- [X] Add stroke joins: miter, bevel, round.
- [X] Add stroke caps: butt, square, round.
- [X] Add dash pattern support.
- [X] Add fill rules: non-zero and even-odd.
- [X] Add anti-alias strategy.

Completion criteria:

- XUI can render a filled and stroked cubic Bezier path through the same proxy path on at least test proxy and XGE proxy.

Phase 2 partial implementation note:

- XUI now defines `xui_path`, `xui_path_command_t`, and command ids for move,
  line, quadratic, cubic, and close.
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
- Added `xuiPathFlatten` for fixed-step quadratic/cubic flattening.
- Added `xuiPathBuildFillMesh` for convex/simple filled paths by converting
  flattened points to a triangle fan mesh. This is sufficient for the first
  area-chart and simple shape path, while complex fill rules remain tracked
  below.
- Added `xuiPathBuildStrokeMesh` for width-based stroke tessellation over
  flattened path segments. The first implementation emits independent quad
  segments, so join and cap quality remain tracked separately.
- Added `xuiPathBuildDashedStrokeMesh` and dash-aware `xuiPainterDrawPath`
  stroke rendering. Dash pattern values are logical pixels along the flattened
  path.
- `xuiPainterDrawPath` now honors `xui_path_style_t.iLineCap` for butt,
  square, and round stroke caps. Round caps are emitted as deterministic
  triangle-fan semicircles on top of the shared stroke mesh route.
- `xuiPainterDrawPath` now honors `xui_path_style_t.iLineJoin` for solid
  path strokes. Miter, bevel, and round joins are emitted as extra mesh
  geometry at flattened path corners; dashed strokes keep independent dash
  segments to avoid incorrectly joining visible segments across gaps.
- `test_xui/xui_chart_test.c` now builds a mixed line/quadratic/cubic closed
  path, flattens it, generates fill and stroke meshes, and submits the mesh
  through the painter/test-proxy route.
- `xuiPainterDrawPath` now honors `xui_path_style_t.iFillRule` for fill style
  validation. V1 fill tessellation supports both `XUI_PATH_FILL_NON_ZERO` and
  `XUI_PATH_FILL_EVEN_ODD` on simple filled paths through the shared triangle
  fan route. Compound paths with holes still require the later full
  tessellator; they are not represented as a separate widget-local solution.

Anti-alias strategy:

- V1 correctness path is deterministic tessellated mesh without guaranteed
  anti-aliasing. This is acceptable for test/software proxies and any backend
  that does not report `XUI_PROXY_CAP_PATH_AA`.
- Backends that can provide high-quality vector edges should advertise
  `XUI_PROXY_CAP_PATH_AA`. XUI controls must treat that as a quality
  capability, not as a correctness precondition.
- The preferred medium-term implementation is geometry AA fringe generation in
  the shared path tessellator for fills and strokes. Fringe vertices should use
  premultiplied alpha falloff and preserve the original path bounds plus a
  documented half-pixel expansion.
- A later native XGE path renderer may replace painter-level fringe
  tessellation when profiling or backend quality justifies it, but widgets must
  keep calling the same painter/proxy path APIs.
- Validation rule: smoke tests assert mesh/path submission and payload bounds;
  visual AA quality is checked in backend-specific visual regression once an
  AA-capable backend path exists.

## Phase 3: Proxy/Painter API

- [X] Add path capability flag.
- [X] Add path draw API to proxy or define path-to-mesh as the required painter implementation.
- [X] Add `xuiPainterPath...` helpers.
- [X] Add shape style structs for fill, stroke, dash, cap, join, and fill rule.
- [X] Add validation tests for unsupported capability fallback.

Completion criteria:

- Controls can ask painter to render path/mesh without knowing backend details.
- Backends can report unsupported path features cleanly.

Phase 3 implementation note:

- The current path rendering route is painter-level path-to-mesh over
  `drawMeshTriangles`. No backend-specific path callback is required yet.
- Added `xui_path_style_t` with fill color, stroke color/width, fill rule,
  join, cap, and dash fields.
- Added `xuiPainterFillPath` and `xuiPainterDrawPath`.
- `xuiPainterDrawPath` supports fill and basic width-based stroke styles over
  the mesh route.
- `test_xui/xui_chart_test.c` verifies fill path rendering and the unsupported
  fallback when the active proxy does not provide `drawMeshTriangles`.

## Phase 4: Chart Adoption

- [X] Use path/mesh for smooth line curves.
- [X] Use path/mesh for high-quality area chart fill.
- [X] Use path/mesh for dashed chart lines.
- [X] Use path/mesh for high-quality pie/sector edges if needed.
- [X] Keep current straight-line chart rendering as fallback.

Completion criteria:

- Chart can render a smooth line and filled area without adding chart-local curve rasterization.

Phase 4 partial implementation note:

- Line series now supports optional area fill through:
  - `xuiChartSetSeriesAreaFill`
  - `xuiChartGetSeriesAreaFill`
- Area fill uses the XUI path builder and fill-mesh route, then submits the
  mesh through `drawMeshTriangles`.
- Smooth line rendering is opt-in per line series. It builds a cubic path from
  the sampled line points and renders the path through stroke mesh.
- Dashed line rendering is opt-in per line series and uses the same path/mesh
  stroke route with dash pattern values in logical pixels.
- Pie and rose sectors now use one triangle mesh per slice when the active
  proxy provides `drawMeshTriangles`. Backends without mesh support keep the
  previous per-segment triangle fallback.
- If the active proxy has no mesh support, chart keeps the existing primitive
  rendering where possible and skips area fill instead of failing the whole chart.

## Phase 5: UI Asset Adoption

- [X] Add vector icon path support.
- [X] Add SVG path subset import or compile-time conversion tool.
- [X] Use vector icons in selected controls where bitmap assets are currently too limiting.
- [X] Add visual regression/smoke example for vector-heavy UI.

Completion criteria:

- XUI2 can display complex icons and vector shapes cleanly across DPI scales.

Phase 5 partial implementation note:

- Added `test_xui/xui_vector_smoke_test.c` and
  `test_xui/build_vector_smoke_test.bat` as a focused vector-heavy smoke
  entrypoint.
- Added `xuiPathParseSvg` as the first SVG path subset import API. It supports
  absolute and relative `M`, `L`, `H`, `V`, `Q`, `C`, and `Z` commands and
  writes into the existing `xui_path` builder.
- Added the first vector icon path API:
  - `xuiVectorIconGetCount`
  - `xuiVectorIconGetName`
  - `xuiPainterDrawVectorIcon`
- The initial built-in vector icon catalog includes `check`, `close`,
  `chevron_down`, `search`, `user`, `lock`, and `eye`, rendered through
  SVG-subset path import and the shared painter path/mesh stroke route.
- Input decoration icons now prefer vector icon drawing for search, user,
  lock, eye, and clear decorations when the active proxy supports mesh
  drawing. The previous atlas-backed bitmap assets remain as fallback.
- The smoke test renders filled cubic/quadratic paths, round-cap/round-join
  strokes, dashed square-cap strokes, SVG-subset imported paths, and vector
  icons through the painter path route, then asserts mesh proxy submissions
  and payload sizes.
- This test is the current regression carrier for path/mesh expressiveness
  and will be extended as vector icons and additional SVG commands land.

## Phase 6: XGE SVG Path Renderer

- [X] Add XGE-owned path data model and lifecycle API.
- [X] Add XGE SVG path subset parser.
- [X] Add XGE path flattening.
- [X] Add XGE fill mesh generation over `xge_shape_vertex_t`.
- [X] Add XGE stroke mesh generation with width, joins, caps, and dashes.
- [X] Add XGE path draw helpers over `xgeShapeMeshFill` and `xgeShapeMeshFillPx`.
- [X] Add XGE smoke coverage for SVG path parse, fill mesh, stroke mesh, dashed
  stroke mesh, and curve flattening.
- [X] Add SVG file/resource loader for `<svg>` and `<path>` assets.
- [X] Add SVG viewBox, transform stack, inherited style, and viewport mapping.
- [X] Add remaining SVG path commands: smooth cubic/quadratic and elliptical arc.
- [X] Add common SVG geometry elements: `rect`, `circle`, `ellipse`, `line`, `polyline`, and `polygon`.
- [X] Add XGE SVG example covering memory SVG strings and file SVG assets.
- [X] Add `preserveAspectRatio` viewport mapping for `none`, `meet`, `slice`, and x/y alignment modes.
- [X] Add path/icon resource caching and invalidation.
- [X] Add robust concave/compound path tessellation with holes and true fill rules.
- [ ] Add geometry anti-alias fringe or backend-native path AA.
- [~] Add gradients, clip paths, masks, and opacity groups.

Completion criteria:

- XGE can load and draw SVG file/icon assets with GPU-backed path rendering and
  quality comparable to the XUI vector needs, while keeping XUI on the proxy and
  painter contracts instead of duplicating renderer code.

Phase 6 partial implementation note:

- XGE now exposes `xge_path`, `xge_path_command_t`, `xge_path_style_t`, SVG path
  command constants, fill-rule constants, join constants, and cap constants.
- Added `src/xge_svg.c` and included it from `src/xge_impl.c`.
- `xgePathParseSvg` supports absolute and relative `M`, `L`, `H`, `V`, `Q`, `T`,
  `C`, `S`, `A`, and `Z` path commands.
- Smooth quadratic/cubic commands are lowered into existing `XGE_PATH_CMD_QUAD`
  and `XGE_PATH_CMD_CUBIC` commands by reflecting the previous control point.
- Elliptical arc commands are lowered into one or more cubic Bezier segments
  using the SVG endpoint-arc conversion algorithm. Zero-radius arcs degrade to
  line segments.
- `xgePathBuildFillMesh`, `xgePathBuildStrokeMesh`, and
  `xgePathBuildDashedStrokeMesh` generate deterministic triangle meshes using
  XRT allocation and XGE shape vertex/index payloads.
- `xgePathDraw` and `xgePathDrawPx` submit generated meshes through the existing
  XGE shape mesh renderer, so the current acceleration path is GPU triangle
  upload rather than a separate GPU-native path rasterizer.
- `test/test_main.c` now carries the XGE-level smoke for this path/SVG subset.
- Added `xge_svg` as an XGE-owned SVG asset handle with:
  - `xgeSvgCreate`
  - `xgeSvgDestroy`
  - `xgeSvgClear`
  - `xgeSvgLoad`
  - `xgeSvgLoadMemory`
  - `xgeSvgGetViewBox`
  - `xgeSvgGetPathCount`
  - `xgeSvgGetPathInfo`
  - `xgeSvgDraw`
  - `xgeSvgDrawPx`
- `xgeSvgLoad` uses the existing XGE resource loader, so file paths, `file://`
  URIs, and registered resource providers follow the same path as image loads.
- The SVG loader now parses `<svg>`, `<g>`, `<path>`, `rect`, `circle`,
  `ellipse`, `line`, `polyline`, and `polygon` tags, `viewBox`,
  width/height fallback, path `d`, inherited fill/stroke style, inline `style`,
  stroke width, line join, line cap, dash array, dash offset, fill rule, opacity,
  and simple colors.
- Transform stack support covers nested `svg`/`g`/`path` transforms for
  `matrix`, `translate`, `scale`, and `rotate`. Parsed path coordinates are
  transformed into XGE path geometry during load.
- Basic SVG geometry elements are converted into `xge_path` objects during load.
  Rounded `rect` uses path arcs, `circle`/`ellipse` lower to cubic arcs, and
  `line`/`polyline`/`polygon` lower to move/line/close path commands.
- `preserveAspectRatio` parsing now supports the SVG default `xMidYMid meet`,
  explicit `none`, all x/y align combinations, and `meet`/`slice` viewport
  mapping. `xgeSvgDrawPx` clips `slice` overflow through the existing XGE
  scissor clip state and restores any previous clip after SVG drawing.
- XGE fill tessellation now uses a scanline trapezoid mesh generator over
  flattened contours instead of the previous convex triangle fan. It handles
  concave polygons, compound paths, holes, and both non-zero and even-odd fill
  rules for the SVG/path subset currently lowered to flattened geometry.
- SVG loader now parses basic `<linearGradient>` definitions with `id`,
  `gradientUnits`, `x1`, `y1`, `x2`, `y2`, and child `<stop>` elements carrying
  `offset`, `stop-color`, and `stop-opacity`. `fill="url(#id)"` is stored on the
  SVG path item and `xgeSvgDraw` / `xgeSvgDrawPx` render the fill as a
  per-vertex color mesh over the existing GPU triangle path. Current coverage is
  linear-gradient fill; radial gradients, stroke gradients, gradientTransform,
  general `clipPath`, masks, and true opacity groups remain open under this
  `[~]` item.
- Added shared SVG asset cache APIs:
  - `xgeSvgLoadCached`
  - `xgeSvgAddRef`
  - `xgeSvgCacheInvalidate`
  - `xgeSvgCacheClear`
  Cache entries reuse `xgeResourceLoad` for data access and hold a shared
  `xge_svg` reference until invalidated or cleared.
- `xgeSvgDraw` and `xgeSvgDrawPx` map the parsed SVG viewBox into the supplied
  destination rectangle and draw each path through the existing XGE path draw
  route.
- `test/test_main.c` now validates SVG memory load, SVG file load, viewBox,
  inherited style, group transform, dash style, and mesh generation from a
  loaded SVG path.
- `test/test_main.c` now also validates smooth cubic reflection, smooth
  quadratic reflection, elliptical arc conversion, and SVG file loading with
  `T`/`A` path commands.
- `test/test_main.c` now validates concave path fill, even-odd compound holes,
  non-zero reversed-contour holes, and SVG-loaded `fill-rule="evenodd"` mesh
  generation.
- `test/test_main.c` now validates SVG-loaded linear-gradient fill references
  through `xge_svg_path_info_t.sFillGradientId`, including cache reload coverage.
- `examples/xge_svg` demonstrates direct XGE SVG rendering from an in-memory SVG
  string and from `examples/xge_svg/assets/shapes.svg`, including an even-odd
  compound path, linear-gradient fills, and `slice` viewport clipping.

## Deferred Details

- Full SVG file rendering beyond the current path/basic shape subset.
- General SVG `clipPath` element support beyond the `slice` scissor case.
- Text-on-path.
- Radial gradients, stroke gradients, gradientTransform, and spread methods.
- Mask elements and true opacity groups.
- GPU-native path rendering beyond tessellated mesh.
- Advanced boolean path operations.
