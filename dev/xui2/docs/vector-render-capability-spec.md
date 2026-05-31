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

## Deferred Details

- Full SVG rendering.
- Text-on-path.
- Gradient meshes.
- GPU-native path rendering beyond tessellated mesh.
- Advanced boolean path operations.
