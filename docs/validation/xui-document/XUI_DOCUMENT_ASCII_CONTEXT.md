# Document ASCII 段落的脚本与上下文

更新：2026-10-02。这是整体重构 R1 正确性的一项修复；ABI16、原生格式7保持。

## 错误与修复

纯 ASCII 段落原先没有脚本图，Document 也因此不传 `sContext`。当 `(a)` 的三个文本节点使用不同字体时，括号成为独立 item，无法从自身推断 Latin。具有不同 DFLT/latn OpenType 规则的字体会选错字形，括号 advance 从12变成8，段落总宽度从48变成40。Rich 与 Markdown 都可通过公开接口复现。

现在上下文与脚本图是否存在分开处理。Document 按代理能力传递完整显示上下文、当前 item 的字节偏移、语言和解析后的脚本。多个来源片段的 ASCII 段落使用同一 Script_Extensions 解析器缓存脚本图；单个 ASCII 片段继续省略脚本图分配。原生 XGE 的自动脚本入口也会在纯 ASCII 子 item 上解析所给完整上下文，解决独立调用的标点脚本归属。

必需的换行仍重置脚本，只有标点的上下文保持 Common。显式脚本仍优先，缺少上下文/脚本能力的代理仍省略派生输入。ASCII 显示文本中的 SHY 插入上下文也有保留的可复用缓冲区，不再依赖脚本图存在；单次插入借用不会修改基础文本。

## 验证与边界

独立的 `xge_ascii_script_fixture.ttf` 明确设置不同 DFLT/latn 规则，以字面 PUA 字形提供图像参照。生成器固定时间戳，原有字体不改。公开无窗口复现核对 XGE glyph7/advance12、Rich 和 Markdown 括号12/总宽48；旧 DLL 的相同复现得到 glyph5/advance8、总宽40。

原生108次完整 RGBA 对比覆盖40/37字号、Rich跨字体与跨字号、Markdown粗体使用较小字体、六种缓存/基础范围/仅drawText/无持久paint配置，以及caret/hit、改宽、字体失效和释放Document后的Snapshot。参照图只绘制字面PUA，不用被测上下文生成参照。

工厂故障注入覆盖ASCII多片段新增脚本图分配、失败时不发布输出、原上下文不变和零分配reflow。Linux使用实际C脚本/回退入口核对Latin、换行重置、Common、脚本图OOM重试与缓存。最终发布、真实memory-debug及可选WebView回归的终态以同前缀 `validation-2026-10-02-ascii-script-final-state.json` 为准。

本次传递完整上下文不等于将不同字体的整个段落放入同一个glyph buffer。跨字体的任意GSUB前后向替换、跨样式整词字体选择、语言itemization和完整性能/原生平台矩阵仍是剩余工作。通用WebView保持Windows基础承载和其他平台预留，Document私有公式/Mermaid/HTML渲染通道保留。

整词边界另有本ASCII批次封存DLL的公开失败探针：主字体缺少 `i`，fallback覆盖完整 `ffi`，只塑形首个 `f` 并给出完整 `ffi` 上下文时，实际仍选部分主字体。探针源为 `build/ascii-script-final/word-context-audit.c`，日志 `validation-2026-10-02-ascii-script-word-context-remaining.log` 实际退出1。它说明当前选择单元仍受item边界限制，单独作为剩余项，不能计入本批通过门槛。

后续跨item整词选择的实现与独立新批次验证见 [Document整词字体回退](XUI_DOCUMENT_WORD_CONTEXT.md)。此处保留ASCII批次的历史失败与验收边界。
