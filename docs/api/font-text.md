# Font / Text API

> Font / Text API 负责 UTF-8 解码、TTF/XRF 字体加载、字形查询、文本测量和文本绘制。

[返回 API 索引](README.md) | [文本教程](../guide/text-intro.md) | [Text 范例](../case/text-rendering.md)

---

## 模块定位

XGE 使用 UTF-8 作为文本输入编码。字体模块支持 TTF 加载，也支持 XRF 点阵字体格式。XRF 的定位是快速加载、快速渲染的字形缓存格式，可作为 TTF 渲染缓存后端。

第一版目标支持中文 UCS2 范围。emoji 暂不作为第一版目标。当前实现基于 `stb_truetype`，如果构建关闭字体后端，相关函数会返回 `XGE_ERROR_UNSUPPORTED` 或空结果。

## 标准调用顺序

```text
xgeFontLoad / xgeFontLoadXRF / xgeFontLoadCached
  -> xgeTextMeasure / xgeTextDraw / xgeTextDrawRect
  -> xgeFontFree
```

需要 CPU 字形位图时，使用 `xgeFontGlyphRasterize`，并用 `xgeGlyphBitmapFree` 释放返回位图。

## 常量与宏

| 名称 | 值 | 说明 |
| --- | --- | --- |
| `XGE_FONT_FALLBACK` | `0x0001` | 字体来自 fallback。 |
| `XGE_TEXT_ALIGN_LEFT` | `0x0000` | 左对齐。 |
| `XGE_TEXT_ALIGN_CENTER` | `0x0001` | 水平居中。 |
| `XGE_TEXT_ALIGN_RIGHT` | `0x0002` | 右对齐。 |
| `XGE_TEXT_ALIGN_TOP` | `0x0000` | 顶部对齐。 |
| `XGE_TEXT_ALIGN_MIDDLE` | `0x0010` | 垂直居中。 |
| `XGE_TEXT_ALIGN_BOTTOM` | `0x0020` | 底部对齐。 |
| `XGE_TEXT_CLIP` | `0x0100` | 裁剪到目标矩形。 |
| `XGE_XRF_MAGIC` | `0x32465258u` | XRF 文件 magic。 |
| `XGE_XRF_VERSION` | `1` | XRF 格式版本。 |
| `XGE_XRF_PAGE_A8` | `1` | A8 字形页。 |
| `XGE_XRF_PAGE_RGBA8` | `2` | RGBA8 字形页。 |
| `XGE_XRF_FLAG_KERNING` | `0x0001` | 包含 kerning 数据。 |

## 公共类型

### `xge_font_t`

| 字段 | 说明 |
| --- | --- |
| `iRefCount` | 引用计数。 |
| `fSize` / `fScale` | 字号和缩放。 |
| `fAscent` / `fDescent` / `fLineGap` / `fLineHeight` | 字体垂直度量。 |
| `iFlags` | 字体标志。 |
| `pData` / `iDataSize` | 字体数据。 |
| `pBackend` | 字体后端状态。 |
| `pFallback` | fallback 字体。 |
| `tAtlas` | 字形 atlas。 |
| `pGlyphs` | 字形缓存。 |

### `xge_glyph_metrics_t`

字形度量数据，包含 codepoint、glyph index、advance、bearing 和字形边界。

### `xge_glyph_bitmap_t`

CPU 字形位图。`pPixels` 由 `xgeFontGlyphRasterize` 分配时，调用者必须用 `xgeGlyphBitmapFree` 释放。

### `xge_glyph_t`

Atlas 中的字形位置，包括 page、x/y、width/height、offset 和 advance。

## API 参考

### xgeTextUTF8Next

读取下一个 UTF-8 codepoint。

**功能：**

你可以用它遍历 XGE 文本字符串，并让函数自动推进输入指针。

**函数原型：**

```c
XGE_API int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint);
```

**参数：**

- `psText`：输入/输出字符串指针，不能为 `NULL`，且 `*psText` 不能为 `NULL`。
- `pCodepoint`：输出 Unicode codepoint，不能为 `NULL`。

**返回值：**

- 成功读取返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 遇到字符串结束返回 `XGE_ERROR`。
- 遇到当前不支持或非法的 UTF-8 序列返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

函数不分配资源。`psText` 指向调用者持有的字符串。

**补充说明：**

- 成功时 `*psText` 会前进到下一个字符位置。
- 第一版字体渲染主要承诺 UCS2 范围，超出范围的 codepoint 可能无法被后续字体 API 处理。

