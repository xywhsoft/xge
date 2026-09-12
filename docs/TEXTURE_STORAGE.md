# 低成本纹理存储优化

XGE / XUI 默认在创建纹理时尝试低成本、像素精确的通道精简；不适合或平台不支持时保留 RGBA8。绘制、材质、自定义 `sampler2D`、更新与 CPU 回读继续使用原有接口。

这不是通用无损压缩器，也不保证所有图片都能节省显存。首版不增加 CPU 副本的 Zstd / Deflate 编解码，不增加纹理移出 GPU / 按需恢复机制，不做调色板编码。既有资源文件解码和缓存生命周期保持不变。

## 三档策略

| 创建策略 | 候选顺序 | 内容条件 |
| --- | --- | --- |
| `LOSSLESS`（默认，值为 0） | R8 → RG8 → RGBA8 | 逐像素验证，只有能够精确还原四个通道才缩减 |
| `LOSSY`（显式选择） | RGB565 → RGB5_A1 → RGBA4 → 无损链 → RGBA8 | RGB565 只用于全不透明图片；RGB5_A1 只用于 alpha 全为 0 / 255 的图片；其余可尝试 RGBA4 |
| `NONE` | RGBA8 | 不扫描内容、不打包通道 |

不符合内容条件的候选直接跳过。后端拒绝某一格式或通道重排时，销毁该次尝试的临时 GPU 对象，继续下一个候选。实际 GPU 内存不足或上下文错误直接返回错误，不把真实资源故障当作“不支持格式”反复重试。

首版有损档使用**创建时一次性的 16 位像素打包**，不是 BC / ETC / ASTC 块压缩。它的编码成本低，但只提供约 2:1 的格式数据量缩减，并会量化颜色或透明度；默认无损档不会进入这些候选。

## 无损档如何节省

只对不少于 64 个像素的 RGBA8 采样纹理尝试以下表示，且每个像素必须符合条件：

| 原始像素模式 | 存储及硬件采样还原 | 相对 RGBA8 的格式数据量 |
| --- | --- | --- |
| `R = G = B = A`，例如预乘白色字体遮罩 | R8，采样为 `(r,r,r,r)` | 25% |
| `R = G = B` 且 `A = 255`，不透明灰度 | R8，采样为 `(r,r,r,1)` | 25% |
| RGB 全白或全黑，alpha 任意 | R8 存 alpha，采样为 `(1,1,1,r)` 或 `(0,0,0,r)` | 25% |
| `R = G = B`，alpha 独立变化 | RG8 存灰度与 alpha，采样为 `(r,r,r,g)` | 50% |
| 一般彩色图片，或不符合以上条件 | RGBA8 | 100% |

例如 512 × 512 的字体图集，RGBA8 数据量为 1 MiB，R8 为 256 KiB。现有灰度字形图集无需调用方改代码即可受益；彩色字形不能假定有同样收益。

这些数字是纹理格式的理论数据量，不是驱动实际显存分配的测量值：不包含对齐、隐藏元数据、渲染目标附件，也不计驱动自行实施的透明压缩。小纹理的物理分配粒度可能抵消收益。

代价不是绝对为零：创建时有一次线性内容检查，以及选中候选的一次通道打包和临时缓冲；不符合条件的普通彩色图片可提前结束检查。原有 RGBA8 CPU 副本保留，不新增持久压缩副本，也不节省这部分 CPU 内存。绘制时依靠纹理硬件通道重排，不添加着色器分支、额外纹理访问或绘制调用。

## XGE 用法

```c
xge_texture_t texture = {0};

/* 首次创建时显式选择策略。三种模式只能选一种。 */
int result = xgeTextureCreateRGBAEx(&texture, width, height, rgba,
                                    XGE_TEXTURE_COMPRESS_NONE);
```

旧接口 `xgeTextureCreateRGBA(&texture, width, height, rgba)` 保持可用，默认尝试低成本无损优化。不能覆盖仍存活的纹理，复用对象前需 `xgeTextureFree`。

其他创建入口：

- `xgeTextureCreateFromImageEx`：为已解码图像指定模式。
- `xgeTextureLoadEx` / `xgeTextureLoadMemoryEx`：在现有 `iFlags` 中组合 `XGE_IMAGE_*` 和 `XGE_TEXTURE_COMPRESS_*`。
- `xgeAsyncTextureLoad`：现有 `iFlags` 同样支持模式，线程与同步路径一致。
- `xgeSvgTextureLoadEx` / `xgeSvgTextureLoadMemoryEx`：为光栅化后的 SVG 纹理指定模式。旧 SVG 入口默认无损。

