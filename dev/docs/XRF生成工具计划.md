# XRF 生成工具计划

XRF 是 XGE 的快速 bitmap font/cache 格式，用于把 TTF 动态栅格化的成本提前到工具链阶段。

## 目标

- 输入 TTF/TTC 字体文件。
- 输入 glyph 范围，第一版重点支持 UCS2。
- 按指定像素字号栅格化 glyph。
- 打包到 A8 或 RGBA8 atlas pages。
- 输出 XRF binary。
- XRF 内部不做压缩，压缩交给 xpack 或外层资源系统。

## 命令形式

```text
xge_xrf_gen --font path/to/font.ttf --size 32 --range 0x20-0x7E --range 0x4E00-0x9FFF --format A8 --out font.xrf
```

## 第一版参数

- `--font`：输入字体。
- `--size`：像素字号。
- `--range`：可重复指定 UCS2 范围。
- `--chars`：显式字符列表文件，UTF-8 编码。
- `--page`：atlas page 尺寸，默认 512。
- `--format`：`A8` 或 `RGBA8`，默认 `A8`。
- `--out`：输出路径。

## 生成流程

1. 使用 stb_truetype 读取 TTF/TTC。
2. 展开 range/chars 为 codepoint 集合。
3. 对每个 codepoint 计算 metrics。
4. 栅格化 glyph bitmap。
5. 使用与运行时兼容的 row packing 生成 atlas pages。
6. 写入 `xge_xrf_header_t`。
7. 写入 `xge_xrf_range_t[]`。
8. 写入 `xge_xrf_glyph_t[]`。
9. 写入 `xge_xrf_page_t[]`。
10. 写入可选 `xge_xrf_kerning_t[]`。
11. 写入 pixel data。

## 后续扩展

- 支持多字号合并。
- 支持 fallback 字体合并。
- 支持生成预览图。
- 支持增量缓存。
- 支持 SDF/MSDF 字体页。
