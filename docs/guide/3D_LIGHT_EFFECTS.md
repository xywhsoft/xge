# 可选轻量画质增强

首批提供色调映射、基础调色、低分辨率 Bloom 和解析高度雾。复用现有前向 PBR、RGBA16F 线性合成、FXAA、太阳阴影和资源管理；无新第三方依赖，无光影包加载器。所有新增效果在库中默认关闭，设置复制到 Renderer。

## 开关与编译裁剪

| 宏 | 功能 | 运行时关闭 |
| --- | --- | --- |
| `XGE3D_ENABLE_TONEMAP` | Reinhard、拟合 ACES 色调映射 | Color 设置的 `tone_mapping = XGE3D_TONEMAP_NONE` |
| `XGE3D_ENABLE_COLOR_GRADING` | 线性 RGB 增益、饱和度、对比度 | 增益 `(1,1,1)`、饱和度/对比度 1 |
| `XGE3D_ENABLE_BLOOM` | 小尺寸多尺度泛光 | `RendererSetBloom(renderer, NULL)` 或 strength=0 |
| `XGE3D_ENABLE_HEIGHT_FOG` | 指数高度密度的解析雾 | `RendererSetHeightFog(renderer, NULL)` 或 density/max_opacity=0 |

四个宏独立默认跟随 3D，必须为 0/1；HEIGHT_FOG 不依赖旧距离雾 FOG，Bloom 不依赖色调映射。关闭宏移除对应存储、算法和 GLSL；TONEMAP 与 COLOR_GRADING 共享两项 Color API，仅当两者都关闭才移除该 API。单项裁剪时，Color 设置中被裁剪的功能只接受 identity 参数，否则返回 UNSUPPORTED。

关闭 Color/Bloom 不影响已有线性颜色合成。运行时关闭后跳过对应效果计算和额外绘制；仍有必要的共享管线、参数设置和开关判断，不声称整个渲染过程零成本。Bloom 关闭立即回收专用纹理，已编译程序保留供再次开启复用；从未开启时不编译 Bloom 程序，也不分配 Bloom 纹理。

```bat
python tools/build/build.py 3d build/no-light-effects --define XGE3D_ENABLE_TONEMAP=0 --define XGE3D_ENABLE_COLOR_GRADING=0 --define XGE3D_ENABLE_BLOOM=0 --define XGE3D_ENABLE_HEIGHT_FOG=0
```

## 最小 C 接入

下面各段按对应功能宏保护；Renderer 必须在有效上下文的帧回调中创建，设置与释放也在其上下文线程执行。

```c
#if XGE3D_ENABLE_TONEMAP || XGE3D_ENABLE_COLOR_GRADING
xge3d_color_settings_t color = xge3dColorDefault();
/* Default opts into fitted ACES if compiled; grading starts at identity. */
int result = xge3dRendererSetColor(renderer, &color);
#endif
#if XGE3D_ENABLE_BLOOM
xge3d_bloom_settings_t bloom = xge3dBloomDefault();
/* Check each result in application code. */
xge3dRendererSetBloom(renderer, &bloom);
#endif
#if XGE3D_ENABLE_HEIGHT_FOG
xge3d_height_fog_settings_t fog = xge3dHeightFogDefault();
fog.height = 10.0; /* Global Y metres. */
xge3dRendererSetHeightFog(renderer, &fog);
#endif
```

`Default()` 返回可直接启用的预设，调用它不会改变 Renderer。传 NULL 关闭 Color 的两项功能或对应的 Bloom/高度雾。非法 size、NaN、无穷和超出范围的参数在提交前拒绝，旧设置保留。当前没有自动曝光、LUT 调色、体积采样、屏幕空间反射或时序历史。

## 色调映射、调色与透明

渲染顺序为：材质/灯光 → 距离雾 → 高度雾 → 已有曝光倍率 → 线性透明合成 → Bloom → 基础调色 → 色调映射 → sRGB 与显示空间预乘 Alpha → 可选 FXAA → 后续 2D/XUI。

现有 `render_desc.exposure` 继续使用线性倍率，0 表示 1。Color 不新增独立曝光。调色先应用 gain，再按 Rec.709 亮度调整饱和度，最后以线性 0.18 为支点调整对比度，负值归零。饱和度/对比度范围 `[0,4]`，各 gain 范围 `[0,16]`；这些是简单参数调色，未引入额外纹理和 LUT。

