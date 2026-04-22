# Drawing API

> Drawing API 负责提交 2D sprite、2.5D quad、shape 和 batch 绘制命令。

[返回 API 索引](README.md) | [绘制教程](../guide/drawing-intro.md) | [Texture / Sprite 范例](../case/texture-sprite.md)

---

## 模块定位

Drawing 是最常用的渲染入口。它把纹理、形状、顶点和颜色转换为渲染命令，默认在当前帧末 flush。它不负责加载纹理，不负责创建 render target，也不负责 shader/material 的高级状态管理。

简单游戏可直接使用 `xgeDraw`、`xgeDrawEx` 和 shape immediate API。大量同纹理 sprite 使用 `xgeSpriteBatch*`。图像扭曲、透视卡片、伪 3D 地图等使用 `xgeDrawQuad3D` 或 Mesh/Material 高级 API。

## 标准调用顺序

```text
xgeClear
  -> xgeDraw / xgeDrawEx / xgeShape*
  -> 默认帧末 flush
```

手动模式下需要显式 `xgeBegin` / `xgeEnd` / `xgeFlush`，详见 [Graphics API](graphics.md)。

## 公共类型

### `xge_draw_t`

Sprite 绘制描述。包含纹理、源矩形、目标矩形、旋转原点、旋转角、颜色和 `XGE_DRAW_*` 标志。

### `xge_vertex_t`

2.5D 和 Mesh 使用的顶点。包含位置 `fX/fY/fZ/fW`、纹理坐标 `fU/fV` 和顶点颜色 `iColor`。

### `xge_sprite_batch_t`

同纹理 sprite batch。调用者持有对象，内部缓存由 `xgeSpriteBatchInit` 分配，`xgeSpriteBatchFree` 释放。

### `xge_shape_batch_t`

shape batch 对象，用于减少大量同色三角形或矩形的提交成本。

## API 列表

- `xgeDraw`
- `xgeDrawEx`
- `xgeDrawQuad3D`
- `xgeDrawPx`
- `xgeSpriteBatchInit`
- `xgeSpriteBatchFree`
- `xgeSpriteBatchClear`
- `xgeSpriteBatchAdd`
- `xgeSpriteBatchFlush`
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

## Sprite

### xgeDraw

绘制完整纹理。

**功能：**

你可以用纹理原始尺寸在逻辑坐标中绘制一个 sprite。

**函数原型：**

```c
XGE_API void xgeDraw(xge_texture pTexture, float fX, float fY);
```

**参数：**

- `pTexture`：输入参数，可以为 `NULL`。为 `NULL` 时忽略。
- `fX` / `fY`：输入参数，目标位置，单位为逻辑坐标。

**返回值：**

无。

**资源归属：**

函数不接管纹理所有权。纹理必须在命令提交期间有效。

**补充说明：**

- 当前实现会构造默认 `xge_draw_t` 并调用 `xgeDrawEx`。
- 默认颜色为白色不调制。

**范例代码：**

```c
xgeDraw(&tex, 100.0f, 120.0f);
```

**相关 API：**

- `xgeDrawEx`
- `xgeDrawPx`
- `xgeTextureLoad`

---

### xgeDrawEx

使用完整绘制描述绘制 sprite。

**功能：**

你可以控制源矩形、目标矩形、旋转、颜色调制和绘制标志。

**函数原型：**

```c
XGE_API void xgeDrawEx(const xge_draw_t* pDraw);
```

**参数：**

- `pDraw`：输入参数，指向绘制描述。无效描述会被底层命令提交路径忽略或返回错误。

**返回值：**

无。

**资源归属：**

函数会读取 `pDraw` 内容，不保存 `pDraw` 指针。`pDraw->pTexture` 不转移所有权。

**补充说明：**

- `tSrc` 为 0 时通常表示使用完整纹理区域。
- `XGE_DRAW_SCREEN_SPACE` 可让绘制不受 camera 影响。

**范例代码：**

```c
xge_draw_t draw;
memset(&draw, 0, sizeof(draw));
draw.pTexture = &tex;
draw.tDst = (xge_rect_t){ 100.0f, 120.0f, 64.0f, 64.0f };
draw.iColor = xgeColorRGBA(255, 255, 255, 255);
xgeDrawEx(&draw);
```

**相关 API：**

