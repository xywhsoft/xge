# Texture API

> Texture API 负责图片解码、CPU 像素访问、GPU 纹理创建、采样参数、fallback、上传队列和像素读回。

[返回 API 索引](README.md) | [纹理教程](../guide/texture-intro.md) | [Texture / Sprite 范例](../case/texture-sprite.md)

---

## 模块定位

Image 是 CPU 侧像素对象，Texture 是 GPU 侧纹理对象。常见流程是先从文件或内存加载 `xge_image`，必要时做 premultiplied alpha，再创建或更新 `xge_texture`。如果只需要直接显示图片，可以使用 `xgeTextureLoad*` 一步完成。

Texture API 不负责绘制，绘制请使用 [Drawing API](drawing.md)。资源路径协议和压缩包接入请看 [Resource API](resource.md)。

## 标准调用顺序

```text
xgeImageLoad / xgeTextureLoad
  -> 可选：xgeImagePremultiply / xgeTextureSetSampler
  -> xgeDraw / xgeDrawEx
  -> xgeTextureFree / xgeImageFree
```

异步加载时，后台线程负责文件和解码，GPU 上传应通过上传队列或 render thread 执行。

## 常量与宏

| 名称 | 说明 |
| --- | --- |
| `XGE_PIXEL_RGBA8` | RGBA8 像素格式。 |
| `XGE_PIXEL_A8` | 单通道 alpha 格式，常用于字形。 |
| `XGE_IMAGE_PREMULTIPLIED` | 图片已预乘 alpha。 |
| `XGE_IMAGE_STRAIGHT_ALPHA` | 图片保留 straight alpha。 |
| `XGE_TEXTURE_FALLBACK` | 纹理为 fallback 结果。 |
| `XGE_TEXTURE_UPLOAD_QUEUED` | 纹理存在待上传数据。 |
| `XGE_FILTER_NEAREST` / `XGE_FILTER_LINEAR` | 最近点 / 线性采样。 |
| `XGE_WRAP_CLAMP` / `XGE_WRAP_REPEAT` | clamp / repeat wrap。 |

## 公共类型

### `xge_image_t`

CPU 侧像素对象。结构体透明，调用者可读取宽高、格式和像素指针。加载成功后，`pPixels` 由 XGE 分配，必须用 `xgeImageFree` 释放。

### `xge_texture_t`

GPU 侧纹理对象。结构体透明，但 `iBackendId`、`pBackend` 属于后端私有状态，用户不应直接修改。纹理对象采用引用计数。

### `xge_sampler_t`

采样参数结构，包含 `iMinFilter`、`iMagFilter`、`iWrapS`、`iWrapT`。

## API 列表

- `xgeImageLoad`
- `xgeImageLoadEx`
- `xgeImageLoadMemory`
- `xgeImageLoadMemoryEx`
- `xgeImageGetPixels`
- `xgeImagePremultiply`
- `xgeImageFree`
- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`
- `xgeTextureAddRef`
- `xgeTextureUpdateRGBA`
- `xgeSamplerDefault`
- `xgeTextureSetSampler`
- `xgeTextureGetSampler`
- `xgeTextureFallbackSetRGBA`
- `xgeTextureFallbackGet`
- `xgeTextureFallbackClear`
- `xgeTextureUploadQueue`
- `xgeTextureUploadFlush`
- `xgeTextureReadPixels`
- `xgeTextureFree`

## Image

### xgeImageLoad

从文件加载图片。

**功能：**

你可以从普通路径或 `res://` 资源路径加载图片到 CPU 像素对象。默认使用 premultiplied alpha 路径。

**函数原型：**

```c
XGE_API int xgeImageLoad(xge_image pImage, const char* sPath);
```

**参数：**

- `pImage`：输出参数，不能为 `NULL`。函数会写入 CPU 图片对象。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 资源加载或图片解码失败时返回对应错误码。

**资源归属：**

加载成功后，`pImage->pPixels` 由 XGE 持有，调用者必须调用 `xgeImageFree` 释放。

**补充说明：**

- 该函数等价于使用 `XGE_IMAGE_PREMULTIPLIED` 调用 `xgeImageLoadEx`。
- 文件读取走 Resource API，因此可受资源 provider 影响。

**范例代码：**

```c
xge_image_t image;
if (xgeImageLoad(&image, "assets/player.png") == XGE_OK) {
	xgeImageFree(&image);
}
```

