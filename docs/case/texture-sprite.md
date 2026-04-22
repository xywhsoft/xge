# 纹理和 Sprite 绘制

本案例展示从文件加载图片，创建纹理，并在帧回调中绘制 sprite。

[返回范例解析](README.md) | [纹理教程](../guide/texture-intro.md) | [Texture API](../api/texture.md) | [Drawing API](../api/drawing.md)

## 问题

你希望把一张图片绘制到窗口中，并能控制位置、缩放、旋转、颜色和采样方式。

这个案例适合：

- 验证图片解码和 GPU 上传链路。
- 学习 `xgeTextureLoad` 与 `xgeDrawEx` 的关系。
- 给 sprite batch、材质和 XUI image 控件打基础。

## 数据流

```text
assets/player.png
  -> xgeTextureLoad
    -> CPU image decode
    -> GPU texture
  -> xgeDraw / xgeDrawEx
    -> batch command
    -> frame flush
```

## 步骤 1：准备纹理对象

纹理对象由调用方持有，XGE 负责内部 GPU 资源和引用计数。

```c
static xge_texture_t g_player;

static int LoadAssets(void)
{
	xge_sampler_t sampler;

	if ( xgeTextureLoad(&g_player, "assets/player.png") != XGE_OK ) {
		return XGE_ERROR_IO;
	}

	sampler = xgeSamplerDefault();
	sampler.iMinFilter = XGE_FILTER_NEAREST;
	sampler.iMagFilter = XGE_FILTER_NEAREST;
	xgeTextureSetSampler(&g_player, &sampler);
	return XGE_OK;
}
```

像素风格图片通常使用 nearest，普通 UI 或插画可以使用 linear。采样配置挂在纹理上，绘制时会被批处理系统读取。

## 步骤 2：绘制普通 sprite

最简单的绘制只需要纹理和左上角坐标。

```c
static int MainFrame(void* pUser)
{
	(void)pUser;

	xgeClear(xgeColorRGBA(24, 28, 36, 255));
	xgeDraw(&g_player, 120.0f, 96.0f);
	return 0;
}
```

## 步骤 3：绘制带变换的 sprite

需要缩放、旋转、裁剪或调色时使用 `xgeDrawEx`。

```c
static void DrawPlayer(float fX, float fY, float fAngle)
{
	xge_draw_t draw;

	memset(&draw, 0, sizeof(draw));
	draw.pTexture = &g_player;
	draw.tDst.fX = fX;
	draw.tDst.fY = fY;
	draw.tDst.fW = 96.0f;
	draw.tDst.fH = 96.0f;
	draw.tOrigin.fX = 48.0f;
	draw.tOrigin.fY = 48.0f;
	draw.fRotation = fAngle;
	draw.iColor = xgeColorRGBA(255, 255, 255, 255);
	xgeDrawEx(&draw);
}
```

默认颜色应保持白色不透明。需要闪白、受伤、变暗等效果时再改变 `iColor`。

## 步骤 4：释放资源

```c
static void FreeAssets(void)
{
	xgeTextureFree(&g_player);
}
```

对象可以手动释放，也可以由更高层的资源管理器统一释放。XGE 第一版不内建全局资源缓存。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeTextureLoad` | 从路径加载图片并创建纹理 |
| `xgeTextureLoadMemory` | 从内存加载图片并创建纹理 |
| `xgeSamplerDefault` | 获取默认采样配置 |
| `xgeTextureSetSampler` | 设置纹理采样方式 |
| `xgeDraw` | 以原图尺寸绘制 |
| `xgeDrawEx` | 提交完整 sprite 绘制参数 |
| `xgeTextureFree` | 释放纹理资源 |

## 常见问题

如果图片透明边缘发黑，确认资源和绘制路径使用 premultiplied alpha，并避免对同一像素重复预乘。

如果纹理加载成功但没有显示，检查 `draw.tDst.fW`、`draw.tDst.fH` 是否为 0，以及当前 clip、viewport 是否裁掉了目标区域。

如果异步加载时先显示 fallback，参考 [异步加载范例](async-loading.md)。
