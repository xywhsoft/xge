# Document 引用编辑的 Tab 逻辑列与来源保护

更新：2026-10-04。对应整体重构 K2/E1，统一 Document、SourceStore、快照和事务历史不变。整体目标仍在进行。

## 问题与改动

当前源码的先失败证据表明：列表内容列落在一个 Tab 字节内部时，原前缀适配器返回 NOT_FOUND；部分场景随后走整列表写回，连选区外的 `&amp;` 也变为 `\&`，空行和缩进被重新生成。不是所有案例都会直接返回命令错误，核心缺陷是缺少安全、局部的来源编辑路径。

现在从解析器确认的列表续行缩进计算逻辑内容边界。边界在 Tab 内部时，仅把该 Tab 拆成边界两侧实际占用的空格，再插入引用标记；完整边界前后的原始字节继续复制，未选列表内容和兄弟容器保持原样。

引用标记还会改变后续 Tab 的物理列位置。把所有后续 Tab 展开成空格会改变未使用脚注中保留的代码正文，因此本批保留这些 Tab：选中范围在内容边界后还有缩进 Tab 时，所有新引用标记前加两个合法的引用缩进空格，令新增前缀占四列，原 Tab 停点不变。统一范围策略也保证脚注头部与正文使用一致的原点。

已有 Quote 内再次包裹及根层引用包裹采用同样的四列策略，修复连续命令对脚注代码正文的二次影响。普通无缩进 Tab 的路径保留原两列前缀。推断的空行与懒续行前缀使用同一内容边界。长来源扫描每 16 KiB 检查取消，未建立另一份可写正文或历史。

所有候选仍完整重解析，并核对目标语义树与全部有序定义值，包括未使用、重复和代码正文脚注。该策略不靠放宽语义或丢弃来源核对来接受编辑。

## 维护测试与边界

新入口为 `test_xui/xui_document_tab_prefix_test.c`，Windows 独立 Core 和实际 DLL/Editor 构建脚本为 `build_document_tab_prefix_test.bat`；已加入完整发布 suite 与 Linux Core sanitizer。

- 17 组字面输入和字面黄金源码 × 正反向 × 4 种 TEXT/GAP × 3 种 EOL，共 408 场景。覆盖 Tab 内第 1/2/3 列、完整 Tab 边界、有序／任务／嵌套／引用列表、隐藏链接、已使用和未使用脚注、脚注代码正文、普通代码、Tab 空行、首子块、懒续行、根层与已有 Quote。BOM 和混合 EOL 保留原行的换行；根层新增边界行采用原选中末行 EOL。
- 17 个连续五次包裹事务，复用返回选区、一次发布及一次 Undo/Redo，独立全文重载并比较来源语法和定义缓存。
- 专用脚注 fixture 的 1/8/32/64/123 层通过；第 124 层因脚注及列表子树的累计深度触达既有 `DOC_MAX_DEPTH=128` 而返回 LIMIT，已发布源码、revision 和历史保持不变。此 fixture 的层数不能等同于节点树深度，也不能据此放宽 schema 限制。
- 17 个单命令及 5 个双命令 OOM 扫描共 20,919 点；6 组分配触发取消扫描共 4,019 点。每个失败核对旧源码、树、来源语法、revision、历史和存活分配，最终成功重试、释放为零。
- 136 个实际 DLL Editor 场景覆盖 Query/CanExecute/Execute、结果选区、SOURCE/LIVE/VISUAL 和共同 Undo/Redo。

黄金源码独立写定，不调用生产适配器。定义缓存 oracle 直接核对种类、尺寸和字节；不调用生产相等函数。

## 验收范围

本批使用独立证据前缀 `artifacts/xui-document-rebuild/validation-2026-10-04-tab-prefix-`。当前源码、维护测试和公开头指纹在运行前冻结；终态以实际命令退出记录为准。Windows 完整发布、Linux Core/Renderer/View ASan/UBSan、修改模块 GCC analyzer、可选 WebView 测试/生产构建和 5 项浏览器串行程序均实际退出 0。生产 DLL 的 284 项 Document API 审计与 20 项基础/provider 导出、14 项私有符号不导出的边界检查通过。三张新公式、Mermaid、HTML 截图已实际查看。652 CommonMark 为解析/来源/原生往返和增量差分，不是 HTML conformance；Linux UI 是无窗口代理，不代替真实后端、IME、读屏和 DPI 验收。完整终态见本前缀 checks-final.json、sources-final.json、hashes-final.json 与 scope-final.json。

本批补引用前缀编辑的 Tab 来源边界，不代表所有结构命令的 Tab 处理或完整 CST/token/trivia 已完成。其他复杂结构、异步/三模式矩阵、性能和真实平台等仍按原 13 个工作包追踪。通用 WebView 仍仅 Windows 基础网页承载，Document 私有公式、Mermaid、HTML 与独立 HTML 交互保留。