**相关 API：**

- `xgeImageLoadEx`
- `xgeImageFree`
- `xgeResourceLoad`

---

### xgeImageLoadEx

从文件加载图片并指定加载标志。

**功能：**

你可以控制图片加载后是否预乘 alpha，适配默认 premultiplied blend 或需要保留 straight alpha 的特殊管线。

**函数原型：**

```c
XGE_API int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags);
```

**参数：**

- `pImage`：输出参数，不能为 `NULL`。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。
- `iFlags`：输入参数，`XGE_IMAGE_*` 标志。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 资源加载或图片解码失败时返回对应错误码。

**资源归属：**

加载成功后，调用者必须调用 `xgeImageFree`。

**补充说明：**

- 当前实现先用 `xgeResourceLoad` 读取资源，再调用 `xgeImageLoadMemoryEx` 解码。
- 如果未设置 `XGE_IMAGE_STRAIGHT_ALPHA`，结果默认按 premultiplied alpha 处理。

**范例代码：**

```c
xge_image_t image;
xgeImageLoadEx(&image, "assets/ui.png", XGE_IMAGE_STRAIGHT_ALPHA);
xgeImageFree(&image);
```

**相关 API：**

- `xgeImageLoad`
- `xgeImageLoadMemoryEx`
- `xgeImagePremultiply`

---

### xgeImageLoadMemory

从内存图片数据加载图片。

**功能：**

你可以从已经读取到内存的 PNG/JPEG 等图片数据解码出 CPU RGBA8 像素。

**函数原型：**

```c
XGE_API int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize);
```

**参数：**

- `pImage`：输出参数，不能为 `NULL`。
- `pData`：输入参数，图片文件数据，不能为 `NULL`。
- `iSize`：输入参数，数据字节数，必须大于 `0`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 解码失败时返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

加载成功后，调用者必须调用 `xgeImageFree`。

**补充说明：**

- 该函数等价于使用 `XGE_IMAGE_PREMULTIPLIED` 调用 `xgeImageLoadMemoryEx`。

**范例代码：**

```c
xge_image_t image;
xgeImageLoadMemory(&image, data, size);
xgeImageFree(&image);
```

**相关 API：**

- `xgeImageLoadMemoryEx`
- `xgeImageFree`
- `xgeTextureLoadMemory`

---

### xgeImageLoadMemoryEx

从内存图片数据加载图片并指定加载标志。

**功能：**

你可以从内存图片数据解码 CPU RGBA8 像素，并控制 alpha 处理策略。

**函数原型：**

```c
XGE_API int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags);
```

**参数：**

- `pImage`：输出参数，不能为 `NULL`。
- `pData`：输入参数，图片文件数据，不能为 `NULL`。
- `iSize`：输入参数，数据字节数，必须大于 `0`。
- `iFlags`：输入参数，`XGE_IMAGE_*` 标志。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 解码失败时返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

加载成功后，`pImage->pPixels` 由 XGE 分配，调用者必须调用 `xgeImageFree`。

**补充说明：**

- 当前实现使用 stb_image 解码，并强制输出 RGBA8。
- 未指定 `XGE_IMAGE_STRAIGHT_ALPHA` 时会自动执行 `xgeImagePremultiply`。

**范例代码：**

```c
xge_image_t image;
if (xgeImageLoadMemoryEx(&image, data, size, XGE_IMAGE_PREMULTIPLIED) == XGE_OK) {
	/* use image */
	xgeImageFree(&image);
}
```

**相关 API：**

- `xgeImagePremultiply`
- `xgeTextureCreateFromImage`
- `xgeImageFree`

---

### xgeImageGetPixels

获取图片 CPU 像素指针。

**功能：**

你可以读取 image 内部像素，用于工具、调试、CPU 侧处理或创建纹理。

**函数原型：**

```c
XGE_API void* xgeImageGetPixels(xge_image pImage);
```

**参数：**

- `pImage`：输入参数，可以为 `NULL`。

**返回值：**

- 返回 CPU 像素指针。
- `pImage == NULL` 时返回 `NULL`。

**资源归属：**

返回值是借用指针，调用者不能直接释放。释放 image 时使用 `xgeImageFree`。

**补充说明：**

- 指针在 `xgeImageFree` 后失效。

