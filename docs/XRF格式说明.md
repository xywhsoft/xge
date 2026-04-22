# XRF 字体格式说明

XRF 是 XGE V2 的快速 bitmap font/cache 格式。它不是复杂排版格式，也不负责压缩、资源打包或富文本；这些能力分别由 XUI、xpack/xrt 资源层和上层 UI 系统处理。

## 定位

XRF 用于：

- 快速加载预烘焙点阵字。
- 快速渲染中文 UCS2 范围字体。
- 作为 TTF glyph rasterize/cache 的持久化后端。
- 降低板卡 Linux、小程序、WebGL、移动端低端设备上的字体初始化成本。
- 避免每次启动时重新 rasterize 大量中文 glyph。

非目标：

- 不保存段落排版。
- 不保存 rich text 样式。
- 不内建压缩。
- 不保存完整 OpenType feature。
- 第一版不支持 emoji 和 UCS4 完整范围。

## 基线

第一版 XRF 基线：

- 文件 magic：`XGE_XRF_MAGIC`
- 版本：`XGE_XRF_VERSION == 1`
- 字符范围：UCS2，`0x0000 - 0xFFFF`
- 输入文本编码：UTF-8，由 XGE 解码到 codepoint。
- Atlas 格式：A8 或 RGBA8。
- 支持多 atlas page。
- 支持 glyph metrics。
- 支持 ascent/descent/lineGap。
- 支持 range table。
- 可选 kerning table。

当前常量：

```c
#define XGE_XRF_MAGIC			0x32465258u
#define XGE_XRF_VERSION			1
#define XGE_XRF_PAGE_A8			1
#define XGE_XRF_PAGE_RGBA8		2
#define XGE_XRF_FLAG_KERNING	0x0001
```

`0x32465258u` 对应 little-endian 文件中的 `XRF2`。

## 文件布局

XRF v1 文件布局：

```text
xge_xrf_header_t
xge_xrf_range_t[]
xge_xrf_glyph_t[]
xge_xrf_page_t[]
xge_xrf_kerning_t[] optional
pixel data blocks
```

实际读取必须以 header 中的 offset 为准，不依赖固定顺序。推荐生成器按上述顺序写入，便于人类检查和顺序读取。

所有多字节数值使用 little-endian。结构体中的 `float` 使用 IEEE-754 32-bit float。

## Header

当前 header：

```c
typedef struct xge_xrf_header_t {
	uint32_t iMagic;
	uint16_t iVersion;
	uint16_t iHeaderSize;
	uint32_t iFlags;
	uint32_t iGlyphCount;
	uint32_t iPageCount;
	uint32_t iRangeCount;
	uint32_t iKerningCount;
	float fAscent;
	float fDescent;
	float fLineGap;
	uint32_t iGlyphOffset;
	uint32_t iPageOffset;
	uint32_t iRangeOffset;
	uint32_t iKerningOffset;
	uint32_t iPixelOffset;
} xge_xrf_header_t;
```

字段说明：

- `iMagic`：必须为 `XGE_XRF_MAGIC`。
- `iVersion`：当前为 1。
- `iHeaderSize`：header 字节数，用于兼容后续扩展。
- `iFlags`：格式标志，目前定义 kerning 标志。
- `iGlyphCount`：glyph record 数量。
- `iPageCount`：atlas page 数量。
- `iRangeCount`：range record 数量。
- `iKerningCount`：kerning record 数量。
- `fAscent`：字体 ascender。
- `fDescent`：字体 descender。
- `fLineGap`：行间距。
- `iGlyphOffset`：glyph table 文件偏移。
- `iPageOffset`：page table 文件偏移。
- `iRangeOffset`：range table 文件偏移。
- `iKerningOffset`：kerning table 文件偏移，无 kerning 时为 0。
- `iPixelOffset`：pixel data 区域起始偏移。

校验规则：

- magic、version 必须匹配。
- header size 必须大于等于当前结构体最小尺寸。
- offsets 必须落在文件大小范围内。
- count * record size 不能溢出。
- page pixel block 不能越界。

## Range Record

```c
typedef struct xge_xrf_range_t {
	uint32_t iFirstCodepoint;
	uint32_t iCount;
} xge_xrf_range_t;
```

Range 用于描述文件覆盖的 codepoint 范围，便于快速判断某个字体是否可能包含目标 glyph。

约定：

- range 应按 `iFirstCodepoint` 升序排列。
- range 不应重叠。
- UCS2 第一版要求 `iFirstCodepoint + iCount - 1 <= 0xFFFF`。
- 允许 range table 缺失，但推荐生成器始终写入。

## Glyph Record

```c
typedef struct xge_xrf_glyph_t {
	uint32_t iCodepoint;
	uint16_t iPage;
	uint16_t iGlyph;
	uint16_t iX;
	uint16_t iY;
	uint16_t iWidth;
	uint16_t iHeight;
	float fOffsetX;
	float fOffsetY;
	float fAdvanceX;
} xge_xrf_glyph_t;
```

字段说明：

- `iCodepoint`：Unicode codepoint。
- `iPage`：所在 atlas page index。
- `iGlyph`：源字体 glyph index 或生成器内部 glyph id。
- `iX/iY`：glyph bitmap 在 page 中的位置。
- `iWidth/iHeight`：glyph bitmap 尺寸。
- `fOffsetX/fOffsetY`：绘制时相对 pen position 的偏移。
- `fAdvanceX`：绘制后 pen X 前进量。