`xgeTextureCreateRGBAEx` 和 `xgeTextureCreateFromImageEx` 不改变调用方 RGBA 像素的 alpha 语义；图片加载仍执行原有解码 / 预乘选项。全局图片加载兜底也保留此次请求的存储策略。

可选诊断，不属于普通绘制的必要步骤：

```c
xge_texture_storage_info_t info = {0};
info.iSize = sizeof(info);
if (xgeTextureGetStorageInfo(&texture, &info) == XGE_OK) {
    /* iCompression：请求的策略；iStorage：当前选中的存储格式。
       iGpuBytes：已上传纹理的格式数据量；未上传时为 0。
       iCpuBytes：此纹理保留的规范像素副本大小。 */
}
```

在 GPU 上下文就绪前创建的纹理仍走原有上传队列；在真正上传时决定存储格式。公开的 `texture.iFormat` 保持逻辑格式 `XGE_PIXEL_RGBA8`，不要用它判断实际 GPU 通道数。`xgeFontCacheGetStats().iAtlasGpuBytes` 已按选中的存储格式统计；调试纹理总量也会随选型和升级调整，但原有总量仍包含排队纹理的估算。

## XUI 用法

XUI 不扩展强制代理回调，不修改代理版本 / 结构布局。通过现有创建或加载 flags 传递 `XUI_SURFACE_COMPRESS_LOSSLESS`、`XUI_SURFACE_COMPRESS_LOSSY` 或 `XUI_SURFACE_COMPRESS_NONE`：

```c
xui_proxy_t proxy = xuiProxyXge();
xui_surface surface = NULL;
int result = proxy.surfaceCreateRGBA(&proxy, &surface,
    width, height, rgba, width * 4,
    XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_COMPRESS_LOSSLESS);
```

`surfaceCreate` 使用 `xui_surface_desc_t.iFlags`；普通图片和 SVG 的文件 / 内存加载回调也支持同一组标志。现有不传标志的代码默认无损。XGE 代理负责实际优化，其他代理可以继续保留 RGBA8。

## 更新、兼容性与边界

- 无损存储的局部更新只检查、打包脏区；内容仍符合当前精确表示时，不重建整张纹理。
- 首次写入不能精确表示的像素时，单向升级到 RGBA8；有损纹理在首次更新时也升级。以后不再反复压缩。升级保留未修改区域的原始像素；失败时不提交新的 CPU / GPU 对象。
- 升级有一次完整复制和上传，成功切换前需暂时保留新旧 GPU 对象。已知持续写入任意彩色内容的纹理可直接选 `NONE`，避免这次升级成本。
- `xgeTextureReadPixels` / XUI 普通表面的 `surfaceReadRGBA` 继续返回原有 CPU 规范像素。有损档也如此；要观察实际量化后的采样结果，应绘制到渲染目标后回读目标。
- `xgeRenderTargetCreate` 和 `XUI_SURFACE_USAGE_TARGET` 始终保留可写 RGBA8；YUV420P 原有三平面路径不变。不减少分辨率，不丢弃 mip 层，不更改 UI 缓存回收策略。
- OpenGL 3.3 / 原生 GLES 3.0 路径可尝试硬件通道重排，并在上传失败时退避。WebGL2 明确不支持纹理 swizzle，因此首版无损档保留 RGBA8；未知后端同样保守处理。有损 16 位候选不依赖 swizzle，可在受支持的 GL 兼容后端尝试。
- 公共纹理对象、引用计数和绘制接口不变；升级时底层 GL ID 可能变化。直接缓存原生 ID、外部挂载 FBO 或修改 GL 纹理状态的集成不属于普通 XGE 纹理接口，宜选择 `NONE`。

平台依据：[OpenGL ES 3.0 规范](https://registry.khronos.org/OpenGL/specs/es/3.0/es_spec_3.0.pdf)、[WebGL 2.0 规范的 No texture swizzles 限制](https://registry.khronos.org/webgl/specs/latest/2.0/)。兼容性退避不等于已完成所有平台真机验证；移动端和浏览器发布仍需按项目的平台流程验证。

## 验证

在仓库根目录执行：

```bat
build_dll.bat
test\build_texture_storage_test.bat
test\build_texture_storage_render_test.bat
```

第一组使用可注入失败的 GL 替身，验证格式链、WebGL2 保守路径、GLES 能力选择、像素还原、行距、局部更新、GPU 内存不足、规范 CPU 回读、异步创建与释放。Linux 可使用 `sh test/build_texture_storage_test.sh`，需要项目的 X11 / GL 开发依赖。

第二组需要真实 GPU 上下文，比较无损与 RGBA8 的完整输出帧，覆盖最近邻 / 线性过滤、透明混合、自定义采样着色器、升级、XUI 表面、SVG 和字体脏区上传。脚本使用现有 DLL，修改源码后先重建。