**范例代码：**

```c
void* pixels = xgeImageGetPixels(&image);
```

**相关 API：**

- `xgeImageLoad`
- `xgeImageFree`
- `xgeTextureCreateFromImage`

---

### xgeImagePremultiply

把图片原地转换为 premultiplied alpha。

**功能：**

你可以把 RGBA8 straight alpha 像素转换为适配 XGE 默认 blend 路径的预乘 alpha 像素。

**函数原型：**

```c
XGE_API void xgeImagePremultiply(xge_image pImage);
```

**参数：**

- `pImage`：输入/输出参数，可以为 `NULL`。必须是已加载的 RGBA8 image 才会执行转换。

**返回值：**

无。

**资源归属：**

不改变 image 所有权。函数原地修改 `pImage->pPixels`。

**补充说明：**

- 非 RGBA8、空像素或 `NULL` image 会被忽略。
- 转换后会清除 `XGE_IMAGE_STRAIGHT_ALPHA` 并设置 `XGE_IMAGE_PREMULTIPLIED`。

**范例代码：**

```c
xgeImagePremultiply(&image);
```

**相关 API：**

- `xgeImageLoadEx`
- `xgeBlendSet`
- `xgeTextureCreateFromImage`

---

### xgeImageFree

释放图片 CPU 像素。

**功能：**

你可以释放由 `xgeImageLoad*` 分配的像素内存，并清空 image 结构。

**函数原型：**

```c
XGE_API void xgeImageFree(xge_image pImage);
```

**参数：**