**范例代码：**

```c
const char* text = "XGE";
uint32_t cp;
while (xgeTextUTF8Next(&text, &cp) == XGE_OK) {
    /* 使用 cp */
}
```

**相关 API：**

- `xgeTextMeasure`
- `xgeTextDraw`

---

### xgeFontLoad

从 TTF 文件加载字体。

**功能：**

你可以用它从资源路径加载 TrueType 字体，并按指定字号初始化 `xge_font_t`。

**函数原型：**

```c
XGE_API int xgeFontLoad(xge_font pFont, const char* sPath, float fSize);
```

**参数：**

- `pFont`：输出字体对象，不能为 `NULL`。
- `sPath`：字体路径，不能为 `NULL`。
- `fSize`：字号，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 资源读取、内存分配或字体解析失败时返回对应错误码。
- 如果加载失败且存在全局 fallback，可能返回 fallback 加载结果。

**资源归属：**

创建成功后，`pFont` 持有字体数据副本和后端状态。调用者必须使用 `xgeFontFree` 释放。

**补充说明：**

- 路径加载经由 XGE Resource API，因此可受资源协议和 VFS 影响。
- 加载失败时会尝试使用全局 fallback 字体。

**范例代码：**

```c
xge_font_t font;
if (xgeFontLoad(&font, "assets/ui.ttf", 18.0f) == XGE_OK) {
    xgeTextDraw(&font, "Hello", 20.0f, 20.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
    xgeFontFree(&font);
}
```

**相关 API：**

- `xgeFontLoadMemory`
- `xgeFontFree`
- `xgeFontFallbackSet`

---

### xgeFontLoadMemory

从内存 TTF 加载字体。

**功能：**

你可以在资源系统、压缩包、网络下载或内嵌资源中取得字体数据后直接初始化字体。

**函数原型：**

```c
XGE_API int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize);
```

**参数：**

- `pFont`：输出字体对象，不能为 `NULL`。
- `pData`：TTF 数据，不能为 `NULL`。
- `iSize`：数据字节数，必须大于 0。
- `fSize`：字号，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。
- 字体解析失败返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

函数会复制 `pData`。调用返回后输入内存可以释放，字体对象用 `xgeFontFree` 释放。

**补充说明：**

加载成功后会计算 ascent、descent、line gap 和 line height。

**范例代码：**

```c
xge_font_t font;
int ret = xgeFontLoadMemory(&font, data, size, 20.0f);
```

**相关 API：**

- `xgeFontLoad`
- `xgeFontFree`

---

### xgeFontLoadXRF

从 XRF 文件加载字体缓存。

**功能：**

你可以用它快速加载预生成的点阵字形缓存，减少 TTF 首次栅格化成本。

**函数原型：**

```c
XGE_API int xgeFontLoadXRF(xge_font pFont, const char* sPath);
```

**参数：**

- `pFont`：输出字体对象，不能为 `NULL`。
- `sPath`：XRF 路径，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 资源读取或 XRF 解析失败时返回对应错误码。

**资源归属：**

创建成功后，`pFont` 持有 XRF 中解析出的 atlas 和 glyph cache，使用 `xgeFontFree` 释放。

**补充说明：**

XRF 适合工具链预生成常用字符集，例如中文 UI 常用字范围。

**范例代码：**

```c
xge_font_t font;
xgeFontLoadXRF(&font, "assets/ui.xrf");
```

**相关 API：**

- `xgeFontLoadXRFMemory`
- `xgeFontSaveXRF`

---

### xgeFontLoadXRFMemory

从内存 XRF 加载字体缓存。

**功能：**

你可以在压缩包、内嵌资源或自定义 VFS 中读取 XRF 数据后直接加载。

**函数原型：**

```c
XGE_API int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize);
```

**参数：**

- `pFont`：输出字体对象，不能为 `NULL`。
- `pData`：XRF 数据，不能为 `NULL`。
- `iSize`：数据字节数，至少要容纳 XRF header。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- XRF magic、version、offset 或数据范围非法返回 `XGE_ERROR_RESOURCE_FAILED`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会把需要的 glyph/page 数据转换到字体对象内部，调用者仍持有输入内存。

**补充说明：**

- 当前 XRF 版本为 `XGE_XRF_VERSION`。
- 页数据当前以 RGBA8 atlas 路径为主。

**范例代码：**

