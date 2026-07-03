# 文本与字体入门

> 状态：中文初稿已生成，待审阅。

XGE 使用 UTF-8 作为唯一文本编码。字体可以从 TTF 加载，也可以从 XRF 点阵字体缓存加载。XRF 的目标是快速加载、快速渲染，减少运行时字形栅格化成本。

## 直接加载 TTF

```c
xge_font_t font;
memset(&font, 0, sizeof(font));

if ( xgeFontLoad(&font, "assets/ui.ttf", 18.0f) == XGE_OK ) {
	xgeTextDraw(&font, "Hello XGE", 40.0f, 40.0f, xgeColorRGBA(255, 255, 255, 255));
	xgeFontFree(&font);
}
```

TTF 灵活，但首次绘制某些字形时可能发生栅格化。

## 使用 XRF 缓存

```c
xgeFontLoadXRF(&font, "assets/ui_18.xrf");
```

如果你希望优先使用缓存，缓存不存在时从 TTF 构建：

```c
xgeFontLoadCached(&font, "assets/ui.ttf", "assets/ui_18.xrf", 18.0f, 0x4E00, 0x9FA5 - 0x4E00 + 1);
```

中文 UI 或固定字库游戏更适合使用 XRF。

## 测量文本

```c
xge_vec2_t size = xgeTextMeasure(&font, "开始游戏");
```

测量结果可用于 UI 布局、按钮尺寸、居中和裁剪判断。

## 矩形内绘制

```c
xge_rect_t rc = { 20.0f, 20.0f, 200.0f, 40.0f };
xgeTextDrawRect(
	&font,
	"开始游戏",
	rc,
	xgeColorRGBA(255, 255, 255, 255),
	XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP
);
```

## Fallback font

如果主字体不包含中文，可以设置 fallback。

```c
xgeFontFallbackSet("assets/chinese.ttf", 18.0f);
xgeFontFallbackGet(&font, 18.0f);
```

第一版目标支持中文 UCS2 范围。emoji 暂不作为第一版目标。

## UTF-8 遍历

```c
const char* p = "中文";
uint32_t cp;
while ( xgeTextUTF8Next(&p, &cp) == XGE_OK ) {
	/* cp */
}
```

业务层应统一使用 UTF-8，不在 XGE API 层混用本地编码。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 中文不显示 | 字体不含对应字形 | 使用中文字体或 fallback。 |
| 首次显示卡顿 | TTF 动态栅格化 | 使用 XRF 或预热常用字形。 |
| 文本乱码 | 输入不是 UTF-8 | 在进入 XGE 前转换为 UTF-8。 |

## 下一步

- 查完整函数看 [Font / Text API](../api/font-text.md)。
- 图形绘制路径看 [Shape、Sprite 与文本绘制入门](drawing-intro.md)。

[返回教程入口](README.md)
