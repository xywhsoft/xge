# XUI Image

`xge_xui_image_t` 是静态图片控件。它只负责把一张纹理的矩形区域绘制到控件内容区里的矩形区域；背景、边框、圆角、padding、clip、enabled 和 owner draw 都由 Widget 提供。

## 设计思路

- Image 的第一目标是显示静态图片，不承担 Canvas、Sprite、Tile 或自由四点变形职责。
- 默认模式按图片原始尺寸显示，并通过九宫格对齐决定位置。
- 裁剪和自定义显示区域都使用两个顶点表达：`x1, y1, x2, y2`。
- 高级但仍属于图片显示的模式包括 `stretch`、`contain`、`cover`、`scaleDown` 和 `custom`。
- Image 不使用缓存。纹理本身已经是 GPU 资源，正常图片绘制就是两个三角形。

## 模式

| 模式 | 说明 |
| --- | --- |
| `XGE_XUI_IMAGE_NATURAL` | 原尺寸显示，按 `alignX/alignY` 九宫格对齐。默认模式。 |
| `XGE_XUI_IMAGE_STRETCH` | 拉伸填满 content rect。 |
| `XGE_XUI_IMAGE_CONTAIN` | 等比例缩放，完整显示图片。 |
| `XGE_XUI_IMAGE_COVER` | 等比例缩放，填满 content rect，允许超出后被 clip。 |
| `XGE_XUI_IMAGE_SCALE_DOWN` | 原尺寸能放下就按原尺寸显示，放不下就 `contain`。 |
| `XGE_XUI_IMAGE_CUSTOM` | 使用 `customRect` 指定目标矩形。 |

`XGE_XUI_IMAGE_FIT` 是 `XGE_XUI_IMAGE_CONTAIN` 的别名。

## API

```c
int xgeXuiImageInit(xge_xui_image image, xge_xui_widget widget, xge_texture texture);
void xgeXuiImageUnit(xge_xui_image image);

void xgeXuiImageSetTexture(xge_xui_image image, xge_texture texture);
void xgeXuiImageSetSourceRect(xge_xui_image image, float x1, float y1, float x2, float y2);
void xgeXuiImageClearSource(xge_xui_image image);

void xgeXuiImageSetMode(xge_xui_image image, int mode);
void xgeXuiImageSetAlign(xge_xui_image image, int alignX, int alignY);
void xgeXuiImageSetCustomRect(xge_xui_image image, float x1, float y1, float x2, float y2);
void xgeXuiImageSetTint(xge_xui_image image, uint32_t color);
```

`xgeXuiImageSetSource` 和 `xgeXuiImageSetColor` 仍可直接操作底层 rect/tint；新代码优先使用两点语义的 `SetSourceRect` 和语义更清楚的 `SetTint`。

## XSON

```json
{
  "type": "image",
  "texture": "@textures.demo",
  "source": [8, 8, 72, 48],
  "mode": "natural",
  "alignX": "center",
  "alignY": "middle",
  "tint": "#FFFFFFFF"
}
```

字段：

- `texture`: 纹理 token，例如 `@textures.demo`。
- `src`: 图片路径或模型绑定。
- `source` / `srcRect`: 源图片裁剪区域，格式为 `[x1, y1, x2, y2]`。
- `mode`: `natural`、`stretch`、`contain`、`fit`、`cover`、`scaleDown`、`custom`。
- `alignX`: `left`、`center`、`right`。
- `alignY`: `top`、`middle`、`bottom`。
- `customRect` / `targetRect`: custom 模式目标区域，格式为 `[x1, y1, x2, y2]`，坐标相对 content rect。
- `tint` / `color`: 图片颜色乘算。

## 范例

- `examples/xui_image`: C API 版本。
- `examples/xui_image_xson`: XSON 描述版本。

两个范例都分为四个区域：九宫格对齐、九宫格对齐加边框、九宫格对齐加背景填充，以及 stretch/contain/cover/scaleDown/custom 等其他显示方式。
