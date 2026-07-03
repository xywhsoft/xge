# XUI Chart Control Design

This document defines the first XUI2-native chart control. ECharts is only a feature reference: XUI2 will not implement ECharts option compatibility in this slice. The public API must stay C-style, explicit, and consistent with the existing `xui` widget APIs.

## Goals

- Add one reusable `xuiChart` typed widget.
- Support four V1 chart families: line, bar, pie, and scatter.
- Keep chart data owned by the application where practical.
- Use cache-first rendering so static charts do not redraw all geometry every frame.
- Share common chart infrastructure across all series types.
- Keep the first version small enough to validate with focused tests and examples.

## Non-Goals

- No ECharts option parser or API compatibility.
- No animation system in V1.
- No dataZoom UI, brush selection, or timeline playback in V1.
- No SVG path symbols or image symbols in V1.
- No native platform dialogs or native windows.
- No XSON format until the broader XUI2 control migration allows it.

## Control Model

```text
Chart widget
  title
  legend
  tooltip overlay
  chart layout
    cartesian plot area
      x axis
      y axis
      grid
      line series
      bar series
      scatter series
    polar plot area
      pie series
  interaction state
    hover item
    selected item
    visible series mask
```

The chart widget owns chart presentation state, layout caches, and interaction state. Application code owns business data unless the caller chooses to pass copied arrays into the chart.

## File Layout

Initial implementation should use these files under `dev\xui2`:

```text
src\xui_chart.c
src\xui_chart_axis.c
src\xui_chart_series.c
src\xui_chart_line.c
src\xui_chart_bar.c
src\xui_chart_pie.c
src\xui_chart_scatter.c
src\xui_chart_legend.c
src\xui_chart_tooltip.c

test_xui\xui_chart_test.c
test_xui\build_chart_test.bat

examples\xui_chart\main.c
examples\xui_chart\build.bat

docs\xui\widget-chart.md
```

If the first implementation is easier to land as one file, it may start in `src\xui_chart.c`, but the internal sections should keep the same boundaries so later splitting is mechanical.

## Public API Shape

Public declarations belong in `xui.h`. Public names use the `xui` prefix, not `xui2`.

```c
#define XUI_CHART_SERIES_LINE     1
#define XUI_CHART_SERIES_BAR      2
#define XUI_CHART_SERIES_PIE      3
#define XUI_CHART_SERIES_SCATTER  4

#define XUI_CHART_AXIS_VALUE      1
#define XUI_CHART_AXIS_CATEGORY   2

#define XUI_CHART_SYMBOL_CIRCLE   1
#define XUI_CHART_SYMBOL_RECT     2
#define XUI_CHART_SYMBOL_TRIANGLE 3
#define XUI_CHART_SYMBOL_DIAMOND  4

typedef struct xui_chart_point_t {
    double x;
    double y;
    double value;
    const char* label;
    unsigned int color;
} xui_chart_point_t;
```

Expected API direction:

```c
xui_widget chart;
int sales;

xuiChartCreate(ctx, &chart);
xuiChartSetTitle(chart, "Sales");
xuiChartSetXAxis(chart, XUI_CHART_AXIS_CATEGORY);
xuiChartSetYAxis(chart, XUI_CHART_AXIS_VALUE);
xuiChartSetLegendVisible(chart, 1);
xuiChartSetTooltipVisible(chart, 1);

sales = xuiChartAddSeries(chart, XUI_CHART_SERIES_LINE, "Sales");
xuiChartSeriesSetData(chart, sales, points, pointCount);
```

The exact signatures may adjust during implementation after checking current widget creation conventions, but the API should remain explicit and small.

## Data Model

V1 uses a common point model:

- `x` is the horizontal value or category index.
- `y` is the vertical value.
- `value` is the pie slice value or optional scalar for scatter size/color.
- `label` is the category, slice, or tooltip label.
- `color` is an optional per-item override.

Series interpretation:

