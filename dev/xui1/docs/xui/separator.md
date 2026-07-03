# XUI Separator

`xge_xui_separator_t` 是静态分隔线控件。它只负责在 widget content rect 中绘制一条线；背景、边框、padding、clip、enabled 和 owner draw 都由 Widget 提供。

## 设计思路

- Separator 是基础静态控件，不承担图形装饰、标题分隔、图片分割线或 Canvas 绘制职责。
- 控件本身的宽度和高度由布局控制，`thickness` 只表示线条厚度。
- 线条默认在 content rect 内居中；也可以贴近上/左边界或下/右边界。
- 虚线、点线和点划线用多段 `draw_rect` 绘制，不引入专用 shader。
- 不使用缓存。实线通常是一次矩形绘制，非实线是少量矩形绘制。

## 属性

| 属性 | 说明 |
| --- | --- |
| `orientation` | `horizontal` 或 `vertical`。 |
| `thickness` | 线条厚度。horizontal 时作用于高度，vertical 时作用于宽度。 |
| `color` | 线条颜色。alpha 为 0 时不绘制。 |
| `align` | `start`、`center`、`end`。horizontal 对应上/中/下，vertical 对应左/中/右。 |
| `lineStyle` | `solid`、`dot`、`dash`、`dashDot`。 |

## API

```c
int xgeXuiSeparatorInit(xge_xui_separator sep, xge_xui_widget widget);
void xgeXuiSeparatorUnit(xge_xui_separator sep);

void xgeXuiSeparatorSetOrientation(xge_xui_separator sep, int orientation);
void xgeXuiSeparatorSetThickness(xge_xui_separator sep, float thickness);
void xgeXuiSeparatorSetColor(xge_xui_separator sep, uint32_t color);
void xgeXuiSeparatorSetAlign(xge_xui_separator sep, int align);
void xgeXuiSeparatorSetLineStyle(xge_xui_separator sep, int lineStyle);

xge_vec2_t xgeXuiSeparatorMeasureProc(xge_xui_widget widget, void* user);
void xgeXuiSeparatorPaintProc(xge_xui_widget widget, void* user);
```

常量：

```c
#define XGE_XUI_SEPARATOR_HORIZONTAL 0
#define XGE_XUI_SEPARATOR_VERTICAL   1

#define XGE_XUI_SEPARATOR_SOLID      0
#define XGE_XUI_SEPARATOR_DOT        1
#define XGE_XUI_SEPARATOR_DASH       2
#define XGE_XUI_SEPARATOR_DASH_DOT   3
```

`align` 使用 `XGE_XUI_ALIGN_START`、`XGE_XUI_ALIGN_CENTER`、`XGE_XUI_ALIGN_END`。

## XSON

```json
{
  "type": "separator",
  "orientation": "horizontal",
  "thickness": 2,
  "color": "#5A687AB4",
  "align": "center",
  "lineStyle": "dash"
}
```

字段：

- `orientation`: `horizontal` 或 `vertical`。
- `thickness`: 线条厚度。
- `color`: 线条颜色，支持颜色 token。
- `align`: `start`、`center`、`end`，也接受 `left/top`、`middle`、`right/bottom`。
- `lineStyle`: `solid`、`dot`、`dotted`、`dash`、`dashed`、`dashDot`、`dash-dot`、`dash_dot`。

## 范例

- `examples/xui_separator`: C API 版本。
- `examples/xui_separator_xson`: XSON 描述版本。

范例左侧使用段落文字和四种横向分隔线混排，右侧用横向文本组混排四种纵向分隔线。