- `pImage`：输入/输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pImage->pPixels` 指向的像素内存，并把结构体清零。

**补充说明：**

- 只能释放 XGE image 加载函数创建的像素。
- `NULL` 参数会被忽略。

**范例代码：**

```c
xgeImageFree(&image);
```

**相关 API：**

- `xgeImageLoad`
- `xgeImageLoadMemory`
- `xgeMemoryFree`

---

## Texture

### xgeTextureCreateRGBA

从 RGBA8 像素创建纹理对象。

**功能：**

你可以从调用者提供的 RGBA8 像素创建 `xge_texture_t`，用于绘制、fallback 或动态纹理。

**函数原型：**

```c
XGE_API int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels);
```

**参数：**

- `pTexture`：输出参数，不能为 `NULL`。
- `iWidth`：输入参数，纹理宽度，必须大于 `0`。
- `iHeight`：输入参数，纹理高度，必须大于 `0`。
- `pPixels`：输入参数，RGBA8 像素指针，可以按实现路径允许为 `NULL`，但有像素数据时必须至少包含 `iWidth * iHeight * 4` 字节。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存或 GPU 上传失败时返回对应错误码。

**资源归属：**

成功后纹理对象归调用者管理，初始引用计数为 `1`，最终调用 `xgeTextureFree`。

**补充说明：**

- 当前实现会保留 CPU shadow，用于延迟上传、readback 和 fallback。
- Sokol/GL 正在运行时会尝试立即上传。

**范例代码：**

```c
xge_texture_t tex;
xgeTextureCreateRGBA(&tex, 2, 2, pixels);
xgeTextureFree(&tex);
```

**相关 API：**

- `xgeTextureFree`
- `xgeTextureUpdateRGBA`
- `xgeTextureUploadQueue`

---

### xgeTextureCreateFromImage

从 image 创建纹理。

**功能：**

你可以把已经解码的 CPU image 转为 GPU 纹理。

**函数原型：**

```c
XGE_API int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage);
```

**参数：**

- `pTexture`：输出参数，不能为 `NULL`。
- `pImage`：输入参数，不能为 `NULL`，且 `pImage->pPixels` 必须有效。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 创建纹理失败时返回对应错误码。

**资源归属：**

函数不接管 `pImage`。纹理创建成功后由调用者调用 `xgeTextureFree`；image 仍由调用者调用 `xgeImageFree`。

**补充说明：**

- 当前实现调用 `xgeTextureCreateRGBA`，使用 image 的宽高和像素。

**范例代码：**

```c
xge_image_t image;
xge_texture_t tex;
xgeImageLoad(&image, "assets/player.png");
xgeTextureCreateFromImage(&tex, &image);
xgeTextureFree(&tex);
xgeImageFree(&image);
```

**相关 API：**

- `xgeImageLoad`
- `xgeTextureCreateRGBA`
- `xgeTextureFree`

---

### xgeTextureLoad

从文件加载并创建纹理。

**功能：**

你可以一步完成文件读取、图片解码、alpha 预乘和纹理创建。

**函数原型：**

```c
XGE_API int xgeTextureLoad(xge_texture pTexture, const char* sPath);
```

**参数：**

- `pTexture`：输出参数，不能为 `NULL`。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 失败时如果 fallback 纹理可用，可能返回 fallback 创建结果。
- 参数、资源、解码或 GPU 失败时返回对应错误码。

**资源归属：**

成功后纹理归调用者管理，最终调用 `xgeTextureFree`。

**补充说明：**

- 该函数等价于使用 `XGE_IMAGE_PREMULTIPLIED` 调用 `xgeTextureLoadEx`。

**范例代码：**

```c
xge_texture_t tex;
if (xgeTextureLoad(&tex, "assets/player.png") == XGE_OK) {
	xgeDraw(&tex, 100.0f, 120.0f);
	xgeTextureFree(&tex);
}
```

**相关 API：**

- `xgeTextureLoadEx`
- `xgeTextureFallbackSetRGBA`
- `xgeDraw`

---

### xgeTextureLoadEx

从文件加载纹理并指定图片标志。

**功能：**

你可以控制文件图片加载时的 alpha 策略，并创建纹理。

**函数原型：**

```c
XGE_API int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags);
```

**参数：**

- `pTexture`：输出参数，不能为 `NULL`。
- `sPath`：输入参数，UTF-8 路径，不能为 `NULL`。
- `iFlags`：输入参数，`XGE_IMAGE_*` 标志。

**返回值：**

- 成功返回 `XGE_OK`。
- 失败时如果 fallback 纹理可用，可能返回 fallback 创建结果。
- 参数、资源、解码或 GPU 失败时返回对应错误码。

**资源归属：**

成功后纹理由调用者管理，最终调用 `xgeTextureFree`。

**补充说明：**

- 函数内部会临时创建 image，纹理创建后释放 image。

**范例代码：**

```c
xgeTextureLoadEx(&tex, "assets/raw-alpha.png", XGE_IMAGE_STRAIGHT_ALPHA);
```

**相关 API：**

- `xgeImageLoadEx`
- `xgeTextureCreateFromImage`
- `xgeTextureFallbackGet`

---

### xgeTextureLoadMemory

从内存图片数据创建纹理。

**功能：**

你可以把内存中的 PNG/JPEG 等图片数据直接解码并创建纹理。

**函数原型：**

```c
XGE_API int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize);
```

**参数：**

- `pTexture`：输出参数，不能为 `NULL`。
- `pData`：输入参数，图片文件数据，不能为 `NULL`。
- `iSize`：输入参数，数据字节数，必须大于 `0`。

**返回值：**

- 成功返回 `XGE_OK`。
- 失败时如果 fallback 纹理可用，可能返回 fallback 创建结果。
- 参数或解码失败时返回对应错误码。

**资源归属：**

成功后纹理由调用者管理，最终调用 `xgeTextureFree`。

**补充说明：**

- 该函数默认使用 premultiplied alpha。

**范例代码：**

```c
xgeTextureLoadMemory(&tex, data, size);
```

**相关 API：**

- `xgeTextureLoadMemoryEx`
- `xgeImageLoadMemory`
- `xgeTextureFree`

---

### xgeTextureLoadMemoryEx

从内存图片数据创建纹理并指定图片标志。

**功能：**

你可以控制内存图片数据解码后的 alpha 策略，并创建纹理。

**函数原型：**

```c
XGE_API int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags);
```

**参数：**

- `pTexture`：输出参数，不能为 `NULL`。
- `pData`：输入参数，图片文件数据，不能为 `NULL`。
- `iSize`：输入参数，数据字节数，必须大于 `0`。
- `iFlags`：输入参数，`XGE_IMAGE_*` 标志。

**返回值：**

- 成功返回 `XGE_OK`。
- 失败时如果 fallback 纹理可用，可能返回 fallback 创建结果。
- 参数、解码或 GPU 失败时返回对应错误码。

**资源归属：**

成功后纹理由调用者管理，最终调用 `xgeTextureFree`。

**补充说明：**

- 函数内部会临时创建 image，纹理创建后释放 image。

**范例代码：**

```c
xgeTextureLoadMemoryEx(&tex, data, size, XGE_IMAGE_PREMULTIPLIED);
```

**相关 API：**

- `xgeImageLoadMemoryEx`
- `xgeTextureCreateFromImage`
- `xgeTextureFallbackSetRGBA`

---

### xgeTextureAddRef

增加纹理引用计数。

**功能：**

你可以让多个系统共享同一纹理对象，并通过引用计数管理生命周期。

**函数原型：**

```c
XGE_API int xgeTextureAddRef(xge_texture pTexture);
```

**参数：**

- `pTexture`：输入/输出参数，不能为 `NULL`。

**返回值：**

- 成功返回新的引用计数。
- `pTexture == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用成功后调用者持有一份引用，之后必须对应调用 `xgeTextureFree`。

