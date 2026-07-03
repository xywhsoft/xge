# Font Text Lab

`examples/font_text_lab` 是字体和文本 API 的覆盖型范例，用来验证 TTF/TTC 文件加载、内存加载、XRF 缓存、fallback、glyph 度量/栅格化/atlas，以及矩形文本绘制的对齐和裁剪。

## 覆盖 API

- `xgeTextUTF8Next`
- `xgeFontLoad`
- `xgeFontLoadMemory`
- `xgeFontLoadXRF`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`
- `xgeFontLoadCached`
- `xgeFontAddRef`
- `xgeFontFree`
- `xgeFontSetFallback`
- `xgeFontFallbackSet`
- `xgeFontFallbackSetMemory`
- `xgeFontFallbackGet`
- `xgeFontFallbackClear`
- `xgeFontGlyphGet`
- `xgeFontGlyphRasterize`
- `xgeFontGlyphAtlasGet`
- `xgeGlyphBitmapFree`
- `xgeTextMeasure`
- `xgeTextDraw`
- `xgeTextDrawRect`

## 构建和运行

```bat
examples\font_text_lab\build.bat
build\xge_font_text_lab.exe --frames 120
```

可用 `--font <path>` 指定字体文件。默认会优先尝试 Windows 系统字体；如果当前平台找不到可用字体，程序会输出 skip 信息并退出。

## 验证点

- 控制台输出 `font-text-lab init`，其中 `xrf_memory=1`、`xrf_file=1`、`cached=1` 表示 XRF 内存、文件和缓存路径都已经覆盖。
- 控制台输出 `fallback=1/1/1` 表示文件 fallback、fallback 拷贝和内存 fallback 都已经覆盖。
- 窗口中能看到主字体、内存字体、XRF 字体和 cached 字体的文本块。
- 下方两个矩形文本块分别覆盖居中、右下对齐和 clip。
- 程序运行时会在可执行文件目录生成 `font_text_lab_ascii.xrf` 和 `font_text_lab_cached.xrf`，它们是运行期缓存产物，不需要提交。
