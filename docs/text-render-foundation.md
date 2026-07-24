# XGE 文字渲染底座

## 目标

文字渲染底座将字体资源、字体实例、文字塑形、字形缓存和文本装饰拆分为独立层次。旧的 `xgeFontLoad`、`xgeTextMeasure`、`xgeTextDraw` 接口继续保留，新功能通过增量 API 提供。

当前版本完成基础塑形后端。接口允许以后接入 HarfBuzz，但当前实现仍使用 stb_truetype 提供 glyph 映射、kerning 和栅格化，不宣称支持复杂文字塑形和双向文本。

## 对象模型

### FontFace

`xge_font_face` 持有字体文件、TTC face index、stb 字体信息和标准字体元数据。多个字号实例可以共享同一个 face，避免重复复制字体文件。

通过 `xgeFontFaceLoad` 或 `xgeFontFaceLoadMemory` 创建，通过引用计数管理。

### FontFamily

`xge_font_family` 保存一组 face。`xgeFontFamilyResolve` 根据 weight 和 slant 选择最接近的真实 face，不执行人工加粗或人工倾斜。

### FontInstance

现有 `xge_font_t` 继续表示确定字号的字体实例。`xgeFontCreate` 使用 face 和像素字号创建实例。

为了保持源代码兼容，`xge_font_t` 的公开尺寸没有增加。内部 `pBackend` 不再直接指向 `stbtt_fontinfo`，调用方不应访问 `pBackend`、`pData` 或 `pGlyphs`。

## GlyphRun

`xgeTextShape` 生成 `xge_glyph_run_t`，每个 glyph position 包含：

- Unicode codepoint；
- UTF-8 byte cluster；
- glyph id；
- 实际使用的 fallback 字体；
- advance 和 offset；
- 换行标志。

基础后端支持四字节 UTF-8、字体 fallback、标准 kerning、多行测量和 cluster hit test。`xgeGlyphRunFree` 释放 run 及其字体引用。

旧 `xgeTextMeasure` 保持零堆分配扫描，避免影响 XUI 高频测量。需要缓存布局或进行选择命中的上层控件应持有 GlyphRun。

## 矢量字形

`xgeFontGlyphOutlineAppendShapeEx` 将指定 glyph id 的 TrueType 轮廓追加到
ShapeEx path，`xgeGlyphRunAppendShapeEx` 则按 GlyphRun 的 advance、offset、换行和
fallback 字体组合整段路径。轮廓数据缓存在 FontFace 上，因此同一字体的不同字号实例
共享 `stbtt_vertex` 数据，只在输出时应用实例缩放。

SVG 使用该出口实现填充、描边、渐变和任意仿射变换；普通 XGE/XUI 文本仍优先使用
atlas 路径，以保持高频界面文字的批处理效率。两条渲染路径共享 FontFace、字体指标、
glyph 映射和 GlyphRun，不重复解析字体。

ShapeEx 当前的边缘覆盖质量仍由 ShapeEx 渲染器决定。矢量字形出口不单独实现另一套
抗锯齿器，后续 ShapeEx 的覆盖抗锯齿改进会同时作用于 SVG 字形和其他矢量路径。

## 字形缓存

字形和 codepoint 映射使用独立哈希表：

- glyph id 映射到唯一 atlas glyph；
- 多个 codepoint 可以映射到同一个 glyph；
- 查询平均复杂度为 O(1)；
- cache stats 可通过 `xgeFontCacheGetStats` 获取。

动态字形先写入 CPU atlas。绘制 GlyphRun 前统一创建缺失字形，然后按照每页 dirty rectangle 调用 `xgeTextureUpdateRGBA`。atlas 纹理对象在局部更新期间保持稳定，已经提交的绘制命令不会引用被覆盖的纹理对象。

## XRF v2

新写出的 XRF 使用 version 2，增加：

- 字体源数据 hash；
- face index；
- 像素字号；
- weight、stretch、slant；
- underline 和 strike 指标。

加载器继续读取 version 1。`xgeFontLoadCached` 会验证 XRF 与 TTF 的 source hash、face 和行高：

- 匹配时保留 XRF atlas，并附加 TTF face；
- 缓存未命中的字符可以动态生成到新 atlas page；
- 不匹配时丢弃旧 XRF，从 TTF 重建。

XRF 预载页面被标记为不可继续打包，动态字形写入新页面，避免覆盖缓存像素。

## 文本装饰

`xge_text_decoration_t` 支持 underline、overline、strike、squiggle、dotted 和 dashed。字体实例读取 OpenType `post` 与 `OS/2` 表中的 underline/strike 指标，缺失时使用与字号相关的默认值。

装饰不参与 glyph shaping。`xgeGlyphRunDrawDecorated` 在 glyph 绘制后绘制整段装饰。XUI 的诊断 Annotation 应先通过布局层切分范围，再对对应视觉片段调用装饰绘制。

## 尚未完成

- HarfBuzz 复杂文字塑形；
- Unicode bidi 和标准断行；
- variable font axis；
- atlas 页预算、淘汰和压缩；
- XRF shaping 结果或 kerning pair 持久化；
- XUI RichText span 与 Annotation 接入。

这些能力应建立在当前 API 上，不应重新让上层依赖 codepoint 逐字绘制。