**补充说明：**

- 引用计数达到 `INT32_MAX` 时不再增加，返回当前引用计数。

**范例代码：**

```c
if (xgeTextureAddRef(&tex) > 0) {
	xgeTextureFree(&tex);
}
```

**相关 API：**

- `xgeTextureFree`
- `xgeTextureLoad`
- `xgeDebugGetStats`

---

### xgeTextureUpdateRGBA

更新纹理局部 RGBA8 像素。

**功能：**

你可以更新动态纹理的一块区域，例如视频帧、软件绘制缓存或动态图集。

**函数原型：**

```c
XGE_API int xgeTextureUpdateRGBA(xge_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride);
```

**参数：**

- `pTexture`：输入/输出参数，不能为 `NULL`，必须是有效 RGBA8 纹理。
- `iX` / `iY`：输入参数，更新区域左上角，不能小于 `0`。
- `iWidth` / `iHeight`：输入参数，更新区域尺寸，必须大于 `0`。
- `pPixels`：输入参数，RGBA8 像素，不能为 `NULL`。
- `iStride`：输入参数，每行字节数。小于等于 `0` 时按 `iWidth * 4` 处理。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法、区域越界或 stride 不足时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存或 GPU 更新失败时返回对应错误码。

**资源归属：**

XGE 会复制像素到纹理 shadow 或 GPU，不保存 `pPixels` 指针。

**补充说明：**

- 该函数可能触发 GPU 上传，频繁大面积更新有性能成本。

**范例代码：**

```c
xgeTextureUpdateRGBA(&tex, 0, 0, w, h, pixels, w * 4);
```

**相关 API：**

- `xgeTextureCreateRGBA`
- `xgeTextureUploadQueue`
- `xgeTextureReadPixels`

---

## Sampler 与 Fallback

### xgeSamplerDefault

获取默认采样参数。

**功能：**

你可以快速创建默认 sampler，再按需要修改过滤或 wrap。

**函数原型：**

```c
XGE_API xge_sampler_t xgeSamplerDefault(void);
```

**参数：**

无。

**返回值：**

- 返回 `xge_sampler_t` 结构体副本。
- 当前默认值为 linear filter 与 clamp wrap。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 纹理创建时会默认使用该 sampler。

**范例代码：**

```c
xge_sampler_t sampler = xgeSamplerDefault();
sampler.iMinFilter = XGE_FILTER_NEAREST;
```

**相关 API：**

- `xgeTextureSetSampler`
- `xgeTextureGetSampler`
- `xgeTextureCreateRGBA`

---

### xgeTextureSetSampler

设置纹理采样参数。

**功能：**

你可以控制纹理放大缩小过滤和 U/V wrap 模式。

**函数原型：**

```c
XGE_API int xgeTextureSetSampler(xge_texture pTexture, const xge_sampler_t* pSampler);
```

**参数：**

- `pTexture`：输入/输出参数，不能为 `NULL`，必须是有效纹理。
- `pSampler`：输入参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端应用 sampler 失败时返回对应错误码。

**资源归属：**

XGE 会复制 sampler 内容，不保存 `pSampler` 指针。

**补充说明：**

- sampler 中为 `0` 的字段会被规范化为默认值。
- 纹理已上传且 GL 可用时，会立即应用到后端纹理。

**范例代码：**

```c
xge_sampler_t sampler = xgeSamplerDefault();
sampler.iMagFilter = XGE_FILTER_NEAREST;
xgeTextureSetSampler(&tex, &sampler);
```

