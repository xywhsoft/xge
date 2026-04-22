# Shape、Sprite 与文本绘制入门

> 状态：中文初稿已生成，待审阅。

XGE 的绘制 API 分三层：shape 用于无资源绘制，sprite 用于纹理绘制，text 用于字体绘制。高级效果再使用 material、mesh 和 render target。

## 先从 shape 开始

Shape 不依赖外部资源，适合调试、占位、UI 背景和简单几何。

```c
xgeClear(xgeColorRGBA(24, 32, 48, 255));

xge_rect_t rect;
rect.fX = 40.0f;
rect.fY = 40.0f;
rect.fW = 240.0f;
rect.fH = 120.0f;
xgeShapeRectFill(rect, xgeColorRGBA(64, 128, 220, 255));

xgeShapeCircleFill(360.0f, 100.0f, 48.0f, xgeColorRGBA(240, 96, 72, 255));
```

## 绘制 sprite

最简单的 sprite 绘制只需要纹理和坐标。

```c
xgeDraw(&tex, 100.0f, 120.0f);
```

需要源矩形、目标矩形、旋转、颜色或翻转时，使用 `xgeDrawEx`。

```c
xge_draw_t draw;
memset(&draw, 0, sizeof(draw));
draw.pTexture = &tex;
draw.tSrc = (xge_rect_t){ 0.0f, 0.0f, 64.0f, 64.0f };
draw.tDst = (xge_rect_t){ 100.0f, 120.0f, 128.0f, 128.0f };
draw.tOrigin = (xge_vec2_t){ 64.0f, 64.0f };
draw.fRotation = 0.25f;
draw.iColor = xgeColorRGBA(255, 255, 255, 255);
xgeDrawEx(&draw);
```

## 大量 sprite 使用 batch

如果一帧绘制大量同纹理 sprite，使用 `xgeSpriteBatch*` 可以减少提交成本。

```c
xge_sprite_batch_t batch;
xgeSpriteBatchInit(&batch, &tex, 1024, 0);

for ( int i = 0; i < count; ++i ) {
	xgeSpriteBatchAdd(&batch, &draws[i]);
}

xgeSpriteBatchFlush(&batch);
xgeSpriteBatchFree(&batch);
```

Batch 最重要的限制是同批次绑定同一张纹理。上层可以按纹理排序，把 sprite 分成多个 batch。

## 绘制文本

文本绘制需要先加载字体。

```c
xge_font_t font;
memset(&font, 0, sizeof(font));

if ( xgeFontLoad(&font, "assets/ui.ttf", 18.0f) == XGE_OK ) {
	xgeTextDraw(&font, "Hello XGE", 40.0f, 40.0f, xgeColorRGBA(255, 255, 255, 255));
	xgeFontFree(&font);
}
```

需要在矩形里对齐和裁剪时，使用 `xgeTextDrawRect`。

## 2.5D quad

`xgeDrawQuad3D` 接收 4 个 `xge_vertex_t`，用于图像扭曲、透视卡片、伪 3D 地面和特殊 UI 变形。

如果只是普通 2D sprite，不要过早使用 2.5D API。它更适合高级效果。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 大量 sprite 帧率低 | 没有 batch 或纹理切换太多 | 按纹理排序并使用 `xgeSpriteBatch*`。 |
| 文本第一次显示卡顿 | TTF 字形首次栅格化 | 使用 XRF 缓存或预热常用字。 |
| 旋转中心不对 | `tOrigin` 没设置为目标局部中心 | 根据 `tDst` 尺寸设置 origin。 |
| GUI 裁剪失效 | 没设置 clip | 使用 `xgeClipSet` / `xgeClipClear` 或让 XUI host 处理。 |

## 下一步

- 查 sprite 和 shape 函数看 [Drawing API](../api/drawing.md)。
- 查文本和字体看 [Font / Text API](../api/font-text.md)。
- 想做后处理继续看 [RenderTarget 与离屏渲染入门](render-target-intro.md)。

[返回教程入口](README.md)
