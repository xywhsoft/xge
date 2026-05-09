# XUI Progress

`xge_xui_progress_t` 是确定型进度显示控件。它在 widget content rect 内绘制未加载轨道、已加载填充区域和可选文本。Widget 背景、边框、padding、裁剪、启用状态和 owner draw 仍然由 Widget 负责。

## 设计

- Progress 是静态数值指示器，不是输入控件。它不获取焦点，也不处理指针事件。
- 控件只负责进度语义：范围、当前值、填充方向、轨道颜色、填充颜色和可选文本。
- 进度文本使用两种颜色。`textColor` 先绘制在整个轨道上，然后同一段文本会被裁剪到填充区域内，再用 `fillTextColor` 绘制一次。
- 文本使用模板字符串。`NULL` 表示不显示文本。包含真实 `%` 的模板会用一个 `double percent` 参数格式化。`%%` 表示字面量百分号。
- 渲染后的文本缓存为 `sDisplayText`，只在模板、范围或当前值变化时刷新。控件主体只是两个矩形加可选文本，所以不使用渲染缓存。
- 外框应使用 Widget 的背景和边框。`trackColor` 和 `fillColor` 只描述进度内容本身。
- 图片进度使用可复用的 `xge_nine_patch_t` 对象。`trackPatch` 绘制未加载轨道，`fillPatch` 绘制当前填充区域。patch 存在时会替代对应的颜色矩形。
- 简单图片进度可以选择填充图的显示方式。`stretch` 会把完整填充图缩放到当前进度区域；`reveal` 会保持完整图片映射，只显示当前进度部分，所以从左到右模式下 50% 会显示图片左半边。

## 九宫格

引擎级九宫格对象由 XUI 和非 XUI 渲染共享：

```c
#define XGE_NINE_PATCH_STRETCH 0
#define XGE_NINE_PATCH_TILE    1

typedef struct xge_nine_patch_t {
    xge_texture pTexture;
    xge_rect_t tSrc;
    float fX1;
    float fY1;
    float fX2;
    float fY2;
    uint32_t iColor;
    int iMode;
    int bEasyMode;
} xge_nine_patch_t;
```

`fX1, fY1, fX2, fY2` 是中心矩形的归一化坐标。它们定义 `#` 字九宫格的四条分割线。简单模式是 `0, 0, 1, 1`；在 stretch 模式下渲染为一张完整图片，在 tile 模式下重复完整源区域。

九宫格 API：

```c
void xgeNinePatchInitSimple(xge_nine_patch patch, xge_texture texture, xge_rect_t src);
void xgeNinePatchInit(xge_nine_patch patch, xge_texture texture, xge_rect_t src, float x1, float y1, float x2, float y2);
void xgeNinePatchSetMode(xge_nine_patch patch, int mode);
void xgeNinePatchSetColor(xge_nine_patch patch, uint32_t color);
void xgeNinePatchDraw(const xge_nine_patch_t* patch, xge_rect_t dst, uint32_t flags);
```

## API

```c
int xgeXuiProgressInit(xge_xui_progress progress, xge_xui_widget widget);
void xgeXuiProgressUnit(xge_xui_progress progress);

void xgeXuiProgressSetRange(xge_xui_progress progress, float min, float max);
void xgeXuiProgressSetValue(xge_xui_progress progress, float value);
float xgeXuiProgressGetValue(xge_xui_progress progress);

void xgeXuiProgressSetText(xge_xui_progress progress, xge_font font, const char* textTemplate);
void xgeXuiProgressSetTextTemplate(xge_xui_progress progress, const char* textTemplate);
void xgeXuiProgressSetTextColor(xge_xui_progress progress, uint32_t color);
void xgeXuiProgressSetFillTextColor(xge_xui_progress progress, uint32_t color);

void xgeXuiProgressSetColors(xge_xui_progress progress, uint32_t track, uint32_t fill);
void xgeXuiProgressSetFillDirection(xge_xui_progress progress, int fillDirection);
void xgeXuiProgressSetTrackPatch(xge_xui_progress progress, const xge_nine_patch_t* patch);
void xgeXuiProgressSetFillPatch(xge_xui_progress progress, const xge_nine_patch_t* patch);
void xgeXuiProgressSetFillPatchMode(xge_xui_progress progress, int mode);

xge_vec2_t xgeXuiProgressMeasureProc(xge_xui_widget widget, void* user);
void xgeXuiProgressPaintProc(xge_xui_widget widget, void* user);
```

