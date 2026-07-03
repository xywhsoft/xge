# XUI Chart

Chart is the first XUI2-native data visualization widget. It is not an ECharts compatibility layer. It borrows familiar chart concepts such as series, legend, tooltip, axes, and multiple chart families, while keeping the public API in normal XUI C style.

## V1 Coverage

Implemented chart families:

- line
- bar
- pie
- scatter

Implemented shared features:

- typed `chart` widget
- copied point-array data path
- multiple series
- value and category axis modes
- chart title
- legend rendering
- legend click visibility toggle
- default hover/selection tooltip
- line, bar, pie, and scatter hit testing
- donut-style pie inner radius
- multi-ring pie charts from multiple visible pie series
- rose pie mode with equal slice angles and value-mapped radius
- scatter value-to-radius and value-to-color visual mapping
- cartesian wheel zoom and drag pan
- cartesian brush/range selection
- line and scatter large-data LOD stride
- smooth line rendering through the path/mesh renderer
- dashed line rendering through the path/mesh renderer
- line-series area fill through the path/mesh renderer
- opt-in series transition fade progress
- grouped, stacked, vertical, and horizontal bar modes

Deferred:

- ECharts option compatibility
- XSON
- smooth line curves
- dashed chart lines
- split implementation files

## Data Model

```c
typedef struct xui_chart_point_t {
	double x;
	double y;
	double value;
	const char* label;
	uint32_t color;
} xui_chart_point_t;
```

Series interpretation:

| Series | Data fields |
| --- | --- |
| Line | `x`, `y`, optional `label` |
| Bar | `x`, `y`, optional `label` |
| Pie | `value`, `label`, optional per-slice `color` |
| Scatter | `x`, `y`, optional `value`, optional `label`, optional per-point `color` |

V1 copies the point array into the chart. The label strings are referenced, not copied, so their lifetime must cover chart use.

## Basic Usage

```c
xui_chart_desc_t desc;
xui_widget chart;
xui_chart_point_t points[] = {
	{0.0, 12.0, 12.0, "Jan", 0},
	{1.0, 19.0, 19.0, "Feb", 0},
	{2.0, 15.0, 15.0, "Mar", 0}
};
int series;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pFont = font;
desc.sTitle = "Sales";

xuiChartCreate(ctx, &chart, &desc);
xuiWidgetSetRect(chart, (xui_rect_t){20.0f, 20.0f, 420.0f, 280.0f});
xuiChartAddSeries(chart, XUI_CHART_SERIES_LINE, "Actual", &series);
xuiChartSetSeriesData(chart, series, points, 3);
```

## Example Run

Interactive run:

```bat
cd /d D:\git\xge\dev\xui2
examples\xui_chart\run.bat
```

`run.bat` starts `build\xui_chart.exe` without duration arguments, so the
window stays open until it is closed or Esc is pressed.

Double-clicking `build\xui_chart.exe` is also an interactive no-duration run.
The example writes `xui_chart_run.log` in the current working directory; when
launched from Explorer this is normally next to the executable in `build`.
If the window exits unexpectedly, inspect that log for the exit reason.

Smoke runs are intentionally short-lived:

```bat
cd /d D:\git\xge\dev\xui2
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
build\xui_chart.exe --seconds 2
```

Use `--frames` or `--seconds` only for automated verification; both options
quit automatically after the requested duration.

## Series Types

```c
XUI_CHART_SERIES_LINE
XUI_CHART_SERIES_BAR
XUI_CHART_SERIES_PIE
XUI_CHART_SERIES_SCATTER
```

Line and scatter use point symbols:

```c
XUI_CHART_SYMBOL_CIRCLE
XUI_CHART_SYMBOL_RECT
XUI_CHART_SYMBOL_TRIANGLE
XUI_CHART_SYMBOL_DIAMOND
```

Line series can render a filled area down to the zero baseline when the active
proxy supports triangle mesh rendering:

```c
xuiChartSetSeriesAreaFill(
	chart,
	series,
	1,
	XUI_COLOR_RGBA(42, 124, 221, 48));
```

`xuiChartGetSeriesAreaFill` returns `1` when enabled and can report the active
area color.

Line series can also render a smooth cubic path instead of straight segments:

