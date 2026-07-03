# 文本渲染和 XRF 缓存

本案例展示加载 TTF 字体、绘制 UTF-8 文本，并使用 XRF 缓存提升常用字渲染速度。

[返回范例解析](README.md) | [文字教程](../guide/text-intro.md) | [Font/Text API](../api/font-text.md)

## 问题

中文 UI 和游戏文字需要稳定的 UTF-8 输入、字体 fallback、测量和快速绘制。XGE 第一版支持 UCS2 范围中文，emoji 暂不作为 MVP 目标。

## 步骤 1：加载字体

```c
static xge_font_t g_font;

if ( xgeFontLoad(&g_font, "assets/fonts/ui.ttf", 18.0f) != XGE_OK ) {
	return XGE_ERROR_IO;
}
```

## 步骤 2：测量文本

```c
xge_vec2_t size;

size = xgeTextMeasure(&g_font, "开始游戏");
```

测量结果可用于按钮、标签、列表项和布局系统的 content size。

## 步骤 3：绘制单行和矩形文本

```c
xgeTextDraw(&g_font, "开始游戏", 32.0f, 40.0f, xgeColorRGBA(255, 255, 255, 255));

xgeTextDrawRect(
	&g_font,
	"这是一段可以放进矩形区域的说明文字。",
	(xge_rect_t){ 32.0f, 80.0f, 360.0f, 120.0f },
	xgeColorRGBA(220, 230, 240, 255),
	XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP
);
```

## 步骤 4：使用 XRF 缓存

```c
if ( xgeFontLoadCached(&g_font, "assets/fonts/ui.ttf", "cache/ui_18.xrf", 18.0f, 0x4E00, 0x9FA5 - 0x4E00 + 1) != XGE_OK ) {
	xgeFontLoad(&g_font, "assets/fonts/ui.ttf", 18.0f);
}
```

XRF 适合常用字点阵缓存，也可以作为 TTF glyph atlas 的快速启动后端。

## 步骤 5：fallback 字体

```c
static xge_font_t g_fallback;

xgeFontLoad(&g_fallback, "assets/fonts/cjk.ttf", 18.0f);
xgeFontSetFallback(&g_font, &g_fallback);
```

业务字体缺字时，fallback 可以避免出现大量方块。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeTextUTF8Next` | 遍历 UTF-8 codepoint |
| `xgeFontLoad` | 加载 TTF |
| `xgeFontLoadXRF` | 加载 XRF 位图字体 |
| `xgeFontLoadCached` | 优先使用缓存，不存在则从 TTF 构建 |
| `xgeTextMeasure` | 测量文本尺寸 |
| `xgeTextDraw` | 绘制单行文本 |
| `xgeTextDrawRect` | 在矩形内绘制文本 |

## 常见问题

如果中文显示为空，检查字体是否包含对应 UCS2 字符，以及输入字符串是否为 UTF-8。

如果启动时构建 XRF 很慢，把 XRF 生成放到工具链或资源打包阶段，不要每次运行都重新生成。
