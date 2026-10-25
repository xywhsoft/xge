# 基础渲染完整性

2026-10-26，补齐线性透明合成、基础抗锯齿和节点阴影投射/接收。保持已有 C 数据结构布局，复用现有材质、目标、渲染器与 xrt；无新增第三方依赖。进度与验收见 [记录](../plans/xge-render-quality/PROGRESS.md)，可运行场景见 [范例](../../examples/xge/xge_3d_render_quality/README.md)。

## 颜色与透明

材质颜色、节点颜色、灯光、发光和雾色使用线性 RGB；颜色/发光纹理按 sRGB 解码，法线/MR/AO 数据纹理保持线性。glTF 和 `TextureCreate` 输入保留 straight alpha，应用不需先预乘。

现在场景在缓存的线性 RGBA16F 中间目标中合成，透明 RGB 按源 Alpha 混合，Alpha 单独按 source-over 累积，最终统一转换成显示空间的预乘 RGBA8。天空、普通、实例化、蒙皮、透明材质沿用同一路径。曝光在线性空间作用于 3D 片元，发光不会在透明混合前被 8 位颜色截断；最终输出仍为 LDR，不增加 HDR 素材或色调映射接口。

例如半透明红色覆盖不透明蓝色，结果从旧的 `(128,0,128)` 修正为约 `(188,0,188)`。若背景透明，则先用最终 Alpha 还原直通线性色、转换 sRGB、再按 Alpha 预乘，避免透明边缘出现亮边。

兼容现有目标约定：

- `render_desc.clear_color` 是显示空间的预乘 RGBA，保持原有清屏观感；常用不透明清屏 Alpha 为 1。它和已有目标内容不受曝光影响。
- `TargetTexture` 和 `TargetReadPixels` 是显示空间预乘 RGBA8。可直接用于现有 2D 绘制；保存透明 PNG 时使用 `xgeImageSavePNGEx(..., XGE_IMAGE_PREMULTIPLIED)`。
- 未清颜色时，将原目标内容解码为线性预乘色再继续合成；未清深度时保留原深度。公开 3D 目标共享深度附件，窗口/外部目标按实际深度格式复制。
- 失败路径恢复修改过的 GL 状态；不承诺所有 GPU 错误下已绘制的颜色和深度均回滚。

透明仍按对象原点从远到近排序，不写深度。相交透明对象、单网格内部透明排序、折射和体积透射不在这次范围；Alpha Mask 继续写深度并支持阴影。

## 可选 FXAA

`XGE3D_ENABLE_FXAA` 默认跟随 3D。设为 0 后，设置 API 和 FXAA 着色器代码均移除，线性合成保持正确。没有单独引入后处理框架或依赖。

```c
#if XGE3D_ENABLE_FXAA
int result = xge3dRendererSetAntialiasing(renderer, XGE3D_AA_FXAA);
/* XGE3D_AA_NONE disables filtering; this is the renderer's default. */
#endif
```

FXAA 对最终显示图像进行边缘过滤，同时考虑 RGB 与 Alpha，兼容透明背景；不改变深度、阴影贴图或几何查询。先画 3D，再画 2D/XUI，可让 UI 保持清晰；如果使用不清颜色的渲染将之前的 UI 合入图像，它也会参与过滤。FXAA 不依赖多采样窗口，不解决时间闪烁，细线与高频纹理可能变软。

```bat
python tools/build/build.py 3d build/no-fxaa --define XGE3D_ENABLE_FXAA=0
```

渲染器按视口尺寸缓存中间目标，相同尺寸不重复分配，尺寸/深度格式变化时先创建新目标再替换。基础开销为一张 RGBA16F 颜色纹理和深度附件；只有首次保留旧颜色时才分配额外 RGBA8 输入纹理。约为每像素 8 B 颜色、通常 3～4 B 深度，保留旧颜色时再加 4 B；不含驱动开销，不应将 DLL 体积与 GPU 运行内存混为一谈。RendererFree 在上下文线程释放这些对象。模型上传预算不覆盖中间目标的首次分配，可在初始化阶段先渲染一帧预热。

几何 draw_calls、triangles、upload_bytes 继续统计原场景与阴影提交，不计合成器的全屏绘制/内部目标分配；RendererProfile 的 CPU/GPU 总时间包括合成。常规清屏帧追加一次全屏输出，保留旧颜色时增加一次导入。

## 节点阴影开关

启用 SHADOW 时，每个节点默认具有 `XGE3D_SHADOW_CAST | XGE3D_SHADOW_RECEIVE`，可分别关闭：

```c
/* Cast only. */
int result = xge3dNodeSetShadowFlags(scene, node, XGE3D_SHADOW_CAST);
/* Receive only. */
result = xge3dNodeSetShadowFlags(scene, floor, XGE3D_SHADOW_RECEIVE);
uint32_t flags;
result = xge3dNodeGetShadowFlags(scene, node, &flags);
```

标志作用于当前节点的网格，不向子节点继承。模型/地形包装根通常没有网格，需要遍历实际网格节点设置。非法位和失效/跨场景句柄返回 INVALID_ARGUMENT；失败保持旧标志，Get 失败保持输出值。

投射须同时满足 CAST、可见性、shadow layers 和渲染器 shadow_layers；接收由 RECEIVE 决定。关闭接收不会取消物体投射，也不改变 IBL 或 unlit 材质。原有灯光 `casts_shadow` 控制该光源是否生成阴影。Opaque/Mask 可投射，Blend 保持原有不投射规则；接收可用于三种材质模式。

开关覆盖普通/实例化/蒙皮网格和 LOD，各通道使用正确状态；不同接收状态不会错误合并为同一批次。NodeClone 复制标志，源与克隆之后可独立修改。关闭 `XGE3D_ENABLE_SHADOW` 后移除两项 API、标志存储及相关着色器。

## 运行与验证

```bat
python examples/xge/xge_3d_render_quality/build.py --run
python tests/xge/build_3d_suite.py render-quality 3d
python tests/xge/check_3d_trimming.py FXAA SHADOW LIGHTING BARE --output-root build/render-quality/trimming --evidence-root artifacts/xge-render-quality --evidence-prefix trim
```

主要验收平台为 Windows 原生 OpenGL。中间目标要求支持 RGBA16F 渲染与混合；缺少支持时明确报错。GLES/WebGL2 和其他操作系统本轮未实机验证。外部当前目标应为单颜色输出、单采样、非整数且具有深度的 framebuffer；多采样、MRT、整数输出不隐式降级，返回 UNSUPPORTED。3D 目标和普通 XGE 窗口走已验证路径；外部 sRGB 目标已在桌面 GL 验证，GLES sRGB 当前目标不支持。

验收覆盖解析颜色值、曝光/发光、透明边缘、无清屏绘制、目标尺寸变化、偏移视口、五种深度格式、sRGB 外部目标、GL 状态和上传 PBO、阴影开关与层/克隆/蒙皮/实例化组合。3d/full-dev 各 29 组回归通过，FXAA/SHADOW/LIGHTING/BARE 四种裁剪及关闭 3D 的 core 构建通过；增强发光混合测试在六个 SDK 再次通过。

同工具链纯 3D DLL 从 966,123 B 增至 972,779 B，增加 6,656 B（6.5 KiB）；关闭 FXAA 为 971,755 B，纯程序化范例 SDK 为 783,636 B。948 个旧导出全部保留，新增 3 个 API；本轮基线头文件中 59 个完整公开 3D 结构 sizeof 一致，但不是全部字段偏移或二进制兼容性的完整证明。完整结果和源码/产物哈希记录于 `artifacts/xge-render-quality/verification.json`。