- `xgeDraw`
- `xgeSpriteBatchAdd`
- `xgeMaterialDraw`

---

### xgeDrawQuad3D

绘制 2.5D 四边形。

**功能：**

你可以用 4 个顶点绘制带 Z/W 和 UV 的纹理四边形，用于透视扭曲、卡牌翻转、伪 3D 地面块或特殊 UI 变形。

**函数原型：**

```c
XGE_API void xgeDrawQuad3D(xge_texture pTexture, const xge_vertex_t* pVertices, uint32_t iFlags);
```

**参数：**

- `pTexture`：输入参数，纹理对象，可以为 `NULL`，具体容错由实现决定。
- `pVertices`：输入参数，指向 4 个顶点，不能少于 4 个。
- `iFlags`：输入参数，`XGE_DRAW_*` 标志。

**返回值：**

无。

**资源归属：**

函数不接管纹理或顶点内存。调用期间顶点数组必须有效。

**补充说明：**

- 2.5D 不是通用 3D 管线，复杂 3D 应使用更高层或自定义 material/mesh。
- 需要深度遮挡时配合 `xgeDepthTestSet` 和 `xgeCameraPerspective`。

**范例代码：**

```c
xge_vertex_t v[4] = {0};
xgeDrawQuad3D(&tex, v, 0);
```

**相关 API：**

- `xgeCameraPerspective`
- `xgeDepthTestSet`
- `xgeMeshDraw`

---

### xgeDrawPx

用像素坐标绘制完整纹理。

**功能：**

你可以跳过 camera 映射，用屏幕空间像素位置绘制纹理。

**函数原型：**

```c
XGE_API void xgeDrawPx(xge_texture pTexture, int iX, int iY);
```

**参数：**

- `pTexture`：输入参数，可以为 `NULL`。为 `NULL` 时忽略。
- `iX` / `iY`：输入参数，像素位置。

**返回值：**

无。

**资源归属：**

函数不接管纹理所有权。

**补充说明：**

- 当前实现设置 `XGE_DRAW_SCREEN_SPACE`。

**范例代码：**

```c
xgeDrawPx(&tex, 10, 20);
```

**相关 API：**

- `xgeDraw`
- `xgeViewportSet`
- `xgeClipSet`

---

## Sprite Batch

### xgeSpriteBatchInit

初始化同纹理 sprite batch。

**功能：**

你可以为大量同纹理 sprite 创建批处理缓存，减少提交次数。

**函数原型：**

```c
XGE_API int xgeSpriteBatchInit(xge_sprite_batch pBatch, xge_texture pTexture, int iCapacity, uint32_t iFlags);
```

**参数：**

- `pBatch`：输出参数，不能为 `NULL`。
- `pTexture`：输入参数，绑定纹理，不能为 `NULL`。
- `iCapacity`：输入参数，batch 容量，必须大于 `0`。
- `iFlags`：输入参数，绘制标志。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或内存分配失败时返回对应错误码。

**资源归属：**

batch 对象由调用者持有；内部缓存由 XGE 分配，必须调用 `xgeSpriteBatchFree`。

**补充说明：**

- 一个 batch 绑定一个纹理。跨纹理绘制应拆分多个 batch。

**范例代码：**

```c
xge_sprite_batch_t batch;
xgeSpriteBatchInit(&batch, &tex, 1024, 0);
```

**相关 API：**

- `xgeSpriteBatchAdd`
- `xgeSpriteBatchFlush`
- `xgeSpriteBatchFree`

---

### xgeSpriteBatchFree

释放 sprite batch。

**功能：**

你可以释放 `xgeSpriteBatchInit` 创建的内部缓存。

**函数原型：**

```c
XGE_API void xgeSpriteBatchFree(xge_sprite_batch pBatch);
```

**参数：**

- `pBatch`：输入/输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 batch 内部缓存并清空对象。

**补充说明：**

- 不释放绑定纹理，纹理生命周期由调用者管理。

**范例代码：**

```c
xgeSpriteBatchFree(&batch);
```

**相关 API：**

- `xgeSpriteBatchInit`
- `xgeTextureFree`
- `xgeSpriteBatchClear`

---

### xgeSpriteBatchClear

清空 sprite batch。

**功能：**

你可以复用 batch 内部缓存，清除已添加 sprite。