Reinhard 使用 `x/(1+x)`；ACES 使用 Narkowicz 的拟合曲线（[原始说明及 CC0 许可选项](https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/)），不等同完整 ACES 色彩管理。它们都在显示编码前处理 HDR 线性色。两种曲线的明暗和饱和度观感不同，应结合素材与曝光选择。关闭时保持之前的截断输出。

Color/Bloom 处理完整 3D 颜色帧，因此启用非 identity Color 或非零 Bloom 时，Render 必须包含 `XGE3D_CLEAR_COLOR`；否则返回 UNSUPPORTED，防止把上一次已经映射的画面再处理一次。多视口可分别渲染到独立 3D Target 后再由 2D 合成。高度雾和 identity Color 没有这个限制。清屏颜色、天空也参与完整帧 Color/Bloom；2D/XUI 应在这些效果之后绘制，保持 UI 色彩与清晰度。

公开 Target 输出仍为显示空间预乘 RGBA8。Color 在去预乘的颜色上运算，再按 Alpha 预乘；透明 Bloom 扩散颜色并扩展覆盖 Alpha，使物体轮廓外也能产生光晕。Opaque 背景 Alpha 保持 1。保存透明 PNG 使用 `XGE_IMAGE_PREMULTIPLIED`。Glow 是画面效果，不会让发光材质自动成为照亮其他物体的光源。

## Bloom 成本与资源

Bloom 首层为视口的半分辨率（奇数向上取整），逐级缩小至 1×1 或指定层数，最多 6 层。先用柔和阈值提取高亮，再降采样/逐级上采样合成；首层按实际层数归一化，避免增加层数就无条件增加常量画面的能量。使用固定大小数组管理最多六个目标，无逐帧 C 容器分配或 CPU 像素读回。

strength 范围 `[0,4]`、threshold/knee 范围 `[0,64]`。默认 strength=.08、threshold=1、knee=.5、levels=4。knee=0 为硬阈值；threshold/knee=0 时对非黑色像素执行无阈值扩散。更大 levels 让光晕更宽，也增加 Pass；不是简单的“越多越好”。它是轻量空间滤波，不提供时序稳定或物理镜头的完整模拟。

实际 n 层执行 `2n-1` 个额外全屏三角形 Pass，均在小尺寸目标进行；最后的合成继续复用原输出 Pass。四层为 7 个额外 Pass。全部使用 RGBA16F，字节数为 `sum(width[i] * height[i] * 8)`；偶数尺寸完整纹理链约为主颜色目标的三分之一，不计驱动/程序对象开销。

```c
#if XGE3D_ENABLE_BLOOM
xge3d_bloom_info_t info = {.size = sizeof(info)};
xge3dRendererBloomInfo(renderer, &info);
/* After a successful disabled Render: levels=passes=storage_bytes=0. */
#endif
```

Info 可在资源预热和质量选择时用于观测。保持尺寸/层数时复用纹理；重建目标先创建新链，成功后替换，失败保留原链。Shader/纹理首次申请可能造成开启帧的延迟，应用可在加载阶段先渲染一帧预热。失败路径恢复 GL 状态，但不保证 GPU 故障后已经写入的深度和画面全部回滚。

## 高度雾

密度随全局高度变化：`rho(y) = density * exp(-falloff * (y-height))`。对相机到片元的射线积分，采用稳定的近水平分支；没有体积纹理、辅助深度目标或光线步进。falloff=0 为均匀密度；max_opacity 限制最大混合量。density/falloff 范围 `[0,1000]`，max_opacity 范围 `[0,1]`。颜色为非负有限线性 RGB。

height 使用 double 全局 Y，Render 根据 Scene origin 转成相对坐标，原点重定位不要求应用修改雾的世界高度。透视使用真实视线长度，正交使用平行射线及对应相机平面的起点。当前按世界 Y 分层，仅处理网格片元；天空不变。旧距离雾启用时先处理距离雾，再处理高度雾。它不产生阴影光束、体积散射、多次散射或局部雾体。

## 范例与验证

```bat
python examples/xge/xge_3d_light_effects/build.py --run
python tests/xge/build_3d_suite.py light-effects 3d
```

[范例](../../examples/xge/xge_3d_light_effects/README.md)提供四项独立开关与 all/none 自动截图。数值验证比较色调曲线、调色、透明预乘及雾积分；GPU 验证覆盖 Bloom 能量/层数/开关、纹理计数、奇数/极小尺寸、内存失败重试及 GL 状态恢复。验收结果与平台边界见[实施进度](../plans/xge-light-effects/PROGRESS.md)。