```c
xuiChartSetSeriesSmooth(chart, series, 1);
```

When the active proxy has no mesh support, chart falls back to straight line
rendering.

Line series can use dash patterns in logical pixels:

```c
float dash[] = {10.0f, 5.0f};
xuiChartSetSeriesDash(chart, series, dash, 2);
```

`xuiChartClearSeriesDash` disables dashes, and `xuiChartGetSeriesDash` copies
the configured pattern into a caller-provided buffer.

Scatter can map `point.value` to symbol size and color:

```c
xuiChartSetSeriesValueRadius(chart, series, 6.0f, 18.0f);
xuiChartSetSeriesValueColor(
	chart,
	series,
	XUI_COLOR_RGBA(42, 124, 221, 255),
	XUI_COLOR_RGBA(211, 75, 98, 255));
```

The mapping range is computed from the series' current point values. If all values are equal, the midpoint of the configured visual range is used. A non-zero `point.color` still overrides the value-color mapping for that point.

The getter APIs return `1` when mapping is enabled, `0` when disabled, and a negative XUI error code for invalid arguments:

```c
float minSize;
float maxSize;
uint32_t minColor;
uint32_t maxColor;

xuiChartGetSeriesValueRadius(chart, series, &minSize, &maxSize);
xuiChartGetSeriesValueColor(chart, series, &minColor, &maxColor);
xuiChartClearSeriesValueRadius(chart, series);
xuiChartClearSeriesValueColor(chart, series);
```

## Axes

```c
xuiChartSetXAxis(chart, XUI_CHART_AXIS_CATEGORY);
xuiChartSetYAxis(chart, XUI_CHART_AXIS_VALUE);
```

V1 supports value and category modes. Category mode currently uses point order for placement. Full axis label rendering and advanced tick formatting are deferred.

## Bar Mode

Bar charts use grouped mode by default:

```c
xuiChartSetBarMode(chart, XUI_CHART_BAR_GROUPED);
```

Stacked mode combines visible bar series in each item slot:

```c
xuiChartSetBarMode(chart, XUI_CHART_BAR_STACKED);
```

Positive and negative values are stacked separately from the zero baseline. Hidden bar series are excluded from stack calculation and hit testing.

Bar direction is vertical by default:

```c
xuiChartSetBarDirection(chart, XUI_CHART_BAR_VERTICAL);
xuiChartSetBarDirection(chart, XUI_CHART_BAR_HORIZONTAL);
```

Horizontal bars still read the numeric value from `point.y`; the value is mapped to X and the item/category index is mapped to Y. Grouped and stacked modes both work with horizontal bars.

## Layout And Dirty State

Chart has optional extra plot padding:

```c
xui_thickness_t padding = {12.0f, 8.0f, 10.0f, 6.0f};

xuiChartSetPadding(chart, padding);
padding = xuiChartGetPadding(chart);
```

The chart also exposes coarse internal dirty flags for diagnostics:

```c
uint32_t dirty = xuiChartGetDirtyFlags(chart);
```

Dirty parts:

```c
XUI_CHART_DIRTY_STATIC
XUI_CHART_DIRTY_PLOT
XUI_CHART_DIRTY_OVERLAY
```

These flags describe chart-local invalidation intent. Rendering still uses the normal XUI widget cache invalidation path.

## Pie And Donut

Pie charts read `value` from each point. Optional per-slice colors can be supplied through `point.color`.

```c
xuiChartSetPieInnerRadius(chart, 0.35f);
```

An inner radius of `0.0f` renders a normal pie. Values from `0.0f` up to but not including `1.0f` are valid.

Multiple visible pie series are rendered as concentric rings inside the same plot area:

```c
xuiChartAddSeries(chart, XUI_CHART_SERIES_PIE, "Current", &seriesA);
xuiChartSetSeriesData(chart, seriesA, current, currentCount);
xuiChartAddSeries(chart, XUI_CHART_SERIES_PIE, "Previous", &seriesB);
xuiChartSetSeriesData(chart, seriesB, previous, previousCount);
```

Series order maps from inner ring to outer ring. The configured pie inner radius is the hole inside the innermost ring. Hit testing respects the ring bounds and returns the matching series and slice.

