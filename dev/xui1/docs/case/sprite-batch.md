# Sprite Batch

本案例展示如何批量绘制大量同纹理 sprite。

[返回范例解析](README.md) | [绘制教程](../guide/drawing-intro.md) | [Drawing API](../api/drawing.md)

## 问题

大量 sprite 如果逐个切换状态和提交，会增加 draw call 和 CPU 开销。Sprite batch 用同一张纹理合并多个绘制命令。

## 初始化

```c
xge_sprite_batch_t batch;

if ( xgeSpriteBatchInit(&batch, &atlas, 2048, 0) != XGE_OK ) {
	return;
}
```

## 添加绘制

```c
int i;

for ( i = 0; i < sprite_count; ++i ) {
	xge_draw_t draw;

	memset(&draw, 0, sizeof(draw));
	draw.pTexture = &atlas;
	draw.tSrc = sprites[i].tSrc;
	draw.tDst = sprites[i].tDst;
	draw.iColor = xgeColorRGBA(255, 255, 255, 255);
	xgeSpriteBatchAdd(&batch, &draw);
}
```

## 提交

```c
xgeSpriteBatchFlush(&batch);
xgeSpriteBatchClear(&batch);
```

退出时释放：

```c
xgeSpriteBatchFree(&batch);
```

## 优化建议

按纹理排序，尽量让同一 atlas 的 sprite 连续提交。

UI、小地图、粒子和 tilemap 都适合 batch。

不同 blend、shader、RenderTarget 的对象不要强行放进同一个 batch。

## 常见失败原因

容量不足：提高 `iCapacity`，或分批 flush。

贴图错乱：检查 atlas 源矩形和 UV 是否以同一坐标约定生成。

没有性能提升：可能瓶颈在纹理切换、shader 切换或填充率，而不是 draw call。