约定：

- glyph table 推荐按 `iCodepoint` 升序排列，运行时可二分查找。
- 同一 codepoint 不应有多个 glyph record。
- `iPage` 必须小于 `iPageCount`。
- glyph rect 必须落在对应 page 尺寸范围内。
- 缺字由 fallback font chain 或 missing glyph placeholder 处理，不要求 XRF 内必须包含所有字符。

## Page Record

```c
typedef struct xge_xrf_page_t {
	uint16_t iWidth;
	uint16_t iHeight;
	uint16_t iFormat;
	uint16_t iReserved;
	uint32_t iPixelOffset;
	uint32_t iPixelSize;
} xge_xrf_page_t;
```

字段说明：

- `iWidth/iHeight`：atlas page 尺寸。
- `iFormat`：`XGE_XRF_PAGE_A8` 或 `XGE_XRF_PAGE_RGBA8`。
- `iReserved`：保留，生成器写 0，loader 忽略。
- `iPixelOffset`：该 page pixel data 的文件偏移。
- `iPixelSize`：pixel data 字节数。

格式：

- A8：每像素 1 字节 alpha。
- RGBA8：每像素 4 字节 RGBA。

推荐：

- 常用 page 尺寸为 1024x1024 或 2048x2048。
- 小程序和低端设备优先使用 1024x1024，避免纹理尺寸上限问题。
- A8 适合纯字体 alpha mask，运行时由 shader 乘以文字颜色。
- RGBA8 适合彩色位图字或特殊字体效果。

## Kerning Record

```c
typedef struct xge_xrf_kerning_t {
	uint32_t iLeftCodepoint;
	uint32_t iRightCodepoint;
	float fAdvanceX;
} xge_xrf_kerning_t;
```

Kerning 是可选能力。

约定：

- `XGE_XRF_FLAG_KERNING` 置位时，`iKerningCount` 和 `iKerningOffset` 应有效。
- kerning table 推荐按 `(left, right)` 排序。
- 第一版渲染路径可以先忽略 kerning，但 loader 必须能跳过或保存数据。

## 加载流程

运行时加载流程：

```text
read file or memory
validate header
validate table offsets and sizes
copy or map glyph/page/range/kerning data
create atlas textures from pixel blocks
fill xge_font_t metrics and glyph atlas
mark font as XRF-backed
```

当前公开 API：

```c
int xgeFontLoadXRF(xge_font pFont, const char* sPath);
int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize);
```

加载失败时应返回错误码，不应留下半初始化 font。若已有 font 内容，调用方应先释放或使用独立对象加载。

## 渲染路径

XRF 字体渲染路径：

```text
UTF-8 decode
  -> codepoint
  -> glyph lookup in XRF glyph table
  -> fallback font if missing
  -> emit glyph quad
  -> batch by atlas page texture
```

A8 page shader：

- 使用 atlas alpha。
- 输出颜色来自 draw color。
- 默认 premultiplied alpha 路径。

RGBA8 page shader：

- 使用 atlas RGBA。
- 可乘以 draw color，具体策略由 material/text shader 定义。

## TTF-to-XRF Cache

TTF-to-XRF cache path 是目标能力，当前 spec 中仍标记为未完成实现。

目标流程：

```text
load TTF
select font size and glyph ranges
rasterize glyphs
pack atlas pages
write XRF v1
runtime load XRF fast path
```

生成器输入建议：

- TTF path。
- 输出 XRF path。
- font size。
- ranges，例如 `0x20-0x7E,0x4E00-0x9FFF`。
- atlas size。
- format A8/RGBA8。
- 是否写 kerning。

该能力可以先作为离线工具，不进入 XGE runtime。

## 与 xpack/xrt 的关系

XRF 不内建压缩。原因：

- 压缩策略属于资源包和 IO 层。
- xpack 可以压缩整个 XRF 文件。
- 小程序、WebGL 和板卡环境可以按平台选择 gzip/brotli/包内压缩。
- runtime loader 保持简单，减少依赖和内存峰值。

加载来源：

- 普通文件路径。
- memory。
- `res://` 资源协议。
- xpack provider 返回的内存块。

## 兼容性与扩展

v1 兼容策略：

- loader 使用 `iHeaderSize` 跳过未来 header 扩展字段。
- 未识别 flags 默认忽略或返回不支持，具体取决于 flag 是否影响解析。
- record 结构 v1 固定，未来新增复杂数据应通过新 table offset 或 version 处理。

可能的 v2 扩展：

- Unicode plane > UCS2。
- SDF/MSDF 字体页。
- per-glyph bearing/advance Y。
- vertical text metrics。
- compressed page block 标志。
- page mipmap 或 distance field 参数。
- glyph name/debug table。

## 测试要求

XRF loader 测试必须覆盖：

- magic/version 错误。
- offset 越界。
- count 溢出。
- A8 page。
- RGBA8 page。
- glyph lookup。
- range lookup。
- missing glyph fallback。
- memory load。
- 多 page 字体。

生成器测试后续补充：

- TTF 输入到 XRF 输出。
- XRF 再加载后 metrics 一致。
- atlas rect 不重叠。
- UCS2 中文范围抽样渲染。

