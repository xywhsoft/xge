# Shape Full Gallery

`examples/shape_full_gallery` 是 shape API 的完整画廊范例。它把 pixel-space、world-space、显式 shape batch 和 auto batch 都放进同一窗口：上左展示 pixel-space 形状，上右展示 `xgeShapeBatch*` 聚合绘制，下方展示 world-space 形状。

## 覆盖 API

- `xgeShapePoint`
- `xgeShapePointPx`
- `xgeShapeLine`
- `xgeShapeLinePx`
- `xgeShapeRectFill`
- `xgeShapeRectFillPx`
- `xgeShapeRectStroke`
- `xgeShapeRectStrokePx`
- `xgeShapeCircleFill`
- `xgeShapeCircleFillPx`
- `xgeShapeCircleStroke`
- `xgeShapeCircleStrokePx`
- `xgeShapeArc`
- `xgeShapeArcPx`
- `xgeShapeTriangleFill`
- `xgeShapeTriangleFillPx`
- `xgeShapePolygonFill`
- `xgeShapePolygonFillPx`
- `xgeShapeBatchInit`
- `xgeShapeBatchFree`
- `xgeShapeBatchClear`
- `xgeShapeBatchTriangleFill`
- `xgeShapeBatchRectFill`
- `xgeShapeBatchFlush`

## 构建和运行

```bat
examples\shape_full_gallery\build.bat
build\xge_shape_full_gallery.exe --frames 120
```

## 验证点

- 控制台输出 `shape-full-gallery init`。
- `pixel=1 world=1` 表示 world-space 和 pixel-space 两组 API 都已进入绘制路径。
- `point/line/rect/circle/arc/triangle/polygon` 都应为 `1`。
- `batch=1/1/1/1/1` 表示显式 `shape batch` 的 init/clear/rect/triangle/flush 都通过。
- `auto=1 batch_path=1` 且 `stats(batch=2...)` 或更大，表示 auto batch 和显式 batch 都实际提交过。

## 视觉检查表

- 左上面板应同时看到点、粗线、填充矩形、描边矩形、填充圆、描边圆、弧线、三角形和五边形。
- 右上面板应看到由同色矩形和三角形组成的一组 batch 图形，没有缺角、闪烁或断裂。
- 下方面板应看到与左上相同类别的 world-space 图形，但位置由底部 viewport 内的中心坐标系决定。
- 整个窗口运行时不应黑屏；按 `ESC` 或 `--frames` 到达后应打印 summary 并退出。
