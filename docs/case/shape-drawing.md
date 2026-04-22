# Shape 绘制

本案例展示点、线、矩形、圆、弧、三角形和多边形的基础绘制。

[返回范例解析](README.md) | [绘制教程](../guide/drawing-intro.md) | [Drawing API](../api/drawing.md)

## 问题

Shape 不依赖纹理资源，适合调试、占位、编辑器辅助线、UI 背景和简单 2D 效果。

## 基础图形

```c
xgeClear(xgeColorRGBA(18, 22, 30, 255));

xgeShapePoint(32.0f, 32.0f, 4.0f, xgeColorRGBA(255, 255, 255, 255));
xgeShapeLine(48.0f, 48.0f, 220.0f, 80.0f, 2.0f, xgeColorRGBA(80, 180, 240, 255));
xgeShapeRectFill((xge_rect_t){ 40.0f, 120.0f, 160.0f, 80.0f }, xgeColorRGBA(60, 130, 220, 255));
xgeShapeRectStroke((xge_rect_t){ 240.0f, 120.0f, 160.0f, 80.0f }, 3.0f, xgeColorRGBA(220, 220, 80, 255));
xgeShapeCircleFill(120.0f, 280.0f, 48.0f, xgeColorRGBA(80, 200, 120, 255));
xgeShapeCircleStroke(280.0f, 280.0f, 48.0f, 4.0f, xgeColorRGBA(240, 100, 80, 255));
```

## 多边形

```c
xge_vec2_t points[5] = {
	{ 460.0f, 120.0f },
	{ 540.0f, 160.0f },
	{ 520.0f, 240.0f },
	{ 440.0f, 240.0f },
	{ 420.0f, 160.0f }
};

xgeShapePolygonFill(points, 5, xgeColorRGBA(160, 120, 240, 255));
```

## 批量绘制

大量同色三角形可使用 shape batch：

```c
xge_shape_batch_t batch;

xgeShapeBatchInit(&batch, xgeColorRGBA(255, 255, 255, 255), 1024, 0);
xgeShapeBatchRectFill(&batch, (xge_rect_t){ 0.0f, 0.0f, 32.0f, 32.0f });
xgeShapeBatchFlush(&batch);
xgeShapeBatchFree(&batch);
```

## 常见失败原因

图形位置不对：检查 camera、viewport 和坐标系。

线条太粗或太细：不同后端线宽支持差异较大，复杂描边后续应走 mesh。

多边形异常：第一版应优先使用凸多边形，复杂凹多边形建议业务侧三角化。
