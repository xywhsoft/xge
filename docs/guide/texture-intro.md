# 资源与纹理入门

> 状态：中文初稿已生成，待审阅。

纹理是 XGE 里最常用的图形资源。图片文件先被解码成 CPU 侧 image，再上传成 GPU 侧 texture。你可以手动分两步做，也可以直接用 `xgeTextureLoad` 一步完成。

## 你要先理解的 2 个对象

| 对象 | 位置 | 适合做什么 |
| --- | --- | --- |
| `xge_image_t` | CPU 内存 | 解码、读取像素、预乘 alpha、生成纹理前处理。 |
| `xge_texture_t` | GPU | 绘制、采样、作为 render target 纹理。 |

大多数游戏代码只需要直接加载 texture。

## 直接加载纹理

```c
xge_texture_t tex;
memset(&tex, 0, sizeof(tex));

if ( xgeTextureLoad(&tex, "assets/player.png") != XGE_OK ) {
	return 1;
}

/* frame 中使用 */
xgeDraw(&tex, 100.0f, 120.0f);

/* 退出时释放 */
xgeTextureFree(&tex);
```

调用顺序：

```text
xgeTextureLoad
  -> xgeDraw / xgeDrawEx
  -> xgeTextureFree
```

## 先加载 image 再创建 texture

如果你需要读取像素、修改像素或控制 premultiplied alpha，可以先加载 image。

```c
xge_image_t image;
xge_texture_t tex;
memset(&image, 0, sizeof(image));
memset(&tex, 0, sizeof(tex));

if ( xgeImageLoadEx(&image, "assets/player.png", XGE_IMAGE_PREMULTIPLIED) == XGE_OK ) {
	xgeTextureCreateFromImage(&tex, &image);
	xgeImageFree(&image);
}
```

Image 和 Texture 的生命周期是独立的。创建纹理后，image 可以释放。

## Sampler

Sampler 控制纹理缩放和重复方式。

```c
xge_sampler_t sampler = xgeSamplerDefault();
sampler.iMinFilter = XGE_FILTER_NEAREST;
sampler.iMagFilter = XGE_FILTER_NEAREST;
sampler.iWrapS = XGE_WRAP_CLAMP;
sampler.iWrapT = XGE_WRAP_CLAMP;
xgeTextureSetSampler(&tex, &sampler);
```

像素风游戏通常使用 nearest。普通 UI 和图片通常使用 linear。

## Fallback 纹理

异步资源和微端场景下，真实纹理可能暂时不可用。可以设置 fallback 纹理作为占位。

```c
uint32_t magenta = xgeColorRGBA(255, 0, 255, 255);
xgeTextureFallbackSetRGBA(1, 1, &magenta);
xgeTextureFallbackGet(&tex);
```

真实资源加载完成后，再用真实纹理替换。

## Alpha 约定

XGE 默认按 premultiplied alpha 设计混合路径。建议：

- UI、sprite、字体贴图默认使用 premultiplied alpha。
- 从外部图片加载时明确使用 `XGE_IMAGE_PREMULTIPLIED`。
- 避免一部分资源预乘、一部分资源 straight alpha 但共用同一个 blend 模式。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 图片边缘有黑边 | straight alpha 和 premultiplied blend 混用 | 加载时预乘 alpha。 |
| 纹理释放后仍被绘制 | 对象生命周期早于绘制命令 | 确保 texture 活到最后一次绘制之后。 |
| 后台线程上传失败 | GL context 不在后台线程 | 后台只解码，上传交给 render thread 或上传队列。 |

## 下一步

- 继续看 [Shape、Sprite 与文本绘制入门](drawing-intro.md)。
- 查函数细节看 [Texture API](../api/texture.md)。

[返回教程入口](README.md)
