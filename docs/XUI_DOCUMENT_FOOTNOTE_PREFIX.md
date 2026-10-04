# Document 脚注定义值、引用前缀与增量缓存

更新：2026-10-04。对应整体重构 K2/E1/K4。统一 Document、事务和历史不变，没有新增公开 API 或旧接口兼容层。整体目标仍在进行。

本批从当前源码重新复现：引用包裹的选区内部含未使用脚注定义时，命令返回 `UNREPRESENTABLE (-107)`，源码、树、来源语法、revision 和历史保持原样。现在完整脚注定义可在已验证的根层、已有引用、列表项和引用中的任务列表项前缀路径内随选区增加引用层。部分定义和无法证明等价的其他结构仍保守拒绝或走既有受检路径。

## 实现

正常解析为每一条脚注定义采集标签和正文，包括未引用、重复和空正文定义。正文起点直接复用 MD4C 实际脚注正文分析的逐行去缩进函数，保留去除容器前缀后的 Markdown 标记、额外缩进、实体、尾部空白和原 LF/CR/CRLF；不以可见语义树代替未使用定义的内容。临时正文缓冲只在解析回调期间借用，正常退出、取消和分配失败均释放；较长读取与复制按 16 KiB 检查取消。

脚注与链接定义使用同一不可变、有序、带种类的值序列。链接原有标签/目标/标题/标题存在性字段不变，脚注保存标签/正文，不再是空占位。快照与历史共享这些派生值。已使用及未使用的独立脚注正文增量路径各替换一个 payload，完整加载、局部增量、Prepare 和连续 Prepare 的独立缓存对照验证一致。

引用前缀适配器继续逐行复制非前缀原字节，私有候选必须完整重解析，并同时通过目标语义树和全部有序定义值核对。该核对包含不可见的脚注正文、未使用链接、重复定义及定义顺序。列表续行的 `>` 插入点改为解析器确认的内容列，额外缩进仍属于脚注或代码正文；已有引用和列表层共享的原始缩进范围按真实物理列对齐。选中最后一个物理行时，仅在实际 SourceStore EOF 允许没有末尾换行。

另外修复当前可选 WebView 构建器的 SDK 参数：使用正斜杠，避免 GCC Make 依赖文件把 Windows 反斜杠误作字符转义。Windows 基础网页承载的公开范围没有扩大，Document 的私有公式、Mermaid、HTML 渲染及独立 HTML 交互保留。

## 维护测试

新增 `test_xui/xui_document_footnote_prefix_test.c`，Windows 独立 Core 和实际 DLL/Editor 入口为 `build_document_footnote_prefix_test.bat`；已加入完整发布套件和 Linux Core sanitizer。

- 12 类来源 × 正反向 × 4 种 TEXT/GAP 端点 × 3 种 EOL，共 288 个黄金源码场景。包括根层、已有引用、双层引用、列表、任务列表、已使用/未使用/重复脚注、正文里的链接定义、转义/实体、多行块正文、空头部、代码及无末尾换行。
- 24 个复合事务：12 个 Markdown 与 12 个 Rich，各连续包裹五次，复用返回选区，一次发布和共同 Undo/Redo。
- 10 个深度场景：1/8/32/64/124 层、正反向。
- 12 组 OOM 共 5,050 点，5 组取消共 2,207 点；核对失败时树、源码、来源语法、revision、历史和存活分配，最终释放为零。
- 96 个实际 DLL Editor 场景：Query/CanExecute/Execute、结果选区、完整重载、SOURCE/LIVE/VISUAL 切换和共同历史。
- Core 缓存专项另以 9 组字面正文和 11 组等价/不等价配对独立核对 payload；快照在 Document 释放后仍可读取。包含 Tab、懒续行、实体、空正文、混合 EOL、额外代码缩进、定义顺序及重复定义正文变化。

黄金源码生成只使用 fixture 预先声明的前缀和字面 `> `，不调用生产写回器或解析器。缓存 oracle 直接读取种类、尺寸和字节，不借用生产相等函数。

## 本批验证

证据前缀为 `artifacts/xui-document-rebuild/validation-2026-10-04-footnote-prefix-`。初始拒绝、编译和中间失败日志保留，未覆盖 2026-10-03 的历史封存包。

Windows 完整发布套件、Core 专项与上一批引用回归、Linux Core/Renderer/View ASan/UBSan、5 份修改源码的 GCC analyzer、未插桩 MD4C 严格编译，以及可选 WebView 测试/生产 DLL 构建均实际退出 0。完整 Windows 套件包含实际 DLL、Editor、MessageList 和原生 GPU 绘制。

652 项 CommonMark 0.31.2 语料通过解析、原始来源保留、原生往返和局部/完整解析差分；这不等于 Markdown→HTML conformance。Linux UI 为无窗口代理，不能替代真实 Linux XGE 窗口、GPU、IME 和读屏验收。

5 项浏览器串行程序实际退出 0：基础 WebView、Document provider、离线静态渲染、截图请求、独立 HTML 交互。三张新的 500×350 PNG 已实际查看，分别是分式、Start→Finish 图和带样式 HTML。生产 DLL 的 20 项基础/provider 导出存在、14 项内部浏览器符号不导出；Document 发布审计确认 284 个公开函数且无旧富文本 API。

最终逐项实际退出记录、源码守护指纹和 DLL 指纹见同前缀 `checks-final.json`、`sources-final.json`、`hashes-final.json`、`scope-final.json`。历史首次失败记录单列，不能计为通过。

## 未完成范围

本批只补确认前缀路径的脚注内容证明。完整 CST/token/trivia、其他命令的隐藏定义保护、复杂结构无损回写、异步/三模式完整矩阵仍未完成。列表内容列落在一个 Tab 字节内部时没有直接的原字节插入点，本适配器返回 NOT_FOUND，再由既有通用路径受检处理，不能宣称全部 Tab 结构编辑已支持。

124 层正确性及当前机器的专项结果不能替代固定机器性能门槛。真实字体、复杂 shaping/Bidi、输入法、读屏、DPI/主题、其他声明平台和剩余迁移继续按原 13 包追踪。完整余项见 [剩余任务](XUI_DOCUMENT_REMAINING.md)。

2026-10-04 后续更新：上述 Tab 内部内容列的引用前缀限制已由 [Tab 逻辑列批次](XUI_DOCUMENT_TAB_PREFIX.md) 补齐；原脚注批次的记录继续保留。其余结构命令和完整 CST 仍在进行。