**相关 API：**

- `xgeSamplerDefault`
- `xgeTextureGetSampler`
- `xgeTextureCreateRGBA`

---

### xgeTextureGetSampler

获取纹理采样参数。

**功能：**

你可以读取当前纹理 sampler，用于临时修改后恢复。

**函数原型：**

```c
XGE_API xge_sampler_t xgeTextureGetSampler(xge_texture pTexture);
```

**参数：**

- `pTexture`：输入参数，可以为 `NULL`。

**返回值：**

- `pTexture` 有效时返回纹理 sampler 副本。
- `pTexture == NULL` 时返回默认 sampler。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

- 该函数不检查后端纹理状态。

**范例代码：**

```c
xge_sampler_t sampler = xgeTextureGetSampler(&tex);
```

**相关 API：**

- `xgeTextureSetSampler`
- `xgeSamplerDefault`
- `xgeTextureLoad`

---

### xgeTextureFallbackSetRGBA

设置全局 fallback 纹理。

**功能：**

你可以设置资源缺失或异步加载未完成时使用的占位纹理。

**函数原型：**

```c
XGE_API int xgeTextureFallbackSetRGBA(int iWidth, int iHeight, const void* pPixels);
```

**参数：**

- `iWidth`：输入参数，宽度，必须大于 `0`。
- `iHeight`：输入参数，高度，必须大于 `0`。
- `pPixels`：输入参数，RGBA8 像素，可以按 `xgeTextureCreateRGBA` 规则传入。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 创建 fallback 失败时返回对应错误码。

**资源归属：**

fallback 纹理由 XGE runtime 持有。调用 `xgeTextureFallbackClear` 清除。

**补充说明：**

- 设置新 fallback 前会先清除旧 fallback。

**范例代码：**

```c
xgeTextureFallbackSetRGBA(2, 2, pixels);
```

**相关 API：**

- `xgeTextureFallbackGet`
- `xgeTextureFallbackClear`
- `xgeTextureLoad`

---

### xgeTextureFallbackGet

把全局 fallback 写入目标纹理。

**功能：**

你可以在加载失败时创建一个 fallback 纹理对象，让绘制流程继续工作。

**函数原型：**

```c
XGE_API int xgeTextureFallbackGet(xge_texture pTexture);
```

**参数：**

