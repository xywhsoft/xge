# 文字渲染底座主线合并说明

## 当前主线状态

补丁已人工合并到当前 XGE 主线。合并没有覆盖主线的 ShapeEx、SVG 和 XUI
实现，而是按下列边界接入：

- `xge_font.c` 负责 FontFace、FontFamily、字体实例、字形缓存和 XRF；
- `xge_text_run.c` 负责基础 shaping、GlyphRun、命中测试和文字装饰；
- `xge_text_vector.c` 负责复用 FontFace 轮廓缓存并向 ShapeEx 输出字形路径；
- SVG 文字通过 GlyphRun 和 ShapeEx 使用同一份字形轮廓，不再访问字体私有 atlas；
- XUI 字体代理通过 `xgeFontGetMetrics` 获取指标，不再解释字体内部结构；
- CodeEdit 取消绘制 span 的 511 字节截断，长行不再因为临时栈缓冲而缺字。

本页后续的开发基线、冲突热点和推荐顺序用于记录补丁来源及人工合并过程，
不表示这些内容仍处于待合并状态。

## 开发基线

- 功能分支：`codex/text-render-foundation-2b3fd72`
- 基线提交：`2b3fd7221bb838bb3ba546e11c7540865fd7f797`
- 开发原因：主线正在进行 ShapeEx 与 SVG 更新，文字底座必须在已验证旧节点独立开发。

合并时不要直接合并整个工作树。旧节点还包含 MessageList、XRT、工具和构建产物的其他未提交修改，它们不属于文字底座。

## 应合并文件

| 文件 | 方式 | 说明 |
|---|---|---|
| `src/xge_text_run.c` | 直接新增 | GlyphRun、基础 shaping、命中和装饰绘制 |
| `test/test_text_foundation.c` | 直接新增 | CPU 字体专项测试 |
| `test/build_text_foundation_test.bat` | 直接新增 | 专项测试构建入口 |
| `docs/text-render-foundation.md` | 直接新增 | 架构和 API 语义 |
| `docs/text-render-foundation-merge.md` | 直接新增 | 本文档 |
| `src/xge_font.c` | 优先整体应用，再人工处理冲突 | FontFace、哈希缓存、Unicode、atlas、XRF v2 |
| `xge.h` | 人工合并文字相关块 | 不覆盖主线 ShapeEx/SVG 声明 |
| `src/xge_impl.c` | 人工添加一行 | 在 `xge_font.c` 后包含 `xge_text_run.c` |

## 不应合并文件

以下旧工作树内容与文字底座无关：

- `lib/xrt/xrt.h`；
- `xui.h`；
- `src/xui_message_list.c`；
- `docs/xui/widget-messagelist.md`；
- `examples/xui_messagelist/main.c`；
- `test_xui/xui_message_list_test.c`；
- `build/` 下所有 DLL、LIB、EXE、OBJ 和日志；
- `dev/xui2/`、UIDesign、SkeletonEdit 等未跟踪目录。

实现验收后应创建一个路径受限的独立提交，再生成补丁或 cherry-pick。不要使用 `git add -A`：

```powershell
git add -- `
  xge.h src/xge_font.c src/xge_text_run.c src/xge_impl.c `
  test/test_text_foundation.c test/build_text_foundation_test.bat `
  docs/text-render-foundation.md docs/text-render-foundation-merge.md
git commit -m "升级 XGE 文字渲染底座"
git format-patch -1 --stdout > text-render-foundation.patch
```

## 冲突热点

### xge.h

只合并以下内容：

- font weight/slant/stretch 常量；
- decoration 和 shaping 常量；
- XRF version 2 定义；
- FontFace、FontFamily、metrics、cache stats、GlyphRun、decoration 结构；
- 对应 `XGE_API` 声明。

不要用旧文件覆盖主线新增的 ShapeEx、SVG 或其他结构。

### src/xge_impl.c

在主线最终的源文件包含区中保证顺序：

```c
#include "xge_font.c"
#include "xge_text_run.c"
```

`xge_text_run.c` 调用公开 Shape Line API 绘制装饰。如果主线 ShapeEx 删除了旧的 `xgeShapeLine`/`xgeShapeLinePx`，只需要在该文件的 decoration helper 中适配新 API，不要把旧 `xge_shape.c` 合回主线。

### src/xge_font.c

必须保留以下不变量：

- `xge_font_t` 公开尺寸不变化；
- `pBackend` 指向 instance backend，不能再强制转换为 `stbtt_fontinfo*`；
- FontFace 独占字体数据，FontInstance 引用 face；
- glyph cache 使用 hash store，不恢复链表查找；
- atlas 更新使用 `xgeTextureUpdateRGBA`，不销毁并重建稳定纹理；
- XRF v1 可读、v2 可读写；
- XRF + TTF 使用 hybrid miss 路径；
- UTF-8 最大值为 `U+10FFFF`。

如果主线也修改了字体实现，应按以上不变量人工移植，不建议直接选择任一侧完整文件。

## 推荐合并顺序

1. 将新文档、专项测试和 `src/xge_text_run.c` 加入主线。
2. 将 `xge.h` 中的文字常量、类型和 API 分块移植。
3. 移植 `src/xge_font.c` 的 FontFace 与 cache store。
4. 移植 Unicode、glyph-by-index、metrics 和 XRF v2。
5. 移植 atlas dirty rectangle 更新。
6. 在 `src/xge_impl.c` 加入新文件。
7. 根据主线 ShapeEx API 调整 decoration helper。
8. 编译并运行验证矩阵。

## 验证矩阵

```powershell
cmd /c build_dll.bat
cmd /c test\build_text_foundation_test.bat
cmd /c build_test.bat
cmd /c test_xui\build_proxy_xge_test.bat
cmd /c test_xui\build_text_test.bat
cmd /c test_xui\build_code_layout_test.bat
cmd /c test_xui\build_code_edit_test.bat
```

需要额外人工验证：

- 同一帧首次绘制多个新字符时纹理不闪烁；
- XRF 缓存之外的字符能够由 TTF 动态补充；
- Regular/Bold/Italic/BoldItalic 选择真实 face；
- 4 字节 Unicode、中文 fallback、混合字号基线；
- underline、strike 和 squiggle 在 DPI 缩放下位置稳定。

## 后续并行开发规则

主线大规模重构期间，其他 XGE 子系统更新也采用相同方式：

1. 从明确提交建立命名功能分支，并在文档记录完整基线 hash。
2. 新能力优先放入新源文件，公共聚合文件只增加最小 include。
3. 不修改并行主线正在重构的文件；无法避免时记录语义不变量和人工合并点。
4. 测试使用新文件，不向主线热点测试文件追加大段代码。
5. 构建产物不作为功能补丁的一部分。
6. 合并文档必须列出应合并、不应合并、冲突热点、顺序和验证矩阵。
