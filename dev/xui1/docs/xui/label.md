# XUI Label

`xge_xui_label_t` 是基础静态文本控件。它只负责文本内容、字体、文字颜色、对齐、下划线和可选文本缓存；背景、边框、圆角、padding、clip、enabled、owner draw 都由 Widget 基础设施负责。

## 设计思路

- Label 的职责边界必须简单：显示一段文本，不承担输入、选择、富文本、图标或复杂排版职责。
- 文本永远绘制在 widget content rect 内，并强制带 `XGE_TEXT_CLIP`，避免文本溢出破坏布局。
- 控件支持 enabled/disabled 两套文字颜色。禁用状态来自 Widget，Label 只根据 Widget 状态选择文字颜色。
- Widget 提供背景、边框、圆角、padding、布局、裁剪和 owner draw，所以 Label 不重复实现这些能力。
- Label 可以按需缓存文字渲染结果。缓存只覆盖文字内容，不把 Widget 的背景和边框一起缓存进去。
- 每次文本、字体、颜色、对齐、下划线、缓存模式或启用状态变化时，缓存会失效并重新绘制。

## Cache

缓存模式：

| 模式 | 说明 |
| --- | --- |
| `XGE_XUI_CACHE_AUTO` | 默认模式。允许控件使用渲染缓存。 |
| `XGE_XUI_CACHE_OFF` | 关闭缓存，每次 paint 直接绘制文字。 |
| `XGE_XUI_CACHE_FORCE` | 强制使用缓存；如果缓存纹理创建失败，会回退到直接绘制。 |

Label 的缓存不是为了减少三角形数量，而是为了避免频繁文本栅格化。缓存命中后，Label 只提交一张缓存纹理；背景和边框仍由 Widget 在内容绘制前处理。

## API

```c
int xgeXuiLabelInit(xge_xui_label label, xge_xui_widget widget, xge_font font, const char* text);
void xgeXuiLabelUnit(xge_xui_label label);

void xgeXuiLabelSetText(xge_xui_label label, const char* text);
void xgeXuiLabelSetFont(xge_xui_label label, xge_font font);

void xgeXuiLabelSetColor(xge_xui_label label, uint32_t color);
void xgeXuiLabelSetDisabledColor(xge_xui_label label, uint32_t color);
void xgeXuiLabelSetAlign(xge_xui_label label, uint32_t textFlags);
void xgeXuiLabelSetUnderline(xge_xui_label label, int underline);
void xgeXuiLabelSetCacheMode(xge_xui_label label, int mode);

xge_vec2_t xgeXuiLabelMeasure(xge_xui_label label);
xge_vec2_t xgeXuiLabelMeasureProc(xge_xui_widget widget, void* user);
void xgeXuiLabelPaintProc(xge_xui_widget widget, void* user);
```

默认值：

- `text`: `NULL` 会当作空字符串。
- `color`: `#FFFFFFFF`。
- `disabledColor`: `#A0A6AEFF`。
- `align`: `XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP`。
- `cacheMode`: `XGE_XUI_CACHE_AUTO`。

## XSON

```json
{
  "type": "label",
  "text": "Label",
  "font": "@fonts.body",
  "textColor": "#243446FF",
  "disabledColor": "#828A96FF",
  "textAlign": "center",
  "textVAlign": "middle",
  "underline": false,
  "cacheMode": "auto"
}
```

字段：

- `text`: 显示文本。支持模型绑定字符串，绑定更新时调用 `xgeXuiLabelSetText`。
- `font`: 字体 token 或字体值。
- `textColor` / `color`: 正常状态文字颜色。
- `disabledColor`: Widget disabled 时使用的文字颜色。
- `textAlign`: `left`、`center`、`right`。
- `textVAlign`: `top`、`middle`、`bottom`。
- `underline`: 是否绘制下划线。
- `cacheMode`: `auto` / `default`、`off` / `none` / `disabled`、`force` / `on`。也可以直接使用整数缓存模式。

Widget 通用字段同样适用，例如 `width`、`height`、`padding`、`margin`、`background`、`borderColor`、`borderWidth`、`radius`、`enabled` 等。

## Examples

- `examples/xui_label`: C API 版本。
- `examples/xui_label_xson`: XSON 描述版本。

两个范例都分成四个区域：九宫格文本对齐、九宫格文本对齐加边框、九宫格文本对齐加背景色，以及字体大小、禁用状态、下划线、缓存模式等自定义状态。
