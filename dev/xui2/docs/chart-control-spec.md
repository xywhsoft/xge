# XUI Chart Control SPEC

This SPEC tracks the XUI2-native chart control. Status markers:

- `[ ]` not started
- `[~]` in progress
- `[X]` completed

## Scope

V1 delivers one `xuiChart` widget with four chart families:

- line chart
- bar chart
- pie chart
- scatter chart

The control borrows product concepts from charting libraries such as ECharts, but it does not implement ECharts API or option compatibility.

## Maintenance Rules

- Keep this SPEC updated whenever chart implementation state changes.
- Do not mark a task `[X]` until code, test, example, and documentation evidence exists for that task.
- Use `xui` public names only; `xui2` is a folder/workstream name.
- Keep XSON deferred.
- Do not add proxy APIs silently.
- Preserve cache-first rendering and dirty overlay behavior.
- Examples must support `--frames N` and `--seconds N`.

## Phase 0: Design And Planning

- [X] Create chart control design document: `docs\chart-control-design.md`.
- [X] Create tracked chart SPEC: `docs\chart-control-spec.md`.
- [X] Add chart documents to `docs\README.md`.
- [X] Create widget user document: `docs\xui\widget-chart.md`.

## Phase 1: Core Widget Skeleton

- [X] Add public constants and structs to `xui.h`.
- [X] Add `xuiChartCreate` and basic chart lifecycle.
- [X] Add series add/remove/query APIs.
- [X] Add title, legend visibility, tooltip visibility, and chart padding APIs.
- [X] Add chart dirty flags for static, plot, and overlay caches.
- [X] Add minimal unit test for create/destroy and empty render.

Completion criteria:

- `test_xui\build_chart_test.bat` builds.
- Empty chart can be created, sized, rendered, and destroyed.
- No new proxy capability is required.

## Phase 2: Data And Axis Infrastructure

- [X] Add common `xui_chart_point_t` data path.
- [X] Add copied series data storage.
- [X] Add value axis range calculation.
- [X] Add category axis label handling.
- [X] Add cartesian plot rect calculation.
- [X] Add data-to-pixel and pixel-to-data helpers.
- [X] Add axis/grid rendering.
- [X] Add tests for axis range and coordinate mapping.

Completion criteria:

- Line/bar/scatter series can share the same cartesian mapping.
- Axis mapping tests cover normal, single-value, negative-value, and empty-data cases.

## Phase 3: Line Chart

- [X] Render straight line segments.
- [X] Render optional point markers.
- [X] Support multiple line series.
- [X] Implement line point hit testing.
- [X] Implement line hover and selection overlay.
- [X] Add line chart tests.

Completion criteria:

- A line chart renders in `examples\xui_chart`.
- Hover tooltip identifies the expected point.
- Selection state survives repaint.

## Phase 4: Bar Chart

- [X] Render single-series vertical bars.
- [X] Render grouped vertical bars for multiple visible bar series.
- [X] Implement bar rectangle hit testing.
- [X] Implement bar hover and selection overlay.
- [X] Add bar chart tests.

Completion criteria:

- A bar chart renders in `examples\xui_chart`.
- Grouped bars align with category ticks.
- Hidden series no longer consumes visible bar width.

## Phase 5: Pie Chart

- [X] Add polar plot layout.
- [X] Convert values to slice angles.
- [X] Render pie slices.
- [X] Support optional donut inner radius if it does not expand the proxy contract.
- [X] Implement slice hit testing.
- [X] Implement pie hover and selection overlay.
- [X] Add pie chart tests.

Completion criteria:

- A pie chart renders in `examples\xui_chart`.
- Slice angles sum to a full circle for positive values.
- Zero and negative values do not crash rendering.

## Phase 6: Scatter Chart

- [X] Render fixed-size scatter symbols.
- [X] Support circle symbol.
- [X] Support rect, triangle, and diamond symbols if existing proxy primitives are enough.
- [X] Support multiple scatter series.
- [X] Implement nearest-point hit testing.
- [X] Implement scatter hover and selection overlay.
- [X] Add scatter chart tests.

Completion criteria:

- A scatter chart renders in `examples\xui_chart`.
- Hit testing chooses the nearest visible point inside tolerance.
- Hidden series are excluded from hit testing.

## Phase 7: Legend And Tooltip

- [X] Render legend entries for visible/hidden series.
- [X] Toggle series visibility through legend click.
- [X] Render tooltip overlay using default text formatting.
- [X] Add custom tooltip callback.
- [X] Add tests for legend hit testing and tooltip formatting.

Completion criteria:

- Legend toggling updates plot cache and overlay cache correctly.
- Tooltip works for line, bar, pie, and scatter items.

## Phase 8: Example And Documentation

- [X] Add `examples\xui_chart\main.c`.
- [X] Add `examples\xui_chart\build.bat`.
- [X] Add `examples\xui_chart\run.bat` for interactive no-duration launch.
- [X] Example shows line, bar, pie, and scatter charts in one screen.
- [X] Example supports `--frames N`.
- [X] Example supports `--seconds N`.
- [X] Add `docs\xui\widget-chart.md`.
- [X] Update `docs\work.md` with implementation summary and verification commands.

Completion criteria:

- `examples\xui_chart\build.bat` succeeds.
- `examples\xui_chart\run.bat` launches the chart without duration arguments.
- `build\xui_chart.exe --frames 5` exits cleanly.
- `docs\xui\widget-chart.md` documents API, data model, chart types, and limitations.

## Phase 9: Verification Gate

- [X] Run `test_xui\build_chart_test.bat`.
- [X] Run `examples\xui_chart\build.bat`.
- [X] Run `build\xui_chart.exe --frames 5`.
- [X] Run `git diff --check`.
- [X] Record final command output summary in `docs\work.md`.

Completion criteria:

- All V1 commands pass.
- No whitespace errors are reported.
- The SPEC accurately reflects completed and deferred work.

## Deferred After V1

- [X] Smooth line curves.
- [X] Area line fill.
- [X] Stacked bars.
- [X] Horizontal bars.
- [X] Rose charts.
- [X] Multi-ring pie charts.
- [X] Scatter value-to-radius and value-to-color mapping.
- [X] Wheel zoom and drag pan.
- [X] Brush/range selection.
- [X] Large data LOD.
- [X] Animation transitions.
- [~] Optional XSON/chart description once the standard control migration allows it.

Dependency note:

- Smooth curves, dashed curves, high-quality area fills, and richer vector chart shapes should be implemented after `docs\vector-render-capability-spec.md` provides a path/mesh-capable XGE/XUI proxy route. Chart should not grow a private curve rasterizer.
- XSON/chart description is dependency-blocked by the standard control
  migration and the shared XSON loader pass. Do not implement a chart-only
  parser or schema loader; chart should consume the shared XRT/XSON route when
  that system is available.