```c
xgeFontLoadXRFMemory(&font, xrf_data, xrf_size);
```

**相关 API：**

- `xgeFontLoadXRF`
- `xgeFontBuildXRFMemory`

---

### xgeFontBuildXRFMemory

构建 XRF 内存块。

**功能：**

你可以把当前字体的指定 codepoint 范围预栅格化并打包成 XRF 数据，用于后续快速加载。

**函数原型：**

```c
XGE_API int xgeFontBuildXRFMemory(xge_font pFont, uint32_t iFirstCodepoint, uint32_t iCount, void** ppData, int* pSize);
```

**参数：**

- `pFont`：已加载的字体。
- `iFirstCodepoint`：起始 codepoint。
- `iCount`：codepoint 数量。
- `ppData`：输出内存指针，不能为 `NULL`。
- `pSize`：输出字节数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或范围非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 没有可导出的 glyph 返回 `XGE_ERROR_RESOURCE_FAILED`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

成功后 `*ppData` 由 XGE 使用 `xrtMalloc` 分配，调用者使用完后调用 `xrtFree` 释放。

**补充说明：**

- 函数会先尝试把范围内字形放入 atlas。
- 当前导出页格式为 RGBA8。

**范例代码：**

```c
void* data = NULL;
int size = 0;
if (xgeFontBuildXRFMemory(&font, 0x4E00, 512, &data, &size) == XGE_OK) {
    xrtFree(data);
}
```

**相关 API：**

- `xgeFontSaveXRF`
- `xgeFontGlyphAtlasGet`

---

### xgeFontSaveXRF

保存 XRF 文件。

**功能：**

你可以把指定字形范围导出为 XRF 文件，作为下次启动的字体缓存。

**函数原型：**

```c
XGE_API int xgeFontSaveXRF(xge_font pFont, const char* sPath, uint32_t iFirstCodepoint, uint32_t iCount);
```

**参数：**

- `pFont`：已加载的字体。
- `sPath`：输出路径，不能为 `NULL`。
- `iFirstCodepoint`：起始 codepoint。
- `iCount`：codepoint 数量。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 构建 XRF 失败时返回 `xgeFontBuildXRFMemory` 的错误码。
- 路径解析或写文件失败返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

函数内部构建的 XRF 内存会在返回前释放。输出文件归调用者或资源管线管理。

**补充说明：**

保存路径会经过 XGE 路径解析。

**范例代码：**

```c
xgeFontSaveXRF(&font, "assets/ui_cache.xrf", 0x20, 0x100);
```

**相关 API：**

- `xgeFontBuildXRFMemory`
- `xgeFontLoadXRF`

---

### xgeFontLoadCached

优先加载 XRF 缓存，不存在时从 TTF 构建。

**功能：**

你可以用它简化字体缓存流程：先尝试读取 XRF，失败后加载 TTF，并尝试保存新的 XRF。

**函数原型：**

```c
XGE_API int xgeFontLoadCached(xge_font pFont, const char* sTTFPath, const char* sXRFPath, float fSize, uint32_t iFirstCodepoint, uint32_t iCount);
```

**参数：**

- `pFont`：输出字体对象，不能为 `NULL`。
- `sTTFPath`：TTF 路径，不能为 `NULL`。
- `sXRFPath`：XRF 缓存路径，不能为 `NULL`。
- `fSize`：TTF 字号，必须大于 0。
- `iFirstCodepoint`：缓存构建起始 codepoint。
- `iCount`：缓存构建数量。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或加载失败时返回对应错误码。

**资源归属：**

成功后 `pFont` 由调用者持有，并使用 `xgeFontFree` 释放。

**补充说明：**

- 如果 XRF 加载成功，不会再加载 TTF。
- 如果 TTF 加载成功但 XRF 保存失败，函数仍返回 `XGE_OK`，因为运行时字体已可用。

**范例代码：**

```c
xgeFontLoadCached(&font, "assets/ui.ttf", "assets/ui.xrf", 18.0f, 0x20, 0x200);
```

**相关 API：**

- `xgeFontLoad`
- `xgeFontLoadXRF`
- `xgeFontSaveXRF`

---

### xgeFontAddRef

增加字体引用计数。

**功能：**

你可以在多个对象共享同一个字体时调用它，避免字体被提前释放。

**函数原型：**

```c
XGE_API int xgeFontAddRef(xge_font pFont);
```

**参数：**

- `pFont`：字体对象，可以为 `NULL`。

**返回值：**