常量：

```c
#define XGE_XUI_PROGRESS_LEFT_TO_RIGHT 0
#define XGE_XUI_PROGRESS_RIGHT_TO_LEFT 1
#define XGE_XUI_PROGRESS_BOTTOM_TO_TOP 2
#define XGE_XUI_PROGRESS_TOP_TO_BOTTOM 3

#define XGE_XUI_PROGRESS_FILL_STRETCH 0
#define XGE_XUI_PROGRESS_FILL_REVEAL 1
```

模板示例：

```c
xgeXuiProgressSetText(progress, font, NULL);          // 不显示文本
xgeXuiProgressSetText(progress, font, "%1.0f%%");     // 42%
xgeXuiProgressSetText(progress, font, "Loading");     // 静态文本
xgeXuiProgressSetText(progress, font, "进度：%1.2f"); // 格式化百分比值
```

模板解析器刻意保持轻量：任何不是 `%%` 的单个 `%` 都会把模板标记为动态模板。动态模板接收一个 `double percent` 参数。

## XSON

```json
{
  "type": "progress",
  "min": 0,
  "max": 100,
  "value": 42,
  "font": "@fonts.body",
  "text": "%1.0f%%",
  "fillDirection": "leftToRight",
  "trackColor": "#D8ECF8",
  "fillColor": "#2E7CD6",
  "textColor": "#243446",
  "fillTextColor": "#FFFFFF",
  "trackPatch": {
    "texture": "@textures.progressTrack",
    "center": [0.375, 0.25, 0.625, 0.75],
    "mode": "stretch"
  },
  "fillPatch": {
    "texture": "@textures.progressFill",
    "mode": "stretch"
  },
  "fillPatchMode": "reveal"
}
```

字段：

- `min`, `max`, `value`: 数值范围和当前值。
- `font`: 可选字体 token 或字体值。
- `text`: 可选模板字符串。`NULL` 或缺省表示不显示文本。
- `fillDirection`: `leftToRight`、`rightToLeft`、`bottomToTop` 或 `topToBottom`。也接受别名 `rtl`、`btt`、`ttb`。
- `trackColor`: 未加载轨道颜色。Alpha 为 0 时跳过轨道绘制。
- `fillColor`: 已加载填充颜色。Alpha 为 0 时跳过填充绘制。
- `textColor`: 轨道/未填充区域上的文本颜色。
- `fillTextColor`: 裁剪到填充区域内的文本颜色。
- `trackPatch`: 可选九宫格对象，用于未加载轨道。
- `fillPatch`: 可选九宫格对象，用于已加载填充。
- `fillPatchMode`: `stretch` 或 `reveal`。`reveal` 面向简单图片填充；九宫格填充仍绘制到当前填充矩形中，以保持圆角端部正确。

九宫格字段：

- `texture`: 纹理 token 或纹理指针值。
- `src`: 可选 `[x, y, w, h]` 源矩形。缺省或尺寸为 0 表示完整纹理。
- `center`: 可选 `[x1, y1, x2, y2]` 归一化中心矩形。缺省表示简单模式。
- `mode`: `stretch` 或 `tile`。
- `color`: 可选 patch 颜色乘算。

## Examples

- `examples/xui_progress`: C API 版本。
- `examples/xui_progress_xson`: XSON 版本。

两个范例使用一个纵向布局，包含十组 label/progress：空白文本、百分比文本、静态文本、自定义模板文本、自定义配色、无边框透明外背景、图片拉伸进度、图片裁剪进度、九宫格拉伸进度和九宫格平铺进度。