- `pTexture`：输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- `pTexture == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- fallback 未设置或不可用时返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

成功后目标纹理由调用者管理，最终调用 `xgeTextureFree`。

**补充说明：**

- 成功后目标纹理会设置 `XGE_TEXTURE_FALLBACK` 标志。

**范例代码：**

```c
xge_texture_t tex;
if (xgeTextureFallbackGet(&tex) == XGE_OK) {
	xgeTextureFree(&tex);
}
```

**相关 API：**

- `xgeTextureFallbackSetRGBA`
- `xgeTextureFallbackClear`
- `xgeTextureLoadEx`

---

### xgeTextureFallbackClear

清除全局 fallback 纹理。

**功能：**

你可以释放全局占位纹理，通常在 shutdown 或切换资源方案时调用。

**函数原型：**

```c
XGE_API void xgeTextureFallbackClear(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

释放 XGE runtime 持有的 fallback 纹理。

**补充说明：**

- 没有设置 fallback 时调用是安全的。

**范例代码：**

```c
xgeTextureFallbackClear();
```

**相关 API：**

- `xgeTextureFallbackSetRGBA`
- `xgeTextureFallbackGet`
- `xgeTextureFree`

---

## 上传与读回

### xgeTextureUploadQueue

把纹理加入 GPU 上传队列。

**功能：**

你可以让后台解码得到的纹理延迟到 render owner 线程上传。

**函数原型：**

```c
XGE_API int xgeTextureUploadQueue(xge_texture pTexture);
```

**参数：**

- `pTexture`：输入/输出参数，不能为 `NULL`，必须是有效 RGBA8 纹理。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 没有 CPU shadow 时返回 `XGE_ERROR_RESOURCE_FAILED`。
- 分配上传节点失败时返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

XGE 会在内部队列中记录纹理指针。调用者必须保证纹理在上传完成或释放前仍有效；`xgeTextureFree` 会尝试从队列移除该纹理。

**补充说明：**

- 已经有后端纹理 ID 或已经排队时，函数直接返回 `XGE_OK`。

**范例代码：**

```c
xgeTextureUploadQueue(&tex);
```

**相关 API：**

- `xgeTextureUploadFlush`
- `xgeRenderThreadSet`
- `xgeTextureFree`

---

### xgeTextureUploadFlush

刷新 GPU 上传队列。

**功能：**

你可以在拥有 GL context 的线程把已排队纹理上传到 GPU。

**函数原型：**

```c
XGE_API int xgeTextureUploadFlush(void);
```

**参数：**

无。

**返回值：**

- 返回成功上传的纹理数量。
- 当前没有可用 GL context 时返回 `0`。

**资源归属：**

函数会消费内部上传队列节点，并释放队列节点内存。

**补充说明：**

- `xgeFlush` 当前会调用该函数。
- 上传失败的纹理不会计入返回数量。

**范例代码：**

```c
int uploaded = xgeTextureUploadFlush();
```

**相关 API：**

- `xgeTextureUploadQueue`
- `xgeFlush`
- `xgeRenderThreadGet`

---

### xgeTextureReadPixels

读回纹理像素。

**功能：**

你可以把纹理的 RGBA8 像素复制到调用者缓冲区，用于截图、测试、工具或 CPU 侧检查。

**函数原型：**

```c
XGE_API int xgeTextureReadPixels(xge_texture pTexture, void* pPixels, int iStride);
```

**参数：**

- `pTexture`：输入参数，不能为 `NULL`，必须是有效 RGBA8 纹理。
- `pPixels`：输出参数，不能为 `NULL`。
- `iStride`：输出缓冲区每行字节数。小于等于 `0` 时按 `pTexture->iWidth * 4` 处理。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或 stride 不足时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 没有 CPU shadow 或后端不支持读回时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

调用者持有 `pPixels` 缓冲区。XGE 不保存该指针。

**补充说明：**

- 读回可能导致 GPU 同步，避免每帧大量调用。

**范例代码：**

```c
xgeTextureReadPixels(&tex, pixels, tex.iWidth * 4);
```

**相关 API：**

- `xgeTextureUpdateRGBA`
- `xgeRenderTargetReadPixels`
- `xgeOffscreenReadPixels`

---

### xgeTextureFree

释放纹理引用。

**功能：**

你可以减少纹理引用计数，并在引用归零时释放 CPU shadow、GPU 后端资源和统计计数。

**函数原型：**

```c
XGE_API void xgeTextureFree(xge_texture pTexture);
```

**参数：**

- `pTexture`：输入/输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

当引用计数归零时，XGE 释放纹理持有的内部资源并清空结构体。

**补充说明：**

- `NULL` 参数会被忽略。
- 如果引用计数大于 1，只减少引用计数，不释放底层资源。

**范例代码：**

```c
xgeTextureFree(&tex);
```

**相关 API：**

- `xgeTextureAddRef`
- `xgeTextureLoad`
- `xgeTextureFallbackClear`

---

## 生命周期与所有权

`xge_image_t` 表示 CPU 侧像素数据，使用 `xgeImageFree` 释放。`xge_texture_t` 表示 GPU 纹理对象，使用 `xgeTextureFree` 释放。

`xgeTextureAddRef` 增加引用计数。每次成功 add-ref 后都需要对应 free。Fallback 纹理由 XGE runtime 持有，通过 `xgeTextureFallbackClear` 清理。

## 线程约束

图片解码可在资源线程执行。纹理创建、上传、更新、readback 必须在拥有 GL context 的线程执行，或通过 upload queue 交给 render thread。

## 后端差异

默认纹理格式为 RGBA8，按 premultiplied alpha 路径设计。不同后端的最大纹理尺寸、readback 性能和 sampler 边界能力不同，应通过 caps 查询。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 图片显示黑边或透明异常 | alpha 格式与 blend 路径不一致 | 默认使用 premultiplied alpha，加载时明确标志或转换。 |
| 后台线程直接创建 GPU 纹理失败 | GL context 不在后台线程 | 后台只解码，GPU 上传走 `xgeTextureUploadQueue` 或 render thread。 |
| 读回导致卡顿 | `xgeTextureReadPixels` 触发同步 | 只在工具、截图、测试中使用。 |

## 相关示例

- `examples/texture`
- `examples/sprite_batch`
- `build_texture_exe.bat`