- 字体有效时返回新的引用计数。
- `pFont` 为 `NULL` 时返回 `0`。

**资源归属：**

每次增加引用后，都需要在不再使用时调用一次 `xgeFontFree`。

**补充说明：**

引用计数达到 `INT32_MAX` 后不再继续增加。

**范例代码：**

```c
xgeFontAddRef(&font);
```

**相关 API：**

- `xgeFontFree`

---

### xgeFontFree

释放字体引用或字体资源。

**功能：**

你可以用它减少字体引用计数，并在引用归零时释放后端状态、字体数据、atlas 和 glyph cache。

**函数原型：**

```c
XGE_API void xgeFontFree(xge_font pFont);
```

**参数：**

- `pFont`：字体对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pFont` 内部资源，不释放结构体本身。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 引用计数大于 1 时只减少引用，不释放实际资源。

**范例代码：**

```c
xgeFontFree(&font);
```

**相关 API：**

- `xgeFontLoad`
- `xgeFontAddRef`

---

### xgeFontSetFallback

设置对象级 fallback 字体。

**功能：**

你可以为某个字体指定 fallback，当主字体缺字或没有后端信息时，glyph 查询会尝试转到 fallback。

**函数原型：**

```c
XGE_API void xgeFontSetFallback(xge_font pFont, xge_font pFallback);
```

**参数：**

- `pFont`：要设置 fallback 的字体。
- `pFallback`：fallback 字体，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只保存借用指针，不增加引用计数。调用者必须保证 fallback 字体生命周期覆盖使用期。

**补充说明：**

如果需要长期共享 fallback，请调用者自行管理引用计数。

**范例代码：**

```c
xgeFontSetFallback(&latin_font, &cjk_font);
```

**相关 API：**

- `xgeFontFallbackSet`
- `xgeFontGlyphGet`

---

### xgeFontFallbackSet

设置全局 fallback 字体。

**功能：**

你可以用它配置一个全局 fallback TTF，供 `xgeFontLoad` 失败时或显式 `xgeFontFallbackGet` 时使用。

**函数原型：**

```c
XGE_API int xgeFontFallbackSet(const char* sPath, float fSize);
```

**参数：**

- `sPath`：fallback TTF 路径，不能为 `NULL`。
- `fSize`：字号，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 资源读取或加载失败返回对应错误码。

**资源归属：**

全局 fallback 字体由 XGE 全局状态持有，调用 `xgeFontFallbackClear` 清除。

**补充说明：**

设置新 fallback 前会清理旧 fallback。

**范例代码：**

```c
xgeFontFallbackSet("assets/cjk.ttf", 18.0f);
```

**相关 API：**

- `xgeFontFallbackSetMemory`
- `xgeFontFallbackGet`
- `xgeFontFallbackClear`

---

### xgeFontFallbackSetMemory

从内存设置全局 fallback 字体。

**功能：**

你可以在字体数据来自压缩包或内嵌资源时设置全局 fallback。

**函数原型：**

```c
XGE_API int xgeFontFallbackSetMemory(const void* pData, int iSize, float fSize);
```

**参数：**

- `pData`：TTF 数据，不能为 `NULL`。
- `iSize`：数据字节数，必须大于 0。
- `fSize`：字号，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 加载失败返回 `xgeFontLoadMemory` 的错误码。

**资源归属：**

函数会复制输入字体数据。全局 fallback 由 XGE 持有，使用 `xgeFontFallbackClear` 释放。

**补充说明：**

成功后 fallback 字体会带有 `XGE_FONT_FALLBACK` 标志。

**范例代码：**

```c
xgeFontFallbackSetMemory(data, size, 18.0f);
```

**相关 API：**

- `xgeFontFallbackSet`
- `xgeFontFallbackClear`

---

### xgeFontFallbackGet

获取全局 fallback 字体副本。

**功能：**

你可以用它把全局 fallback 字体按指定字号加载到调用者提供的字体对象中。

**函数原型：**

```c
XGE_API int xgeFontFallbackGet(xge_font pFont, float fSize);
```

**参数：**

- `pFont`：输出字体对象，不能为 `NULL`。
- `fSize`：目标字号，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 未设置全局 fallback 返回 `XGE_ERROR_RESOURCE_FAILED`。
- 加载失败返回对应错误码。

**资源归属：**

成功后 `pFont` 是独立字体对象，由调用者使用 `xgeFontFree` 释放。

**补充说明：**

返回字体会带有 `XGE_FONT_FALLBACK` 标志。

**范例代码：**

```c
xge_font_t fallback;
if (xgeFontFallbackGet(&fallback, 20.0f) == XGE_OK) {
    xgeFontFree(&fallback);
}
```

**相关 API：**

- `xgeFontFallbackSet`
- `xgeFontFree`

---

### xgeFontFallbackClear

清除全局 fallback 字体。

**功能：**

你可以用它释放全局 fallback 字体，并让后续 fallback 获取失败。

**函数原型：**

```c
XGE_API void xgeFontFallbackClear(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

释放 XGE 全局状态中持有的 fallback 字体资源。

**补充说明：**

清除不会影响此前通过 `xgeFontFallbackGet` 创建出来的独立字体对象。

**范例代码：**

```c
xgeFontFallbackClear();
```

**相关 API：**

- `xgeFontFallbackSet`
- `xgeFontFallbackGet`

---

### xgeFontGlyphGet

查询字形度量。

**功能：**

你可以用它获取单个 codepoint 的 advance、bearing 和边界信息，用于排版或自定义绘制。

**函数原型：**

```c
XGE_API int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics);
```

**参数：**

- `pFont`：字体对象。
- `iCodepoint`：Unicode codepoint，当前必须小于等于 `0xFFFF`。
- `pMetrics`：输出度量，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 字体未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。

**资源归属：**

函数只写入调用者提供的 `pMetrics`，不分配资源。

**补充说明：**

- 如果 glyph 已在 cache 中，会直接从 cache 返回度量。
- 主字体缺字且设置了对象级 fallback 时，会递归查询 fallback。

**范例代码：**

```c
xge_glyph_metrics_t metrics;
xgeFontGlyphGet(&font, 'A', &metrics);
```

**相关 API：**

- `xgeFontGlyphRasterize`
- `xgeTextMeasure`

---

### xgeFontGlyphRasterize

栅格化单个字形到 CPU 位图。

**功能：**

你可以用它取得 A8 字形位图，用于构建 atlas、调试字体或自定义字体缓存。

**函数原型：**

```c
XGE_API int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap);
```

**参数：**

- `pFont`：字体对象。
- `iCodepoint`：Unicode codepoint，当前必须小于等于 `0xFFFF`。
- `pBitmap`：输出位图，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 字体未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 位图内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

成功后 `pBitmap->pPixels` 由字体后端分配，调用者必须使用 `xgeGlyphBitmapFree` 释放。

**补充说明：**

- 当前输出格式为 `XGE_PIXEL_A8`。
- 主字体缺字且设置了对象级 fallback 时，会使用 fallback 栅格化。

**范例代码：**

```c
xge_glyph_bitmap_t bitmap;
if (xgeFontGlyphRasterize(&font, 0x4E2D, &bitmap) == XGE_OK) {
    xgeGlyphBitmapFree(&bitmap);
}
```

**相关 API：**

- `xgeGlyphBitmapFree`
- `xgeFontGlyphAtlasGet`

---

### xgeFontGlyphAtlasGet

获取或创建 atlas 中的字形。

**功能：**

你可以用它确保某个 codepoint 已经进入字体 atlas，并取得 atlas 页、坐标、尺寸和 advance。

**函数原型：**

```c
XGE_API int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph);
```

**参数：**

- `pFont`：字体对象。
- `iCodepoint`：Unicode codepoint，当前必须小于等于 `0xFFFF`。
- `pGlyph`：输出 atlas 字形，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 字体未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- atlas 分配失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

返回的是字形信息副本。atlas 页资源由字体对象持有，并随 `xgeFontFree` 释放。

**补充说明：**

- 已缓存字形会直接返回。
- 空白字形可能没有实际 page，但仍可返回 advance。
- 文本绘制内部会调用该函数。

**范例代码：**

```c
xge_glyph_t glyph;
xgeFontGlyphAtlasGet(&font, 'A', &glyph);
```

**相关 API：**

- `xgeTextDraw`
- `xgeFontBuildXRFMemory`

---

### xgeGlyphBitmapFree

释放 glyph bitmap。

**功能：**

你可以用它释放 `xgeFontGlyphRasterize` 返回的 CPU 字形位图。

**函数原型：**

```c
XGE_API void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap);
```

**参数：**

- `pBitmap`：glyph bitmap，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pBitmap->pPixels` 并清零结构体。

**补充说明：**

对 `NULL` 调用是安全的。

**范例代码：**

```c
xgeGlyphBitmapFree(&bitmap);
```

**相关 API：**

- `xgeFontGlyphRasterize`

---

### xgeTextMeasure

测量 UTF-8 文本尺寸。

**功能：**

你可以用它获取多行文本在当前字体下的宽高，用于 UI 排版、按钮自适应或文本对齐。

**函数原型：**

```c
XGE_API xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText);
```

**参数：**

- `pFont`：字体对象。
- `sText`：UTF-8 文本。

**返回值：**

- 返回文本尺寸。
- 参数非法、文本为空或测量失败时返回 `{0, 0}` 或部分已测量结果。

**资源归属：**

函数不分配调用者需要释放的资源。

**补充说明：**

- 换行符会增加一行 `fLineHeight`。
- 当前测量按 glyph advance 累加，不做复杂 shaping。

**范例代码：**

```c
xge_vec2_t size = xgeTextMeasure(&font, "Hello\nXGE");
```

**相关 API：**

- `xgeTextDrawRect`
- `xgeFontGlyphGet`

---

### xgeTextDraw

在指定坐标绘制文本。

**功能：**

你可以用它绘制基础多行 UTF-8 文本。

**函数原型：**

```c
XGE_API void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor);
```

**参数：**

- `pFont`：字体对象。
- `sText`：UTF-8 文本。
- `fX`：起始 X 坐标。
- `fY`：起始 Y 坐标。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。字体必须在绘制期间保持有效。

**补充说明：**

- 绘制过程中会按需把 glyph 放入 atlas 并上传 atlas 页。
- 缺失或无法栅格化的字符会被跳过。

**范例代码：**

```c
xgeTextDraw(&font, "Hello XGE", 32.0f, 32.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeTextDrawRect`
- `xgeTextMeasure`

---

### xgeTextDrawRect

在矩形内绘制文本。

**功能：**

你可以用它在 UI 控件或布局区域中按水平/垂直对齐方式绘制文本，并可选择裁剪到矩形。

**函数原型：**

```c
XGE_API void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags);
```

**参数：**

- `pFont`：字体对象。
- `sText`：UTF-8 文本。
- `tRect`：目标矩形，宽高必须大于 0。
- `iColor`：文本颜色。
- `iFlags`：`XGE_TEXT_ALIGN_*` 和 `XGE_TEXT_CLIP` 组合。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

- 水平支持 left/center/right。
- 垂直支持 top/middle/bottom。
- 设置 `XGE_TEXT_CLIP` 时，会临时启用 clip，绘制后清除。

**范例代码：**

```c
xge_rect_t rect = xgeRect(0.0f, 0.0f, 320.0f, 48.0f);
xgeTextDrawRect(&font, "Start", rect, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
```

**相关 API：**

- `xgeTextMeasure`
- `xgeClipSet`
- `xgeClipClear`

---

## 生命周期与所有权

`xge_font_t` 由调用者持有，使用 `xgeFontLoad`、`xgeFontLoadMemory`、`xgeFontLoadXRF` 或缓存接口初始化，使用 `xgeFontFree` 释放。

对象级 fallback 只保存借用指针，不改变调用者对字体对象的生命周期责任。`xgeFontAddRef` 后需要对应 `xgeFontFree`。

`xgeFontBuildXRFMemory` 返回的内存由 XGE 通过 `xrtMalloc` 分配，调用者使用后应使用 `xrtFree` 释放。

## 线程约束

TTF 文件读取、XRF 文件读取和部分 glyph 准备可以在资源线程执行。Glyph atlas 上传和文本绘制必须在渲染 owner 线程执行或通过命令队列提交。

不要在多个线程同时修改同一个 font 对象的 fallback、atlas 或 cache 状态。

## 后端差异

字体栅格化主要是 CPU 侧行为，但最终绘制依赖纹理 atlas 和 blend。不同后端的纹理尺寸限制会影响 atlas 策略。

第一版支持中文 UCS2 范围，emoji 不作为 MVP 承诺。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 中文显示为空 | 当前字体不含字形且没有 fallback | 设置包含中文的 fallback font。 |
| 文本首次绘制卡顿 | TTF 字形运行时栅格化 | 使用 XRF 或预热常用字形。 |
| glyph bitmap 泄漏 | 没调用 `xgeGlyphBitmapFree` | 栅格化后配对释放。 |

## 相关示例

- `examples/text`
- `examples\text\build.bat`
