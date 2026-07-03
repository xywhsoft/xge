# Image 与 Texture Lab

本案例集中展示图像加载、内存加载、纹理创建、采样器、fallback、上传队列、局部更新和 readback 校验。

[返回范例解析](README.md) | [Texture API](../api/texture.md) | [Resource API](../api/resource.md)

## 问题

你希望确认图片文件、内存图片、程序生成纹理和 fallback 纹理都能走通，并且能在退出时打印可复查的 readback summary。

## 运行方式

```bat
examples\image_texture_lab\build.bat
build\xge_image_texture_lab.exe --frames 180
```

Unix-like 平台：

```sh
./examples/image_texture_lab/build.sh
./build/xge_image_texture_lab --frames 180
```

范例会在运行时生成 `build/image_texture_lab_asset.tga`，用于覆盖文件加载路径。

## 覆盖 API

| API | 用途 |
| --- | --- |
| `xgeImageLoad` / `xgeImageLoadEx` | 从文件加载图像 |
| `xgeImageLoadMemory` / `xgeImageLoadMemoryEx` | 从内存加载图像 |
| `xgeImageGetPixels` | 读取 image 像素指针 |
| `xgeImagePremultiply` | 手动预乘 alpha |
| `xgeImageFree` | 释放 image |
| `xgeTextureCreateRGBA` | 从程序生成 RGBA 数据创建纹理 |
| `xgeTextureCreateFromImage` | 从 image 创建纹理 |
| `xgeTextureLoad` / `xgeTextureLoadEx` | 从文件加载纹理 |
| `xgeTextureLoadMemory` / `xgeTextureLoadMemoryEx` | 从内存加载纹理 |
| `xgeTextureUpdateRGBA` | 局部更新纹理 |
| `xgeSamplerDefault` / `xgeTextureSetSampler` / `xgeTextureGetSampler` | 设置并读取采样器 |
| `xgeTextureFallbackSetRGBA` / `xgeTextureFallbackGet` / `xgeTextureFallbackClear` | fallback 纹理路径 |
| `xgeTextureUploadQueue` / `xgeTextureUploadFlush` | 上传队列路径 |
| `xgeTextureReadPixels` | 从 shadow/readback 读取纹理像素 |
| `xgeTextureAddRef` / `xgeTextureFree` | 引用计数和释放 |

## 通过标准

- 构建成功并生成 `build\xge_image_texture_lab.exe`。
- 窗口中能看到文件纹理、内存纹理、程序生成纹理和 fallback 纹理。
- 退出时打印 `image-texture-lab final-summary`。
- `readback=1` 表示局部更新后的像素被读回验证。
- `fallback=1` 表示 fallback 纹理创建与获取成功。
- 达到 `--frames` 或 `--seconds` 限制后自动退出；按 `ESC` 可提前退出。

## 可跳过条件

- 当前环境无法创建窗口或 GL context 时，可以记录构建通过，运行验证标记为跳过。
- 如果只验证无窗口资源路径，应补充专门的 resource/provider 自动测试，而不是用本窗口范例替代。