**函数原型：**

```c
XGE_API void xgeSpriteBatchClear(xge_sprite_batch pBatch);
```

**参数：**

- `pBatch`：输入/输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放缓存，只重置计数。

**补充说明：**

- 适合每帧重用 batch。

**范例代码：**

```c
xgeSpriteBatchClear(&batch);
```

**相关 API：**

- `xgeSpriteBatchAdd`
- `xgeSpriteBatchFlush`
- `xgeSpriteBatchFree`

---

### xgeSpriteBatchAdd

向 sprite batch 添加一个 sprite。

**功能：**

你可以把一个 `xge_draw_t` 加入同纹理 batch。

**函数原型：**

```c
XGE_API int xgeSpriteBatchAdd(xge_sprite_batch pBatch, const xge_draw_t* pDraw);
```

**参数：**

- `pBatch`：输入/输出参数，不能为 `NULL`。
- `pDraw`：输入参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或 batch 已满时返回对应错误码。

**资源归属：**

函数会复制绘制数据到 batch 缓存，不保存 `pDraw` 指针。

**补充说明：**

- `pDraw->pTexture` 应与 batch 绑定纹理一致，或由实现按 batch 纹理处理。

**范例代码：**

```c
xgeSpriteBatchAdd(&batch, &draw);
```

**相关 API：**

- `xgeSpriteBatchInit`
- `xgeSpriteBatchFlush`
- `xgeDrawEx`

---

### xgeSpriteBatchFlush

提交 sprite batch。

**功能：**

你可以把 batch 中积累的 sprite 作为较少的绘制命令提交。

**函数原型：**

```c
XGE_API int xgeSpriteBatchFlush(xge_sprite_batch pBatch);
```

**参数：**

- `pBatch`：输入/输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或提交失败时返回对应错误码。

**资源归属：**

不释放 batch。提交后是否清空由实现约定，调用者可显式调用 `xgeSpriteBatchClear`。

**补充说明：**

- batch 的性能收益来自减少状态切换和命令数量。

**范例代码：**

```c
xgeSpriteBatchFlush(&batch);
```

**相关 API：**

- `xgeSpriteBatchAdd`
- `xgeSpriteBatchClear`
- `xgeFlush`

---

## Shape Immediate

### xgeShapePoint

绘制逻辑坐标点。

**功能：**

你可以提交一个点形状，用于调试标记或轻量绘制。

**函数原型：**