| Series | Uses |
| --- | --- |
| Line | `x`, `y`, optional `label` |
| Bar | `x`, `y`, optional `label` |
| Pie | `value`, `label`, optional `color` |
| Scatter | `x`, `y`, optional `value`, optional `label` |

For V1, copied data arrays are acceptable. A later adapter API can let large charts read from an external model without copying.

## Layout

The chart computes these rectangles from the widget content rect:

```text
content rect
  title rect
  legend rect
  plot rect
  tooltip overlay rect
```

Cartesian charts reserve space for axis labels and grid lines. Pie charts use the plot rect as a centered polar area.

V1 should use deterministic layout:

- title at top
- legend at top-right or bottom
- x axis at bottom
- y axis at left
- plot rect fills the remaining area

## Rendering

Rendering is cache-first:

- static cache: background, title, legend frame
- plot cache: axes, grid, visible series geometry
- overlay cache: hover marker, selected marker, tooltip

Dirty triggers:

- size/style/title/legend changes invalidate static and plot caches
- data/series/axis changes invalidate plot and overlay caches
- hover/selection changes invalidate overlay cache only

The control must not add new proxy APIs silently. If the current proxy cannot draw a required primitive, implementation should either use existing shape/surface/text functions or stop and document the missing proxy capability.

## Series Behavior

### Line

V1 line charts support:

- single or multiple visible line series
- straight line segments
- optional point markers
- hover hit testing on points
- selected point marker

Deferred:

- smooth curves
- area fill
- stepped lines
- missing-value gaps
- animation

### Bar

V1 bar charts support:

- category/value axes
- single series bars
- grouped bars for multiple visible bar series
- hover and selection by bar rectangle

Deferred:

- stacked bars
- horizontal bars
- rounded bar caps
- waterfall bars

### Pie

V1 pie charts support:

- one pie series per chart
- value-to-angle conversion
- slice colors
- slice hover and selection
- optional inner radius for donut mode if cheap to implement

Deferred:

- rose charts
- multiple nested rings
- outside label collision avoidance
- leader lines

### Scatter

V1 scatter charts support:

- value/value axes
- multiple series
- fixed symbol size
- circle and basic geometric symbols
- hover and selection by nearest point

Deferred:

- visual mapping from value to color/size
- large data LOD
- symbol image/path support

## Interaction

V1 interactions:

- hover data item and show tooltip
- click data item to select it
- click legend item to toggle series visibility
- leave chart clears hover

Deferred interactions:

- wheel zoom
- drag pan
- brush/range selection
- keyboard navigation

Callbacks:

```c
typedef void (*xui_chart_select_proc)(
    xui_widget chart,
    int seriesIndex,
    int itemIndex,
    void* user);

typedef int (*xui_chart_tooltip_proc)(
    xui_widget chart,
    int seriesIndex,
    int itemIndex,
    char* buffer,
    int bufferSize,
    void* user);
```

## Tests

Unit tests should cover:

- chart create/destroy
- series add/remove/visibility
- axis range calculation
- cartesian data-to-pixel mapping
- pie value-to-angle calculation
- legend hit testing
- line point hit testing
- bar rectangle hit testing
- pie slice hit testing
- scatter nearest-point hit testing
- overlay dirty behavior after hover changes

## Example

`examples\xui_chart` should show all four V1 chart types in one screen:

- top-left: line chart
- top-right: bar chart
- bottom-left: pie chart
- bottom-right: scatter chart

The example must follow current XUI2 conventions:

- build with `examples\xui_chart\build.bat`
- run interactively without auto-exit
- support `--frames N`
- support `--seconds N`

## V1 Completion Criteria

V1 is complete when:

- `xuiChart` is a typed widget exposed through `xui.h`
- line, bar, pie, and scatter all render in the example
- hover tooltip works for all four chart types
- legend visibility toggling works for cartesian series
- unit tests build and pass
- example builds and runs with `--frames 5`
- `git diff --check` passes