Pie mode controls how values are interpreted:

```c
xuiChartSetPieMode(chart, XUI_CHART_PIE_NORMAL);
xuiChartSetPieMode(chart, XUI_CHART_PIE_ROSE);
```

Normal mode maps `value` to slice angle. Rose mode gives each point an equal angle and maps `value` to radius inside the series ring. Rose mode also works with multiple pie series, producing multi-ring rose charts.

## Legend And Tooltip

Legend is visible by default:

```c
xuiChartSetLegendVisible(chart, 1);
```

Clicking a legend entry toggles that series. Tooltip is also visible by default:

```c
xuiChartSetTooltipVisible(chart, 1);
```

Tooltip text is generated by the chart by default. Custom tooltip formatting can be installed with:

```c
static int chart_tooltip(
	xui_widget chart,
	int series,
	int item,
	char* buffer,
	int capacity,
	void* user)
{
	(void)chart;
	(void)user;
	snprintf(buffer, (size_t)capacity, "series=%d item=%d", series, item);
	return XUI_OK;
}

xuiChartSetTooltipCallback(chart, chart_tooltip, user);
```

The callback receives chart-local series and item indexes. Return `XUI_OK` and write a non-empty buffer to override the default tooltip text.

## Coordinate Mapping

Cartesian charts expose mapping helpers:

```c
xui_vec2_t pixel;
double dataX;
double dataY;

xuiChartDataToPixel(chart, 1.0, 7.0, &pixel);
xuiChartPixelToData(chart, pixel.fX, pixel.fY, &dataX, &dataY);
```

The mapping uses the current visible cartesian series range and plot rect.

## View Range And Interaction

Cartesian charts use automatic data ranges by default. A manual view range can be set or reset:

```c
xuiChartSetViewRange(chart, minX, maxX, minY, maxY);
xuiChartGetViewRange(chart, &minX, &maxX, &minY, &maxY);
xuiChartResetViewRange(chart);
```

`xuiChartGetViewRange` returns `1` when a manual view range is active and `0` when the range is automatic. Mouse wheel over the plot zooms around the cursor and activates a manual view range. Pointer drag inside the plot pans the active cartesian view. Pie-only charts ignore wheel zoom and drag pan.

Cartesian charts also expose a brush range:

```c
xuiChartSetBrushRange(chart, minX, maxX, minY, maxY);
xuiChartGetBrushRange(chart, &minX, &maxX, &minY, &maxY);
xuiChartClearBrushRange(chart);
```

`xuiChartGetBrushRange` returns `1` when a brush range is active and `0` when it is clear. Shift+drag inside the plot creates or updates the brush range. Normal drag remains pan. Pie-only charts ignore brush drag.

## Large Data LOD

Line and scatter rendering can downsample very large series by stride:

```c
xuiChartSetLodThreshold(chart, 2000);
xuiChartGetLodThreshold(chart);
xuiChartGetLastLodStride(chart);
```

A threshold of `0` disables LOD. When enabled, line and scatter drawing keep the first and last points and draw every Nth intermediate point so the rendered point count stays near the threshold. Hit testing still uses the full copied data.

## Animation

Chart transitions are opt-in and advanced by the caller:

```c
xuiChartSetAnimation(chart, 1, 0.25f);
xuiChartStepAnimation(chart, deltaSeconds);
xuiChartGetAnimation(chart, &enabled, &duration, &progress);
```

When enabled, data and view-range changes start a transition. Current rendering applies transition progress as a series alpha fade. `xuiChartGetAnimation` returns `1` while a transition is active and `0` otherwise.

## Hit Testing

```c
xui_chart_hit_t hit;

memset(&hit, 0, sizeof(hit));
hit.iSize = sizeof(hit);
xuiChartHitTest(chart, x, y, &hit);
```

Hit parts:

```c
XUI_CHART_HIT_NONE
XUI_CHART_HIT_SERIES
XUI_CHART_HIT_LEGEND
```

Coordinates are chart-local coordinates.

## Example

Build and run:

```bat
examples\xui_chart\build.bat
build\xui_chart.exe --frames 5
```

The example shows line, bar, pie, and scatter charts in a four-panel layout.