```c
XGE_API void xgeShapePoint(float fX, float fY, float fSize, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，逻辑坐标。
- `fSize`：输入参数，点尺寸。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 受当前 camera 影响。

**范例代码：**

```c
xgeShapePoint(10.0f, 20.0f, 4.0f, xgeColorRGBA(255, 0, 0, 255));
```

**相关 API：**

- `xgeShapePointPx`
- `xgeShapeLine`
- `xgeColorRGBA`

---

### xgeShapePointPx

绘制像素坐标点。

**功能：**

你可以用屏幕空间坐标绘制点。

**函数原型：**

```c
XGE_API void xgeShapePointPx(float fX, float fY, float fSize, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，像素坐标。
- `fSize`：输入参数，点尺寸。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 像素版本通常不受 camera 影响。

**范例代码：**

```c
xgeShapePointPx(10.0f, 20.0f, 4.0f, xgeColorRGBA(255, 0, 0, 255));
```

**相关 API：**

- `xgeShapePoint`
- `xgeDrawPx`
- `xgeViewportSet`

---

### xgeShapeLine

绘制逻辑坐标线段。

**功能：**

你可以绘制一条带宽度的线段。

**函数原型：**

```c
XGE_API void xgeShapeLine(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
```

**参数：**

- `fX0` / `fY0`：输入参数，起点逻辑坐标。
- `fX1` / `fY1`：输入参数，终点逻辑坐标。
- `fWidth`：输入参数，线宽。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 后端线宽能力可能存在差异，复杂描边建议转 mesh。

**范例代码：**

```c
xgeShapeLine(0.0f, 0.0f, 100.0f, 100.0f, 2.0f, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapeLinePx`
- `xgeShapeRectStroke`
- `xgeShapeBatchTriangleFill`

---

### xgeShapeLinePx

绘制像素坐标线段。

**功能：**

你可以用屏幕空间坐标绘制线段。

**函数原型：**

```c
XGE_API void xgeShapeLinePx(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
```

**参数：**

- `fX0` / `fY0`：输入参数，起点像素坐标。
- `fX1` / `fY1`：输入参数，终点像素坐标。
- `fWidth`：输入参数，线宽。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 适合 UI、debug overlay 和工具绘制。

**范例代码：**

```c
xgeShapeLinePx(0.0f, 0.0f, 100.0f, 100.0f, 1.0f, xgeColorRGBA(255, 255, 0, 255));
```

**相关 API：**

- `xgeShapeLine`
- `xgeClipSet`
- `xgeDrawPx`

---

### xgeShapeRectFill

绘制逻辑坐标填充矩形。

**功能：**

你可以绘制一个填充矩形，用于 UI 背景、调试块或简单 2D 图形。

**函数原型：**

```c
XGE_API void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor);
```

**参数：**

- `tRect`：输入参数，逻辑坐标矩形。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 受当前 camera 影响。

**范例代码：**

```c
xgeShapeRectFill((xge_rect_t){ 0, 0, 100, 50 }, xgeColorRGBA(0, 0, 0, 255));
```

**相关 API：**

- `xgeShapeRectFillPx`
- `xgeShapeRectStroke`
- `xgeShapeBatchRectFill`

---

### xgeShapeRectFillPx

绘制像素坐标填充矩形。

**功能：**

你可以绘制屏幕空间填充矩形。

**函数原型：**

```c
XGE_API void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor);
```

**参数：**

- `tRect`：输入参数，像素坐标矩形。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 适合 UI、debug overlay 和 APP 模式。

**范例代码：**

```c
xgeShapeRectFillPx((xge_rect_t){ 0, 0, 100, 50 }, xgeColorRGBA(0, 0, 0, 255));
```

**相关 API：**

- `xgeShapeRectFill`
- `xgeClipSet`
- `xgeViewportSet`

---

### xgeShapeRectStroke

绘制逻辑坐标矩形描边。

**功能：**

你可以绘制矩形边框。

**函数原型：**

```c
XGE_API void xgeShapeRectStroke(xge_rect_t tRect, float fWidth, uint32_t iColor);
```

**参数：**

- `tRect`：输入参数，逻辑坐标矩形。
- `fWidth`：输入参数，描边宽度。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 受当前 camera 影响。

**范例代码：**

```c
xgeShapeRectStroke((xge_rect_t){ 0, 0, 100, 50 }, 2.0f, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapeRectStrokePx`
- `xgeShapeLine`
- `xgeShapeRectFill`

---

### xgeShapeRectStrokePx

绘制像素坐标矩形描边。

**功能：**

你可以绘制屏幕空间矩形边框。

**函数原型：**

```c
XGE_API void xgeShapeRectStrokePx(xge_rect_t tRect, float fWidth, uint32_t iColor);
```

**参数：**

- `tRect`：输入参数，像素坐标矩形。
- `fWidth`：输入参数，描边宽度。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 适合 UI 和 debug overlay。

**范例代码：**

```c
xgeShapeRectStrokePx((xge_rect_t){ 0, 0, 100, 50 }, 1.0f, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapeRectStroke`
- `xgeClipSet`
- `xgeShapeLinePx`

---

### xgeShapeCircleFill

绘制逻辑坐标填充圆。

**功能：**

你可以绘制一个填充圆形。

**函数原型：**

```c
XGE_API void xgeShapeCircleFill(float fX, float fY, float fRadius, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，圆心逻辑坐标。
- `fRadius`：输入参数，半径。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 圆形细分策略由实现决定。

**范例代码：**

```c
xgeShapeCircleFill(50.0f, 50.0f, 24.0f, xgeColorRGBA(255, 0, 0, 255));
```

**相关 API：**

- `xgeShapeCircleFillPx`
- `xgeShapeCircleStroke`
- `xgeShapeArc`

---

### xgeShapeCircleFillPx

绘制像素坐标填充圆。

**功能：**

你可以绘制屏幕空间填充圆。

**函数原型：**

```c
XGE_API void xgeShapeCircleFillPx(float fX, float fY, float fRadius, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，圆心像素坐标。
- `fRadius`：输入参数，半径。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 适合 UI 和 debug overlay。

**范例代码：**

```c
xgeShapeCircleFillPx(50.0f, 50.0f, 24.0f, xgeColorRGBA(255, 0, 0, 255));
```

**相关 API：**

- `xgeShapeCircleFill`
- `xgeShapeCircleStrokePx`
- `xgeClipSet`

---

### xgeShapeCircleStroke

绘制逻辑坐标圆描边。

**功能：**

你可以绘制圆形边框。

**函数原型：**

```c
XGE_API void xgeShapeCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，圆心逻辑坐标。
- `fRadius`：输入参数，半径。
- `fWidth`：输入参数，描边宽度。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 圆形描边通常由三角形近似。

**范例代码：**

```c
xgeShapeCircleStroke(50.0f, 50.0f, 24.0f, 2.0f, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapeCircleStrokePx`
- `xgeShapeCircleFill`
- `xgeShapeArc`

---

### xgeShapeCircleStrokePx

绘制像素坐标圆描边。

**功能：**

你可以绘制屏幕空间圆形边框。

**函数原型：**

```c
XGE_API void xgeShapeCircleStrokePx(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，圆心像素坐标。
- `fRadius`：输入参数，半径。
- `fWidth`：输入参数，描边宽度。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 适合 UI 和 debug overlay。

**范例代码：**

```c
xgeShapeCircleStrokePx(50.0f, 50.0f, 24.0f, 1.0f, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapeCircleStroke`
- `xgeShapeArcPx`
- `xgeClipSet`

---

### xgeShapeArc

绘制逻辑坐标圆弧。

**功能：**

你可以绘制带宽度的圆弧，用于进度环、仪表盘或调试标记。

**函数原型：**

```c
XGE_API void xgeShapeArc(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，圆心逻辑坐标。
- `fRadius`：输入参数，半径。
- `fStartRadians` / `fEndRadians`：输入参数，起止角度，单位为弧度。
- `fWidth`：输入参数，弧线宽度。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 角度方向和细分由实现约定，复杂矢量图建议业务侧生成 mesh。

**范例代码：**

```c
xgeShapeArc(50.0f, 50.0f, 24.0f, 0.0f, 3.14f, 2.0f, xgeColorRGBA(0, 255, 0, 255));
```

**相关 API：**

- `xgeShapeArcPx`
- `xgeShapeCircleStroke`
- `xgeShapeLine`

---

### xgeShapeArcPx

绘制像素坐标圆弧。

**功能：**

你可以在屏幕空间绘制圆弧。

**函数原型：**

```c
XGE_API void xgeShapeArcPx(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
```

**参数：**

- `fX` / `fY`：输入参数，圆心像素坐标。
- `fRadius`：输入参数，半径。
- `fStartRadians` / `fEndRadians`：输入参数，起止角度，单位为弧度。
- `fWidth`：输入参数，弧线宽度。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 适合 UI 和 debug overlay。

**范例代码：**

```c
xgeShapeArcPx(50.0f, 50.0f, 24.0f, 0.0f, 3.14f, 2.0f, xgeColorRGBA(0, 255, 0, 255));
```

**相关 API：**

- `xgeShapeArc`
- `xgeShapeCircleStrokePx`
- `xgeClipSet`

---

### xgeShapeTriangleFill

绘制逻辑坐标填充三角形。

**功能：**

你可以提交一个填充三角形，作为自定义形状的基本单元。

**函数原型：**

```c
XGE_API void xgeShapeTriangleFill(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
```

**参数：**

- `tA` / `tB` / `tC`：输入参数，三个逻辑坐标顶点。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 顶点顺序可能影响后续扩展的背面剔除策略。

**范例代码：**

```c
xgeShapeTriangleFill(a, b, c, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapeTriangleFillPx`
- `xgeShapePolygonFill`
- `xgeShapeBatchTriangleFill`

---

### xgeShapeTriangleFillPx

绘制像素坐标填充三角形。

**功能：**

你可以在屏幕空间绘制一个填充三角形。

**函数原型：**

```c
XGE_API void xgeShapeTriangleFillPx(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
```

**参数：**

- `tA` / `tB` / `tC`：输入参数，三个像素坐标顶点。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

不持有外部资源。

**补充说明：**

- 适合 UI 和 debug overlay 的低层图元绘制。

**范例代码：**

```c
xgeShapeTriangleFillPx(a, b, c, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapeTriangleFill`
- `xgeShapePolygonFillPx`
- `xgeClipSet`

---

### xgeShapePolygonFill

绘制逻辑坐标填充多边形。

**功能：**

你可以提交一组点绘制填充多边形。

**函数原型：**

```c
XGE_API void xgeShapePolygonFill(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
```

**参数：**

- `pPoints`：输入参数，指向点数组，不能为 `NULL`，调用期间必须有效。
- `iCount`：输入参数，点数量。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

函数读取点数组，不保存 `pPoints` 指针。

**补充说明：**

- 当前多边形填充适合简单形状。复杂凹多边形建议业务侧三角化后使用 batch 或 mesh。

**范例代码：**

```c
xge_vec2_t pts[4] = {{0,0},{100,0},{100,100},{0,100}};
xgeShapePolygonFill(pts, 4, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapePolygonFillPx`
- `xgeShapeTriangleFill`
- `xgeMeshCreate`

---

### xgeShapePolygonFillPx

绘制像素坐标填充多边形。

**功能：**

你可以在屏幕空间绘制填充多边形。

**函数原型：**

```c
XGE_API void xgeShapePolygonFillPx(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
```

**参数：**

- `pPoints`：输入参数，指向点数组，不能为 `NULL`，调用期间必须有效。
- `iCount`：输入参数，点数量。
- `iColor`：输入参数，RGBA8 颜色。

**返回值：**

无。

**资源归属：**

函数读取点数组，不保存 `pPoints` 指针。

**补充说明：**

- 适合 UI 和 debug overlay 中的简单多边形。

**范例代码：**

```c
xgeShapePolygonFillPx(pts, 4, xgeColorRGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeShapePolygonFill`
- `xgeShapeTriangleFillPx`
- `xgeClipSet`

---

## Shape Batch

### xgeShapeBatchInit

初始化 shape batch。

**功能：**

你可以为大量同色 shape 三角形或矩形创建批处理缓存。

**函数原型：**

```c
XGE_API int xgeShapeBatchInit(xge_shape_batch pBatch, uint32_t iColor, int iTriangleCapacity, uint32_t iFlags);
```

**参数：**

- `pBatch`：输出参数，不能为 `NULL`。
- `iColor`：输入参数，RGBA8 颜色。
- `iTriangleCapacity`：输入参数，三角形容量，必须大于 `0`。
- `iFlags`：输入参数，绘制标志。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或内存分配失败时返回对应错误码。

**资源归属：**

batch 对象由调用者持有；内部缓存由 XGE 分配，必须调用 `xgeShapeBatchFree`。

**补充说明：**

- 一个 shape batch 适合一组同样颜色和状态的图形。

**范例代码：**

```c
xge_shape_batch_t batch;
xgeShapeBatchInit(&batch, xgeColorRGBA(255, 255, 255, 255), 1024, 0);
```

**相关 API：**

- `xgeShapeBatchTriangleFill`
- `xgeShapeBatchRectFill`
- `xgeShapeBatchFree`

---

### xgeShapeBatchFree

释放 shape batch。

**功能：**

你可以释放 shape batch 内部缓存。

**函数原型：**

```c
XGE_API void xgeShapeBatchFree(xge_shape_batch pBatch);
```

**参数：**

- `pBatch`：输入/输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 batch 内部缓存并清空对象。

**补充说明：**

- 不影响已经提交到渲染命令队列中的命令。

**范例代码：**

```c
xgeShapeBatchFree(&batch);
```

**相关 API：**

- `xgeShapeBatchInit`
- `xgeShapeBatchClear`
- `xgeShapeBatchFlush`

---

### xgeShapeBatchClear

清空 shape batch。

**功能：**

你可以复用 shape batch 内部缓存，清除已添加图元。

**函数原型：**

```c
XGE_API void xgeShapeBatchClear(xge_shape_batch pBatch);
```

**参数：**

- `pBatch`：输入/输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放缓存，只重置计数。

**补充说明：**

- 适合每帧重用 batch。

**范例代码：**

```c
xgeShapeBatchClear(&batch);
```

**相关 API：**

- `xgeShapeBatchTriangleFill`
- `xgeShapeBatchRectFill`
- `xgeShapeBatchFlush`

---

### xgeShapeBatchTriangleFill

向 shape batch 添加三角形。

**功能：**

你可以把一个填充三角形加入 shape batch。

**函数原型：**

```c
XGE_API int xgeShapeBatchTriangleFill(xge_shape_batch pBatch, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC);
```

**参数：**

- `pBatch`：输入/输出参数，不能为 `NULL`。
- `tA` / `tB` / `tC`：输入参数，三个顶点。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或 batch 容量不足时返回对应错误码。

**资源归属：**

函数把顶点数据复制到 batch 内部缓存。

**补充说明：**

- 颜色来自 batch 初始化时的 `iColor`。

**范例代码：**

```c
xgeShapeBatchTriangleFill(&batch, a, b, c);
```

**相关 API：**

- `xgeShapeBatchInit`
- `xgeShapeBatchRectFill`
- `xgeShapeBatchFlush`

---

### xgeShapeBatchRectFill

向 shape batch 添加填充矩形。

**功能：**

你可以把一个矩形作为两个三角形加入 shape batch。

**函数原型：**

```c
XGE_API int xgeShapeBatchRectFill(xge_shape_batch pBatch, xge_rect_t tRect);
```

**参数：**

- `pBatch`：输入/输出参数，不能为 `NULL`。
- `tRect`：输入参数，矩形。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或 batch 容量不足时返回对应错误码。

**资源归属：**

函数把矩形转换出的顶点复制到 batch 内部缓存。

**补充说明：**

- 颜色来自 batch 初始化时的 `iColor`。

**范例代码：**

```c
xgeShapeBatchRectFill(&batch, (xge_rect_t){ 0, 0, 100, 50 });
```

**相关 API：**

- `xgeShapeBatchTriangleFill`
- `xgeShapeBatchFlush`
- `xgeShapeRectFill`

---

### xgeShapeBatchFlush

提交 shape batch。

**功能：**

你可以把 batch 中积累的 shape 图元提交到渲染命令队列。

**函数原型：**

```c
XGE_API int xgeShapeBatchFlush(xge_shape_batch pBatch);
```

**参数：**

- `pBatch`：输入/输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或提交失败时返回对应错误码。

**资源归属：**

不释放 batch。提交后可调用 `xgeShapeBatchClear` 复用。

**补充说明：**

- 大量简单图形应优先 batch，而不是逐个 immediate 提交。

**范例代码：**

```c
xgeShapeBatchFlush(&batch);
```

**相关 API：**

- `xgeShapeBatchClear`
- `xgeFlush`
- `xgeShapeBatchFree`

---

## 生命周期与所有权

Drawing API 本身不持有业务资源所有权。`xgeDraw`、`xgeDrawEx`、`xgeDrawQuad3D` 等函数引用的 texture 和 draw 参数必须在提交期间有效。

`xgeSpriteBatchInit`、`xgeShapeBatchInit` 创建的 batch 由调用者持有，退出时分别调用 `xgeSpriteBatchFree`、`xgeShapeBatchFree`。

## 线程约束

普通绘制函数应在 frame/render 提交阶段调用。多线程业务可以构建自己的绘制数据，但最终提交应进入 XGE 的渲染命令路径。

启用专用 render thread 后，绘制 API 仍是提交命令，不允许调用者绕过 XGE 直接跨线程调用 GL。

## 后端差异

Shape 的线宽、弧线细分和多边形填充在不同 GL/GLES/WebGL 后端可能有精度差异。复杂描边或凹多边形建议业务侧转 mesh。

2.5D API 依赖 depth、camera 和 shader 能力。WebGL2/GLES3 可覆盖主要路径，但 shader 语法仍需遵守目标后端限制。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 大量 sprite 性能差 | 每个 sprite 独立提交 | 使用 `xgeSpriteBatch*`，并按纹理排序。 |
| 形状边缘透明异常 | blend 与 premultiplied alpha 状态不匹配 | 使用默认 alpha blend 和 RGBA8 预乘路径。 |
| 2.5D quad 不显示 | camera/depth/顶点 Z 设置不匹配 | 检查 `xgeCameraPerspective`、`xgeDepthTestSet` 和顶点顺序。 |

## 相关示例

- `examples/shape`
- `examples/texture`
- `examples/sprite_batch`
- `examples/perspective_quad`
