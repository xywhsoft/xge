# Document 重构实施与验证记录

日期：2026-10-02。平台：本机 Windows x64，GCC 16.1.0，实际 XGE DLL 与 GPU 后端。

首次可撤销大文档编辑计费验证：`xui_document_scale_test.c` 清空 10,000 段初始导入历史后，在实际 DLL Editor 首次插入一个字节，要求新历史独占、LiveBytes 增量均小于原 CurrentBytes 的 1%，新增分配少于 1,000 次，并继续验证 Undo/Redo 与可见区规模测试。实测 20,001 节点、原 CurrentBytes 12,336,504、首个历史步骤 1,592 字节、LiveBytes 增量 1,962 字节、19 次新增分配；独立 Core 的相同规模直接事务测得 18 次新增分配和相同历史/存活增量，并在 Undo/Redo 后核对文本。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-first-undo-full-suite.log`、Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-30-first-undo-linux-core.log` 均退出码零；专项实际 DLL 规模日志 `validation-2026-09-30-first-undo-scale-final2.log` 亦通过。这是存储与分配数量的规模门禁；CPU 时间尚未建立固定测试机 P95 门槛，真实 GPU/IME 响应与独立绘制参考仍需验收。

Markdown 表格列级来源验证：`artifacts/xui-document-rebuild/validation-2026-09-30-table-column-source-before.log` 先证实整表写回会把未操作列的 `:--:` 改成 `:---:`。`markdown_table_column_source_preservation` 要求完整竖线边界的根层 GFM 表格在中间插入、末尾追加、首列/末列删除后，仅目标列源码改变；表头转义 `\\|`、正文代码跨度内的 `|`、原对齐符、CRLF/LF、无末尾换行及表格外 `&amp;` 保持原字节，Undo 恢复全文。私有候选重解析并核对完整语义和链接定义拼写。`markdown_table_column_source_failures` 的插入 539 点、删除 319 点分配失败扫描验证失败时旧 revision、源码、树和历史不变且无泄漏。Windows 完整套件 `validation-2026-09-30-table-column-source-full-suite.log` 包含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-30-table-column-source-linux-core.log` 均退出码零。严格 GCC 静态分析 `validation-2026-09-30-table-column-source-analyzer.log` 为空。未覆盖的表格源码形态经现有通用写回处理，完整 CST 仍待实现。

Markdown 表格数据行局部来源验证：修复前 `artifacts/xui-document-rebuild/validation-2026-09-30-table-row-source-before.log` 复现整表写回将对齐分隔线 `:--:` 改为 `:---:`。`markdown_table_row_source_preservation` 现逐字节核对根层 GFM 表格的行中插入、表尾追加和数据行删除：转义表头、代码跨度内竖线、CRLF、选区外 `&amp;`、LF 无外侧竖线、无末尾换行及 Undo/Redo 均保留原字节；候选重新解析并核对目标语义与定义原文。`markdown_table_row_source_failures` 的插入 497 点、删除 320 点分配失败扫描分别验证失败时源码、树、历史、revision 不变且分配归零。独立 Core `validation-2026-09-30-table-row-source-core-final.log`、Windows 完整套件 `validation-2026-09-30-table-row-source-full-suite.log`、Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-30-table-row-source-linux-core.log` 均退出码零；Windows 套件包含实际 DLL、Renderer/Editor、MessageList 和原生绘制。修改源码的 GCC `-fanalyzer -Wall -Wextra -Werror` 日志 `validation-2026-09-30-table-row-source-analyzer.log` 为空，`git diff --check` 通过。仅根层普通数据行进入此保真路径，表头、列、嵌套表格和完整 CST 仍待后续工作。

原生属性数值边界验证：`test_xui/xui_document_test.c` 新增四个字段的事务拒绝及 1,000,000 边界值原生往返。修复前 `artifacts/xui-document-rebuild/validation-2026-09-30-native-attribute-bounds-before.log` 在第一项超限事务断言失败，证明可写入无法读回的数据；修复后 Core `validation-2026-09-30-native-attribute-bounds-core.log`、Windows 完整套件 `validation-2026-09-30-native-attribute-bounds-full-suite.log`、Linux Core ASan/UBSan 与 652 项 CommonMark 语料 `validation-2026-09-30-native-attribute-bounds-linux-core.log` 均退出码零。Windows 全量还覆盖正式 DLL、Renderer/Editor、MessageList 和原生绘制。四个修改源码的 GCC `-Wall -Wextra -Werror -fanalyzer -fsyntax-only` 日志 `validation-2026-09-30-native-attribute-bounds-analyzer.log` 为空，差异检查通过。该批未涉及完整 Markdown CST、Bidi、真实 IME/读屏或多平台窗口后端。

缩进代码列界补测：列表内两枚 Tab 的第二枚跨越第 6 逻辑列时，CodeBlock 记录原始两字节范围、代码缩进 `[2,6)` 和完整缩进终点 8；新增用例后的 Windows Core `artifacts/xui-document-rebuild/validation-2026-09-29-code-indent-tab-inside-core.log` 退出码零。产品源码未因该补测改变，完整 Windows/Linux 套件采用同一实现。

同批可选 Windows WebView2 基础控件、Document 内部 provider/MessageList 与离屏 KaTeX/Mermaid/HTML 渲染分别见 `artifacts/xui-document-rebuild/validation-2026-09-29-code-indent-webview-build.log`、`validation-2026-09-29-code-indent-web-provider.log`、`validation-2026-09-29-code-indent-web-render.log`，三项退出码零；通用 WebView 对外仍限 Windows 基础网页功能。

Markdown 缩进代码逐行来源验证：`markdown_indented_code_syntax` 逐字节核对非围栏代码块的四列缩进、Tab、CRLF、内部空行及引用/列表嵌套，测试源码首行四空格改 Tab 后旧/新快照和 Undo；2 万行样本核对最后一条记录。73 点解析/Document 分配失败扫描保持提交原子且释放后无泄漏。`prepare_tree_equal` 对 indented CodeBlock 的每条记录进行局部准备/完整解析差分，独立 Windows 语料 `artifacts/xui-document-rebuild/validation-2026-09-29-code-indent-corpus-first.log` 覆盖 652 项 CommonMark，退出码零。完整 Windows 套件 `validation-2026-09-29-code-indent-final-suite.log` 退出码零，含 Core、实际 DLL/新增导出、Renderer/Editor、MessageList 与原生绘制。Linux Core ASan/UBSan/同一语料 `validation-2026-09-29-code-indent-linux-core.log`、无窗口 Renderer `validation-2026-09-29-code-indent-linux-renderer.log` 和 View/Editor `validation-2026-09-29-code-indent-linux-view.log` 均退出码零；View 编译仅有未修改 DatePicker 的可能未初始化告警。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror`、未打补丁 MD4C 严格编译、API 注释覆盖和 lint 154/154 均通过。该接口只声明解析器确认的缩进代码来源，不等于完整块内 CST 或复杂结构视觉编辑完成。

同批可选 Windows WebView2 基础控件、Document 内部 provider/MessageList 与离屏 KaTeX/Mermaid/HTML 渲染复测分别见 `artifacts/xui-document-rebuild/validation-2026-09-29-list-columns-webview-build.log`、`validation-2026-09-29-list-columns-web-provider.log`、`validation-2026-09-29-list-columns-web-render.log`，三项退出码零；通用 WebView 对外范围仍是 Windows 基础版。

Markdown 列表逐层 Tab 列边界验证：MD4C 的最终列表续行判定现回传每层原始空白范围和 `iIndentStartColumn/iContentColumn/iIndentEndColumn`，按四列 Tab 停靠表示可落在同一 Tab 字节内部的不同内容边界。`markdown_list_continuation_indents` 核对嵌套列表共享 Tab 的外层 `[0,2)` 与内层 `[2,4)`、引用前缀后的列、空格改 Tab 的新旧快照与 Undo、语义文字编辑及 2 万行；435 点分配失败扫描保持发布原子与无泄漏。`prepare_tree_equal` 在 652 项 CommonMark 的增量准备/完整解析差分中逐项比较来源范围和列数据，Windows 专项 `artifacts/xui-document-rebuild/validation-2026-09-29-list-columns-corpus-first.log` 退出码零。完整 Windows 套件 `validation-2026-09-29-list-columns-final-suite.log` 退出码零，含独立 Core、语料、实际 DLL/导出、Renderer/Editor、MessageList 和原生绘制。Linux Core ASan/UBSan/语料 `validation-2026-09-29-list-columns-linux-core.log`、无窗口 Renderer `validation-2026-09-29-list-columns-linux-renderer.log` 与 View/Editor `validation-2026-09-29-list-columns-linux-view.log` 均退出码零；View 构建仅有未修改 DatePicker 的可能未初始化告警。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror`、未打补丁 MD4C 严格编译、API 注释及 lint 153/153 通过。列边界覆盖解析器确认的列表续行，不代表完整块内 CST 或任意跨结构视觉编辑已完成。

换行来源批次的可选 Windows WebView2 基础控件、Document 内部 provider/MessageList 与离屏 KaTeX/Mermaid/HTML 渲染复测分别见 `artifacts/xui-document-rebuild/validation-2026-09-29-break-source-webview-build.log`、`validation-2026-09-29-break-source-web-provider.log`、`validation-2026-09-29-break-source-web-render.log`，三项退出码零。通用 WebView 对外范围仍为 Windows 基础网页功能，Document 内部脚本及截图渲染保留。

Markdown 来源行 trivia 分区验证：新增 `xuiDocumentSnapshotGetSourceLine` 对原始 SourceStore 给出 `[行首,前导空白结束)`、正文、尾随空白和原始行结束符的连续字节范围。`markdown_source_line_partition` 逐字节核对 BOM、标题、列表/引用、围栏、LF/CR/CRLF、全空白行和无末尾换行；9 KiB 长行验证反向/正向分块扫描，CR 位于 4 KiB 块末尾且 LF 位于下一块的用例验证跨块行结束符，局部 SOURCE 事务后旧快照不变、新快照范围平移，Undo/Redo 恢复。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-source-line-suite.log` 退出码零，覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-source-line-linux-core-final.log` 退出码零。新增扫描边界后的 Windows Core `validation-2026-09-29-source-line-core-final.log`、实际 DLL Core `validation-2026-09-29-source-line-dll-core-final.log` 通过；新增 DLL 导出经 `validation-2026-09-29-source-line-dll-exports.log` 验证，修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 日志 `validation-2026-09-29-source-line-analyzer.log` 为空；API 注释覆盖 `xui_document.h` 148/148、头文件语法和差异检查均通过。该层精确分区原始 trivia，不把正文中的 `>`、列表符或围栏字符声明为已解析 token；标记归属及完整 CST 仍是后续目标。

Markdown VISUAL 跨段落 Unicode 空白边界验证：`markdown_cross_block_source_patch` 的新增用例先失败于前导 NBSP 直接写成原始字节（`artifacts/xui-document-rebuild/validation-2026-09-29-cross-block-unicode-boundary-before.log`）。修复后，前导 NBSP、单独 NBSP、CRLF 段尾 U+2002 写为 `&#160;` / `&#8194;`；边界普通中文仍是原始 UTF-8。每例检查选区外 `&amp;` 与引用定义原字节、完整重载后的语法、Undo/Redo。实际 DLL `document_cross_block_source_editor_cases` 还覆盖 VISUAL 输入、SOURCE 切换与一步 Undo/Redo，专项 `validation-2026-09-29-cross-block-unicode-boundary-editor.log` 退出码零。Windows 完整套件 `validation-2026-09-29-cross-block-unicode-boundary-suite.log` 覆盖 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-cross-block-unicode-boundary-linux-core.log` 退出码零。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 日志 `validation-2026-09-29-cross-block-unicode-boundary-analyzer.log` 为空，差异检查通过。本批不证明完整 Markdown CST 或复杂跨结构编辑。

共享属性池百万基数及碰撞链验收：`test_xui/xui_document_attribute_pool_scale_test.c` 的独立 `--million` 档逐桶检查 100 万种不同属性的 AVL 顺序、高度、平衡、计数及释放后零存活字节；Windows O2 为创建 1.180 秒、释放 0.930 秒，Linux ASan/UBSan 为创建 4.109 秒、释放 3.659 秒，后者见 `artifacts/xui-document-rebuild/validation-2026-09-29-attr-million-linux-million.log`。默认门禁还把三个不同属性人工移入同一哈希链，先从链后部查找并复用已有属性，再分别先删除根、中间、尾节点，逐步核对索引；Windows 完整套件 `validation-2026-09-29-attr-million-lookup-final-suite.log` 与 Linux 高基数 sanitizer `validation-2026-09-29-attr-million-lookup-linux-scale.log` 均通过。完整 Windows 套件覆盖实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList、原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-attr-million-linux-core.log` 通过。此人工注入只验证碰撞链查找和删除，不证明哈希抗攻击性；百万档是属性池压力测试，不是完整 Document 的平台 P95。

图片 alt 边界入口补测：`image_insert_local_patch_cases` 现在用前导 ASCII 空格与尾部 NBSP 的 alt 替换段内文字，要求精确源码 `![&#32;a&#160;](<\\/new> "title")`，且选区外 `&amp;`、CRLF、粗体和后续段落不变；独立 Core 测试通过。实际 DLL 的 `image_editor_cases` 分别在 VISUAL 插入带两端空白的图片、更新为前导空格 alt，检查源码实体写法、撤销及模式/只读策略；Editor 专项测试通过。两项均走上一批共享的图片序列化与局部补丁路径，不改变通用 WebView 范围。

公开 API 门禁迁移验证：`python -B -m unittest discover -s api-docs/tools -p test_comment_lint.py` 的五项用例覆盖旧 API 删除、新无注释 API、存续 API 丢注释和新头发现；`python -B api-docs/tools/comment_lint.py --verify` 对四个活跃公开头报告 755/755、3255/3255、147/147、128/128，并通过四头 GCC 语法检查。`api_comment_coverage.py --no-write` 前后的 `coverage.json` SHA-256 相同，基线不被 CI 检查步骤改写；基线的 `xui.h` 不再包含旧 RichDocument/RichEdit 名称。`git diff --check` 通过。这些是注释覆盖与门禁证据，不是新 API 运行时行为的证明。

Markdown 图片 alt 边缘空白验证：新增 alt-only 引用式图片前导空格用例先观察到整个图片被改为内联格式（源码从期望 `![&#32;new][ref]` 变为 `![ new][ref]`）；局部补丁与图片序列化现在共用首尾 Unicode 空白实体写法。`image_alt_only_reference_patch` 核对引用式保真、前导 ASCII 空格、尾部 NBSP、实体/CRLF、独立重载、NodeId 和 Undo/Redo；`image_local_patch_cases` 另以 URL/标题同时更新核对完整图片序列化的前导空格。`image_command_failures` 新增 157 点边缘空白分配失败扫描，失败时 revision、历史、树与来源不变且无泄漏。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-image-alt-edge-full-suite.log` 覆盖独立 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-image-alt-edge-linux-core.log` 均退出码零。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与 `git diff --check` 通过；完整 CST 未由此验收。

Markdown 图片 alt 单独更新验证：先失败 `artifacts/xui-document-rebuild/validation-2026-09-29-image-alt-only-before.log` 证实旧写回把引用式图片转成内联图片并添加空行。`image_alt_only_reference_patch` 的三组来源对照要求保留 `[ref]` 写法、引用定义、内联图片单引号标题、图片外实体/CRLF，且更新后的 alt 经独立重载一致、原图片 NodeId 存续、Undo/Redo 恢复原字节。`image_command_failures` 新增引用式 alt 单改的 139 点分配失败扫描，发布状态/历史不变且无泄漏。Windows 完整套件 `validation-2026-09-29-image-alt-only-final-suite.log` 包含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-image-alt-only-linux-core.log` 均退出码零。GCC `-fanalyzer -Wall -Wextra -Werror` 与 `git diff --check` 通过；复杂来源及完整 CST 未由此验收。

Markdown 空 alt 图片属性更新验证：先失败 `artifacts/xui-document-rebuild/validation-2026-09-29-empty-image-update-before.log` 证明 `TxnUpdateImage` 对 `![]` 回退整篇写回，未能保留图片外的 `&amp;` 源码。局部补丁现仅接受与零宽 alt 来源完全对齐的图片语法记录，候选重新解析并核对语义；`image_local_patch_cases` 增补空 alt 的 URL/alt/标题、稳定 NodeId、周围源码与 Undo/Redo。`image_command_failures` 增补 152 点空 alt 更新分配失败扫描，历史/发布状态原子且无泄漏。Windows 完整套件 `validation-2026-09-29-empty-image-update-final-suite.log` 包含实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与语料 `validation-2026-09-29-empty-image-update-linux-core.log` 均退出码零。修改源码的 GCC `-fanalyzer -Wall -Wextra -Werror` 与 `git diff --check` 通过；这不证明复杂嵌套图片语法或完整 CST 已完成。

Markdown 图片 alt 文本编辑验证：先失败 `artifacts/xui-document-rebuild/validation-2026-09-29-image-alt-text-before.log` 证实 `TxnReplaceText` 在非空图片 alt 中插入 `]` 被旧的字面写回路径拒绝；`validation-2026-09-29-empty-image-alt-before.log` 证实空 alt 缺少精确来源而改写选区外源码。`markdown_image_alt_text_edit` 用六组空 alt、内联/引用图片核对 `]`、`*`、`&amp;` 字样及前导空格的源码转义、已有实体与定义原文、图片 URL、独立重载语义和 Undo/Redo；空 alt 的零宽位置映射为精确来源；非空/空 alt 的 150/131 点分配失败扫描保持旧 revision/源码并无泄漏。Windows 完整套件 `validation-2026-09-29-image-alt-text-final2-suite.log` 覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-image-alt-text-final2-linux-core.log` 均退出码零。GCC `-fanalyzer -Wall -Wextra -Werror` 与 `git diff --check` 通过；复杂 alt 语法和完整来源 CST 尚未由此证明完成。

Markdown VISUAL Unicode 空白边界验证：先失败 `artifacts/xui-document-rebuild/validation-2026-09-29-visual-unicode-space-before.log` 证明同节点斜体首端插入 NBSP 被旧写回拒绝。`markdown_visual_boundary_space` 现含十一组空格、Tab、NBSP、全角空格的首尾输入，逐例核对源码实体写法、解析后的文字与 marks、未选中 `&amp;`/CRLF/引用定义及 Undo/Redo；三种边缘字符的分配失败扫描各覆盖 73 点，失败时 revision/源码不变且无泄漏。实际 DLL Editor `validation-2026-09-29-visual-unicode-space-editor.log` 验证 NBSP 插入后 VISUAL/SOURCE 切换和历史。Windows 完整套件 `validation-2026-09-29-visual-unicode-space-final-suite.log` 覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-visual-unicode-space-linux-core.log` 均退出码零。GCC `-fanalyzer -Wall -Wextra -Werror` 和 `git diff --check` 通过；CR/LF 不经此空白实体路径，完整来源 CST 仍待实现。

Markdown VISUAL 单节点 Tab 边界验证：`markdown_visual_boundary_space` 扩充为八组空格/Tab 语义输入，验证段首 `\tabc` 保存为 `&#9;abc`、段尾 Tab、加粗末尾和引用块斜体开头；未选中的 `&amp;`、CRLF、引用定义保持原字节，重载后文字与 marks 正确，Undo/Redo 恢复原源码。旧实现先在 `artifacts/xui-document-rebuild/validation-2026-09-29-visual-tab-edge-before.log` 复现行首 Tab 事务失败；修复后 Core `validation-2026-09-29-visual-tab-edge-core-final.log` 中 Tab 与原空格各 73 个分配失败点原子且无泄漏。实际 DLL Editor `validation-2026-09-29-visual-tab-edge-editor.log` 验证 VISUAL 插入、SOURCE 切换与撤销重做。Windows 完整套件 `validation-2026-09-29-visual-tab-edge-final-suite.log` 覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 和同一语料 `validation-2026-09-29-visual-tab-edge-linux-core.log` 均退出码零。修改源码通过 GCC `-fanalyzer -Wall -Wextra -Werror` 与 `git diff --check`。

Markdown 多字符实体内部端点验证：`markdown_cross_block_source_patch` 将 `&fjlig;` 解码后的 `f|j` 边界作为跨段落选区端点，失败日志 `artifacts/xui-document-rebuild/validation-2026-09-29-collapsed-entity-boundary-before2.log` 中旧结构回写把未选中的 `&amp;` 规范化为 `\&` 并增加空行。新局部补丁将完整实体源码替换为保留标量的数字实体，必要时左右端同时处理；五组单端及容器/换行/Unicode 样本、一组双端样本都按预期原字节输出，独立完整重载的语法与 Undo/Redo 一致。204 点分配失败扫描保持源码/revision 原子性且无泄漏。实际 DLL Editor `validation-2026-09-29-collapsed-entity-boundary-editor.log` 验证 VISUAL 选区输入、SOURCE 切换、Undo/Redo；最终 Windows 完整套件 `validation-2026-09-29-collapsed-entity-final-suite-retry.log` 覆盖 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制，Linux Core ASan/UBSan 和同一语料 `validation-2026-09-29-collapsed-entity-linux-core.log` 均退出码零。第一次完整套件在共享 Win32 剪贴板 DIB 检查失败，单独 DIB 重跑 `validation-2026-09-29-collapsed-entity-dib-retry.log` 和第二次完整套件通过；不能据此断言剪贴板测试绝无瞬时干扰。修改源码通过 GCC `-fanalyzer -Wall -Wextra -Werror` 与 `git diff --check`。局部补丁仍要求实体片段完整且全文重解析语义相等，其他折叠映射与完整 CST 继续追踪。

原生 Markdown 方言校验：`file_roundtrip` 从真实原生序列化中删除 `dialect`，先失败 `artifacts/xui-document-rebuild/validation-2026-09-29-native-dialect-before.log` 证明旧加载器会接受并按调用方方言重解析。修复后，合法文件中的 EXTENDED 方言覆盖调用方 COMMONMARK 描述；缺失字段返回 `XUI_DOC_ERROR_FORMAT` 且输出置空。独立 Core `validation-2026-09-29-native-dialect-core.log`、Windows 完整套件 `validation-2026-09-29-native-dialect-final-suite.log`、Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-29-native-dialect-linux-core.log` 均退出码零；Windows 套件还覆盖实际 DLL、Renderer/Editor、MessageList 与原生同步/异步绘制。GCC `-fanalyzer -Wall -Wextra -Werror` 和 `git diff --check` 通过。这一校验针对原生工程文件；单独 `.md` 的方言仍由打开时的描述指定。

`currentColor` 语义验证：HTML fragment 导入、导出再导入保留可表达的前景继承和文字/背景关键字；无效后写声明不覆盖有效颜色，压平的容器背景不传给段落。`html_current_color_roundtrip` 当前验证原生 `schemaVersion: 6` 写出、版本 3 降格拒绝和版本 6 读取；初次实现时的版本 4 验证见当批日志。`current_color_style_commands` 核对公开编辑 API 的互斥/零值校验、祖先关键字样式查询、Markdown 损失、清除格式和 Undo；`current_color_render` 使用实际 DLL 验证链接指定 `color:currentColor` 时采用正文色、背景在祖先文字色变化后重新求值、主题颜色变化后重新绘制。Core 见 `artifacts/xui-document-rebuild/validation-2026-09-29-currentcolor-core-final4.log`，Renderer 见 `validation-2026-09-29-currentcolor-renderer.log`；完整 Windows 套件与 Linux Sanitizer 验证见同目录的 `validation-2026-09-29-currentcolor-final-suite.log`、`validation-2026-09-29-currentcolor-linux-core-final2.log`，均退出码零；修改源码通过 GCC `-fanalyzer -Wall -Wextra -Werror` 和差异检查。实现参照 [CSS Color 4 currentColor](https://drafts.csswg.org/css-color/#currentcolor)。容器被压平且已提供继承文字色时导入实体化为 RGBA，因此不能保留该容器原始 CSS 关键字；完整 CSS 层叠仍待实现。

HTML CSS HWB 导入验证：`artifacts/xui-document-rebuild/validation-2026-09-29-hwb-before.log` 先证实导入器跳过 `hwb()`，导致段落颜色缺失。新增用例核对色相单位、白黑混色及超过 100% 的比例灰度、斜杠 alpha、透明零值、无效逗号与缺少空格不覆盖前序有效声明，并通过 HTML 导出再导入对照统一 Document 属性。Core `validation-2026-09-29-hwb-core-final.log`、Windows 完整套件 `validation-2026-09-29-hwb-full-suite.log`（实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制）、Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-hwb-linux-core.log` 均退出码零；修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过。实现依据 [CSS Color 4 HWB](https://drafts.csswg.org/css-color/#the-hwb-notation)；当前不保存缺失分量 `none` 或原始 HWB 写法，导出为等价 RGBA，完整 CSS 兼容仍未达成。

多段嵌套标题来源保真验证：改动前 `artifacts/xui-document-rebuild/validation-2026-09-29-multi-heading-before.log` 复现提醒块内双段标题命令把未触及的 `[!nOtE]`、CRLF 与裸 `>` 行重写。精确路径现对每个独立标题属性操作求原来源位置，验证补丁互不重叠，逆序修改源码；全文重解析语义相等才发布。Core `validation-2026-09-29-multi-heading-core-final.log` 覆盖提醒块、列表项及跨引用与列表父容器的双段标题级别升级和取消、精确源码、独立重载语法、Undo/Redo，并以 94 点分配失败扫描核对树/源码/历史原子性和无泄漏；缩进、制表符、有序列表、行内代码、转义及引用式链接的单段来源矩阵也通过。实际 DLL Editor `validation-2026-09-29-multi-heading-editor.log` 以 VISUAL 多段选区执行命令、SOURCE Undo、VISUAL Redo。Windows 完整套件 `validation-2026-09-29-multi-heading-final-suite.log` 覆盖实际 DLL、652 项 CommonMark、Editor/Renderer、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-multi-heading-final-linux-core.log` 均退出码零。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过。跨容器其他结构命令及完整 CST 未因此完成。

嵌套块标题来源保真验证：改动前 `artifacts/xui-document-rebuild/validation-2026-09-29-admonition-heading-before.log` 复现单段提醒块标题命令重写未触及的 `[!nOtE]`、CRLF 和空引用标记。局部标题标记补丁后，Core `validation-2026-09-29-nested-heading-core-final.log` 覆盖提醒块、列表项、嵌套引用、脚注、加粗/链接首定界符、标题升级/取消、精确源码与 Undo/Redo；与独立全文重载逐项比较行内语法、引用候选及文字来源范围，65 个分配失败点保持旧树/源码/历史且无泄漏。实际 DLL Editor `validation-2026-09-29-nested-heading-editor.log` 验证 VISUAL 命令与 SOURCE 撤销、VISUAL 重做；Windows 完整套件 `validation-2026-09-29-nested-heading-full-suite.log` 覆盖实际 DLL、652 项 CommonMark、Editor/Renderer、MessageList 和原生绘制，Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-nested-heading-linux-core.log`，均退出码零。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 和差异检查通过。多节点标题操作已在上方批次补验；多行及无法精确定位的结构命令仍按原路径回退，完整 CST 尚未完成。

提醒块原生标题验证：改动前 `artifacts/xui-document-rebuild/validation-2026-09-29-admonition-render-before.log` 证明 `[!NOTE]` 与普通引用具有相同高度、光标和正文文字绘制数。现在实际 DLL 渲染专项 `validation-2026-09-29-admonition-render-final.log` 验证增加一行标题、正文光标下移、标题区域命中引用起点，空 Rich 提醒块的绘制与光标，以及整块选区确实覆盖正文；选区修复前见 `validation-2026-09-29-admonition-range-before.log`。Windows 完整套件 `validation-2026-09-29-admonition-visual-final-suite.log` 覆盖 Core、652 项 CommonMark、Editor/Renderer、MessageList 和原生绘制并退出码零。Linux 无窗口 Renderer ASan/UBSan `validation-2026-09-29-admonition-visual-linux-renderer-final.log` 实际执行标题路径，核对 Markdown 正文位置、标题命中与两次文字绘制，退出码零。Renderer/Layout 的 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过；类型专用图标/配色、本地化与真实读屏尚未验收。

扩展 Markdown 提醒块的 HTML 语义往返：先失败 `artifacts/xui-document-rebuild/validation-2026-09-29-admonition-before.log` 证明 `[!NOTE]` 的类型在 HTML 导出时消失。修复后带 `info` 的引用块以转义后的 `data-xui-info` 输出，导入后恢复原值；Core `validation-2026-09-29-admonition-core.log` 验证 Markdown 解析、HTML 片段导入、Rich 正文和再次转换 Markdown 的 `note` 类型。Windows 完整套件 `validation-2026-09-29-admonition-full-suite.log` 包含实际 DLL、652 项 CommonMark、Editor/Renderer、MessageList 与原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-admonition-linux-core.log`，均退出码零。两个修改源码的 GCC `-fanalyzer -Wall -Wextra -Werror` 与 `git diff --check` 通过。原生标题已由上方 R1 批次补足；此批次只验收语义往返。

位移索引批次的完整发布回归：Windows `artifacts/xui-document-rebuild/validation-2026-09-29-shifted-projection-full-suite.log` 含生产 DLL、652 项 CommonMark、Editor/Renderer、MessageList 和原生绘制，退出码零。可选 WebView2 控件 `validation-2026-09-29-shifted-projection-webview-build.log`、DocumentView/MessageList 的内部 KaTeX/Mermaid/HTML provider `validation-2026-09-29-shifted-projection-web-provider.log` 与离线测量、HTML sandbox、PNG 截图 `validation-2026-09-29-shifted-projection-web-render.log` 退出码零；通用 WebView 的公开接口范围未扩张。

Markdown 延迟位移投影验证：优化前 `artifacts/xui-document-rebuild/validation-2026-09-29-shifted-projection-before.log` 中，文首文本增加 1 字节导致 1024 个后续块的接纳投影分配 10355 次。根块 ID 顺序相同且无空块歧义时，现保留解析器的持久位移序列，并复用共享后缀；ID 不一致时仍走旧投影。Core `validation-2026-09-29-shifted-projection-core-v4.log` 同一事务为 115 次分配，单次最大申请 16456 字节，没有整篇大小申请，解析字节低于原文一半；尾部文字的节点来源范围和片段与独立全文重载逐项相同，两个脚注的引用顺序与定义来源顺序相反时范围也一致，Undo 保留原文。Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-29-shifted-projection-linux-core.log` 退出码零；原有分配失败、取消与来源保真测试通过，修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过。语义和 Schema 校验仍按整树执行，这里只确认解析后投影的内存分配改善。

Markdown 语义投影共享子树复用验证：优化前 Core `artifacts/xui-document-rebuild/validation-2026-09-29-projection-baseline.log` 记录 1024 个前置段落后的末块标题编辑需 10364 次分配；仅当接纳阶段的期望与解析索引节点逐个共享、来源坐标无延迟位移且无需清理空节点时，克隆的期望树现在直接复用子树。优化后 Core `validation-2026-09-29-projection-reuse-core-v2.log` 同场景为 124 次分配，文首等长文字编辑且后方保留大量块为 100 次；两项均断言低于 512 次并验证精确源码与 Undo，现有后缀图片/链接/文字来源坐标全文重载对照和分配失败扫描通过。Windows 完整套件 `validation-2026-09-29-projection-reuse-full-suite.log` 覆盖实际 DLL、652 项 CommonMark、Editor/Renderer、MessageList 与原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-projection-reuse-linux-core.log` 同样通过，两项退出码零。可选 WebView2 基础控件 `validation-2026-09-29-projection-reuse-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML provider `validation-2026-09-29-projection-reuse-web-provider.log` 及离线 PNG 渲染 `validation-2026-09-29-projection-reuse-web-render.log` 也均退出码零。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过。语义等价、Schema 和来源块索引仍需整树检查，此处只声明内存分配改善，不声明局部编辑总耗时已达到次线性。

单块结构命令局部解析和来源位移验证：先失败 `artifacts/xui-document-rebuild/validation-2026-09-29-single-block-local-before.log` 证明约 66 KiB 的末尾标题切换仍申请全文解析缓冲；`validation-2026-09-29-single-block-provenance-probe.log` 进一步复现候选局部解析成功后，后续图片节点来源偏移 5 与独立全文重载偏移 8 不符。单块安全路径现立即局部解析，语义候选不符时自动全文重解析；语义投影保留已解析的延迟块位移，并把文字来源片段写为正确绝对坐标。Core `validation-2026-09-29-single-block-core-v2.log` 覆盖 1024 段前置文档、解析计数、精确源码、Undo、图片/链接/普通文字节点与独立全文重载的来源坐标和片段比较，以及 89 点分配失败扫描；Windows 完整套件 `validation-2026-09-29-single-block-full-suite.log` 覆盖实际 DLL、652 项 CommonMark、Editor/Renderer、MessageList 与原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-single-block-linux-core.log` 通过。三项退出码零，修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 和差异检查通过。多块结构回写、隐藏定义及语义投影遍历全树的成本仍待继续改进。

Markdown 语义事务局部解析与结构回写内存验证：先前大分配探针用“源码长度＋1”做回调大小的精确比较，未计入 Document 分配头部；修正为回调申请大小阈值后，`validation-2026-09-29-block-local-many-blocks-before.log` 证明任务切换仍有整篇大小的解析分配。安全限定的即时语义补丁现复用增量解析，普通结构回写只读取目标块和最多 3 字节前置上下文。Core `artifacts/xui-document-rebuild/validation-2026-09-29-semantic-local-core-v4.log` 验证 1024 个独立前置段落后的任务切换没有整篇大小的分配、增量解析计数上升且解析字节小于源长一半；末尾段落改标题保留原字节、Undo，并只容许一次全文解析缓冲；235 点分配失败扫描保持原子性且无泄漏。Windows 完整套件 `validation-2026-09-29-semantic-local-full-suite.log` 包含实际 DLL、652 项 CommonMark、Editor/Renderer、MessageList 与原生绘制；Linux Core ASan/UBSan 和同一语料 `validation-2026-09-29-semantic-local-linux-core.log` 通过。三项退出码零，修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过；语义事务分批结构回写仍走全文解析。

Markdown 任务标记定位验证：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-29-task-empty-probe.log` 复现空父任务项后接子列表时 `TxnSetAttributes` 无法切换外层标记。修复后 Core `validation-2026-09-29-task-syntax-core-final.log` 覆盖空父项、嵌套项、大写 `[X]` 空操作、CRLF、64 KiB 单段前置正文精确源码及 Undo；该批次原分配断言未计入 Document 分配头部，内存结论以后续重测为准；Windows 完整套件 `validation-2026-09-29-task-syntax-full-suite.log` 覆盖实际 DLL、652 项 CommonMark、Editor/Renderer、MessageList 与原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-task-syntax-linux-core.log` 通过。三项退出码均为零，修改源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过。

MessageList 任务项动作验证：实际 DLL 专项 `artifacts/xui-document-rebuild/validation-2026-09-29-message-task-targeted.log` 覆盖 GFM Markdown 与 Rich 任务项标记点击、可访问 TOGGLE、宿主回调只发请求、宿主事务更新勾选及 Markdown 源码、Undo、未配置回调时只读、拖出标记与按下后解绑取消；完整 Windows 套件 `validation-2026-09-29-message-task-full-suite.log` 覆盖 652 项 CommonMark、DLL 导出、Renderer/Editor、MessageList 与原生绘制，退出码零。Linux 无窗口 ASan/UBSan `validation-2026-09-29-message-task-linux-sanitizer.log` 覆盖相同 MessageList 用例并退出码零，仅见未改动 DatePicker 的已知 `maybe-uninitialized` 编译告警；修改源码 Windows GCC `-fanalyzer` 与差异检查通过。真实读屏和深层嵌套列表手感尚未验收。

HTML 子资源隔离验证：实际 WebView2 窗口 `artifacts/xui-document-rebuild/validation-2026-09-28-html-resource-window.log` 在 Markdown 原始 HTML iframe 内放置 localhost 图片和内联 data SVG 图片，开启本地非阻塞 TCP 监听；WebView2 的含来源过滤器拦截 1 次，监听端口收到 0 次连接，远程图片宽度为零，data 图片宽度为 1。普通窗口回归 `validation-2026-09-28-html-resource-default.log` 与系统输入回归 `validation-2026-09-28-html-resource-input.log` 均通过。可选 WebView2 控件、Document 静态公式/Mermaid/HTML provider、离线渲染分别见 `validation-2026-09-28-html-resource-webview-build.log`、`validation-2026-09-28-html-resource-web-provider.log`、`validation-2026-09-28-html-resource-web-render.log`；生产 DLL 检查 `validation-2026-09-28-html-resource-exports.log` 确认 13 个内部浏览器符号未导出。完整 Windows 套件 `validation-2026-09-28-html-resource-full-suite.log` 覆盖 Core、652 项 CommonMark、实际 DLL、Editor/Renderer/MessageList 和原生绘制，退出码零；Windows `-fanalyzer -Wall -Wextra -Werror` 和 Linux 严格编译通过。策略目前只允许 `about:`/`data:` 资源；受控外部资源映射、文件协议专项、真实辅助技术和复杂表单仍未验收。

HTML 外部链接策略验证：实际 WebView2 窗口 `artifacts/xui-document-rebuild/validation-2026-09-28-html-external-window.log` 覆盖 Markdown iframe 中 HTTP(S) 点击向 Document 宿主回调传递解码 URL，且页面保持原位；`javascript:` 链接不回调，伪造的旧代次与非 HTTP(S) 消息被拒绝，表单提交后 iframe 内容仍在。原始 HTML 的脚本与表单导航被 sandbox 禁止，文内锚点继续由面板内脚本滚动。可选 WebView2 基础控件、Document 静态 provider、离线截图回归分别见 `validation-2026-09-28-html-external-webview-build.log`、`validation-2026-09-28-html-external-web-provider.log`、`validation-2026-09-28-html-external-web-render.log`；生产 DLL 的私有脚本/消息/截图符号未导出，见 `validation-2026-09-28-html-external-webview-exports.log`。完整 Windows 套件 `validation-2026-09-28-html-external-full-suite.log` 覆盖 Core、652 项 CommonMark、实际 DLL、Editor/Renderer/MessageList 与原生绘制，退出码零。补充实窗 `validation-2026-09-28-html-refresh-default.log` 在 Markdown iframe 中确认 `<meta refresh>` 原始节点存在且内容仍可读取；设置 `XUI_DOCUMENT_HTML_SYNTHETIC_TEXT=1` 的 `validation-2026-09-28-html-native-input.log` 经 Windows `SendInput` 输入中文和 ASCII，验证 iframe 输入框的实际值及 Document revision 不变。Windows 新增源文件 GCC `-fanalyzer -Wall -Wextra -Werror` 与 Linux 严格编译通过。此批次时子资源策略尚未验证，后续批次已验收默认拦截；多控件表单及键盘导航、辅助技术与大 HTML 性能仍未验收。

HTML 独立交互面板验证：带 WebView2 的实际窗口 `artifacts/xui-document-rebuild/validation-2026-09-28-html-interaction-window.log` 先在 Rich HTML 节点核对 iframe 内联脚本禁用、可编辑 DOM 状态、窗口裁剪与焦点、浏览器 DOM 修改不进入 Document 历史；无关段落事务保留 DOM，HTML 源码事务与 Undo 重载，关闭重开恢复焦点。随后从 Markdown 原始 HTML 节点建立同类面板，文内 `#锚点` 点击滚动到长内容末端而不丢失 iframe 内容。普通 DLL 的禁用路径 `validation-2026-09-28-html-interaction-disabled.log` 核对缺少 viewport 时拒绝、无 WebView2 时返回 UNSUPPORTED；DLL 导出检查包含新增六个 API。最终 Windows 完整套件 `validation-2026-09-28-html-interaction-full-suite.log` 覆盖 Core、652 项 CommonMark、实际 DLL、Editor/Renderer/MessageList 与原生绘制并退出码零；可选 WebView2 控件、其系统注入 Unicode 输入、Document 静态 provider 与离线截图分别见 `validation-2026-09-28-html-interaction-webview-build.log`、`validation-2026-09-28-html-interaction-webview-input.log`、`validation-2026-09-28-html-interaction-web-provider.log`、`validation-2026-09-28-html-interaction-web-render.log`，均退出码零。新宿主源文件的 GCC `-fanalyzer` 和差异检查通过。外部链接点击策略由后续批次补足；网络/文件资源、平台读屏、复杂网页表单及大 HTML 性能尚未通过验收。

前置列表项非段落首块隐藏定义验证：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-28-definition-nonparagraph-before.log` 记录围栏代码首块带来源层链接定义时跨父替换被拒绝。修复后精确保留列表标记、LF/CRLF 与定义原行，删除选中的代码/标题/表格块，全文候选重新解析并核对语义及定义拼写。Core `validation-2026-09-28-definition-nonparagraph-kinds-core.log` 验证代码首块的尾随与块间定义、标题、GFM 表格、CRLF 有序列表、独立重载语法和 Undo/Redo；两条代码路径的 337/357 点分配失败扫描确认旧 revision/源码原子性且无泄漏。实际 DLL Editor `validation-2026-09-28-definition-nonparagraph-editor.log` 验证 VISUAL→SOURCE 与撤销/重做；Windows 完整套件 `validation-2026-09-28-definition-nonparagraph-final-suite.log` 包含 652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制，Linux Core ASan/UBSan 与同一语料 `validation-2026-09-28-definition-nonparagraph-linux-core.log`，均退出码零。Commands 与 Markdown 来源补丁的 GCC `-fanalyzer` 日志为空，差异检查通过。未逐一验证的其他首块、非 LINK 来源与完整 CST 保真仍未完成。

前置列表项块间隐藏定义来源保真验证：过去明确拒绝的“两个可见段落之间夹 LINK 定义”跨父替换现在成功，并保留选区外 `&amp;`、原定义行、空列表项和后续引用前缀。Core `artifacts/xui-document-rebuild/validation-2026-09-28-middle-definition-core-matrix.log` 覆盖单定义、块间加尾部双定义、LF/CRLF、有序列表、重载语法、Undo/Redo；双定义及 CRLF 路径分别经 400/359 点分配失败扫描，失败时 revision 和源码不变且无泄漏。实际 DLL Editor `validation-2026-09-28-middle-definition-editor.log` 覆盖 VISUAL 输入与 SOURCE 切换；最终 Windows 套件 `validation-2026-09-28-middle-definition-final-suite.log` 的 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制通过。Linux Core ASan/UBSan 和同一语料 `validation-2026-09-28-middle-definition-linux-core.log` 退出码零；Commands 与 Markdown 来源补丁的 GCC `-fanalyzer` 日志均为空，差异检查通过。历史条目中的该拒绝边界现已解除；非 LINK 来源和完整 CST 仍未获得通用证明。

MessageList 表格指针与键盘交互验证：先失败的实际 DLL 日志 `artifacts/xui-document-rebuild/validation-2026-09-28-message-table-drag-before.log`、`validation-2026-09-28-message-table-keyboard-before.log` 分别确认 Alt+拖动与 Alt+Shift+方向键原先没有矩形路径。最终 GFM 2×2 用例覆盖正向拖动、反向松手、保留锚点的键盘延伸、从普通 Cell 光标开始扩展/收缩、Escape 清除以及普通文字点击恢复文字选区。独立 Rich 2×3 合并 Cell 用例覆盖局部网格选区向完整跨度展开、Cell 可访问状态及动作、Alt 点击、跨跨度键盘导航、带引号的多段正文 TSV，并在 60 像素高视口验证目标 Cell 被滚入视口；纯选择保持 Document revision。最终实际 DLL 专项 `validation-2026-09-28-message-table-keyboard-scroll.log`、Windows 完整套件 `validation-2026-09-28-message-table-interaction-suite.log` 退出码零；后者覆盖 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux MessageList ASan/UBSan `validation-2026-09-28-message-table-interaction-linux.log` 退出码零，仅有未修改 DatePicker 的可能未初始化编译告警；修改的 MessageList GCC `-fanalyzer` 日志为空，差异检查通过。真实平台读屏、复杂嵌套表格及大型可访问树的延迟仍未验收。

MessageList 表格矩形选区验证：GFM 2×2 表格中，Cell 可访问 `SET_SELECTION(NULL)` 建立 1×1 矩形；公开接口扩为 2×2 后，四格在共享 Renderer 中以指定选区色绘制，消息级 GetSelectedText/CopySelection 输出 `A\tB\nc\t你` 的 TSV。局部范围 payload 仍选择文字，矩形与文字选区互斥；非法行号保持原矩形，显式清除与实际文档提交清除矩形，整个纯选择流程不修改 revision。实际 DLL 专项 `artifacts/xui-document-rebuild/validation-2026-09-28-message-table-rectangle-final.log`、Windows 完整套件 `validation-2026-09-28-message-table-rectangle-final-suite.log` 均退出码零；完整套件覆盖 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux MessageList ASan/UBSan `validation-2026-09-28-message-table-rectangle-linux.log` 退出码零，仅有未修改 DatePicker 的可能未初始化编译告警；修改的 MessageList GCC `-fanalyzer` 日志为空，新接口在 DLL 中导出，差异检查通过。合并 Cell 展开复用已由 DocumentView 验证的内核函数；该批次当时尚未单独构造 MessageList 合并 Cell、指针或键盘用例，后续验收见上文。平台真实读屏仍未验收。

MessageList 复杂容器与对象选区验证：共享 Document 可访问投影现在为列表、列表项、引用、脚注、表格、行和 Cell 报告局部文本范围；MessageList 按同一 UTF-8/制表符/换行投影执行选择并报告方向与选中状态。GFM 2×2 表格专项与独立 DocumentView 对照表格和 Cell 的角色和值，核对行列层级、Cell 内中文反向选择、非法 UTF-8 中间字节、表格到行的范围裁剪与复制。图片按父容器两侧 GAP 组成结构选区，辅助技术可读到 SELECTABLE/SELECTED；测试从选区反查到 IMAGE 节点，并确认纯选择不改变 revision。实际 DLL 专项 `artifacts/xui-document-rebuild/validation-2026-09-28-message-structured-object.log`、Windows 含 652 项 CommonMark 的完整套件 `validation-2026-09-28-message-structured-final-suite.log` 退出码零；Linux MessageList ASan/UBSan `validation-2026-09-28-message-structured-linux-message.log` 与 View/Editor ASan/UBSan `validation-2026-09-28-message-structured-linux-view.log` 分别验证嵌入和原有控件。adapter、View 和 MessageList 的 GCC `-fanalyzer` 日志为空，差异检查通过。该批次结束时 MessageList 的 Cell 无 payload 动作尚只选择文本；后续矩形支持见本页开头。任务项动作、深层嵌套容器、大文档无障碍查询延迟及真实读屏仍需后续验收。

MessageList 绑定 Document 子树验证：列表现在按消息顺序暴露 `List → ListItem → Document → 段落/标题/文字等语义节点`。测试与同一 Document 的独立 DocumentView 对照根、段落和 Text 的角色与值，核对父子关系、文档提交后不变节点的可访问 ID、解绑后的 ID 失效。段落/Text 局部正反向选择共用 DocumentView 的 UTF-8 偏移映射，验证选择状态、复制文本、非法偏移原子拒绝；链接节点可激活，已有边界的节点可滚入视口。提交与解绑通知树结构变化。实际 DLL 专项见 `artifacts/xui-document-rebuild/validation-2026-09-28-message-document-subtree-events.log`，Windows 完整套件见 `validation-2026-09-28-message-document-subtree-final-suite-v2.log`，Linux MessageList ASan/UBSan 见 `validation-2026-09-28-message-document-subtree-linux-final.log`；修改的 adapter 与 MessageList 严格编译及 GCC `-fanalyzer` 通过。结构容器/对象的完整选择状态、真实读屏平台桥接和大文档树查询延迟仍未验收。

共享选择映射的独立 DocumentView/Editor Linux ASan/UBSan 回归见 `artifacts/xui-document-rebuild/validation-2026-09-28-message-document-subtree-linux-view.log`，Rich/Markdown 的 VISUAL/SOURCE/LIVE 输入、Undo、命中和绘制均通过。该构建有未修改的 DatePicker `tValue` 可能未初始化编译警告，测试退出码为零。

MessageList 动态语义事件验证：先失败的实际 DLL 用例 `artifacts/xui-document-rebuild/validation-2026-09-28-message-a11y-events-before.log` 记录清除正文选区后缺少 `SELECTION_CHANGED`。现在实际输入拖动改变端点、显式清除、绑定 Document 提交映射及解绑清除均排队选区事件；辅助消息的可访问动作和公开折叠 setter 排队状态、正文和几何事件。测试在外层 `xuiUpdate` 后核对事件合并送达，并验证已变化的选区或折叠值。最终实际 DLL 专项 `validation-2026-09-28-message-a11y-events-final-editor.log`、Windows 完整套件 `validation-2026-09-28-message-a11y-events-final-suite.log` 均退出码零；后者覆盖 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 MessageList ASan/UBSan `validation-2026-09-28-message-a11y-events-linux.log` 退出码零，只有未修改 DatePicker 的可能未初始化编译告警；MessageList GCC `-fanalyzer` 日志为空，差异检查通过。平台读屏桥接仍未安装，事件回调验证不等于真实读屏验收。

MessageList 消息正文局部范围验证：`test_message_accessibility` 在普通消息中文 UTF-8 反向范围与绑定 Markdown Document 的正反向局部范围间切换，核对 `iTextStart/iTextEnd`、SELECTED、实际复制内容和稳定消息 ID。`SET_SELECTION(NULL)` 选择整篇 Document 正文并复制末尾换行；越界或落在汉字内部的偏移返回错误且原选区不变，折叠辅助消息不接受隐藏正文的范围动作。先失败的实际 DLL 日志 `artifacts/xui-document-rebuild/validation-2026-09-28-message-a11y-range-before.log` 记录旧 provider 拒绝 Document 范围动作；最终实际 DLL 专项 `validation-2026-09-28-message-a11y-range-editor-final.log`、Windows 完整套件 `validation-2026-09-28-message-a11y-range-final-suite.log` 均退出码零，后者覆盖 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 MessageList ASan/UBSan `validation-2026-09-28-message-a11y-range-linux.log` 退出码零，仅有未修改 DatePicker 的可能未初始化编译告警；MessageList GCC `-fanalyzer` 日志为空，差异检查通过。绑定 Document 的内部语义子节点已接入，复杂结构和对象的完整选择状态、真实平台读屏与大文档可访问查询延迟仍未验收。

MessageList 消息级辅助技术验证：先失败的实际 DLL 用例 `artifacts/xui-document-rebuild/validation-2026-09-28-message-a11y-before.log` 证明列表原先未提供消息语义节点。现在列表暴露稳定消息 ID、发送者/辅助标题、普通或绑定 Document 的纯文本正文、折叠/选中/离屏状态；可访问动作可选择消息、展开辅助内容、滚入视口。用例还验证 Document revision 更新与纯文本追加保持 ID，整表替换使旧 ID 失效。实际 DLL 专项 `validation-2026-09-28-message-a11y-editor.log`、Windows 完整套件 `validation-2026-09-28-message-a11y-final-suite.log` 均退出码零；后者覆盖 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 MessageList ASan/UBSan `validation-2026-09-28-message-a11y-linux-actual.log` 退出码零，只有未修改 DatePicker 的可能未初始化编译告警；MessageList GCC `-fanalyzer` 日志为空，差异检查通过。绑定 Document 的内部语义子树已接入，复杂结构和对象的完整选择状态、嵌入交互 Widget 与真实平台读屏仍待实现或验收。消息正文局部文本范围的选择与报告见上文验证记录。

段落/标题和代码块辅助技术范围验证：`accessible_container_text_cases` 用统一 Rich Document 构造中文、粗体、链接、alt 长于占位符的图片、硬换行、后续段落与含中文的代码块；验证段落 `sValue` 使用 U+FFFC 表示对象、局部范围映射到图片两侧 GAP、正反向与跨段落偏移、UTF-8 内部偏移拒绝、代码块独立角色/文字选择、只读与禁用选区及纯选择 revision 不变。Markdown 标题含强调与图片的同一映射在 VISUAL 成功，SOURCE 明确拒绝。旧实现先在段落值和选择动作断言失败，见 `artifacts/xui-document-rebuild/validation-2026-09-28-container-a11y-before.log`；实际 DLL Editor 专项 `validation-2026-09-28-container-a11y-editor.log`、Windows 完整套件 `validation-2026-09-28-container-a11y-final-suite.log` 均退出码零，后者覆盖 652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-28-container-a11y-linux-view-final.log` 也通过含图片段落的范围选择；只有未修改 DatePicker 的可能未初始化编译告警。adapter GCC `-fanalyzer` 日志 `validation-2026-09-28-container-a11y-analyzer-final.log` 为空，差异检查通过。列表项、多段表格 Cell 与平台真实读屏尚未验收。

Text/Link 叶节点辅助技术范围验证：`accessible_text_range_cases` 在 Rich VISUAL Editor 对含中文 UTF-8 和不同 marks 的相邻 Text 测试局部范围选择、反向 anchor/caret、跨叶节点相交范围、非法 UTF-8 中间字节与越界/无效 payload 拒绝、`NULL` 整节点选择、只读 View 和禁用选区；既有 GFM Link 用例验证同一动作不影响链接激活。纯选择不改变 Document revision。旧实现先失败于 Text 无 `SET_SELECTION` 动作，见 `artifacts/xui-document-rebuild/validation-2026-09-28-text-range-before.log`；实际 DLL Editor 专项 `validation-2026-09-28-text-range-editor.log`、Windows 完整套件 `validation-2026-09-28-text-range-final-suite.log` 均退出码零，后者覆盖 652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-28-text-range-linux-view.log` 验证按节点选择和报告范围，退出码零，只有未修改 DatePicker 的可能未初始化编译告警；adapter GCC `-fanalyzer` 日志为空，`git diff --check` 通过。跨容器完整文本范围和平台读屏桥接仍未验收。

2026-09-28 父列表项其他首块类型：`validation-2026-09-28-first-block-kinds-before.log` 复现标题首块跨父替换回退并改写未选中实体；来源探针 `validation-2026-09-28-first-block-kinds-probe.log`、`validation-2026-09-28-first-block-extensions-probe.log` 核对标题、分隔线、HTML、GFM 表格与 Mermaid 围栏的父项子节点和有序语法范围。精确补丁接纳这些已验证块，保留原列表标记及行结束符，跳过完整选中块后接回内层列表；候选重新解析并核对全文语义及全部 LINK 定义原拼写。Core `validation-2026-09-28-first-block-kinds-core-final.log` 覆盖上述五类、分行标题、来源重载、Undo/Redo 和 GFM 表格 309 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-first-block-kinds-editor.log` 覆盖表格首块 VISUAL→SOURCE 与历史。Windows 完整套件 `validation-2026-09-28-first-block-kinds-final-suite.log` 包括 652 项 CommonMark、Renderer/MessageList/原生绘制；Linux ASan/UBSan 与同一语料见 `validation-2026-09-28-first-block-kinds-linux-core.log`，均退出码零。修改源码 GCC `-fanalyzer` 日志为空，差异检查通过。其他块类型、非 LINK 定义和完整 CST 尚未获得通用保真证明。

2026-09-28 列表首块的长围栏首行：`validation-2026-09-28-long-fence-prefix-before.log` 复现超过 256 字节的围栏信息串使跨父源码补丁回退，选区外实体被改写。现先从有限前缀识别列表标记及围栏/Quote 起始，再在块来源中分块查找原行结束符；4096 字节扫描块末端的 `\r` 与下一块的 `\n` 仍按 CRLF 保留。Core `validation-2026-09-28-long-fence-boundary-core-final.log` 验证 320 字节与跨块 4092 字节信息串、源码拼写、重新载入语法、Undo/Redo；测试源码复制辅助函数已按实际长度扩容。完整 Windows 套件 `validation-2026-09-28-long-fence-final-suite.log` 覆盖实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制；Linux ASan/UBSan 和同一语料见 `validation-2026-09-28-long-fence-linux-core.log`，均退出码零。修改源码 GCC `-fanalyzer` 日志为空，差异检查通过。列表标记前缀本身仍受有限缓冲区约束。

2026-09-28 父列表项首块非段落：`validation-2026-09-28-parent-nonparagraph-before.log` 先复现代码块/Quote 首块跨父替换走结构回写后改变未选中的实体与列表来源；`validation-2026-09-28-parent-nonparagraph-deep-before.log` 复现三层列表中四格缩进的同类问题。精确补丁现保留分行列表标记，或从首块同行中提取原标记与行结束符；按语法范围跳过已选前导块，把完整 LINK 定义原字节移到保留标记之后，接回未选中的内层列表与后继兄弟。私有候选重新解析并核对全文语义与所有定义拼写。Core `validation-2026-09-28-parent-nonparagraph-deep-core.log` 验证代码块、Quote、LF/CRLF、已使用定义、多层嵌套、独立重载语法、Undo/Redo 和 345 点分配失败扫描；最终 Windows 套件 `validation-2026-09-28-parent-nonparagraph-final-suite.log` 覆盖实际 DLL Editor VISUAL→SOURCE、652 项 CommonMark、Renderer、MessageList 与原生绘制；Linux ASan/UBSan 及同一语料见 `validation-2026-09-28-parent-nonparagraph-final-linux-core.log`，均退出码零。修改源码 GCC `-fanalyzer` 日志为空，差异检查通过。其他首块类型和完整 CST 尚未建立通用保真路径。

同批可选 Windows WebView2 基础控件 `validation-2026-09-28-parent-nonparagraph-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-parent-nonparagraph-web-provider.log` 与离线测量/PNG `validation-2026-09-28-parent-nonparagraph-web-render.log` 均退出码零；通用 WebView 对外功能没有扩张。

同批可选 Windows WebView2 控件 `validation-2026-09-28-parent-middle-definition-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-parent-middle-definition-web-provider.log` 与离线测量/PNG `validation-2026-09-28-parent-middle-definition-web-render.log` 均退出码零；通用 WebView 的公开范围没有扩张。

2026-09-28 父列表项块间隐藏链接定义：`validation-2026-09-28-parent-middle-definition-before.log` 先复现父项段落、隐藏定义、第二段落和内层列表的跨父替换返回 `UNREPRESENTABLE`；`validation-2026-09-28-parent-middle-definition-candidates.log` 对照原输入与多种候选的解析树，确认定义原行紧随空父项标记且没有额外空行时仍属该列表项。精确来源路径逐段验证被略过的来源只含空白与完整 LINK 定义，复制定义全部原字节并按原顺序移到保留的父项标记之后；全文私有候选需匹配目标语义与所有定义拼写。Core `validation-2026-09-28-parent-middle-definition-core-final.log` 覆盖已使用/未使用、双定义、代码块、跨行标题、CRLF 有序列表、重新载入语法、Undo/Redo 和双定义路径 302 点分配失败扫描。实际 DLL Editor `validation-2026-09-28-parent-middle-definition-editor.log` 覆盖 VISUAL→SOURCE、撤销/重做；含 652 项 CommonMark 的 Windows 完整套件 `validation-2026-09-28-parent-middle-definition-suite.log`、Linux ASan/UBSan 与同一语料 `validation-2026-09-28-parent-middle-definition-linux-core.log` 均退出码零。修改源码 GCC `-fanalyzer` 日志为空，差异检查通过；非 LINK 定义及未识别来源仍不走此精确补丁。

2026-09-28 父列表项后续可见块：`validation-2026-09-28-parent-following-block-before.log` 先复现“首段落 + 第二段落 + 内层列表”跨父替换返回 `UNREPRESENTABLE`；`validation-2026-09-28-parent-following-block-probe.log` 核对段落/代码块及子列表的语法范围。新路径保留原列表标记、首段落换行、内层列表与选区外兄弟，删除选中的父项可见块，同时把由 loose 变为 tight 的父列表属性同步到语义目标。只接受按序块范围、纯空白间隙与无夹层隐藏定义；私有候选重新解析并核对全文语义和定义原文。Core `validation-2026-09-28-parent-following-block-core-final.log` 覆盖两段落、段落加围栏代码、三个前导块、CRLF 有序列表、前后兄弟、独立重载语法、Undo/Redo 和 295 点分配失败扫描；夹层隐藏定义仍原子拒绝。实际 DLL Editor `validation-2026-09-28-parent-following-block-editor.log` 的 VISUAL→SOURCE/Undo/Redo、Windows 完整套件 `validation-2026-09-28-parent-following-block-suite.log` 的 652 项 CommonMark、Renderer/MessageList/原生绘制、Linux ASan/UBSan 与同一语料 `validation-2026-09-28-parent-following-block-linux-core-final.log` 均退出码零。三个前导块用例在完整套件后加入，仅测试代码变化；最终 Core/Linux 包含它。修改的 Commands/Markdown 写回 GCC `-fanalyzer` 日志为空，差异检查通过。其他父项结构与完整 CST 保真仍未完成。

同批可选 Windows WebView2 控件 `validation-2026-09-28-parent-following-block-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML provider `validation-2026-09-28-parent-following-block-web-provider.log` 与离线测量/PNG `validation-2026-09-28-parent-following-block-web-render.log` 均退出码零；通用 WebView 仍限 Windows 基础网页承载，内部渲染通道保留。

最终补充边界：前置项两个可见段落之间夹来源层链接定义时，本批不做局部补丁；诊断日志 `artifacts/xui-document-rebuild/validation-2026-09-28-nested-multiblock-middle-definition-boundary.log` 记录 `UNREPRESENTABLE`、源码和定义计数不变，正式测试还核对 revision 不变。新增断言后的 Core `validation-2026-09-28-nested-multiblock-definition-core-final.log`、完整 Windows 套件 `validation-2026-09-28-nested-multiblock-definition-suite-final.log`、Linux ASan/UBSan 与 652 项 CommonMark `validation-2026-09-28-nested-multiblock-definition-linux-core-final.log` 均退出码零。

本轮完整验收：Windows 套件 `artifacts/xui-document-rebuild/validation-2026-09-28-nested-multiblock-definition-suite.log` 的实际 DLL、Renderer/Editor、MessageList、原生绘制与 652 项 CommonMark 均通过。可选 Windows WebView2 基础控件 `validation-2026-09-28-nested-multiblock-definition-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-multiblock-definition-web-provider.log`、离线测量/PNG `validation-2026-09-28-nested-multiblock-definition-web-render.log` 均退出码零。通用 WebView 的公开范围未扩张，Document 私有渲染通道继续保留。

2026-09-28 嵌套列表多可见块与尾随隐藏定义：`validation-2026-09-28-nested-multiblock-definition-before.log` 先复现前置项含两个段落时跨父替换返回 `UNREPRESENTABLE`；`validation-2026-09-28-nested-multiblock-definition-probe.log` 核对两个段落及段落加代码块的来源范围、定义所属项和预期来源重解析。结构删除现保留定义所在列表项并删去所有可见子块；精确来源补丁保留首段落的列表标记/换行与尾随定义原文，丢弃被选中块及空白间隙，并再次核对整篇语义与全部定义。Core `validation-2026-09-28-nested-multiblock-definition-core.log` 覆盖两段落、段落加围栏代码、CRLF 有序列表、重载语法、Undo/Redo 和新增路径 307 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-multiblock-definition-editor.log` 覆盖 VISUAL→SOURCE、撤销/重做；Linux ASan/UBSan 与 652 项 CommonMark `validation-2026-09-28-nested-multiblock-definition-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-multiblock-definition-analyzer.log` 均退出码零。首块非段落、可见块之间夹隐藏定义、非空白来源间隙及完整 CST 未获此项保证。

2026-09-28 嵌套列表隐藏定义前的空白来源：`validation-2026-09-28-nested-definition-gap-before.log` 先复现前置项段落与定义之间有多条空行时跨父替换返回 `UNREPRESENTABLE`。解析探针 `validation-2026-09-28-nested-definition-gap-probe.log` 显示定义紧随段落时被当作可见文本（定义数为零），多条空行及空格/Tab 空白行之后则仍被识别为独立链接定义。现在补丁验证整段间隙只含空白且在定义前以行结束符结尾，再按原字节保留定义和列表标记，略去会把列表拆开的间隙；完整候选仍需通过语义及定义原文核对。Core `validation-2026-09-28-nested-definition-gap-core.log` 覆盖 LF/CRLF 多空行、混合空白、非定义边界、重载语法、Undo/Redo 和新增路径 298 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-definition-gap-editor.log` 覆盖 VISUAL→SOURCE 与撤销/重做；含 652 项 CommonMark 的 Windows 完整套件 `validation-2026-09-28-nested-definition-gap-suite.log`、Linux ASan/UBSan 与同一语料 `validation-2026-09-28-nested-definition-gap-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-definition-gap-analyzer.log` 均退出码零。多个可见块或非空白间隙仍未覆盖。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-definition-gap-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-definition-gap-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-definition-gap-web-render.log` 均退出码零；通用 WebView 保持 Windows 基础网页承载，Document 的私有渲染通道保留。

2026-09-28 嵌套列表可见前项内隐藏定义：`validation-2026-09-28-nested-visible-definition-before.log` 先复现前置项的可见段落与后续隐藏定义一起被结构删除，跨父替换返回 `UNREPRESENTABLE`。`validation-2026-09-28-nested-visible-definition-probe.log` 对比候选来源的完整解析：留住多余空行会拆分列表，删去该行后定义原文仍被识别且列表保持一体，同时由 loose 转为 tight。结构命令现在只删除可见段落并更新该列表属性；精确补丁逐字节保留列表标记、定义及未选中来源，略去正文和这一条空行；整篇候选仍需通过语义和定义拼写核对。Core `validation-2026-09-28-nested-visible-definition-core.log` 覆盖 LF/CRLF、无序/有序列表、已使用/未使用定义、前后兄弟、重载语法、Undo/Redo 和新增路径 434 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-visible-definition-editor.log` 覆盖 VISUAL→SOURCE 与撤销/重做；含 652 项 CommonMark 的 Windows 完整套件 `validation-2026-09-28-nested-visible-definition-suite.log`、Linux ASan/UBSan 与同一语料 `validation-2026-09-28-nested-visible-definition-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-visible-definition-analyzer.log` 均退出码零。其他空白布局和更复杂前项仍待完成。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-visible-definition-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-visible-definition-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-visible-definition-web-render.log` 均退出码零；通用 WebView 保持 Windows 基础网页承载，Document 的私有渲染通道保留。

2026-09-28 嵌套列表前置隐藏定义：`validation-2026-09-28-nested-preceding-definition-before.log` 复现只含链接定义的前置空列表项被结构删除后返回 `UNREPRESENTABLE`；`validation-2026-09-28-nested-preceding-definition-range-probe.log` 核对定义落在空项完整语法范围内。现在保留该空项并按原字节重放其所在列表路径；被语义选区完整覆盖的可见前项仍删除。修复了仅搬运定义行、遗漏外层列表标记与目标项前缀的候选拼接错误，完整候选继续核对语义和定义原文。Core `validation-2026-09-28-nested-preceding-definition-core.log` 覆盖单个/多个定义项、交错可见项、有序标记、CRLF、重新载入语法、Undo/Redo 及新增路径 296 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-preceding-definition-editor.log` 覆盖 VISUAL→SOURCE 与撤销/重做；含 652 项 CommonMark 的 Windows 完整套件 `validation-2026-09-28-nested-preceding-definition-suite.log`、Linux ASan/UBSan 与同一语料 `validation-2026-09-28-nested-preceding-definition-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-preceding-definition-analyzer.log` 均退出码零。含可见正文并在其后带隐藏定义的前项、任意 CST 间隙仍待解决。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-preceding-definition-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-preceding-definition-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-preceding-definition-web-render.log` 均退出码零；通用 WebView 保持 Windows 基础网页承载，Document 的私有渲染通道保留。

2026-09-28 嵌套列表目标前的兄弟项：`validation-2026-09-28-nested-preceding-sibling-before.log` 先复现跨父选区落到内层列表第二项时精确路径退出，结构回写规范化列表并转义选区外实体；`validation-2026-09-28-nested-preceding-sibling-range-probe.log` 显示前项与目标项各有不重叠的完整语法范围。精确补丁现核对结构目标只删除列表前缀兄弟项、保留从目标项开始的原子节点后缀，复制来源时跳过被完整选中的前项范围。全文候选仍须重新解析，并与语义目标及引用定义拼写一致。Core `validation-2026-09-28-nested-preceding-sibling-final-core.log` 覆盖内层一项/多项与外层前项、可见父项/后继兄弟组合、有序列表、CRLF、重载语法、Undo/Redo 和新增组合路径 386 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-preceding-sibling-editor.log` 覆盖 VISUAL→SOURCE、撤销和重做。含 652 项 CommonMark 的 Windows 完整套件 `validation-2026-09-28-nested-preceding-sibling-suite.log`、Linux Core ASan/UBSan 与同一语料 `validation-2026-09-28-nested-preceding-sibling-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-preceding-sibling-analyzer.log` 均退出码零。前项中的隐藏定义与通用 CST 来源编辑不在这一已验证范围。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-preceding-sibling-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-preceding-sibling-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-preceding-sibling-web-render.log` 均退出码零；通用 WebView 仍限 Windows 基础网页承载。

2026-09-28 嵌套列表可见父项与后继兄弟：`validation-2026-09-28-nested-visible-parent-before.log` 先复现跨过父项正文时精确补丁退出、结构回写改写实体和列表源码。`validation-2026-09-28-nested-visible-parent-markup-range-probe.log` 显示含 `**outer**` 的段落 `source_start` 在开头定界符之后；单用该边界会把 `**` 留到空列表项中。补丁现通过列表标记语法和段落行结束符确定被选中正文的完整边界，保留原标记、空行与换行，允许目标项后有未选中兄弟项，并逐字节重放其来源。完整候选仍须重新解析并与结构目标及引用定义拼写相同。Core `validation-2026-09-28-nested-visible-parent-markup-after.log` 覆盖普通/有序标记、粗体定界符、CRLF、后继兄弟、重载语法、Undo/Redo 和两条路径 283/353 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-visible-parent-editor.log` 覆盖 VISUAL→SOURCE、撤销和重做。含 652 项 CommonMark 的 Windows 完整套件 `validation-2026-09-28-nested-visible-parent-suite.log`、Linux Core ASan/UBSan 与同一语料 `validation-2026-09-28-nested-visible-parent-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-visible-parent-analyzer.log` 均退出码零。更早的兄弟项、父项尾部结构及完整 CST 仍待处理。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-visible-parent-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-visible-parent-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-visible-parent-web-render.log` 均退出码零；通用 WebView 仍限 Windows 基础网页承载。

2026-09-28 空嵌套列表中的引用块：`validation-2026-09-28-nested-list-quote-before.log` 先复现跨父选区替换将分行的 `-` / `  - >` 合成 `- - >`、转义选区外实体并丢失末尾 `>`；`validation-2026-09-28-nested-list-quote-range-probe.log` 核对最外层列表与内层段落的语法起点不同。精确补丁现要求右侧连续 Quote/ListItem/List 路径只有被移走的可见叶节点，左侧块与列表之间只有空白，然后移动原始块间空白、列表/引用前缀及换行。候选必须全文重解析并与结构目标及定义原文一致。Core `validation-2026-09-28-nested-list-quote-matrix.log` 覆盖两层/三层列表、有序标记、CRLF、引用块内隐藏定义、语法重载、Undo/Redo 和新增路径 237 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-list-quote-editor.log` 覆盖 VISUAL→SOURCE、撤销和重做。含 652 项 CommonMark 的 Windows 完整套件 `validation-2026-09-28-nested-list-quote-suite.log`、Linux Core ASan/UBSan 与同一语料 `validation-2026-09-28-nested-list-quote-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-list-quote-analyzer.log` 均退出码零。带可见父项或额外兄弟节点的嵌套列表及完整 CST 不在这一已验证范围。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-list-quote-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-list-quote-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-list-quote-web-render.log` 均退出码零；通用 WebView 仍限 Windows 基础网页承载。

2026-09-28 列表项内多层 Quote：`validation-2026-09-28-nested-double-quote-before.log` 先复现双层引用合并后选区外实体被转义、空引用尾行丢失。精确补丁现检查右端连续 Quote 链在目标树中只剩空容器，要求原列表首行前缀包含相同数量的引用标记，并按原字节移动该前缀及换行；全文重新解析的语义与定义拼写校验不变。Core `validation-2026-09-28-nested-deep-quote-core.log` 覆盖双层/三层、CRLF、有序标记、引用块内隐藏定义、语法重载、Undo/Redo 和新增路径 226 点分配失败扫描；实际 DLL Editor `validation-2026-09-28-nested-deep-quote-editor.log` 验证 VISUAL→SOURCE、撤销和重做。Windows 完整套件 `validation-2026-09-28-nested-deep-quote-suite.log`、单独 Windows 652 项 CommonMark `validation-2026-09-28-nested-deep-quote-win-corpus.log`、Linux Core ASan/UBSan 与同一语料 `validation-2026-09-28-nested-deep-quote-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-deep-quote-analyzer.log` 均退出码零。更深的嵌套列表及通用 CST 来源编辑未由此证明。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-deep-quote-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-deep-quote-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-deep-quote-web-render.log` 均退出码零；通用 WebView 仍限 Windows 基础网页承载。

2026-09-28 列表内空 Quote 标记：`validation-2026-09-28-nested-quote-marker-before.log` 的实际输出把选区外 `&amp;` 改成转义、丢失 `  >` 尾行；`validation-2026-09-28-nested-quote-marker-range-probe.log` 确认右段落及 List/Quote 共用首行语法起点，正文前的原始前缀可独立截取。现将简单列表/引用前缀与原换行移到合并段落之后，保留尾行原字节，由候选全文重解析证明与结构目标一致。Core `validation-2026-09-28-nested-quote-marker-matrix.log` 验证无序 `-`/`+`、有序 `1.`、LF/CRLF、后继根段落、实体、独立重载语法及 Undo/Redo；215 点分配失败扫描保持旧 revision/源码且无泄漏。实际 DLL Editor `validation-2026-09-28-nested-quote-marker-editor.log` 通过 VISUAL→SOURCE 与撤销/重做。Windows 完整套件 `validation-2026-09-28-nested-quote-marker-suite.log` 通过 652 项 CommonMark、Renderer、MessageList 与原生绘制；Linux Core ASan/UBSan 和同一语料 `validation-2026-09-28-nested-quote-marker-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-quote-marker-analyzer.log` 均退出码零。多层嵌套、内部定义与其他复杂行前缀仍需单独验证。

同批可选 Windows WebView2 控件 `validation-2026-09-28-nested-quote-marker-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-nested-quote-marker-web-provider.log` 和离线测量/PNG `validation-2026-09-28-nested-quote-marker-web-render.log` 均退出码零。Document 内部渲染通道保留；通用 WebView 对外范围仍为 Windows 基础网页能力。

2026-09-28 空 Quote 来源标记：失败日志 `artifacts/xui-document-rebuild/validation-2026-09-28-cross-parent-quote-marker-before.log` 显示，跨容器合并后右侧只剩一行 `>` 时，旧路径删除该行并把选区外 `&amp;` 改写为 `\&`。现在结构影子树读取右段落之后、原 Quote 语法范围内的来源；尾部仅有空白与 `>` 且确有标记时保留空 Quote。候选仍必须完整重解析并与影子语义一致。Core `validation-2026-09-28-cross-parent-quote-marker-matrix.log` 覆盖 LF/CRLF、多行标记、后继根段落与普通无标记容器；184 点新增路径分配失败扫描无半成品发布或泄漏。`validation-2026-09-28-cross-parent-quote-marker-editor.log` 在实际 DLL 中核对 VISUAL 输入、SOURCE 切换、Undo/Redo；Windows 完整套件 `validation-2026-09-28-cross-parent-quote-marker-suite.log` 覆盖 652 项 CommonMark、Renderer、MessageList 和原生绘制。Linux Core ASan/UBSan 与同一语料 `validation-2026-09-28-cross-parent-quote-marker-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-cross-parent-quote-marker-analyzer.log` 均退出码零。嵌套列表语法标记及其他非空白来源空隙仍未由此证明无损。

同批可选 Windows WebView2 控件 `validation-2026-09-28-cross-parent-quote-marker-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-cross-parent-quote-marker-web-provider.log` 与离线测量/PNG `validation-2026-09-28-cross-parent-quote-marker-web-render.log` 均退出码零。Document 内部渲染通道保留；通用 WebView 对外范围仍为 Windows 基础网页能力。

2026-09-28 跨容器右端定义与空 Quote：`validation-2026-09-28-cross-container-prefix-before.log` 记录右引用块的隐藏定义位于选中段落前时的 `UNREPRESENTABLE`；`validation-2026-09-28-cross-container-prefix-empty-probe2.log` 确认仅剩定义的源码会解析为空 Quote，旧结构目标删除它导致核对失败。现将右容器的完整定义前缀按原字节移至合并段落后，并在含链接定义的右 Quote 成为空容器时保留其语义节点。`validation-2026-09-28-cross-container-empty-quote-suffix.log` 的核心矩阵通过右侧仍有后段、只剩前置/后置定义、已引用/未引用定义、选区外实体、独立全文重载与 Undo/Redo；三组分配失败扫描分别为 224、262、193 点，失败时 revision/源码不变且无泄漏。`validation-2026-09-28-cross-container-both-sides-suite.log` 通过实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；`validation-2026-09-28-cross-container-both-sides-linux-core.log` 通过 Linux Core ASan/UBSan 与同一语料，`validation-2026-09-28-cross-container-both-sides-analyzer.log` 的 GCC `-fanalyzer` 无诊断。此批只证明完整链接定义和容器空白的指定边界，不代表完整 CST 或任意跨表格结构选区。

同批可选 Windows WebView2 控件 `validation-2026-09-28-cross-container-both-sides-webview-build.log`、Document 内部 KaTeX/Mermaid/HTML 实窗 provider `validation-2026-09-28-cross-container-both-sides-web-provider.log`、离线测量和 PNG `validation-2026-09-28-cross-container-both-sides-web-render.log` 均退出码零。Document 内部通道保留；通用 WebView 对外范围仍为 Windows 基础网页能力。

2026-09-28 跨容器尾部隐藏定义：失败记录 `artifacts/xui-document-rebuild/validation-2026-09-28-cross-container-definition-before.log` 复现从引用块内文字选到外部段落时，尾部隐藏链接定义导致 `UNREPRESENTABLE`；`validation-2026-09-28-cross-container-definition-diagnostic.log` 的来源区间确认定义处于引用块语法范围内、正文之后。精确补丁现沿左端到公共祖先的容器路径提取完整定义间隙，连同根层间隙按原字节重放在合并段落之后；未选中 `&amp;` 保持原拼写，候选仍需全文语义和全部定义原拼写核对。Core `validation-2026-09-28-cross-container-definition-matrix-core.log` 验证引用块、列表项、列表中引用块、LF/CRLF、已引用与未引用定义、独立重载语法、Undo/Redo；224 点分配失败扫描保留旧发布状态且无泄漏。Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-28-cross-container-definition-linux-core.log` 以及 GCC `-fanalyzer` `validation-2026-09-28-cross-container-definition-analyzer.log` 均退出码零。本批不证明容器任意内部位置的定义或完整 CST 已无损。

Windows 完整套件 `validation-2026-09-28-cross-container-definition-suite.log` 包含实际 DLL Editor VISUAL 输入、SOURCE 切换及撤销/重做，652 项 CommonMark、Renderer、MessageList 与原生绘制，退出码零。可选 Windows WebView2 基础控件 `validation-2026-09-28-cross-container-definition-webview-build.log`、DocumentView/MessageList 内部 KaTeX/Mermaid/HTML provider `validation-2026-09-28-cross-container-definition-web-provider.log`、离线测量/PNG `validation-2026-09-28-cross-container-definition-web-render.log` 均通过；通用 WebView 的对外功能没有扩张。

2026-09-28 跨分支文字范围：核心失败用例 `artifacts/xui-document-rebuild/validation-2026-09-28-cross-branch-before.log` 复现含未选中兄弟节点时的 `UNSUPPORTED`；`validation-2026-09-28-cross-branch-definition-before.log` 复现根层隐藏定义导致结构回写改写选区外实体。现按最近公共块容器删除选区内部的完整子树、保留两端外侧的兄弟子树并合并端点段落；根层间隙中的完整链接定义按原字节迁移，Markdown 私有候选经全文重解析、语义和定义拼写核对。Core `validation-2026-09-28-cross-branch-core-final.log` 验证引用块保留前段、右侧引用块保留后段、列表前后项与合并项、完整中间代码块、已引用及未引用定义、Rich 节点身份、未选中文字与迁入文字的精确位置映射、全文重载语法和 Undo/Redo；跨表格行列原子拒绝。224 点分配失败扫描未发布部分源码/事务且无泄漏。Windows 完整套件 `validation-2026-09-28-cross-branch-suite.log` 含实际 DLL Editor VISUAL→SOURCE 跨分支替换、652 项 CommonMark、Renderer、MessageList 和原生绘制，退出码零；Linux Core ASan/UBSan 与同一语料最终复核 `validation-2026-09-28-cross-branch-linux-core-final.log`、GCC `-fanalyzer` `validation-2026-09-28-cross-branch-analyzer.log` 亦通过。复杂结构端点、跨表格行列和完整 CST 未由本批覆盖。

同批可选 Windows WebView2 基础控件 `validation-2026-09-28-cross-branch-webview-build.log`、DocumentView/MessageList 内部 KaTeX/Mermaid/HTML provider `validation-2026-09-28-cross-branch-web-provider.log` 与离线测量/PNG `validation-2026-09-28-cross-branch-web-render.log` 均退出码零；通用 WebView 对外功能未扩张。

2026-09-28 跨父容器文字范围合并：新增失败用例首先复现 `ReplaceRange` 在引用块/列表与根段落之间返回 `UNSUPPORTED`。内核现仅在两端到不同顶层块的路径没有未选中兄弟节点时，将右端剩余行内节点移到左段落并合并，中间完整顶层块一同删除；Markdown 精确端点优先直接修改选中源码字节，并用私有全文重解析、语义树及引用定义原拼写核对。独立 Core `artifacts/xui-document-rebuild/validation-2026-09-28-cross-parent-core.log` 覆盖引用/列表与根段落双向、独立引用块、完整选中的中间代码块、CRLF、外部隐藏定义、反向选区、Rich 节点身份、独立重载的语法元数据与 Undo/Redo；未选中兄弟节点明确拒绝，184 点分配失败扫描未发布部分状态且无泄漏。完整 Windows 套件 `validation-2026-09-28-cross-parent-suite.log` 覆盖实际 DLL、652 项 CommonMark、Renderer、Editor、MessageList 及原生绘制；新增的实际 DLL Editor VISUAL→SOURCE、Undo/Redo 用例另见 `validation-2026-09-28-cross-parent-editor.log`。Linux Core ASan/UBSan 与同一 652 项语料 `validation-2026-09-28-cross-parent-linux-core.log` 均退出码零；修改的命令、源码补丁及测试在 GCC `-fanalyzer` 的 `validation-2026-09-28-cross-parent-analyzer.log` 中无警告。该结果不覆盖含未选中分支的复杂跨父选区或完整 CST。

同批可选 Windows WebView2 基础控件 `validation-2026-09-28-cross-parent-webview-build.log`、DocumentView/MessageList 内部 KaTeX/Mermaid/HTML provider `validation-2026-09-28-cross-parent-web-provider.log` 与离线测量/PNG `validation-2026-09-28-cross-parent-web-render.log` 均退出码零；通用 WebView 对外功能未扩张。

2026-09-28 Markdown 脚注正文跨段落替换：原路径在成功合并后把选区外的 `&amp;` 改写为 `\&`，失败记录为 `artifacts/xui-document-rebuild/validation-2026-09-28-footnote-cross-block-before.log`。局部补丁现沿选区的同父段落向上识别脚注定义，允许其正文长度和字节改变，但核对原始标签及前缀；其他脚注/链接定义仍逐项比较完整源码，私有候选重新解析后与期望语义树比较。Core `validation-2026-09-28-footnote-cross-block-final-core.log` 覆盖脚注直接段落、嵌套引用、LF/CRLF、脚注外独立定义、独立重载的来源/语法元数据、Undo/Redo 和 231 点分配失败原子性/无泄漏。脚注内缩进的 `[unused]: ...` 由当前解析器当作可见正文，诊断 `validation-2026-09-28-footnote-gap-diagnostic.log` 显示来源表仅有脚注定义、语义搜索能找到 `unused`，因此跨越它的选区应删除该正文。Windows 完整套件 `validation-2026-09-28-footnote-cross-block-final-suite.log` 覆盖实际 DLL Editor VISUAL 输入及 SOURCE 切换、652 项 CommonMark、Renderer、MessageList 与原生绘制；Linux Core ASan/UBSan 和同一语料 `validation-2026-09-28-footnote-cross-block-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-footnote-cross-block-analyzer.log` 均退出码零。该批不证明脚注所有结构操作或完整 CST 已无损。

同批可选 Windows WebView2 基础控件 `validation-2026-09-28-footnote-cross-block-webview-build.log`、DocumentView/MessageList 内部公式、Mermaid、HTML provider `validation-2026-09-28-footnote-cross-block-web-provider.log`、离线测量与 PNG 截图 `validation-2026-09-28-footnote-cross-block-web-render.log` 均退出码零。通用 WebView 对外接口未扩张。

2026-09-28 Markdown 嵌套容器的跨段落保真：旧列表项路径保留了 `[unused]: /raw "T"`，却将选区外两个 `&amp;` 改写为 `\&`；具体失败见 `artifacts/xui-document-rebuild/validation-2026-09-28-nested-definition-before-detail.log`。局部源码补丁现支持同父列表项和引用块的相邻段落，允许引用块空行 `>` 前缀；若间隙只含完整链接定义与容器空白，则移到合并段落之后。私有候选重新解析并核对完整语义及全部引用定义原字节。独立 Core `validation-2026-09-28-nested-definition-matrix-core.log` 覆盖内层列表项、引用块、嵌套引用、纯引用空行、独立重载来源元数据、Undo/Redo，引用块 182 点分配失败注入保持旧源码/revision 且无泄漏。Windows 完整套件 `validation-2026-09-28-nested-definition-final-suite.log` 的实际 DLL Editor VISUAL→SOURCE、652 项 CommonMark、Renderer、MessageList、原生同步/异步绘制均通过；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-28-nested-definition-linux-core.log`、GCC `-fanalyzer` `validation-2026-09-28-nested-definition-analyzer.log` 也通过。未覆盖脚注等其他容器和跨父容器无损改写。

同批可选 Windows WebView2 基础控件 `validation-2026-09-28-nested-definition-webview-build.log`、DocumentView/MessageList 内部公式、Mermaid、HTML provider `validation-2026-09-28-nested-definition-web-provider.log`、离线测量与 PNG 截图 `validation-2026-09-28-nested-definition-web-render.log` 均退出码零。通用 WebView 的对外范围仍只包含基础网页承载。

2026-09-28 Markdown 隐藏链接定义的跨段落替换：旧路径虽然返回成功，却把未选中实体 `&amp;` 改写为 `\&` 并多写空行，见 `artifacts/xui-document-rebuild/validation-2026-09-28-hidden-definition-probe.log`。新路径仅在根层同父连续段落/标题、精确文本端点且块间非空白部分完全由解析器记录的链接定义覆盖时，保留完整定义间隙的原字节并移到合并块之后；私有候选完整重解析后核对语义树和定义原拼写。Core `validation-2026-09-28-hidden-definition-final-core.log` 覆盖未引用与已引用定义、反向范围、CRLF 双定义、独立全文重载元数据、Undo/Redo 及 165 点分配失败原子性/无泄漏；Windows 完整套件 `validation-2026-09-28-hidden-definition-final-suite.log` 覆盖实际 DLL Editor VISUAL 输入与 SOURCE 切换、652 项 CommonMark、Renderer、MessageList 和原生绘制，退出码零。Linux Core ASan/UBSan 和同一 652 项语料见 `validation-2026-09-28-hidden-definition-linux-core.log`，严格 GCC `-fanalyzer` 日志 `validation-2026-09-28-hidden-definition-analyzer.log` 为空；差异检查通过。其他容器内部定义和非定义语法间隙继续走保守结构路径，尚未获得完整 CST 保真保证。

同批可选 WebView2 DLL 基础控件 `validation-2026-09-28-hidden-definition-webview-build.log`、DocumentView/MessageList 的内部公式、Mermaid、HTML provider `validation-2026-09-28-hidden-definition-web-provider.log`、离线网页测量和 PNG 截图 `validation-2026-09-28-hidden-definition-web-render.log` 均退出码零。通用 WebView 对外功能没有扩张。

2026-09-28 Markdown 跨段落来源补丁：旧路径合并两个段落时改写选区外实体的失败见 `artifacts/xui-document-rebuild/validation-2026-09-28-cross-block-before.log`。新路径仅接纳同父连续段落/标题、精确端点与纯空白块间来源，在私有候选里替换字节并核对完整语义及引用定义拼写；其他情形回退结构写回。独立 Core `validation-2026-09-28-cross-block-final-core.log` 覆盖正反向、两段/三段、CRLF、标题、隐藏定义、来源坐标与全文重载元数据对照、Undo/Redo 和 162 点分配失败。Windows 完整套件 `validation-2026-09-28-cross-block-final-suite.log` 包含实际 DLL Editor 的 VISUAL 输入及 SOURCE 切换、652 项 CommonMark、Renderer、MessageList 与原生绘制，退出码零；Linux Core ASan/UBSan 与 652 项语料 `validation-2026-09-28-cross-block-linux-core.log` 通过。可选 WebView2 基础控件、Document 内部 provider 和离线网页渲染分别通过 `validation-2026-09-28-cross-block-webview-build.log`、`validation-2026-09-28-cross-block-web-provider.log` 与 `validation-2026-09-28-cross-block-web-render.log`。修改源码 GCC `-fanalyzer` 与差异检查通过。完整 CST 与跨父容器无损回写仍未完成。

2026-09-28 结构工具栏：引用切换、分隔线和空代码块插入进入统一 `QueryCommand`/`Execute` 与 Toolbar 块格式组，继续使用已有 Document 根事务。实际 DLL Editor 专项 `artifacts/xui-document-rebuild/validation-2026-09-28-structure-toolbar-editor.log` 覆盖 Rich/Markdown 的按钮状态、结构树、撤销、只读和 Markdown SOURCE/LIVE；完整 Windows 套件 `validation-2026-09-28-structure-toolbar-final-suite.log` 覆盖独立 Core、652 项 CommonMark、Renderer/Editor、MessageList 及原生绘制，均退出码零。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-28-structure-toolbar-view-sanitizer.log` 也通过，构建仍报既存 DatePicker 可能未初始化警告。可选 Windows WebView2 基础控件、Document 内部 provider 及离线渲染分别通过 `validation-2026-09-28-structure-toolbar-webview-build.log`、`validation-2026-09-28-structure-toolbar-web-provider.log`、`validation-2026-09-28-structure-toolbar-web-render.log`；Editor/Toolbar GCC `-fanalyzer` 日志无警告。跨父容器结构选区和工具栏自动宿主同步尚未验收。

图片占位原生像素与可选 WebView 复核：`examples/xui_document/main.c` 的 `--verify-image-placeholder` 在真实 XGE 后端渲染缺失 Markdown 图片，对预览区域的默认底色与边框像素计数并保存 `artifacts/xui-document-rebuild/native-image-placeholder.png`，截图已人工检查；专项日志 `validation-2026-09-28-image-native-placeholder.log` 和随后包含该模式的完整套件 `validation-2026-09-28-image-state-final-suite.log` 均退出码零。可选 WebView2 基础控件 `validation-2026-09-28-image-state-webview-build.log`、Document 内部 provider `validation-2026-09-28-image-state-web-provider.log` 与离线截图 `validation-2026-09-28-image-state-web-render.log` 也均通过。此验收不代表图片加载故障提示或所有对象状态的 GPU 像素矩阵已完成。

2026-09-28 图片资源状态：缺失图片未使用旧富文本的默认占位颜色及 alt 文本居中，先失败记录为 `artifacts/xui-document-rebuild/validation-2026-09-28-image-placeholder-before.log`。统一 Renderer 现默认绘制 RGBA(238,241,245,255) 底色、RGBA(170,180,192,255) 边框和 RGBA(90,100,112,255) 的居中裁剪 alt；样式表显式覆盖仍生效。`xui_document_style_test.c` 在实际 DLL 上验证缺失、有效 surface、错误资源类型、替换和移除的重绘，以及有效图像与占位两种状态的对象选中叠加；资源变化不增加 Document revision。专项 `validation-2026-09-28-image-state-style.log`、完整 Windows 套件 `validation-2026-09-28-image-state-final-suite.log` 均退出码零，后者含 652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制；Linux 共享 Renderer ASan/UBSan 为 `validation-2026-09-28-image-state-renderer-sanitizer.log`，也通过。Renderer GCC `-fanalyzer` 与差异检查通过。

普通外框批次再次构建可选 Windows WebView2 DLL，基础控件与真实窗口见 `validation-2026-09-28-style-frame-webview-build.log`；Document 内部 View/MessageList 公式、Mermaid、HTML 静态承载见 `validation-2026-09-28-style-frame-web-provider.log`，离线测量/PNG 截图见 `validation-2026-09-28-style-frame-web-render.log`。三项退出码均为零，通用 WebView 的公开接口未扩张。

2026-09-28 普通外框跟进：`document.border.color` 原先注册但没有在普通 View/Editor 外框绘制；现于文档绘制后描边，Editor 的焦点边框最后覆盖它，IME 预编辑投影走相同普通边框路径。`xui_document_style_test.c` 的 Rich/Markdown 两类视图均核对该主题色。表格矩形选区用例将单元格边框与控件外框设为不同颜色，分别断言四个 Cell 和一个 View 外框，避免原颜色复用导致旧计数失效。专项 `artifacts/xui-document-rebuild/validation-2026-09-28-style-frame-style.log`、`validation-2026-09-28-style-frame-editor.log` 与完整 Windows 套件 `validation-2026-09-28-style-frame-final-suite.log` 均退出码零，后者包含 652 项 CommonMark、实际 DLL/Renderer/Editor/MessageList 和原生绘制；Linux View/Editor ASan/UBSan `validation-2026-09-28-style-frame-view-sanitizer.log` 通过，DatePicker 仍有未改动的可能未初始化告警。View/Editor GCC `-fanalyzer` 与差异检查通过。

统一内核和原生显示、编辑基础已经实现。整个重构方案尚未达到方案规定的最终发布标准或成熟前端 Markdown 编辑器的全部功能。新实现已加入主 DLL；旧 RichDocument/RichEdit 的公开 API、实现和 DLL 导出已移除，行为迁移仍待完成。2026-09-24 起，通用 WebView 当前只验收 Windows 基础网页承载，不要求公开数据通信；Document 内部公式/Mermaid/HTML 静态渲染通道继续单独验收。其他平台的 WebView 后端留待逐个实现。

2026-09-28 富文本主题矩阵迁移：`xui_document_style_test.c` 用真实统一 Rich Document 构造文字、显式颜色、规则线、顶层引用、含表头与普通 Cell 的表格及缺失图片；代理逐项记录主题文字、背景、引用/规则、段落、表格、图片占位/边框/alt 和焦点边框实际绘制颜色。主题再次改变引用线、表头和图片边框时，新颜色出现，Renderer shaped 字节数、几何和 Document revision 不变；Markdown VISUAL 仍验证链接、高亮、代码、引用与规则，SOURCE/LIVE 查找继续通过。属性尚未注册的先失败日志为 `artifacts/xui-document-rebuild/validation-2026-09-28-style-palette-before.log`；首次实现后发现顶层引用线缺失的诊断为 `validation-2026-09-28-style-palette-diagnostic.log`；修复后的专项为 `validation-2026-09-28-style-palette-second.log`。90 KiB 普通及含行内图片的 MessageList 文档在字体回调切换与恢复后保留深处阅读位置，见 `validation-2026-09-28-font-anchor-message.log`。完整 Windows 套件 `validation-2026-09-28-style-palette-final-suite.log` 含独立 Core、实际 DLL/导出表、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制，退出码零；Linux Renderer/View ASan/UBSan 为 `validation-2026-09-28-style-palette-renderer-sanitizer.log` 与 `validation-2026-09-28-style-palette-view-sanitizer.log`，均退出码零；View 构建仍有未改动的 DatePicker 编译告警。相关 C 源码 GCC `-fanalyzer` 与差异检查通过。更复杂的主题/对象状态和真实 GPU 像素矩阵仍待验收。

2026-09-28 外部字体回调失效验证：`onFont` 从 14px 改选 24px、保持 context 默认字体指针、DPI 和布局宽度不变时，独立 VISUAL Renderer 的 100 KiB 深处光标与全新 Renderer 对照一致，并重新 shape；48 KiB SOURCE 准备态和 LIVE 活动区的深处光标在调用 `InvalidateFonts` 前仍是旧行高，调用后与新建 SOURCE Renderer 一致，改宽后仍一致。固定高度 DocumentView 的 300 行 SOURCE/LIVE 在回调切换后保持同一可见源码行，内容高度随字体变化；MessageList 绑定 Document 的行高及后继消息位置更新，解绑与不存在消息返回 `NOT_FOUND`。Windows 规模、消息和完整回归日志分别为 `artifacts/xui-document-rebuild/validation-2026-09-28-font-invalidate-scale.log`、`validation-2026-09-28-font-invalidate-message.log`、`validation-2026-09-28-font-invalidate-final-suite.log`；最后一项含独立 Core、实际 DLL/导出表、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制，均退出码零。Linux 共享 Renderer/View ASan/UBSan 日志为 `validation-2026-09-28-font-invalidate-renderer-sanitizer.log` 和 `validation-2026-09-28-font-invalidate-view-sanitizer.log`，均退出码零；View 构建仍有未改动的 DatePicker `-Wmaybe-uninitialized` 告警。相关源文件 GCC `-fanalyzer` 和 `git diff --check` 通过。旧字体回调批次中的“缺少外部通知”限制由本批解决；可打印 ASCII 的自定义内容依赖 shape 行高、大量 Unicode 前驱行的首次精确查询延迟及真实平台字体热替换仍未覆盖。

可选 WebView 回归：Windows WebView2 DLL 基础 widget 构建和真实窗口用例 `artifacts/xui-document-rebuild/validation-2026-09-28-font-invalidate-webview-build.log` 通过；Document 内部 `xuiDocumentWebProvider` 的 View/MessageList 公式、Mermaid、HTML 静态承载 `validation-2026-09-28-font-invalidate-web-provider.log` 与离线测量/PNG 截图 `validation-2026-09-28-font-invalidate-web-render.log` 均通过。此项不增加通用 WebView 的公开脚本、截图或消息接口。

2026-09-28 SOURCE/LIVE 变高字形冷查询验证：测试代理把中文字符的 shape 行高变为基础字体度量三倍；旧实现的 SOURCE 冷命中与 LIVE 冷光标相对已布局 Renderer 偏移，分别见 `artifacts/xui-document-rebuild/validation-2026-09-28-source-unicode-height-before.log`、`validation-2026-09-28-live-unicode-height-before.log`。新源码行目录标记非 ASCII/控制字符行；SOURCE 行树聚合未确认行高，LIVE 平面块目录保留最早未确认位置，几何查询只精排目标之前的这类行。Windows 规模专项 `validation-2026-09-28-source-live-unicode-exe-verified.log` 验证 SOURCE/LIVE 冷/热深处光标、SOURCE 冷命中和选区矩形、前置 Unicode 行插入后的增量复用、改宽后实际行高变化、失败注入后的重试，以及少量局部 shaping；Linux 共享 Renderer ASan/UBSan `validation-2026-09-28-source-live-unicode-linux-renderer.log` 用独立无窗口代理执行 SOURCE/LIVE 冷查询。完整 Windows 套件 `validation-2026-09-28-source-live-unicode-final-suite.log` 含独立 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 和原生同步/异步绘制，退出码零；GCC `-fanalyzer` 与差异检查通过。可打印 ASCII 若由自定义 shaping 产生内容依赖的行高仍只用字体度量估高；大量非 ASCII 前驱行首次精确查询的延迟也尚未单独验收。

2026-09-28 SOURCE/LIVE 字体行高验证：`onFont` 返回 24px 字体而基础字体为 14px 时，冷 SOURCE 光标原与已测量 Renderer 的纵坐标不同；前置源码插入的临时行目录也未传入 context/proxy，改宽后仍留旧字体行高。旧行为在 `artifacts/xui-document-rebuild/validation-2026-09-28-source-callback-cold-before.log` 和 `validation-2026-09-28-source-callback-resize-before.log` 失败。修复后收集范围按实际回调字体度量估高，SOURCE/LIVE 增量目录传递渲染上下文，改宽或字体资源失效时重估未测量行；48 KiB 源码长行的冷/热、增量、LIVE 活动区和改宽对照见 `validation-2026-09-28-source-callback-final-scale.log`，仍只局部 shaping。Linux 共享 Renderer ASan/UBSan `validation-2026-09-28-source-callback-linux-renderer.log` 和完整 Windows 套件 `validation-2026-09-28-source-callback-final-suite.log` 均退出码零；完整套件含独立 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 与原生同步/异步绘制。GCC `-fanalyzer` 与 `git diff --check` 通过。该批尚未覆盖内容依赖的 shape 行高变化，非 ASCII/控制字符已由上方批次补齐；回调外部状态无通知的字体切换仍可能产生冷估算误差。

2026-09-27 无空格长词前缀排版专项：旧行为由 `artifacts/xui-document-rebuild/validation-2026-09-27-unbroken-prefix-before.log` 复现，128 KiB 连续英文段落首屏 shape 全文。最终规模专项 `validation-2026-09-27-unbroken-prefix-script-safe-scale.log` 验证首屏 shape 8,192/131,072 字节，深处光标/命中与完整布局一致；跨样式 run 的连续英文及 ASCII 基字加组合附标按字素续排，阿拉伯连写样本保持完整 shaping。Linux 共享 Renderer 的实际 ASan/UBSan 无窗口测试 `validation-2026-09-27-unbroken-prefix-final-linux-renderer.log` 覆盖连续长词的前缀和深处命中，退出码零；GCC `-fanalyzer` 记录为 `validation-2026-09-27-unbroken-prefix-script-safe-analyzer.log`，无警告。受保护连写脚本和代码块无硬换行长行仍需单独处理。

同批最终 Windows 完整套件 validation-2026-09-27-unbroken-prefix-script-safe-final-suite.log 覆盖独立 Core、652 项 CommonMark、实际 DLL 导出、Renderer/Editor、MessageList 和原生同步/异步绘制，退出码零。

2026-09-27 查找选项与捕获组替换专项：`FindEx` 在字面/正则模式增加 Unicode 整词过滤；View 的选区范围随 revision 和模式映射，查找窗口提供“整词”“仅选区”；正则 `ReplaceAllEx` / `ReplaceCurrentEx` 支持 `$0`、`$1`、`${name}`、`$$` 模板。独立 Core `artifacts/xui-document-rebuild/validation-2026-09-27-find-capture-oom-core.log` 验证跨样式/Unicode/范围边界、Rich/Markdown SOURCE、撤销、无效模板原子性，以及 Markdown 语义模式 143 点、SOURCE 模式 86 点分配失败扫描；实际 DLL Editor `validation-2026-09-27-find-complete-editor.log` 验证窗口选项、选区限定、当前/全部捕获替换与 Undo，`validation-2026-09-27-find-mode-scope-editor.log` 补验 SOURCE↔VISUAL 模式切换后选区范围仍生效。完整 Windows 套件 `validation-2026-09-27-find-options-final-suite.log` 的 652 项 CommonMark、发布导出、Renderer、Editor、MessageList 和原生绘制均通过，退出码零。Linux Core ASan/UBSan 与 652 项语料 `validation-2026-09-27-find-options-linux-core.log`、共享 Renderer `validation-2026-09-27-find-options-linux-renderer.log`、View/Editor 三模式 `validation-2026-09-27-find-options-linux-view.log` 均退出码零；Unicode 专项 `validation-2026-09-27-find-options-unicode.log` 与修改源码的 GCC `-fanalyzer` `validation-2026-09-27-find-options-analyzer.log` 也通过。Linux View 编译仍有既存 `src/xui_date_picker.c` 的可能未初始化变量警告。整词模式超过 `INT_MAX` 字节投影返回 LIMIT；大文档查找 UI 延迟尚未验收。

2026-09-27 查找扩展专项：`FindEx` 与 `ReplaceAllEx` 为统一 Document 的 VISUAL 语义文本和 Markdown SOURCE 源码增加正则、大小写不敏感匹配；View/Editor 共享查询选项，查找窗口可切换选项并处理无效表达式。`artifacts/xui-document-rebuild/validation-2026-09-27-find-regex-core-final.log` 的独立 Core 用例覆盖跨样式 Rich 命中、Unicode 大小写、限定范围锚点、零宽正则、Rich/Markdown 替换及 Undo、无效表达式不发布事务，退出码零。实际 DLL Editor 专项 `validation-2026-09-27-find-regex-editor.log` 验证窗口复选框、结果表、错误提示、正则全部替换与 Undo，退出码零；完整套件 `validation-2026-09-27-find-regex-final-suite.log` 覆盖 652 项 CommonMark、发布导出表、Renderer、Editor、MessageList 及原生绘制，退出码零。Linux GCC 15.2 的 Core ASan/UBSan 与 652 项语料记录为 `validation-2026-09-27-find-regex-linux-sanitizer.log`，两个测试程序直接复跑退出码均为零；`validation-2026-09-27-find-regex-linux-view-sanitizer.log` 的 Rich/Markdown View/Editor 三模式无窗口代理也退出码零。后者编译时仍报告与本批无关的 `src/xui_date_picker.c` 可能未初始化变量警告。静态 C/GCC `-fanalyzer` 与差异检查通过。正则替换当前只插入字面量，整词、限定选区 UI 和捕获组模板仍待实现。

2026-09-27 Document Web 截图失败与回收验证：测试代理在真实 WebView2 截图完成后先创建 GPU surface，再分别让 PNG 解码和内容裁剪调用返回错误。`artifacts/xui-document-rebuild/validation-2026-09-27-web-crop-provider-v2.log` 两阶段都核对 `injection=1`、`release=1`、`failed` 增长、`queued=0`，并在 MessageList 中绘制 `Document renderer screenshot failed` / `Document renderer screenshot crop failed`；修改 Document 源码后，两次均恢复正常公式绘制，worker 未进入永久失败。`build/webview/xui_document_web_decode_error.png` 和 `xui_document_web_crop_error.png` 已人工检查。实现同时避免裁剪失败时把整个浏览器 viewport 错当已完成的对象缓存。可选 WebView2 DLL/基础控件 `validation-2026-09-27-web-crop-build.log` 和含 652 项 CommonMark、实际 DLL/Renderer/Editor/MessageList/原生绘制的完整 Document 套件 `validation-2026-09-27-web-crop-final-suite.log` 均退出码零；严格 GCC `-fanalyzer` 与差异检查通过。该注入不覆盖截图请求 API 拒绝、异步捕获超时、结果尺寸不符或缓存额度耗尽。

2026-09-27 Document Web 单对象超时验证：真实 WebView2 页面报告渲染依赖已就绪，但故意不响应渲染请求。`artifacts/xui-document-rebuild/validation-2026-09-27-web-timeout-provider-final.log` 验证超过 600 次更新后该公式从等待转为失败，MessageList 绘制断行后的 `Document renderer response timed out`，`failed=1`、`queued=0`、`bWorkerFailed=0`；截图 `build/webview/xui_document_web_timeout.png` 已人工检查。实现还给截图请求持续暂不可用、截图任务超时/失败、尺寸不符和缓存不足提供具体错误原因，并终结当前对象等待。可选 WebView2 DLL/基础控件 `validation-2026-09-27-web-timeout-build.log` 和含 652 项 CommonMark、实际 DLL/Renderer/Editor/MessageList/原生绘制的完整 Document 套件 `validation-2026-09-27-web-timeout-final-suite.log` 均退出码零；严格 GCC `-fanalyzer` 与差异检查通过。实际故障注入只覆盖脚本无响应；截图及缓存失败路径尚未分别注入验证。

2026-09-27 Document Web worker 永久故障验证：人为移除页面依赖资源，旧 DLL 的 `xuiDocumentWebProviderUpdate` 返回 `-8`，帧更新中断，见 `artifacts/xui-document-rebuild/validation-2026-09-27-web-worker-before.log`。修复后真实 WebView2/MessageList 用例 `validation-2026-09-27-web-worker-provider-final.log` 验证缺失资源变成带原因的错误卡片（`failed=1`、标题和详情均绘制），同一 provider 上修改 Document 源码后新对象也直接显示错误（`failed=2`、`queued=0`），故障后切换调色板仍绘制 13,272 个填充像素。`GetStats` 能区分 worker 永久失败及其原因；`Update` 不再把已记录故障传播为应用帧错误。三张窗口截图 `build/webview/xui_document_web_assets_error.png`、`xui_document_web_assets_future.png`、`xui_document_web_assets_dark.png` 已人工检查。可选 WebView2 DLL/基础控件 `validation-2026-09-27-web-worker-build.log`、离线网页渲染 `validation-2026-09-27-web-worker-render.log`、含 652 项 CommonMark、实际 DLL/Renderer/Editor/MessageList/原生绘制的完整套件 `validation-2026-09-27-web-worker-final-suite.log` 均退出码零。该故障注入仅覆盖资源缺失；其他页面/导航事件、截图失败、单对象超时、辅助技术和完整平台矩阵仍待验收。

2026-09-27 Document 内部 Web provider 错误显示验证：新增真实 WebView2 窗口中的无效 KaTeX 公式，旧 DLL 的先失败记录 `artifacts/xui-document-rebuild/validation-2026-09-27-web-error-before.log` 在 900 帧内没有可读错误。修复后 provider 保留浏览器返回的原因，并用 XUI 自身的测量、绘制回调在 MessageList 中生成按字体宽度分行的错误卡片。`validation-2026-09-27-web-error-provider-final.log` 验证错误标题、KaTeX 错误前缀和出错命令均被绘制，暗色卡片 16,571 个填充像素，消息行高为 165；把源码修正后，公式再次实际渲染。目标截图 `build/webview/xui_document_web_error.png` 已人工检查。可选 WebView2 DLL/基础控件 `validation-2026-09-27-web-error-wrap-build.log`、离线 KaTeX/Mermaid/HTML 渲染 `validation-2026-09-27-web-error-render.log`、含 652 项 CommonMark 与实际 DLL/Renderer/Editor/MessageList/原生绘制的完整套件 `validation-2026-09-27-web-error-final-suite.log` 均退出码零。这里只证明单对象脚本错误的可见呈现与恢复；启动/截图超时的具体诊断、HTML 交互、辅助技术及跨平台 WebView 尚未验收。

同一真实窗口测试随后把 Document 源码改为无效 Mermaid 围栏、超尺寸 HTML 块，`validation-2026-09-27-web-error-provider-final.log` 分别核对 `Mermaid error` 与 `No diagram type detected`、`HTML error` 与 `HTML size exceeds limit` 的绘制及暗色填充像素。`build/webview/xui_document_web_mermaid_error.png` 和 `build/webview/xui_document_web_html_error.png` 已人工检查，三类脚本错误都能在 MessageList 中直接阅读。此测试仍不覆盖浏览器自身不可用时的启动或捕获故障。

2026-09-27 Linux View/Editor 无窗口验证：`test_xui/build_document_view_sanitizer.sh` 从正式 `xui_sources.bat` 和统一 Document 清单构建 XUI 控件，排除平台 XGE 代理与 WebView，以测试代理运行真实 View/Editor。`artifacts/xui-document-rebuild/validation-2026-09-27-linux-view-sanitizer-final.log` 退出码零：Rich View 文档订阅更新、Markdown View VISUAL/SOURCE、Rich Editor 插入与撤销、Markdown Editor SOURCE/LIVE/VISUAL 插入与撤销、布局、命中和绘制在 Linux GCC 15.2 的 ASan/UBSan 下通过。PNG 入口在测试中设为触发即失败，因而图片/剪贴板并未验收；真实 XGE 后端、平台 IME/读屏和完整交互矩阵也仍待完成。全量 XUI 编译保留一项 DatePicker 的 `-Wmaybe-uninitialized` 非致命警告，未把它计为 Document sanitizer 错误。

2026-09-27 Linux 独立内核及共享 Renderer 验证：`test_xui/build_document_core_sanitizer.sh` 从 Windows 构建的 `xui_document_sources.bat` 自动提取核心源码，在 WSL/Linux GCC 15.2 下启用 ASan/UBSan 编译运行 Core 与可选 CommonMark 语料。实际调用 `bash test_xui/build_document_core_sanitizer.sh artifacts/xui-document-rebuild/commonmark-0.31.2.json` 通过，日志为 `artifacts/xui-document-rebuild/validation-2026-09-27-linux-core-sanitizer-script.log`；Core 故障注入、增量与来源映射以及 652 项 CommonMark 解析/来源/原生往返均未报告 sanitizer 错误。`test_xui/build_document_renderer_sanitizer.sh` 在相同内核上运行无窗口 Rich/Markdown Renderer 代理用例，布局、光标、命中、绘制及 ASan/UBSan 通过，见 `validation-2026-09-27-linux-renderer-sanitizer-script.log`。该证据不覆盖完整 View/Editor、真实 XGE GPU、WebView 和平台输入。

2026-09-27 纯颜色更新缓存验证：旧 Renderer 对整段文字仅改前景/底色仍释放所在块并重新 shaping，失败复现为 `artifacts/xui-document-rebuild/validation-2026-09-27-paint-only-before.log`。现在没有结构、正文、资源或度量变化的 Text/Paragraph/Heading/Cell 属性事务保持已测量块，按新快照更新 run 的有效颜色和背景框颜色；段落排版时保留零色背景框几何，后续从无底色变为有底色也无需重排。实际 DLL Renderer `validation-2026-09-27-paint-only-inheritance-renderer.log` 验证文字前景/底色、段落底色、显式文字色清除后的祖先颜色继承，像素变化、光标位置和 shaped 字节数保持；`validation-2026-09-27-paint-only-cell-renderer.log` 进一步验证 Cell 底色及继承文字色；MessageList `validation-2026-09-27-paint-only-message-audit.log` 验证长段落整段 run 颜色变化不增加 shaping 或移动滚动。加入 Cell 用例后的完整套件 `validation-2026-09-27-paint-only-cell-final-suite.log`、8,192 条消息审计 `validation-2026-09-27-paint-only-final-audit.log`、可选 Windows WebView2 DLL `validation-2026-09-27-paint-only-webview-build.log` 和公式/Mermaid/HTML 内部真实窗口 provider `validation-2026-09-27-paint-only-web-provider.log` 均通过；严格 C/GCC `-fanalyzer` 与差异检查通过。局部选区纯颜色变更若拆分 Text run 仍会重排受影响块，本批不能视作设计中所有颜色场景完成。

2026-09-27 MessageList 富文本样式锚点验证：单个长段落的前缀字号变化在旧实现中导致深处可见文字行跳动，复现于 `artifacts/xui-document-rebuild/validation-2026-09-27-message-same-block-style-before.log`。现在 Document 变更回调先在旧 Renderer 快照上捕获可见文字位置，再映射到新 revision，后续布局精排受影响前缀并校正列表滚动。专项覆盖多段落前置字号变化、同段落前缀重排，以及连续两次提交后才布局；80 段消息的已测量块从 13 增至 18，没有精排整段前缀。替换绑定或消息节点时清理待应用锚点。完整 Document 套件 `validation-2026-09-27-message-style-final-suite-v2.log` 含 652 项 CommonMark、Core、实际 DLL、Renderer、Editor、MessageList 与原生同步/异步绘制，退出码零；独立 `validation-2026-09-27-message-style-final-audit-v2.log` 包含新增锚点用例和 8,192 条普通消息规模断言。可选 Windows WebView2 DLL `validation-2026-09-27-message-style-webview-build.log` 以及内部公式/Mermaid/HTML 真实窗口 provider `validation-2026-09-27-message-style-web-provider.log` 通过；严格 C/GCC `-fanalyzer` 和差异检查通过。真实字体、其他对象、复杂样式和物理 DPI 的 MessageList 重排仍待验收。

2026-09-27 行内样式拆分与同块锚点验证：部分文字的字号设置会产生 Text INSERT/SPLIT 结构操作，旧 View 因 STRUCTURE 标记未启动锚点，长段落前缀放大后仍使用旧滚动值；复现见 `artifacts/xui-document-rebuild/validation-2026-09-27-style-same-block-before.log`。现在只对配对的文字拆分复用视觉块目录，重新测量受影响块，并用 ChangeSet 映射的可见文字位置校正滚动；同段落深处文字行的视口相对高度在重排后保持。Renderer 用例确认其他已测量块不再重新 shaping，若同一事务另有段落插入仍完整重建。字体族解析回调返回不同度量字体时，视口上方已测量和未测量段落的阅读锚点与 Undo 也通过。实际 DLL Renderer 日志 `validation-2026-09-27-style-split-cache-renderer.log` 和完整套件 `validation-2026-09-27-style-inline-split-final-suite.log` 通过；完整套件含 652 项 CommonMark、Core、Renderer、Editor、MessageList 及原生同步/异步绘制。可选 Windows WebView2 DLL 控件 `validation-2026-09-27-style-inline-split-webview-build.log` 与 Document 内部公式/Mermaid/HTML 真实窗口 provider `validation-2026-09-27-style-inline-split-web-provider.log` 也通过。修改源码及测试通过严格 C、GCC `-fanalyzer` 和差异检查。真实平台字体、其他模式、复杂同块重排及物理 DPI 仍待验收。

2026-09-27 富文本 VISUAL 样式变更阅读锚点验证：固定高度 View 在字号事务发布前捕获可见文字位置，通过 ChangeSet 映射到新 revision；发布后先精排受影响的前置块，再按新几何调整滚动。修改视口上方已测量和未测量的段落时，字号放大后仍命中原来的可见文字，Undo 也保留阅读位置；未测量块转为实测后，Undo 的绝对滚动数值不必回到原估算值。纯颜色修改跳过额外离屏 shaping。原实现由 `artifacts/xui-document-rebuild/validation-2026-09-27-style-anchor-before.log` 复现失败；实际 DLL Renderer 日志 `validation-2026-09-27-style-anchor-renderer.log` 通过。完整套件 `validation-2026-09-27-style-anchor-final-suite.log` 含 652 项 CommonMark、Core、DLL、Renderer、Editor、MessageList 与原生同步/异步绘制，退出码零；修改源码和测试的严格 C/GCC `-fanalyzer`、差异检查通过。字体族变化、Markdown 模式、复杂同块重排和物理 DPI 尚未由本用例验收。

2026-09-27 Markdown 空列表项取消验证：`xuiDocumentTxnUnlistRange` 对没有语义子块的空项，以及仅含无语义空段落的任务项，删除列表标记和空段落，不产生伪造的顶层段落；其他选中项的实际子块照常提升。返回选区在零块时折叠到父容器 gap，混合范围按实际提升块计算。Core `artifacts/xui-document-rebuild/validation-2026-09-27-empty-unlist-core.log` 覆盖单项、中间项、混合项、有序编号、相邻和嵌套列表、任务项、精确 Undo；空中间项与任务项分别通过 124/85 点分配失败的来源/树/历史原子性与回收扫描。实际 DLL Editor 日志 `validation-2026-09-27-empty-unlist-editor.log` 验证列表按钮激活后再次点击取消、落点与 Undo。完整套件 `validation-2026-09-27-empty-unlist-final-suite.log` 覆盖 652 项 CommonMark、发布导出、Renderer、Editor、MessageList 及原生同步/异步绘制，退出码零；修改源码 GCC `-fanalyzer` 与 `git diff --check` 通过。复杂来源边界和跨父容器选区仍待补齐。

2026-09-27 同父块移动验证：`xuiDocumentTxnMoveBlockRange` 可把连续兄弟块或列表项上移/下移一位，结构 gap 保留方向与范围，文字/列表项/表格 Cell 内段落保留 NodeId 和一步 Undo；Markdown 通过同一候选源码回写和语义核对。Core `artifacts/xui-document-rebuild/validation-2026-09-27-block-move-core.log` 覆盖 Markdown 段落、引用、列表项，Rich 列表项、表格、列表内 Cell、边界拒绝，以及段落光标 108 点、根结构 gap 108 点、列表项 145 点的分配失败原子性/回收扫描。实际 DLL Editor 日志 `validation-2026-09-27-block-move-editor.log` 验证命令状态、Alt+上方向键、下移、Undo 和 SOURCE 禁用；发布导出表包含新事务 API。完整套件 `validation-2026-09-27-block-move-final-suite.log` 含 652 项 CommonMark、Renderer、Editor、MessageList 与原生同步/异步绘制，退出码零；GCC `-fanalyzer` 和 `git diff --check` 通过。跨父容器移动、复杂多视图选区及真实平台输入仍需验收。

2026-09-27 完整列表结构选区验证：同父容器的 gap 可选择一个或多个完整列表，`xuiDocumentTxnSetListStyleRange` 转换列表样式，`xuiDocumentTxnUnlistRange` 取消并把返回 gap 映射到提升后的块范围；反向选区和未选中的 Markdown 前后源码保留。Rich 嵌套列表在列表项内选中直接子列表时保持外层列表项身份，直接选中嵌套列表自身的非折叠 gap 也只操作该列表；折叠 gap 没有选中项时明确拒绝。夹杂普通块的结构选区拒绝。独立 Core 日志 `artifacts/xui-document-rebuild/validation-2026-09-27-list-parent-gap-core.log` 包含身份/Undo 与 199/200 个分配失败点的原子性和回收；实际 DLL Editor 日志 `validation-2026-09-27-list-parent-gap-editor.log` 覆盖按钮状态、切换/取消和选区映射。最终完整套件 `validation-2026-09-27-list-parent-gap-final-suite.log` 覆盖 652 项 CommonMark、发布导出、Renderer、Editor、MessageList 与原生同步/异步绘制，退出码零；GCC `-fanalyzer` 和 `git diff --check` 通过。跨父容器范围仍未实现；Markdown 无语义空项的限制已由本批修复。

2026-09-27 相邻兄弟列表范围验证：`xuiDocumentTxnSetListStyleRange` 与 `xuiDocumentTxnUnlistRange` 在同一父容器下处理连续相邻列表中的选中项；Rich 三组列表和 Markdown 混合项目符号/有序列表验证存续 NodeId、未选项、后段编号、反向文字及列表项 gap 选区、一步 Undo。中间夹杂普通段落或跨父容器时明确拒绝。独立 Core 日志 `artifacts/xui-document-rebuild/validation-2026-09-27-cross-list-restyle-core.log` 含 279/250 点分配失败扫描，验证源码/树/历史原子性与无泄漏；实际 DLL Editor 日志 `validation-2026-09-27-cross-list-restyle-editor.log` 覆盖混合状态、切换任务列表、再次点击取消及 Undo。完整套件 `validation-2026-09-27-cross-list-restyle-final-suite.log` 覆盖 652 项 CommonMark、发布导出、Renderer、Editor、MessageList 及原生同步/异步绘制，退出码零；GCC `-fanalyzer` 与 `git diff --check` 通过。跨父容器范围仍未实现；Markdown 无语义空项的限制已由本批修复。

2026-09-27 列表创建验证：公开事务命令 `xuiDocumentTxnCreateListRange` 与 VISUAL Editor 的项目符号、有序、任务列表命令共用根事务。Core 用例覆盖 Rich/Markdown、反向/完整结构选区、NodeId 保留、空项、方言拒绝、Undo 和 131 个内存分配失败点的原子性；日志为 `artifacts/xui-document-rebuild/validation-2026-09-27-list-create-core.log`。实际 DLL Editor 用例覆盖 Toolbar 按钮、命令状态、只读、SOURCE、GFM 与 CommonMark，见 `validation-2026-09-27-list-create-editor.log`。最终完整套件 `validation-2026-09-27-list-create-final-suite.log` 覆盖独立 Core、652 项 CommonMark、发布导出表、Renderer、Editor、MessageList 和原生同步/异步绘制，退出码零；修改源码 GCC `-fanalyzer` 与 `git diff --check` 通过。该批不含现有列表类型切换/取消列表或跨父容器混合选区。

2026-09-27 既有列表类型切换与取消验证：`xuiDocumentTxnSetListStyleRange` 和 `xuiDocumentTxnUnlistRange` 在统一事务中处理单个列表内的连续选中项，保留未选项样式、任务勾选状态、后段有序编号及存续 NodeId；反向列表结构 gap 与文字选区均验证。独立 Core 日志 `artifacts/xui-document-rebuild/validation-2026-09-27-list-restyle-core.log` 包括 Rich/Markdown、Undo 和 187/164 个分配失败点的原子性与回收；实际 DLL Editor 日志 `validation-2026-09-27-list-restyle-editor.log` 验证三类列表按钮在已有项上的激活、切换、再次点击取消和 Toolbar 状态。最终完整套件 `validation-2026-09-27-list-restyle-final-suite.log` 覆盖 652 项 CommonMark、发布导出表、Renderer、Editor、MessageList 与原生同步/异步绘制，退出码零；GCC `-fanalyzer` 与差异检查通过。跨不同列表或父容器的混合选区当时仍可能被拒绝；Markdown 无语义空项的取消已由本批补齐。

## 已落地内容

| 模块 | 已实现及验证的范围 |
| --- | --- |
| Document | RICH/MARKDOWN 共用节点树、64 位结构位置、持久化存储、不可变快照、根事务、共同历史、完整子树新 ID 复制、订阅与保存点 |
| 原子性 | 失败事务不发布内容、revision、历史或通知；过期位置拒绝；回调重入受控 |
| 历史内存 | 共享存储去重计费，Undo/Redo 步数/字节双预算、运行时调整/清空、空历史时当前根待命登记、快照额外保留诊断 |
| Prepare 内核 | 私有源码候选、worker 独立块增量/完整解析与计费预计算、状态/投影、协作取消、基线/代次校验及根事务统一发布 |
| 表格内核 | Cell 属于根树，行列增删、矩形合并/拆分、TSV 矩阵复制/粘贴/清空、跨度校验与根撤销 |
| Markdown | 固定版本 MD4C、CommonMark/GFM 及声明扩展的节点解析、原始源码保留、独立正文/块语法边界、引用定义记录、SOURCE 与结构 VISUAL 回写 |
| 三模式 | Source、Visual、块级 Live Markdown；模式切换共用 Document 和历史，不导出再导入 |
| Renderer/View | 可见区优先布局、块高度索引、嵌套内容、单元格递归布局、绘制/命中/光标、选择与多宽度视图 |
| Editor | 文本与段落输入、字素删除、导航、基础 marks、命令状态、只读、VISUAL 原生/纯文本/HTML 选区复制与原生→HTML→纯文本粘贴、IME 候选投影和选区撤销 |
| 异步输入 | SOURCE/LIVE 按源码或输入达到默认 100 KiB 门槛进入 Prepare，具有候选显示、非阻塞 Flush、失败重试和流式追加；VISUAL 具有普通段落/标题内单文本叶节点的精确映射输入候选，同组内插入、删除和选区替换可延续；无格式段落跨纯文本/软换行的选区可异步替换并在原编辑光标续输，后台解析并核对语义后发布，其他 VISUAL 编辑仍同步 |
| 查找 | 区分大小写的 UTF-8 字面查找、跨样式片段查找、原子全部替换；View/Editor 保存全部命中并区分当前结果着色，文档与模式变化后重算；多处源码替换只解析一次 |
| IO | 原生 schemaVersion 4 JSON（仍可读 1/2/3；旧版本分别拒绝显式零和 currentColor 标记）、Markdown 保存、纯文本/转义 HTML 导出、UTF-8 路径与原子写入 |
| 宿主接口 | XUI 通用编辑协议、基础文本辅助技术、自动高度 View 的父级滚轮传递 |

API、线程、所有权、接入方法与详细行为见 [XUI_DOCUMENT.md](XUI_DOCUMENT.md)。

## 本轮 Document 内核正确性修复

- 修复已复现的 Markdown MoveNode 改变 NodeId：语义操作记录和命令树身份成为依据，解析结果负责语义核对与源码范围更新。根节点重排、跨容器移动、相同内容互换、后续原节点编辑、Undo/Redo 均保留存续身份。
- 保留语义文本分段，不让解析器的实体/转义回调分段替换应用持有的文本节点；后续 SOURCE 编辑对未变化子树也保留这些分段。
- Markdown ChangeSet 同时保留原生语义操作和实际源码补丁。TEXT/GAP 跟随语义操作，SOURCE 跟随字节补丁，移动不再被当作语义删除。
- 为段落 SPLIT/MERGE 增加容器间隙映射，并区分只负责祖先关系的子节点 MOVE；验证正文位置、间隙位置、边界亲和性、正向/撤销映射及原地址映射。
- `SourceToPosition` 拒绝 UTF-8 编码中间的源码偏移。后续来源层已补充分段映射与 EXACT / COLLAPSED / SYNTAX / APPROXIMATE 状态，见下一节。
- 显式历史分组增加 domain 边界；同 origin/group/domain 的连续语义提交可以合并，SOURCE 与 SEMANTIC 切换仍进入同一历史栈中的独立步骤。
- 新增 800 条随机移动命令（含合法空操作），用节点顺序模型和持久文本位置校验；新增跨模式身份、连续移动/改字/删除复合事务、逐点分配失败和 Undo/Redo 分配失败检查。失败时内容、身份、revision、保存点、历史和通知均不发布，保留快照不变，最终分配平衡。

公开 `xui_doc_operation_t` 与 `xui_doc_change_info_t` 已扩展，使用方需要重新编译。源码编辑改变了结构的区域仍使用来源匹配与必要的近似映射，完整行内 CST 及所有源码位置的精确映射仍待补齐。

## 来源层实施批次（2026-09-22—23）

- 持久化正文来源片段，区分普通字节、转义、实体、换行归一化和未知来源；多码点实体跨保留文本节点时标记部分边界。双向位置支持亲和性，EXACT 的来源位置逐字节往返。
- 解析器新增受宏保护的行内定界符回调和详细引用定义回调；持久化嵌套语法范围及已解析链接/图片与实际定义的关联。单独编译未启用补丁的 MD4C，`-Wall -Wextra -Werror` 通过。
- 专项覆盖嵌套 `***`、下划线、删除线、多反引号裁剪空格、三种引用写法、自动链接、重复定义、Unicode 标签、公式、脚注来源、BOM/CRLF、原生保存重建及 Undo/Redo。修改定义更新链接资源，保留旧快照和存续 NodeId；正文编辑保留引用原拼写。
- 清除完整 marks 时仅删除对应定界符；多个范围批量改源码后只解析一次。保留实体、引用、CRLF 和容器内部定义。局部修改及代码暴露新语法时，先拒绝候选再结构回写；修复跨行 marks 遗漏换行节点的问题。
- 新增 121 轮逐点分配失败扫描（包含首个成功预算），覆盖私有候选、解析、来源投影、发布前历史；失败时源码、语义、revision、保存点、历史、通知和旧快照不变，最终分配平衡。

第一批 `validation-2026-09-22-source-segments.log` 的 core / 实际 DLL / Renderer / Editor / 原生 GPU 通过，八组基础回归见 `source-regression-*.log`。第二批 `validation-2026-09-23-inline-syntax.log` 的 core、652 项语料、实际 DLL、Renderer、Editor、15,470 个 LIVE 光标边界及原生 GPU 均通过。新 DLL 的编辑协议与 accessibility 回归（62 passed / 0 failed）见 `inline-regression-*.log`；原生 LIVE 截图已查看。

来源层尚非完整 CST。已解析脚注关联与未解析标签候选已有来源记录；独立块的链接引用查找在定义保持不变、局部与整篇解析额度均可证明安全时也可局部解析。独立链接定义的目标/标题字段内替换、删除与插入可处理零个或一个受影响块；定义标签、多块使用及脚注的完整依赖失效仍缺。块内 trivia/token 和完整无损结构回写仍缺。只清除加粗区前缀、后缀或中间词时，已有一组数字字符引用的局部改写路径，能保留相邻空格与实体的语义；其他边界空白的复杂 marks 拼写待补。

## 历史内存实施批次（2026-09-23）

- 当前版本与历史分别维护可达存储的缓存计数，共享索引、文本块、来源与资源只计一次。预算包含历史记录/操作数组，默认同时约束 256 步与 64 MiB；按栈尾距离淘汰，保持连续的 Undo/Redo 链。
- 支持即时调整/清空，操作不分配、不改变 revision/保存点；写事务和通知回调中返回 BUSY。超额单步和 Undo 后超额 Redo 有明确淘汰测试；分组记录增长同样受限。
- 快照登记不扫描全文，显式诊断按保留快照并集去重。验证富文本/Markdown 清空历史的真实分配下降量与诊断严格一致；存在外部快照时，保留量转入快照额外占用，释放该快照时相应字节才回收。live Document 销毁后仍能通过快照查询并读取。
- 新增 10 轮预算/计费逐点失败扫描（含首个成功预算），失败前后诊断逐字段不变；快照获取 OOM 不登记半成品。四个线程并发查询/读取快照，同时主线程编辑、淘汰历史、登记临时快照和销毁文档，检查内存分区恒等式及稳定内容。
- 反序列化的私有构建状态与 IME 投影快照已接入同一所有权统计。独立 core、实际 DLL、652 项语料、Renderer、Editor、15,470 个 LIVE 光标边界及原生 GPU 均通过；日志 `validation-2026-09-23-memory-budget.log`。

本批完成的是 Document 历史预算；共享属性池、Renderer 缓存预算、1/10 MiB 完整响应矩阵仍待实现或验收。

## Prepare 内核实施批次（2026-09-23）

- 每个 live Document 只持有一个当前候选；创建完成后才淘汰上一代，失败创建保留原候选。detached builder 没有 live Document 指针或独立历史，原子 NodeId 序列支持候选与所有者线程并发构建。
- Run 在调用线程执行，可由宿主执行器派发；候选源码和状态可以并发查询，已提交快照保持不变。取消、过期或失败不改变当前源码、节点树、历史、保存点或通知。
- Publish 在所有者线程核对文档身份、base revision 和代次，复用根事务原子提交、位置映射、origin/group、Undo/Redo 与通知；已有 writer/回调时 BUSY 可重试。成功普通写入和 Undo/Redo 淘汰旧候选，失败写入不淘汰。
- 测试包含工作线程解析、所有者线程通知、失败创建/错误基线文档、取消排队/就绪/运行中候选、新输入取代旧输入、外部提交、Doc 销毁、通知中释放 Doc、空候选、历史分组、资源上限和候选释放后的精确内存平衡。
- 117 轮逐点分配失败覆盖创建/解析/来源映射/提交；223 个分配位置触发取消，覆盖解析和来源重建的清理。数字均包含首个成功预算或未触发取消的终止轮次，并非同数量的失败事件。
- 652 项 CommonMark 加载与另外 652 项双补丁编辑，逐节点比较 prepare 与完整解析的种类、属性、资源、文字、块/正文范围、来源片段、行内语法和引用定义。该测试是候选路径的差分检查，**不是增量解析或 HTML 规范一致性验收**。

本批独立 core、实际 DLL、语料、Renderer、Editor、15,470 个 LIVE 光标边界及原生 GPU 全部通过；记录为 `validation-2026-09-23-prepare.log`。未启用 XUI 宏的 MD4C 另行通过 `-Wall -Wextra -Werror` 编译。Editor 仍走同步输入，待输入投影、合并/flush、流式 UTF-8 及安全增量解析继续保留为缺项。

该日志的 1/10 MiB 固定样本使用 512 字节重复段落，包含标题、加粗、实体和链接。宿主复制补丁并创建候选，worker 完整解析，所有者发布；解析期间读取 64 次旧快照，发布后逐字节核对完整源码。

| 源码 | 所有者创建候选 | Worker 准备 | 所有者发布 | Document 分配峰值 | 节点数 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 1 MiB | 0.915 ms | 61.150 ms | 10.904 ms | 26,524,372 B | 20,481 |
| 10 MiB | 9.819 ms | 700.893 ms | 119.846 ms | 265,170,172 B | 204,801 |

以上为第一批每个规模一次样本，不是 P95 或 UI 帧时间。峰值只覆盖 Document 分配，不含 OS/线程栈、输入测试缓冲、字体或 GPU。当时 10 MiB 发布需要遍历新树更新当前/历史可达存储计费，约 120 ms 的写线程开销促成了下面的第二批优化；不能将后台解析可运行解释为完整输入响应已达标。

同次 64 样本回归中，10,000 段富文本 P95 为 0.002 ms；102,480 字节 Markdown 同步编辑 P50 / P95 / 最大值为 28.449 / 49.995 / 52.207 ms，保留历史仍为 64,866,837 字节和 17 步。前一批性能数据继续保留在下方作为原日志记录，不能把不同运行时负载的差值归因为改进或退化。

## Prepare 发布成本实施批次（2026-09-23）

- 捕获历史记录的只读内容与额度，worker 预建 ChangeSet/新历史记录，独立计算提交后的当前/历史可达存储、共享去重和预算裁剪。不修改 live 计数，不在后台长时间持有诊断锁。
- 发布前验证历史根、数量、额度和 revision；一致时只应用按地址排序的存储计费变更表，不分配、不遍历源码或节点树。历史设置或栈已变化时回退普通提交；回退分配失败也不发布内容、计数或通知。
- 新增历史记录引用计数，保证 worker 能在所有者淘汰历史或销毁文档后安全结束。最终候选释放可能回收大图，性能测试把剩余引用转交给 worker 并单独记录回收时间；生产 Editor 的调度仍待接入。
- 240 次 prepare 提交与同步提交逐步比较内容/NodeId、物理存活字节、当前/历史/快照并集与 OtherBytes、保存点、共同历史、分组、超额单步、Redo 分支以及发布前额度/清空变化；开启和关闭历史都通过。
- 已验证分配器预算为零时仍可使用预计算计划发布；计划失效后的 OOM 保持原子性。运行中改变历史的两个确定性线程场景，以及四个快照诊断线程与 120 次 prepare 发布/淘汰/Doc 释放并发通过。
- prepare 逐点故障/取消扫描扩展到 121 / 230 轮（含首个成功预算或未触发取消的结束轮次）。独立 core、实际 DLL、652 项语料、652 + 652 项候选差分、Renderer、Editor、15,470 个 LIVE 光标边界及原生 GPU 均通过：`validation-2026-09-23-prepared-publication.log`。

以下与第一批使用相同 512 字节重复段落，新增首次加载后的单字节局部编辑。发布前后均断言 Document 分配次数不增加且 `iPreparedPublishes` 增加；发布后逐字节检查源码。

| 规模 / 操作 | 所有者创建 | Worker 准备 | 所有者发布 | Worker 回收 | 峰值字节 | 发布计费项数 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 MiB 加载 | 0.891 ms | 93.618 ms | 0.880 ms | 0.216 ms | 33,644,444 | 109,932 |
| 1 MiB 局部编辑 | 0.006 ms | 139.427 ms | 1.284 ms | 0.222 ms | 45,894,950 | 139,972 |
| 10 MiB 加载 | 9.695 ms | 1,121.428 ms | 10.667 ms | 1.750 ms | 304,926,804 | 1,099,117 |
| 10 MiB 局部编辑 | 0.007 ms | 1,944.051 ms | 16.075 ms | 69.390 ms | 537,964,343 | 1,686,229 |

每行仅一次内核样本，排除视图通知后的排版、GPU、输入、调度排队及线程创建/等待；峰值为该文档运行至该行的 Document 分配峰值。此优化把遍历/预算准备移到后台，以额外时间和内存换取较短的发布路径；发布仍需更新线性数量的存储项。最终 Release 若在 UI 线程执行，会把上表回收成本带回 UI。没有把这些数据当作完整 Editor、取消响应或跨平台 P95 验收。

本批同步基线：富文本局部提交仍为 15 次分配，保留量为 1,584 字节（历史记录增加引用计数字段）；10,000 段 P95 为 0.004 ms。102,480 字节 Markdown P50 / P95 / 最大值为 24.945 / 32.426 / 36.675 ms；当前 5,740,462 字节，历史 64,866,973 字节、17 步。原批次数字作为历史记录保留，不覆盖成新测量。

## 连续输入与流式缓冲实施批次（2026-09-23）

- 新增 ContinueSource，补丁相对上一代待输入源码，保留共同基线、origin/group 和此前操作；持久化源码与操作日志共享，累计操作数组仅在 Run 展开。无分配 ReadSource 支持范围读取。
- QUEUED / RUNNING / READY 候选均可延续，未取消的 Run 失败可以修改或重试。新候选创建失败时旧代次不失效；已经取消/发布/过期的候选被明确拒绝。成功链通过一次事务通知和共同 Undo 发布，来源位置映射与未修改 NodeId 保留。
- 确定性线程验证：旧 worker 停在分配点时继续输入，新代次先完成发布及 Undo，随后销毁 Document，旧 worker 再取消退出；分配全部平衡。新的延续故障扫描为 122 轮，包含最后成功预算。
- 10 MiB 待输入源码连续追加 2,048 次，没有解析；额外保留 890,880 字节的源码/日志路径。测试约束逐次和总量保留增长，并检查跨片段读取；它不包含最终解析/发布，不是 UI 延迟样本。
- StreamSource 把网络分片中的最多三个 UTF-8 尾字节保留在候选中，未补齐时不允许 Run/Publish/普通 ContinueSource/保存。非法分片、过长编码、代理区、越过 Unicode 上限、已经无效的部分前缀、最终截断及容量超限保持原子失败。
- 覆盖一份含中文、emoji、CRLF、引用和 UTF-8 编码边界的样本全部 76 种两片分割；另测单字节分片、宿主按批派发 worker、跨发布的共同分组撤销和流式故障扫描 75 轮。原 prepare 故障/取消扫描随持久化日志调整为 124 / 231 轮，均含最后成功轮次。
- 新增 652 项连续候选语料差分，与批量补丁和完整解析比较树、样式、正文/来源/行内及引用元数据。原有 652 项加载与 652 项批量编辑差分保留。独立 core、实际 DLL、语料、Renderer、Editor、15,470 个 LIVE 光标边界及原生 GPU 均通过：`validation-2026-09-23-continued-stream.log`。

本批没有把调度接入 Editor，也没有加入独立的流式可写文档或历史。宿主负责按帧派发和最终后台回收；待输入显示、flush、模式切换与失焦策略、安全增量解析及完整窗口响应仍待后续验收。

`xui_doc_prepare_info_t` 增加缓冲字节数字段，使用方需与 DLL 一起重新编译。本批 10 MiB 局部编辑内核样本为创建 0.006 ms、后台准备 1,888.049 ms、发布 15.316 ms、worker 回收 66.647 ms，峰值 537,963,648 字节；与上批相同限制，不代表 UI 指标或本批性能改进证明。

## SOURCE Editor 异步实施批次（2026-09-23）

- Editor 以持续 worker 运行最新候选；所有者线程发布与通知。已接受输入通过 prepare 只读源码投影显示，仍共用一个 Document/历史；未提交内容不出现在其他普通预览中。位置增加 `iInputGeneration`，旧代次和提交快照拒绝混用。
- 新增非阻塞 Flush、RetryInput、CancelInput、GetPendingInput；模式/文档切换与查找先 Flush，BUSY 由宿主更新后重试。失焦继续保留/发布，销毁取消并等待 worker 退出；已接受输入需要保存时应先 Flush 再保存/销毁。
- SOURCE IME 以只读投影预览，不改变当前输入代次、不解析或发布。取消只清预编辑，确认并入共同候选；组合期间推迟发布。解析或发布 OOM 保留可见输入、阻止保存，允许下一代重试。
- 受控分配器暂停 worker 时继续输入/选择/复制/字素删除/绘制；验证另一 Editor 不抢写、预览 revision 不变、模式和保存 BUSY、IME 确认/取消、选区与共同一次 Undo、运行中外部写入。后台和所有者发布回退 OOM、64 轮队列/取消/发布压力、排队 Undo 和销毁回收全部通过，最终 Document 分配数归零。
- 原生新增 `--verify-async`：源码栏先出现待输入，预览栏像素保持原值；发布后源码栏像素不变、预览出现新段落；字节核对及一次 Undo 恢复原文通过。`native-async-pending.png` 和 `native-async-committed.png` 已查看，不能替代真实 OS IME。
- 完整 core、实际 DLL、652 项语料与各 652 项三组候选差分、Renderer、Editor、15,470 个 LIVE 光标边界、原生 Source/Live/异步 GPU 全部通过：`validation-2026-09-23-async-source-editor-final.log`。编辑协议及 accessibility（62/0）分别见 `validation-2026-09-23-async-edit-contract.log`、`validation-2026-09-23-async-accessibility.log`。

本批发现并修复了一次实际堆错误：输入投影 CopyRange 使用内部带头分配器，但公开 FreeBuffer 调用 plain free，导致 Editor 退出 `0xC0000374`。原批处理只检测正退出码，首次 `validation-2026-09-23-async-source-editor.log` 因而误报成功；该日志不能作为通过证据。现改用非零退出码检查，模拟负退出码被正确拒绝。修正分配器后 `validation-2026-09-23-async-source-editor-fixed.log` 又正确报告代理断言失败：文字绘制发生在控件缓存面而非最终合成目标。改为检查合成调用，并以实际 GPU 像素验证显示内容；最终日志包含 Editor 完成输出且退出码为零。

结构 ABI 已变化，使用方须重新编译。当前 SOURCE 每次候选仍重建完整行目录，部分导航/通用编辑协议展开完整源码；尚未交付完整延后动作队列、自动输入分组、LIVE/VISUAL 异步和流式 Editor。没有 1/10 MiB 完整 Editor P95 证据，不把内核时间当作输入帧时间。

## SOURCE 局部投影实施批次（2026-09-23）

- prepare 记录不可变的直接前驱代次及本次单补丁范围；Renderer 只在代次/基线对应时使用该范围，零补丁复用全部行，多补丁或跳过前驱回退完整扫描。该元数据不读取运行中 builder，也不触发解析。
- 扫描修改行及前后完整行以涵盖 CRLF 拆合；终止空行只归属于一个分区。所有新目录/高度分配完成后才更改旧缓存，未变行保留 run/fragment/font，后缀只平移源码偏移；高度索引改为线性构建。发布与投影 SourceStore 相同时不再重扫或重新排版。
- 新增累计扫描字节、局部更新次数、复用行次数的 Renderer stats。1,302 项差分用独立 Document 完整加载作为参照，比较最终尺寸、全部有效 UTF-8 双亲和性光标、命中位置及文字/矩形/颜色/flags 绘制调用；覆盖全部 CR/LF 范围替换、Unicode、600 步连续/多补丁/跳过代次/无补丁链、换宽与发布后 Undo。
- 1 MiB 和 10 MiB 各 128 次前部换行插入/删除，均扫描 131,200 字节、shaping 130,880 字节；分别累计复用 262,016 / 2,621,312 个行块，没有解析器运行。远处的已排版行也参与复用和偏移校验。该样本是 Renderer 投影验证，不能当作完整 Editor 延迟。
- Editor 的 1/10 MiB 样本使用与内核样本相同的 512 字节重复段落。先接受一个前部插入并把 worker 暂停在分配点；随后预热 4 次、测量 64 次同位置单字节交替替换，计时覆盖 InsertText、Update（含布局/文本事件）、代理渲染。选区设置在计时外，工作线程在测量期间不完成解析；计时后释放 worker，逐字节验证提交文本、一次共同 Undo 和所有 Document 分配回收。

专项日志 `validation-2026-09-23-source-projection-renderer-final.log` 和 `validation-2026-09-23-source-projection-editor-final.log` 均通过。Editor 该次 P50 / P95 / 最大值分别为：1 MiB 的 0.443 / 0.508 / 0.528 ms；10 MiB 的 3.726 / 4.034 / 8.974 ms。它不包含首次打开、选区设置、worker 完成及发布、真实 GPU 或第二预览；不能作为完整平台 P95 验收。行块数组/高度索引的复制构建仍随总行数增长，部分导航/编辑协议仍展开整份源码。

本批测试调整保留原失败证据：`source-projection-renderer.log` 的第一轮因测试尝试在已加载字体的 Context 更换 Proxy 而正确失败，已改为独立 Context 提前安装捕获器。Editor 初测及 `source-projection-editor-budget.log` 发现 10 MiB 单次历史超过默认 64 MiB，提交成功但没有保留 Undo；该行为符合预算契约。最终撤销专项显式配置 **256 MiB** 额度，1/10 MiB 实际保留 10,726,897 / 107,212,457 字节和一个 Undo 步骤，不把默认预算下的淘汰描述为撤销通过。Renderer stats 结构增加字段，调用方需重新编译。

最终完整回归为 `validation-2026-09-23-source-projection-final.log`：core、实际 DLL、652 项语料与三组各 652 项候选差分、1,302 项来源投影差分、15,470 个 LIVE 光标边界、Renderer/Editor、原生 Source/Live/异步 GPU 均通过；两张原生异步截图再次查看。编辑协议及 accessibility（62/0）通过，分别记录于 `validation-2026-09-23-source-projection-edit-contract.log`、`validation-2026-09-23-source-projection-accessibility.log`。最终套件的 SOURCE Editor P50 / P95 / 最大值为 1 MiB 的 0.432 / 0.465 / 0.520 ms，10 MiB 的 3.525 / 3.934 / 10.030 ms；测试边界和 256 MiB 历史预算与上述专项一致，不把运行间差值当作性能变化证明。

## 异步事件顺序实施批次（2026-09-23）

- 原实现只保留一条延后 Undo/Redo，后来的命令会覆盖它，后续输入也可能混入将要撤销的候选。现使用值拷贝的控件事件 FIFO，按顺序处理文字、命令、导航、IME、指针与剪贴板。直接编辑/选区 API 在同 Editor 有排队事件时返回 BUSY，未接受该次调用；其他视图读取/选择已提交内容不受阻塞。
- Document 增加队列所有者标记，SaveFile 同时检查候选和已接受事件。专项在 Undo 通知中启用 OOM，确认后续文字没有创建 prepare 时仍保留在队列、保存及其他 Editor 写入仍 BUSY；Retry 后形成新的一次根提交与 Undo，失败不会让字节静默丢失。
- FIFO 限制 4,096 个事件，入队 OOM/超限只拒绝新事件，旧队列不变；Flush 按最多 64 个事件及事件之间的 4 ms 预算推进。时间预算不抢占单个原子命令或宿主回调。相邻拖动 move 合并，悬停不排队，未知快捷键继续冒泡；捕获/焦点在收到事件时改变，回放只处理文档选择，不夺回已转走的焦点。
- 测试重复 Undo/Undo/Redo 后继续文字与光标移动，核对最终字节与撤销选区；验证 160 个输入跨批处理、队列满载、分配失败、永久不支持命令只跳过自身、IME 预编辑/取消/确认、复制、外部权威写入、显式取消、回调中分别在发布与 Undo 销毁控件，最终分配平衡。新增 GetPendingWork 区分 prepare 与等待事件，GetPendingInput/HasPrepare 保持只查询 prepare 的语义。
- 指针测试发现实际视口错误：撤销长行后，短内容仍保留横向 scroll=28，左侧点击落到文末。Editor reveal 现在按最新内容尺寸夹紧视口；同一测试已确认滚动归零、点击在前部输入且焦点停留在另一 Editor。

初轮 `validation-2026-09-23-ordered-editor-cases.log` 因测试的另一个 Editor 未挂在 root 下而无法取得焦点；改为共同宿主后 `validation-2026-09-23-ordered-editor-cases-final.log` 复现上述滚动错误。修复后的 `validation-2026-09-23-ordered-editor-scroll.log` 专项通过。`validation-2026-09-23-ordered-editor-final.log` 是加入完整 FIFO/容量/快捷键/焦点用例后的整套回归；随后补充事件间 4 ms 预算及 8 ms 慢剪贴板回调，`validation-2026-09-23-ordered-editor-verified.log` 的 core、实际 DLL、652 项语料、Renderer/Editor/LIVE 与原生 GPU 均通过。慢回调后的文字保留到下一次 Flush，无 prepare 时保存仍 BUSY。

回调正确性追加修复：

- `validation-2026-09-23-editor-reentry-before.log` 复现宿主剪贴板回调继承 replay 权限，公开 InsertText 可以插队。现分离私有事件执行与公开 API，公开输入、命令、选区、缩放、滚动、模式/查找/只读、Flush/Retry/CancelComposition 均不能在回放或剪贴板回调中插队；命令查询给出 BUSY 禁用状态。通用编辑协议也走同一入口。
- `validation-2026-09-23-editor-callback-cancel-before.log` 复现回调已 CancelInput，Cut 仍继续删除，将 `base` 变为空文档。现检查取消代次及控件生命周期，在回调取消、外部写入或销毁后中止旧动作；队列清理后重新入队也不会因地址复用被旧动作移除。
- 专项覆盖 Cut 和 Paste 两次回调位置各四种动作（取消、外部替换、销毁、取消并重新送字），即时 Copy 重入、复制不取消 IME、无 prepare 的预编辑取消、末次发布观察者新送字以及模式/Find/换 Document 的 Flush 销毁边界。新事件未处理时 Flush 不误报完成；所有受控 Document 分配归零。
- `validation-2026-09-23-editor-callbacks.log` 专项通过；编辑协议及 accessibility（62/0）分别通过 `validation-2026-09-23-callback-edit-contract.log` 与 `validation-2026-09-23-callback-accessibility.log`。最终完整回归 `validation-2026-09-23-editor-callbacks-final.log` 的 core、实际 DLL、652 项语料与三组各 652 项候选差分、1,302 项投影差分、15,470 个 LIVE 光标边界、Renderer/Editor 和原生 GPU 均通过，进程退出码为零；两张原生异步截图已重新查看。

最终套件中暂停 worker 的 SOURCE Editor P50 / P95 / 最大值为：1 MiB 的 0.451 / 0.516 / 0.755 ms，10 MiB 的 3.859 / 4.441 / 4.708 ms。仍只包含输入/Update/代理绘制，显式采用 256 MiB 历史预算，不包含后台完成及真实 GPU；这些样本不代替完整性能验收。

本批不宣称已完成跨发布自动输入分组、完整结构/对象命令或 LIVE/VISUAL 异步化；真实 OS IME、拖动/焦点平台矩阵及完整 GPU/后台延迟仍属原设计验收缺项。

## 跨发布输入分组实施批次（2026-09-23）

- 交互 text/key 事件的连续键入和同方向删除，使用同一 Document 的 origin/group 跨发布合并；默认 1,000 ms，可由 Editor 描述配置或禁用。FIFO 保存到达时间；批量回放仍保留原输入停顿。
- 富文本、同步 SOURCE、LIVE、VISUAL 和异步 SOURCE 均测试三次发布后的一次 Undo/Redo、首尾选区、选择替换、历史分支、Unicode 字素/删除方向、导航、保存点、粘贴/剪切/换行/IME、焦点/模式/格式边界及单步历史额度。程序 API 仍不跨发布自动分组。
- 暂停 SOURCE worker，输入后移动光标并再次输入：后一组排队，前一组发布保留当前光标；分别 Undo/Redo 还原各组的首尾位置。另以 20 ms 配置确认回放使用到达间隔，并测试 UINT32_MAX 禁用合并。
- IME 带不同于当前选区的替换范围，旧组先发布后注入所有者 OOM：根文档保留旧组，IME 与确认事件保留；Retry 后按原范围替换，两个 Undo 分别撤销确认和旧组，Document 分配平衡。
- `validation-2026-09-23-input-groups-before.log` 在改动前复现每次发布各成一步。实现后的 `validation-2026-09-23-input-group-ime-oom-diagnostic.log` 复现同步错误被忽略，等待确认误报过期后从队列丢失；现传播同步错误并在发布时转换保留的 IME 范围。`validation-2026-09-23-input-group-boundaries-verified.log` 专项通过；最终 `validation-2026-09-23-input-groups-final.log` 包含追加剪切/Enter 用例，完整 core、实际 DLL、652 项语料和三组各 652 项候选差分、1,302 项投影差分、15,470 个 LIVE 光标边界、Renderer/Editor 与原生 GPU 均通过，进程退出码为零。
- 编辑协议与 accessibility（62/0）分别通过 `validation-2026-09-23-group-edit-contract.log`、`validation-2026-09-23-group-accessibility.log`。最终套件的未发布程序批次 P50 / P95 / 最大值：1 MiB 为 0.458 / 0.522 / 0.641 ms，10 MiB 为 4.030 / 4.750 / 9.645 ms；仍使用显式 256 MiB 历史额度并冻结 worker，不包含分组边界的解析等待。原生异步两图已查看。

测试修正保留在日志：模式专项初版错误要求 Markdown 不转义斜杠，后改为普通字符以隔离分组行为；重置 fixture 改用 LoadMarkdown，避免全选替换产生的合法段落分隔差异。焦点用例必须调用 DispatchPendingEvents 才会送达 Blur，Update 本身不派发这些事件。OOM 用例在恢复分配器后才读取快照，避免把预期的分配失败当作内容错误。

Editor 描述增加字段，需要与 DLL 一起重新编译。新组遇到未完成候选时可能等待完整 Markdown 解析；这一等待及真实 IME/完整 GPU 输入延迟尚未验收。1/10 MiB 的旧 SOURCE Editor 样本仍是显式程序批次的输入/Update/代理绘制，不能作为跨组响应结果。

## 独立块增量解析实施批次（2026-09-23）

- 同步 SOURCE 与 prepare/continue 共用局部路径：一个顶层段落或标题，由空行或文档边界隔开，无全局定义，且新解析仍是一个完整段落/标题。保持根事务、NodeId 规则、源码与树原子发布；其余语法回退完整解析。新增 `iMarkdownIncrementalParses`，统计结构须与 DLL 同步编译。
- `xui_document_incremental_cases.h` 在独立 core 与实际 DLL 中使用完整新加载为 oracle，比较树、属性、资源、文字、正文与块范围、全部来源片段、行内语法及定义。6,375 个三方言编辑案例中，5,373 个使用局部候选；同步/prepare 交错覆盖 Undo/Redo、旧快照和身份恢复。另测累计补丁、块外 NodeId、未解析引用变成定义、未使用脚注、CR/LF、front matter，以及减少的解析字节。
- 157 轮分配预算和 144 轮取消位置扫描，包含最后成功轮次，验证发布内容、保存点、历史、revision、通知不出现半成品；所有自定义分配均归零。
- `validation-2026-09-23-incremental-crlf-before.log` 复现真实错误：窗口末尾 CR 与下一字节 LF 合成 CRLF 后，原空行消失，局部树却仍保留两个段落。修复后该边界回退完整解析。早期 chain 用例的手写字节偏移错误已改为按标记定位；保留 `incremental-differential.log` 的失败记录。
- `validation-2026-09-23-incremental-corpus.log` 已通过独立 core、652 项 CommonMark、三组各 652 项候选差分，以及 1/10 MiB 的首/中/尾等长替换、插入、删除样本。每个大文档样本核对完整源码、一次 Undo 和最多 512 字节解析量；显式使用 256 MiB 历史预算。
- 最终 `validation-2026-09-23-incremental-final.log` 的独立 core、实际 DLL、语料、Renderer、Editor、1,302 项来源投影差分、15,470 个 LIVE 光标边界及原生 GPU 全部通过，进程退出码为零；异步发布前/后两张截图已查看。另有 `incremental-edit-contract.log` 与 `incremental-accessibility.log`，编辑协议及 accessibility（62/0）通过。MD4C 无 XUI hook 的独立严格语法编译、新增文件严格编译和 diff 检查通过。

最终套件中，10 MiB 标题等长修改的 worker 准备为 432.033 ms，发布 0.003 ms，worker 回收 0.005 ms。另一个在每个样本前清空历史的段落测试：首段等长替换只解析 499 字节，准备 538.199 ms、发布 9.548 ms、历史 8,479 字节；首段插入解析 500 字节，准备 1,009.396 ms、发布 16.387 ms、历史 102,294,512 字节。两组的历史初始状态不同，不能直接比较发布耗时。102,480 字节同步来源编辑的 64 样本 P50/P95/最大值为 0.071 / 0.103 / 0.199 ms；均不包括实际控件/GPU 输入延迟。该日志首次单次摘要残留的 `full parse path` 旧标签已在测试源码移除，不作为路径证据；局部路径由专项计数器断言验证。

这只是受限的块增量解析。字节长度变化仍复制后续节点的绝对来源范围，后缀行内元数据、块目录/根范围及历史计费计划仍有线性工作。10 MiB 首段插入/删除仍需约 1 秒后台准备并保留约 102 MB 历史；不能据“只解析 500 字节”推导交互已达到帧预算。持久来源索引、全局依赖、更多块类型、计费增量化和完整窗口 P95 继续保留为未完成工作。

## Prepare 共享存储差额计费批次（2026-09-23）

- 已计费历史包含当前提交基线时，PrepareRun 从受锁保护的当前/历史可达计数出发，模拟新旧当前版本和历史预算淘汰；共享的持久子树只处理所有者计数边界，不重新遍历内部节点。历史刚清空、没有可复用基线或后台遇到所有者计数变化时，走原有完整计费，Publish 仍检查捕获的版本与历史配置。计费变更表保持零分配发布。新增 `iPreparedAccountingVisits`，公开统计结构再次变化，应用须与 DLL 一起编译。
- `validation-2026-09-23-accounting-delta-final.log` 的完整独立 core、实际 DLL、652 项语料、三组各 652 项候选差分、Renderer、Editor、1,302 项 SOURCE 局部投影、15,470 个 LIVE 光标边界和原生 GPU 全部通过，退出码为零。240 次准备发布与同步提交的物理内存/历史/快照逐项对照、历史分组/淘汰/限额变更、准备期清空历史、124 轮分配失败、231 个取消检查点和四线程快照诊断保持通过。
- 同一固定样本的 10 MiB 标题局部修改，旧 `incremental-final.log` 的后台准备 432.033 ms；本批 7.375 ms，计费访问 279 次、实际更新 116 项，所有者发布 0.001 ms。1 MiB 样本为 0.688 ms、219 次访问。计数断言要求这两种局部修改各小于 8,192 次访问。它们是独立内核样本，不包含 GPU、输入分组边界或完整窗口延迟。
- 每次清空历史的 10 MiB 段落样本仍须第一次为历史标记大量原有存储；首段等长修改的后台准备 532.676 ms，插入 997.901 ms。插入还有后缀绝对偏移更新和约 102 MB 历史，因此本批不能宣称所有编辑都达到输入帧预算。该性能边界与持久来源索引、更多块局部解析和完整 UI P95 一起留在 K4 工作包。

## SOURCE 按需导航与 Renderer 尺寸缓存批次（2026-09-23）

- SOURCE 选区左右/按词移动、Backspace/Delete 通过持久源码的 1 KiB 读缓存调用与原先相同的 Unicode 字素/自然词算法，不再为一次移动分配或复制整篇源码。SOURCE IME 替换范围直接检查源码长度并映射偏移；非 SOURCE 的段落投影路径保持原有语义。大于 `INT_MAX` 的通用编辑偏移仍返回 LIMIT。
- Renderer 的 GetSize 宽度及 exact 采用失效缓存；行目录重建/局部拼接、块布局成功或失败、内容无结构变更的块失效和宽度变更都会标记脏。稳定布局下显露光标无需扫描全部行；高度仍从 Fenwick 索引读取。编辑后第一次查询可以扫描一次行数组，不代表已交付持久行索引。
- `xui_document_source_editor_cases.h` 在真实 Editor 事件路径上用完整文本 Unicode 结果作为 oracle，逐项比较 SOURCE 和 LIVE 的字素/自然词移动，覆盖 CRLF、旗帜、ZWJ、跨 1 KiB 边界的组合字符、Backspace 与 IME 源码范围。暂停 worker 后发布的 1/10 MiB SOURCE 文档在中部各执行 64 对 Left+Right，确认选区往返且共用一次 Undo。
- 最终 `validation-2026-09-23-source-navigation-final.log` 独立 core、实际 DLL、652 项语料、三组各 652 项候选差分、1,302 项 SOURCE 投影、15,470 个 LIVE 光标边界、Editor 与原生 GPU 全部通过，进程退出码为零；编辑协议及 accessibility（62/0）专项、严格 C 编译和 `git diff --check` 通过。中部导航 P50/P95 为：1 MiB 0.001/0.002 ms，10 MiB 0.002/0.002 ms；改尺寸缓存前同一代理 UI 样本为 0.025/0.039 ms 与 0.212/0.228 ms。样本包含按键事件和显露光标，不包含真实 GPU/IME，不能替代全场景 UI P95。
- 此批次的通用 `xuiEdit*` 文本/选区适配仍会展开全文；下一批已移除 SOURCE/LIVE 选区操作的全量投影，GetText 与订阅完整文本的编辑事件仍按协议返回完整文本。局部 SOURCE 行拼接仍复制行块数组并重建高度索引，更多块解析、全局依赖和无历史基线计费亦未解决。

## 通用 SOURCE 选区直接映射批次（2026-09-23）

- `xuiEditSetSelection/GetSelection` 在 SOURCE/LIVE 模式直接使用持久源码偏移。Set 验证长度和 UTF-8 边界，按 Snapshot 身份/revision 或待输入 Prepare 代次构造来源位置；Get 验证当前来源选区再返回 `int` 偏移。反向选区的亲和性、折叠选区、错误不修改已有选区及原有 `INT_MAX` 输出限制保持一致。VISUAL 的富文本段落映射路径不变。
- Editor 专项覆盖 CRLF/emoji 源码、UTF-8 中间字节与越界、SOURCE/LIVE 反向选区。1/10 MiB 已提交源码各 64 次 `xuiEditSetSelection/GetSelection` 往返，以及冻结 worker 的待输入选区，均断言 Document 分配计数和存活字节不增加；`GetText` 仍明确返回完整文本。
- `validation-2026-09-23-direct-source-selection-final.log` 的独立 core、实际 DLL、652 项语料、三组各 652 项候选差分、1,302 项 SOURCE 投影、15,470 个 LIVE 光标边界、Editor 与原生 GPU 全部通过，退出码零；编辑协议和 accessibility（62/0）专项、严格 C 编译与 `git diff --check` 通过。通用 GetText 与订阅完整文本的编辑事件仍完整展开，行块数组/高度索引和来源绝对偏移仍有全局更新。

## SOURCE 局部行树批次（2026-09-23）

- SOURCE Renderer 使用可局部拆分/拼接的行树；每节点拥有原有布局缓存，子树维护数量、合计高度、最大宽度和未测量数量，后缀源码偏移延迟平移。单补丁直接后继先分配新行，再无分配地替换受影响行；失败保留旧目录。VISUAL/LIVE 的行块数组和高度索引未改变。
- `iSourceRowsCreated` 为成功目录构建/局部更新累计创建的行节点数。1 MiB 与 10 MiB 两组各 128 次首/中/尾换行插删均为扫描 174,208 字节、创建 446 个新行节点、shaping 66,452 字节；复用行分别为 261,890 / 2,621,186。专项断言新行数不超过每次 8 行，且输入期间未运行解析器。该统计验证行节点分配的局部性；Document 语义来源的后缀绝对偏移尚未解决。
- 1,302 项 CR/LF、UTF-8、连续/跳过代次和多补丁差分与独立完整加载比较尺寸、所有合法位置的双亲和性光标、命中及实际文字绘制；1/10 MiB 样本验证不同位置的拆分/拼接与远处已排版行的延迟偏移。Renderer 专项 `validation-2026-09-23-source-row-tree-renderer-mixed.log` 及最终 `validation-2026-09-23-source-row-tree-final.log` 退出码零，后者覆盖独立 core、实际 DLL、652 项语料、Renderer、Editor、15,470 个 LIVE 光标边界与原生 GPU。编辑协议、accessibility（62/0）、严格 C 编译及 `git diff --check` 再次通过。
- 暂停 worker 的 1/10 MiB SOURCE Editor 各 64 次输入/Update/代理绘制样本 P50/P95：1 MiB 0.075/0.078 ms，10 MiB 0.075/0.077 ms；同批测试另验证选区/导航及发布/Undo，不计入后台准备、真实 GPU/IME、首次打开或平台整体 P95。Renderer 行树不等于完整编辑器性能验收。

## Document 来源片段共享批次（2026-09-23）

- 独立段落/标题增量解析移动未变后缀时，节点继续共享不可变 `provenance` 序列，通过原始/当前锚点在公开读取、双向位置映射和语义重写时计算有效来源偏移。初次位移找片段最小锚点，后续位移不再复制或逐项改写片段。其他节点范围与全局行内语法仍存绝对坐标并逐项更新，故不是完整持久来源索引。
- 原有 6,375 项三方言增量/完整重载差分（5,373 项局部）和新增长期快照用例通过。新用例连续两次插入、一次删除，检查后缀来源片段、尾段每个源码字节的双亲和性映射、节点身份、旧快照、再做语义文本替换及多步 Undo/Redo；结果逐项对照完整重载。分配失败/取消扫描分别为 137/124 轮，覆盖首个成功预算及失败不发布半成品。
- 固定 10 MiB 首段插入样本仍只解析 500 字节，历史额外存储为 67,398,440 字节；上一批相同样本为 102,294,512 字节。后台准备 841.946 ms、所有者发布 12.187 ms，仍不满足完整输入帧目标。两次耗时是不同运行的单次样本，不能据此证明稳定速度提升。`validation-2026-09-23-lazy-provenance-final.log` 的独立 core、实际 DLL、652 项语料、Renderer、Editor、15,470 个 LIVE 光标边界与原生 GPU 均通过，退出码零；编辑协议、accessibility（62/0）、严格 C 编译及 `git diff --check` 通过。

## 无历史基线差额计费批次（2026-09-23）

- Prepared publish 的差额路径现在只要求 captured base 仍属于当前 Document；是否同时属于 Undo/Redo 历史不再是前提，禁用历史的 Document 也可使用。新历史条目仍按原策略精确计费、裁剪和发布，失效的 owner 计数回退根遍历，发布前继续校验 revision/额度/历史栈。
- 同一固定 10 MiB 样本先清空历史再做首次局部编辑：首部等长修改的 `iPreparedAccountingVisits` 从修改前 3,297,453 降至 1,099,503；首部插入从 3,904,941 降至 2,921,975。两次计费仍准确报告相同的历史字节和存储更新数；测试增加访问次数上限以防全量回退。禁用历史的独立 1 MiB 局部修改只访问 220 项、发布更新 104 项，并保持零 Undo/历史字节。
- 启用历史时首次提交仍要给旧根登记新历史所有权，因此 10 MiB 首部插入继续访问约 292 万项，不能宣称完整局部计费或 UI 帧目标。样本本机 worker 时间 760.526 ms、所有者发布 10.887 ms；计时没有作为正确性断言。`validation-2026-09-23-no-history-accounting-final.log` 的独立 core、实际 DLL、652 项语料、240 次同步计费/预算/分支/快照对照、Renderer、Editor、15,470 个 LIVE 光标边界及原生 GPU 全部通过，退出码零；编辑协议、accessibility（62/0）、严格 C 编译及 `git diff --check` 通过。

## Document 有序来源检索批次（2026-09-23）

- 源码位置到语义位置的查找现在利用已验证的顶层块语法范围二分定位，按可能的来源距离扩展邻块，最后按原先的深度优先顺序评估所有候选，保留双亲和性、映射质量和同分决胜结果。完整解析、身份协调和语义回写后重新核验所有可命中的正文来源段；局部解析只验证替换块与相邻边界。结构改动使索引资格失效，异常或跨块来源直接回退原有全树扫描。
- 652 项 CommonMark 样例中 30,940 个有效来源位置的索引结果与原完整扫描逐项相同；102,480 字节连续局部编辑及 1/10 MiB 后台局部编辑后的首、中、尾位置也对照完整扫描。最终完整套件的固定 10 MiB 文档局部修改后，已热缓存的索引查找 256 次平均 3.488 微秒，单次完整扫描 77.118 毫秒；仅说明该本机样本的搜索量差异，不作为跨平台性能承诺。首次历史所有权登记和跨块增量解析仍是独立未完成项。

## 独立块家族增量解析批次（2026-09-23）

- 在段落/标题之外，规则线、围栏及缩进代码、Mermaid、HTML、引用块、列表和表格也可作为单个顶层块局部解析。围栏、HTML 和容器块会额外解析左右邻块并核对语义、来源范围和片段；列表等容器若将分隔空行计入语法范围，则先验证该物理行确实空白。邻块超出 64 KiB、边界不稳、全局引用/脚注依赖仍保守回退。失败分配和取消不被语法回退掩盖。
- 三方言新增 8,700 次 LF/CRLF 逐字节编辑与完整加载逐项对照，其中 5,925 次走局部候选；另有连续围栏代码、列表和段落变长编辑的全部源码位置映射、旧版本与 Undo/Redo 对照。原 6,375 次差分及 Undo/Redo 仍通过。邻块路径的 163 个分配失败预算和 149 个取消检查点验证失败原子性、旧快照和回收。发现并修正代码块首行为空时，归一化换行沿用前一段落来源游标的问题；LF/CRLF 回归现在要求首片段始于围栏内部内容行，列表和缩进代码的 CRLF 分隔空行可保持局部解析。完整 core、实际 DLL、Renderer、Editor、原生 GPU 及编辑协议通过，无障碍测试为 62/0；严格 C 编译和 diff 检查通过。

## 列表项结构命令批次（2026-09-23）

- `SplitListItem` 在富文本与 Markdown 的同一事务中移动尾段和后续块到新兄弟项；保持其他列表项 NodeId、列表有序/紧凑属性，任务新项取消勾选。`ExitListItem` 删除空项并在父容器创建段落；中间位置分裂前后列表，第二个有序列表沿用原编号。Markdown 回写对相邻列表交替使用标记，完整重解析后验证两段语义结构不被合并。
- VISUAL Editor 的折叠光标 Enter 调用上述命令，SOURCE/LIVE 仍走源码输入。专项覆盖富文本、Markdown、有序/无序列表、嵌套空项、GFM 任务项、CRLF、连续 Enter→退出→输入及共同 Undo。分配失败预算扫描分别为 213/184 点，逐点检查未提交版本的树、来源、历史和分配平衡。完整 core、实际 DLL、652 项 CommonMark 语料、Renderer、Editor、LIVE 与原生绘制均通过；新增连续 Enter 用例后，Editor 与实际 DLL 单独复核通过。通用编辑协议及无障碍 62/0 通过。列表层级调整和完整命令矩阵仍未交付。

## 列表层级命令批次（2026-09-23）

- `IndentListItem` 把单个项移到前一兄弟项的子列表；`OutdentListItem` 将嵌套项提升到父项之后，并把原列表中它后面的兄弟项接到被提升项下。顶层提升把当前项的内容块转成普通块，必要时拆分前后列表并延续有序编号。富文本保留移动项及其子树的 NodeId；Markdown 用同一语义命令生成候选、回写受影响顶层块并重解析核对，不可表达时不提交。
- VISUAL 折叠光标支持 Tab / Shift+Tab 和命令状态查询；SOURCE/LIVE 或不适用的选区不截获 Tab。专项覆盖富文本节点身份、Markdown 的有序/无序与任务列表、CRLF 未触及尾部、首/中/末/单项顶层提升、光标来源映射与 Undo。缩进、嵌套提升和顶层提升的分配失败扫描分别通过 160/284/164 个预算点，逐点检查树、源码、历史不变及无泄漏。`validation-2026-09-23-list-levels-final.log` 的独立 core、实际 DLL、652 项语料、Renderer、Editor、LIVE、原生绘制和异步绘制完整通过；编辑协议与无障碍 62/0 通过。跨范围列表选区、其他块与对象命令仍未完成。

## 同列表范围调整批次（2026-09-23）

- `IndentListRange` / `OutdentListRange` 接受语义选区，要求两个端点属于同一个列表；包含端点项，并按原顺序整组调整。顶层多项提升会生成多个普通块，剩余有序列表从正确编号继续。富文本保持列表项 NodeId，Markdown 先在影子树完成所有移动，再一次回写并完整重解析核对；选区方向和单步 Undo 保持。不同列表的范围明确拒绝且不发布。VISUAL 的 Tab / Shift+Tab 已接入非折叠选区；SOURCE/LIVE 不截获。跨不同列表、嵌套层级的混合选区仍需补齐。
- 专项在富文本和 Markdown 中验证三项反向选区缩进与提升、节点身份、范围来源位置、按组撤销；另测有序列表中间两项提升及剩余列表起始编号。实际 DLL Editor 检验键盘事件、保留反向选区与撤销恢复。Markdown 缩进、嵌套提升、顶层提升的分配失败扫描为 217/290/226 个预算点，逐点核对源码/树/历史不变及分配平衡。`validation-2026-09-23-list-range-final.log` 的独立 core、实际 DLL、652 项语料、Renderer、Editor、LIVE、原生与异步绘制完整通过；编辑协议和无障碍 62/0 通过。

## EXTENDED 行内格式批次（2026-09-23）

- EXTENDED 解析 `==高亮==`、`~下标~`、`^上标^`，记录语义 mark 和定界符来源；插入、整段清除、结构回写与撤销共享 Document 事务。CommonMark/GFM 不可表达的 mark 明确拒绝。VISUAL Editor 增加代码、高亮、上标、下标命令及 active/mixed 查询，Renderer 绘制可配置高亮背景并移动上下标基线。
- 源码专项检查 LF/CRLF、未修改邻块、撤销后再格式化，以及无额外空行；实际 DLL Editor 检查连续命令、互斥 marks 和方言禁用。插入/清除故障注入分别扫描 98/52 个分配预算点，逐点核对已发布源码、语义树、历史不变及分配平衡。完整套件 `validation-2026-09-23-extended-inline-final.log` 的独立 core、实际 DLL、652 项语料、Renderer、Editor、原生/异步绘制通过；编辑协议与无障碍 62/0 通过。复杂 delimiter、链接与富文本字体/颜色编辑仍待完成。

## 选区链接批次（2026-09-23）

- `TxnSetLink` 接受非折叠语义选区，富文本在边界拆分 run，设置/清除链接 mark、URI 和标题；通用 `SetMarks` 拒绝 LINK 位，防止目标与 mark 不一致。Markdown 采用影子事务和完整语义核对；完整链接解除优先剥离原始定界符，精确单文字片段创建链接时优先局部插入来源，复杂选区回退结构回写。`SnapshotQueryLink` 检查选区目标一致或混合。专项覆盖部分解除、跨加粗 run 创建、引用定义/实体/CRLF 保留、Undo、Editor 创建与解除。
- Renderer 使用可配置链接颜色并给链接文字加下划线；View 命中链接文字后经 `onActivate` 回传资源目标。富文本创建、Markdown 局部创建/完整清除/跨样式创建/部分清除的分配失败扫描分别为 18/120/62/135/165 个预算点，逐点核对失败时源码、语义树、历史不变且无泄漏。`validation-2026-09-23-link-final.log` 的独立内核、实际 DLL、652 项语料、Renderer/View/Editor、原生及异步绘制完整通过；编辑协议、无障碍 62/0、严格 C 编译、实际 DLL 导出和 diff 检查通过。折叠选区插入、图片链接及完整 CST 保真仍需实现。

## 链接插入批次（2026-09-23）

- `TxnInsertLink` 在折叠光标或选区位置插入单行 UTF-8 链接文字；先由统一 `ReplaceRange` 替换正文，再在同一根事务中设置链接目标与 mark，并映射拆分后的最终光标。Markdown 空根、正文中间、整段替换和 Undo 共享原来源/语义发布路径。VISUAL Editor 提供自定义文字的 `InsertLink`；折叠光标调用 `SetLink` 时以 URI 作为默认显示文字，提交与撤销仍只有一步。
- 专项覆盖富文本/Markdown、中文文字、空文档、选区替换、实际 DLL Editor 光标/Undo。富文本光标及 Markdown 光标/空根/替换选区的分配失败预算分别为 57/183/129/119 点，失败不发布树、源码或历史并回收分配。`validation-2026-09-23-link-insert-final.log` 的独立内核、实际 DLL、652 项语料、Renderer/View/Editor、原生与异步绘制通过；编辑协议、无障碍 62/0、严格 C 编译、DLL 新 API 导出及 diff 检查通过。图片链接和完全无损 CST 仍未交付。

## 已有结构回写补全

- 解析阶段记录完整块语法边界，覆盖空块、分隔线、围栏结束、Setext 下划线、front matter、引用/列表和脚注定义；Live 使用这些边界，并对按引用顺序输出的脚注重新按源码位置分区。
- 共同结构写回器覆盖嵌套列表/引用中的跨样式替换和段落拆分/合并、节点删除/移动、资源修改，以及 Markdown 表格增删行列。一个事务可以连续操作同一张表，表头和对齐保持一致。
- 只改写受影响顶层块，保留其他原文及块间引用定义；回写后统一重解析并验证完整语义。引用块及列表项内的链接引用定义可按各自原始空隙保留：首空隙定义在非任务项中移到正文之前，后续空隙定义可移到普通项或任务项正文之后。嵌套列表/引用中的单段文字修改、段落拆分或相邻段落合并可只补丁受影响的段落语法范围；夹层定义可保留原字节移到合并段落之后；加粗、斜体和引用链接段落也覆盖在内。其他未覆盖情况明确拒绝，避免静默丢失。
- 增加嵌套结构、表格连续操作、空节点位置、资源修改、引用保留、拒绝操作原子性，以及 Editor 嵌套编辑/跨模式撤销用例。
- 修复源码转义将文本拆成多个 run 后的输入光标定位，验证连续输入标点再输入普通文字仍停留在原段落。

## 图片资源渲染批次（2026-09-23）

- Renderer 专项 `validation-2026-09-23-image-renderer-final.log` 验证富文本/Markdown 图片节点在资源缺失时占位、后注册时按固有尺寸并受宽度约束、替换时重测、移除时回退。实际 `drawSurface` 的来源和目标矩形、资源旧 handle 销毁、View 调用 `InvalidateObjects` 后的内容高度均断言通过。
- `validation-2026-09-23-image-native.log` 的原生 XGE `--verify` 在共享 Markdown 预览中绘制彩色 surface，截图 `native-smoke.png` 已人工检查；像素门槛验证预览区有至少 1,000 个指定绿色像素。`validation-2026-09-23-image-native-async.log` 的异步输入、预览发布和共同 Undo 也通过。该结果不代表文件/网络加载、真实图片解码、资源自动失效、公式、Mermaid 或 HTML 已完成。
- 最终 `validation-2026-09-23-image-final.log` 的完整 Document 套件通过：独立 core、实际 DLL、652 项 Markdown 语料、Renderer、Editor、原生同步/异步绘制。修改的 C 文件和原生示例经 `-Wall -Wextra -Werror` 语法检查；`validation-2026-09-23-image-exports.log` 检查实际 DLL 的两个新增对象失效导出；`git diff --check` 通过。

## 图片插入与属性编辑批次（2026-09-23）

- 根事务新增 `TxnInsertImage` 和 `TxnUpdateImage`：富文本支持显式宽高，Markdown 经过影子树回写和语义核对，显式宽高返回不可表达。更新保留图片 NodeId；富文本 HTML 导出保留宽高。VISUAL Editor 可插入/更新，SOURCE 和只读编辑器拒绝。
- 独立内核用例覆盖空根、选区替换、Markdown GFM 表格单元格、alt/地址/标题/尺寸、稳定身份及 Undo/Redo。富文本插入、Markdown 插入与更新的 25/78/70 个分配失败点均验证内容、源码、历史不发布且分配回收。实际 DLL Editor 用例验证两类文档的插入/更新和单步撤销。
- `validation-2026-09-23-image-commands-final.log` 的完整套件通过：独立 core、实际 DLL、652 项 CommonMark 语料、Renderer/View、Editor、原生同步/异步绘制。`image-commands-edit-contract.log` 和 `image-commands-accessibility.log` 分别通过通用编辑协议与辅助技术 62/0；`image-commands-exports-final.log` 验证四个新增 DLL API 导出。修改的 C 源码/测试通过 `-Wall -Wextra -Werror` 语法检查，`git diff --check` 通过。图片剪贴板、拖放及真实平台交互不在这批验证结论内。

## Markdown 图片局部回写与资源自动失效（2026-09-23）

- 修改已解析的 Markdown 图片时，优先只替换其行内语法范围，重解析并比较完整语义后才接受；无法证明局部回写等价时回退到结构回写。包含引用定义的引用/图片旁边保留未触及的实体、粗体、CRLF 和定义原文；图片 NodeId 和 Undo/Redo 保持稳定。图片更新的分配失败扫描扩展至 74 点，失败不发布半成品。
- XUI 命名资源注册表新增可查询的代次；Set/Remove/Touch 后，独立 Renderer 在后续排版/绘制时重测已测量的命名图片，View 在更新帧刷新缓存和布局。无命名图片的文档不因资源代次变化而重排。替换注册资源时先分配新记录，并清理旧资源在其他记录中的依赖引用。富文本/Markdown 测试覆盖缺失→注册→替换→Touch→移除、surface 销毁、绘制矩形、自动高度和文档 revision 不变。
- `validation-2026-09-23-image-local-resource-final.log` 通过独立 core、实际 DLL、652 项 CommonMark、Renderer、Editor 及原生同步/异步绘制；`image-resource-layout-cache.log` 专项证明无图片文档的布局次数不受无关资源变化影响，`image-resource-render-schedule.log` 通过现有资源依赖/渲染调度回归。`image-resource-exports.log` 检查新增注册表代次 API 的实际 DLL 导出，`image-resource-edit-contract.log` 与 `image-resource-accessibility.log` 分别通过通用编辑协议和辅助技术 62/0。修改的 C 源码与测试通过 `-Wall -Wextra -Werror` 语法检查；`git diff --check` 通过。该批次尚无真实文件/网络图片加载、资源策略与滚动锚点的验收证据。

## Markdown 图片插入局部回写（2026-09-23）

- 在同一文字节点内插入图片或替换选区时，只对两个精确来源边界之间的源码打补丁，重解析并比较完整语义；不能证明等价时回退结构回写。专项验证折叠光标、选区、相邻实体/粗体、CRLF、引用定义及 GFM 表格单元格的原样保留；图片 NodeId、撤销、重做保持稳定。
- Markdown 插入的分配失败扫描为 82 个预算点，逐点验证失败时文档树、源码、历史不变和分配回收。`validation-2026-09-23-image-insert-local-final.log` 记录完整 Document 套件；严格 C 语法与 diff 检查通过。完整来源无损重写、宿主授权的文件/网络加载、异步缓存、图片链接与剪贴板尚无验收证据。

## 图片编码资源加载（2026-09-23）

- `xuiDocumentImageResourceLoadMemory/File` 接收宿主提供的编码字节或明确授权的路径，在 XUI 所有者线程通过 proxy 解码、校验编码/像素限额并注册命名 surface。Markdown 图片地址不会自行访问文件或网络；解码后像素校验不能充当解码前的安全预算。失败替换保留旧资源，成功替换与移除释放旧 surface，文档历史不参与资源更新。
- `validation-2026-09-23-image-resource-load-renderer.log`、`image-resource-load-teardown.log` 与 `image-resource-load-limits.log` 覆盖编码/像素限额、IO 失败、旧资源保留、实际文件读取、替换/移除/context 销毁释放与注册表变化。原生 `--verify` 使用 XGE 解码仓库 PNG 并检查有效尺寸；`validation-2026-09-23-image-resource-load-final.log` 记录完整回归，`image-resource-load-exports.log` 检查实际 DLL 导出。文件/网络 URI 解析、异步解码、并发/缓存预算和恶意图片解码前防护仍无完整验收证据。

## 图片后台解码与所有者线程发布（2026-09-23）

- `xgeImageInfoMemory` 在像素缓冲分配前读取编码尺寸。`xuiDocumentImageResourceLoadFileAsync/LoadMemoryAsync` 的 worker 读文件或独立复制的字节、预检像素数并解码；`Poll` 在 XUI 所有者线程上传和发布。默认编码与像素限额同同步接口。取消、同名新请求及原资源代次变化使旧结果不发布；Release 等待 worker 结束并清理未发布像素。
- `validation-2026-09-23-image-async-renderer-final.log`、`image-async-extra.log` 和 `image-async-probe-final.log` 覆盖文件/内存成功、输入缓冲覆写、字节与像素超限、无效格式及其零输出、取消、未轮询释放、过期资源和同名取代；原生 `--verify` 通过实际 XGE 文件读取、后台解码和 GPU 上传。`validation-2026-09-23-image-async-final.log` 为完整 Document 回归，`image-async-exports.log` 检查实际 DLL 导出，严格 C 编译与 diff 检查通过。尚未交付自动 URI provider、网络抓取、并发队列/缓存预算以及恶意编码图片的完整防护。

## 图片请求并发与编码缓冲预算（2026-09-23）

- 异步请求按 context 排队，最多 4 个 worker、64 个未完成请求和 64 MiB 已复制输入。`Poll` 处理同 context 的完成结果再启动排队项；同名新请求取消旧排队项并释放其编码副本。`GetAsyncStats` 只读返回 running/queued/outstanding/copied bytes。
- `validation-2026-09-23-image-queue-first.log`、`image-queue-budget.log`、`image-queue-preflight.log` 和 `image-queue-limit-final.log` 的专项用例覆盖 5 个请求时 4+1 分配、只轮询队尾推动队列、65 个请求的限额、80 次同名更新及 4×16 MiB 后拒绝再复制。`image-queue-final.log` 的完整 Document 回归通过；最终实现还在复制前检查累计额度，并在解码前限制 XGE 整数像素计数范围。该批次尚无已发布资源缓存预算、阅读锚点和自动 URI provider 的验收证据。

## 可重复运行的验证

仓库根目录执行，GCC/windres 必须位于 PATH：

```bat
call test_xui\build_document_suite.bat artifacts\xui-document-rebuild\commonmark-0.31.2.json
```

不传语料路径时明确跳过官方语料。文件取自 [CommonMark 0.31.2 官方样本](https://spec.commonmark.org/0.31.2/spec.json)，SHA256：

```text
d431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20
```

| 验证目标 | 结果与含义 |
| --- | --- |
| 独立 core | 通过；不链接窗口/Renderer；3,000 次随机编辑、800 条随机移动、4 个快照读取线程、身份/正逆位置映射/事务/表格/IO/Markdown 回写 |
| 实际 DLL | 同一核心用例链接主 DLL 后通过，验证公开导出和运行时集成 |
| CommonMark 官方语料 | 652 项解析、原文保持、原生序列化重载、顶层语法范围有序/不重叠/不越界通过；**不是 HTML 输出规范一致性测试** |
| 独立块家族 | 新增 8,700 次三方言 LF/CRLF 逐字节编辑，5,925 次局部候选与完整加载树/来源/历史一致；连续代码/列表/段落位置映射及邻块 OOM/取消 163/149 点通过 |
| 列表项命令 | 富文本/Markdown 拆分、空项退出、嵌套/任务/中间项/编号/CRLF、VISUAL Enter 与 Undo；213/184 点分配失败原子性通过 |
| 正文来源片段 | 1,733 个片段、30,940 个双亲和性来源位置有效；EXACT 来源位置逐字节往返 |
| 顶层块来源检索 | 652 项语料、30,940 个双亲和性位置及局部编辑后的 1/10 MiB 抽样，与原完整扫描的节点、偏移和来源映射逐项相同 |
| 行内语法与引用 | 340 个嵌套范围、84 条定义、70 处实际引用关联符合边界/嵌套/指向约束 |
| 历史与快照内存 | 真实释放字节、共享去重、双预算裁剪、分组/分支/超额 Redo、失败不变和四线程并发诊断通过 |
| Prepare 差分/故障/取消 | 各 652 项加载/双补丁/连续候选差分，124 轮分配预算、231 轮取消位置；另有延续 122 轮、流式 75 轮故障及 76 种分片位置 |
| Prepare 计费发布 | 240 次与同步物理内存/历史对照、零分配发布、计划失效/回退 OOM、四线程诊断并发 120 次提交 |
| LIVE 来源光标 | 652 项语料、15,470 个有效 UTF-8 边界通过；整篇源码回退由 40 项降为 0，并设断言防止退化；另测空块、CRLF 与前置/乱序脚注 |
| Renderer/View | 富文本/MD、嵌套单元格、宽窄多视图、绘制、命中、局部更新、10,000 段惰性布局与容器嵌入检查 |
| Editor | Source/Visual/Live 交错编辑和共同撤销、选区还原、输入/marks/剪贴板、中文预编辑/取消/确认、只读和通用编辑协议 |
| SOURCE 异步 Editor | 暂停 worker 仍可输入/选择/复制/预编辑；一次 Undo、失败重试、显式取消、外部写入和析构回收通过 |
| 有序输入协调 | 重复 Undo/Redo 与后续文字/导航/IME/指针 FIFO、直接 API/保存阻塞、无 prepare 的 OOM 重试、容量/取消/外部写入与回调销毁通过 |
| 自动输入分组 | 富文本与三种 MD 模式、跨发布/到达超时、编辑边界、保存点、首尾选区、单步额度及 IME 分配失败重试通过 |
| SOURCE 局部投影 | 1,302 项完整重建差分、1/10 MiB 各 128 次换行局部扫描/缓存复用；SOURCE Editor 暂停 worker 的输入/Update/代理绘制与共同 Undo 样本通过 |
| SOURCE 导航/尺寸 | 实际 Editor 的 SOURCE/LIVE 字素与自然词移动匹配完整文本 oracle；CRLF/emoji/跨缓存边界、删除和 IME 通过；1/10 MiB 中部导航往返及尺寸缓存失效差分通过 |
| SOURCE 通用选区 | 1/10 MiB 已提交与待输入的通用选区往返不增加 Document 分配；反向、折叠、UTF-8 边界和错误不变通过 |
| 真实 XGE | 三栏实际 GPU 渲染；Source、Live 各三帧；每栏有文本像素，模式切换产生不同显示结果；PNG 已人工查看 |
| 基础回归 | context、widget、layout、input、text、edit_contract、rich_edit 通过；accessibility 为 62 passed / 0 failed |

最新 core 逐点分配失败扫描分别覆盖：普通文档编辑 12 轮、Markdown 解析（含引用记录）358 轮、表格结构 89 轮、Markdown 结构 95 轮、嵌套 Markdown 结构 177 轮、Markdown 表格 326 轮、列表项拆分/退出 213/184 轮、单项缩进/嵌套提升/顶层提升 160/284/164 轮、同列表范围缩进/嵌套提升/顶层提升 217/290/226 轮、EXTENDED mark 插入/清除 98/52 轮、选区链接富文本创建/Markdown 局部创建/完整清除/跨样式创建/部分清除 18/120/62/135/165 轮、链接插入富文本光标/Markdown 光标/空根/替换选区 57/183/129/119 轮、语义全部替换 85 轮、源码全部替换 83 轮、Markdown 身份/复合事务 288 轮、定界符候选 121 轮、预算/计费 10 轮。另注入 Undo、Redo 发布前两处分配失败。轮数包含首个成功预算；失败分支检查无半提交和分配平衡，不能将这些数字写成对应数量的实际失败调用。

文件用例包括正常保存/打开、UTF-8 路径、覆盖保存、不可写入目标失败后 dirty 不变，以及保存旧快照成功时新版本仍然 dirty。

## 性能样本

以下是 core 的本机测量：4 次预热、64 次同一位置的单字节交替替换并提交，历史受默认 256 步 / 64 MiB 预算约束；使用 XRT 单调微秒时钟。不含 IO、UI 排版、GPU 或 IME，不代表所有命令和文档的 P95。

| 固定样本 | P50 | P95 | 最大值 |
| --- | ---: | ---: | ---: |
| 10,000 段富文本，末段局部修改 | 0.002 ms | 0.002 ms | 0.023 ms |
| 102,480 字节 Markdown，首标题局部修改 | 21.679 ms | 24.496 ms | 25.850 ms |

富文本一次局部提交分配 15 次，增加 1,576 字节存活内存；本次构造 10,000 段约 34 ms。Markdown 加载约 11 ms，一次编辑仍走全量解析，分配 180,083 次。以上来自 `validation-2026-09-23-memory-budget.log`。富文本当前版本占用 10,346,618 字节，历史额外占用 1,403,006 字节（70 步）；Markdown 当前版本占用 5,740,462 字节，历史额外占用 64,866,837 字节（17 步），已按 67,108,864 字节预算裁剪。当前内容和外部快照分别拥有存储，不包含在历史额度中。

表中两个固定样本达到设计提出的 8 ms / 50 ms 初始目标，但 1/10 MiB 异步响应、Renderer 缓存预算和完整场景性能验收仍缺失。时间会随本机负载变化，历史保留策略也已变化，不能将两次测量差值视为性能改进证明。

## 原生运行与证据

```bat
call examples\xui_document\build.bat
build\xui_document.exe
build\xui_document.exe --verify
build\xui_document.exe --verify-async
```

左栏为富文本 Editor，中栏为 Markdown Editor，右栏为同一 Markdown Document 的只读预览。Source/Visual/Live MD 按钮切换中栏模式。

本地输出：

- `build/xge.dll`、`build/xge.lib`。
- `build/xui_document.exe`。
- `artifacts/xui-document-rebuild/validation.log`。
- `artifacts/xui-document-rebuild/validation-2026-09-22.log`（本轮完整测试）。
- `artifacts/xui-document-rebuild/validation-2026-09-22-final.log`（光标修复后的 DLL/UI/原生渲染与基础回归复核）。
- `artifacts/xui-document-rebuild/validation-2026-09-22-kernel.log`（Document 身份、位置映射和事务正确性修复后的完整测试）。
- `artifacts/xui-document-rebuild/validation-2026-09-22-source-segments.log`（持久化来源片段及双亲和性映射）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-inline-syntax.log`（行内语法、引用关联和无损清除 marks）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-memory-budget.log`（共享存储计费、历史预算和快照诊断）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-prepare.log`（detached prepare、取消/过期/发布、差分和 1/10 MiB 内核样本）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-prepared-publication.log`（后台计费计划、同步对照/并发及发布/回收分项计时）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-continued-stream.log`（持久化连续输入、候选代次并发、流式 UTF-8 原子性及语料差分）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-async-source-editor-final.log`（SOURCE Editor 异步投影、调度、失败/生命周期与原生像素验证；本批最终通过日志）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-source-projection-final.log`（SOURCE 局部投影差分、1/10 MiB 输入样本、整套回归与原生像素验证；第五批最终通过日志）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-ordered-editor-verified.log`（有序事件、保存阻塞、容量/时间预算与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-editor-callbacks-final.log`（回调重入、取消/外部替换/销毁、预编辑与转换边界，以及整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-input-groups-final.log`（交互输入跨发布分组、超时/边界、选区书签、IME 重试与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-source-navigation-final.log`（按需 SOURCE Unicode 导航、Renderer 尺寸缓存、1/10 MiB 导航样本与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-source-navigation-edit-contract.log`、`source-navigation-accessibility.log`（通用编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-direct-source-selection-final.log`（通用 SOURCE/LIVE 直接选区、1/10 MiB 零 Document 分配与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-direct-source-selection-edit-contract.log`、`direct-source-selection-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-source-row-tree-final.log`（SOURCE 行树、局部节点分配、1,302 项 Renderer 差分与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-source-row-tree-edit-contract.log`、`source-row-tree-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-lazy-provenance-final.log`（后缀来源片段共享、跨版本位置映射、语义重写、故障与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-lazy-provenance-edit-contract.log`、`lazy-provenance-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-no-history-accounting-baseline.log`、`no-history-accounting-final.log`（无历史基线计费访问前后样本、禁用历史路径与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-no-history-accounting-edit-contract.log`、`no-history-accounting-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-syntax-index-final.log`（行内语法后缀持久化偏移、八次连续编辑/快照/历史、10 MiB 样本与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-syntax-index-edit-contract.log`、`syntax-index-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-indexed-locator-final.log`（有序块/行内语法定位、根边界计算、10 MiB 禁历史首中尾段落样本与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-indexed-locator-edit-contract.log`、`indexed-locator-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-offset-final-verified.log`（顶层块持久偏移、未变后缀节点共享、完整解析回退及 174 点失败注入、10 MiB 样本与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-offset-edit-contract.log`、`block-offset-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-source-index-corpus.log`、`source-index-final.log`（有序块来源检索与完整扫描差分、10 MiB 样本及整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-source-index-edit-contract.log`、`source-index-accessibility.log`（编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-crlf-matrix-final.log`（独立块家族、8,700 次 LF/CRLF 差分、邻块安全、来源游标、故障注入与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-boundaries-edit-contract.log`、`block-boundaries-accessibility.log`（独立块批次的编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-list-commands-final.log`（列表项拆分/退出、来源回写、213/184 点失败注入及整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-list-commands-editor-chain.log`（连续 Enter 拆项、空项退出、输入、三次 Undo 的实际 DLL Editor 复核）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-list-commands-edit-contract.log`、`list-commands-accessibility.log`（列表命令批次编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-list-levels-final.log`（列表项缩进/嵌套提升/顶层提升、160/284/164 点失败注入、core / DLL / 652 语料 / Renderer / Editor / 原生绘制整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-list-levels-edit-contract.log`、`list-levels-accessibility.log`（列表层级命令批次编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-list-range-final.log`（同列表多项选择、方向/撤销、217/290/226 点故障注入、core / DLL / 652 语料 / Renderer / Editor / 原生绘制整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-list-range-edit-contract.log`、`list-range-accessibility.log`（同列表范围命令批次编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-heading-root-final.log`（段落/标题批量转换、空根插入、Ctrl+A 根选区、Markdown 表格保留、168 点失败注入及完整回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-heading-empty-oom.log`（Markdown 空根插入标题的 36 点分配失败原子性与回收扫描）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-heading-root-edit-contract.log`、`heading-root-accessibility.log`（根选区边界后的编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-style-core-final.log`（富文本对齐、显式零/默认间距、空根、JSON 往返、Undo 和 15 点失败注入）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-style-renderer-final.log`、`block-style-editor-final.log`（增量布局高度/光标几何与实际 DLL Editor 的对齐、间距及混合状态）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-style-final.log`（块样式批次的内核、实际 DLL、CommonMark、Renderer、Editor 和原生绘制完整回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-block-style-edit-contract.log`、`block-style-accessibility.log`（块样式批次的通用编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-extended-inline-final.log`（EXTENDED 高亮/上下标、来源与回写、98/52 点故障注入、Renderer/Editor/实际 DLL/语料/原生绘制完整回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-extended-inline-editor-rich.log`（补测富文本 VISUAL 的高亮、上下标互斥、状态和 Undo，实际 DLL Editor 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-extended-inline-edit-contract.log`、`extended-inline-accessibility.log`（行内格式批次的通用编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-link-final.log`（选区链接、来源保真、18/120/62/135/165 点分配预算、Renderer/View/Editor/实际 DLL/语料/原生绘制完整回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-link-edit-contract.log`、`link-accessibility.log`（选区链接批次的通用编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-link-insert-final.log`（折叠光标/选区插入链接、57/183/129/119 点分配预算、实际 DLL Editor 与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-link-insert-edit-contract.log`、`link-insert-accessibility.log`（链接插入批次的通用编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-text-style-final.log`（富文本跨 run/段落颜色、底色、字号、字体族；查询混合状态、HTML/JSON、Undo、41 点分配预算、实际 DLL Editor、Renderer 与整套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-text-style-edit-contract.log`、`text-style-accessibility.log`（样式批次的通用编辑协议与辅助技术 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-x1-edit-contract.log`（通用编辑协议以统一 Document Editor 代替旧富文本控件，通过富文本通用文本、只读、查找、撤销、能力测试及 Markdown SOURCE/VISUAL 跨模式历史）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-x1-platform-exports.log`、`x1-former-example.log`、`x1-former-example-async.log`（新 DLL 导出检查与旧示例入口的统一三视图、异步发布原生绘制通过；旧 DLL 符号仍未退场）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-style-final.log`、`document-style-suite.log`（新 `document.*` 主题颜色、富文本 Editor/Markdown View、运行时切换/透明覆盖/显式颜色优先、选区/光标、EXTENDED 高亮、revision 与几何稳定；完整 Document 与实际 DLL 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-style-edit-contract.log`、`document-style-accessibility.log`（主题批次的通用编辑协议与无障碍 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-scale-final.log`、`document-scale-suite.log`（实际 DLL 上的 1 万段富文本 Editor 可见区、滚动、热重绘、换宽、中部/跨段编辑与 Undo/Redo；10 万字节单段 View 的 shaping 复用；完整 Document、CommonMark 和原生绘制回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-scale-edit-contract.log`、`document-scale-accessibility.log`（规模批次的通用编辑协议与无障碍 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-fractional-final.log`、`document-fractional-suite.log`（48 段分数坐标、单/双行行距、换行亲和光标、命中与滚动后像素对齐；完整 Document、CommonMark、实际 DLL 和原生绘制回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-fractional-edit-contract.log`、`document-fractional-accessibility.log`（分数坐标批次的通用编辑协议与无障碍 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-table-geometry-nested.log`（独立 Cell 几何、嵌套表格、视口外查询与最大合法 1024 行跨度专项）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-table-final-suite.log`（新增 Cell 接口下独立 core、实际 DLL、652 项语料、Renderer/Editor 及原生同步/异步绘制完整通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-document-table-edit-contract.log`、`document-table-accessibility.log`（表格几何批次通用编辑协议与无障碍 62/0 回归；四个新 API 的 DLL 导出和严格 C 语法另行验证）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-column-width-core-failures.log`（富表格列宽的快照/Undo、原生 JSON/HTML、行列/合并维护、Markdown 策略和 13/63/33 点故障注入）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-column-width-geometry-first.log`、`column-width-suite.log`（实际 DLL 的列宽/缩放/跨度/水平溢出/ChangeSet 重排；独立 core、652 项语料、Renderer/Editor 与原生同步/异步绘制全套回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-column-width-edit-contract.log`、`column-width-accessibility.log`（列宽批次通用编辑协议及无障碍 62/0 回归；严格 C 语法与两个新 DLL 导出另行确认）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-drag-editor-external.log`（实际 DLL 指针边界拖动、捕获、实时列宽、单步 Undo/Redo、文本选择、外部修改取消、只读/Markdown 策略）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-drag-suite.log`（加入 Editor 列宽拖动后的独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 与原生同步/异步绘制完整回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-drag-edit-contract.log`、`table-drag-accessibility.log`、`table-drag-exports.log`（通用编辑协议、无障碍 62/0 与七项新表格/Cell API 导出检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-navigation-editor.log`、`table-navigation-suite.log`（表格 Tab/Shift+Tab 导航、合并单元格、只读及 Markdown VISUAL；拖动取消时释放指针捕获；前者为 Editor 专项，后者为完整 Document 套件）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-navigation-edit-contract.log`、`table-navigation-accessibility.log`（本批通用编辑协议与无障碍 62/0 回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-structure-suite.log`、`table-structure-editor.log`（行列增删、末尾 Tab、富文本拆分、空 GFM Cell 源码位置映射及整表删除/Undo；前者为完整套件，后者为追加边界后的 Editor 专项）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-structure-edit-contract.log`、`table-structure-accessibility.log`、`table-structure-exports.log`（通用编辑协议、无障碍 62/0 与 DLL 导出回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-merge-editor.log`、`table-merge-suite.log`（富文本选区表格合并、非法跨度查询、Markdown 禁用、Undo 与完整 Document 套件）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-merge-edit-contract.log`、`table-merge-accessibility.log`、`table-merge-exports.log`（合并命令后的通用编辑协议、无障碍 62/0 与 DLL 导出检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-cell-provenance-suite.log`（显式空 Cell 的零宽源码锚点、双亲和性正逆映射、短行合成 Cell 的近似映射；独立内核、实际 DLL、652 项语料、Renderer/Editor 与原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-cell-provenance-shift-core.log`、`table-cell-provenance-shift-dll.log`（追加的局部源码编辑位移、旧快照不变、NodeId 保留与 Undo，用独立内核及实际 DLL 复核）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-cell-provenance-edit-contract.log`、`table-cell-provenance-accessibility.log`（通用编辑协议与无障碍 62/0 回归；未插桩 MD4C 的 `-Werror` 编译及 diff 检查另行通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-matrix-suite.log`（Rich/Markdown 纯文本 TSV 矩阵粘贴、扩行扩列、CRLF/短行、合并拒绝、单步 Undo、Rich 226 点和 Markdown 598 点分配失败扫描；独立内核、实际 DLL、652 项语料、Renderer/Editor 与原生同步/异步绘制通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-matrix-exports.log`、`table-matrix-edit-contract.log`、`table-matrix-accessibility.log`（新事务 API 的实际 DLL 导出、通用编辑协议与无障碍 62/0）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-quoted-suite.log`（TSV 双引号字段中的 Tab、换行、转义引号、格式错误和 Markdown 不可表达性；Rich/Markdown 的 Editor 剪贴板和单步 Undo；Rich 普通/引号 226/106 点、Markdown 普通/引号 598/400 点故障注入；Renderer 多行 Rich Cell 光标；独立 core、实际 DLL、652 项语料及原生绘制完整回归）。
- `artifacts/xui-document-rebuild/validation-2026-09-23-table-quoted-edit-contract.log`、`table-quoted-accessibility.log`、`table-quoted-exports.log`（通用编辑协议、无障碍 62/0、实际 DLL 编辑接口导出）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-table-rectangle-final-suite.log`（`CopyTableMatrix` 的 Rich/Markdown 带引号 TSV、空格与合并跨度；VISUAL Alt 拖动矩形、选中 Cell 着色、Rich/Markdown Editor 与只读 View 的通用复制、反向拖选后的左上角粘贴、模式切换清理，以及禁用未实现的矩形 Delete；独立 core、实际 DLL、652 项语料、Renderer/Editor 和原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-table-rectangle-edit-contract.log`、`table-rectangle-accessibility.log`、`table-rectangle-exports.log`（通用编辑协议、无障碍 62/0，以及 `SnapshotCopyTableMatrix`、`ViewGetTableSelection` 的实际 DLL 导出）。
- `artifacts/xui-document-rebuild/table-rectangle-clear-core.log`（矩形清空的 Rich 合并跨度、Markdown 源码往返、部分相交原子拒绝、一步 Undo，以及 Rich 122 点/Markdown 216 点分配失败扫描）。
- `artifacts/xui-document-rebuild/table-rectangle-clear-editor.log`（实际 DLL Editor 的矩形 Delete、Backspace、Cut、通用删除接口、只读、Rich 合并跨度与 Markdown 清空/撤销专项）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-table-rectangle-clear-final-suite.log`（改动后的独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步与异步绘制全套通过；随后仅增加 Markdown 表头清空往返及空文本/通用剪切测试，分别在更新后的 core/Editor 专项再次通过）。
- `artifacts/xui-document-rebuild/table-rectangle-clear-edit-contract.log`、`table-rectangle-clear-accessibility.log`、`table-rectangle-clear-exports.log`（通用编辑协议、无障碍 62/0、`TxnClearTableMatrix` 的实际 DLL 导出）；未插桩 MD4C 的严格 C 编译和 `git diff --check` 通过。
- `artifacts/xui-document-rebuild/validation-2026-09-24-subtree-copy-final-suite.log`（完整子树复制的 Rich 新 ID/嵌套 marks/资源/表格列宽、跨文档保留 Snapshot、Markdown 插入、不可表达属性原子拒绝和 Undo/Redo；Rich 21 点、Markdown 81 点分配失败扫描；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/subtree-copy-core-final.log`、`subtree-copy-dll-final.log`（套件后新增 Rich→空 Markdown 成功复制与 Undo/Redo，更新的独立 core 与实际 DLL 再次通过）。
- `artifacts/xui-document-rebuild/subtree-copy-exports.log`、`subtree-copy-edit-contract.log`、`subtree-copy-accessibility.log`（`TxnCopySubtree` 实际 DLL 导出、通用编辑协议与无障碍 62/0；未插桩 MD4C 严格 C 编译及 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-range-copy-final-suite.log`（`TxnCopyRange` 的 UTF-8 部分文字、跨 marks/链接 run、完整图片、跨段落首尾裁剪、Rich→Markdown、Undo/Redo、部分表格原子拒绝，以及 Rich 35 点/Markdown 118 点分配失败扫描；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 与原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/range-copy-exports.log`、`range-copy-edit-contract.log`、`range-copy-accessibility.log`（`TxnCopyRange` 实际 DLL 导出、通用编辑协议、无障碍 62/0；新增片段源码通过 `-Wall -Wextra -Werror` 语法检查，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-native-fragment-final-suite.log`（独立可序列化 fragment：来源文档/快照释放后再插入，行内和多块结构、Unicode、marks/链接、Rich/Markdown、完整表格列宽与 Cell、损坏头部拒绝、Undo/Redo；Rich 35 点和 Markdown 118 点插入故障扫描。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 与原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/native-fragment-exports.log`、`native-fragment-edit-contract.log`、`native-fragment-accessibility.log`（六个原生 fragment API 的实际 DLL 导出、通用编辑协议及无障碍 62/0；严格 C 语法和 `git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-native-fragment-editor-final-suite.log`（`TxnReplaceRangeWithFragment` 在 Rich/Markdown 的文字光标分裂行内 run 或段落，Undo；34/82 点分配失败扫描。Editor 复制发布纯文本+原生片段，跨文档粗体与多块粘贴、Markdown、损坏原生数据纯文本回退、写入回调重入及 Cut/两次 Paste 读取取消；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 与原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/native-fragment-editor-exports.log`、`native-fragment-editor-edit-contract.log`、`native-fragment-editor-accessibility.log`（新增事务 API 实际 DLL 导出、通用编辑协议及无障碍 62/0；新增片段/Editor 源码通过严格 C 语法，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-native-fragment-view-final-suite.log`（只读 View 和 VISUAL Editor 对零纯文本长度、空 alt 图片选区仍发布原生片段，跨文档粘贴保留图片资源并可 Undo；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 和原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/native-fragment-view-exports.log`、`native-fragment-view-edit-contract.log`、`native-fragment-view-accessibility.log`（实际 DLL 导出、通用编辑协议、无障碍 62/0；新增片段、Editor 与 View 编辑适配源码通过严格 C 语法和 `git diff --check`）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-rich-md-conversion-corpus-suite.log`（Rich→Markdown 逐节点损失分析与严格转换；保留来源快照、跨段落样式、方言差异、GFM 表格、列宽及重新解析不等价拒绝；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步与异步绘制完整通过）。
- `artifacts/xui-document-rebuild/rich-md-conversion-exports.log`（新分析与转换 API 的实际 DLL 导出通过；新增转换源码与片段源码通过 `-Wall -Wextra -Werror` 语法检查，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/rich-md-conversion-edge-core.log`、`rich-md-conversion-edge-dll.log`、`rich-md-conversion-edge-exports.log`（最终补测 Cell 内换行定位与块级 HTML 转换；独立 core、重建的实际 DLL 和两个新导出均通过；转换源码、片段源码与内核测试通过严格 C 语法和 diff 检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-rich-md-loss-policy-suite.log`（显式授权的 marks/文字样式/块样式/图片尺寸/表格列宽舍弃，以及未授权原因、Cell 换行、合并跨度和解析差异拒绝；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步与异步绘制全通过）。
- `artifacts/xui-document-rebuild/rich-md-loss-policy-exports.log`（策略转换 API 的实际 DLL 导出通过；转换与测试源码通过 `-Wall -Wextra -Werror` 语法检查，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-markdown-rich-conversion-suite.log`（Markdown→Rich 来源字节与已用/未用引用定义报告、显式损失接受、链接资源/标题和表格语义复制、原快照释放来源 Document 后独立转换、空文档、富文本编辑/Undo；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步与异步绘制完整通过）。
- `artifacts/xui-document-rebuild/markdown-rich-conversion-exports.log`（两个 Markdown→Rich API 的实际 DLL 导出通过；转换和测试源码通过严格 C 语法、`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-html-fragment-suite.log`（`FragmentExportHtml` 在来源 Document 释放后导出行内 marks/链接及整块结构；VISUAL Editor/只读 View 同时发布原生、纯文本、HTML，零文本长度的空 alt 图片保留 `<img>`；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步与异步绘制完整通过）。
- `artifacts/xui-document-rebuild/html-fragment-exports.log`（新 API 的实际 DLL 导出通过；修改的 C 源码/测试通过 `-Wall -Wextra -Werror` 语法检查，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-html-import-suite.log`（HTML→统一 Rich fragment、VISUAL HTML-only 粘贴与一步 Undo；常见块/行内结构、实体、样式、图片、表格列宽、安全 URL 与活动内容跳过；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 和原生同步/异步绘制完整通过）。
- `artifacts/xui-document-rebuild/html-import-core-post-suite.log`、`html-import-editor-post-suite.log`、`html-import-exports.log`（套件后新增代码块/引用块/规则线与不规则表格拒绝、HTML 查询/读取取消后的原子性专项通过；新导入 API 的实际 DLL 导出通过。修改的 C 源码/测试通过严格编译，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/html-import-editor-markdown.log`（HTML-only 粗体片段粘贴到 GFM Markdown，经统一 Document 回写为 `**` 并可一步 Undo；Editor 专项通过）。
- `artifacts/xui-document-rebuild/html-import-edit-contract.log`、`html-import-accessibility.log`（通用编辑协议通过；辅助技术回归 62/0，通过代理测试，不代表真实读屏验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-html-object-roundtrip-suite.log`（HTML 片段中的行内/块公式、Mermaid、HTML 源码、front matter、扩展、脚注/引用、任务列表状态与对象元数据导出→导入→插入保持；段落对齐/显式间距、字体族 CSS 转义、列宽/结构与不可信输入边界专项通过。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 和原生同步/异步绘制完整通过）。
- `artifacts/xui-document-rebuild/html-object-final-core.log`、`html-object-boundary-core.log`（对象节点类型、资源/info/标题、块/行内标记和任务列表不产生空段落；未闭合对象拒绝。HTML 源码及测试严格 C 编译、GCC `-fanalyzer` 通过）。
- `artifacts/xui-document-rebuild/html-object-exports.log`（实际 DLL 的 Document/Editor 导出复验通过；`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-image-clipboard-final-suite.log`（单图选区附带真实 PNG、原生/HTML/PNG 优先级、PNG-only 粘贴与一步 Undo、坏 PNG 和只读拒绝；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步与异步绘制完整通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-png-memory-final.log`、`validation-2026-09-24-png-win32-clipboard-final.log`、`validation-2026-09-24-image-clipboard-exports.log`（RGBA8 直通/预乘 alpha/带步幅 PNG 往返、Win32 注册 PNG 格式与字节读回、两个新编码 API 的 DLL 导出通过；修改的 XUI 源码与测试通过严格 C 语法检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-drag-autoscroll-final-suite.log`（DocumentView/Editor 拖动选区越过固定高度视口后立即及持续自动滚动，回到视口或松开后停止；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 和原生同步/异步绘制完整通过。修改的 View/编辑适配器通过严格 C 与 GCC `-fanalyzer`，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-old-rich-unlinked-final-suite.log`、`validation-2026-09-24-old-rich-unlinked-exports.log`（`xui_sources.bat` 不再链接旧 RichDocument/RichEdit；重复高频指针事件不加速拖动滚动。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor 和原生同步/异步绘制完整通过。`objdump -p build/xge.dll` 检查 `xuiRich*` 导出为零，新 Document/Editor/fragment/图片接口仍导出；公开头和旧独立测试仍待迁移）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-semantic-accessibility-final-suite.log`、`validation-2026-09-24-semantic-accessibility-editor-final.log`（VISUAL 富文本/Markdown 的 Document 语义树、链接/图片激活、表格与 Cell 边界、任务项切换及 Undo、选中文字状态、只读/View、SOURCE 文本框与扩展 Markdown 节点枚举；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步/异步绘制通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-semantic-accessibility-edit-contract.log`、`validation-2026-09-24-semantic-accessibility-platform-a11y.log`、`validation-2026-09-24-semantic-accessibility-exports.log`（通用编辑协议、平台无障碍代理 62/0、新节点几何 DLL 导出通过；修改的 Renderer/View/适配器与测试通过严格 C 和 GCC `-fanalyzer`，`git diff --check` 通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-message-document-final-suite.log`、`validation-2026-09-24-message-document-final.log`（MessageList 嵌入 Markdown/Rich Document 的高度、绘制、滚动命中、拖动选区、复制、富文本链接激活、文档更新、解绑及字体/颜色刷新；无字体排版失败不残留绑定。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步/异步绘制全套通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-message-list-audit-final.log`、`validation-2026-09-24-message-list-legacy-final.log`（独立 MessageList 源码审计改为链接实际 DLL 的 Document 符号；原有显示行映射、冷/热命中、可见行绘制与大量消息节点规模测试通过，常规消息列表测试也通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-legacy-api-removal-suite.log`、`validation-2026-09-24-legacy-api-removal-message-audit.log`、`validation-2026-09-24-legacy-api-removal-example.log`、`validation-2026-09-24-legacy-api-removal-exports.log`（公开头、内部声明、旧实现及旧 API 专项移除后的独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、原生同步/异步绘制、MessageList 审计和统一富文本示例构建通过；导出测试确认新 Document/MessageList 入口存在且代表性 `xuiRich*` 入口不存在）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-legacy-api-removal-export-audit.log`、`validation-2026-09-24-legacy-api-removal-edit-contract.log`、`validation-2026-09-24-legacy-api-removal-accessibility.log`（完整 DLL 导出表中 `xuiRich*` 为零，Document 四个主要入口均存在；统一编辑协议和无障碍代理回归通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-find-highlight-suite.log`、`validation-2026-09-24-find-highlight-exports.log`（统一 View/Editor 的全部/当前匹配颜色、向前向后导航、结果查询、文档修改/切换与 Markdown 三模式重算通过；完整内核、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 文档与原生同步/异步绘制均通过，四项新查找接口 DLL 导出通过；严格 C/analyzer 和 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-embedded-editor.log`（普通容器嵌套的 Editor 位置、世界坐标点击与焦点、兄弟 View 同 Document 更新、销毁父节点后的焦点清理及剩余 View 继续工作通过；Editor 全专项复跑通过，严格 C/analyzer 和 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-hit-test-final-suite.log`、`validation-2026-09-24-embedded-editor-final.log`（旧算法下稳定失败的短行右侧空白、邻 Cell 与混合字号行命中专项在修复后通过；嵌入式 Editor 真实指针事件验证行尾与文档下方选区端点。完整内核、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 文档、原生同步/异步绘制通过；最终 Editor 全专项复跑通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-object-source-full-suite.log`、`validation-2026-09-24-object-source-exports.log`（Rich 与 Markdown 的公式、Mermaid、HTML payload 源码更新、稳定 ID、围栏冲突处理、失败原子性、只读/模式限制和 Undo；独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制全套通过；新接口 DLL 导出通过。修改的 C 文件通过严格编译与 GCC `-fanalyzer`，diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-code-fence-before.log`、`validation-2026-09-24-long-fence-before.log`、`validation-2026-09-24-shifted-fence.log`（前两项为旧路径预期失败：反引号围栏代码块内部结束行无法编辑，长围栏本可局部编辑时却丢失开围栏空格；最后一项为修复后的实际 DLL 专项通过，覆盖前方来源偏移变化、对象身份和 Undo）。
- `artifacts/xui-document-rebuild/validation-2026-09-24-fenced-text-final-suite.log`、`validation-2026-09-24-fenced-text-exports.log`（代码块及 Mermaid 的安全围栏回写并入完整回归：独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList、原生同步/异步绘制通过；对象源码新导出通过。严格 C/analyzer 与 diff 检查通过）。
- `artifacts/xui-webview-probe/validation-2026-09-24-w0-final2.log`、`capture-xui.png`、`capture-xge.png`（独立 Win32 与实际 XGE HWND 的 WebView2 Runtime/SDK、C/COM/Loader、内存 HTML、DOM 脚本及 PNG 输出；XUI TextEdit 与网页分区同窗的真实绘制。仅为 WebView W0 部分可行性，未验收正式控件或中文输入；见 [W0 记录](XUI_WEBVIEW_W0.md)）。
- `artifacts/xui-webview-probe/validation-2026-09-24-capture-clipped-probe.log`、`validation-2026-09-24-document-web-render-first.log`（通用 WebView PNG 请求的隐藏取消、1×1 可见区域保留完整 viewport、PNG 解码及像素；本地 KaTeX/Mermaid 页面分别输出 41×27 与 72×140 的非空图像）。
- `artifacts/xui-webview-probe/validation-2026-09-24-static-provider-viewport.log`、`build/webview/xui_document_web_provider.png`、`build/webview/xui_document_web_provider_edited.png`（真实 XGE DocumentView 的静态公式与 Mermaid 绘制；Markdown 来源事务后按新 revision 重渲染，目标画面有 10,657 个字节变化，且较宽图形的可见右边界增大；两张目标 PNG 已人工查看）。测试入口为 `test_xui/build_document_web_provider_test.bat`，需要启用 WebView2 的 DLL 和前台窗口。
- `artifacts/xui-webview-probe/validation-2026-09-24-static-provider-message.log`、`build/webview/xui_document_web_message.png`（同一真实窗口测试又把 provider 绑定到 MessageList：异步完成后两次重测，公式与 Mermaid 都在消息气泡内实际绘制，最终消息行高 281；PNG 已人工查看）。`validation-2026-09-24-message-provider-invalidation.log` 用代理对象独立检查尺寸变化时的消息行重测、解绑后的错误状态。
- `artifacts/xui-webview-probe/validation-2026-09-24-palette-inflight.log`、`build/webview/xui_document_web_dark.png`、`build/webview/xui_document_web_dark_resumed.png`（真实 DocumentView 中不透明暗色背景与浅色公式/Mermaid 已按像素验证；调色板代次从 1 到 4，浅色请求在处理中又切回暗色后，迟到结果未写入当前缓存；两张暗色 PNG 哈希相同并已人工查看）。页面配色依据 [Mermaid 主题配置](https://mermaid.js.org/config/theming.html) 的 base theme 变量，使用本地打包的固定版本进行实际验证。
- `artifacts/xui-webview-probe/validation-2026-09-24-provider-crop-first.log`（真实窗口验证完整 viewport 截图裁成对象矩形后仍绘制完整公式/Mermaid；同一 DocumentView 的 8 个条目从约 5.7 MiB 降至 209,176 字节，MessageList 与暗色切换测试仍通过）。`build/webview/xui_document_web_message.png` 与 `xui_document_web_dark.png` 已在裁剪后再次人工查看。
- `artifacts/xui-webview-probe/validation-2026-09-24-provider-crop-build.log`、`provider-crop-plain-build.log`、`provider-crop-document-dll.log`、`provider-crop-message-document.log`、`provider-crop-exports-plain.log`、`provider-crop-exports-webview.log`、`provider-crop-disabled.log`（两种 DLL 构建、实际 DLL Document 内核/MessageList 回归、两份导出及禁用 WebView2 行为通过；裁剪源码和测试经严格 C/GCC analyzer，脚本语法和 diff 检查通过）。完整 652 项 CommonMark/Editor/原生绘制套件在裁剪之前的同一调色板批次通过，见 `validation-2026-09-24-palette-document-suite.log`；裁剪后未重复无关的完整套件。
- `artifacts/xui-webview-probe/validation-2026-09-24-static-provider-document-suite.log`（独立 core、实际普通 DLL、652 项 CommonMark、Renderer、Editor、MessageList 与原生同步/异步绘制全部通过）；`validation-2026-09-24-static-provider-capture.log`、`static-provider-script.log`、`static-provider-local.log` 为真实 WebView2 窗口回归。修改的 C 源码与测试通过严格编译和 GCC `-fanalyzer`；普通/可选 DLL 导出分别见 `static-provider-exports-plain.log`、`static-provider-exports-webview.log`。
- `artifacts/xui-webview-probe/validation-2026-09-24-resume-html-base.log`、`validation-2026-09-24-html-render-resume.log`（重新构建可选 DLL，真实窗口控件回归通过；离线页面使用 DOMPurify、CSP 与禁脚本 iframe 对 HTML 块清理、测量并截图，检查禁用活动标签及非空像素）。`validation-2026-09-24-html-provider-final.log`、`build/webview/xui_document_web_html.png`（真实 Markdown DocumentView 经 provider 显示 HTML，红色区域 22,694 像素，截图已人工查看；MessageList 的 HTML 对象完成后失效重测并绘制，消息行高 88；原公式/Mermaid 与调色板回归在同次测试中通过）。`artifacts/xui-document-rebuild/validation-2026-09-24-html-static-full-suite.log` 的普通 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整回归通过。这只证明静态块级 HTML，不代表浏览器内交互或完整 HTML/CSS 支持。
- `artifacts/xui-webview-probe/validation-2026-09-25-private-channel-webview-build.log` 和同前缀的 `build_document_web_render_test`、`build_document_web_provider_test`、`build_webview_script_test`、`build_webview_local_test`、`build_webview_capture_test` 日志（从公开 `xui.h` 移出脚本/消息/截图声明后，重新构建可选 DLL；基础控件、Document 内部高级 provider 和相关窗口专项均零退出码）。`validation-2026-09-25-private-channel-plain-build.log`、`artifacts/xui-document-rebuild/validation-2026-09-25-private-channel-document-dll.log` 记录普通 DLL 与 Document 内核回归；禁用 WebView2 后端状态及普通/可选 DLL 导出检查也通过。内部符号仍在 DLL 导出表，当前仅完成公开头文件契约隔离。
- `artifacts/xui-webview-probe/validation-2026-09-25-zoom-widget.log`、`validation-2026-09-25-zoom-focus-window.log`、`validation-2026-09-25-zoom-pixels-window.log`（WebView2 页面缩放 1.5 倍后，PNG 视觉标记宽度变化而原生宿主矩形不变；XUI 虚拟 DPI 1.5 时宿主裁剪保持，真实鼠标点击与双向 Tab 回归通过）。普通/可选 DLL 和导出检查通过。实际显示器 DPI 为 1.0，不把此结果记作物理高 DPI 或中文 IME 验收。
- `artifacts/xui-webview-probe/validation-2026-09-25-unicode-pointer-tab.log`（真实前台窗口中系统 `KEYEVENTF_UNICODE` 向网页 textarea 连续送入中文字符及 ASCII，DOM 读取结果匹配；同次流程的原生鼠标点击和双向 Tab 焦点交接通过）。这不构成拼音 IME 预编辑、候选窗口或原生编辑器交替输入验收。
- `artifacts/xui-document-rebuild/validation-2026-09-25-footnote-dependency-nested-final.log`（来源索引新增已使用/未使用脚注定义及已解析脚注引用的实际定义索引；重复大小写标签、正文引用顺序与定义顺序相反、修改定义后重新解析、Undo 均通过。修复 MD4C 输出脚注时原地排序定义数组导致标签哈希表指针失效的问题；脚注正文可引用已有或此前未使用的另一个脚注，并按首次引用顺序输出。含嵌套脚注的 529 点解析分配失败扫描、独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过）。未启用 XUI 插桩的 MD4C 及修改的适配器/测试通过 `-Wall -Wextra -Werror` 编译。全局定义相关编辑仍保守地完整解析；未解析引用候选未纳入索引。
- `artifacts/xui-document-rebuild/validation-2026-09-25-reference-candidates-full-suite.log`（新增独立的未解析引用候选来源序列，记录链接、图片和脚注的原始标签字节范围；同一查询去重，代码跨度中的字面方括号不入索引。定义新增后候选转成已解析链接，Undo 恢复；多块候选编辑仍使用局部解析并和完整解析逐项比对候选及后缀位移。含候选的 562 点解析分配失败扫描、独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过）。未插桩 MD4C、修改的适配器和测试严格 C 编译以及 diff 检查通过。候选目前是查询位置元数据，尚未用于跨块定义变更的局部失效；该路径继续完整解析。
- `artifacts/xui-document-rebuild/validation-2026-09-25-strong-prefix-final2-suite.log`（仅清除 `__alpha &amp; beta__` 中 `alpha` 的加粗，使用数字字符引用使剩余加粗区从空格开始；候选重新解析并与目标语义核对后提交。`**`、Unicode 末字符、连续空格、前置正文、CRLF/引用定义、Undo/Redo 均通过，每次命令只解析一次；127 点候选分配失败扫描保持源码、历史和通知原子性且无泄漏。最终独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过）。新路径只读取当前语法片段；修改的源码与测试通过严格 C 编译，源码通过 GCC `-fanalyzer`，`git diff --check` 通过。其他复杂边界组合仍需补。
- `artifacts/xui-document-rebuild/validation-2026-09-25-strong-tail-suite.log`（在加粗区内仅清除后缀 `beta` 或中间词 `beta`，保留左侧末空格和右侧首空格的 marks；中间词改用另一种 strong 定界符起始右侧加粗区，防止原定界符重新绑定。`__`/`**`、Unicode、CRLF、引用定义、Undo/Redo 和单次解析专项通过。后缀 107 点、中间 165 点分配失败扫描保持源码/历史/通知原子性且无泄漏。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过）。严格 C、GCC `-fanalyzer` 和 diff 检查通过；剩余复杂格式交错与完整无损回写仍待处理。
- `artifacts/xui-document-rebuild/validation-2026-09-25-rich-md-boundary-suite.log`（共享行内写回器使用数字字符引用表达加粗 run 的首尾空格及相邻普通文字边界；两侧加粗、中间普通时交替使用 strong 定界符。富文本→Markdown 的首/尾/中间边界、Unicode、拆分普通节点和三次连续加粗往返均不报告损失，转换结果重新解析后与原语义一致。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制完整通过）。修改的写回器通过严格 C 与 GCC `-fanalyzer`，测试源码通过严格 C，diff 检查通过；未据此宣称复杂交错 marks 全部可表示。
- `artifacts/xui-document-rebuild/validation-2026-09-25-inline-mark-boundary-final-suite.log`（在共享行内写回器扩展数字字符引用边界处理：CommonMark 斜体、GFM 删除线、EXTENDED 高亮/下标/上标与普通文字相邻时的首尾空格，以及单独斜体 run 的首尾空格。专项还覆盖两段同格式间隔普通文字、三段斜体、加粗与斜体相邻、删除线与高亮相邻、下标与上标相邻。富文本→Markdown 损失分析均为零，转换结果重新解析并核对完整语义；原有加粗边界和多段测试继续通过。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制完整通过）。写回器通过严格 C 与 GCC `-fanalyzer`，测试源码通过严格 C，diff 检查通过；更复杂的格式交错仍需单独验收。
- `artifacts/xui-document-rebuild/validation-2026-09-25-mark-clear-before.log`、`validation-2026-09-25-mark-clear-after.log`、`validation-2026-09-25-mark-clear-suite.log`（修复前清除非加粗格式的部分选区虽能保持语义，却会把未选中 `&amp;` 规范化为 `\&`；修复后斜体、GFM 删除线和 EXTENDED 高亮/下标/上标的首段、末段与中间段优先移动原定界符，并保留未选中实体的源码拼写。15 个精确源码案例逐项验证 marks、单次解析、Undo/Redo；斜体前段、删除线末段和上标中段分别进行 127、123、187 点分配失败扫描，失败不发布源码/树/历史且无泄漏。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制完整通过）。源码通过严格 C 与 GCC `-fanalyzer`，测试通过严格 C，diff 检查通过；复杂嵌套定界符仍需继续补齐。
- `artifacts/xui-document-rebuild/validation-2026-09-25-nested-mark-suite.log`、`validation-2026-09-25-nested-mark-after.log`（`***alpha &amp; beta***` 的外层斜体包裹内层加粗：仅清除 `alpha` 或 `beta` 的斜体时，识别完整包裹的子语法边界，只移动外层定界符并保留 `&amp;` 与内层加粗。正式用例验证精确源码、marks、单次解析和 Undo/Redo；首段/末段分别进行 127/123 点分配失败扫描。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过；严格 C/GCC `-fanalyzer` 和 diff 检查通过）。`validation-2026-09-25-triple-rich-gap.log` 另证实 Rich→Markdown 的加粗连续、斜体两侧分段仍返回 `UNREPRESENTABLE`；`validation-2026-09-25-triple-gamma-candidate.log` 验证一种等价写法的四个内容片段 marks，后续须改进共享写回器。
- `artifacts/xui-document-rebuild/validation-2026-09-25-paired-marks-core.log`、`validation-2026-09-25-paired-marks-suite.log`（共享行内写回器将相邻文字 run 的共同加粗或斜体提到外层，另一种格式只在切换处开闭，必要边界写为数字字符引用。解决上一批发现的“加粗连续、斜体两侧分段”可表示却被拒绝的案例；反向嵌套、无空格边界和拆分 run 同样通过零损失分析、严格转换及重新解析后的完整语义核对。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制完整通过；写回器严格 C/GCC `-fanalyzer`、测试严格 C 与 diff 检查通过）。其他格式交错仍受严格往返门禁约束，不能据此认定全部可表示。
- `artifacts/xui-document-rebuild/validation-2026-09-25-nested-middle-before.log`、`validation-2026-09-25-nested-middle-core.log`、`validation-2026-09-25-nested-middle-suite.log`（修复前在 `***alpha &amp; beta***` 中只清除 `&` 的斜体，会把选中实体改写为 `&#38;`；新增局部来源补丁把完整包裹的加粗移到外层，将斜体拆为两段，原样保留中间 `&amp;` 或 `\&`。`***`/`___` 两种定界符、精确源码、marks、一次解析及 Undo/Redo 通过；109 点候选分配失败扫描保持源码/树/历史/通知原子性且无泄漏。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过；源码通过严格 C/GCC `-fanalyzer`，测试严格 C 和 diff 检查通过）。其余重叠来源与复杂格式交错仍需验收。
- `artifacts/xui-document-rebuild/validation-2026-09-25-table-arrow-control.log`、`validation-2026-09-25-table-arrow-editor-final.log`、`validation-2026-09-25-table-arrow-suite.log`（VISUAL Editor 的 ↑/↓ 若在当前 Cell 内没有排版上的下一位置，则按同一逻辑列进入相邻行；向下越过合并 Cell 的行跨度，向上进入目标 Cell 末端。对照构建关闭新回退后，合并单元格内部内容走完仍不能进入下一行；正式实现的 Rich 合并/跨行跨度、GFM 表格上下左右、只读导航和 revision 不变专项通过。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过；严格 C/GCC `-fanalyzer`、测试严格 C、diff 检查通过）。完整 Bidi 视觉移动仍待实现。
- `artifacts/xui-document-rebuild/validation-2026-09-25-table-outer-edge-before.log`、`validation-2026-09-25-table-outer-auto-before.log`、`validation-2026-09-25-table-outer-auto-editor-final.log`、`validation-2026-09-25-table-outer-edge-suite.log`（对照构建先复现最右侧边界无法命中，以及仅放开命中后自动列会重新填满视口、表格外边界不跟随拖动；现首次拖动固定其他自动列的当前显示宽度，最后一列随指针改变。固定列与自动列两种表格的外边界、光标、捕获、真实几何、连续拖动一次 Undo 和撤销后恢复自动宽度通过。实际 DLL Editor、完整 Document 套件、严格 C/GCC `-fanalyzer` 与 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-table-rectangle-enter-editor.log`、`validation-2026-09-25-table-rectangle-enter-suite.log`（Rich 矩形 Enter 清空左上角 Cell 后插入段落分隔，其他 Cell 保持原文；覆盖合并 Cell 跨度、落点、一步 Undo 与只读拒绝。Markdown GFM 矩形 Enter 的命令查询和执行均返回不可表示，源码、revision 和矩形选区不变。实际 DLL 的 Editor 专项及完整 Document 套件通过；修改源码/测试的严格 C、GCC `-fanalyzer` 与 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-trailing-reference-core-final.log`、`validation-2026-09-25-trailing-reference-suite.log`（Markdown 单个 Quote 顶层块或列表项结构回写时，按定义所在的源码空隙保留其原字节与全局顺序；首空隙定义仍位于非任务项正文之前，后续空隙定义可移到普通项或任务项正文之后，尾项以原始 LF/CRLF 结束。重解析后逐条核对全部定义原文及完整语义。专项覆盖 Quote 前/中/后空隙、重复定义与解析优先级，无序/有序列表首项、后续项及多项定义，普通项尾部、任务项中间与空任务项、后续兄弟项的引用解析、CRLF、Undo/Redo；嵌套列表内部定义仍原子拒绝。Quote/列表首项/后续项/多项/尾部/任务项分别扫描 109/118/157/163/118/147 个分配失败点，失败不发布源码、revision、历史或通知且无泄漏。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过；严格 C/GCC `-fanalyzer`、测试严格 C 和 diff 检查通过）。嵌套容器等复杂来源的完整 CST 回写仍未完成。
- `artifacts/xui-document-rebuild/validation-2026-09-25-nested-reference-core-oom.log`、`validation-2026-09-25-nested-reference-suite.log`（嵌套列表或引用内部有链接引用定义时，单段文字替换和段落拆分只补丁目标段落的来源语法范围，外围列表/引用的原文及定义不进入重写范围。候选全篇重解析后核对完整语义与全部定义原字节；列表套列表、引用套列表、列表套引用、CRLF、Undo/Redo 专项通过，包含行内定界符的复杂前缀仍原子拒绝。嵌套拆分/替换分别进行 168/112 点分配失败扫描，失败不发布源码、树、历史或通知且无泄漏。独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制完整通过；严格 C/GCC `-fanalyzer`、测试严格 C 和 diff 检查通过）。其他嵌套结构操作与完整块内 token/trivia 仍未完成。
- `artifacts/xui-document-rebuild/validation-2026-09-25-nested-inline-core.log`、`validation-2026-09-25-nested-inline-suite.log`、`validation-2026-09-25-nested-inline-link-core.log`（局部补丁将列表/引用行前缀与段首加粗、斜体、链接定界符分开，嵌套段落拆分后保持两侧文字的格式标记和链接目标；同段包含加粗文字和引用链接时，替换文字仍保持定义原字节和 Undo/Redo。加粗拆分进行 171 点分配失败扫描，失败保持状态原子性与无泄漏。独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过；新增的纯链接拆分也通过独立 Core；严格 C/GCC `-fanalyzer`、测试严格 C 和 diff 检查通过）。其他嵌套结构操作与完整块内 token/trivia 仍未完成。
- `artifacts/xui-document-rebuild/validation-2026-09-25-nested-merge-before.log`、`validation-2026-09-25-nested-merge-core-final.log`、`validation-2026-09-25-nested-merge-suite.log`（先复现嵌套列表含定义时两个相邻段落无法合并；现仅重写两个段落及其可证明为空白的容器行。列表套列表、引用套列表、列表套引用、加粗段落、CRLF、定义原字节、Undo/Redo 专项通过；两段之间若有定义，仍在提交前原子拒绝。合并路径 131 点分配失败扫描无半提交、幽灵通知或泄漏。独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过；严格 C/GCC `-fanalyzer`、测试严格 C 和 diff 检查通过）。定义夹在被合并段落之间等复杂来源移动仍待完成。
- `artifacts/xui-document-rebuild/validation-2026-09-25-gap-merge-core-oom.log`、`validation-2026-09-25-gap-merge-suite.log`（嵌套相邻段落合并现在可以跨越完整的来源级引用定义：仅将定义和周围空白容器行按原字节移到合并段落之后，其他非空白来源仍拒绝。第一段位于列表标记行时保留原始无序/有序/任务标记，并区分首行与续行缩进。覆盖嵌套无序、有序、GFM 任务列表、引用套列表、列表套引用、CRLF、重复定义顺序及第一条链接目标、Undo/Redo；新路径 131 点分配失败扫描无半提交、幽灵通知或泄漏。独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制完整通过；严格 C/GCC `-fanalyzer`、测试严格 C 和 diff 检查通过）。完整块内 token/trivia 与其他嵌套结构回写仍未完成。
- `artifacts/xui-document-rebuild/validation-2026-09-25-table-keyboard-editor-final.log`、`validation-2026-09-25-table-keyboard-suite.log`（富文本 VISUAL Editor 增加当前单元格列宽缩小、增大及恢复自动的统一命令与 Ctrl+Alt+Shift 快捷键，每次 8 逻辑单位；合并 Cell 作用于跨度最右列。最右侧自动列首次键盘调整固定其他自动列的当前显示宽度，使外边界实际移动，Undo 一步恢复；查询覆盖自动状态、上下限、跨 Cell 选区、只读及 Markdown 明确拒绝。实际 DLL 的 Editor 专项和完整 Document 套件通过，包括独立 Core、652 项 CommonMark、Renderer、MessageList 与原生同步/异步绘制；严格 C/GCC `-fanalyzer`、测试严格 C 和 diff 检查通过）。跨视图滚动及真实键盘/辅助技术仍需发布验收。
- `artifacts/xui-document-rebuild/validation-2026-09-25-table-insert-suite.log`、`validation-2026-09-25-table-insert-empty-editor.log`（VISUAL Editor 新增表格创建入口；Rich/GFM 在空文档和段首/段末直接插入，在段落中部拆分并插入，首格可继续输入，撤销一步恢复原段落、Markdown 源码与选区，重做恢复网格。非法尺寸、非折叠选区、SOURCE、只读和 CommonMark 均拒绝且不改变 revision。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；修改的源码通过严格 C 与 GCC `-fanalyzer`，测试源码通过严格 C，diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-cross-list-core-final.log`、`validation-2026-09-25-cross-list-core-oom.log`、`validation-2026-09-25-cross-list-empty-order-core.log`、`validation-2026-09-25-cross-list-editor.log`、`validation-2026-09-25-cross-list-suite.log`（同一父容器的相邻不同列表现可执行范围缩进与提升；Rich/GFM 的有序编号顺延、三组列表、空列表项、嵌套兄弟组实际文字顺序、反向选区及 Undo 均有专项验证。Editor 的 Tab/Shift+Tab 查询与执行通过实际 DLL，非列表块或不同父容器的选区明确拒绝。Markdown 缩进/提升分别完成 318/284 点分配失败扫描，源码/树/历史保持失败原子性且无泄漏。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；严格 C/GCC `-fanalyzer`、测试严格 C 和 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-object-insert-suite.log`、`validation-2026-09-25-object-insert-core-oom-final.log`、`validation-2026-09-25-object-insert-editor-final.log`（统一事务与实际 DLL 的 VISUAL Editor 现可创建行内/展示公式、Mermaid、行内/块级 HTML；块对象可在段落中拆分插入，Markdown 在保留原语义的前提下处理缺少结尾换行的对象正文与段落边缘空格，Undo 一步恢复原源码。GFM 对公式/Mermaid、非折叠块选区、只读和 SOURCE 模式按契约拒绝。公式、Mermaid、块级 HTML 和段落中拆分 Mermaid 分别完成 103/96/77/159 个分配失败点扫描，失败不发布源码、树或历史且无泄漏。完整套件含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制；修改源码与测试通过严格 C/GCC `-fanalyzer`，diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-code-rule-suite.log`、`validation-2026-09-25-code-rule-core-oom-final.log`、`validation-2026-09-25-code-rule-editor-final.log`（统一事务和 VISUAL Editor 新增代码块、分隔线创建；代码块支持单 token 语言与正文，段落中部插入可一步撤销，Markdown 内容缺少末尾 LF 时补齐并重解析核对，围栏内容含 `~~~` 时自动选足够长的围栏。Editor 可继续更新代码块正文；CommonMark、EXTENDED、Rich、只读与 SOURCE 策略均有专项。代码块和分隔线分别完成 159/137 个分配失败点扫描，失败无半提交或泄漏；完整套件包含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；修改源码与测试通过严格 C/GCC `-fanalyzer` 和 diff 检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-code-language-quote-suite.log`、`validation-2026-09-25-code-language-core-oom.log`、`validation-2026-09-25-quote-list-core.log`、`validation-2026-09-25-quote-heading-reverse-core.log`、`validation-2026-09-25-quote-editor-final.log`（代码块语言修改接入统一事务和 VISUAL Editor，保留 NodeId/正文，清除语言、非法 token 与 EXTENDED `mermaid` 语义变更均按契约处理；语言修改经过 52 点分配失败扫描。引用块公开事务和 Editor 入口可把当前或连续兄弟段落/标题包入一层，也可在光标处提升最近一层；Rich、Markdown 根容器、嵌套引用和列表项均覆盖，原块 ID、选区和单步 Undo 保持，跨非段落块选区原子拒绝。包裹/提升分别经过 95/57 点分配失败扫描，失败无半提交或泄漏。完整套件涵盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；严格 C/GCC `-fanalyzer` 与 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-footnote-final-core.log`、`validation-2026-09-25-footnote-final-suite.log`、`validation-2026-09-25-footnote-rich-multiline-core.log`（新增统一事务/VISUAL Editor 脚注创建入口：引用和定义同一提交、显式/自动标签、已有及未使用定义占用、正文光标可继续输入、选区替换、原源码 Undo；定义按正文首次引用顺序加入语义根。修复编辑已有脚注正文时将自身来源定义误判为不可改写的问题，其他引用定义仍受保护。初始正文/空正文/有旧定义分别完成 165/129/261 点分配失败扫描，失败不发布源码、树或历史且无泄漏。Markdown 多段正文因当前 MD4C 解析边界明确返回 `UNREPRESENTABLE`，不丢弃后续段落；Rich 两段创建及 Undo 已单独验证。最终独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制通过；修改源码与测试通过严格 C/GCC `-fanalyzer` 和 diff 检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-footnote-multipara-core-final.log`、`validation-2026-09-25-footnote-multipara-editor-edit2.log`、`validation-2026-09-25-footnote-definition-like-final-core.log`、`validation-2026-09-25-footnote-containers-final-suite.log`（MD4C 将顶层/列表/引用块脚注的空行与缩进续段归入同一定义，分别发出段落并延展脚注/引用定义的来源范围；未使用定义的续段不再成为根部代码块，足额缩进且形似新定义的续段仍保留为正文。专项覆盖 LF/CRLF、三段正文、格式、嵌套脚注引用、列表缩进、引用前缀、选区创建、第二段输入与 Undo。多段插入完成 212 点分配失败扫描，失败不发布源码、树或历史且无泄漏；未插桩 MD4C 严格 C 编译通过。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制。混合列表/引用前缀和复杂块式续段尚未完成）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-footnote-mixed-final-core.log`、`validation-2026-09-25-footnote-mixed-boundary-core.log`、`validation-2026-09-25-footnote-mixed-final-suite.log`（续段扫描按 MD4C 的容器顺序逐层匹配，覆盖列表内引用、引用内列表、引用/列表/引用三层嵌套、无缩进列表空白行和 CRLF；第二段正文、脚注与定义来源末端、源码往返均通过。缺失引用前缀或足额缩进时不吞并后续文字。上一批记录中的“混合列表/引用前缀”限制已解除；脚注正文的列表、代码块等复杂块续段仍未实现）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-footnote-remove-final-suite.log`、`validation-2026-09-25-footnote-remove-orphan-core.log`、`validation-2026-09-25-footnote-remove-exports.log`（统一事务与 VISUAL Editor 可删除脚注引用，并仅清理本次失去正文可达性的定义；多引用共享定义保留，自引用、脚注依赖链和嵌套引用删除均覆盖，原本未使用的 Rich 定义不受影响。列表/引用内的引用删除、大小写不同的共享标签、原源码 Undo、光标、只读与模式策略通过。最后引用、共享定义、链式清理、嵌套引用四类分别完成 61/115/72/136 点分配失败扫描，失败不发布源码、树或历史且无泄漏。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；DLL 导出检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-footnote-label-final-suite.log`（脚注创建、来源中未使用定义的占用检查、引用删除可达性及定义顺序改为共用 MD4C 的标签比较/语法判定；专项覆盖 `Ä/ä` 的 Unicode 大小写折叠、Kelvin 符号与 `K` 的不同字节长度等价、源级定义冲突、非法内部空白以及 76/77 字节语法边界。独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制完整通过；适配器与命令源码通过严格 C/GCC analyzer 和差异检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-message-cross-final2-suite.log`、`validation-2026-09-25-message-cross-focused-final2.log`、`validation-2026-09-25-message-cross-audit-final2.log`（MessageList 可从 Document 拖到另一 Document 或普通文本，也可从普通文本拖进 Document；正反向复制一致，中间 Document 返回全文结构范围，系统消息与 `sText` 回退文字不进入复制结果。真实指针事件还覆盖两条 Document 消息不同选区颜色的绘制、正文外空白拖选、剪贴板、文档事务后的端点映射、清空选区与重新绑定消息时的失效。大列表审计改为直接测统一端点路径，并验证原纯文本拖选、8,192 条消息索引及长正文布局。最终完整套件包含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；严格 C/GCC analyzer 及差异检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-message-lazy-count-suite.log`、`validation-2026-09-25-message-lazy-final-audit.log`（MessageList 对 2,000 段 Markdown 消息离屏时只取块高估算，初次进入视口测量 10/2,001 个块，跳至深处累计测量 23 个块；后续消息位置随高度修正，缩窄宽度后重新按需布局，显式 ScrollToEnd 在末尾精排后保持底部定位，解绑后恢复纯文本路径。原纯文本 8,192 消息索引、长正文显示与命中审计通过；完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制。单个极长块的 shaping 与长期缓存预算尚未验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-message-image-pre.log`、`validation-2026-09-25-message-image-fix.log`、`validation-2026-09-25-message-image-suite.log`、`validation-2026-09-25-message-image-audit.log`（先复现 MessageList 中的已测量 Markdown 图片在命名资源注册后未自动失效；修复后通过 `xuiUpdate` 检测资源代次，并让可见的已精确测量消息在跳过布局前查询 Renderer 是否因资源变化失效。真实消息列表专项覆盖图片注册、尺寸替换、移除后的行高和后续消息位置，Document revision 不变；全量 MessageList 审计覆盖纯文本 8,192 条消息与多块长文档，完整 Document 套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-shape-reflow-font.log`、`validation-2026-09-25-shape-reflow-font-suite.log`、`validation-2026-09-25-shape-reflow-font-audit.log`（视觉 Renderer 将文本 shaping 缓存在旧视口附近的块内，宽度改变时重新计算断行/对象几何，离屏块释放缓存；仅在 Document revision、字体对象、DPI 与缓存 run 顺序仍匹配时复用，发现不匹配时回退完整排版。10 万字节正文加 9 字节加粗尾段冷排 shape 100,009 字节，两次改宽没有重复 shape；复用与全新 Renderer 的高度及多处正文/加粗尾段光标坐标一致；切换字体对象后改宽重新 shape 全文。独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList、原生同步/异步绘制和 MessageList 8,192 消息审计通过；单段落首次按需 shaping、完整段落 Bidi 与缓存内存预算仍缺失）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-renderer-cache-suite.log`、`validation-2026-09-25-renderer-cache-audit.log`（Renderer 新增逐视图 32 MiB 默认块布局缓存软预算，可设定字节上限并查询当前占用/预算淘汰次数。VISUAL 与 SOURCE 的 64 KiB 约束测试覆盖滚动回访重排、同视口复用、改宽、对象失效、SOURCE 局部输入投影与清理；整篇内容可见时允许超过软预算，缩回小视口后回落，`GetSize.exact` 从真退回估算。实际 DLL、独立 Core、652 项 CommonMark、Renderer/Editor、MessageList、原生同步/异步绘制完整通过；MessageList 8,192 条与多块 Document 审计通过。严格 C/GCC analyzer 与差异检查通过。预算只计块内文本、clusters 和几何数组，不限制目录、字体、外部资源及当前可见块；首次排版极长块和完整段落 Bidi 仍待实现）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-long-code-slices-suite.log`、`validation-2026-09-25-long-code-slices-audit.log`（超过 8 KiB 的多行代码块按完整硬换行拆成 Renderer 布局段，仍属于同一个 Rich/Markdown 代码节点。96,256 字节、2,048 行样本产生 12 段，首屏仅 shaping 8,225 字节；Rich 的深处光标、命中、实际代理绘制、分段边界 BEFORE/AFTER 光标与跨段选区着色、整段高度及开头插入换行后的变更重建与全量排版对齐，Markdown 解析的相同正文也通过首屏按需布局和深处命中。非零行距跨分段只计一次，背景盒延伸覆盖该行距。完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制；8,192 条 MessageList 审计通过；严格 C/GCC analyzer 和差异检查通过。真实 GPU 的超长代码专项、无硬换行超长行、普通自动换行段落的首次按需 shaping 与 Bidi 仍未验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-resource-anchor-complete-suite.log`、`validation-2026-09-25-resource-anchor-renderer-v2.log`、`validation-2026-09-25-resource-anchor-message-audit.log`（固定高度 Rich/Markdown View 中，命名图片放大和缩小后，滚动量随块高变化，屏幕同一点命中相同节点与偏移；未曾测量的离屏图片也会在资源变化时补测并保留可见正文位置。自定义对象 provider 显式失效和同一段落内公式上方尺寸变化均保留阅读行；只释放含对象块，纯文本 shaping 缓存不变。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；MessageList 8,192 条及多块长文档审计通过。`validation-2026-09-25-resource-anchor-webview-build.log` 与 `validation-2026-09-25-resource-anchor-web-provider.log` 进一步验证当前源码的可选 Windows WebView2 DLL 以及 DocumentView/MessageList 公式、Mermaid、HTML 内部 provider 的真实窗口回调。严格 C/GCC analyzer 与差异检查通过。资源登记仍按全局代次触发，主题/DPI 与更复杂同块重排锚点尚未验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-zoom-anchor-before.log`、`validation-2026-09-25-zoom-anchor-iterative.log`、`validation-2026-09-25-zoom-anchor-full-suite.log`（先复现固定高度 View 放大时滚动量不变、阅读段落离开视口，再以语义位置及行内相对高度为锚点迭代精排新 Renderer。富文本与 Markdown VISUAL 放大/缩回后保留可见段落；SOURCE/LIVE 放大/缩回后同一屏幕采样点仍对应同一源码行，LIVE 活动源码区也延续。SOURCE 的绝对 scroll 数值可因离屏行高从估算转为精确而变化。最终完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；`validation-2026-09-25-zoom-anchor-webview-build.log`、`validation-2026-09-25-zoom-anchor-web-provider.log` 验证可选 Windows WebView2 DLL 和 Document 内部高级 provider 的真实窗口回归。严格 C/GCC analyzer 与差异检查通过。主题/DPI 锚点及真实鼠标滚轮缩放窗口专项尚未验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-dpi-anchor-before.log`、`validation-2026-09-25-dpi-anchor-complete-suite.log`、`validation-2026-09-25-dpi-anchor-renderer-all-modes.log`、`validation-2026-09-25-dpi-anchor-message-audit.log`（先复现虚拟 DPI 改变后固定高度 View 仍沿用旧滚动值；修复后 Rich VISUAL 中 DPI 相关公式高度增大，屏幕同点保持原节点与偏移，Rich/Markdown VISUAL 往返后保留可见段落，Markdown SOURCE/LIVE 往返后保持可见源码行。独立 VISUAL/SOURCE Renderer 重新 shaping，并使 `GetSize.exact` 从精确退回估算；按字号创建的字体缓存同步失效。MessageList 中 2,000 段 Markdown 消息随 DPI 重新 shaping，8,192 条普通消息审计通过。最终完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；`validation-2026-09-25-dpi-anchor-webview-build.log` 与 `validation-2026-09-25-dpi-anchor-web-provider.log` 复测可选 Windows WebView2 DLL 和 Document 内部公式、Mermaid、HTML provider 的现有功能；严格 C/GCC analyzer 与差异检查通过。测试使用虚拟 DPI 和代理对象，实际物理多 DPI、主题字体变化及可选 WebView2 provider 的 DPI 尚未验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-theme-font-before.log`、`validation-2026-09-25-theme-color-before.log`、`validation-2026-09-25-theme-font-all-modes.log`、`validation-2026-09-25-theme-font-message-audit.log`、`validation-2026-09-25-theme-font-complete-suite.log`（先复现切换默认主题字体后 Rich 固定高度 View 沿用旧滚动位置，以及 MessageList 仅改变默认文字色就重建内嵌 Document Renderer。修复后独立 Renderer 重新 shaping 且 `GetSize.exact` 退回估算；Rich 固定高度 View 滚动随字体变化并保留可见段落，自动高度 View 重算高度，Markdown SOURCE/LIVE 保留可见源码行；显式 normal 字体的 View 保持原高度。MessageList 的 2,000 段文档消息随默认字体变化重建 Renderer；纯颜色变化保留 Renderer 与 shaped 字节数。完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制，8,192 条普通消息规模审计通过。`validation-2026-09-25-theme-font-webview-build.log` 与 `validation-2026-09-25-theme-font-web-provider.log` 复测可选 Windows WebView2 DLL 及 Document 内部公式、Mermaid、HTML provider 的现有功能。严格 C/GCC analyzer 与差异检查通过。仅验证默认字体对象替换，样式字号/字体族、物理 DPI 和真实平台 IME/读屏仍未覆盖）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-message-reading-before.log`、`validation-2026-09-25-message-anchor-audit.log`、`validation-2026-09-25-message-anchor-final-audit.log`、`validation-2026-09-25-message-anchor-complete-suite.log`（先复现 2,000 段 Markdown 消息在深处改变默认字体后跳到其他段落；修复后列表保存可见语义位置和行内相对高度，在字体、虚拟 DPI、命名图片及显式公式对象高度变化时补测前缀并恢复同一段落。图片和公式的放大、缩小均验证离屏块实际更新；2,003 块的消息离屏测量 0 块、首次可见 9 块，多次转换后累计测量 207 次。最终源码的全量 MessageList 审计含 8,192 条普通消息门槛；完整 Document 套件含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制。`validation-2026-09-25-message-anchor-webview-build.log` 和 `validation-2026-09-25-message-anchor-web-provider.log` 复测可选 Windows WebView2 DLL 及 Document 内部公式、Mermaid、HTML provider；源码严格 C/GCC analyzer 与 diff 检查通过。测试仍为代理虚拟 DPI，不覆盖真实跨显示器物理 DPI 或复杂同块布局）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-clear-format-complete-suite.log`、`validation-2026-09-25-clear-format-exports.log`（Rich/Markdown 共用清除选区行内格式事务和 VISUAL 命令；保留选区外格式，局部复合 Markdown 格式及链接可回写并 Undo，已无格式片段不拆分。Rich/Markdown 分配失败分别覆盖 51/64 点，失败不发布源/树/历史。完整套件包括独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；新 API 导出、严格 C/GCC analyzer 与差异检查通过。折叠光标待用格式、复杂定界符无损保真和真实平台输入仍待补）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-large-paragraph-before.log`、`validation-2026-09-25-large-paragraph-lines-v2.log`、`validation-2026-09-25-line-index-table.log`、`validation-2026-09-25-line-index-final-suite.log`（100,000 字节单段落首帧约 11 ms、shape 100,009 字节；普通块视觉行索引后，热帧 Draw 检查 12,156 个片段、命中检查 274 个、深处光标检查 18 个。改宽复用原 shaping，表格第二行坐标重定位后仍正确命中。最终套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList、原生同步/异步绘制；严格 C/GCC analyzer 与差异检查通过。计数是内部片段访问量，非绘制耗时基准；首次按需 shaping 未完成）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-caret-clear-v3.log`、`validation-2026-09-25-caret-clear-final-suite.log`（VISUAL 折叠光标清除格式仅设置待用状态，不改变 revision；后续单行输入在同一事务中清除继承格式，并允许显式叠加待用 marks/样式。富文本和 Markdown 链接内部插字、邻接文字保留原格式、查询状态、一次 Undo 及 Markdown 原来源恢复已测。该阶段完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；严格 C/GCC analyzer 与差异检查通过。跨行输入在后续批次处理）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-caret-clear-multiline-v1.log`、`validation-2026-09-25-caret-clear-multiline-final-suite.log`、`validation-2026-09-25-caret-clear-newline-v1.log`、`validation-2026-09-25-caret-clear-newline-final-suite.log`（Rich/Markdown 的链接内部执行待用清除格式，再输入 `U\nV`；事务位置映射界定新建的跨节点范围，两行新文字无继承 marks/链接，原相邻富文本仍保留格式，Undo 恢复 Markdown 原源码。仅输入换行时跳过空范围清除，Enter 成功并保留待用状态。最终源码完整套件含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；严格 C/GCC analyzer 与差异检查通过。`validation-2026-09-25-final-webview-build.log`、`validation-2026-09-25-final-web-provider.log`、`validation-2026-09-25-final-web-render.log` 复测可选 Windows WebView2 基础控件和 Document 内部 KaTeX/Mermaid/HTML 静态渲染。跨行输入叠加显式待用 marks/样式、复杂来源语法和真实 IME 仍待验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-caret-multistyle-before.log`、`validation-2026-09-25-caret-multistyle-v3.log`、`validation-2026-09-25-caret-multistyle-final-suite.log`（先复现跨行新字未获得待用斜体与颜色；修复后清除继承格式、待用 marks、富文本颜色都在同一根事务作用于新建多节点范围。Rich/Markdown 的两行新字、邻接文字未受影响、一次 Undo 和 Markdown 原源码恢复通过；Rich 不清除格式时的跨行待用 marks/颜色也通过。最终套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；严格 C/GCC analyzer 与差异检查通过。复杂 Markdown 定界符和真实 IME 仍待验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-caret-multistyle-oom-v3.log`、`validation-2026-09-25-caret-multistyle-oom-v3-final-suite.log`（对 Rich/Markdown 跨行待用样式复合事务分别扫描 100/550 个 Document 分配失败点；失败保持内容、revision、保存点、Undo/Redo 计数与历史字节不变且无 Document 分配残留，首次成功仅发布一次，单次 Undo 恢复内容和保存状态。最终套件再次通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制。此项是事务原子性验证，不覆盖真实 IME）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-object-selection-v4.log`、`validation-2026-09-25-object-selection-final-suite.log`（实际 DLL Editor/Renderer 专项：空 alt 图片点击/API 结构选区、图像上方选中高亮、原生片段与 HTML 复制、Rich/Markdown Cut/Delete 和一次 Undo；行内公式与块级 Mermaid/HTML 的程序选中，以及 SOURCE 模式拒绝结构选择。该阶段完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；修改源码的严格 C/GCC analyzer 与差异检查通过。字素级键盘路径在后续批次验证，拖放和对象辅助技术尚未验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-object-keyboard-v1.log`、`validation-2026-09-25-object-keyboard-final-suite.log`（实际 DLL Editor 专项：对象改为单个 U+FFFC 导航字素，Rich/Markdown 空 alt 图片和富文本公式可由 Shift+Right 整体选中并以 Delete/Undo 原子处理。最终套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；严格 C/GCC analyzer 与差异检查通过。其余方向、词级/Bidi 视觉移动和对象辅助技术仍待验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-range-geometry-v3.log`、`validation-2026-09-25-range-geometry-final-suite.log`（新增 Renderer 选区矩形查询，跨 run/行的片段可合并，同一结果支持反向选区和截断缓冲；空 alt 图片、顶层 Mermaid、表格第二行、Markdown SOURCE 跨行与离屏块按需精排通过实际 DLL 专项。根结构 gap 的单对象查询只新增一个精排块；新 API 已在实际 DLL 导出。最终套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；源码严格 C/GCC analyzer 与差异检查通过。选区之前的离屏块仍可能产生估算 Y；完整段落 shaping/Bidi 和极长单段落首次按需排版仍待实现）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-range-geometry-webview-build.log`、`validation-2026-09-25-range-geometry-web-provider.log`、`validation-2026-09-25-range-geometry-web-render.log`（可选 Windows WebView2 DLL 的基础导航/窗口生命周期，以及 Document 内部 KaTeX、Mermaid、HTML 的离屏测量/截图、真实 DocumentView 和 MessageList 绘制均复测通过；这些内部渲染能力不扩展通用 WebView 对外通信契约）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-object-accessibility-v1.log`、`validation-2026-09-25-object-accessibility-final-suite.log`（VISUAL 语义节点可通过辅助技术动作选中图片等对象，结构 gap 覆盖时报告已选中；实际 DLL 测试覆盖 Markdown Editor 和只读 Rich View。完整套件含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；修改源码经严格 C/GCC analyzer、测试经严格 C 检查。真实读屏及平台桥接未验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-object-gap-before.log`、`validation-2026-09-25-paragraph-crlf-before.log`、`validation-2026-09-25-paragraph-crlf-all.log`、`validation-2026-09-25-paragraph-crlf-scale.log`、`validation-2026-09-25-paragraph-crlf-final-suite.log`（先复现空 alt 图片的前后 gap 光标同点，以及相邻 run 中 CRLF 造成重复硬换行；修复后结构光标落在对象两侧，跨 run CRLF 与代码块同 run CRLF 均只换一次。段落/标题在 shaping 前构造跨 run 连续断行投影，拆分与未拆分 run 的所有光标及后续对象坐标一致。实际 DLL 专项、10,000 段规模、10 万字节单段落、独立 Core、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制全部通过；严格 C/GCC analyzer 与差异检查通过。投影仍是一次布局内的临时数据，10 万字节首帧仍 shape 100,009 字节；完整跨样式 shaping/Bidi 及首帧按需续排未完成）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-paragraph-crlf-webview-build.log`、`validation-2026-09-25-paragraph-crlf-web-provider.log`、`validation-2026-09-25-paragraph-crlf-web-render.log`（可选 Windows WebView2 基础窗口与 Document 内部 KaTeX、Mermaid、HTML 的真实 DocumentView/MessageList 绘制、离屏测量和 PNG 捕获在本次段落排版修改后复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-lazy-ascii-unicode.log`、`validation-2026-09-25-lazy-ascii-final-suite.log`（长 ASCII 单段落首帧 shape 8,197/100,009 字节，近处命中/光标保持前缀缓存，改宽仍只 shape 前缀；多 run ASCII 的近处坐标和命中在深处滚动补全前后保持一致。混合组合字符、emoji、阿拉伯文和中文因当前 run 级行高及 Bidi 上下文风险走全量精排。`GetSize.exact` 区分估算与精确。最终套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList、原生同步/异步绘制；修改源码严格 C/GCC analyzer 与 diff 检查通过。完整投影仍扫描整段，深处请求重排全块；持久续排检查点、Unicode 安全分段和完整 Bidi 待完成）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-lazy-ascii-webview-build.log`、`validation-2026-09-25-lazy-ascii-web-provider.log`、`validation-2026-09-25-lazy-ascii-web-render.log`（可选 Windows WebView2 基础控件的窗口、焦点、缩放与生命周期以及 Document 内部公式、Mermaid、HTML 的离屏测量/截图、DocumentView/MessageList 实际绘制在此排版修改后复测通过；通用 WebView 的公开范围仍为 Windows 基础网页承载）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-underestimated-before-v2.log`、`validation-2026-09-25-prefix-height-final.log`（字号前小后大时，长段落前缀估算低于实际高度，先失败用例复现深处滚动错误命中下一块；现在先补全前置部分块再定位。字号前大后小时，首次 HitTest 在当前块高度收缩后重选目标，命中真实后续块。近处前缀仍按需 shaping，离屏节点矩形可按契约返回估算 Y；估算误差界限与持久续排检查点未完成）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-prefix-height-final-suite.log`、`validation-2026-09-25-prefix-height-webview-build.log`、`validation-2026-09-25-prefix-height-web-provider.log`、`validation-2026-09-25-prefix-height-web-render.log`（最终源码完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；可选 Windows WebView2 基础控件及 Document 内部 KaTeX/Mermaid/HTML 静态通道复测通过。修改源码严格 C/GCC analyzer 与 diff 检查通过；普通 DLL 的 WebView2 禁用状态也通过专项）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-unicode-cuts-final3-scale.log`、`validation-2026-09-25-unicode-final-suite.log`（段落/标题从完整 Unicode 断行投影选字素与正常/硬断行共同边界，前缀和全量布局使用相同切点；混合组合字符、emoji、阿拉伯文、中文的长段落首屏只 shape 前缀，并保持补全前后的近处光标与命中位置。后段中文字符使整个 shaping run 行高增大时，已提交前缀仍保持原坐标。无安全切点的 128 KiB 长行走完整 shaping；切点扫描单调前进，不重复扫后缀。代码块和 SOURCE 的全局文本偏移随 run 保存；代码块往返改宽验证不重复 shaping。规模专项、严格 C/GCC analyzer、独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制全部通过；`git diff --check` 通过。投影仍每次扫描整个段落，深处请求补全整块；持久续排检查点、完整段落 Bidi 与无断行长行的按需排版未完成）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-unicode-webview-widget.log`、`validation-2026-09-25-unicode-web-provider.log`、`validation-2026-09-25-unicode-web-render.log`（可选 Windows WebView2 基础控件与 Document 专用 KaTeX/Mermaid/HTML 离屏测量、截图、DocumentView/MessageList 绘制在本次排版修改后复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-incremental-lines-height.log`、`validation-2026-09-25-incremental-lines-final-suite.log`（至少 32 KiB 且有安全断行点的段落在 Renderer 中保留 Unicode 投影、切点、已 shape 的 run 与已提交视觉行；同块滚动只追加新片段，从上一条未提交行继续排版，不重做先前行。90 KiB 混合段落的首屏 shaping 为 8,196 字节，第一次和第二次续排分别新增 16,380 与 32,777 字节，后者包括一次故障注入后丢弃并重试的片段。近处、新增中部和更深处的光标在各阶段、失败重试及完整排版后坐标一致；冷排较高视口同样不重复 shape 前缀。模拟第二个新增 run shaping 失败时，原前缀继续可用且重试成功；冷排直接覆盖整段后，总高度与已完成 Renderer 一致，防止部分估算高度污染块高度索引。128 KiB 无安全切点长行仍完整 shape。规模测试、严格 C/GCC analyzer、独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制通过。深处精确光标/范围查询仍补全整块；跨块定位仍需先消除前置块估算，完整段落 Bidi 和无断行长行按需排版未完成）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-incremental-webview-widget.log`、`validation-2026-09-25-incremental-web-provider.log`、`validation-2026-09-25-incremental-web-render.log`（Windows WebView2 基础控件及 Document 内部 KaTeX/Mermaid/HTML 离屏测量、截图和 DocumentView/MessageList 绘制在本次布局修改后复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-message-paragraph-render.log`、`validation-2026-09-25-message-paragraph-final-suite.log`（90 KiB 单段落 Rich Document 嵌入真实 MessageList，关闭自动滚底后首次仅 shape 8,212 字节，其中包含消息标签；在同一消息内滚至约 5,000 / 16,000 像素，分别新增约 16,380 / 24,582 字节 shaping，没有补全整段。两处可见位置的文档命中保持同一文字节点且偏移前进，实际消息缓存渲染包含文字绘制。消息原有高度、选区、复制、更新和生命周期专项继续通过；完整套件的独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制通过。度量变化时极长消息的阅读锚点和更精确的高度估算仍待验收）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-caret-continue-oom.log`、`validation-2026-09-25-caret-continue-final-suite.log`（90 KiB 混合 Unicode 段落的深处文本光标查询沿缓存切点追加，返回坐标与全量排版相同，累计 shaping 少于全文三分之二，`GetSize.exact` 仍为假；多 run 斜体段落部分/全量几何一致。第二次新增 shaping 故障时原前缀仍可用且重试成功；无后续安全切点的靠尾光标全量完成。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制）。
- `artifacts/xui-document-rebuild/validation-2026-09-25-caret-continue-webview-build.log`、`validation-2026-09-25-caret-continue-web-provider.log`、`validation-2026-09-25-caret-continue-web-render.log`（可选 Windows WebView2 基础控件，以及 Document 内部公式、Mermaid、HTML 在真实 DocumentView/MessageList 中的静态绘制、离屏测量与 PNG 捕获均通过；通用 WebView 的公开能力仍按 Windows 基础网页承载收敛）。本批次修改的 Renderer/规模测试通过严格 C/GCC `-fanalyzer`，`git diff --check` 通过。
- `artifacts/xui-document-rebuild/validation-2026-09-25-range-continue-before.log`、`validation-2026-09-25-range-continue-multirun.log`、`validation-2026-09-25-range-continue-final-suite.log`（先失败用例复现深处文本终点选区触发全量 shaping；修复后 90 KiB 混合 Unicode 段落的选区只续排到终点所在已提交行，shaping 少于全文三分之二且文档尺寸仍为估算。矩形数及逐项坐标/尺寸与全量 Renderer 一致；跨两个样式 run 的部分/全量矩形数一致。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-range-continue-webview-build.log`、`validation-2026-09-26-range-continue-web-provider.log`、`validation-2026-09-26-range-continue-web-render.log`（最终 Renderer 源码的可选 Windows WebView2 基础控件、Document 内部公式/Mermaid/HTML 静态渲染及真实 DocumentView/MessageList 绘制复测通过；严格 C/GCC `-fanalyzer` 与差异检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-message-inline-object-final.log`、`validation-2026-09-26-message-single-anchor-final-suite.log`（90 KiB Rich 单段落消息的纯文本与段首命名图片混排，约 16,000 像素处的同一屏幕采样点在默认字体、虚拟 DPI 和 8 次列表改宽往返后仍命中同一文本节点，偏移变化小于 64 字节。图片高度 40→140→20 时，滚动补偿方向正确、深处文字位置保持、Document revision 不变，新增 shaping 共 81,922 字节。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；修改测试通过严格 C/GCC analyzer 和差异检查）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-object-geometry-before.log`、`validation-2026-09-26-object-range-before.log`、`validation-2026-09-26-text-gap-range-before.log`、`validation-2026-09-26-object-text-gap-final-scale.log`、`validation-2026-09-26-object-text-gap-final-suite.log`（先失败专项分别复现段落中部图片节点矩形、图片后 gap 及文本后 gap 选区触发全段 shaping；修复后 86,400 字节双文本 run 中的图片约在 37.5% 处，节点矩形及两类选区矩形数和逐项坐标/尺寸与全量排版相同，累计 shaping 少于全文三分之二，尾部仍标为估算。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制；严格 C/GCC analyzer 与差异检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-object-gap-webview-build.log`、`validation-2026-09-26-object-gap-web-provider.log`、`validation-2026-09-26-object-gap-web-render.log`（可选 Windows WebView2 基础控件以及 Document 内部公式、Mermaid、HTML 的离屏测量、截图、DocumentView/MessageList 静态绘制复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-cold-object-before.log`、`validation-2026-09-26-cold-geometry-final-scale.log`、`validation-2026-09-26-cold-geometry-final-suite.log`（先失败测试复现未排版 Renderer 的段内深处图片矩形会全量 shape；修复后先建立最小前缀，再对 90 KiB Unicode 文本光标/文本终点选区和 86,400 字节双 run 图片矩形/文本后 gap 选区按切点续排。坐标与全量 Renderer 一致，shaping 少于全文三分之二且尾部仍为估算。完整套件覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；严格 C/GCC analyzer 和差异检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-cold-geometry-webview-build.log`、`validation-2026-09-26-cold-geometry-web-provider.log`、`validation-2026-09-26-cold-geometry-web-render.log`（最终 Renderer 源码的可选 Windows WebView2 基础控件，以及 Document 内部公式、Mermaid、HTML 的离屏测量、截图与真实 DocumentView/MessageList 静态绘制复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-reference-incremental-core-final.log`、`validation-2026-09-26-reference-incremental-final-suite.log`（Markdown 已有全局链接或脚注定义时，编辑后无 `[` 且不触及定义范围的独立块可局部解析；后续定义来源坐标平移，其他块引用关联保留。定义前置/后置、重复定义、删除引用用法、未使用脚注、定义变更与引入新引用均与完整重载逐项比较，Undo/Redo 通过。67 点分配失败及 53 点取消扫描保持原子性与无泄漏。102,431 字节样本的中部段落只解析 75 字节；完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；严格 GCC `-fanalyzer` 和差异检查通过。定义编辑、新引用和跨块影响仍回退全量解析）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-text-node-rect-before.log`、`validation-2026-09-26-text-node-rect-final-scale.log`、`validation-2026-09-26-text-node-rect-final-suite.log`（先失败用例复现冷 Renderer 查询长段落文本节点矩形触发全段 shaping；修复后 86,400 字节双文本 run 与内联图片的段落只 shape 49,151 字节，前部 32,400 字节文本节点矩形的 x/y/宽/高与完整 Renderer 一致，尾部仍为估算。完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；源码严格 GCC `-fanalyzer` 和差异检查通过。该路径只在节点尾部视觉行已提交时返回部分布局结果，靠尾和无安全切点时补全）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-text-node-rect-webview-build.log`、`validation-2026-09-26-text-node-rect-web-provider.log`、`validation-2026-09-26-text-node-rect-web-render.log`（可选 Windows WebView2 基础控件构建与导航/缩放/焦点/生命周期回归通过；Document 专用公式、Mermaid、HTML 的离屏测量/截图、真实 DocumentView 与 MessageList 静态绘制及调色板切换通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-external-def-before.log`、`validation-2026-09-26-external-def-final-suite.log`（先失败用例确认独立块使用未变的外部链接定义时会全量解析；修复后把定义原始行作为仅供 MD4C 的虚拟后缀，逐项验证定义元数据与原文一致，并对局部窗口的引用展开量设限；整篇先前引用耗额的遗漏已由后续回归发现并修正。三方言 756 次编辑与完整重载的树、来源、语法和 Undo/Redo 对照，其中 747 次走局部路径；覆盖定义前置/后置、重复定义、链接/图片、Unicode 标签、实体、CRLF、front matter、BOM 与 EOF。102,385 字节样本仅解析 69 字节；114 点分配失败和 100 点取消扫描保持原子性。完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制；严格 GCC `-fanalyzer` 与差异检查通过。定义编辑、脚注及无法证明安全的场景继续全量解析）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-external-def-webview-build.log`、`validation-2026-09-26-external-def-web-provider.log`、`validation-2026-09-26-external-def-web-render.log`（同批最终源码的可选 Windows WebView2 基础控件、Document 内部公式/Mermaid/HTML 离屏测量与截图、真实 DocumentView/MessageList 静态绘制及调色板切换通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-unused-def-before.log`、`validation-2026-09-26-global-budget-before.log`、`validation-2026-09-26-used-def-before.log`、`validation-2026-09-26-definition-value-final-suite.log`（先失败用例分别复现未使用定义值修改走全量、前文引用耗尽整篇 MD4C 额度造成局部与完整解析不一致、单块使用定义值修改走全量。现在独立链接定义的目标/标题等长编辑先单独重解析定义并核对全部来源字段：未被使用时保留树，只在一个独立文本块使用时重解析该块；多块、脚注、标签/变长等不满足条件的编辑回退全量。引用额度同时按整篇所有可能查找和局部查找证明安全。三方言 210 次未使用定义差分中 144 次局部，180 次单块依赖差分中 123 次局部；102,406 字节未使用样本解析 15 字节，102,385 字节单块依赖样本解析 78 字节。未使用/单块路径分别执行 24/108 点分配失败与 18/103 点取消扫描；独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList、原生同步/异步绘制完整通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-definition-value-webview-build.log`、`validation-2026-09-26-definition-value-web-provider.log`、`validation-2026-09-26-definition-value-web-render.log`（同批最终源码的 Windows WebView2 基础控件导航/缩放/焦点/生命周期，以及 Document 内部公式、Mermaid、HTML 离屏截图、真实 DocumentView/MessageList 绘制和调色板切换复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-variable-def-before.log`、`validation-2026-09-26-variable-budget-before.log`、`validation-2026-09-26-variable-used-before.log`、`validation-2026-09-26-value-boundary-before.log`、`validation-2026-09-26-variable-definition-final-suite.log`（先失败用例确认变长定义值编辑走全量、定义长度变化可使远处引用因 MD4C 整篇额度变化而改变语义、单块依赖变长编辑走全量，以及字段边界插入走全量。修复后，独立链接定义目标/标题字段内的替换、删除和插入先局部解析定义并校验来源字段，持久化平移后续定义、块及行内语法/候选来源；零个使用保留树，一个独立块使用则以新定义重解析该块，多块/脚注/标签变化回退全量。旧版与新版整篇引用额度均保守核对。三方言未使用/单块使用各 210 次差分均有 186 次局部；102,406 字节未使用样本解析 20 字节，102,385 字节单块使用样本解析 88 字节；32/118 点分配失败、26/113 点取消扫描与两种亲和性的全部源码位置映射通过。完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-variable-definition-webview-build.log`、`validation-2026-09-26-variable-definition-web-provider.log`、`validation-2026-09-26-variable-definition-web-render.log`（同批源码的 Windows WebView2 基础控件及 Document 内部公式、Mermaid、HTML 的离屏测量/截图、真实 DocumentView/MessageList 静态绘制和调色板切换复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-multi-definition-before.log`、`validation-2026-09-26-multi-definition-final-suite.log`（先失败用例确认同一独立链接定义被多个块使用时走全量；现在目标/标题字段编辑可按源码顺序重解析最多 32 个独立依赖块，统一协调来源、节点身份和行内语法，全部成功后才发布。定义后缀与整篇引用额度校验在整组中只做一次，各块仍校验局部额度。三方言 210 次多块变长差分中 186 次局部；双亲和性来源位置与 33 块保守回退通过。102,337 字节、三处远距依赖样本只解析 217 字节；等长/变长路径分别经过 180/190 点分配失败和 173/183 点取消扫描，未发布状态不变且无泄漏。完整套件含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-multi-definition-webview-build.log`、`validation-2026-09-26-multi-definition-web-provider.log`、`validation-2026-09-26-multi-definition-web-render.log`（同批源码的 Windows WebView2 基础控件导航/缩放/焦点/生命周期，以及 Document 内部公式、Mermaid、HTML 离屏测量/截图、真实 DocumentView/MessageList 静态绘制和调色板切换复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-definition-label-before.log`、`validation-2026-09-26-definition-label-final-suite.log`（先失败用例确认独立链接定义标签修改走全量。现在对标签内等长/变长编辑先局部解析定义，再保守地重解析最多 32 个含 `[` 的独立顶层块，涵盖旧引用失效、未解析候选生效及同名定义优先级变化；不确定容器与超限回退全量。三方言 81 次标签差分中 66 次局部；来源位置双亲和性、CRLF、front matter、Unicode 折叠与完整重载一致。102,310 字节三处远距块只解析 170 字节；等长/变长路径分别经过 141/152 点分配失败和 134/145 点取消扫描。完整套件含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-definition-label-webview-build.log`、`validation-2026-09-26-definition-label-web-provider.log`、`validation-2026-09-26-definition-label-web-render.log`（同批源码的 Windows WebView2 基础控件构建、导航/缩放/焦点/生命周期，以及 Document 内部公式、Mermaid、HTML 离屏测量/截图、真实 DocumentView/MessageList 静态绘制和调色板切换复测通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-editor-stream-before.log`、`validation-2026-09-26-editor-stream-final-editor.log`、`validation-2026-09-26-editor-stream-final-suite.log`（先失败构建确认新 SOURCE Editor 流式接口尚未导出；接入后，实际 DLL 编辑器验证拆分 UTF-8、错误分片原子拒绝、待提交投影、跨帧发布、保存/模式屏障、分组 Undo、排队交互事件、帧间取消及只支持 Markdown SOURCE。完整套件通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；严格 GCC `-fanalyzer`、新 DLL 导出与 diff 检查通过）。
- `artifacts/xui-document-rebuild/validation-2026-09-26-editor-stream-webview-build.log`、`validation-2026-09-26-editor-stream-web-provider.log`、`validation-2026-09-26-editor-stream-web-render.log`（同批可选 Windows WebView2 基础控件导航/缩放/焦点/生命周期，以及 Document 内部公式、Mermaid、HTML 的离屏测量、截图、DocumentView/MessageList 静态绘制及调色板切换均通过）。
- `artifacts/xui-document-rebuild/native-async-pending.png`、`native-async-committed.png`。
- `artifacts/xui-document-rebuild/native-smoke.png`、`native-live-smoke.png`。

`build/` 和 `artifacts/` 被仓库忽略；本文件和测试脚本保存可提交的结果摘要及复现方式。原生截图验证了绘制，不能代替真实 Windows IME、物理键盘和读屏验收。

## Document 内部 WebView provider 超时验证（2026-09-29）

内部公式/Mermaid/HTML provider 的启动、响应、绘制等待和截图等待改用 `xrtClock` 的微秒单调时钟计时，公开描述中的 `iTimeoutMs` 零值为 10 秒，设置时按毫秒换算。以前按 600 次 `Update` 计数，高低帧率会改变实际超时时长；绘制等待前 5 次稳定帧也已加入时限检查。真实 Windows WebView2 页面故意不回复渲染请求，在配置的 1 秒上限后以 68 次更新显示 `response timed out` 错误卡，`failed=1`、`queued=0`、`bWorkerFailed=0`，未等待旧的 600 次更新；同一窗口专项还复测普通对象、源码变更、MessageList、缺失资源、PNG 解码/裁剪失败后的恢复。可选 DLL 与基础控件、provider、离屏 KaTeX/Mermaid/HTML 证据分别为 `artifacts/xui-document-rebuild/validation-2026-09-29-web-timeout-clock-webview-build.log`、`validation-2026-09-29-web-timeout-clock-provider.log`、`validation-2026-09-29-web-timeout-clock-render.log`，均退出码零；完整 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 与原生绘制记录于 `validation-2026-09-29-web-timeout-clock-final-suite.log`。修改源码和窗口测试的 GCC `-fanalyzer -Wall -Wextra -Werror` 均无告警，差异检查通过。本专项没有故意冻结 UI 帧，也未覆盖真实平台 IME、读屏和 DPI。

## 最终发布前的缺项

| 缺项 | 当前边界与需要补齐的内容 |
| --- | --- |
| 完整 Markdown 来源层 | 已有块边界、正文片段、行内定界符、已解析链接/脚注依赖、未解析标签候选及未使用脚注定义；需块内 trivia/token、扩展重叠来源及完整无损回写 |
| 完整视觉结构编辑 | 已支持嵌套列表/引用的段落操作、列表项拆分/空项退出/单项、同列表及相邻兄弟列表多项层级调整、段落/标题转换、富文本选区对齐/段后间距、EXTENDED 高亮/上下标、选区链接、富文本字体/颜色及 GFM 表格创建/行列、图片插入与属性编辑、公式/Mermaid/HTML 对象、代码块与分隔线创建、代码语言修改、引用块包裹/提升、脚注创建/引用删除及现有对象/代码块/脚注正文更新；单个引用块或同一列表各项内的链接引用定义可在结构回写时保持原文和顺序，后续空隙定义也覆盖任务项；嵌套列表/引用的单段文字修改、段落拆分及相邻段落合并可局部补丁以保留定义，含夹层定义的原字节移动，包含段首加粗/斜体及同段引用链接；复杂块式脚注续段、夹杂普通段落或跨父容器的列表混合选区、嵌套容器的其余无损重写、其他对象插入、复杂 delimiter、其余格式转换仍待补 |
| 解析调度 | SOURCE Editor 已接入待输入/流式追加/局部行树/按需导航/直接源码选区/顺序事件/跨发布分组/flush/后台回收；LIVE Editor 已接入同一待输入/流式工作线程、活动块候选混合排版、邻行更新及同行数原位交换；段落、标题、规则线、围栏代码、Mermaid、HTML、引用、列表和表格可在边界可证实时增量解析。已有全局链接定义且独立块出现引用查找时，可携带未变定义原文进行局部解析并验证结果与额度；独立定义标签/目标/标题字段编辑可在可证实时局部处理按需增长的独立受影响块，包括无内部定义的顶层引用块、列表和表格。顶层块和行内语法按来源范围定位，未变后缀共享、当前根可差额计费；已验证有序顶层块的源码到语义位置检索。空历史首次局部编辑的历史登记已有待命根转移；仍需脚注及含内部定义容器的失效、VISUAL 待提交其他跨叶节点或来源不精确的编辑/结构编辑/流式异步投影、LIVE 跨块候选混合显示及完整 UI 验收 |
| 原生排版 | 段落/标题在 shaping 前建立跨 run 连续断行投影，当前仍按样式 run shaping；可见块改宽可复用已 shaping 的 clusters 并重建断行/对象几何。跨 run CRLF、同 run 代码块 CRLF 只产生一次硬换行；空 alt 图片前后结构 gap 的光标在对象左右边界。已精排的普通块按视觉行裁剪 Draw/HitTest、按 run 片段偏移定位光标；表格保守回退。Renderer 可查询跨 run/行、对象、表格与 SOURCE 行的选区矩形，并按需精排选区块；此前未精排的块仍可使绝对 Y 为估算值。块内布局缓存已有逐 Renderer 软预算及离屏回收；超长多行代码块按硬换行分段并按需 shaping，长 ASCII 及含 Unicode 的自动换行段落按字素/断行切点首屏 shape 前缀，同块可见区滚动及冷/热深处文本光标、文本/同段直接子节点 gap 终点选区、段内可选对象矩形可先建最小前缀并沿缓存切点追加片段；未排尾部标为估算。命名图片、显式对象失效、缩放、虚拟 DPI、默认字体替换，以及富文本 VISUAL 字号和经回调解析的字体族变化下的固定高度 View 阅读锚点已测；段内部分文字样式拆分也保持可见文字行；未拆分的纯文字/段落颜色变化保留 shaping 与几何，仅刷新绘制属性。仍需局部选区纯颜色拆分时的无重排更新、完整段落 shaping/Bidi、无安全断行点的极长段落及无硬换行长行的按需排版、其余跨容器结构终点范围与复合节点几何的分段续排及 MessageList 复杂同块锚点、其他资源缓存预算、真实平台字体与其他模式的样式度量变化、物理 DPI 和复杂同块重排锚点 |
| 完整编辑命令 | 已有自动连续键入合并、列表项拆分/空项退出/同列表及相邻兄弟列表范围缩进与提升、段落/六级标题命令、富文本选区对齐、任意段后间距与行内字体/颜色、Rich/Markdown 选区及折叠光标后续单行/跨行输入清除行内格式并叠加待用 marks/富文本样式、图片插入和属性更新、Rich/GFM 表格创建、表格内部及最右侧外边界列宽拖动、键盘列宽命令与自动宽度恢复、Tab/Shift+Tab 导航及末尾新增行、表格行列增删、富文本合并/拆分、含引号字段的 TSV 矩阵粘贴、Alt 拖动矩形复制及矩形 Cut/Delete/Backspace、Rich 矩形 Enter；Markdown GFM 矩形 Enter 明确拒绝；仍需复杂 Markdown 来源边界、夹杂普通块或跨父容器的列表选区、其余块/对象命令、复杂列表/表格片段粘贴、完整 HTML5/CSS 导入、图片剪贴板 DIB/跨应用兼容及资产持久化 |
| 资源和高级显示 | 图片节点已可绘制 context 命名 surface，注册表变化自动触发图片重测与 DocumentView/MessageList 更新；可选 WebView2 provider 已在真实 DocumentView 和 MessageList 中静态排版、绘制公式、Mermaid 和块级原始 HTML，源码事务更新、消息高度重测及不透明调色板切换也已测。文件/网络加载与策略、细粒度缓存、DPI、不同主题多视图、辅助技术、HTML 交互和高级对象的完整错误呈现仍缺失；普通 DLL 不含 WebView2 后端 |
| 格式转换 | Rich→Markdown 已提供逐节点损失分析、严格无损转换和显式显示格式舍弃；Markdown→Rich 已提供来源损失分析与显式语义转换；结构内容降级及用户确认界面仍缺失 |
| 可访问性 | VISUAL 已按统一 Document 快照暴露段落/列表/任务项、链接、图片、表格/单元格和扩展对象的语义节点；已有激活、滚动、任务切换与 Undo；图片等对象暴露按节点选中动作和已选中状态。复杂容器精确文本范围、更多动作、原生平台桥接及真实读屏验证仍缺失 |
| 宿主与迁移 | 普通容器自动高度及 MessageList 的 Document 高度、绘制、滚动命中、Document/普通文本跨消息双向选择复制和链接激活已测；多块长文档的离屏估算、可见区精排、后续消息高度修正、宽度变化与滚动到底部已测；旧富文本公开 API、实现、DLL 导出与旧专项已移除。消息语义无障碍、90 KiB 单段落消息在默认字体、虚拟 DPI、反复改宽及同块命名图片高度变化下已验证深处阅读行；富文本字号变更的前置段落和同块部分文字重排已验证阅读锚点；其他对象/复杂样式重排、估算精度及旧专项未覆盖的新行为验收仍未完成 |
| 发布矩阵 | 需真实平台 IME、多 DPI/主题及大文档验收；Document 按实际声明的平台矩阵验证，WebView 当前仅验收 Windows，其他 WebView 后端后续逐个实现；当前结果只覆盖本机 Windows x64 |

旧富文本专项移除后的行为去向与缺项见 [迁移清单](XUI_LEGACY_RICH_TEST_MIGRATION.md)。以上 Document 缺项仍需验收；通用 WebView 的公开高级能力和其他平台后端已按 2026-09-24 决定延后，不计入 Windows 基础版完成门槛。

2026-09-26 LIVE 候选回归：Markdown LIVE Editor 的后台输入和末尾流式分片复用 SOURCE Prepare/worker，并以活动块候选源码加其余语义块构成混合投影；跨块候选回退全文源码。Renderer 的直接后继单补丁只重扫活动块内受影响行及两侧上下文，保留其他行和语义块缓存；约 82,972 字节、1,024 行的活动块连续 16 次单字节编辑并进行一次 CRLF 拆分和合并，累计扫描 4,376 字节、复用 18,468 个块次。此结果只计源码目录扫描，不含每次重建数组/高度索引、Prepare/历史、绘制和真实输入延迟。完整独立 core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制见 `artifacts/xui-document-rebuild/validation-2026-09-26-live-splice-final-suite.log`；严格 GCC `-fanalyzer` 和 `git diff --check` 通过。Windows 可选 WebView2 基础控件及 Document 专用 KaTeX/Mermaid/HTML 渲染同批通过 `validation-2026-09-26-live-splice-webview-build.log`、`validation-2026-09-26-live-splice-web-provider.log` 和 `validation-2026-09-26-live-splice-web-render.log`。

LIVE 增量正确性与输入响应后续验证：96 次连续插入、删除、替换及 CR/LF 改动逐步对照重新构建的混合 Renderer，块数量、尺寸、未改动语义锚点、候选源码光标及命中一致。该专项促使修复只有链接定义而无语义块的文档在 LIVE 光标二分定位时误走语义路径；定点用例覆盖已提交与待提交状态，652 项 CommonMark 的 15,470 个有效 UTF-8 位置全部通过。为避免同行数普通输入每次复制整篇块目录，Renderer 先建立新行，随后原位交换受影响行并更新高度索引；行数变化继续使用预先构建的数组和索引。冻结解析线程、计入输入/View 更新/代理绘制的本机 1/10 MiB 各 64 次候选输入 P95 分别为 0.074/0.147 ms，均只扫描 897 字节；修改前的两次同机样本约为 0.74/8.61 ms。Editor 专项日志为 `artifacts/xui-document-rebuild/validation-2026-09-26-live-inplace-editor.log`，Renderer 专项为 `validation-2026-09-26-live-inplace-renderer.log`。数字不包含真实 GPU、系统输入派发和后台发布时间，尚不能证明最终平台 P95。

同批完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制见 `artifacts/xui-document-rebuild/validation-2026-09-26-live-inplace-final-suite.log`；Windows 可选 WebView2 基础控件及 Document 内部 KaTeX/Mermaid/HTML 渲染见 `validation-2026-09-26-live-inplace-webview-build.log`、`validation-2026-09-26-live-inplace-web-provider.log` 和 `validation-2026-09-26-live-inplace-web-render.log`，均通过。

VISUAL 单次文本异步专项：普通段落/标题内、无格式文本叶节点的严格内部选区和精确来源映射可在本次输入达到门槛时先生成私有语义候选，已提交 Document 和其他视图保留旧内容。worker 对候选源码解析并与暂存语义树比较，结果相同才发布；发布后按源码位置重新定位语义光标和撤销书签，避免转义字符令文本节点分裂后遗留失效光标。冻结 worker、同 revision 候选进入/取消后的查找缓存刷新、保存/模式屏障、转义回写、取消、后台 OOM 后重试和 Undo/Redo 已通过实际 DLL Editor 用例；完整 Core、DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制见 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-paste-final-suite.log`。可选 Windows WebView2 基础控件及 Document 内部公式/Mermaid/HTML 静态渲染见 `validation-2026-09-26-visual-paste-webview-build.log`、`validation-2026-09-26-visual-paste-web-provider.log` 和 `validation-2026-09-26-visual-paste-web-render.log`，均通过。严格 GCC `-fanalyzer` 与差异检查通过。该批次尚无 VISUAL 连续输入、结构编辑或流式异步投影，也没有 1/10 MiB 主线程与后台耗时矩阵；其他 VISUAL 编辑仍同步，不能据此宣称完整异步验收。

VISUAL 连续文字候选专项：源码或本次输入达到默认 100 KiB 门槛时，普通段落/标题中来源精确、无格式的单文本叶节点内部编辑可先形成异步候选；同组当前光标处的后续插入生成下一代私有语义预览及顺序源码补丁，成功设置 Renderer 后才替换旧 Prepare。冻结 worker 的程序 API 与真实 `XUI_EVENT_TEXT` 用例验证连续输入、UTF-8 和转义标点、其他视图保留已提交内容、查找失效、取消、分配失败保持旧代次、worker OOM 重试及单次 Undo/Redo。实际 DLL 专项见 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-continuous-editor.log`。1/10 MiB 各 64 次连续输入加 View 更新、代理绘制，专项 P95 为 0.165/0.200 ms；完整套件同项 P95 为 0.222/0.239 ms，见 `validation-2026-09-26-visual-continuous-final-suite.log`。完整套件涵盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制；严格 GCC `-fanalyzer` 通过。样本冻结了解析线程，不含后台解析与发布、真实系统输入及 GPU；该批次结束时，待提交删除/选择替换、移位光标输入、结构编辑和流式投影仍未实现，不能据此宣称完整 VISUAL 异步验收。

同批可选 Windows WebView2 基础控件及 Document 内部 KaTeX/Mermaid/HTML 静态渲染通过 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-continuous-webview-build.log`、`validation-2026-09-26-visual-continuous-web-provider.log` 和 `validation-2026-09-26-visual-continuous-web-render.log`。通用 WebView 仍仅以 Windows 基础承载为本阶段目标，内部渲染通道保留。

VISUAL 删除专用候选专项：首次删除限于 Markdown 无格式段落/标题的同一文本叶节点内部及精确来源范围；同组连续 Backspace/Delete 沿原始未改动前缀/后缀定位源码补丁，候选仍由相同 Prepare worker 解析并与私有语义树比较。冻结 worker 的程序 API 与真实键盘用例覆盖连续及左右交替的 Backspace/Delete、UTF-8、所有者 OOM 保留旧代次、发布后原文一次 Undo；1/10 MiB 各 64 次连续 Backspace 加 View 更新、代理绘制，完整套件 P95 为 0.122/0.154 ms。实际 DLL 专项见 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-delete-mixed-editor.log`，完整独立 Core、DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制见 `validation-2026-09-26-visual-delete-final-suite.log`。严格 GCC `-fanalyzer` 与差异检查通过。该样本冻结解析线程，不包含真实系统输入、GPU、后台解析和发布；非连续/跨叶节点删除、待提交选区替换、结构编辑和流式投影未验收。

同批可选 Windows WebView2 基础控件与 Document 内部 KaTeX/Mermaid/HTML 静态渲染通过 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-delete-webview-build.log`、`validation-2026-09-26-visual-delete-web-provider.log` 和 `validation-2026-09-26-visual-delete-web-render.log`；通用 WebView 对外仍维持 Windows 基础功能范围。

VISUAL 候选来源映射专项：实际 DLL 冻结 worker 后，同一无格式文本叶节点内的待提交选区替换与移位光标插入可继续生成候选。128 次确定性插入、删除、替换逐步核对显示文本，最后核对已发布 Markdown 源码与一次 Undo；转义标点、UTF-8、正反向选区、失败保留旧候选、发布后选区复制以及编辑结束光标的 Redo 书签均已覆盖。先失败日志为 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-selection-before.log`；专项通过见 `validation-2026-09-26-visual-map-differential-editor.log`；完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制通过见 `validation-2026-09-26-visual-map-final-suite.log`。严格 GCC `-fanalyzer` 及差异检查通过。冻结 worker 的 1/10 MiB 现有连续输入快路径 P95 为 0.209/0.285 ms；该数字不包括本批选区路径、真实系统输入、GPU、后台解析与发布。交互式导航结束当前组并排队后续输入；跨叶节点、来源不精确的范围及结构编辑仍无待提交 VISUAL 异步投影。

同批 Windows WebView2 基础控件及 Document 内部 KaTeX/Mermaid/HTML 静态渲染通过 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-map-webview-build.log`、`validation-2026-09-26-visual-map-web-provider.log` 和 `validation-2026-09-26-visual-map-web-render.log`。

补充的实际 DLL 故障扫描逐一覆盖同叶节点待提交选区替换的 36 个所有者 Document 分配点；每次失败后旧候选代次与显示内容不变，释放 worker 后旧候选可正常发布，销毁后分配余额为零。专项见 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-map-oom-editor.log`；最终完整回归见 `validation-2026-09-26-visual-map-oom-final-suite.log`，包括独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制。该扫描不覆盖 `calloc` 等非 Document 分配器调用。

VISUAL 跨软换行候选专项：在普通无格式段落、两端精确来源且中间仅有纯文本或软换行时，跨文本叶节点选区的替换/删除会显示私有语义预览，再由 worker 对一条源码补丁重新解析核对；同组后续输入仅在编辑光标继续插入。实际 DLL 验证程序调用与文本事件、反向删除、两笔补丁后未改段落选区发布/复制、源码、Undo/Redo 和 Redo 光标；首次构建的 41 个 Document 分配点全部完成失败回滚及释放扫描。先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-cross-leaf-before.log`、`validation-2026-09-26-visual-cross-leaf-typing-before.log`，最终专项 `validation-2026-09-26-visual-cross-leaf-final-editor.log`；包含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制的完整套件 `validation-2026-09-26-visual-cross-leaf-final-suite.log` 通过。跨叶节点性能矩阵、含格式/对象或不同容器的选区仍未验收。

同批 Windows WebView2 基础控件及 Document 内部 KaTeX/Mermaid/HTML 静态渲染复测通过 `artifacts/xui-document-rebuild/validation-2026-09-26-visual-cross-leaf-webview-build.log`、`validation-2026-09-26-visual-cross-leaf-web-provider.log` 和 `validation-2026-09-26-visual-cross-leaf-web-render.log`。

空历史首次编辑计费专项：历史启用且无 Undo/Redo 时，以当前根的待命历史所有权承接首次真实记录；`ClearHistory` 与最后记录超额淘汰在释放前转移所有权，同步提交和后台计划发布分别处理一次。文件打开、反序列化及 Rich/Markdown 转换路径也接入待命登记。10 MiB 已清空历史的九种局部替换/插入/删除只访问 401–711 个计费对象，历史额外字节为 9,847–13,471；计数上限 `<8192` 写入语料规模测试。`artifacts/xui-document-rebuild/validation-2026-09-26-history-standby-final-suite.log` 的独立 Core、240 次后台与同步物理内存/历史对照、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制通过。10 MiB 首次完整加载仍访问 2,198,222 个对象、后台准备约 1.38 秒；这些固定样本不代表整体平台响应验收。

同批可选 Windows WebView2 基础控件、Document 内部 KaTeX/Mermaid/HTML 静态渲染与 MessageList 接入复测分别见 `artifacts/xui-document-rebuild/validation-2026-09-26-history-standby-webview-build.log`、`validation-2026-09-26-history-standby-web-provider.log`、`validation-2026-09-26-history-standby-web-render.log`，三项通过。

全篇链接引用额度缓存专项：增量解析仍按 MD4C 的保守上界检查局部及整篇引用展开，但整篇 `[` 数量改由源码版本精确计数，补丁只读被删和新增的字节。CommonMark 652 项的加载、两补丁、续接候选及原生格式反序列化均将该计数与原始字节逐项对照；原有链接额度边界、114 点分配失败及 100 点取消扫描继续通过。10 MiB、一个外部定义和一个远处引用的样本三次局部编辑只解析 528/511/528 字节，候选后台准备为 0.153/0.075/0.081 ms；这些固定测量不包含初次加载或真实平台输入。完整 Core、实际 DLL、Renderer/Editor、MessageList 及原生同步/异步绘制见 `artifacts/xui-document-rebuild/validation-2026-09-26-reference-budget-cache-final-suite.log`；严格 C/GCC analyzer 和差异检查通过。

同批可选 Windows WebView2 基础控件、Document 内部 KaTeX/Mermaid/HTML 静态渲染和 MessageList 嵌入复测分别见 `artifacts/xui-document-rebuild/validation-2026-09-26-reference-budget-cache-webview-build.log`、`validation-2026-09-26-reference-budget-cache-web-provider.log`、`validation-2026-09-26-reference-budget-cache-web-render.log`，三项通过。

定义编辑来源索引专项：先前的 10 MiB 外部定义样本在普通块编辑及远处依赖重解析后关闭了原本有效的来源索引；完整遍历仍证明块有序，错误来自节点写入清除标记后才读取标记。修复后逐次断言三次普通块编辑、定义目标加长及恢复原长度均保留索引；远处链接在加长/恢复后解析到对应目标，来源位置按新长度移动。两次定义编辑分别解析 567/543 字节，worker 固定样本为 0.095/0.085 ms；先前失去索引的同类样本约 5 ms。定义字段变长的引用额度仍取旧/新整篇保守上界，现复用精确 `[` 计数而不重新读取整篇源码。完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制通过 `artifacts/xui-document-rebuild/validation-2026-09-26-definition-index-final-suite.log`；严格 C/GCC `-fanalyzer` 与差异检查通过。样本不覆盖所有定义依赖图或真实窗口输入延迟。

同批 Windows WebView2 基础控件及 Document 内部 KaTeX/Mermaid/HTML 静态渲染复测分别通过 `artifacts/xui-document-rebuild/validation-2026-09-26-definition-index-webview-build.log`、`validation-2026-09-26-definition-index-web-provider.log` 和 `validation-2026-09-26-definition-index-web-render.log`；通用 WebView 对外功能范围保持 Windows 基础版。

独立链接定义依赖扩容专项：原先 33 个独立引用块的目标或标签编辑会回退完整解析。现在依赖集合按需增长；第 32/33、64/65、128/129 块的目标与标签编辑均增量完成，并以完整重载逐项对照语义树、来源、行内语法及 Undo/Redo。97 KiB、129 个分散引用块样本的两类编辑各解析 7,008 字节。针对首次扩容请求的分配失败和取消，已发布快照、源码与 revision 不变，销毁后分配余额为零，专项日志见 `artifacts/xui-document-rebuild/validation-2026-09-27-dependents-growth-failure-targeted.log`；完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制见 `validation-2026-09-27-dependents-growth-final-suite.log`。严格 GCC `-fanalyzer` 与差异检查通过。容器/脚注依赖和大于本次 129 块样本的性能尚未得到证明。

同批可选 Windows WebView2 基础控件、Document 内部 KaTeX/Mermaid/HTML 静态渲染与 DocumentView/MessageList 绘制分别通过 `artifacts/xui-document-rebuild/validation-2026-09-27-dependents-growth-webview-build.log`、`validation-2026-09-27-dependents-growth-web-provider.log` 和 `validation-2026-09-27-dependents-growth-web-render.log`。

独立容器定义依赖专项：12 组引用块、列表、嵌套组合、GFM 表格、CRLF 与多块引用的目标/标签编辑共 24 次增量完成，语义树、来源范围、行内语法、Undo/Redo 与完整重载一致。含内部定义容器和脚注的样本确认继续走完整解析。列表目标编辑注入 171 个分配失败点和 164 个取消点，引用块标签编辑注入 152/146 个点，已发布快照、源码与 revision 保持原状且无分配泄漏。完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制见 `artifacts/xui-document-rebuild/validation-2026-09-27-container-definition-final-suite.log`；严格 GCC `-fanalyzer`、C 语法检查和 `git diff --check` 通过。此处验证独立容器的链接定义失效，不涵盖脚注依赖、内部定义容器或真实平台响应 P95。

同批可选 Windows WebView2 基础控件、Document 内部 KaTeX/Mermaid/HTML 静态渲染以及 DocumentView/MessageList 绘制分别通过 `artifacts/xui-document-rebuild/validation-2026-09-27-container-definition-webview-build.log`、`validation-2026-09-27-container-definition-web-provider.log` 和 `validation-2026-09-27-container-definition-web-render.log`。通用 WebView 仍只对外提供 Windows 基础网页功能；Document 内部脚本及截图渲染不在此次收缩范围内。

WebView 生产导出边界：`test_xui/build_webview_exports_test.bat` 使用正常可选 WebView2 DLL，在运行时确认 20 个基础控件/Document provider 函数仍导出，11 个仅供 Document 内部使用的消息、脚本、截图和请求函数不再导出；记录为 `artifacts/xui-document-rebuild/validation-2026-09-27-webview-private-exports-final.log`。同一正式 DLL 上的 KaTeX/Mermaid/HTML DocumentView 与 MessageList 真实窗口回归见 `validation-2026-09-27-webview-private-exports-provider.log`。启用测试专用导出的 DLL 上，基础控件、脚本、同源消息、截图及离线高级渲染专项分别见同日期 `webview-private-test-build`、`webview-private-script`、`webview-private-local`、`webview-private-capture`、`webview-private-render` 日志。未启用 WebView2 的普通 DLL 重建和 Document 核心回归见 `validation-2026-09-27-webview-private-plain-document.log`。公开声明与 DLL 导出均已收缩；Document 内部渲染通道仍可工作。

脚注正文块解析专项：新增七组根、CRLF、空定义首行与引用容器中的列表、围栏/缩进代码和引用块，逐项验证块在脚注内及原始语法字节范围；先失败记录为 `artifacts/xui-document-rebuild/validation-2026-09-27-footnote-blocks-before.log`。改用 MD4C 共用块解析器后，修复空首行续段缩进和脚注末尾容器闭合范围，217 点分配失败扫描确认旧快照/源码/revision 不变、释放后无泄漏。最终 `validation-2026-09-27-footnote-blocks-final-suite.log` 通过独立 Core、实际 DLL、652 项 CommonMark 解析/源码/原生往返、Renderer/Editor、MessageList 以及原生同步/异步绘制；严格 GCC `-fanalyzer`、有/无 XUI 宏的 C 语法检查和差异检查通过。脚注依赖的局部失效与完整 CST 仍未完成。

含脚注文档的独立块增量专项：目标普通块使用方括号链接而脚注定义/用法位于别处时，现以原定义原文局部解析，并逐项比较完整重载的语义树、来源段、行内语法、引用定义/候选和 Undo/Redo。脚注引用增删、定义相交及脚注正文行内语法回调失序样本确认回退完整解析。提前定义和按引用顺序附根的多个脚注使旧根子节点不再按来源排序；本批将块定位限定于普通块前缀，并修正变长编辑后的脚注来源偏移及根来源包络。先失败证据见 `artifacts/xui-document-rebuild/validation-2026-09-27-footnote-independent-before.log`、`validation-2026-09-27-many-footnotes-before.log`。137 点分配失败、122 点取消扫描均不发布半成品且无泄漏；158,454 字节样本只解析 50 字节。完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制通过 `artifacts/xui-document-rebuild/validation-2026-09-27-many-footnotes-final-suite.log`；严格 GCC `-fanalyzer`、C 语法和差异检查通过。脚注引用/定义本身的跨块依赖失效、未排序行内语法的局部更新与完整 CST 仍待实现。

未引用脚注定义正文增量专项：仅在根没有已渲染脚注节点时，对独立定义正文内部的单补丁做局部解析；重解析片段须只保留原定义标签与边界，变长更新后续定义、块、行内语法/候选的源码范围，保守校验链接展开额度。110 个正文替换差分中 108 个局部完成，另 2 个语法不确定案例安全回退；单定义、重复定义、CRLF、链接定义混排及同步/Prepare 均对照完整重载的树、来源、引用表、Undo/Redo。46 个分配失败点、32 个取消点验证快照/源码/revision 不发布半成品且无泄漏；158,442 字节样本只解析 24 字节。完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制记录为 artifacts/xui-document-rebuild/validation-2026-09-27-unused-footnote-final-suite.log；严格 GCC -fanalyzer 和 git diff --check 通过。已引用脚注正文、脚注标签/引用及未排序脚注行内语法的局部失效仍未完成。

补充位置映射验证：未引用脚注定义变长编辑后，逐字节检查两个 affinity 的 SourceToPosition 结果、节点种类与来源范围，同完整重载一致；核心回归记录为 artifacts/xui-document-rebuild/validation-2026-09-27-unused-footnote-position.log。

已引用脚注正文局部失效专项：定义窗口与虚拟脚注引用重建单个脚注节点，外部链接定义按原有相对顺序参与解析；更新的定义表、行内语法父索引/定义索引、来源候选及根脚注顺序均与完整重载逐项对照。171 次格式/链接字符替换中 162 次局部完成，另外 9 次安全回退；定点覆盖脚注逆序、未解析链接候选、嵌套强调、脚注内列表、CRLF、重复定义、已引用定义位于普通块之前及同事务第二补丁回退。209 个分配失败点和 194 个取消点验证未发布半成品且释放后无泄漏；158,460 字节样本仅解析 53 字节。完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制记录为 `artifacts/xui-document-rebuild/validation-2026-09-27-used-footnote-final-suite.log`；后补的多次引用和 Unicode 标签折叠用例在 `validation-2026-09-27-used-footnote-unicode.log` 的独立核心测试通过；严格 GCC `-fanalyzer` 和 `git diff --check` 通过。嵌套脚注依赖、标签/标记编辑、容器内定义、多补丁旧源码窗口与完整 CST 尚未完成。

性能范围说明：53 字节只代表 MD4C 解析输入量；根普通块扫描、行内语法序列重建及发布开销未包含在该数字中，大量语法节点与真实 UI 的 P95 仍需单独测量。

已引用脚注大型语法文档专项：818,060 字节、18,000 个普通块及 36,004 条行内语法的变长正文编辑，在同机单次进程 CPU 计时中由旧实现约 12 ms 降至约 2–3 ms；解析输入仍为 53 字节。普通块定位/根包络改用有序边界，行内语法通过持久序列替换与分段来源平移保留共享存储；提交后的当前 Document 存储只增加约 956 字节。样本提交结果与独立全文重载的完整树、来源段、行内语法、定义和候选逐项相等，并以 64 KiB 增量存储上限作回归断言；其他脚注逆序、重复引用、Unicode 标签等定点差分也保留。更新后的分配失败/取消扫描各 213/198 个点，保持快照与发布原子性且无泄漏。完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制记录为 `artifacts/xui-document-rebuild/validation-2026-09-27-footnote-scale-final-suite.log`；严格 GCC `-fanalyzer` 和差异检查通过。这是单次 CPU 样本，不是 UI P95；行内语法分类仍线性读取全部记录，10 MiB 与大量脚注文档尚待测量。

同批 Windows WebView2 生产 DLL 重建后，基础控件/Document provider 导出与 11 个内部脚本、消息、截图入口不导出的边界见 `artifacts/xui-document-rebuild/validation-2026-09-27-footnote-scale-webview-exports.log`；真实 DocumentView/MessageList 的 KaTeX、Mermaid、HTML 内部渲染见 `validation-2026-09-27-footnote-scale-webview-provider.log`，两项通过。

已引用脚注同事务连续补丁专项：以前第二次源码补丁因只有事务初始 SourceStore 可用于旧依赖扫描，会对整篇回退。现在每次立即解析的补丁临时保留其紧邻旧源码；单脚注、逆序双脚注、先增长普通块再编辑脚注的两次补丁均局部完成，与完整重载及 Undo/Redo 对照一致；第二补丁 118 点分配失败扫描保持已发布版本不变且无泄漏。10,900,060 字节、100,000 普通块样本的连续两次编辑，修改前约 4.12 秒并解析约 10.9 MiB，修改后同机单次约 17–23 ms、共解析 96 字节。新增 `test_xui/build_document_footnote_perf_test.bat` 可重复运行 65 次单补丁与两次连续补丁的 10 MiB 验证，记录于 `artifacts/xui-document-rebuild/validation-2026-09-27-footnote-chain-10m-repeatable.log`；其中单补丁 CPU P95 为约 18 ms，不代表真实 Editor 输入/绘制 P95。完整套件记录为 `validation-2026-09-27-footnote-chain-final-suite.log`；严格 GCC `-fanalyzer` 通过。Prepare 批量多补丁依赖、嵌套脚注和完整 CST 未由此覆盖。

已引用脚注 Prepare 批量补丁专项：同一脚注定义正文里的两次顺序源码补丁，无论一次 PrepareSource 提交还是通过 PrepareContinueSource 续接，均在同一局部定义窗口解析一次；跨不同脚注定义的双补丁保守全文解析。三种路径同独立完整重载比较语义树、来源、行内语法、定义和候选。221 点分配失败及 199 点取消扫描确认无半成品发布和泄漏。10,900,060 字节、100,000 普通块样本的同脚注双补丁 Prepare 仅解析 48 字节、单次 CPU 约 13 ms，见 `artifacts/xui-document-rebuild/validation-2026-09-27-footnote-prepare-10m.log`；`test_xui/build_document_footnote_perf_test.bat` 已加入完整套件。最终 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList、原生同步/异步绘制见 `validation-2026-09-27-footnote-prepare-final-suite.log`；严格 GCC `-fanalyzer` 与差异检查通过。指标不包含真实 UI 输入/绘制 P95；跨定义、跨块批量依赖及嵌套脚注仍未完成。

等长脚注正文编辑的链接展开额度回归：70 KiB 长链接定义和多个方括号组成的样本在替换一个方括号后，局部候选保守回退全量解析，结果与独立重载及 Undo/Redo 一致，见 `artifacts/xui-document-rebuild/validation-2026-09-27-footnote-budget-guard-core.log`。校验现在计算编辑前、后的保守方括号预算，不再仅在来源长度变化时检查旧侧额度；最终完整套件复测仍见 `validation-2026-09-27-footnote-prepare-final-suite.log`。

Markdown 视觉输入边界空格回归：先失败用例 `artifacts/xui-document-rebuild/validation-2026-09-27-visual-boundary-before.log` 复现段落开头输入空格被裁掉、事务拒绝；修复后针对文本叶节点两端的输入空格使用字符引用 `&#32;`，并以解析后完整语义校验。普通段落前后、加粗尾部、引用块斜体头部、CRLF、邻接已有 `&amp;` 及未改动引用定义的源码拼写与 Undo/Redo 均通过。73 个分配失败预算点确认拒绝时 revision/源码不变、释放无泄漏。最终完整回归 `artifacts/xui-document-rebuild/validation-2026-09-27-visual-boundary-final-suite.log` 包含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；严格 GCC `-fanalyzer` 和差异检查通过。跨叶节点/复杂定界符和完整 CST 不属于该批验收。

Markdown 同段跨文本叶节点局部来源回归：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-27-cross-leaf-before.log` 复现替换第二个实体所在选区时，第一个未选中 `&amp;` 因整块写回而改变拼写。新路径只在两端同属一个段落或标题、位置均精确映射时生成私有来源补丁，解析后核对完整语义；不等价则回退现有写回器。正反向范围、删除、插入需转义的 `&*`、加粗与 CRLF、未触及引用定义、格式跨界回退及 Undo/Redo 已测。106 个分配失败预算点确认失败不发布且无泄漏。最终 `artifacts/xui-document-rebuild/validation-2026-09-27-cross-leaf-final-suite.log` 通过独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；严格 GCC `-fanalyzer` 和差异检查通过。该回归不声称跨段落/不精确来源或完整块内 CST 已实现。

Markdown 带格式 VISUAL 异步候选回归：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-27-visual-marked-async-before.log`、`validation-2026-09-27-visual-same-marked-before.log` 分别复现加粗跨文本叶节点替换和单叶节点内部输入直接同步提交。现在两者均先显示待提交私有预览，可在 worker 冻结时继续输入，发布后保留原有实体/引用定义拼写并一步 Undo；GFM 删除线反向选择与 CRLF 也通过。随后 EXTENDED 斜体、高亮、上下标的跨叶节点候选通过 `artifacts/xui-document-rebuild/validation-2026-09-27-visual-marked-matrix-editor.log`。包含独立格式定界符的选区仍走同步结构路径。跨叶节点 41 点及单叶节点 30 点所有者 Document 分配失败扫描确认源码不变、无待提交半成品，销毁后分配余额为零；已存在的纯文本连续候选和跨软换行扫描分别为 36/41 点。实际 DLL 专项 `validation-2026-09-27-visual-marked-final-editor.log`，矩阵添加前的完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制见 `validation-2026-09-27-visual-marked-final-suite.log`，严格 GCC `-fanalyzer` 通过。故障扫描只覆盖 Document 分配器；该批没有测带格式新路径的真实平台 P95、跨块结构编辑或完整 CST。

Markdown 链接标签 VISUAL 异步候选回归：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-27-visual-link-before.log` 证明内联链接标签中的跨叶节点文字替换直接同步提交。加入链接标记后，只接受两端及中间文本的完整属性和资源相同、来源精确且补丁范围不含原始定界符的情形；私有预览与 worker 完整语义核对不变。`validation-2026-09-27-visual-link-first-editor.log` 验证内联与引用链接在 worker 冻结时待提交，发布后目标未变、源码保留未修改的实体与定义、新文字仍有 LINK 标记，一步 Undo 返回原文。完整独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制记录于 `validation-2026-09-27-visual-link-final-suite.log`。跨链接目标、标签定界符和行内代码仍不属于这条快速候选路径。

Windows 位图剪贴板输入验证：`test/test_clipboard_win32.c` 以真实 Win32 剪贴板构造顶向下 32 位带 alpha 的 `CF_DIBV5`、底向上 24 位带行填充的 `CF_DIB`，对 XGE 生成的 PNG 解码后逐像素比较；截断栅格、重叠掩码须拒绝。注册 PNG 延迟渲染失败时，验证仍从有效 `CF_DIB` 或系统合成的 `CF_DIBV5` 获得正确像素。`test_xui/xui_document_dib_paste_test.c` 使用实际 XGE/XUI DLL，从仅有 `CF_DIBV5` 的系统剪贴板粘贴到 VISUAL Editor，检查新图片节点、命名 surface、revision、Undo 和 Redo。完整套件 `artifacts/xui-document-rebuild/validation-2026-09-27-dib-final-suite.log` 退出码零，最后的延迟格式边界分别通过 `validation-2026-09-27-dib-delayed-win32.log` 和 `validation-2026-09-27-dib-delayed-document.log`；此前实际 Editor 专项连续重复四次通过。此处只证明内建的 Win32 格式与 Document 入口；未用 Paint、浏览器或 Office 做跨进程兼容矩阵，也未验证 DIB 写出、压缩/索引色 DIB、ICC、资产重开或移动平台。

Windows 位图剪贴板输出验证：先失败的 `artifacts/xui-document-rebuild/validation-2026-09-27-dib-output-before.log` 表明原始 PNG 往返正常，但系统剪贴板没有可用 `CF_DIBV5`。新增 PNG 伴随格式后，原始 PNG 字节仍逐字节一致；Win32 原生读取 2×2 `BITMAPV5HEADER`、负高度、32 位位掩码、sRGB 标记和四个 BGRA/alpha 像素均符合预期。真实 Document VISUAL Editor 从 DIB 粘贴并选中图片复制，再读取系统 `CF_DIBV5` 的像素、资源与 Undo/Redo 通过。专项日志为 `validation-2026-09-27-dib-output-win32.log`、`validation-2026-09-27-dib-output-document.log`，最终完整套件为 `validation-2026-09-27-dib-output-final-suite.log`。这不等于外部应用兼容性或色彩管理验收；Windows 系统位图格式转换规则见 [微软剪贴板格式文档](https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats)。

Windows DIB 到 PNG 双阶段缓存验证：`artifacts/xui-document-rebuild/validation-2026-09-27-dib-cache-before.log` 中，`CF_DIBV5` 长度查询后把系统剪贴板保持打开，后续内容读取失败；改动后同一用例成功，说明第二次读取复用了首次合成的 PNG。另测先缓存一张 `CF_DIBV5`，再由 Win32 更换为 24 位 `CF_DIB`，读取须得到新像素；延迟 PNG 仍显示可用时，同序号的第二次读取也在剪贴板被占用期间成功。测试所有者轮询并派发 `WM_RENDERFORMAT`，避免外部剪贴板监听器请求延迟格式时阻塞本线程；[微软文档](https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats)说明 `CF_DIB` 可自动合成为 `CF_DIBV5`，因此断言最终像素而非强制某个格式未渲染。`validation-2026-09-27-dib-cache-pump-200x.log` 记录原生专项连续 200 次零失败，实际 Document 粘贴/复制专项包含于 `validation-2026-09-27-dib-cache-priority-final-suite.log`；该完整套件还覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制，退出码零。`validation-2026-09-27-dib-cache-priority-analyzer.log` 为零警告。未做实际耗时对照、外部应用矩阵或非 Windows 平台验收。

不透明扩展数据验证：`test_xui/xui_document_test.c` 使用含零字节、非 UTF-8 字节的 payload，核对 Snapshot 查询/复制、不可变旧快照、事务替换与 Undo/Redo、Save/Open、schemaVersion 2 原生 JSON、版本 1 读取、原生 fragment 与 XUI 标记 HTML fragment 往返。原生 Base64 损坏、HTML Base64 损坏、版本零/溢出、必需标志非法、缺失非空类型、单文档总量超限均返回错误且不提交；11 点分配失败扫描确认无半成品发布或分配残留。小额度导入的先失败证据 `artifacts/xui-document-rebuild/validation-2026-09-27-extension-payload-small-limit-before.log` 复现合法 6 字节 payload 被 JSON 字符串额度错误拒绝，修复后完整套件 `validation-2026-09-27-extension-payload-final-suite.log` 退出码零，覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制。`validation-2026-09-27-extension-payload-final-analyzer.log` 无 GCC `-fanalyzer` 警告；IDE 配置语法和 `git diff --check` 通过。本项验证的是扩展数据保留及内核边界，不等同于图片资产持久化、完整 HTML/CSS 或未知扩展的真实插件渲染。
HTML 内联 CSS marks 验证：新增 `html_style_mark_roundtrip` 将带段落级 `font-weight:700`/`font-style:italic`、同名颜色覆盖、嵌套 `normal` 清除、下划线与删除线组合、同一 style 的重复声明/`!important`、`<strong>`/`<em>` 标签默认字形覆盖的 HTML 导入为独立 fragment，并通过公开 Document API 逐节点检查文本、颜色和 marks；完整 HTML 导出再导入后重复同样断言。先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-27-html-css-marks-before.log`，最终 `validation-2026-09-27-html-css-inline-final-suite.log` 退出码零，覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；`validation-2026-09-27-html-css-inline-final-analyzer.log` 无警告，`git diff --check` 通过。实现只解释受支持的内联属性并把数值字重近似映射到二值 marks，不声称符合完整浏览器 CSS 级联；声明顺序和重要性的依据分别见 [W3C CSS Cascade 5](https://www.w3.org/TR/css-cascade-5/)、[CSS Fonts 4](https://www.w3.org/TR/css-fonts-4/) 与 [CSS Text Decoration 4](https://www.w3.org/TR/css-text-decor-4/)。
HTML 容器内联样式验证：`html_container_style_roundtrip` 从 `<body>` 与嵌套 `<div>` 导入文字颜色、18px 字号、右对齐及粗体，在内层 `font-weight:normal` 清除粗体并改色；分别检查显式 `<p>`、直接文本形成的隐式段落和列表项内段落，再导出 HTML、重新导入并逐节点重复断言。独立核心日志为 `artifacts/xui-document-rebuild/validation-2026-09-27-html-container-core.log`；最终完整套件 `validation-2026-09-27-html-container-final-suite.log` 退出码零，覆盖实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制。严格 GCC `-fanalyzer` 日志 `validation-2026-09-27-html-container-analyzer.log` 为空，`git diff --check` 通过。本用例不代表完整 HTML/CSS 浏览器一致性或外部应用粘贴验收。
HTML CSS 无效声明与表格样式验证：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-27-html-css-validity-before.log` 复现后写的无效 `!important` 值令前面的有效段落样式丢失。`html_invalid_css_declarations` 经公开 fragment 和 snapshot API 核对颜色、背景色、字号、字体族、字重、斜体、对齐、段后间距及下划线仍取先前有效声明；扩展后的 `html_container_style_roundtrip` 检查 body 字体族及 `table → tr → td → p` 的颜色、22px 字号、居中、正常字重，并在 HTML 导出再导入后重复断言。独立核心日志 `validation-2026-09-27-html-css-validity-core.log`、最终套件 `validation-2026-09-27-html-css-validity-final-suite.log` 均退出码零；后者包含实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制。严格 GCC `-fanalyzer` 日志 `validation-2026-09-27-html-css-validity-analyzer.log` 为空，`git diff --check` 通过。CSS 无效声明处理遵循 [W3C CSS Cascade 5](https://www.w3.org/TR/css-cascade-5/) 与 [CSS Syntax 3](https://www.w3.org/TR/css-syntax-3/) 的规则；这里只验证导入器支持的值子集，完整 HTML/CSS 一致性仍待验收。
HTML 数值 CSS 样式验证：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-27-html-numeric-css-before.log` 复现 `#abc`、`rgba(...)`、12pt 字号和无单位零间距未进入 Document 属性。`html_numeric_css_roundtrip` 检查 `#abc`、`#1234`、逗号式 RGB/RGBA、空格及斜杠透明度、百分比通道、越界裁剪、旧式 RGB 混合单位无效后的前值回退、12pt→16 逻辑像素、显式零段后间距，并在 HTML 导出再导入后重复属性断言。独立 Core 日志 `validation-2026-09-27-html-numeric-css-core.log`、最终完整套件 `validation-2026-09-27-html-numeric-css-final-suite.log` 均退出码零；后者含实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制。严格 GCC `-fanalyzer` 日志 `validation-2026-09-27-html-numeric-css-analyzer.log` 为空，`git diff --check` 通过。数值语法和单位换算参照 [W3C CSS Color 4](https://www.w3.org/TR/css-color-4/) 与 [CSS Values 4](https://www.w3.org/TR/css-values-4/)；本测试不证明命名色、其他颜色空间、完全透明黑的显式文字色、完整浏览器 CSS 或外部应用粘贴兼容。
HTML 容器样式渲染/查询验证：`html_block_style_render` 从公开 HTML fragment 导入带 body 文字色、24px 字号、字体族与段落背景的内容，确认 TEXT 叶仍无局部样式，同时 `SnapshotQueryTextStyle` 的光标值取得有效继承颜色/字号/字体族、底色仍是独立块属性；Renderer 的字体回调收到 24px，代理文字色与块底色均正确。`paragraph_background_prefix` 用 100,000 字节单段落证明首屏和向下续排均绘制块底色，同时 shaping 仍少于整段一半；修复前分别在 `artifacts/xui-document-rebuild/validation-2026-09-27-html-block-render-before.log` 与 `validation-2026-09-27-html-prefix-background-before.log` 失败。真实 Editor 的 2×2 Alt 矩形选区将首格段落设为独立底色，测试 surface 记录各颜色最后绘制顺序，确认四次选区填充在段落底色之后、四格边框总共四次；旧 DLL 在 `validation-2026-09-27-table-border-before.log` 记录选择色 4、段落底色 1、边框 8。最终完整套件 `validation-2026-09-27-inherited-style-border-final-suite.log` 退出码零，覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制；GCC 静态分析 `validation-2026-09-27-inherited-style-border-analyzer.log` 和 `validation-2026-09-27-inherited-style-border-test-analyzer.log` 为空，差异检查通过。Windows 内部 provider 的真实 DocumentView/MessageList 公式、Mermaid、HTML 与截图专项分别通过 `validation-2026-09-27-inherited-style-border-web-provider.log` 和 `validation-2026-09-27-inherited-style-border-web-render.log`。

显式透明黑颜色验证：`html_explicit_zero_color_roundtrip` 用 body 非零颜色、段落 `#0000`/`#00000000`、内联 `rgba(0,0,0,0)` 和 `transparent` 检查显式零标记、祖先覆盖、样式查询及 HTML 导出再导入；`explicit_zero_style_commands` 验证非零颜色与显式零标记的 schema 冲突被拒绝，选区样式、混合状态、原生 JSON、Markdown `TEXT_STYLE` 损失与显式舍弃、清除格式、Undo 和恢复默认。`explicit_zero_color_render` 验证代理收到零文字色且显式透明底色抑制高亮主题回退；实际 XGE 代理的 alpha 零绘制路径会跳过透明文字。Editor 专项验证折叠光标设置显式透明前景/底色后，新输入文字保留两项标记并可撤销。修复前日志 `artifacts/xui-document-rebuild/validation-2026-09-27-explicit-zero-before.log` 在 HTML 往返断言失败；独立 Core、Renderer、Editor 最终日志分别为 `validation-2026-09-27-explicit-zero-core-final.log`、`validation-2026-09-27-explicit-zero-renderer.log`、`validation-2026-09-27-explicit-zero-editor.log`。完整套件 `validation-2026-09-27-explicit-zero-final-suite.log` 退出码零，覆盖实际 DLL、652 项 CommonMark、MessageList 与原生同步/异步绘制。Windows WebView2 基础控件、DocumentView/MessageList 的公式、Mermaid、HTML 内部静态渲染及离线测量/PNG 截图分别由 `validation-2026-09-27-explicit-zero-webview-build.log`、`validation-2026-09-27-explicit-zero-web-provider.log`、`validation-2026-09-27-explicit-zero-web-render.log` 验证，均退出码零。相关源码和测试的严格 C 编译及 GCC `-fanalyzer`、`git diff --check` 均通过。此批未验收完整浏览器 CSS、跨应用 HTML 粘贴、真实平台 IME 或辅助技术。

原生格式版本 3 历史批次验证：`explicit_zero_style_commands` 将带文字色与底色显式零标记的版本 3 JSON 分别改标为版本 2 和 1，`Deserialize` 均返回 FORMAT 且输出为空；恢复版本 3 后颜色查询与原生往返保真。`roundtrip` 对不含新标记的 Rich/Markdown 文件验证版本 1/2 仍可加载、未来版本 4 明确拒绝。`extension_payload_roundtrip` 验证版本 2 的二进制扩展 payload 仍可读取，新写出为版本 3。修复前独立核心失败日志为 `artifacts/xui-document-rebuild/validation-2026-09-27-schema-v3-before.log`；修复后 `validation-2026-09-27-schema-v3-core.log` 与含实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制的 `validation-2026-09-27-schema-v3-final-suite.log` 均退出码零。Windows 可选 WebView2 基础控件及 Document 内部公式、Mermaid、HTML 静态渲染分别由 `validation-2026-09-27-schema-v3-webview-build.log`、`validation-2026-09-27-schema-v3-web-provider.log`、`validation-2026-09-27-schema-v3-web-render.log` 验证，均通过。相关 C 源码及测试通过严格编译和 GCC `-fanalyzer`，`git diff --check` 通过。该验证只证明本次原生格式边界，不替代跨应用文件交换或旧程序二进制兼容验收。

HTML 命名色/HSL 验证：`html_named_hsl_color_roundtrip` 将大小写混用的 `RebeccaPurple`、`navy`、HSL/HSLA 的逗号与空格形式及 deg/grad/rad/turn 色相导入 Rich fragment，检查段落背景、各文本颜色、透明度裁剪、负饱和度裁剪和显式透明黑标记；`hsl(0,100,50)!important` 缺少旧式百分比时，不得覆盖前面的有效 `red`；`hsl(0x1p2 ...)` / `rgb(0x10,...)` 等 C 十六进制浮点写法及 `rgb(1.,...)` 尾随小数点无效，`hsl(1.2e2 ...)` 十进制指数有效；`hsl(15 200% 50%)` 先用 200% 饱和度换算，再将 RGB 裁剪为红色。导出 HTML 后重新导入并逐节点重复断言。修复前在段落命名色/HSL 断言处失败，见 `artifacts/xui-document-rebuild/validation-2026-09-27-html-hsl-before.log`；十六进制浮点与尾随小数点的先失败证据分别见 `validation-2026-09-27-html-css-number-before.log`、`validation-2026-09-27-html-css-bounds-before.log`；超范围 HSL 由修复后的往返测试断言。独立 Core `validation-2026-09-27-html-hsl-core-final.log` 和完整套件 `validation-2026-09-27-html-hsl-final-suite.log` 均通过，后者包含实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生同步/异步绘制。`validation-2026-09-27-html-hsl-analyzer.log` 无 GCC `-fanalyzer` 警告，`git diff --check` 通过。可选 Windows WebView2 控件与 Document 内部 KaTeX/Mermaid/HTML 静态渲染由 `validation-2026-09-27-html-hsl-webview-build.log`、`validation-2026-09-27-html-hsl-web-provider.log`、`validation-2026-09-27-html-hsl-web-render.log` 分别验证通过。此批只覆盖内联 CSS 颜色子集，不代表完整浏览器 CSS 或跨应用粘贴兼容；色相/HSL 换算依据 [W3C CSS Color 4](https://www.w3.org/TR/css-color-4/)，十进制数字边界依据 [W3C CSS Syntax 3](https://www.w3.org/TR/css-syntax-3/)。

HTML CSS 数字区域设置验证：本机 `LC_NUMERIC=German_Germany.1252` 时，`html_css_locale_numeric_roundtrip` 从 HTML fragment 导入带 `12.5px` 字号、`2.5px` 段距、`650.5` 字重、`hsl(0.5turn ...)` / `rgb(12.5 ...)` 颜色、`12.5px × 6.25px` 图片和两列 `24.5px` / 无单位 `17.5` 的富文本表格；对同属性后写的 `0x1p...` 或 `1.` 无效值验证前一有效值仍生效。HTML 导出须包含点号小数，再在相同区域设置下导入并重复节点与列宽断言，最后恢复原区域设置。修复前 `artifacts/xui-document-rebuild/validation-2026-09-27-html-locale-before.log` 于段落属性失败；修复后独立 Core `validation-2026-09-27-html-locale-core.log` 与完整套件 `validation-2026-09-27-html-locale-final-suite.log` 退出码零，后者覆盖实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制。GCC `-fanalyzer` 的 `validation-2026-09-27-html-locale-import-analyzer.log`、`validation-2026-09-27-html-locale-export-analyzer.log` 为空，`git diff --check` 通过。Windows 可选 WebView2 与 Document 内部公式、Mermaid、HTML 静态渲染分别由 `validation-2026-09-27-html-locale-webview-build.log`、`validation-2026-09-27-html-locale-web-provider.log`、`validation-2026-09-27-html-locale-web-render.log` 复核通过。本机验证不等于其他平台或完整 CSS 引擎验收。

富文本复杂文档往返验证：`rich_nested_table_document_roundtrip` 通过统一 Document 事务创建两层表格，含表头、粗体链接、带尺寸与标题的图片、行内公式、代码块、Mermaid、块级原始 HTML 和两层列宽。原生 JSON 反序列化，以及导出 HTML 再以 fragment 导入新的 Rich Document，均与来源整树逐节点比较种类、子节点次序、文本、属性、资源、信息和标题；另核对内外表列宽。独立 Core 日志为 `artifacts/xui-document-rebuild/validation-2026-09-27-rich-nested-core.log`，完整套件 `validation-2026-09-27-rich-nested-final-suite.log` 包含实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制，均退出码零；`git diff --check` 通过。此测试覆盖 XUI 自身生成的规范 HTML，不证明任意网页或外部编辑器 HTML 的无损导入。

统一 Document 工具栏验证：`document_toolbar_cases` 将真实 XUI Toolbar 与 DocumentEditor 放入同一窗口，通过鼠标按下/抬起触发选择回调，执行加粗并核对选区 marks 与勾选状态，再从工具栏撤销；只读状态禁用格式命令，恢复后重新启用。切换内建中文和运行时修改自定义语言包译文时，提示文字正确同步；替换译文而尚未同步时，Toolbar 保留旧文字的有效副本。重新绑定 Markdown Editor 后，对齐命令按 profile 禁用，SOURCE 模式禁用 VISUAL 格式命令。实际 DLL Editor 专项 `artifacts/xui-document-rebuild/validation-2026-09-27-editor-toolbar-owned-tooltip.log` 与独立 Toolbar 控件 `validation-2026-09-27-editor-toolbar-widget.log` 均通过。发布导出表测试从旧 WebView 高级入口要求更新为当前基础公开契约，并加入 Document 完整套件；初次旧检查失败见 `validation-2026-09-27-editor-toolbar-exports.log`，修正后 `validation-2026-09-27-editor-toolbar-exports-final.log` 通过。最终 `validation-2026-09-27-editor-toolbar-tooltip-final-suite.log` 覆盖独立 Core、实际 DLL、652 项 CommonMark、导出表、Renderer/Editor、MessageList 和原生同步/异步绘制，退出码零；新适配源文件与通用 Toolbar 的 GCC `-fanalyzer` 无警告，`git diff --check` 通过。查找窗口和上下文菜单仍未由此验证。
统一 Document 内建上下文菜单验证：`document_menu_cases` 通过真实 `XUI_EVENT_CONTEXT_MENU` 测试右键命中选区内部与末端时的保留/折叠、键盘菜单键、Undo/Redo 实际菜单提交、只读禁用、中文及自定义译文运行时替换；已打开菜单保留旧译文有效副本，下次打开更新。Rich 与 Markdown VISUAL/SOURCE 共用命令状态。`table_rectangle_copy_cases` 补测 Alt 拖动矩形表格选区后在所选 Cell 右键，Copy 仍可用且表格选区不丢失；Document 观察者在菜单 Undo 中销毁 Editor 时，菜单回调安全退出。实际 DLL 专项 `artifacts/xui-document-rebuild/validation-2026-09-27-editor-menu-final-editor.log`、新编辑器源码 GCC `-fanalyzer` 的 `validation-2026-09-27-editor-menu-table-analyzer.log`、导出表 `validation-2026-09-27-editor-menu-exports.log` 均通过。最终 `validation-2026-09-27-editor-menu-final-suite.log` 覆盖独立 Core、实际 DLL、652 项 CommonMark、导出表、Renderer/Editor、MessageList 与原生同步/异步绘制，退出码零；差异检查通过。内建查找窗口、格式菜单扩展与工具栏自动宿主同步仍待实现。

统一 Document 内建查找窗口验证：`document_find_ui_cases` 在实际 DLL 中打开 XUI Window，输入查询后核对虚拟结果表与两处 Rich 匹配，执行结果激活、“替换当前”与“全部替换”按钮并逐项核对文本和 Undo；只读禁用替换按钮与 API，`Ctrl+F` 可重开，宿主主动销毁窗口后可重建，Markdown SOURCE 替换及 LIVE 模式匹配继续使用同一 Document。窗口实际布局和绘制、中文语言切换也经测试。新增 `xuiDocumentViewActivateFindResult` 和 `xuiDocumentEditorReplaceCurrent` 支撑表格激活与原子替换。测试同时暴露观察者销毁 Editor 时控件状态队列可能保留已释放子控件；`xui_widget.c` 在最终释放前把它从队列摘除，原菜单观察者回归、替换期间观察者销毁 Editor 与新窗口销毁均通过。专项日志 `artifacts/xui-document-rebuild/validation-2026-09-27-find-ui-test.log`；完整套件与导出表见 `validation-2026-09-27-find-ui-final-suite.log`。当前查询仍为大小写敏感的 UTF-8 字面匹配，未包含正则、整词、选区限定和真实平台输入法矩阵。

跨父列表项脚注定义来源拼接验证：`markdown_cross_parent_range_merge` 增补六组成功样本，涵盖只有段落与内层列表两个语义子块的父项、首块围栏代码、LF/CRLF、多个脚注、链接定义先于脚注及多行脚注正文。每组都断言精确 Markdown 原文、独立重新加载后的行内语法和定义表、Undo/Redo；当时的单位置搬运对脚注后接链接定义保守返回 UNREPRESENTABLE，下一批已补全前置搬运；失败事务仍不修改源码、revision 或定义表。新增 409/405 点分配失败扫描，失败无半成品发布和泄漏。实际 DLL `document_cross_block_source_editor_cases` 检查 VISUAL 输入、SOURCE 切换与 Undo/Redo。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-28-parent-footnote-final-suite.log` 包括 652 项 CommonMark、Renderer、Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 和 652 项语料见 `validation-2026-09-28-parent-footnote-linux-core.log`；GCC `-fanalyzer` 日志 `validation-2026-09-28-parent-footnote-analyzer.log` 无警告，`git diff --check` 通过。


脚注后接链接定义前置搬运验证：新增的跨父列表项矩阵将同一来源间隙里的脚注/链接定义按原字节与原顺序移至外层列表之前，覆盖单个与多个定义、LF/CRLF、围栏代码首块、内层后续兄弟项及根部后续块。每例核对精确 Markdown、独立全文重载的行内语法/定义表和 Undo/Redo；实际 DLL Editor 检查 VISUAL 输入、SOURCE 切换及一次撤销重做。新增 527 点分配失败扫描保持原子性与无泄漏。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-28-footnote-link-prelude-final-suite.log`、Linux Core ASan/UBSan 和 652 项语料 `validation-2026-09-28-footnote-link-prelude-linux-core.log` 均退出码零；GCC `-fanalyzer` 日志 `validation-2026-09-28-footnote-link-prelude-analyzer.log` 无警告。

同批可选 WebView2 复测：`artifacts/xui-document-rebuild/validation-2026-09-28-footnote-link-prelude-webview-build.log` 验证 Windows 基础控件构建与交互；`validation-2026-09-28-footnote-link-prelude-web-provider.log` 验证 DocumentView/MessageList 内部公式、Mermaid、HTML 绘制及错误恢复；`validation-2026-09-28-footnote-link-prelude-web-render.log` 验证离线测量、HTML 沙箱和真实 PNG 截图。三项均退出码零。

同父块间脚注定义来源搬运验证：`markdown_cross_block_source_patch` 增补根级已引用/未引用、LF/CRLF、多行与重复脚注、链接/脚注混排，以及引用块和列表项容器用例；精确来源、独立完整重载的语法/定义表及 Undo/Redo 均一致。`document_cross_block_source_editor_cases` 在实际 DLL 中验证 VISUAL 输入、SOURCE 切换和单步撤销重做。先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-28-root-footnote-gap-before.log` 复现旧回退重写选区外实体；修复后 313 点分配失败扫描无发布半成品或泄漏。Windows 完整套件 `validation-2026-09-28-root-footnote-gap-final-suite.log`、Linux Core ASan/UBSan 和 652 项 CommonMark 语料 `validation-2026-09-28-root-footnote-gap-linux-core.log` 均通过；GCC `-fanalyzer` 日志 `validation-2026-09-28-root-footnote-gap-analyzer.log` 无警告。
SOURCE/LIVE 可打印 ASCII 内容依赖行高验证：规模测试以代理 `textShape` 让前置短行及 9000 字节长行末尾的 `~` 使行高增加，先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-28-ascii-height-before.log` 的实际 DLL 在长行后光标仍给出普通行高。启用 `bSourceLineHeightMayVary` 后，冷 Renderer 与完整布局、只去掉长行尾部 `~` 的对照文档分别比较光标 Y、命中源码偏移和改宽重建；SOURCE 与 LIVE 均通过 `validation-2026-09-28-ascii-height-scale-final.log`。Windows 全套件 `validation-2026-09-28-ascii-height-final-suite.log` 包含 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制，退出码零；Linux 无窗口 Renderer ASan/UBSan `validation-2026-09-28-ascii-height-linux-renderer.log` 退出码零；修改后的 Renderer GCC `-fanalyzer` 日志为空，差异检查通过。普通默认长行前缀规模测试继续通过；该显式策略在深处冷查询时会增加前置行整形成本，真实平台字体/IME 与全部布局矩阵仍需验收。
矩形单元格选区合并验证：`table_rectangle_merge_cases` 在实际 DLL Editor 通过 Alt+拖动选 2×2 Rich 表格；修复前 `validation-2026-09-28-rectangle-merge-before.log` 记录 `TABLE_MERGE_CELLS` 查询被禁用。修复后核对命令状态、跨四格合并、原四个文字节点存续、矩形高亮清除、单步 Undo 恢复 2×2 网格和原矩形选区、Redo 清除选区、反向拖动、已合并单 Cell 再合并拒绝，以及只读/GFM Markdown 的无修改拒绝。专项 `artifacts/xui-document-rebuild/validation-2026-09-28-rectangle-merge-undo-editor.log` 和 Windows 全套件 `validation-2026-09-28-rectangle-merge-undo-final-suite.log` 退出码零；后者含 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 与原生绘制。Linux View/Editor ASan/UBSan `validation-2026-09-28-rectangle-merge-undo-linux-view.log` 退出码零，只有未修改的 DatePicker 编译告警；修改后的 Editor GCC `-fanalyzer` 无警告，差异检查通过。该专项不证明真实平台指针、IME、读屏。
程序化表格矩形选区验证：实际 DLL `table_programmatic_selection_cases` 测试合并 Cell 跨度扩展、无效范围保持旧矩形/普通光标/revision、`NULL` 清除、只读选区、Rich 矩形合并与 Undo 恢复、GFM 合并不可表示和 SOURCE 模式拒绝；首子节点是嵌套表格时，选择外层矩形仍把普通光标放在外层 Cell 并启用外层合并命令。专项 `artifacts/xui-document-rebuild/validation-2026-09-28-table-selection-api-nested-editor.log` 和 Windows 完整套件 `validation-2026-09-28-table-selection-api-final-suite.log` 退出码零，后者覆盖 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 View 测试额外覆盖矩形绘制、无效请求和清除；ASan/UBSan `validation-2026-09-28-table-selection-api-linux-view.log` 退出码零，只有未修改的 DatePicker 可能未初始化编译告警。View GCC `-fanalyzer` 日志 `validation-2026-09-28-table-selection-api-view-analyzer.log` 为空，`git diff --check` 通过。真实平台键盘/辅助技术矩阵尚未验收。
表格 Cell 辅助技术验证：`semantic_accessibility_cases` 枚举 GFM Cell，断言行列、1×1 跨度、SELECTABLE 与 SET_SELECTION；调用首格动作后核对统一矩形选区、仅首格 SELECTED、Document revision 未变，根文本选区清除矩形。Rich 2×3 表中合并 Cell 报告 1×2 跨度、已选状态，按节点动作选其完整跨度；SOURCE 拒绝 Cell 动作，只读 View 仍可选择。旧实现的实际 DLL 先失败见 `artifacts/xui-document-rebuild/validation-2026-09-28-cell-accessibility-before.log`，专项 `validation-2026-09-28-cell-accessibility-scroll-editor-final.log` 与 Windows 完整套件 `validation-2026-09-28-cell-accessibility-scroll-final-suite.log` 均退出码零；后者含 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-28-cell-accessibility-scroll-linux-view.log` 退出码零（只有未修改 DatePicker 的可能未初始化编译告警），并验证禁用选区时 Cell 不广告动作且拒绝调用。adapter GCC `-fanalyzer` 日志 `validation-2026-09-28-cell-accessibility-scroll-analyzer.log` 为空，`git diff --check` 通过。专项还先把首格移出可见区，验证 Cell 滚动动作以单元格边界滚动到视口并保留矩形选区。此项不证明真实读屏平台桥接或完整文本区间。
Alt+Shift+方向键表格选区验证：实际 DLL `table_keyboard_selection_cases` 从 3×3 Rich 表中心启动，逐次右/下扩展、左/上收缩并跨过起点反向选择；公开 API 从右下角接续收缩，反向 Alt 拖动后继续收缩，只读仍可选，普通 Shift+右键仍选文字。键盘矩形执行 Rich 合并、单步 Undo 后再次缩小，证实选区方向与表格历史一并恢复；Rich 合并 Cell 在边界首次选完整跨度，随后跨到右侧及下一行；GFM Markdown 只选择、不启用不可表示的合并命令，SOURCE 不建立矩形。Document revision 在纯选择期间不变。实际 DLL 专项 `artifacts/xui-document-rebuild/validation-2026-09-28-keyboard-table-editor.log`、完整 Windows 套件 `validation-2026-09-28-keyboard-table-final-suite.log` 均退出码零，后者含 Core、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-28-keyboard-table-linux-view.log` 覆盖只读 View 的扩展/收缩，退出码零，仅有未修改 DatePicker 的可能未初始化编译告警。View/Editor GCC `-fanalyzer` 日志 `validation-2026-09-28-keyboard-table-analyzer.log` 为空，`git diff --check` 通过。该专项不等于真实键盘设备或读屏平台验收。

多块容器辅助技术验证：`accessible_structured_container_cases` 在实际 DLL Editor 构造一个首格含两段的 Rich 2×2 表，核对 Cell 的 `a\nb`、Row 的 `a\nb\tc` 与 Table 的完整行列文本；局部选择分别跨段落换行、行内制表符、单元格和表格行映射到 Document 文字端点，反向选择保留方向，落入汉字 UTF-8 内部的偏移返回错误。Cell `SET_SELECTION(NULL)` 仍产生矩形选区；携带 payload 的动作清除矩形并选文字；只读 View 也能选择。GFM 嵌套任务列表同时核对 ListItem/List 连续文本、内部文字范围和任务切换动作，纯选择不改 revision/Markdown 源码。先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-28-structured-a11y-before.log`；实际 DLL 专项 `validation-2026-09-28-structured-a11y-editor.log`、含 652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制的完整 Windows 套件 `validation-2026-09-28-structured-a11y-final-suite.log` 均退出码零。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-28-structured-a11y-linux-view.log` 退出码零，仅有未修改 DatePicker 的可能未初始化编译告警；adapter GCC `-fanalyzer` 日志 `validation-2026-09-28-structured-a11y-analyzer.log` 为空，`git diff --check` 通过。仍需真实辅助技术桥接/读屏验收和大文档查询性能测试。

VISUAL 根节点全文可访问值验证：旧实现对 Document 根节点提供 `SET_SELECTION` 动作却无 `sValue`，实际 DLL 的先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-28-root-a11y-before.log` 在根值断言处停止。现在根节点显式查询返回与 Edit 语义文本投影相同的全文，包括块分隔换行，`iTextStart/iTextEnd` 与动作共享 UTF-8 字节坐标；Rich 多段表格的根局部范围跨段落映射到原始 Text，非空范围标记 SELECTED。实际 DLL 专项 `validation-2026-09-28-root-a11y-editor.log` 与最终 Windows 全套件 `validation-2026-09-28-root-a11y-final-suite.log` 退出码零；后者覆盖 Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-28-root-a11y-linux-view-final.log` 新增根值与范围断言，退出码零，只有未修改 DatePicker 的可能未初始化编译告警；adapter GCC `-fanalyzer` 日志 `validation-2026-09-28-root-a11y-analyzer.log` 为空，`git diff --check` 通过。全文投影在首次显式查询时生成，大文档辅助技术延迟、真实平台桥接及读屏验收仍待完成。
表格单元格样式与对齐验证：`html_table_cell_style_roundtrip` 经公开 HTML fragment 导入、导出和原生 JSON 往返，检查 Cell 自身保存继承/显式前景色、`background-color:currentColor`、透明背景以及居中/右对齐；子段落显式 `text-align:left` 覆盖右对齐 Cell。原生 schema 当前写出版本 5，降为版本 4 时显式左对齐标记必须被拒绝。先失败证据为 `artifacts/xui-document-rebuild/validation-2026-09-29-cell-css-before.log`。实际 DLL Renderer 的 `html_table_cell_style_render` 核对背景在 Cell 当前文字色上解析、透明背景压制默认底色；`table_cell_alignment_render` 核对 Rich Cell 继承、显式左覆盖、恢复继承以及 GFM 居中/右对齐的光标几何。Windows 完整套件 `validation-2026-09-29-cell-align-final-suite.log` 退出码零，含 652 项 CommonMark、Core、实际 DLL、Renderer/Editor、MessageList 与原生绘制。Linux Core 与 Renderer ASan/UBSan 日志 `validation-2026-09-29-cell-align-linux-core.log`、`validation-2026-09-29-cell-align-linux-renderer-final.log` 退出码零；后者新增 GFM 列对齐几何断言。修改源码的 GCC `-fanalyzer` 日志 `validation-2026-09-29-cell-align-analyzer.log` 为空，`git diff --check` 通过。尚未证明完整浏览器 CSS、其他平台真实字体及系统读屏。
表格继承对齐的编辑器状态验证：`cell_alignment_command_state_cases` 在实际 DLL 的富文本 Editor 中创建右对齐 Cell 与未显式设置对齐的子段落，先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-29-cell-align-editor-before.log` 复现 Renderer 显示右对齐但对齐命令状态误报左对齐。现在 schema 的 `doc_effective_alignment` 同时供 Renderer 布局与 Editor 命令状态查询使用；点击左对齐设置显式覆盖，Undo 后查询重新显示继承的右对齐。专项 `validation-2026-09-29-cell-align-editor-after.log`、Windows 完整套件 `validation-2026-09-29-cell-align-ui-final-suite.log` 均退出码零；后者包含 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 View/Editor 与 Renderer 的 ASan/UBSan 日志 `validation-2026-09-29-cell-align-ui-linux-view.log`、`validation-2026-09-29-cell-align-ui-linux-renderer.log` 退出码零；View/Editor 构建仅有未修改 DatePicker 的可能未初始化警告。修改源码的 GCC `-fanalyzer` 日志 `validation-2026-09-29-cell-align-ui-analyzer.log` 为空，`git diff --check` 通过。

富文本粗斜体交错 Markdown 写回验证：`rich_markdown_adjacent_mark_transitions` 对 3、4、5、6 段普通、粗体、斜体、粗斜体 run 的 6,080 种空格/连续文字排列，在 CommonMark、GFM、EXTENDED 下执行 18,240 次转换分析与真实转换，要求无语义损失且重解析等价。六段的“组合格式→粗体→组合格式→斜体→粗体”先失败用例见 `artifacts/xui-document-rebuild/validation-2026-09-29-inline-mark-six-run-before.log`，修正了此前仅凭前一原始节点 marks 选择定界符、忽略组合段实际输出字符的问题。最终 Windows 完整套件 `validation-2026-09-29-inline-mark-six-run-final-suite.log` 退出码零，包含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 及原生同步/异步绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-inline-mark-six-run-final-linux.log` 退出码零。由于此前一次源码修改与 DLL 构建重叠，先强制重建 DLL 并单独通过 `validation-2026-09-29-inline-mark-six-run-forced-dll-test.log`，随后再运行最终套件。修改源码以 GCC `-fanalyzer -Wall -Wextra -Werror` 检查无警告，`git diff --check` 通过。本矩阵针对粗斜体交错的字母与空格，不证明任意 Unicode/标点边界、其他 marks、链接或无限长度组合完全可表达；语义不等的候选仍原子拒绝。

Markdown 强调 Unicode 标点边界验证：`rich_markdown_adjacent_mark_transitions` 的 6,976 组排列在 CommonMark、GFM、EXTENDED 下逐组调用 Analyze 与 Convert，累计 20,928 组语义往返检查。先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-29-inline-punctuation-before.log` 记录 ASCII 标点及转义字符紧邻粗斜体时的可表达内容误拒绝；解析探针亦复现 `**a。**bc` 不能关闭粗体，而在后继普通字符使用实体后可正确关闭。写回器现在按同一 MD4C 解析器的 Unicode P/S 表识别标点，编码相邻普通文字的单个边界标量；CJK 普通文字及既有六段连续格式矩阵保持通过。Windows 完整套件 `validation-2026-09-29-inline-punctuation-final-suite.log` 退出码零，含独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-inline-punctuation-final-linux.log` 退出码零。写回器经 GCC `-fanalyzer -Wall -Wextra -Werror`，MD4C 的 ASCII 构建经严格语法检查，差异检查通过。该矩阵验证所列标点/Unicode 边界，不替代完整 Markdown 来源 CST、复杂链接混排或无限长度验收。

Markdown VISUAL Tab 来源与 Unicode 空白边界验证：`markdown_cross_block_source_patch` 新增两项精确端点的跨段落替换：LF 段首 `\tJOIN` 与 CRLF 段尾 `JOIN\t`，要求保存为 `&#9;`、选区外 `&amp;` 及引用定义原字节不变、完整重载来源语法相同，并可 Undo/Redo。先失败的 `artifacts/xui-document-rebuild/validation-2026-09-29-markdown-tab-boundary-before2.log` 实际回退为 `\&` 且增加空行；修复后局部来源补丁直接通过。`rich_markdown_adjacent_mark_transitions` 加入四种 U+00A0 边界，原先每种在多个粗斜体排列下失败，见 `validation-2026-09-29-inline-nbsp-before.log`；现与 MD4C 共用 Unicode 空白/标点分类，7,232 组排列在三种方言下合计 21,696 组分析与实际转换通过。Windows 完整套件 `validation-2026-09-29-tab-nbsp-final-suite.log` 退出码零，覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 和同一语料 `validation-2026-09-29-tab-nbsp-final-linux.log` 退出码零。写回器 GCC `-fanalyzer -Wall -Wextra -Werror`、MD4C ASCII 变体严格编译和 `git diff --check` 通过。该批证明列出的精确来源补丁与强调边界，不证明完整 Markdown CST、任意跨结构编辑或所有 Unicode 空白组合。

链接图片统一语义验证：linked_image_roundtrip 检查 Markdown 引用式外链图片的两个独立资源/标题、未修改源码原字节、局部图片更新保留外层引用、链接目标改写、Undo/Redo、原生 schemaVersion 6 与版本 5 降格拒绝、Rich→Markdown 转换、HTML fragment 往返，以及 Rich/Markdown 通用 SetAttributes 解除链接后清除字段。富文本链接图片插入和 Markdown 链接图片更新的分配失败扫描分别为 27/166 点，失败时树、源码和历史不发布且无泄漏。实际 DLL 的 View 图片点击回传外链目标，Editor 插入/更新保留链接，MessageList 图片指针点击、无障碍激活及描述均指向外链。Core、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制的 Windows 完整套件 artifacts/xui-document-rebuild/validation-2026-09-29-linked-image-suite.log 退出码零；Linux Core ASan/UBSan 与同一语料 validation-2026-09-29-linked-image-linux-core.log、Linux 无窗口 View/Editor Sanitizer validation-2026-09-29-linked-image-linux-view.log 均退出码零。后者仅有未修改 DatePicker 的可能未初始化编译告警。GCC -fanalyzer -Wall -Wextra -Werror 日志 validation-2026-09-29-linked-image-analyzer.log 为空；公开头语法、Document API 注释覆盖 147/147 和差异空白检查通过。该批没有完成图片资产打包、全部 Markdown 来源拼写保真或真实平台读屏验收。

辅助 blob 内存计费验证：`memory_auxiliary_blob_accounting` 先使链接图片更新后 `OtherBytes` 异常增长，失败日志为 `artifacts/xui-document-rebuild/validation-2026-09-29-aux-blob-before.log`。修复节点可达性和 prepare 完整/增量发布遍历后，Rich 普通事务及 Markdown prepared source 发布都将外链目标/标题计入当前根；保留旧图片 snapshot、修改目标并清空历史后，snapshot 独占字节包含旧链接数据，释放时 live 字节恰好减少该数额。Core `validation-2026-09-29-aux-blob-snapshot-core.log`、实际 DLL `validation-2026-09-29-aux-blob-snapshot-dll.log`、Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-29-aux-blob-snapshot-linux-core.log` 退出码零。修改源码后的完整 Windows 套件 `validation-2026-09-29-aux-blob-full-suite.log` 及可选 WebView2/Document 内部渲染专项通过；源码 GCC `-fanalyzer -Wall -Wextra -Werror` 与差异检查通过。扩展 payload 同步进入 prepare 的两条可达性路径；该修正不代表共享属性池或全部存储目标完成。

链接图片历史计费补测：无 snapshot 时再次更新图片外链目标和标题，清空历史后 `LiveBytes` 的实际下降量等于清空前 `HistoryBytes`，证明旧链接 blob 纳入历史预算。最终独立 Core `artifacts/xui-document-rebuild/validation-2026-09-29-aux-blob-history-core.log`、实际 DLL `validation-2026-09-29-aux-blob-history-dll.log`、Linux Core ASan/UBSan 及 652 项 CommonMark `validation-2026-09-29-aux-blob-history-linux-core.log` 均退出码零；自前述完整 Windows 套件后仅添加该测试断言，没有再次修改功能源码。

共享属性池与快照计费验证：`shared_attribute_pool` 构造两份结构和文字相同的 Rich 文档，96 个 Text 节点分别使用同一属性或 96 种字号；共享版本的 `CurrentBytes` 少 18,240 字节。单节点修改属性后，另一个节点与旧快照仍保持原字号，释放所有句柄后自定义分配器无未回收字节；既有历史预算、快照诊断、分配失败、并发与 Markdown prepare 回归继续通过。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-attr-pool-final-suite.log` 退出码零，覆盖独立 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 和原生同步/异步绘制。Linux Core ASan/UBSan 与语料 `validation-2026-09-29-attr-pool-linux-core.log`、Linux 无窗口 View/Editor sanitizer `validation-2026-09-29-attr-pool-linux-view.log` 均退出码零；后者只有未修改 DatePicker 的可能未初始化编译告警。可选 WebView2 控件 `validation-2026-09-29-attr-pool-webview-build.log`、Document 内部 provider `validation-2026-09-29-attr-pool-web-provider.log`、离屏截图 `validation-2026-09-29-attr-pool-web-render.log` 均通过。关键源码的 GCC `-fanalyzer -Wall -Wextra -Werror` 日志为空，差异检查通过。固定桶属性池的百万级不同样式性能、完整 Markdown CST 与其余跨平台验收仍待完成。

高基数属性索引验证：原 256 桶线性链在 2.5 万、5 万、10 万、20 万种不同属性下的 Windows O2 创建耗时分别为 0.024、0.084、0.362、2.951 秒，见 `artifacts/xui-document-rebuild/validation-2026-09-29-attr-pool-scale-before.log`。桶内改为按 64 位哈希排序的 AVL 树后，同档为 0.011、0.026、0.065、0.141 秒；探针逐桶核对排序、实际树高、左右高度差和对象总数，5 万属性交错删除/重插/复用/打乱释放后仍无遗留分配，见 `validation-2026-09-29-attr-pool-avl-final-invariant.log`。Linux ASan/UBSan 对同一高基数及交错场景通过 `validation-2026-09-29-attr-pool-avl-linux-final-invariant.log`。含高基数探针的 Windows 完整套件 `validation-2026-09-29-attr-pool-avl-final-suite.log` 退出码零，覆盖 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制；Linux Core/语料和无窗口 View/Editor sanitizer 分别见 `validation-2026-09-29-attr-pool-avl-linux-core.log`、`validation-2026-09-29-attr-pool-avl-linux-view.log`，均通过。可选 WebView2 控件及 Document 内部公式/Mermaid/HTML 渲染见 `validation-2026-09-29-attr-pool-avl-webview-build.log`、`validation-2026-09-29-attr-pool-avl-web-provider.log`、`validation-2026-09-29-attr-pool-avl-web-render.log`，均退出码零。`src/xui_document_store.c` 的 GCC `-fanalyzer -Wall -Wextra -Werror` 和差异检查通过。该批证据只证明属性池内部 20 万种属性与相应回归；上文后续批次补了百万级属性和人工碰撞链查找/删除，仍未覆盖恶意构造的真实哈希碰撞或完整 Document 级跨平台性能矩阵。

Markdown 块标记来源验证：`markdown_block_syntax_markers` 查询解析器确认的 ATX 起始/闭合井号、Setext 下划线、嵌套 Quote/List 中围栏的起始/闭合范围，以及普通和引用定义后显现的分隔线；原始源码片段必须与返回字节范围一致。未闭合围栏只返回首标记，缩进代码不误认反引号；源码前插后当前与旧快照、Undo/Redo、语义修改标题级别后的投影仍正确。最初闭合 ATX 标记测试揭示正文尾部空白再修剪后的边界偏差，修正后 Core 通过。最终 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-block-syntax-compact-final-suite.log` 包含 Core、652 项 CommonMark、实际 DLL 与新增导出表、Renderer/Editor、MessageList 和原生绘制，退出码零；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-block-syntax-compact-linux-core.log` 退出码零。可选 WebView2 DLL/控件、Document 内部 provider/MessageList 与离屏 KaTeX/Mermaid/HTML 截图分别见 `validation-2026-09-29-block-syntax-webview-build.log`、`validation-2026-09-29-block-syntax-web-provider.log`、`validation-2026-09-29-block-syntax-web-render.log`，均通过。公开 API 注释覆盖 149/149、头文件检查、未打补丁 MD4C 严格编译及修改文件的 GCC `-fanalyzer -Wall -Wextra -Werror` 均通过。该来源接口尚未覆盖列表、引用、表格和信息串的完整块内 CST。

Markdown 容器来源标记验证：markdown_container_syntax_markers 检查 CRLF 引用内连续兄弟列表项的原始符号、GFM/EXTENDED 任务框、CommonMark 非任务框、有序九位编号、列表内嵌套引用、脚注任务项和 admonition 首个 >；围栏正文中的同形字符不能误报。源码前插后旧/新快照及 Undo 保持各自来源范围。Core 专项 artifacts/xui-document-rebuild/validation-2026-09-29-container-marker-core-footnote.log、最终 Windows 套件 validation-2026-09-29-container-marker-final-suite.log 均退出码零；后者含实际 DLL、Renderer/Editor、MessageList、原生绘制和 652 项 CommonMark。Linux Core ASan/UBSan 与语料 validation-2026-09-29-container-marker-linux-core.log 退出码零。可选 WebView2 控件 validation-2026-09-29-container-marker-webview-build.log、Document 内部 provider/MessageList validation-2026-09-29-container-marker-web-provider.log、离屏 KaTeX/Mermaid/HTML 截图 validation-2026-09-29-container-marker-web-render.log 均退出码零。未打补丁 MD4C 严格编译、修改源文件 GCC -fanalyzer -Wall -Wextra -Werror、公开头文件检查/API 注释覆盖 149/149、注释 lint 与差异检查通过。此批只保证语义节点首个容器标记，不是完整的容器 trivia/CST；引用续行前缀仍未归属到节点。

Markdown 围栏同行尾部来源验证：xuiDocumentSnapshotGetBlockSyntax 对已确认的代码/Mermaid 围栏新增 iFenceTailStart/End，直接返回从开围栏末端到原始行内容末端的范围，保留空格/Tab；空尾部是相等的两个有效坐标，非围栏节点仍返回 UINT64_MAX。markdown_block_syntax_markers 与 markdown_resources_and_trivia 检查嵌套 CRLF 围栏、源码前插后的旧/新快照、Undo、空尾部、仅有开围栏的 EOF、含 Tab/尾随空格的信息、Mermaid 及语义修改语言名后的投影，原字节逐段核对。新增 Mermaid 用例后的 Core 见 artifacts/xui-document-rebuild/validation-2026-09-29-fence-tail-diagram-core.log，退出码零。功能源码最终版的 Windows 完整套件 validation-2026-09-29-fence-tail-final-suite.log 退出码零，覆盖独立 Core、实际 DLL、Renderer/Editor、MessageList 与原生绘制；此默认调用跳过独立语料脚本，因此另以 validation-2026-09-29-fence-tail-win-corpus.log 和 validation-2026-09-29-fence-tail-linux-corpus.log 验证 Windows/Linux 各 652 项 CommonMark 解析/来源/原生往返和增量差分，均退出码零。Linux Core ASan/UBSan 见 validation-2026-09-29-fence-tail-linux-core.log，退出码零。可选 WebView2 控件、Document 内部 provider/MessageList 和离屏 KaTeX/Mermaid/HTML 分别见 validation-2026-09-29-fence-tail-webview-build.log、validation-2026-09-29-fence-tail-web-provider.log、validation-2026-09-29-fence-tail-web-render.log，均退出码零。未打补丁 MD4C 严格编译、修改文件 GCC -fanalyzer -Wall -Wextra -Werror、公开头检查、API 注释覆盖 149/149、注释 lint 与 git diff --check 通过。这里只证明开围栏的原始尾部字节边界，未把信息串内部子 token 或其他尚缺的块内 CST 宣称完成。

Markdown 引用逐行前缀来源验证：新 xuiDocumentSnapshotGetQuotePrefix 以 GetBlockSyntax.iQuotePrefixCount 为界，枚举每个被 MD4C 接受的显式 `>`；懒续行不计入。解析器的临时前缀链在 Quote/Admonition 节点开启后一次性传给 Document，多行节点只保存首标记之外的相对 32 位偏移；旧快照与增量来源位移独立。Core 测试逐字节核对嵌套引用、围栏内伪 `>`、脚注、admonition、CRLF、前插后的旧/新快照、Undo、语义改标题后的投影和 2 万行引用的最后一枚前缀；419 个 Document/解析器分配故障点逐项验证提交原子性与释放后无泄漏。专项 artifacts/xui-document-rebuild/validation-2026-09-29-quote-prefix-core-final2.log 退出码零。最终 Windows 完整套件 validation-2026-09-29-quote-prefix-final-suite.log 退出码零，覆盖 Core、652 项 CommonMark 解析/来源/原生往返与增量差分、实际 DLL、Renderer/Editor、MessageList 和原生绘制。Linux Core ASan/UBSan 与同一语料 validation-2026-09-29-quote-prefix-linux-core.log、无窗口 Renderer validation-2026-09-29-quote-prefix-linux-renderer.log、View/Editor validation-2026-09-29-quote-prefix-linux-view.log 均退出码零；View 编译仍有未修改 DatePicker 的可能未初始化警告。可选 WebView2 控件 validation-2026-09-29-quote-prefix-webview-build.log、Document 内部 provider/MessageList validation-2026-09-29-quote-prefix-web-provider.log、离屏 KaTeX/Mermaid/HTML validation-2026-09-29-quote-prefix-web-render.log 均退出码零。未打补丁 MD4C 严格编译、修改文件 GCC -fanalyzer -Wall -Wextra -Werror、公开头检查、API 注释覆盖 150/150、注释 lint 和差异检查通过。此批只证明引用节点的显式 `>` 归属；列表续行缩进、表格 token、围栏信息串内部词法及完整 CST 未完成。

Markdown 表格 token 来源验证：xuiDocumentSnapshotGetBlockSyntax 对已确认的 GFM/EXTENDED Table 给出分隔行内容范围及 iTableTokenCount，xuiDocumentSnapshotGetTableToken 枚举解析器确认的标题/正文分隔竖线与每列分隔线的横线/对齐冒号。Core 用例逐字节检查转义竖线和代码跨度内竖线不误报、CRLF、引用嵌套、CommonMark/围栏负例、源码前插后的旧/新快照与 Undo，以及语义修改后与独立全文重载的 token 差分；266 点分配失败扫描均保持原子与无泄漏。Windows 完整套件 artifacts/xui-document-rebuild/validation-2026-09-29-table-token-final-suite.log 退出码零，覆盖 Core、652 项 CommonMark 解析/来源/原生往返与增量差分、实际 DLL、Renderer/Editor、MessageList 和原生绘制。新增导出检查 validation-2026-09-29-table-token-export.log 退出码零。Linux Core ASan/UBSan 与同一语料 validation-2026-09-29-table-token-linux-core.log、无窗口 Renderer validation-2026-09-29-table-token-linux-renderer.log、View/Editor validation-2026-09-29-table-token-linux-view.log 均退出码零；View 编译仅有未修改 DatePicker 的可能未初始化警告。未打补丁 MD4C 严格编译、修改源码 GCC -fanalyzer -Wall -Wextra -Werror、API 注释覆盖 151/151、注释 lint 及 git diff --check 通过。该来源接口尚非完整 Markdown CST，未覆盖列表续行缩进、围栏信息串内部 token 和全部块内空白。

Markdown 列表续行缩进来源验证：`markdown_list_continuation_indents` 按源码逐字节核对兄弟项与缩进不足的懒续行、嵌套列表/引用、Tab、CRLF、空行、源码前插后旧/新快照、语义重写后的来源投影及 Undo；2 万行围栏正文中的续行记录保持可查询。`markdown_list_indent_allocation_failures` 扫描 434 个 Document/解析器分配故障点，未发布半成品且释放后无泄漏。最终 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-list-indent-final-suite.log` 退出码零，覆盖独立 Core、652 项 CommonMark 解析/来源/原生往返与增量差分、实际 DLL/新增导出、Renderer/Editor、MessageList 和原生绘制。Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-list-indent-linux-core.log`、无窗口 Renderer `validation-2026-09-29-list-indent-linux-renderer.log`、View/Editor `validation-2026-09-29-list-indent-linux-view.log` 均退出码零；View 编译仍有未修改 DatePicker 的可能未初始化警告。可选 Windows WebView2 控件 `validation-2026-09-29-list-indent-webview-build.log`、Document 内部 provider/MessageList `validation-2026-09-29-list-indent-web-provider.log`、离屏 KaTeX/Mermaid/HTML `validation-2026-09-29-list-indent-web-render.log` 均退出码零。未打补丁 MD4C 严格编译、相关源码 GCC `-fanalyzer -Wall -Wextra -Werror`、公开 API 注释覆盖 152/152、注释 lint 和差异检查通过。这里只证明解析器确认的显式列表续行水平空白归属，不包含起始项标记后的空白、Tab 的逐层逻辑列界或完整 Markdown CST。

Markdown 列表首行空白来源验证：`markdown_container_syntax_markers` 逐字节核对符号/九位编号与任务框后的原始空格/Tab、合法空范围、嵌套引用和脚注、CommonMark 与 GFM/EXTENDED 的任务框差异；源码只替换空白时新旧快照各保留自己的坐标，语义文字编辑和 Undo 后也正确。复用节点已有的尾部端点字段后，同一 Linux sanitizer 10 MiB、204801 节点样本的 Document 统计峰值从未合并版本 `validation-2026-09-29-list-gap-linux-core.log` 的 325243733 字节回到 `validation-2026-09-29-list-gap-packed-linux-core.log` 的 323605293 字节，与此前列表续行批次相同；这是内核计费值，不是进程 RSS。最终 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-list-gap-packed-final-suite.log` 退出码零，包含独立 Core、652 项 CommonMark 解析/来源/原生往返与增量差分、实际 DLL 与导出检查、Renderer/Editor、MessageList 和原生绘制。Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-list-gap-packed-linux-core.log`、无窗口 Renderer `validation-2026-09-29-list-gap-packed-linux-renderer.log`、View/Editor `validation-2026-09-29-list-gap-packed-linux-view.log` 均退出码零；View 编译仍有未修改 DatePicker 的可能未初始化警告。可选 Windows WebView2 控件 `validation-2026-09-29-list-gap-packed-webview-build.log`、Document 内部 provider/MessageList `validation-2026-09-29-list-gap-packed-web-provider.log`、离屏 KaTeX/Mermaid/HTML `validation-2026-09-29-list-gap-packed-web-render.log` 均退出码零。未打补丁 MD4C 严格编译、相关源码 GCC `-fanalyzer -Wall -Wextra -Werror`、公开头语法检查、API 注释覆盖 152/152、注释 lint 和差异检查通过。此范围只归属列表首行的原始水平空白；Tab 逐层逻辑列界、围栏信息串内部 token 和其他 Markdown trivia 尚未构成完整 CST。

Markdown 围栏信息词法来源验证：固定版本 MD4C 在 fenced-code detail 内确定有效信息串起止及首个语言 token 终点，Document 的 GetBlockSyntax 同时返回原始尾部、有效信息和语言的快照字节范围；Tab 不按首尾 ASCII 空格修剪，语言名后未解释的元数据保持原字节。Core 逐字节核对嵌套引用/列表、CRLF、空信息/未闭合围栏、Mermaid、首部 Tab 和尾部空格、9 KiB 信息串、源码改语言后的新旧快照与 Undo，以及语义改语言后的投影；含围栏的分配失败扫描为 435 点，失败保持发布原子性且无泄漏。最终 Windows 完整套件 artifacts/xui-document-rebuild/validation-2026-09-29-fence-info-final-suite.log 退出码零，覆盖独立 Core、652 项 CommonMark 解析/来源/原生往返与增量差分、实际 DLL 和导出检查、Renderer/Editor、MessageList 与原生绘制。Linux Core ASan/UBSan/同一语料 validation-2026-09-29-fence-info-linux-core.log、无窗口 Renderer validation-2026-09-29-fence-info-linux-renderer.log、View/Editor validation-2026-09-29-fence-info-linux-view.log 均退出码零；View 编译仍有未修改 DatePicker 的可能未初始化警告。可选 Windows WebView2 控件 validation-2026-09-29-fence-info-webview-build.log、Document 内部 provider/MessageList validation-2026-09-29-fence-info-web-provider.log、离屏 KaTeX/Mermaid/HTML validation-2026-09-29-fence-info-web-render.log 均退出码零。未打补丁 MD4C 严格编译、相关源码 GCC -fanalyzer -Wall -Wextra -Werror、公开头语法检查、API 注释覆盖 152/152、注释 lint 与 git diff --check 通过。10 MiB、204801 节点样本的 Document 统计峰值保持 323605293 字节。此处不声明解析语言后的任意元数据语法、Tab 逐层列界或完整 Markdown CST。

Editor 删除命令边界验证：`deletion_command_boundary_cases` 在实际 DLL 上覆盖 Markdown SOURCE/LIVE/VISUAL、Rich VISUAL 的文档首尾、空文档、正反向选区；`QueryCommand`、`CanExecute` 与程序化 `Execute` 在不可删除的边界一致报告禁用，非空选区仍可删除。修复前 `validation-2026-09-29-delete-command-before.log` 证明起点 Backspace 被错误启用，`validation-2026-09-29-delete-execute-before.log` 证明不可执行命令仍返回成功。最终 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-delete-command-final-suite.log` 退出码零，含 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 与原生绘制；Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-29-delete-command-linux-view.log` 退出码零，仅有未修改 DatePicker 的可能未初始化编译告警。Editor GCC `-fanalyzer -Wall -Wextra -Werror` 见 `validation-2026-09-29-delete-command-analyzer.log`，无诊断。此批只验收删除命令状态边界，不代表 E3 其余命令状态、Bidi 导航或真实 IME/读屏已完成。

同批空行内节点边界补测：Rich 段落中实际保留前后空 Text 节点、光标位于有字 Text 首/尾时，旧结构相邻判断错误地启用 Backspace/Delete；先失败见 `artifacts/xui-document-rebuild/validation-2026-09-29-delete-empty-inline-before.log`。现在命令状态按 Editor 的行内投影长度跳过零字节节点，实际 DLL Editor 专项 `validation-2026-09-29-delete-command-editor-empty-final.log` 通过。最终完整套件和 Linux sanitizer 以本批重跑后的同名日志为准。

Markdown 换行来源验证：`markdown_break_syntax` 核对软换行、行尾两个空格及反斜杠硬换行、尾随空白和 LF/CR/CRLF 的精确 SourceStore 范围；覆盖引用块、跨 4 KiB 块边界、源码与语义视觉编辑后的旧/新快照及 Undo/Redo。`markdown_break_syntax_allocation_failures` 注入 161 个分配失败点，失败发布保持原子且释放后无泄漏。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-break-source-final-suite.log` 退出码零，含独立 Core、652 项 CommonMark 解析/来源/原生往返、实际 DLL 与新增导出、Renderer/Editor、MessageList 和原生绘制。新增的增量准备/完整解析树对照逐节点比较换行元数据，Windows `validation-2026-09-29-break-source-corpus-differential.log` 与 Linux Core ASan/UBSan 加同一 652 项语料 `validation-2026-09-29-break-source-linux-core-final.log` 均退出码零。修改源码 GCC `-fanalyzer -Wall -Wextra -Werror`、未打补丁 MD4C 严格编译、API 注释覆盖和 lint 153/153 以及差异检查通过。此项来源元数据只对应语义 SoftBreak/HardBreak，图片 alt 文本没有对应节点；完整块内 CST 与其他长期验收仍未完成。

Markdown VISUAL 跨段落多行替换验证：`markdown_cross_block_source_patch` 对精确 Text 端点的 LF、CRLF、多行首字 Tab 和选区中隐藏链接定义逐字节核对来源；合并后的快照与独立全文重载语法一致，Undo/Redo 往返。补充单独 CR 输入和来源的 Core 用例，以及 `markdown_cross_parent_range_merge` 的多层 Quote 两侧精确来源样本。实际 DLL 的 `document_cross_block_source_editor_cases` 经 VISUAL 输入 `JOIN\nMORE`、Undo/Redo 及 SOURCE 模式切换核对原字节。修复前日志 `artifacts/xui-document-rebuild/validation-2026-09-29-multiline-range-before.log` 显示结构写回将未选中的 `&amp;` 改为 `\&` 且增加空行；局部补丁由候选全文解析及语义比对把关。隐藏定义用例的分配失败扫描经过 292 点，失败时 revision 和来源保持原状，释放后无泄漏。Windows 完整套件 `validation-2026-09-29-multiline-range-final-suite.log` 退出码零，覆盖 Core、实际 DLL、652 项 CommonMark、Renderer/Editor、MessageList 与原生绘制；新增 Editor 用例的独立重跑 `validation-2026-09-29-multiline-range-editor-final.log`、追加 CR 用例的 Core 重跑 `validation-2026-09-29-multiline-range-cr-core.log` 均退出码零；Linux Core ASan/UBSan 与 652 项语料 `validation-2026-09-29-multiline-range-linux-core.log` 退出码零。Markdown 写回源码 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 和差异检查通过。此证据限于候选解析证明可表示的文字跨段落补丁，不覆盖所有容器内多行写回。

Markdown VISUAL 引用块多行补丁验证：根层补丁之后，`artifacts/xui-document-rebuild/validation-2026-09-29-quote-multiline-before.log` 的 Quote 用例仍显示未选中的 `&amp;` 被结构写回改为 `\&`，并多出一个空 `> ` 行。修复从左端 Paragraph 的原始行首提取与 Quote 祖先层数相符的纯 `>`/水平空白前缀，分别生成空行与新正文行；候选全文解析和语义核对仍为接纳条件。Core 验证同一 Quote、跨 Quote/根层及两层 Quote 的精确源码、独立重载语法、Undo/Redo；两组分配失败扫描各覆盖 292、240 点，失败时 revision/源码不变且释放后无泄漏。实际 DLL Editor 的 Quote 多行输入及 SOURCE 切换已纳入 `document_cross_block_source_editor_cases`。最终 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-29-quote-multiline-final-suite.log` 退出码零，覆盖 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-29-quote-multiline-linux-core.log` 退出码零。改动源码 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 与差异检查通过。列表、混合标记前缀及其他非文字复杂结构不在本批证据范围。

Markdown VISUAL 列表项多行补丁验证：`artifacts/xui-document-rebuild/validation-2026-09-30-list-multiline-before.log` 复现原路径把选区外 `&amp;` 改为 `\&`，并在列表内生成额外空白行。修复按最近 ListItem 的实际标记、任务框及 Tab 逻辑列宽生成新段落续行前缀；Quote 后缀单独保留，私有候选必须经全文解析与目标语义核对。Core 对普通/有序/嵌套/任务列表、列表内 Quote、跨父容器、后续段落左端和 Tab 间隙核对精确源码、独立重载语法、Undo/Redo。四组分配失败扫描覆盖 292、240、273、248 点，失败时 revision 和来源不变，释放后无泄漏。实际 DLL `document_cross_block_source_editor_cases` 验证任务列表 VISUAL 多行输入、Undo/Redo 与 SOURCE 模式往返。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-list-multiline-final-suite.log` 退出码零，含 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-30-list-multiline-linux-core.log` 退出码零。源码 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 与差异检查通过。Quote 内 ListItem 的不同标记顺序和其他复杂容器写回尚未覆盖。

Markdown VISUAL Quote 包住 ListItem 多行补丁验证：`artifacts/xui-document-rebuild/validation-2026-09-30-quote-list-multiline-before.log` 复现 `> -` 选区替换回退结构写回，产生额外空引用行并把 `&amp;` 改为 `\&`。修复保留列表项标记之前的 Quote 前缀，按 Tab 逻辑列宽替换列表标记，并让同父块间隙识别祖先 Quote 的显式 `>`；局部候选必须通过全文解析、目标语义及引用定义拼写核对。Core 验证 `> -`、有序/任务列表、两层 Quote、跨父容器及 `> - >` 顺序的精确源码、独立重载语法和 Undo/Redo。六组分配失败扫描分别覆盖 292、240、273、248、273、284 点，失败时 revision/来源不变且释放无泄漏。实际 DLL Editor 额外验证任务列表外层 Quote 的 VISUAL 输入、SOURCE 切换与 Undo/Redo。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-list-multiline-final-suite.log` 退出码零，包含 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-30-quote-list-multiline-linux-core.log` 退出码零。源码 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 和差异检查通过。带隐藏定义间隙及其他复杂容器仍需单独验收。

Markdown VISUAL 容器隐藏定义与脚注多行补丁验证：同父 ListItem、Quote/ListItem、跨父 Quote 尾部带隐藏链接定义的样本逐字核对保留的定义及选区外实体；Footnote 普通、CRLF 和 `> [^n]:` 定义样本逐字核对四空格续行、Quote 空行前缀、独立全文重载语法与 Undo/Redo。脚注内缩进 `[r]:` 经 GetSourceInfo 确认不计为隐藏引用定义，并经语义查找确认是可见段落，跨越选区后删除正确。八组故障扫描覆盖 292、240、273、248、273、284、295、306 个分配失败点，失败时 revision/来源不发布半成品，释放无泄漏。实际 DLL `document_cross_block_source_editor_cases` 验证普通/Quote 脚注 VISUAL 多行输入、SOURCE 模式和 Undo/Redo。`artifacts/xui-document-rebuild/validation-2026-09-30-footnote-multiline-oom-core.log`、Windows 完整套件 `validation-2026-09-30-footnote-multiline-final-suite.log`、Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-30-footnote-multiline-linux-core.log` 均退出码零；完整套件含实际 DLL、Renderer/Editor、MessageList 和原生绘制。`src/xui_document_markdown_edit.c` 的 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 与本批文件差异检查通过。此证据不覆盖任意 Markdown CST、全部容器前缀或非文字复杂结构。

脚注缩进 Quote 补充验证：新增 `  > [^n]:` 选区多行替换用例，修复前 `artifacts/xui-document-rebuild/validation-2026-09-30-footnote-indented-quote-before.log` 返回 -107；修复后 `validation-2026-09-30-footnote-indented-quote-after.log` 通过。`validation-2026-09-30-footnote-indented-quote-oom-core.log` 的第九组分配失败扫描覆盖 306 点，故障下仍不发布半成品且无泄漏。实际 DLL Editor 对两空格缩进 Quote 脚注验证精确源码、Undo/Redo 和 SOURCE 模式。最终 Windows 完整套件 `validation-2026-09-30-footnote-indented-quote-final-suite.log` 退出码零，含 Core、652 项 CommonMark、实际 DLL、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-30-footnote-indented-quote-linux-core.log` 退出码零。源码 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 和差异检查通过。

Document 发布 API 门禁验证：`test_xui/check_document_release.ps1` 对当前 `xui_document.h` 与 `xui_document_ui.h` 的 282 个公开函数逐一查询真实 DLL 导出表，检查活动源码/头/示例/测试/构建清单和 DLL 无旧 RichDocument/RichEdit 符号；`build/xge.lib` 作为错误输入时导出表解析为零并以非零状态拒绝，见 `artifacts/xui-document-rebuild/validation-2026-09-30-release-api-audit-negative.log`。门禁已在 `test_xui/build_document_suite.bat` 内实际运行，Windows 完整套件 `validation-2026-09-30-release-api-audit-suite.log` 退出码零，记录 652 项 CommonMark、282/282 导出、Core、实际 DLL、Renderer/Editor、MessageList 与原生绘制通过。修改文件 `git diff --check` 通过；此项只验证发布表面与构建迁移，不替代旧行为专项、完整平台矩阵或功能验收。

旧 RichEdit 查找替换格式行为迁移验证：统一 Document 的单段落 `Alpha alpha` 样本先仅给第一处加粗，`ReplaceAllEx` 不区分大小写改成两处 `ONE` 后，QueryMarks 分别返回 BOLD 与无标记，Undo 恢复原文；Windows Core `artifacts/xui-document-rebuild/validation-2026-09-30-rich-replace-style-core.log` 退出码零。实际 DLL Editor 的 FindEx/ReplaceCurrentEx、格式与 Undo 专项 `validation-2026-09-30-rich-replace-style-editor.log` 退出码零。Linux Core ASan/UBSan 与 652 项 CommonMark `validation-2026-09-30-rich-replace-style-linux-core.log` 退出码零。此专项覆盖旧行为中的格式继承一项，不证明全部旧富文本测试迁移完成。

同父混合块 Quote 命令验证：新增 Core Rich/Markdown 完整块 gap 选区用例，保留原节点 ID、选区方向、Markdown 选区外 `&amp;` 原始字节，并比对独立重载后的语法树和 Undo/Redo；新增 160 点失败注入，失败不改变 revision/来源/树/历史且无泄漏。实际 DLL Editor 对 GFM 的段落＋规则线＋列表验证 QueryCommand 启用、Execute 和 Undo/Redo。独立 Windows Core 日志 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-structural-core-final.log`、Editor 日志 `validation-2026-09-30-quote-structural-editor-first.log`、含 652 项 CommonMark 与全部 UI/原生绘制检查的完整套件 `validation-2026-09-30-quote-structural-final-suite.log` 均退出码零。Linux Core ASan/UBSan 加 652 项语料 `validation-2026-09-30-quote-structural-linux-core.log` 与无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-structural-linux-view.log` 退出码零；修改源码严格 GCC 静态分析和差异检查通过。此批针对同父完整块范围，跨父结构范围、任意 Markdown 来源无损写回和真实输入仍未完成。

Markdown Quote 块可表示性验证：Core 对代码块与表格整体包裹检查节点类型、选区外实体原字节、独立重载语法和 Undo/Redo；Front Matter/脚注定义整体包裹明确返回不可表示且来源原子不变。实际 DLL Editor 的命令状态及 Execute 对后二者给出相同原因；Front Matter 修复前误报可用见 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-frontmatter-editor-before.log`。Windows Core `validation-2026-09-30-quote-representability-core.log`、Editor `validation-2026-09-30-quote-representability-editor-second.log`、含 652 项 CommonMark 与完整 UI/原生绘制检查的套件 `validation-2026-09-30-quote-representability-final-suite.log` 均退出码零。Linux Core ASan/UBSan 加语料 `validation-2026-09-30-quote-representability-linux-core.log` 及无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-representability-linux-view.log` 退出码零；严格 GCC 静态分析与差异检查通过。Markdown 的复杂 HTML/表格源形式仍由提交适配器核对，不能以此证明所有引用命令状态都已精确预测。

跨父完整容器 Quote 命令验证：Core 用例以 Markdown Quote 内文字到相邻列表项的文字选区包裹旧 Quote＋List，断言节点 ID、选区外实体原字节、独立重载语法及 Undo/Redo；富文本用例覆盖反向文字选区、跨父反向 GAP 选区及新 Quote 内正确的返回选区方向。部分覆盖旧 Quote 的范围明确拒绝且来源不变。实际 DLL Editor 验证完整范围的命令可用、执行与 Undo/Redo，以及部分范围禁用。先失败日志为 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-cross-parent-before.log` 和 `validation-2026-09-30-quote-cross-gap-before.log`；211 点分配失败扫描证实失败原子性与无泄漏。Windows Core `validation-2026-09-30-quote-cross-gap-core.log`、Editor `validation-2026-09-30-quote-cross-parent-editor-first.log`、含 652 项 CommonMark 和全部 UI/原生绘制检查的完整套件 `validation-2026-09-30-quote-cross-parent-final-suite.log` 均退出码零。Linux Core ASan/UBSan 加语料 `validation-2026-09-30-quote-cross-parent-linux-core.log` 与无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-cross-parent-linux-view.log` 退出码零；严格 GCC 静态分析和差异检查通过。部分覆盖容器的拆分仍需实现。
跨父部分 Quote 拆分验证：Core 覆盖起点、终点、双端不同 Quote 的部分选区，检查原子块 NodeId、未选两侧 `&amp;` 原字节、独立全文重载语义及 Undo/Redo；富文本额外覆盖反向文字、反向 GAP 和 Quote 末端 GAP 边界选区。216、218、420 点故障扫描在失败时保持 revision/来源/树/历史原子且无泄漏。实际 DLL Editor 验证部分 Quote 状态可用、执行与历史恢复，以及部分 List 继续禁用。Windows Core `artifacts/xui-document-rebuild/validation-2026-09-30-quote-partial-rich-core.log`、边界增补 Core `validation-2026-09-30-quote-partial-boundary-core.log`、Editor `validation-2026-09-30-quote-partial-editor-first.log` 和含 652 项 CommonMark、DLL 导出、Renderer/Editor、MessageList、原生绘制的完整套件 `validation-2026-09-30-quote-partial-final-suite.log` 均退出码零。Linux Core ASan/UBSan 加同一语料 `validation-2026-09-30-quote-partial-linux-core.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-partial-linux-view.log` 均退出码零；相关 C 文件严格 GCC 静态分析和差异检查通过。该批只覆盖共同父容器下不同 Quote 的边缘拆分；部分 List、多层容器以及其他未选 Markdown trivia 仍需后续实现。

Quote 跨列表边界验证：Core 的无序前段、有序尾段、双列表两端和同一有序列表中间 GAP 用例检查子项 NodeId、列表起始编号、两侧 `&amp;` 原字节、独立全文重载语义及 Undo/Redo；富文本反向文字选区沿用同一拆分内核。实际 DLL Editor 对列表边缘和中段完整项启用 Quote 命令，执行和历史往返通过，列表项内部部分子块保持禁用。239、240、455、345 点分配故障扫描保持 revision/来源/树/历史原子并无泄漏。Windows Core `artifacts/xui-document-rebuild/validation-2026-09-30-quote-list-final-core.log`、Editor `validation-2026-09-30-quote-list-middle-editor.log`、完整套件 `validation-2026-09-30-quote-list-final-suite.log` 均退出码零；完整套件覆盖 652 项 CommonMark、DLL 发布导出、Renderer/Editor、MessageList 与原生绘制。Linux Core ASan/UBSan 加同一语料 `validation-2026-09-30-quote-list-linux-core.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-list-linux-view.log` 均退出码零，相关源码 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 与差异检查通过。多层容器、列表项内部部分子块、非规则列表标记和完整 Markdown CST 尚无此批验收。

Markdown loose 列表引用拆分验证：起点、终点、同列表中段三个样本确认拆分后的单项简单列表从 loose 变为 tight，结构与独立全文重解析一致，未选 `&amp;` 及空行来源保留，Undo/Redo 恢复两种状态。实际 DLL Editor 验证 loose 列表的起点和有序中段状态、执行与历史。244、242、350 个分配故障点下 revision/来源/树/历史保持原子且无泄漏。Windows Core `artifacts/xui-document-rebuild/validation-2026-09-30-quote-loose-oom-core-fixed.log`、Editor `validation-2026-09-30-quote-loose-editor.log`、含 652 项 CommonMark、282 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制的完整套件 `validation-2026-09-30-quote-loose-final-suite.log` 均退出码零。Linux Core ASan/UBSan 加同一语料 `validation-2026-09-30-quote-loose-linux-core.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-loose-linux-view.log` 均退出码零；严格 GCC 静态分析和差异检查通过。该批只处理拆成单项简单列表的 loose 归一化；多项 loose 列表内空行、复杂项及其他 CST trivia 未覆盖。

Markdown 多项 loose 列表 Quote 拆分验证：结构层按保留的简单 ListItem 之间是否仍有原始空行更新 tight 属性；起点、终点、中段、CRLF 和内部空行对照均与独立全文重解析一致，ListItem NodeId、选区外 `&amp;` 原字节及 Undo/Redo 保持正确。Core 新增 310、312、514 点分配失败扫描，失败不发布半成品且无泄漏。实际 DLL Editor 的命令状态、执行和历史通过。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-loose-multi-final-suite.log` 退出码零，包含 652 项 CommonMark、282 项发布 API 导出、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 加同一语料 `validation-2026-09-30-quote-loose-multi-linux-core.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-loose-multi-linux-view.log` 退出码零。严格 GCC 静态分析与差异检查通过。复杂多块列表项与完整 CST 不在本批验收范围。

根层 Quote 内列表边缘与中段引用拆分验证：选区可从 List 尾项延至同 Quote 段落并保留尾部未选段落，可从同 Quote 中部段落延至 List 首项并保留前段，也可在一张 List 中选择中间完整项。精确写回保留外层 Quote 未选前后 `&amp;` 等原始字节，结果与独立全文重解析语义一致，Undo/Redo 恢复原源码。新增 335、400、376、547 点分配失败扫描，失败不发布半成品且无泄漏；实际 DLL Editor 的状态、执行与历史通过。Windows 最终完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-nested-list-final-suite-verified.log` 退出码零，含 652 项 CommonMark、282 项公开导出、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 与同一语料 `validation-2026-09-30-quote-nested-list-linux-core-final.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-nested-list-linux-view.log` 退出码零。严格 GCC 静态分析与差异检查通过。更深层容器同时拆分和复杂来源 trivia 尚未覆盖。

根列表项内 Quote 拆分验证：Core 精确比对七种输入/输出来源：LF、CRLF、混合换行、`7.` 有序与任务列表标记、同 Quote 未选尾段、父列表未选兄弟项；每例独立全文重载与编辑后的语法树一致，Undo/Redo 还原原始字节。无显式 `>` 的 lazy continuation 被拒绝，revision、来源和历史不变。457 个分配失败点保持原子且无泄漏。实际 DLL Editor 的命令状态、执行和历史往返通过。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-list-item-nested-final-suite-expanded.log` 退出码零，含两轮 Core、282 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 加 652 项 CommonMark `validation-2026-09-30-quote-list-item-nested-linux-core-expanded.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-list-item-nested-linux-view.log` 退出码零。`src/xui_document_commands.c` 与 `src/xui_document_markdown_edit.c` 的 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 和差异检查通过。此证据不覆盖任意嵌套深度、隐藏定义间隙或完整 Markdown CST。
根 List 多层列表项内 Quote 与隐藏定义验证：修复前日志 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-deep-list-before.log` 返回不支持，`validation-2026-09-30-quote-list-hidden-ref-before.log` 返回不可表示。修复后 Core 的来源矩阵逐字核对多层列表、祖先/兄弟实体、LINK 定义在选区前/内/后的原文及引用前缀，全文重载语法与目标树相同，Undo/Redo 往返；无显式 `>` 的 lazy continuation 失败且不改 revision、源码和历史。665、345、471 点分配失败扫描分别覆盖深层列表、选区内定义和两者组合，失败不发布半成品且无泄漏。实际 DLL Editor 的深层与定义样本 QueryCommand、Execute、Undo/Redo 通过。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-deep-reference-final-suite-with-corpus.log` 退出码零，明确执行 652 项 CommonMark、15,470 个 UTF-8 光标边界、282 项 DLL 导出、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 加 652 项语料 `validation-2026-09-30-quote-deep-reference-linux-core.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-deep-reference-linux-view.log` 退出码零。`src/xui_document_commands.c`、`src/xui_document_markdown_edit.c` 严格 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 与差异检查通过。此验证不证明非根 List 祖先、无显式 Quote 标记的局部写回或完整 Markdown CST。
根 Quote/List 统一祖先路径验证：`artifacts/xui-document-rebuild/validation-2026-09-30-quote-root-ancestor-before.log` 记录根 Quote→List→Item→Quote 的选区原先失败。Core 现对一层/两层外 Quote、前后未选根块、混合换行和隐藏 LINK 定义在选区前/内的样本逐字核对源码，独立全文重载语法与编辑结果一致，Undo/Redo 恢复旧字节；无显式 Quote 标记的 lazy continuation 保持不可表示且不改变已发布状态。新增 1,165 与 419 点分配失败扫描证实根 Quote 邻居与选区内定义路径在失败时无半提交、无泄漏。实际 DLL Editor 的 QueryCommand、Execute 和历史往返通过。Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-ancestor-general-final-suite.log` 明确传入语料并以零码退出，包含 652 项 CommonMark、15,470 个 UTF-8 光标边界、282 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 加语料 `validation-2026-09-30-quote-ancestor-general-linux-core.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-quote-ancestor-general-linux-view.log` 均以零码退出。`src/xui_document_commands.c`、`src/xui_document_markdown_edit.c` 的 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 和修改文件差异检查通过。该证据只覆盖根 Quote/List 单块、唯一变化的 Quote/List/ListItem 祖先路径和显式 Quote 行；非根 Footnote/Table 祖先、同时多容器拆分及完整 Markdown CST 未由本批证明。

列表项内部完整子块 Quote 来源验证：失败日志 `artifacts/xui-document-rebuild/validation-2026-09-30-quote-list-item-body-before.log` 显示通用 List 写回会改写未选 `&amp;` 并增空行。局部路径用解析器记录的 ListItem 续行缩进插入 `>`，其余源码保留；Core 的 11 个单块样本、2 个跨块样本与 640 行样本覆盖普通/有序/任务/嵌套/外层 Quote、混合换行、选区外 LINK 定义、无末尾换行、独立全文重载与 Undo/Redo。216、283、214 点分配故障扫描保持 revision/来源/树/历史原子且无泄漏；实际 DLL Editor 验证 QueryCommand、Execute、Undo/Redo。冻结源码 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-list-item-noeof-final-suite.log` 以零码结束，记录 652 项 CommonMark、282 项公开导出、Renderer/Editor、MessageList 和原生绘制；Linux Core ASan/UBSan 加语料 `validation-2026-09-30-list-item-noeof-linux-core-asan.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-list-item-noeof-linux-view-asan.log` 均以零码结束。相关源码严格 GCC 静态分析与差异检查通过。本批只验证同一 ListItem 的连续完整子块，未覆盖跨项内部选区、块内部分内容或完整 Markdown CST。

列表项 Quote 选区内定义及 lazy continuation 验证：新增双段落间隐藏 LINK 定义用例，逐字核对源码与独立全文重载语义，并检查定义标签、目标、标题来源 token。修复前 `artifacts/xui-document-rebuild/validation-2026-09-30-list-item-lazy-before.log` 在已解析的列表项 lazy 续行上返回不可表示；修复后普通列表、外层 Quote、有序列表及混合换行的三组精确来源和 Undo/Redo 通过。328、267、277 点分配失败扫描保持 revision/来源/树/历史原子且无泄漏；实际 DLL Editor 对选区内定义与 lazy 段落的 QueryCommand、Execute、Undo/Redo 通过。冻结源码 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-list-item-lazy-final-suite.log` 退出码零，包含 652 项 CommonMark、282 项公开导出、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 加同一语料 `validation-2026-09-30-list-item-lazy-linux-core-asan.log`、无窗口 View/Editor ASan/UBSan `validation-2026-09-30-list-item-lazy-linux-view-asan.log` 均退出码零。Markdown 写回源码严格 GCC 静态分析、测试严格 C 编译及差异检查通过。此证据不覆盖无法取得同项已确认续行前缀的 lazy 行、跨列表项内部选区和完整 Markdown CST。

列表项首块 lazy Quote 来源验证：先失败日志 artifacts/xui-document-rebuild/validation-2026-09-30-list-item-first-lazy-before.log 记录普通列表首段无法包裹。局部补丁使用解析器记录的标记位置与 Tab 展开的列宽推导续行前缀；普通、有序、外层 Quote、Tab、无文件末尾换行等样本逐字核对源码、独立重载语法与 Undo/Redo。任务列表首段因复选框标记依附开头段落而不可表示；Core 断言错误原子性，实际 DLL Editor 断言命令禁用及执行错误，任务列表后续子块仍保持可用。首块路径 215 个分配失败点无半成品发布或泄漏。Windows Core artifacts/xui-document-rebuild/validation-2026-09-30-list-item-first-lazy-oom-core.log、Editor validation-2026-09-30-list-item-first-lazy-editor.log 与完整套件 validation-2026-09-30-list-item-first-lazy-final-suite.log 均退出码零；完整套件包含 652 项 CommonMark、282 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制。Linux Core ASan/UBSan 加语料 validation-2026-09-30-list-item-first-lazy-linux-core.log、View/Editor ASan/UBSan validation-2026-09-30-list-item-first-lazy-linux-view.log 均退出码零。修改源码与测试严格 GCC -fanalyzer、差异检查通过。此证据不覆盖跨列表项内部选区、任意 Markdown 容器组合或完整 CST。

DocumentEditor 任务框交互验证（2026-09-30）：失败先例 `artifacts/xui-document-rebuild/validation-2026-09-30-editor-task-marker-before.log` 复现点击未改变任务状态；`validation-2026-09-30-editor-task-marker-final-editor.log` 随后的新边界用例复现外部释放捕获后仍误切换。最终 Editor 专项 `validation-2026-09-30-editor-task-marker-final-editor2.log` 验证 Markdown GFM 与 Rich 的指针点击、移出/捕获丢失取消、非复选框点击、空任务项、命令状态与执行、直接 API、辅助技术动作共用 Undo、只读及 SOURCE/LIVE 禁用、只读/模式切换和程序调用打断按下，退出码零。`validation-2026-09-30-editor-task-marker-full-final.log` 为最终源码的 Windows 全量套件，退出码零，包含 652 项 CommonMark 解析/来源往返、283 项公开 DLL 导出、Renderer/Editor、MessageList 和原生绘制。`validation-2026-09-30-editor-task-marker-linux-view-final2.log` 为 Linux 无窗口 View/Editor ASan/UBSan，执行任务框点击、命令、Undo，退出码零；严格 GCC `-Wall -Wextra -Werror -fanalyzer` 通过。该批不构成真实窗口跨平台输入、IME 或读屏桥接验收。

DocumentEditor 纵向目标列验证（2026-09-30）：失败先例 `artifacts/xui-document-rebuild/validation-2026-09-30-editor-vertical-goal-before.log` 在 SOURCE 长—短—长行的第二次向下移动丢失原列；VISUAL 分段用例进一步确认固定行高步长会落入段落间距并停在原行。最终 Windows Editor 专项 `validation-2026-09-30-editor-vertical-goal-editor5.log` 和完整套件 `validation-2026-09-30-editor-vertical-goal-full-final.log` 均退出码零：SOURCE 与 Markdown VISUAL 跨短行回到目标列，Shift 选区保留目标列、Home 重置；Rich VISUAL 的 80 逻辑单位段后距和反向导航也通过。完整套件包括 652 项 CommonMark 语料、283 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制。Linux 无窗口 View/Editor ASan/UBSan `validation-2026-09-30-editor-vertical-goal-linux-view.log` 执行 SOURCE 跨短行导航并退出码零；严格 GCC `-Wall -Wextra -Werror -fanalyzer` 和差异检查通过。此证据不覆盖完整 Bidi 视觉顺序、混合字形高度及真实窗口 IME/读屏。

GFM 表格空单元格来源编辑验证（2026-09-30）：先失败日志 `artifacts/xui-document-rebuild/validation-2026-09-30-table-edit-preservation-probe.log` 记录语义 GAP 输入后旧结构写回把未编辑的 `-` 对齐分隔线改成 `---`，并重排原有空格。`src/xui_document_markdown_edit.c` 现仅对解析器提供零宽来源锚点的显式空 Cell 尝试局部插入：输入按 Markdown 转义，私有候选重解析并核对目标语义和引用定义原始拼写后才发布；其余情况沿原路径处理。Core 覆盖带空格/无空格 Cell、LF/CRLF、输入竖线及原表中已有转义竖线与代码跨度、Undo/Redo；216 个分配失败点验证 revision、来源、树和历史保持原子且释放后无泄漏。实际 DLL VISUAL Editor 从现有空 Cell 输入 `new|cell` 并 Undo/Redo 的专项 `validation-2026-09-30-table-edit-preservation-editor-probe.log` 退出码零。最终 Windows 完整套件 `validation-2026-09-30-table-edit-preservation-final-suite.log` 退出码零，包含 Core、652 项 CommonMark 解析/来源/原生往返、283 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制；Linux Core ASan/UBSan 加同一 652 项语料 `validation-2026-09-30-table-edit-preservation-linux-core.log` 退出码零。修改源码与测试的 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 通过。短行补齐的合成 Cell 没有精确锚点，完整表格 CST 和任意结构编辑来源保真仍未由此证明。

GFM 表格短行合成 Cell 来源编辑验证（2026-09-30）：修复前 `artifacts/xui-document-rebuild/validation-2026-09-30-table-synthetic-cell-before.log` 复现短行目标单元格输入引起未编辑的 `- | - | -` 对齐行被结构写回为 `---` 并重排空白。最终实现仅对解析器确认的末尾合成 Cell 尝试行内扩展；用表格 PIPE 语法 token 区分真实行尾竖线与转义/代码竖线，输入转义后由私有全文解析、语义与引用定义拼写校验决定是否发布。Core 的 6 组短行场景与 8192 个尾随空格长尾场景核对精确源码、新 Cell 来源范围、语义查询和 Undo/Redo；显式/合成 Cell 的分配失败扫描分别覆盖 216/338 点，失败时 revision、源码、树和历史不变，释放后无泄漏。最终 Windows 完整套件 `validation-2026-09-30-table-synthetic-cell-final2-suite.log` 退出码零，包含 Core、652 项 CommonMark、283 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制；实际 DLL Editor 用例报告 exact/omitted cell source 与 Undo/Redo 通过。Linux Core ASan/UBSan 加 652 项语料 `validation-2026-09-30-table-synthetic-cell-final2-linux-core.log` 退出码零；修改源码及测试的 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only` 和差异检查通过。未覆盖完整表格 CST、任意结构变更的无损来源写回和真实跨平台窗口输入。

Windows WebView2 连续生命周期验证（2026-09-30）：`test_xui/xui_webview_widget_test.c` 在真实 XGE/XUI 窗口完成原有焦点、裁剪、根控件切换、关闭与初始化中销毁后，串行执行 12 次 WebView 创建、异步初始化、HTML 加载、关闭和控件销毁。每轮要求一次导航完成与一次关闭事件，关闭后原生 Static 宿主 HWND 无效，下一轮开始前也没有孤留宿主。正式产品源码未因该专项改变；启用 WebView2 的 DLL 构建与最终实窗测试 `artifacts/xui-document-rebuild/validation-2026-09-30-webview-repeated-lifecycle-final.log` 均退出码零，测试使用 GCC `-std=c11 -O2 -Wall -Wextra -Werror` 编译。单窗口串行通过不证明更大规模/跨窗口并发、进程失败恢复、真实 IME、物理 DPI 或读屏行为。

Windows WebView2 双实例隔离验证（2026-09-30）：实窗专项同时创建并初始化两个 WebView，分别完成一次 HTML 导航，核对 READY 状态和两个原生宿主。关闭并销毁第一实例后，仅第二实例的宿主仍在正确矩形，第二实例可继续加载新 HTML 并再次收到导航完成事件；关闭第二实例后无孤留宿主。新增阶段初次误用了原焦点阶段编号，失败日志 `validation-2026-09-30-webview-concurrent-lifecycle-probe.log` 与 `-probe2.log` 属于测试阶段冲突，改用独立编号后的 `-probe3.log` 及最终 `artifacts/xui-document-rebuild/validation-2026-09-30-webview-concurrent-lifecycle-final.log` 均退出码零。测试严格 GCC 编译与差异检查通过；未覆盖更大规模或跨窗口并发及浏览器进程失败恢复。

Windows WebView2 进程退出与正式导出边界验证（2026-09-30）：ProcessFailed 只把浏览器主进程和主框架渲染进程退出转换成控件 FAILED，取消内部待处理请求；释放 WebView2 controller、原生 HWND 和向宿主发送一次 FAILED 延后到下一次 XUI 更新，避免在 COM 事件回调中重入。实窗测试 test_xui/xui_webview_widget_test.c 在前述连续 12 轮及双实例阶段之后创建独立临时 profile，获取 WebView2 报告的浏览器 PID，以进程映像名和宿主 PID 排除误杀，再终止该测试专属进程；断言 FAILED/E_FAIL、恰好一次失败事件、Static 宿主已消失，并用相同 profile 新建控件完成 HTML 导航。最终构建与实窗结果 artifacts/xui-document-rebuild/validation-2026-09-30-webview-process-failure-final-widget.log 退出码零。现有真实窗口 Document provider 套件 validation-2026-09-30-webview-process-failure-final-provider.log 退出码零，覆盖公式、Mermaid、HTML、MessageList 及既有 worker/对象错误路径；它尚未向 provider worker 注入真实进程退出。正式 DLL 重建及运行时导出查询 validation-2026-09-30-webview-process-failure-final-exports.log 退出码零，14 个内部通道/测试专用符号均未导出。最新 WebView2 开启/关闭的产品源码和实窗测试源码经 GCC -std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only，差异检查通过。主框架渲染进程故障及实际 Document provider 失败卡片、运行时缺失、自动重试均未由此证明。

Document provider 的真实浏览器进程退出验证（2026-09-30）：新增的 sUserDataFolder 让 provider worker 使用与其他 WebView 分离的临时 profile，配置由 xuiWebViewCreate 复制。test_xui/xui_document_web_provider_test.c 在既有 KaTeX/Mermaid/HTML、MessageList、资源缺失、超时、截图解码/裁剪恢复之后，用不回应渲染请求的本地页面使一条公式保持排队。异步 worker 达到 READY 后，通过测试专用内部接口取得浏览器 PID，以进程映像名和宿主 PID 排除错误目标，再终止该测试专属进程。provider 随下一次 XUI 更新收到 FAILED，统计显示 bWorkerFailed=1、iQueued=0、iFailed>=1；MessageList 的绘制回调核对错误标题和原因，build/webview/xui_document_web_process_failure.png 的实际像素检查也通过。释放 provider 后用同一 profile 重建，iReady/iDrawn 和公式绘制恢复。首轮 validation-2026-09-30-document-provider-process-integration-provider-probe.log 在 worker 尚未 READY 时请求 PID，属于测试时序失败；修正后 validation-2026-09-30-document-provider-process-integration-provider-probe2.log 退出码零。可选 WebView2 DLL/widget 构建与实窗回归 validation-2026-09-30-document-provider-process-integration-widget-build.log、离线高级对象渲染 -render.log、正式 DLL 导出边界 -exports.log 均退出码零，正式 DLL 不导出 14 个内部/测试专用符号。完整 Windows Document 套件 validation-2026-09-30-document-provider-process-integration-full-suite.log 退出码零，覆盖 652 项 CommonMark、283 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制。此证据覆盖浏览器主进程退出，不覆盖主框架渲染进程、GPU 故障矩阵、运行时缺失或自动重试。


DocumentEditor 表格上下文菜单验证（2026-09-30）：test_xui/xui_document_editor_test.c 在 Rich 2×2 矩形选区检查 18 项菜单、合并启用、拆分禁用、翻译文本、菜单回调执行和 Undo 后 TSV 矩阵原值；在 Markdown GFM 表格核对插入行启用及合并/拆分禁用，在普通正文核对无表格命令。首次全量运行 `artifacts/xui-document-rebuild/validation-2026-09-30-table-context-menu-full-suite.log` 暴露测试事件仍指向已销毁 Editor；修正 pTarget 后，Editor 专项 `validation-2026-09-30-table-context-menu-editor-final2.log` 与完整套件 `validation-2026-09-30-table-context-menu-full-suite-final.log` 均退出码零。完整套件覆盖 652 项 CommonMark、283 项公开 DLL 导出、Renderer/Editor、MessageList 与原生绘制；修改源码及测试的 GCC `-std=c11 -Wall -Wextra -Werror -fanalyzer -fsyntax-only`、差异检查通过。此批未对真实窗口表格长菜单的滚动/键盘操作或跨平台 IME/读屏做专项验收。

Linux 无窗口 View/Editor ASan/UBSan `artifacts/xui-document-rebuild/validation-2026-09-30-table-context-menu-linux-view.log` 退出码零，验证共享源码可构建并执行既有 Rich/Markdown View/Editor 输入、任务框、纵向导航、Undo、命中与绘制；该用例未专门操作新增表格菜单。编译器仍报告未修改的 DatePicker 可能未初始化警告，脚本按现有规则允许此告警。

Document provider 内部 worker 自动恢复验证（2026-09-30）：`src/xui_document_web_provider.c` 只对 WebView FAILED/CLOSED 事件按 1/2/4 秒有界重建，失败卡仍可显示；恢复页面声明 KaTeX/Mermaid/DOMPurify 已就绪后，仅重试因 worker 故障失败的缓存项。真实窗口 `test_xui/xui_document_web_provider_test.c` 用独立 profile、映像名与宿主 PID 检查后终止专属浏览器；在同一个 provider 上断言错误卡实际绘制、`bRestartPending=1`、`iRestartAttempts=0`，随后断言 `iReady=iDrawn=1`、故障/待重启/恢复标志清零、尝试数为 1、新浏览器 PID 不同并再次绘制公式。资源映射失败路径断言无自动重启。最终实窗日志 `artifacts/xui-document-rebuild/validation-2026-09-30-document-provider-auto-restart-provider-final3.log` 退出码零；可选 DLL/widget、离屏高级对象渲染、正式导出边界分别见 `validation-2026-09-30-document-provider-auto-restart-widget-final2.log`、`validation-2026-09-30-document-provider-auto-restart-render-final2.log`、`validation-2026-09-30-document-provider-auto-restart-exports-final2.log`，均退出码零且正式 DLL 不导出 14 个内部通道符号。默认 Windows Document 全量套件 `validation-2026-09-30-document-provider-auto-restart-full-suite.log` 退出码零，覆盖 652 项 CommonMark、283 项公开导出、Renderer/Editor、MessageList 与原生绘制；GCC 严格静态分析、`api-docs/tools/comment_lint.py` 和差异检查通过。60 秒稳定期预算重置、主框架/GPU 进程失败和真实跨平台窗口未由此专项证明。

补充连续故障实窗矩阵：同一 provider 在第一次恢复后又连续遭遇三次经进程身份校验的浏览器退出；第二、第三次按 2/4 秒重新就绪，新旧浏览器 PID 不同，已绘制公式的缓存仍报告 ready/drawn。第四次故障时 iRestartAttempts=3、bRestartPending=0，继续 30 帧也不再重启。最终实窗日志 validation-2026-09-30-document-provider-auto-restart-provider-final3.log 退出码零。稳定运行 60 秒后的预算复位未专项验证。

Document 同字体单行跨 Text 节点字距与着色验证（2026-09-30）：新 Renderer 测试把 AV 分成默认色与显式色两个 Text 节点，用代理模拟两字合排时前字 advance 缩短 3 像素，要求拆分/未拆分段落的末尾光标同位、第二节点起点与字距一致、drawTextSpans 一次收到原文及两个正确颜色范围；再在原节点改色，要求不新增 shaping 字节、光标不移动、绘制颜色更新。新增 Linux 无窗口 Renderer ASan/UBSan 用例让同一拆色段落依次经历宽→窄→宽的联合整形创建、换行回退和缓存释放，最终 `artifacts/xui-document-rebuild/validation-2026-09-30-paint-group-linux-renderer-asan-final.log` 与加入 run 上限后的 `validation-2026-09-30-paint-group-bounded-linux-renderer-asan.log` 均退出码零。首次 Windows 完整套件 `validation-2026-09-30-paint-group-full-suite.log` 在旧样式测试只观察 drawText 的颜色断言处停下；让测试同时观察 drawTextSpans 后，专项 `validation-2026-09-30-paint-group-style-test.log` 和最终 `validation-2026-09-30-paint-group-full-suite-final.log` 与加入 64 run 上限后的 `validation-2026-09-30-paint-group-bounded-final-suite.log` 均退出码零，后者覆盖独立 Core、实际 DLL、652 项 CommonMark、283 项公开导出、Renderer/Editor、MessageList 与原生绘制。GCC -Wall -Wextra -Werror -fanalyzer 与 git diff --check 通过。XGE span 裁剪的原生像素用例已接入 `test_xui/xui_proxy_xge_test.c`，当前离屏环境在绘制阶段返回 NOT_INITIALIZED，因此 `validation-2026-09-30-span-clip-proxy-final.log` 仅验证了无绘制后端的跳过路径；它不构成裁剪像素已通过的证据。该批只涵盖同字体、无装饰、完整单行、至多 64 个 run 且不超过 8 KiB 的跨节点文字；多行/复杂脚本/双向文字仍未完成。

Document 同字体多行段落整形验证（2026-09-30）：test_xui/xui_document_renderer_test.c 的代理对相邻 AV 加入上下文字距；同一 AV AV 原文分别构造成单 Text 节点和四个颜色拆分节点。初始专项 `artifacts/xui-document-rebuild/validation-2026-09-30-wrapped-paint-before.log` 证实拆分段落的光标/行几何不同；只按旧行切分重新整形仍落入三行局部结果，见 `validation-2026-09-30-wrapped-paint-geometry-debug.log`。最终实现先以整段整形提出候选换行，再逐行整形并核对稳定性；专项 `validation-2026-09-30-wrapped-paint-seeded-probe2.log` 通过拆分/未拆分的两行、行内光标和两次颜色跨度断言。加入字形右侧外延宽度后，最终 Windows 套件 `validation-2026-09-30-wrapped-paint-final-suite.log` 退出码零，覆盖独立 Core、实际 DLL、652 项 CommonMark、283 项公开导出、Renderer/Editor、MessageList 与原生绘制；Linux Renderer ASan/UBSan `validation-2026-09-30-wrapped-paint-final-linux-asan.log`、严格 GCC -Wall -Wextra -Werror -fanalyzer `validation-2026-09-30-wrapped-paint-final-analyzer.log` 及 git diff --check 均通过。适用范围只到同字体、无装饰/对象/硬换行、8 KiB/64 run/512 行且 cluster 映射逐一匹配的段落；这不能证明 Bidi、连字跨节点合并或大段落完整排版。

R1 跨节点字素与合并 emoji 验证（2026-09-30）：实际 DLL 与 Linux 共用 `xui_document_grapheme_renderer_cases.h`，对拆分/未拆分的 e+组合重音、ZWJ emoji、含前后普通字符的 emoji，以及字素内部的空 Text 节点，检查宽→窄→宽重排、行末和后续字符光标、两侧点击命中、内部 UTF-8 位置的 BEFORE/AFTER、只选择组合符或 ZWJ 的整字素范围及一次透明选区填充。模拟 shaper 保留组合符的多个 scalar cluster，同时把 emoji 的三个 cluster 合并为一个，覆盖两种代理粒度；绘制检查完整 emoji 与连续颜色跨度。代码行和 SOURCE 行也执行内部位置、命中、范围及背景回归。另用含 1000 个组合符的单字素检查光标访问计数，避免只计二分定位而漏掉后续扫描。实际 Windows XGE proxy 使用 Arial 和自定义 SVG emoji pack，确认 11 字节 ZWJ 序列形成一个真实 cluster，拆分/未拆分文档的宽度与三次重排几何一致；该专项验证整形和布局，不作为 emoji 像素绘制证明。

共享 `xui_document_grapheme_editor_cases.h` 在实际 DLL Editor 和 Linux 无窗口 Editor 上验证跨颜色节点的左右移动、整字素复制范围、鼠标点击后的光标、Backspace/Delete 与一步 Undo。首次扩展 SOURCE 用例复现内部位置错误落在 x=7，而首尾应为 x=0/14，见 `artifacts/xui-document-rebuild/validation-2026-09-30-grapheme-source-probe.log`；补齐源码行和代码切片后，专项 `validation-2026-09-30-grapheme-source-fixed.log` 退出码零。加入真实访问计数后，`validation-2026-09-30-grapheme-final-suite3.log` 复现 Draw 访问次数超过既有 20000 门槛；布局预先标记多片段字素成员，独立片段不再扫描邻接边界，最终保留原门槛通过。

最终源码 Windows 完整套件 `artifacts/xui-document-rebuild/validation-2026-09-30-grapheme-final-suite4.log` 退出码零，包含 Core、652 项 CommonMark 解析/源码/原生往返、实际 DLL、283 项公开导出、Renderer/Editor、样式、规模、fractional、表格、MessageList 与原生绘制；CommonMark 计数不等同于 HTML conformance。最终 Linux Renderer ASan/UBSan `validation-2026-09-30-grapheme-final-linux-renderer2.log`、View/Editor ASan/UBSan `validation-2026-09-30-grapheme-final-linux-view2.log` 退出码零，后者仍有未修改 DatePicker 的已有编译告警。修改源码的严格 GCC `-fanalyzer -Wall -Wextra -Werror` 日志 `validation-2026-09-30-grapheme-final-analyzer.log` 为空；共享测试严格 C 语法检查与差异检查通过。

验收边界：联合整形继续受 8 KiB、64 run、512 行、同字体与无 marks/对象/硬换行限制；跨节点合并只接受同一 Unicode 字素内的合并，跨字素连字、重排 cluster 的 Bidi、复杂脚本与字体变化仍待 R1 完整实现。特别长的单字素仍有线性查询成本，访问计数已记录该成本。字符图形是否正确组合仍受底层 shaper 能力约束，本批几何一致性不等于完整 OpenType/复杂脚本整形交付。整体重构长期目标继续进行。

## 同字体装饰联合整形与原生字体度量（2026-09-30）

共享 test_xui/xui_document_decoration_renderer_cases.h 在实际 Windows DLL 与 Linux Renderer sanitizer 上执行。代理对 AV 模拟 3 像素字距，并把 ZWJ emoji 的三个 scalar cluster 合并为一个；AV AV 分别构造为单 Text 节点与四个带粗体/斜体/代码、高亮、下划线、链接和删除线的节点，显式让五种字体角色解析到同一 font。20→300→20 宽度重排后，拆分和未拆分段落的行内/行末光标同位，两条视觉行各一次完整跨度绘制，颜色分段的下划线/删除线使用指定基线偏移与粗细，高亮只填充对应背景。此处的粗体/斜体 marks 不表示已验证不同字体之间的联合排版。

装饰度量覆盖有效值、NAN/INFINITY/负厚度以及回调先写入有效数值再返回错误，后两类均使用默认值；移除首节点下划线并提交新快照后，装饰从六段变为五段。合并 emoji 中只标记 ZWJ 或末尾码点时装饰覆盖完整字素，空 Text 下划线不产生额外线段；drawLine 返回错误时 Renderer 返回原错误并精确恢复调用方 clip。单节点普通 drawText 路径只绘制一次文字，不携带后端下划线 flag，再由共同逻辑绘制两种装饰，防止重复下划线。drawLine 由现有 xuiSetProxy 校验要求，未把缺少回调的代理作为合法可选组合。

原生 test_xui/xui_document_renderer_test.c 加载 Arial 18，同时读取 XGE 原字体与 XUI proxy；四项装饰位置/厚度逐项一致且符号正确。既有实际 XGE emoji pack 继续通过合并 cluster 与宽窄重排几何检查。通用 native-smoke/native-live-smoke 仍通过，但本批未新增只针对下划线/删除线的原生像素断言，装饰细节证据来自上述回调几何与字体度量核对。

最终验证记录：

- artifacts/xui-document-rebuild/validation-2026-09-30-decorated-final-suite.log：退出码零，Core、652 项 CommonMark、实际 DLL、283 项公开导出、Renderer/Editor、样式、规模、fractional、表格、MessageList 与原生绘制通过；CommonMark 为解析/源码/原生往返，不是 HTML conformance。
- validation-2026-09-30-decorated-final-linux-renderer2.log：Linux Renderer ASan/UBSan，退出码零。
- validation-2026-09-30-decorated-final-linux-view.log：Linux 无窗口 Rich/Markdown View/Editor ASan/UBSan，退出码零；保留未修改 DatePicker 的已有编译警告，不构成真实 Linux 窗口/IME 证据。
- validation-2026-09-30-decorated-final-analyzer.log、validation-2026-09-30-decorated-final-test-syntax.log：最终修改源码严格 GCC analyzer 和共享/原生测试语法检查，退出码零，日志为空。
- validation-2026-09-30-decorated-final-comment-lint.log：API 注释门禁退出码零；git diff --check 通过。

xui_font_metrics_t 的结构长度由 5 个 float 改为 9 个 float，要求宿主、自定义 proxy 与 DLL 同版重编译。联合路径仍受 8 KiB、64 run、512 行、同字体、无对象/硬换行/上下标限制；marks 的 metric 失效保留，以兼容 onFont 按标记选择实际字体。R1、整体富文本/Markdown 重构与真实平台矩阵不因此标记完成。

同一最终源码的 WebView2 可选 DLL 也已重新构建，validation-2026-09-30-decorated-final-webview-widget.log 退出码零，基础控件、12 轮生命周期、两实例独立运行和浏览器故障重建通过。Document 内部 provider 实窗复测 validation-2026-09-30-decorated-final-web-provider.log 退出码零，覆盖 KaTeX/Mermaid 来源更新、DocumentView/MessageList 绘制、主题更新、HTML 静态显示、错误卡片和浏览器故障有界自动恢复；validation-2026-09-30-decorated-final-web-render.log 退出码零，离线公式/Mermaid 截图尺寸与非空像素检查通过。上述记录均位于 artifacts/xui-document-rebuild/。通用 WebView 公开边界没有因本批增加脚本/消息桥，Document 内部渲染仍保留；不构成其他平台 WebView 后端的运行证据。

## 多字体段落、空 Text 与统一换行（2026-09-30）

共享 xui_document_mixed_font_cases.h 用真实不同 font 对象模拟 20/40 号字体，对 AV AV AV 的三个字体区段分别构造完整 Text 与颜色拆分 Text。每种构造在 120→45→12→120 宽度下比较三个区段的末尾光标及全部范围矩形，宽行的三个末尾为 x=27/84/101，普通文字 y=16、大字号文字 y=0；同一行分别使用各字体自己的装饰度量和正确物理基线。清除中间区段粗体后，字号恢复为 20，末尾为 x=71，Undo 恢复 x=101。整段候选与逐行候选中第二字体的模拟整形失败均返回 INVALID_STATE，清理候选后在同一 renderer 上重试成功；ASan/UBSan 验证无泄漏。

两个空节点用例分别把较大字体的空 Text 放在段首及 A/V 之间：空节点字体不切断 AV 字距，末尾仍为 x=17，普通文字 y=16，空节点光标高度仍为 40，文字与装饰都以实际文字原点绘制。段首空 Text 的初始字素成员误标由 byte 0 断行表条目缺失造成，Document 两条映射路径已明确排除零位置。先失败记录为 artifacts/xui-document-rebuild/validation-2026-09-30-mixed-font-before.log、validation-2026-09-30-mixed-font-empty-before.log；调试记录 validation-2026-09-30-mixed-font-empty-debug.log 显示首轮曾错误得到 y=0、h=20。

原生 XGE 专项使用 Arial 18 与 Arial Bold 30，验证颜色拆分不会改变混合字体段落在宽→窄→宽时的区段光标 x/y/height。该专项没有绘制新的混合字体截图，不能代替独立原生像素校验；既有 native-smoke/native-live-smoke、异步预览和图片错误占位像素门槛仍通过。

最终记录均位于 artifacts/xui-document-rebuild/：

- validation-2026-09-30-mixed-font-final-suite2.log：退出码零，Core、652 项 CommonMark 解析/源码/原生往返、实际 DLL、283 项公开导出、Renderer/Editor、样式、规模、fractional、表格、MessageList 和原生绘制。CommonMark 计数不是 HTML conformance。
- validation-2026-09-30-mixed-font-final-linux-renderer2.log：Linux Renderer ASan/UBSan，退出码零，含共享多字体、空节点与故障重试专项。
- validation-2026-09-30-mixed-font-final-linux-view.log：Linux 无窗口 Rich/Markdown View/Editor ASan/UBSan，退出码零；仍有未修改 DatePicker 的既有编译警告。
- validation-2026-09-30-mixed-font-final-analyzer.log、validation-2026-09-30-mixed-font-final-test-syntax.log：严格 GCC analyzer 与测试语法检查，退出码零，日志为空；差异检查通过。

首次 validation-2026-09-30-mixed-font-final-suite.log 在编译阶段因 C 盘临时目录 No space left on device 退出，尚未执行新代码验证；suite2 只为当前测试子进程设置 TMPDIR/TEMP/TMP=D:\GIT\xge\build\document\compiler-tmp。Linux View 编译也使用 D 盘的 compiler-tmp-linux，没有修改系统永久环境。对 C 盘已闲置的本任务唯一命名 WebView provider 测试目录尝试递归清理时，自动审批策略拒绝，未执行删除；后续验证继续使用 D 盘目录。

适用边界：全段最多 8 KiB/64 run/512 行，只有候选换行收敛后发布；字素内部实际字符换字体、上下标、对象与硬换行仍走现有独立 run 路径。完整连字/Bidi/复杂脚本、长段落联合整形、真实跨平台窗口和原生像素矩阵仍未完成。

本批同一最终源码重新构建 WebView2 可选 DLL：artifacts/xui-document-rebuild/validation-2026-09-30-mixed-font-final-webview-widget.log、validation-2026-09-30-mixed-font-final-web-provider.log、validation-2026-09-30-mixed-font-final-web-render.log 均退出码零。Windows 基础控件、生命周期/双实例/进程恢复，以及 Document 内部公式/Mermaid/HTML 静态 provider、来源更新、主题、错误卡片、有界自动恢复和离线截图检查通过；通用公开数据通信边界保持原范围。

## 上下标混排、位移与原生像素（2026-09-30）

xui_document_vertical_font_cases.h 由 Windows 实际 DLL 和 headless Renderer 测试共同包含，本批实际执行证据仅有 Windows。四个连续区段 AV/上标 AV/下标 AV/AV 分别构造完整 Text 与颜色拆分 Text，检查四次宽度变化的光标 x/y/height 和完整范围矩形。20/15 字体策略宽行的区段末尾为 27/46.5/66/83，普通/上标/下标 y 为 .5/0/9；onFont 全部返回 20 时末尾为 27/54/81/98，y 为 6/0/12。后者证明同一字体对象不能合并不同实际基线的文字。宽行每个区段第一字符的近末端命中回到对应 NodeId/offset；12 段装饰线逐一核对颜色、端点、厚度和实际文字基线。

清除上标后，20/15 策略末尾为 90.5，Undo 恢复 83；将下标改为上标后，上下两个脚本区段共享上移位置，Undo 恢复下移位置。同一字体策略也执行以上转换和完整范围对照。整段种子/逐行候选的模拟整形错误返回 INVALID_STATE，随后相同 renderer 重试成功。四种空上下标节点样本覆盖段首/AV 中间，上标空节点 y=0、普通文字 y=6；下标空节点 y=6、普通文字 y=0；末尾 x=17，空节点高度 20，实际文字只有一组绘制，装饰跟随首个实际文字而不是空节点。失败前记录为 validation-2026-09-30-script-before.log。

native_vertical_font_frame 在真实 XGE frame 内使用 Arial 20 与默认 fontCreateSized 路径，验证脚本字号缩小、改宽几何和所有范围；完整/拆分 Text 在两个独立 520×160 RGBA8 目标上绘制，读回全部像素后逐字节比较，并核对四种颜色有实际像素、上标/普通/下标的上下边界严格递增。最初无图形后端的 harness 失败记录保留在 script-native-first/second.log；改为真实 frame 后通过，不对缺失绘制后端放宽或跳过像素断言。

最终证据位于 artifacts/xui-document-rebuild/：

- validation-2026-09-30-script-final-renderer.log：当前完整专项退出码零，含颜色、改宽、命中、装饰、空脚本、格式转换/Undo、故障重试与真实字体 RGBA 对照。
- validation-2026-09-30-script-final-suite2.log：退出码零，完整 Windows Core、652 项 CommonMark 解析/源码/原生往返（不是 HTML conformance）、实际 DLL、283 公开导出、Renderer/Editor、样式/规模/fractional、表格、MessageList 与原生绘制。首次 suite 的测试同时设置上下标，被 schema 正确拒绝；修正测试为互斥转换后再运行完整套件。
- validation-2026-09-30-script-final-layout-analyzer.log：gcc -std=c11 -Wall -Wextra -Werror -fanalyzer -I. -c src/xui_document_layout.c，实际生成对象文件，退出码零，日志为空。
- validation-2026-09-30-script-final-test-syntax.log、validation-2026-09-30-script-final-portable-syntax.log：Windows/native/shared 测试与 portable 测试的严格 C 语法检查，退出码零，日志为空；后者不代表 Linux 运行时验证。

validation-2026-09-30-script-final-linux-renderer.log 的 WSL 命令退出码 -1、Wsl/Service/E_UNEXPECTED，未进入 sanitizer 编译/执行；/bin/true 启动重试 script-wsl-retry.log 仍退出 -1，Ubuntu 当前报告 Stopped。C 盘当时约剩 10–46 MiB，不能仅凭这个值断定全部错误原因；当前 Linux Renderer/View/Editor ASan/UBSan 尚待环境恢复后补跑，上一批通过记录不能用于宣称本批通过。当前 Windows 编译/可选 WebView 子进程使用 D 盘 TMPDIR/TEMP/TMP，不修改系统或 WSL 配置。

实现适用范围仍为最多 8 KiB/64 run/512 行的连续 Text；字素内实际字符的字体或上下标变化、对象/硬换行、跨字素连字、复杂脚本/Bidi 和超长段落联合整形未完成。长期目标保持进行中。

同一最终产品源码的 Windows WebView2 可选 DLL 已重建，script-final-webview-widget.log、script-final-web-provider.log、script-final-web-render.log 均退出码零：基础控件生命周期与双实例、进程失败重建；Document 内部 KaTeX/Mermaid/HTML 静态图像、DocView/MessageList 内容更新、主题、错误卡片、截图解码/裁切恢复及三次有界浏览器重启全部通过。通用 WebView 公开功能范围未扩大。

## 对象与物理换行附近的整形和绘制（2026-09-30）

先失败记录 validation-2026-09-30-object-shape-before.log：图片导致整段联合路径退出，AV 颜色拆分版与完整 Text 的尺寸不同。冻结源码的共享 xui_document_object_shaping_cases.h 在 Windows 实际 DLL 上逐项验证：

- 13×36、baseline 24 的图片/公式两侧普通 20 与上标 15 字体，四次改宽后两种节点结构的尺寸、光标和全部范围矩形相同。宽行普通文字末尾 x=27、y=8，上标末尾 x=52、y=7.5；对象 x=27、y=0，宽高 13/36，点击回到对象 NodeId。
- 资源测量改为 19×44、baseline 30 并调用 InvalidateObjects 后，上标末尾 x=58，普通/上标 y=14/13.5，两种结构范围仍相同。
- 软换行保留原有 6 像素间隙，末尾 x=45；硬换行、Text 内 LF、同 run CRLF、跨 run CRLF 的下一行上标末尾 x=12、y=24。绘制逐项核对两组文字、六段颜色装饰和对象调用次数；文字回调拒绝任何 CR/LF 控制字节。
- 种子及逐行候选的模拟整形错误分别返回 INVALID_STATE，再次布局恢复正确几何；本批没有 Linux sanitizer 运行证据，不能把重试成功当作无泄漏的运行时证明。
- 单 Text 与六个 Text 的 AV\r\nAV\n\nAV，在宽→窄→宽后保留三个字距为 17 的文字行，宽行 y=0/24/72，空白行高度未丢失；三个装饰段位于基线对应的 18.5/42.5/90.5。
- 纯 CRLF/空白控制文本、前导/尾随 CRLF、控制文本加图片：保留正高度，无控制字节绘制；没有实际文字时不发布空的 paint group，图片仍绘制。

native_object_font_pixels 在真实 XGE frame 使用 Arial 20 及默认上标字号缩放，在普通文字与上标文字之间插入 13×36 对象，并让上标 Text 内含 LF。完整与拆分 Text 的对象/光标在三次改宽中相同；两个独立 520×160 RGBA8 目标的所有像素相同，且蓝色普通文字、红色上标和紫色对象分别具有实际像素。此证据为真实字体与 provider 回调矩形的组合；真实 KaTeX/Mermaid/HTML 另由可选 WebView provider 套件检查。

fractional 回归首次在 object-shape-final-suite.log 失败，因为旧记录器只挂接 drawText，未记录新路径的 drawTextSpans。当前记录器覆盖两种回调，仍要求实际绘制行数及顺序完全等于独立期望，并额外验证 x/width、不含控制字节和 span 路径实际执行；48 段、小数缩放/间距及三种滚动位置全部通过，未放宽原像素舍入门槛。

最终证据均位于 artifacts/xui-document-rebuild/：

- validation-2026-09-30-object-shape-frozen-build.log：冻结产品源码后显式重建默认 DLL，退出码零。
- validation-2026-09-30-object-shape-final-renderer.log、validation-2026-09-30-object-shape-fractional-fixed.log：新增共享/原生对象、控制边界与完整小数坐标专项，退出码零。
- validation-2026-09-30-object-shape-final-suite2.log：完整 Windows 套件，退出码零，含 652 项 CommonMark 解析/源码/原生往返（非 HTML conformance）、283 项公开导出、Renderer/Editor、规模/样式/fractional、表格、MessageList 和原生绘制。
- validation-2026-09-30-object-shape-layout-analyzer.log、validation-2026-09-30-object-shape-renderer-analyzer.log：严格 GCC analyzer 实际 -c 编译，两份日志为空，退出码零。
- validation-2026-09-30-object-shape-test-syntax.log、validation-2026-09-30-object-shape-portable-syntax.log：Windows/shared/native 与 portable 测试严格 C 语法检查，退出码零；portable 语法通过不是 Linux 运行时证明。

object-shape-wsl-probe.log 的 /bin/true 启动探测仍为 -1、Wsl/Service/E_UNEXPECTED，未进入 Linux；本轮当前源码 Linux Renderer/View/Editor ASan/UBSan 保持待补。连续文字与对象/控制共享排版仍受 8 KiB 段落投影、64 run、512 行边界限制，非文字占位各按三字节计费；字素内字体/位移切换、完整软换行字体/装饰上下文、跨字素连字、复杂脚本/Bidi、超限段落及真实跨平台/IME/DPI 矩阵仍未完成，长期目标继续。

当前源码的 WebView2 可选 DLL 已重新构建；validation-2026-09-30-object-shape-final-webview-widget.log、validation-2026-09-30-object-shape-final-web-provider.log、validation-2026-09-30-object-shape-final-web-render.log 全部退出码零。Windows 基础控件生命周期/双实例/进程恢复，以及 DocView/MessageList 内部 KaTeX、Mermaid、HTML 来源更新、主题、错误卡片、截图解码/裁切恢复、有界重启、离线测量/沙箱/PNG 通过；没有增加通用公开 WebView 数据通信功能。

## 显式软硬换行的字体与联合绘制（2026-09-30）

先失败 validation-2026-09-30-break-font-before.log 复现了固定 6 像素软换行与相同字体普通空格的光标/尺寸差异。软、硬换行现在各缓存按有效节点属性整形的一个私有空格，使用实际字体行高与上下标逻辑基线；只有软换行使用其宽度。原子节点仍无文本，保存内容、偏移和三字节段落投影占位不变。与相邻同字体文字联合绘制时，软换行投影为一个空格，配合逐 run 颜色/装饰及背景；预算计费仍使用三字节，私有换行字体 run 计入原 64 run 上限。

共享 test_xui/xui_document_break_font_cases.h 验证 12 组组合：默认 20 号、父级 40 号、H1 36 号及 15 号上标，普通/上标硬换行，以及每组 1/1.25 缩放。各组宽 200→0.8×字号→42→200 时，显式换行和对应普通空格/文本 LF 的末端光标 x/y/高度及文档尺寸相等。换行节点矩形采用真实空格宽度、字体行高；硬换行实际 advance 为零，但 GetNodeRect 沿用至少 1 像素的命中矩形契约。宽布局下断言两项装饰经过空格中心、使用正确颜色及实际基线位置。每组测量失败均返回原错误，并在同一 Renderer 上成功重试；20→40 号换行样式提交后间隙 10→20，邻字行顶 0→16，Undo 恢复原值。

Markdown 链接 [A 换行 V] 的 SoftBreak 保留 LINK 标记及零文本长度；原始源码逐字节不变，Renderer 的间隙宽 10、末端光标 x=30，主题链接下划线穿过间隙。纯换行段落、连续软换行及中间硬换行覆盖正常 drawTextSpans 和缺少该回调的路径：40 号空格宽 20、高 40，硬换行后下一行 y=44，下划线/删除线均保留。没有把普通字符串中的 LF 改写为 SoftBreak，也没有修改语义复制中换行输出规则。

原生 XGE Arial 专项在真正图形帧中比较四种软/硬换行与普通/上标组合，宽 120→18→120。每次布局都比较末端光标，并读取两张 520×160 RGBA8 target 的全部像素，断言真实蓝色像素存在。它同时验证文字、下划线、删除线及不透明高亮背景。最初独立提交两侧装饰虽然几何相等，仍出现 18 像素差异，见 break-font-native-debug.log；没有放宽像素断言，而是让软换行进入连续文字和装饰合并，最终所有组合完整 RGBA 相等。

当前最终证据全部位于 artifacts/xui-document-rebuild/：

- validation-2026-09-30-break-font-frozen-build.log：显式默认 DLL 构建，退出码零。
- validation-2026-09-30-break-font-final-renderer.log：完整 Renderer 及共享/原生新增专项，退出码零。
- validation-2026-09-30-break-font-final-suite.log：完整 Windows 套件，退出码零；652 项 CommonMark 解析/源码/原生往返（非 HTML conformance）、283 个公开函数导出、Renderer/Editor、规模/样式/fractional、表格、MessageList 与原生同步/异步/占位图绘制。
- validation-2026-09-30-break-font-layout-analyzer.log、validation-2026-09-30-break-font-renderer-analyzer.log：冻结产品源码后严格 GCC -fanalyzer 实际 -c 对象编译，退出码零、日志为空。
- validation-2026-09-30-break-font-test-syntax.log、validation-2026-09-30-break-font-portable-syntax.log：Windows/shared/native 与 portable 测试严格 C 语法，退出码零。
- validation-2026-09-30-break-font-final-linux-renderer.log、validation-2026-09-30-break-font-final-linux-view.log：当前累积源码 Renderer、Rich/Markdown View/Editor 无窗口代理 ASan/UBSan，退出码零。脚本启用泄漏检查和首次错误终止；无外部 ASAN_OPTIONS/UBSAN_OPTIONS 覆盖，见 break-font-linux-sanitizer-env.log。View 编译保留既有 date-picker maybe-uninitialized 警告，修改的 layout/renderer 严格 analyzer 无警告。

WSL 最初 /bin/true 探测仍失败为 Wsl/Service/E_UNEXPECTED，稍后第二次探测退出码零，之后才运行上述 Linux 检查。没有修改系统/WSL 配置或清理 C 盘，两平台编译临时目录都位于 D 盘。此前上下标、对象与 CRLF 批次的当前 Linux Renderer/View/Editor 验证缺口由本批累积源码运行补齐；Linux 真实 XGE、IME、读屏及其他平台仍待补。联合排版的 8 KiB/64 run/512 行限制、字素内字体/位移变化、跨字素连字、复杂脚本/Bidi 和完整长期目标未完成。

同一最终源码的可选 WebView2 DLL 与基础控件、Document provider、离线高级渲染全部退出码零，日志分别为 validation-2026-09-30-break-font-final-webview-widget.log、validation-2026-09-30-break-font-final-web-provider.log、validation-2026-09-30-break-font-final-web-render.log。真实 DocView/MessageList 的 KaTeX、Mermaid、HTML 内容更新、主题、错误卡片、截图解码/裁切恢复及三次有界重启通过；离线渲染的 500×350 PNG 深色像素分别为 135/426/1233。没有扩展通用 WebView 的公开脚本或数据通信能力。

## Unicode 强制控制的统一断行、绘制与编辑（2026-09-30）

先失败 `validation-2026-09-30-mandatory-before.log` 复现 NEL/LS/PS 等控制后的光标仍在第一行。共享断行表现在区分控制起点 HARD 与完整序列终点 HARD_END，CRLF 只在 LF 的末端结束一次；普通 EOF 不具有强制终点标记。Document 段落投影和代码/SOURCE 路径据此识别独立控制片段，宽度为零、保留原字节和字体行高，拒绝把控制提交到 drawText/drawTextSpans 或装饰。上游 libunibreak 源码及属性表没有修改，仍使用固定版本的数据；本批用 [Unicode UAX #14 的控制分类](https://www.unicode.org/reports/tr14/#BK) 核对行为，没有宣称完成 Document 的全部 Unicode 排版。

代码收集采用与精排相同的 Unicode 硬行扫描；4096 字节读取具有三个前瞻字节，完整消费跨块 UTF-8 控制及 CRLF，8192 字节分片门槛与估高遵循实际强制行数。可打印 ASCII 使用无分配的字节扫描。SOURCE 中同一物理行的 Unicode 视觉行使用零行间距，与相邻物理 SOURCE 行一致；SourceStore 的 CR/LF/CRLF 物理行索引及 Markdown 解析语义不变。

共享 `test_xui/xui_document_unicode_break_cases.h` 同时由 Windows DLL Renderer 和 Linux portable Renderer 编译运行：

- 八类控制为 LF、CR、CRLF、VT、FF、NEL、LS、PS。完整 AV-控制-AV 与颜色拆分节点（含跨 Text 的 CRLF）在 200→40→12→200 改宽后，尺寸、末端光标和全部范围矩形一致；宽行两个 AV 的字距为 17，第二行 y=24，下划线使用各自真实基线。两个文字绘制入口都拒绝控制字节。
- 纯控制文本保持正高度，没有文字或装饰绘制；随后在同一 Document 事务中追加 AV 并更新同一 Renderer，两个空白行加末行总高 68、末端 x=17/y=48，实际文字仅绘制一次，下划线位于 y=66.5。该用例验证中间空行及更新，未固定末尾控制后的空光标行最终规则。
- 代码与 Markdown SOURCE 分别覆盖短行和两段 4095 字符的边界样本，验证第三行末端光标、命中和控制不绘字，覆盖多字节控制/CRLF 跨 4096 字节读取及代码分片边界；代码末端 y=48，SOURCE y=40。
- Windows 真正 XGE 图形帧中的 `native_mandatory_break_pixels` 使用 Arial 20，把八类控制分别与 LF 对照，在宽 120→18→120 后比较光标和两张 520×160 RGBA8 target 的全部像素。文字、下划线、删除线及不透明背景像素完全相同，并检查实际蓝色像素存在。

共享 Editor 用例由 Windows Editor 和 Linux View/Editor 编译运行，在 Rich VISUAL 与 Markdown SOURCE 中逐项验证八类控制的 Down/Up、第二行命中、整个控制字素（含 CRLF）Backspace 及逐字节 Undo。控制字节仍是同一 Document 的内容，未引入另一套可写文本。

本批最终证据位于 `artifacts/xui-document-rebuild/`，以下运行均退出码零：

- `validation-2026-09-30-mandatory-frozen-build.log`：产品源码冻结后显式重新构建默认 DLL。
- `validation-2026-09-30-mandatory-final-renderer2.log`、`validation-2026-09-30-mandatory-editor.log`：最终共享/原生 Renderer 与完整 Editor 专项。
- `validation-2026-09-30-mandatory-final-suite.log`：完整 Windows 套件，包含 Core、652 项 CommonMark 解析/源码/原生往返（非 HTML conformance）、实际 DLL、283 项公开导出且无旧 Rich API、Renderer/Editor、样式/规模/fractional、表格、MessageList 和原生同步/异步/缺图绘制。完整套件结束后仅增强空行追加 AV 测试，产品源码没有变化；增强用例由 Renderer2 两平台独立复跑验证。
- `validation-2026-09-30-mandatory-linux-renderer2.log`、`validation-2026-09-30-mandatory-linux-view.log`：当前累积源码 Linux Renderer、Rich/Markdown View/Editor ASan/UBSan，通过进程局部环境显式启用 `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`、`UBSAN_OPTIONS=halt_on_error=1`。View 保留既有 date-picker maybe-uninitialized 编译警告。
- `validation-2026-09-30-mandatory-text-break.log`：共享 plain text 官方矩阵 UAX14 Unicode 15.0 共 7653 个通过、一个明确验证的既有上游例外，UAX29 Unicode 15.1 共 1187 个通过、零例外；新增八类 HARD/HARD_END、CRLF 和普通 EOF 检查。原有访问量/规模/分配失败门槛保持，九次注入失败、十五次成功均重试且无遗留分配。这是共享模块矩阵，不能当作整个 Document/Bidi 已完成的证据。
- `validation-2026-09-30-mandatory-layout-analyzer.log`、`validation-2026-09-30-mandatory-renderer-analyzer.log`：严格 GCC `-fanalyzer -Wall -Wextra -Werror` 实际 `-c` 对象编译，日志为空。
- `validation-2026-09-30-mandatory-text-analyzer2.log`：同一严格 analyzer 编译 xui_text.c，仅按已有 UI 测试约定抑制公共内部头未使用静态辅助函数的 unused-function，日志为空；第一次不加该抑制的 `mandatory-text-analyzer.log` 因该警告退出码一，原失败记录保留，未抑制 analyzer 内存诊断。
- `validation-2026-09-30-mandatory-test-syntax.log`、`validation-2026-09-30-mandatory-editor-syntax.log`：Windows 与 portable 共享测试严格 C 语法检查，日志为空；后续增强 Renderer 用例又经两平台实际编译运行。

PS 当前在已有 Text 内强制断行，不隐式建立 Paragraph 节点；FF 不引入分页。末尾控制空光标行、PS 段落级输入规则、SHY/ZWSP/WJ/FEFF 等隐形控制与选中软连字符、8 KiB/64 run/512 行以外的联合排版、字素内字体/位移变化、连字、复杂脚本/Bidi、真实跨平台字体/IME/读屏/物理 DPI 矩阵，以及设计中其余工作包仍待完成。两平台编译临时目录均位于 D 盘，没有修改系统/WSL 配置或清理 C 盘。
当前同一产品源码重新构建了可选 WebView2 DLL，`validation-2026-09-30-mandatory-final-webview-widget.log`、`validation-2026-09-30-mandatory-final-web-provider.log`、`validation-2026-09-30-mandatory-final-web-render.log` 均退出码零。基础 Windows 控件通过布局/焦点、十二轮生命周期、双实例与隔离浏览器进程失败后重新创建；Document 内部 KaTeX、Mermaid、HTML 在真实 DocView/MessageList 中通过来源更新、主题、错误卡片、截图解码/裁切恢复与三次有界重启。离线渲染的 500×350 PNG 深色像素分别为 135/426/1233，沙箱/测量/截图检查通过。通用公开 WebView 维持 Windows 基础网页显示与交互，其他原生平台后端暂留空；Document 内部脚本与截图通道继续保留，没有新增通用公开数据通信 API。

## 完整逻辑块的尾部强制换行与可输入末行（2026-09-30）

`validation-2026-09-30-tail-line-before.log` 先失败：20 号字体、行距 4 的两个终端控制原先总高只有两行，本批期望为三行 68。当前布局在完整逻辑块的最后一个强制片段之后建立私有零宽空行标记，保存控制的行高/基线和原始 EOF 偏移，不创建内容节点、不改变 SourceStore/序列化或历史，并排除文字及装饰绘制。代码的中间分片和段落未提交前缀不建立尾行；最终代码片的估高包含尾行，包含 EOF 正好位于 8192 字节切点的情况。SOURCE 已有物理 CR/LF/CRLF 末行目录不改变，行内 Unicode 控制形成的末行继续使用零行距。

共享 Renderer 对八类控制 LF/CR/CRLF/VT/FF/NEL/LS/PS，在完整 Text、颜色拆分（含跨 Text CRLF）、代码和 Markdown SOURCE 中检查宽 200→12→200、BEFORE/AFTER affinity、空行光标/命中/尺寸；代码与 SOURCE 另有 8191 字符前缀的读取/分片 EOF 边界。正文和代码尾行高度 20、起点 x=0/8、无窄行强制拆字时 y=24；SOURCE 尾行 y=20。两个连续控制总高 68、EOF 光标 x=0/y=48，事务追加 AV 后总高仍为 68、末端 x=17/y=48，空行没有文字或装饰提交。显式 HardBreak 前后父级 GAP 的光标分别在 AV 后及下一行，末行命中返回父容器末端 GAP，允许输入新 Text。

共享 `document_trailing_line_editor` 对 Rich Text、CodeBlock、Markdown SOURCE/LIVE 的八类控制，加显式 HardBreak，共 33 种构造，检查 Down/Up、空行命中和类型、末行输入 V、Undo、整个终端控制字素 Backspace 和逐字节恢复。Markdown 输入和恢复读取同一 SourceStore；Rich/代码检查同一内容树的语义纯文本。Linux 无窗口代理与 Windows 实际 DLL 编译运行同一用例，没有借助第二个文本模型。

Windows `native_mandatory_break_pixels` 在真正 XGE frame 使用 Arial 20，增加终端控制与 LF 的对照，和既有控制后有字的样本一起覆盖八类控制、120→18→120。终端光标必须 x=0、y>0；两侧光标 x/y/高度和完整 520×160 RGBA8 一致，且真实蓝色文字像素超过 20。下划线、删除线、高亮背景及空行均通过实际绘制，不以无图形后端的初始化代替像素验收。

证据位于 `artifacts/xui-document-rebuild/`：

- `validation-2026-09-30-tail-line-frozen-build.log`：最终产品源码显式默认 DLL 构建，退出码零。后续只增强 Windows 原生测试的终端控制对照，没有修改产品源码。
- `validation-2026-09-30-tail-line-final-renderer.log`：最终共享及原生 Renderer 专项，退出码零；包含新终端像素对照及既有完整 Renderer 回归。
- `validation-2026-09-30-tail-line-first-editor.log`：33 种新构造及完整 Editor，退出码零。它运行于最后一次等价查找分支整理之前；最终 DLL 的 Editor 由完整发布套件 tail-line-final-suite2 重新执行并通过。
- `validation-2026-09-30-tail-line-linux-renderer.log`、`validation-2026-09-30-tail-line-linux-view.log`：最终产品源码及同一共享测试 Linux Renderer/View/Editor ASan/UBSan，退出码零；显式进程局部 `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`、`UBSAN_OPTIONS=halt_on_error=1`。View 中既有 date-picker maybe-uninitialized 警告保留。Renderer 没有编译 Windows 专属原生像素用例，真实字体像素证据仅来自 Windows。
- `validation-2026-09-30-tail-line-layout-analyzer.log`、`validation-2026-09-30-tail-line-renderer-analyzer.log`：最终 layout/renderer 的 GCC `-fanalyzer -Wall -Wextra -Werror -Wno-unused-function` 实际 `-c` 对象编译，退出码零、日志为空；unused-function 仅抑制公共内部头未使用静态函数。
- `validation-2026-09-30-tail-line-final-webview-widget.log`、`validation-2026-09-30-tail-line-final-web-provider.log`、`validation-2026-09-30-tail-line-final-web-render.log`：同一产品源码的可选 DLL 重新构建、Windows 基础控件、内部高级 provider 和离线渲染均退出码零。十二轮生命周期、双实例/隔离进程恢复、DocView/MessageList 的 KaTeX/Mermaid/HTML 更新/主题、错误/解码/裁切恢复及三次有界重启通过；500×350 PNG 深色像素仍为 135/426/1233。通用公开 WebView 不新增通信 API。

完整 Windows 发布套件 tail-line-final-suite2 已退出码零，最终证据见本节末段。正文内强制控制末端与空样式节点边界、嵌套/复杂字体与末行样式、真实 IME/物理 DPI 等更多边界仍待验收；本批不等于完整复杂脚本/Bidi、SHY/隐形控制或全设计目标完成。

完整套件首次在 `validation-2026-09-30-tail-line-final-suite.log` 的旧规模断言停止（退出码一）：2048 个带终止 LF 的代码行仍按旧的 2048 行计算。已将独立期望改为 2049 行，并在原有按需排版门槛之外增加冷 EOF 光标 x=8/y=2048×16、高 14、末行命中原字节 EOF、少于四个测量分片、shaping 小于正文三分之一，以及 sparse/full 的 EOF 几何相同检查。原先首屏少于四块/shaping 小于四分之一、sparse 估高等于 full 精排、改宽不增加 shaping 和编辑重建门槛均保留。`validation-2026-09-30-tail-line-final-scale.log` 全部退出码零，96256 字节代码正文有十二个分片，日志中的 Markdown 首屏 shaping 为 8225 字节。完整套件 `tail-line-final-suite2.log` 已重新运行通过；没有改变产品源码，也没有放宽旧性能断言。

最终完整 Windows 套件 `artifacts/xui-document-rebuild/validation-2026-09-30-tail-line-final-suite2.log` 退出码零，覆盖当前产品与最终测试：Core、652 项 CommonMark 解析/源码/原生往返（非 HTML conformance）、实际 DLL、283 项公开导出且无旧 Rich API、Renderer 及终端真实像素、新末行 Editor、样式/规模/fractional、表格、MessageList 和原生同步/异步/缺图绘制。代码 EOF 的新规模门槛也在其中重新运行通过。产品源码在 frozen-build 后没有改变；GCC analyzer、两平台 sanitizer、当前可选 WebView2 回归及最终差异检查构成本批证据。长期目标保持进行中，未将该批次替代全部设计验收。

## 正文强制换行的跨节点亲和与空行点击输入（2026-09-30）

`validation-2026-09-30-boundary-before.log` 在新共享用例的跨 Text AFTER 与下一插入节点几何比较处先失败。当前 Renderer 以完整字素的强制终点确定下行位置，跨 Text/CRLF 也支持 AFTER；字号不同的空 Text 仍保留自身插入光标度量。正文 BEFORE 保留上行末端，完整逻辑块的私有 EOF 空行规则不变。未提交段落前缀的 AFTER 查询使用实际目标光标的底部判断是否已提交，不能仅因其前一个控制位于已提交行就发布下一行几何。

只含强制控制的空行按其完整字素起点命中，避免默认 AFTER 把光标移到别的行；显式 HardBreak 按父容器 CHILD_GAP 命中，允许在点击行输入而不是进入无孩子的叶节点。SOURCE 的物理 CR/LF/CRLF 行目录、原始字节、统一内容树、保存与历史没有变化，未增加公开 API。普通软换行的跨 run affinity 及更广字体/容器场景仍需独立验收，不把本批强制控制结果当作全段落排版完成。

共享 `xui_document_break_boundary_cases.h` 同时由 Windows 实际 DLL Renderer 与 Linux portable Renderer 运行：八类控制 LF/CR/CRLF/VT/FF/NEL/LS/PS × 完整/拆分 Text/后接 40 号空节点/末尾空节点四类，在宽 200→12→200 下比较 BEFORE、AFTER 与真实首个插入节点；跨 Text 的 CRLF 不暴露内部端点。宽行 BEFORE x=17/y=0，AFTER x=0/y=24；后接空节点时高度 40，否则 20。删除空节点、同一 Renderer 通过 ChangeSet 更新及 Undo 后，高度 40→20→40；其他光标几何也随节点结构更新。两行连续控制的 Rich/Code/SOURCE 空行命中各自原字节起点，随后 GetCaretRect 仍在实际点击行。两个 HardBreak 的三行命中父 GAP 0/1/2。73728 字节前缀用例要求查询控制的 AFTER 实际增加 shaping、确认目标行提交后，光标再与完整布局相同；首屏 shaping 仍小于 10000 字节。

共享 `document_blank_row_editor` 共 33 种构造（八类控制 × Rich/code/SOURCE/LIVE 加显式 HardBreak），每种检查两行空白的 ViewHitTest、XUI pointer down/up、点击处输入 X 与逐字节 Undo。Markdown 两模式核对当前 SourceStore，Rich/代码/HardBreak 核对当前树的纯文本，语义复制保留其段落结束符。Windows 和 Linux View/Editor 编译运行同一共享用例，不使用第二个内容模型。

Windows 专属 `native_boundary_affinity_pixels` 在真正 XGE 图形帧中使用 Arial 20，八类控制 × 有无 40 号空 Text × 宽 120→18→120，对照 AV+控制 Text 与 A/V/控制（CRLF 跨节点）的结构；两侧 BEFORE/AFTER、真实下一插入节点的字号/高度及整张 520×160 RGBA8 相同，真实蓝色文字像素大于 20。文字和下划线都有真实像素。Linux 无窗口代理运行共享几何/编辑/内存矩阵，不能代替 Linux 真实字体或 IME 证明。

当前已完成证据位于 `artifacts/xui-document-rebuild/`，均退出码零：

- `validation-2026-09-30-boundary-first-build.log`：产品变更后显式默认 DLL 重建；此后只增强测试及文档，产品源码未改动。
- `validation-2026-09-30-boundary-final-renderer2.log`：最终共享 Renderer（含最后新增的未提交行）与 Windows 原生新专项、完整 Renderer 回归。
- `validation-2026-09-30-boundary-first-editor.log`：33 种空行新构造及完整 Windows Editor 专项。
- `validation-2026-09-30-boundary-linux-renderer2.log`、`validation-2026-09-30-boundary-linux-view.log`：最终共享 Renderer/View/Editor ASan/UBSan，进程局部显式 `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`、`UBSAN_OPTIONS=halt_on_error=1`。第一次 Linux Renderer 日志在新增前缀测试前，最终 Renderer2 已重编译包含它；未用旧通过记录代替新用例。View 保留既有 date-picker maybe-uninitialized 警告。
- `validation-2026-09-30-boundary-layout-analyzer.log`、`validation-2026-09-30-boundary-renderer-analyzer.log`：最终产品的 GCC `-fanalyzer -Wall -Wextra -Werror -Wno-unused-function` 实际 -c 对象编译，日志为空；unused-function 仅针对共享内部头未使用静态函数。
- `validation-2026-09-30-boundary-final-webview-widget.log`、`validation-2026-09-30-boundary-final-web-provider.log`、`validation-2026-09-30-boundary-final-web-render.log`：当前产品重新构建可选 DLL；Windows 基础控件布局/焦点、十二轮生命周期、双实例和隔离进程恢复，以及 Document 内部 KaTeX/Mermaid/HTML、DocView/MessageList 更新/主题/错误卡片、解码/裁切恢复、三次有界重启和离线 PNG/沙箱通过。500×350 深色像素仍为 135/426/1233；通用公开 WebView 维持 Windows 基础范围。

完整 Windows 发布套件 `boundary-final-suite.log` 已退出码零，最终证据见本节末段。隐形控制/SHY、PS 段落输入、普通跨 run 软换行 affinity、联合排版上限、字素内字体/位移、连字/复杂脚本/Bidi、真实平台/IME/读屏/物理 DPI 与其余设计工作包未完成，整体目标继续。

当前最终完整 Windows 套件 `artifacts/xui-document-rebuild/validation-2026-09-30-boundary-final-suite.log` 退出码零，包含 Core、652 项 CommonMark 解析/源码/原生往返（非 HTML conformance）、当前默认 DLL、283 项公开导出且无旧 Rich API、Renderer（含新前缀/真实 Arial 像素）、Editor（含新两行 pointer 输入/Undo）、样式/规模/fractional、表格、MessageList 和原生同步/异步/缺图绘制。Linux 当前 View/Editor ASan/UBSan `validation-2026-09-30-boundary-linux-view.log`、当前可选 WebView2 控件/provider/离线渲染的三份 boundary-final-web*.log 亦均退出码零；Document 内部公式/Mermaid/HTML 通道保留，通用 WebView 不扩展公开通信或其他平台后端。最终 `git -c core.safecrlf=false diff --check` 通过。当前证据只覆盖本批和既有回归；长期目标保持进行中，普通跨 run 软换行 affinity、隐形控制/SHY、完整复杂 shaping/Bidi/字体回退、CST 和其余设计验收继续推进。

## 普通软换行的跨节点 affinity、行尾命中与编辑导航（2026-10-01）

产品只改变 `src/xui_document_renderer.c` 和 `src/xui_document_editor.c` 的私有实现。跨 Text/颜色/字号的普通软换行以共享逻辑基线识别行，并把真实字素终点的 AFTER 定位到同一段落内的下一插入位置；原始字素内部位置仍吸附到该字素端点。零长度 Text 若还在上行，AFTER 随下行基线移动但保留自己的字号；已独占新行或位于段落开头/强制控制后的空节点继续拥有可输入行。表格单块内的段落/Cell 不互相捕获 EOF。前缀查询使用实际下游光标底部判断目标行是否已提交。

普通软换行行尾命中返回 BEFORE，避免再查光标时从点击行跳到下一行。编辑器 Home/End/Shift-End、上下和翻页保存视觉命中的 affinity，水平逻辑移动和 Ctrl 文档首尾仍用 AFTER。极大横坐标的减法可能将不同字符距离舍入成相同值，行尾命中按真实视觉边缘打破平局。公开 API、正文树/字节、SOURCE 物理行目录和历史协议未改变。

新增共享 `test_xui/xui_document_soft_wrap_cases.h`：完整/颜色拆分、20/40 正文字号、40 号空 Text 四种文档与宽 200→27→12→200，独立比较 BEFORE/AFTER、真实下一插入位置、HitTest/GetCaretRect 行一致、空节点删除/ChangeSet 更新/Undo，三个段落分布于两个 Cell 的 EOF 范围。73725 字节长段落在 8190 字节边界冷查下游光标，必须延伸初始未提交行、shaping 小于 32768 字节、匹配完整排版；首屏 shaping 小于 10000 字节。共享 `document_soft_wrap_editor` 覆盖 Rich 完整/颜色拆分与解析得到的 Markdown VISUAL，真实 XUI pointer down/up、输入 X/精确 Undo，Home/End/Shift-End、Up/Down/PageUp/PageDown 和 Ctrl+Home/End。通过实际 IME 候选矩形回调核对光标视觉行，这是候选几何/合成事件证明，不是平台 IME 实机验收。

Windows 真正 XGE frame 的 `native_soft_wrap_pixels` 使用 Arial 20，普通、空 40 号 Text、后续 40 号正文三类与宽 120→35→18→120，对照完整文字节点和 A/V 拆分；比较所有 BEFORE/AFTER、行尾命中以及整张 520×160 RGBA8。最终 Renderer2 还要求每张图有超过十个真实蓝色及红色文字像素，避免两个空白画面相等。末尾强制控制、尾行、跨节点字素与既有 Editor 行为继续执行，没有放宽性能门槛或改变强制换行契约。

当前证据位于 `artifacts/xui-document-rebuild/`：

- `validation-2026-09-30-soft-affinity-before.log`：改动前行尾命中/GetCaretRect 先失败；`validation-2026-10-01-soft-affinity-editor-before2.log`：编辑器 End 导航先失败。两者退出码一，是修复前证据。
- `validation-2026-10-01-soft-affinity-frozen-build.log`：最终产品显式默认 DLL 构建退出码零，此后只增强测试/文档，未改变产品源码。
- `validation-2026-10-01-soft-affinity-final-suite.log`：完整 Windows 发布套件退出码零，包含 Core、652 项 CommonMark 解析/源码/原生往返（不是 HTML conformance）、实际 DLL、283 项公开导出且无旧 Rich API、当前共享软换行及扩展导航、真实 Arial 像素、样式/规模/fractional、表格、MessageList、原生同步/异步/缺图绘制。
- `validation-2026-10-01-soft-affinity-final-renderer2.log`：最终 Windows Renderer 退出码零；完整套件后只新增双颜色像素数断言，此专项重新编译并验证最终测试。`validation-2026-10-01-soft-affinity-final-editor2.log`：最终扩展 Windows Editor 导航与全部既有专项退出码零。
- `validation-2026-10-01-soft-affinity-linux-renderer.log`：最终共享 Renderer（包括空节点删除/Undo 和长前缀）ASan/UBSan 退出码零。`validation-2026-10-01-soft-affinity-linux-view.log`：初始软换行 End/Up/Down 版本的 Linux View/Editor 退出码零；扩展 Home/Page/Ctrl 首尾的最终 View2 结果见本节末段。
- `validation-2026-10-01-soft-affinity-renderer-analyzer.log`、`validation-2026-10-01-soft-affinity-editor-analyzer.log`：最终产品实际 GCC -c 对象编译，`-fanalyzer -Wall -Wextra -Werror -Wno-unused-function` 均退出码零、日志为空。unused-function 只按既有 UI 约定抑制公共内部头未用静态函数。
- `validation-2026-10-01-soft-affinity-webview-widget.log`、`validation-2026-10-01-soft-affinity-web-provider.log`、`validation-2026-10-01-soft-affinity-web-render.log`：当前产品可选 WebView2 DLL 重新构建、Windows 基础布局/焦点/十二轮生命周期/双实例/隔离浏览器进程恢复、Document 内部 KaTeX/Mermaid/HTML、DocView/MessageList 更新和主题/错误/解码/裁切/三次恢复上限、离线 PNG/HTML 沙箱全部退出码零；500×350 深色像素为 135/426/1233。通用 WebView 保持 Windows 基础功能，其他平台后端留空；内部渲染通道保留。

整体目标继续进行。R1 的隐形控制/SHY、PS 段落输入、shaping 范围与字素内部字体/位移、连字/复杂脚本/Bidi/字体回退，K2 完整 CST 以及真实平台/IME/读屏/物理 DPI 等剩余设计验收不以本批结果替代。
最终补验 `artifacts/xui-document-rebuild/validation-2026-10-01-soft-affinity-linux-view2.log` 退出码零，包含最终 Home/End/Shift-End、Up/Down/Page 和 Ctrl 文档首尾矩阵。Linux Renderer/View2 均为当前产品源码及最终共享测试重新编译，显式 `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`、`UBSAN_OPTIONS=halt_on_error=1`，未出现 sanitizer 错误；View2 保留既有 date-picker 警告。完整 Windows 发布套件、最终 Renderer2（含新增真实双颜色像素数）/Editor2、两项严格实际 GCC analyzer、当前可选 WebView2 控件/provider/离线渲染均已退出码零。产品源码自 frozen-build 后未修改，最后差异检查通过。本批关闭普通跨 run 软换行亲和及相关编辑导航缺口，整体重构目标继续进行。
## 格式字符显示投影与默认软连字符验收（2026-10-01）

修复前 `artifacts/xui-document-rebuild/validation-2026-10-01-format-before3.log`（退出码一）用旧 DLL 证明 A＋格式字符＋V 的光标宽度不等于 AV：直接送给 shaper 的隐形字符破坏实际字距。新增 `xuiInternalTextCopyDisplay`、`xuiInternalTextShapeProjection` 和 `xuiInternalTextDisplayGraphemes` 为私有共同工具；Document 使用普通文本已有的删除投影/源码坐标整形实现，没有第二套持久内容对象，也没有旧 Rich API 包装。四种字符的 source byte、revision、历史、保存及 SOURCE 物理行目录不变。

Renderer 的 Text、代码和 Markdown VISUAL/SOURCE 均用实际显示字串整形。联合绘制组保留可选的每片段显示结束偏移，颜色范围按显示字节发布，隐藏片段不把不同字号或上下标当成有字形的字体边界；它的输入/行高度量仍由实际样式保留。显示字符删除后形成的组合字素受共同字素投影保护。联合组之外的同 run 绘制也使用删除后的字串；DrawText-only 宿主已测。所有显示偏移都是缓存，不参与 Document 编辑/复制。

SHY 的 '-' 在自身字体下测量并按需缓存，断行候选计入该宽度，仅选中 SHY 断点时加入实际行字串、几何、命中和装饰；改宽回到整行后消失。20/40 字号都实测，测量失败返回错误、清理后可重试。一个隐形的 40 号 SHY 会在选中断行后增加单独字体绘制组，因此容量计入每行可能新增的终端连字符组；四次连续样式连字符专项与 ASan 验证该路径。显示字符串按真实投影字节分配，偏移表纳入 Renderer 缓存计费，释放/重排/失败均释放它。产品没有公开 API 或原生存储格式变化。

共享 `test_xui/xui_document_format_cases.h` 检查四字符 × Rich 完整/颜色拆分、代码、Markdown SOURCE、Markdown VISUAL 共二十种构造，A/字符/V 的 AV 字距及原位置，并逐个检查真实绘制字符串不含格式字符、绘制后源码/节点字节未改。完整/拆分 AV＋SHY＋AV 在宽 100→27→100 中，窄行 BEFORE x=27/y=0、AFTER x=0/y=24，连字符点击返回原终点 BEFORE；窄行绘一个 '-'，宽行不绘。四个 40 号 SHY 在宽 37 下形成四次连字符断行及末行，逐行检查字号/光标和四个实际 '-'。四字符夹在 e/组合重音之间时，完整/拆分节点在宽 100/7 下仍保持一个显示字素。中/ZWSP/文可在宽 10 分两行，WJ/FEFF 保持一行；DrawText-only 的同 run 和连字符 shape 一次失败后的重试另有检查。

共享 Editor 覆盖十六种构造（四字符 × Rich、Markdown VISUAL/SOURCE/LIVE），原字节偏移处插入 X、Undo、整字符 Backspace、再次 Undo；直接核对当前 SourceStore 或 Rich 纯文本字节。Windows `native_format_pixels` 在真正 XGE frame 使用 Arial：四字符的 A/字符/V 与 AV、20/40 号 SHY 与显式连字符＋HardBreak 比较光标及整张 520×160 RGBA8，并检查真实蓝色像素超过二十。它验证这些样本的真实字体路径，不证明所有脚本或语言。

当前最终产品从 `validation-2026-10-01-format-frozen-build.log` 后未改动。`format-final-renderer.log`、`format-editor.log`、`format-linux-renderer-final.log`、`format-linux-view.log`、layout/renderer/text 三项实际 -c 严格 GCC analyzer 均退出码零，日志均位于 artifacts/xui-document-rebuild 并带 validation-2026-10-01- 前缀。Linux 显式启用 ASan leak/halt 与 UBSan halt，View 保留已有 date-picker 警告；Linux 最终 Renderer 包含最后新增的 DrawText-only 与 shape 失败重试，未用旧通过记录替代最终测试。

普通文本投影、58 项断行、Unicode 数据集/规模、13054 项控件显示及两类代理 shaping 回归亦均退出码零：`format-text-projection.log`、`format-text-linebreak.log`、`format-text-break-index.log`、`format-text-consumers.log`。UAX14 15.0 为 7653 项通过加一项已验证的上游例外，UAX29 15.1 为 1187 项通过；未修改上游 libunibreak 或数据。标准依据为 [UAX #14 revision 51](https://www.unicode.org/reports/tr14/tr14-51.html)，尤其 §5.4：SHY 的可见形式依赖语言，当前 '-' 是既有普通文本策略，不能冒充多语言完整实现。

完整 Windows `validation-2026-10-01-format-final-suite.log` 退出码零，覆盖当前 Core、652 项 CommonMark 解析/源码/原生往返（不是 HTML conformance）、DLL、283 项公开导出且无旧 Rich API、新 Renderer/Editor、样式/规模/fractional、表格、MessageList 和原生同步/异步/缺图绘制。当前可选 WebView2 的 `format-webview-widget.log`、`format-web-provider.log`、`format-web-render.log` 也退出码零：基础 Windows 控件生命周期/双实例/隔离浏览器进程恢复，以及保留的内部 KaTeX/Mermaid/HTML、DocView/MessageList 更新、主题、错误/解码/裁切/三次恢复上限与离线 PNG/沙箱。其余平台 WebView 后端及通用对外通信未扩展。

本批补齐四种格式字符与默认 SHY 路径，R1 仍是部分实现。候选连字符的完整上下文整形、语言特定连字符/拼写策略、其余格式字符、PS 段落输入、联合 shaping 超限、字素内部字体/位移、跨字素连字/复杂脚本/Bidi/字体回退，以及 K2 完整 CST、真实平台/IME/读屏/物理 DPI 和其他设计验收继续。长期目标保持进行中。


## 软连字符候选的上下文测量验收（2026-10-01）

修复前的 `artifacts/xui-document-rebuild/validation-2026-10-01-hyphen-context-before.log` 退出码一，复现独立 AV 宽 17、独立 '-' 宽 10，而实际 AV- 因负字距宽 25 时，行宽 25 错误拒绝 SHY 候选。只在选中后的绘制阶段重新整形无法补救一个提前被排除的候选。

当前可用的联合排版路径在溢出时，按实际行起点、显示投影、字体/上下标跨度及候选终端 '-' 测量宽度。只测量可能结束该行的候选，拒绝后回查更早正常断点；隐藏片段及空 Text 之后的 SHY 也可让原本溢出的可见字形重新整形后落入行宽。非文字对象保留已测宽度，强制控制和其他可见字形限制前瞻范围。候选 scratch、字符串及映射均为临时布局数据，不改源码、原偏移或公开 API；真实整形错误从候选、绘制及收敛检查传播，未发布部分宽度。

共享 `test_xui/xui_document_format_cases.h` 新增六类样本 × Rich 完整/颜色拆分/解析得到的 Markdown VISUAL，共十八组：负字距 25、正字距 30、独立连字符宽 2 且未断前缀宽 17 在行宽 14 中可断、最新候选过宽时选择较早 SHY、最新候选可容纳时优先选中、宽行不生成连字符且不做终端候选整形。逐个核对原始终点的 BEFORE/AFTER、点击命中、实际绘制连字符数量和 Markdown 源码字节。候选测量、pending 绘制整形和收敛复核三处分别注入一次 OOM，改宽布局返回错误后在同一 Renderer 重试，光标仍为 x=25/y=0；既有四格式字符、四个 40 号 SHY、DrawText-only 基础删除投影及真实 Arial 像素回归继续通过。

本批显式默认 DLL `validation-2026-10-01-hyphen-context-build.log` 后产品源码未再改变。以下最终日志均位于 `artifacts/xui-document-rebuild`，退出码零：

| 验证 | 本批最终日志 |
| --- | --- |
| Windows Renderer（十八组及三处失败重试、真实 Arial 像素）/Editor | `validation-2026-10-01-hyphen-context-renderer-final.log`、`validation-2026-10-01-hyphen-context-editor.log` |
| Linux Renderer/View-Editor ASan/UBSan 与实际 -c 严格 GCC analyzer | `validation-2026-10-01-hyphen-context-linux-renderer.log`、`validation-2026-10-01-hyphen-context-linux-view.log`、`validation-2026-10-01-hyphen-context-layout-analyzer.log` |
| 当前可选 DLL 的 Windows 基础 WebView、Document 内部 provider、离线 PNG | `validation-2026-10-01-hyphen-context-webview-widget.log`、`validation-2026-10-01-hyphen-context-web-provider.log`、`validation-2026-10-01-hyphen-context-web-render.log` |

Linux 显式启用 `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1` 和 `UBSAN_OPTIONS=halt_on_error=1`，没有 sanitizer 错误；View 保留已有 date-picker 的 maybe-uninitialized 警告。WebView 复验包含十二次生命周期、双实例、隔离进程失败/重建，内部 KaTeX/Mermaid/HTML 的 DocumentView/MessageList 更新、主题、失败/解码/裁切与三次重启预算；离线三张 500×350 PNG 及 HTML 沙箱通过。通用控件仍仅实现 Windows 基础功能，Document 内部通道保留。

本批在可用联合路径内补齐默认 SHY 候选上下文。既有 8 KiB/64 run/512 行/八次迭代上限仍在；超限、不支持的 cluster 映射、DrawText-only 等回退的候选整形、语言特定断词/拼写、复杂脚本/Bidi/字素内部字体策略等未据此宣称完成。R1 和整体重构继续进行，上一批的待办表述保留为当时状态。

完整 Windows 发布套件 `artifacts/xui-document-rebuild/validation-2026-10-01-hyphen-context-final-suite.log` 随后退出码零，使用当前 DLL 与最终共享测试，覆盖 Core、652 项 CommonMark 解析/源码/原生往返（不是 HTML conformance）、283 项公开导出与无旧 Rich API、Renderer/Editor、样式/规模/fractional、表格、剪贴板/对象来源编辑、MessageList 和原生同步/异步/缺图像素。当前默认/可选 DLL、Windows/Linux 专项及内部网页渲染均已完成；最后差异检查通过。整体重构目标保持进行中。

## 联合排版规模验收（2026-10-01）

本批移除联合排版的 8192 显示字节、64 run、512 行硬阈值，动态分配行切点/宽度/pending 组并检查整数界限。段落按需前缀、后续增长和完整排版采用同一联合整形路径。扩展整次成功前保留原尾行绘制组及片段，在中间或后续增长失败时释放新增组/run、恢复原光标/绘制，再允许重试。八次收敛限制、不支持的 cluster 映射、字素内字体策略、DrawText-only 精确路径以及复杂脚本/Bidi 等仍未完成。

共享 `xui_document_joint_scale_cases.h` 在 Windows/Linux 使用可重复 AV 字距代理，覆盖 8999 字节、105 个颜色节点、600 行和 89999 字节按需段落。Rich 完整/拆分和解析 Markdown 比较几何与绘制，105 节点用 Rich 对照；改宽前后源字节保持不变。冷前缀和 8192/16385/32768 原偏移的查询保持按需材料化。三次扩展 OOM 分别在新 run、seed 和后续增长，原光标及包含字串/颜色/位置的绘制 journal 必须完全相同，然后重试通过。Windows `native_joint_scale_pixels` 在真正 XGE 图形帧用 Arial 比较 9000/105/90000 字节样本的完整与拆分渲染，三次宽度切换后的 520×160 全部 RGBA8 相同、蓝色文字超过二十像素，105 节点还要求红色超过二十像素，长段落核对深处光标。MD4C 会去掉段落尾部空格，因此跨模式 fixture 以 V 结尾，比较相同语义文字；不以错误的尾空格 EOF 期望代替断言。

新增 `xui_doc_renderer_stats_t.iTextRunBytes`，只累计成功新增 run 的整形输入（含私有换行测量文本）；`iShapedBytes` 继续记录联合/逐行重整形的实际总量。两者均累计，失败前已成功的 run 也可计入，重新材料化再计数；驻留量用 `iLayoutCacheBytes`，首次完整断行投影的源码读取不由该输入统计代表。结构增大，宿主与 DLL 必须用同一新版头文件重新编译。冷前缀/续排保留原新增输入/片段范围预算，另限定实际 shaping 小于四倍输入；热改宽要求输入零增长并按缓存输入限制行整形工作。100000 字节 View 冷帧/改宽总 shaping 预算、热帧零新增、逐帧耗时和精确几何对照仍保留。没有把重复整形当成新增正文输入，也没有去掉实际工作量检查。

初版 `validation-2026-10-01-joint-scale-frozen-build.log` 构建退出码零。首次完整 Windows 套件在 MessageList 的旧续排总 shaping 预算停止；`message-diagnostic.log` 记录第一次实际增长 49166 字节，超过 90000 字节正文的一半。新增只读 `xuiMessageListGetNodeDocumentRenderStats`，保留原新增输入范围预算并约束实际 shaping 小于四倍新增输入，统计读取不得增加 shaping；NULL/错误大小/缺消息/未绑定/解绑等错误不得改输出。字体替换重建 Renderer 后用新实例输入计数，不能与旧实例做差。

`message3.log` 进一步记录 16 次改宽实际 shaped=2228832，超过原 `bytes*16` 预算。这次没有放宽预算：宽度变化与对象失效保留可复用的原 run、Unicode 投影与精确字体/位移跨度的初始宽度缓存，重建实际行和对象；run 切段或字体不匹配时重新构造。新增 seed 纳入缓存字节预算，并在释放、逐出或续排回滚时清理。`final3-message.log` 的文本改宽 shaped=786528，原预算通过；另断言反复改宽新增 run 输入为零。该日志在尚未修复的图片尺寸变化预算停止；资源失效随后采用同一复用策略，最终结果见本节末段。

最终默认产品 `validation-2026-10-01-joint-scale-final4-build.log` 显式构建退出码零，此后未改产品源码/头文件。以下是初版阶段的通过证据，后续缓存改动再以本节末尾的完整结果验收；日志均在 `artifacts/xui-document-rebuild/`：

- `validation-2026-10-01-joint-scale-frozen-linux-renderer.log`、`validation-2026-10-01-joint-scale-frozen-linux-view.log`：重新编译最终产品及共享测试，ASan/UBSan 显式开启 detect_leaks/halt_on_error，无 sanitizer 错误；View 仍有既有 date-picker maybe-uninitialized 警告。
- `validation-2026-10-01-joint-scale-final-layout-analyzer.log`：实际 `-c -fanalyzer -Wall -Wextra -Werror -Wno-unused-function`，日志为空；unused-function 仅按既有 UI 约定抑制共同内部头未用静态函数。
- `validation-2026-10-01-joint-scale-scale3.log`：最终统计结构的大文档专项，100000 字节 View 冷帧 shaped=16394、改宽 shaped=16394，热帧预算及百万字节量级多段 Editor 保留。这里只是当前代理/机器的一次性能结果，不作为跨平台字体性能证明。`scale.log`/`scale2.log` 是修正统计前的失败记录。
- `validation-2026-10-01-joint-scale-webview-widget.log`、`validation-2026-10-01-joint-scale-web-provider.log`、`validation-2026-10-01-joint-scale-web-render.log`：用最终统计头重建可选 DLL，基础控件双实例/生命周期/隔离进程恢复，保留的内部 KaTeX/Mermaid/HTML、DocView/MessageList 更新、主题、错误/解码/裁切/三次恢复上限，三张 500×350 离线 PNG 和 HTML 沙箱通过。

最终完整 Windows 发布套件和差异检查结果见本节末尾。通用 WebView 范围仍为 Windows 基础控件，Document 内部通道保留，整体重构目标仍未完成。

最终源码/头文件 `validation-2026-10-01-joint-scale-final4-build.log` 后未修改。以下最终补验全部退出码零：

| 验证 | 最终日志 |
| --- | --- |
| Linux Renderer、View/Editor、MessageList，ASan/UBSan | `validation-2026-10-01-joint-scale-final4-linux-renderer.log`、`validation-2026-10-01-joint-scale-final4-linux-view.log`、`validation-2026-10-01-joint-scale-final4-linux-message.log` |
| Windows MessageList：统计 API、前缀增长、缓存和阅读锚点 | `validation-2026-10-01-joint-scale-final4-message.log` |
| layout/renderer/MessageList 实际 -c 严格 GCC analyzer | `validation-2026-10-01-joint-scale-final3-layout-analyzer.log`、`validation-2026-10-01-joint-scale-final4-renderer-analyzer.log`、`validation-2026-10-01-joint-scale-final4-message-analyzer.log` |
| 当前可选 DLL 的基础 WebView、内部 provider、离线 PNG/沙箱 | `validation-2026-10-01-joint-scale-final4-webview-widget.log`、`validation-2026-10-01-joint-scale-final4-web-provider.log`、`validation-2026-10-01-joint-scale-final4-web-render.log` |

Linux 三项均从最终产品及共享测试重新编译，显式 ASan leak/halt 与 UBSan halt，无 sanitizer 错误；两个全 UI 编译保留既有 date-picker 警告。MessageList 文本/图片消息的 16 次改宽 shaped=786528/655376，仍小于原 `bytes*16`；图片增大/缩小的 shape-growth=122883，仍小于原 `bytes*2`。两条路径均新增检查 run 输入零增长，统计读取不触发 shaping，原阅读位置、滚动方向、绘制、复制、历史及生命期检查仍在。第一版/诊断及中间失败日志只代表当时状态，不替代最终验收。完整 Windows 发布套件结果见下段。

最终完整 Windows 发布套件 `artifacts/xui-document-rebuild/validation-2026-10-01-joint-scale-final4-suite.log` 退出码零：最终默认 DLL 与新版统计/API 头文件、Core、652 项 CommonMark 解析/源码/原生往返（不是 HTML conformance）、284 个公开 Document 函数和无旧 Rich API、新共享联合规模/回滚与真实 Arial 像素、Renderer/Editor、样式/规模/fractional、表格、剪贴板、对象来源、MessageList，以及原生同步/异步和缺图像素全部通过。原生输出 `native-smoke.png` 与 `native-live-smoke.png` 已重新捕获。最后差异检查通过，产品源码/头文件自 final4-build 后未改。整体目标保持进行中，本批不替代未收敛/cluster/DrawText-only、语言策略、完整复杂 shaping/Bidi/字体回退、CST 或实际平台/IME/读屏/物理 DPI 的剩余验收。

## 联合断行未收敛验收（2026-10-01）

旧 DLL 在 `artifacts/xui-document-rebuild/validation-2026-10-01-joint-convergence-before4.log` 以退出码一复现：两种上下文断行循环，八次重排耗尽后仍成功返回孤立宽度，第一处行尾 AFTER 光标没有进入下一行。更早 before/before2/before3 是用例编译修正记录，不是产品失败证据。

产品在八次快速全局重排耗尽后，以有限的从左到右候选测量选择断点；候选用实际行字串与字体/位移、对象及 SHY 终端上下文。首词不能容纳时采用字素安全的应急断点，不可拆单元可溢出，每行保证向前。pending 行字符串、宽高/基线及切点一次发布，最终行组装使用同一切点。分配与真实整形失败返回错误，不发布部分宽度。该贪心备用路径可增加候选前缀整形，不保证任意非单调字宽下的最优断点，不据此宣称完整复杂 shaping 或统一性能。

新共享 `xui_document_joint_convergence_cases.h` 覆盖 5 字节循环与 41 字节慢收敛、Rich 完整/颜色拆分和解析 Markdown，各三轮改宽；逐行检查 3/21 个实际绘制字串及坐标/宽度、全部行尾 BEFORE/AFTER、命中与 EOF，Markdown 原源码不变。最终慢收敛代理对长跨度仅给最后两个字符各 11 advance，其余为零，合法且无需浮点近似，保证候选失败前已有八个长跨度整形。两次故障分别在第一个四字节候选和十九个四字节探测之后的首个最终 pending 两字节行；OOM、计数耗尽、revision 不变及同 Renderer 重试都必须通过。中间 renderer/renderer2 日志使用的比例浮点代理未可靠达到预定故障阶段，随后修正 fixture，没有放宽阶段、光标或绘制断言。最终 Windows/Linux 日志两次均打印 large=8，第二次 four=19。

以下均退出码零，日志在 `artifacts/xui-document-rebuild/`：

| 验证 | 最终日志 |
| --- | --- |
| 默认 DLL 显式构建、实际 -c 严格 GCC analyzer | `validation-2026-10-01-joint-convergence-build.log`、`validation-2026-10-01-joint-convergence-layout-analyzer.log` |
| 实际 DLL Renderer、新用例、652 项语料及既有真实 Arial 像素 | `validation-2026-10-01-joint-convergence-renderer3.log` |
| Linux Renderer、View/Editor、MessageList，ASan/UBSan | `validation-2026-10-01-joint-convergence-linux-renderer.log`、`validation-2026-10-01-joint-convergence-linux-view.log`、`validation-2026-10-01-joint-convergence-linux-message.log` |
| 可选 DLL、基础 WebView、内部 provider、离线渲染 | `validation-2026-10-01-joint-convergence-webview-widget.log`、`validation-2026-10-01-joint-convergence-web-provider.log`、`validation-2026-10-01-joint-convergence-web-render.log` |

实际编译 analyzer 使用 -fanalyzer/-Wall/-Wextra/-Werror，日志为空；共同内部头的 unused-function 沿用现有抑制。Linux 从当前产品与共享用例重新编译，显式 detect_leaks/halt_on_error，无 sanitizer 错误；View 和 MessageList 编译保留既有 date-picker 警告。基础 WebView 的十二次生命周期、双实例与隔离进程重建、DocumentView/MessageList 的内部 KaTeX/Mermaid/HTML、主题和失败/恢复预算、三张 500×350 离线 PNG 与 HTML 沙箱通过。通用 WebView 的对外范围继续为 Windows 基础承载，Document 内部通道保留。

产品源码/头文件自本节 build 后未改。最终完整 Windows 发布套件 `artifacts/xui-document-rebuild/validation-2026-10-01-joint-convergence-final-suite.log` 退出码零：当前默认 DLL、Core、652 项 CommonMark 解析/源码/原生往返（不是 HTML conformance）、284 个公开 Document 函数及无旧 Rich API、新循环/慢收敛和两阶段故障重试、既有联合规模/真实 Arial 像素、Renderer/Editor、样式/规模/fractional、表格、剪贴板和对象来源、MessageList、原生同步/异步/缺图像素全部通过。原生 `native-smoke.png` 和 `native-live-smoke.png` 重新捕获；既有实际性能预算没有放宽。最后差异检查通过。

R1 和整体目标保持进行中；本批只补齐可用联合路径内的未收敛处理，cluster 映射、DrawText-only、语言/字素字体/复杂脚本/Bidi、完整 CST 和真实平台/IME/读屏/物理 DPI 等不能由本批替代验收。

## 多字素连字 cluster 与 Markdown 空格式删除验收（2026-10-01）

以下日志位于 `artifacts/xui-document-rebuild/`。旧 DLL 的 `validation-2026-10-01-ligature-before2.log` 以退出码一复现合并 `fi` cluster 内部没有等分光标停点。本批提供可选 XRT 所有权的 `xui_text_caret_t` 元数据，在 Document 的 run、删除格式字符后的显示投影以及实际行整形中按 Unicode 字素归一化；未提供元数据时采用明确的等分约定，未推断字体特定停点。归一化前验证连续覆盖的原始 cluster、严格源码偏移、停点完整性、范围、顺序及有限值；失败不发布部分片段。原生 CRLF 像素检查在 renderer3 阶段失败，原因是 XGE 跳过 CR 造成 cluster 覆盖缺口；XUI 适配器现在将 CR 纳入 LF 强制换行 cluster，修复后的 renderer4 已通过，最终套件另列于下文。

共享 Renderer 用例是两种停点策略×五种样本×六种文档/模式形态，共 60 个 fixture。比较宽度切换后的光标 BEFORE/AFTER、EOF、命中、选区和实际绘制文本/宽度；正文另外覆盖正常词边界和 ASCII 应急断行后的上下行亲和与绘制。样本含 fi/ffi、组合字符+i、多字节字符及删除 WJ 的显示投影。非法元数据含 NAN、cluster 端点、缺停点、UTF-8 内部偏移、坐标逆序、重复偏移和超宽坐标，要求 INVALID_STATE、revision 不变及同 Renderer 重试；代理分配停点后返回 OOM 的案例也要求成功重试。公共原始整形接口保留元数据，ShapeFree 后两个数组及计数均清空。

Editor 是两种策略×前四种样本×五种形态，共 40 个 fixture：Rich 完整/拆分及 Markdown VISUAL/SOURCE/LIVE。代理安装后创建独立 context（XUI 会复制 proxy 表），从方向键得到的范围必须复制出一个完整 Unicode 字素，通用 caret rect 使用等分或代理位置；指针命中、Backspace 后正文及 Undo 后完整 Markdown 源码均核对。代理字体一致，用例不等同真实字体 GSUB/GDEF、语言断词或 Bidi 验收。

Editor6 和 `validation-2026-10-01-ligature-core-before2.log` 另以退出码一证明删除 `**f**iX` 中 f 返回 UNREPRESENTABLE。现在全部删除 Text 内容时回写改变块并继续完整语义核对。Core 11 个真实带标记节点×两种替换 API，核对精确结果源码、合法返回光标、Undo/Redo 和 CRLF 邻块保留。四种源码×两 API 的逐分配预算尝试包括首次成功各 52 轮（有尾文字）或 27 轮（整段清空）；失败只允许 OOM，核对原 revision、源码、身份树、语法记录、历史和全部分配释放。首版删除线 fixture 的相邻字母不满足 parser 定界规则，后改成可解析样本并增加原节点标记/文本长度断言，不降低正确性要求。

最初 Linux Renderer 日志的 ASan 外来内存头访问来自 fixture 的普通 calloc 数组交给 XRT 释放；模型已改成 xrtCalloc，最终 Linux Renderer 无 sanitizer 错误。before（Parse API）、SOURCE ROOT 和 Editor 原 context 的代理复制等修正日志也不作为产品回归证据。

相邻格式补充前的阶段验收均退出码零，最后修复及最终证据另见下文：

| 验证 | 日志 |
| --- | --- |
| 默认 DLL 显式构建 | `validation-2026-10-01-ligature-final-build.log` |
| Core + 652 CommonMark，Linux ASan/UBSan | `validation-2026-10-01-ligature-final-linux-core.log` |
| Renderer，Linux ASan/UBSan | `validation-2026-10-01-ligature-final-linux-renderer.log` |
| View/Editor，Linux ASan/UBSan | `validation-2026-10-01-ligature-final-linux-view.log` |
| text/Markdown/layout/proxy 实际 -c 严格 GCC analyzer | `validation-2026-10-01-ligature-final-text-analyzer.log`、`validation-2026-10-01-ligature-final-markdown-analyzer.log`、`validation-2026-10-01-ligature-final-layout-analyzer.log`、`validation-2026-10-01-ligature-final-proxy-analyzer.log` |
| 当前可选 DLL / 基础 Windows WebView | `validation-2026-10-01-ligature-final-webview-widget.log` |

analyzer 使用 -fanalyzer/-Wall/-Wextra/-Werror，四项日志为空；沿用共享内部头的 unused-function 和 proxy/text 的 unused-parameter 抑制。Linux 显式开启 ASan leak/halt 和 UBSan halt；全 UI 编译沿用 date-picker 既有警告。完整 Windows、Linux MessageList 及 Document 内部 provider/离线渲染的最终结果补录于下文。公开整形结构需要宿主、proxy 和两个 DLL 同版重编译；通用 WebView 仍为 Windows 基础能力，Document 内部通道保留，整体目标未完成。

最后的相邻格式补查在 `validation-2026-10-01-ligature-neighbor-before.log` 以退出码一复现 `**a**f**b**` 删除 f 后仍无法写回。空 run 不能只跳过输出：它还会影响标记邻居、相同格式合并和共有外层格式。序列化现在在私有持久子节点序列中统一过滤空 Text，原语义树和全部既有定界符/等价规则保留。新四案例×两 API 核对粗体/斜体转换、粗体合并及共有粗体/内部斜体，Undo 精确恢复原源码。失败 sweep 扩为六案例×两 API，含首次成功的预算尝试为 53、53、53、27、67、77 轮；新增的过滤序列分配也需原子失败、无泄漏。`validation-2026-10-01-ligature-neighbor-markdown-edit-analyzer.log` 的实际 -c 严格 analyzer 退出码零、日志为空，最终源码及扩展用例的整体结果如下。

最终产品显式构建 `validation-2026-10-01-ligature-neighbor-build.log` 退出码零，其后产品源码/头文件未改。以下重新编译/运行全部退出码零：

| 验证 | 最终日志 |
| --- | --- |
| Windows 完整发布套件 | `validation-2026-10-01-ligature-neighbor-suite.log` |
| Linux Core + 652 CommonMark | `validation-2026-10-01-ligature-neighbor-linux-core.log` |
| Linux Renderer、View/Editor、MessageList | `validation-2026-10-01-ligature-neighbor-linux-renderer.log`、`validation-2026-10-01-ligature-neighbor-linux-view.log`、`validation-2026-10-01-ligature-neighbor-linux-message.log` |
| 最终可选 DLL / 基础 WebView、内部 provider、离线 PNG/HTML 沙箱 | `validation-2026-10-01-ligature-neighbor-webview-widget.log`、`validation-2026-10-01-ligature-neighbor-web-provider.log`、`validation-2026-10-01-ligature-neighbor-web-render.log` |
| 通用断行索引、显示投影、原生 proxy、Edit 契约 | `validation-2026-10-01-ligature-neighbor-text_break_index.log`、`validation-2026-10-01-ligature-neighbor-text_projection.log`、`validation-2026-10-01-ligature-neighbor-proxy_xge.log`、`validation-2026-10-01-ligature-neighbor-edit_contract.log` |

Linux 四项显式 ASan detect_leaks/halt 与 UBSan halt，无 sanitizer 错误；全 UI 编译保留既有 date-picker 警告。五个修改产品 TU 的实际 -c 严格 analyzer 退出码零且日志为空，未再修改的 text/layout/Markdown/proxy 使用上表之前的 final-analyzer 日志，最后 writer 用 neighbor-analyzer 日志。四个新增未跟踪文件的 no-index whitespace 检查无输出（新增文件差异返回一）；最终 tracked diff whitespace 检查通过。

Windows 最终套件覆盖全部本批用例、Core、652 CommonMark 解析/源码/原生往返（不是 HTML conformance）、284 公开导出及无旧 Rich API、真实 Arial 像素、Renderer/Editor、样式/规模/fractional、表格、剪贴板/对象来源、MessageList 与原生同步/异步/缺图像素，重新捕获 native smoke PNG。消息 16 次改宽 shaping=786528/655376，原预算继续通过；通用文本显示投影另覆盖九个 DPI 分配失败恢复点。可选 DLL 的基础控件十二次生命周期/双实例/进程恢复、内部公式/Mermaid/HTML 的显示/测量/失败恢复/重启预算和三张 500×350 离线 PNG/HTML 沙箱通过。通用 WebView 范围仍为 Windows 基础能力，Document 内部通道保留。

本批验收完成，整体目标继续进行；真实字体 GSUB/GDEF/停点、无序 cluster/Bidi、字素内字体与其他回退路径、完整 CST 以及真正平台/IME/读屏/物理 DPI 不能由这些代理、Linux 无窗口或既有 Arial 像素用例替代。

## drawText-only 联合排版及裁剪错误验收（2026-10-01）

旧 DLL 的 `validation-2026-10-01-draw-only-before.log` 以退出码一复现 A/V 拆分在无 drawTextSpans 时失去跨节点字距：首字符 caret=10，完整视觉行应为 7。取消联合排版的可选 spans 守卫后，单色组调用一次 drawText 绘完整实际行；多色以不重叠字素视觉区间裁剪重复绘制同一字符串，跨节点组合字素由首个实际文字片段拥有颜色。无裁剪能力的多色组返回 UNSUPPORTED。这个回退不声称原生逐字形重叠着色精度，重复绘字次数按颜色区间增长。

`validation-2026-10-01-draw-only-clip-before.log` 退出码一证明入口 clipSet 报错被忽略。Renderer 现在返回裁剪查询/设置、绘字及恢复/清除错误，内部颜色裁剪也尝试恢复外部状态。八个注入点要求 INVALID_STATE、错误计数耗尽、非默认调用方裁剪保持或清空后无裁剪，并在同 Renderer 重试成功。shaper 在 AV 最终实际行分配 cluster 后返回 OOM 的回归要求失败、revision 不变及重试后 7 advance 和下一行 EOF=10/y24。

共享用例为 36 fixture×三次宽度：两种裁剪能力、AV/fi/组合字素、Rich 完整/同色拆分/多色拆分与 Markdown 三模式。核对全行字符串、宽度/坐标、颜色区间、字素归属、光标和拒绝路径；命中与选区矩形在完整发布套件结束编译 Renderer 后追加，随后独立 Windows Renderer 和 Linux Renderer 使用最终共享测试再编译。Windows 独立 drawText-only context 的实际 Arial 用例比较 9000 字节、105 颜色节点、约 90000 字节前缀/续排：两个回退表示的 520×160 RGBA8 全帧一致且实际蓝/红像素非零，宽窄切换与深处 caret 通过；比较双方不是不同绘字能力的逐 glyph 颜色等价证明。

最终产品显式构建 `validation-2026-10-01-draw-only-final-build.log` 后未修改源码/头文件，以下验收均退出码零；日志在 `artifacts/xui-document-rebuild/`：

| 验证 | 日志 |
| --- | --- |
| Windows 完整发布套件 | `validation-2026-10-01-draw-only-final-suite.log` |
| Windows 追加命中/范围查询后的 Renderer，含 652 语料与两种能力的实际 Arial | `validation-2026-10-01-draw-only-final2-renderer.log` |
| Linux Renderer 初版及八故障/OOM，ASan/UBSan | `validation-2026-10-01-draw-only-linux-renderer.log`、`validation-2026-10-01-draw-only-final2-linux-renderer.log` |
| Linux View/Editor、MessageList，ASan/UBSan | `validation-2026-10-01-draw-only-final-linux-view.log`、`validation-2026-10-01-draw-only-final-linux-message.log` |
| 最终可选 DLL / 基础 WebView、Document 内部 provider、离线 PNG/HTML 沙箱 | `validation-2026-10-01-draw-only-final-webview-widget.log`、`validation-2026-10-01-draw-only-final-web-provider.log`、`validation-2026-10-01-draw-only-final-web-render.log` |
| renderer/layout 实际 -c 严格 GCC analyzer | `validation-2026-10-01-draw-only-final-renderer-analyzer.log`、`validation-2026-10-01-draw-only-layout-analyzer.log` |

analyzer 使用 -fanalyzer/-Wall/-Wextra/-Werror（沿用共享内部头 unused-function 抑制），日志为空；Linux 显式 detect_leaks/halt 与 UBSan halt，已有完成项无 sanitizer 错误，全 UI 编译保留既有 date-picker 警告。追加查询后的最后 Linux Renderer 和最终差异检查结果补录于下文。

Windows 套件还保留全部旧规模、颜色/装饰、收敛/连字/格式、Editor、表格、剪贴板/对象来源、MessageList 与原生同步/异步/缺图像素；652 项为 CommonMark 解析/源码/原生往返，不是 HTML conformance。284 公开导出、无旧 Rich API、原生 smoke PNG 再次捕获，原预算未放宽。可选基础控件十二次生命周期/双实例/进程恢复，Document 内部 KaTeX/Mermaid/HTML/主题/失败恢复与重启预算、三张离线 PNG 和 HTML 沙箱通过；公开 WebView 范围未扩展。

最后 `validation-2026-10-01-draw-only-final3-linux-renderer.log` 用追加命中和范围断言后的最终共享测试重编译，以 ASan/UBSan 运行，退出码零、无 sanitizer 错误。它与 Windows final2-renderer 均覆盖全部 36 配置的 caret、命中与选区；final-suite 包含之前的全行、颜色、八故障/OOM 和原生像素版本，不能把后来追加的查询归入当时的 suite。最后 tracked diff whitespace 检查通过，新增未跟踪测试头的 no-index whitespace 无输出（差异返回一）。全部进程已成功结束，产品源码/头文件未再修改。

本批完成可用联合路径上的 drawText-only 回退及裁剪错误处理；其他 cluster/字体映射回退、完整复杂字体/语言/Bidi、CST 及真实平台矩阵仍未验收完成。整体目标保持进行中。

## 字素内字体与上下标策略验收（2026-10-01）

旧 DLL 的 before 日志以退出码一证明跨字体组合重音宽度 30，而完整字素期望 11。现在可用联合分段使用首个有显示内容片段的实际字体及位移；下一字素重新选择。源属性/UTF-8/Markdown 来源不因此重写。共享 Renderer 60 对配置×三次改宽覆盖组合重音、ZWJ、区域指示符、Rich 与实体解码 Markdown、四种首片段样式和中间空节点、spans/plain 两种绘字；核对具体度量、亲和、命中、范围、最终完整文本与实际字体/矩形。种子/实际行在分配 clusters 后 OOM，两阶段均保留 revision 且同对象重试通过。

最终追加 font mutation/Undo 检查：仅改尾节点字体不改变先前字素拥有者，改首节点字体则改变完整字素 advance；两次 Undo 恢复对应字体边界及尾文字度量。Windows 全套编译 Renderer 后才追加这些断言，故它们以独立 final2-renderer 和最后 final2-linux-renderer 为据。Editor 18 配置测试方向键、点击、整字素退格、撤销后几何和原实体源码。导航/命中在邻接 Text 叶节点返回不同身份的同一文字边缘时，使用空复制范围验证，而非要求结构位置比较为零。

原生 frame 的 Arial/Arial Bold 24 对配置×三次宽窄重排，完整 360×180 RGBA8 一致且非空；两种绘字能力各与相同能力下的完整样式表示对照。这验证实际 XGE 字形/缺字回退与布局的一致性，不验证完整彩色 emoji、GSUB/GDEF/复杂字体或浏览器样式等价。

| 已结束且退出码零的检查 | 日志（artifacts/xui-document-rebuild） |
| --- | --- |
| 最终产品默认 DLL | `validation-2026-10-01-grapheme-font-build.log` |
| layout 严格实际 -c GCC analyzer | `validation-2026-10-01-grapheme-font-layout-analyzer.log` |
| 完整 Windows 发布套件 | `validation-2026-10-01-grapheme-font-final-suite.log` |
| 包含新增属性修改/Undo 的最终 Windows Renderer | `validation-2026-10-01-grapheme-font-final2-renderer.log` |
| 最终 Windows Editor 独立检查 | `validation-2026-10-01-grapheme-font-editor-probe4.log` |
| 最终 Linux Renderer，含 font mutation/Undo，ASan/UBSan | `validation-2026-10-01-grapheme-font-final2-linux-renderer.log` |
| 最终 Linux View/Editor，含实体边缘检查，ASan/UBSan | `validation-2026-10-01-grapheme-font-final2-linux-view.log` |
| 最终 Linux MessageList，ASan/UBSan | `validation-2026-10-01-grapheme-font-final-linux-message.log` |
| 可选 DLL / 基础 WebView、Document 内部 provider、离线 PNG/HTML 沙箱 | `validation-2026-10-01-grapheme-font-final-webview-widget.log`、`validation-2026-10-01-grapheme-font-final-web-provider.log`、`validation-2026-10-01-grapheme-font-final-web-render.log` |

Windows 套件保留既有 Core、652 项 CommonMark 解析/源码/原生往返（不是 HTML conformance）、284 导出/无旧 Rich API、Editor、样式/规模/fractional/表格、剪贴板/对象来源、MessageList 与原生同步/异步/缺图绘制，未放宽原预算。最终 Linux Renderer/View/Editor/MessageList 的 ASan/UBSan 和差异检查结果补录于下方。

最终三个 Linux 日志均已退出码零；显式 detect_leaks=1、halt_on_error=1 与 UBSan halt，无 sanitizer 错误，全 UI 编译保留既有 date-picker 警告。Renderer 覆盖最终字体属性修改/两次 Undo，View/Editor 使用最终实体文字边缘检查并通过全部 18 新配置；MessageList 原规模与阅读锚点预算通过。最终 tracked diff whitespace 检查通过，三个新共享测试头的 no-index whitespace 无输出（新文件差异返回一）。所有进程结束；本批产品代码在默认 DLL 构建后未再改变。

字素内字体/上下标策略已完成此可用联合路径验收。真实字体连字/停点、复杂脚本/Bidi、其余 cluster 映射回退、完整 CST 和真实平台/IME/读屏/物理 DPI 尚未完成，整体长期目标继续。WebView 的公开基础能力范围未扩展，Document 内部渲染通道保留。


## K2 嵌套段落祖先标记来源补丁（2026-10-01）

真实失败证据为 `validation-2026-10-01-nested-prefix-before.log`：旧默认 DLL 对含隐藏定义的 `+ > abcd` 在正文中插入段落分隔符返回 UNREPRESENTABLE。产品在 `src/xui_document_markdown_edit.c` 的嵌套段落来源路径使用解析器确认的每层列表/任务/引用标记范围生成首行与续行前缀，保留完整祖先链；候选仍重解析并核对全文语义与所有链接定义原拼写。原定义和补丁外实体/源码按原字节保留，粗体交由行内写回，行结束符沿用原 LF/CRLF；未引入第二套文本或公开 API。默认 DLL 构建 `validation-2026-10-01-nested-prefix-build.log` 退出码零，此后产品源码/头文件未改；修改 TU 的实际 `-c -O2 -fanalyzer -Wall -Wextra -Werror` 日志 `validation-2026-10-01-nested-prefix-edit-analyzer.log` 为空且退出码零。

共享九个来源 fixture 覆盖 List→Quote、连续 Quote、多层列表、Quote→List→Quote、多位有序标记、任务框、Tab、粗体、LF/CRLF、定义后缀及合并间隙、前后未改实体段落；比较完整期望源码、合法光标的实际文本坐标、精确 Undo/Redo。独立加载期望源码，以公开快照 API 比较文字/属性语义并允许相邻同属性 Text 的不同分段，逐项比较块来源、引用前缀、列表逻辑列缩进、全部行内和定义语法记录。九组 OOM 预算分别为 136/151/151/176/139/292/162/95/112 次尝试，含每组首次成功；总计 1414 次，失败时源码、树、来源/标记、revision、Dirty、Undo/Redo、通知保持原子，文档释放后 allocator live 为零。

共享 Windows/Linux Editor fixture 验证实际 VISUAL 拆分与选区合并、发布光标继续输入、Undo/Redo、SOURCE 源码投影及切回 VISUAL。直接任务项的 Enter 是拆列表项；含隐藏定义的该结构操作仍原子拒绝，因此测试另外以非折叠选区替换验证任务项内拆段，不降低目标语义或放宽发布守卫。完整 CST、其他容器结构编辑、任意前缀来源及真实平台验收仍待继续。

阶段性失败日志保留：最初 caret 校验使用了事务返回位置的提交前 revision；最初语义 oracle 错把相邻 Text 的不同分段视作不等价；Editor fixture 对任务项 Enter 及替换后光标的期望也已按实际命令语义修正；final2-suite 的 DLL 链接错误源自新测试调用私有等价函数，最终 oracle 仅使用公开快照 API。上述为测试构造修正，产品缺口证据仍是 before 日志。最终验证结果续录于下文。


最终验收全部退出码零，日志位于 `artifacts/xui-document-rebuild/`：

| 验证 | 最终日志 |
| --- | --- |
| 完整 Windows 发布套件（最终公共 API oracle） | `validation-2026-10-01-nested-prefix-final3-suite.log` |
| Linux Core ASan/UBSan（最终公共 API oracle、九组 OOM） | `validation-2026-10-01-nested-prefix-final3-linux-core.log` |
| Linux Core + 652 CommonMark ASan/UBSan（相同最终产品源码） | `validation-2026-10-01-nested-prefix-final2-linux-core.log` |
| Linux Renderer、View/Editor、MessageList ASan/UBSan | `validation-2026-10-01-nested-prefix-linux-renderer.log`、`validation-2026-10-01-nested-prefix-final-linux-view.log`、`validation-2026-10-01-nested-prefix-linux-message.log` |
| 可选 DLL / Windows 基础 WebView、Document 内部 provider、离线渲染 | `validation-2026-10-01-nested-prefix-webview-widget.log`、`validation-2026-10-01-nested-prefix-web-provider.log`、`validation-2026-10-01-nested-prefix-web-render.log` |

Windows 完整套件包含独立 Core、实际 DLL 的全部九组来源/失败用例、652 CommonMark 解析/源码/原生往返与增量差分（不是 HTML conformance）、284 个公开 Document 导出且无旧 Rich API、Renderer/Editor、样式/规模/fractional、表格、剪贴板/对象来源、MessageList 和原生同步/异步/缺图绘制。原有预算未放宽，消息改宽 shaping 仍为 786528/655376；10 MiB SOURCE Editor 的 64 次待发布输入 P95 为 0.131 ms。Linux 明确启用 ASan leak/halt 与 UBSan halt，未报告 sanitizer 错误；全 UI 的既有 date-picker 编译警告保留。产品源码从上述默认 DLL build 后未再修改；最后只调整测试的公共 API oracle、事务光标 revision 约定及测试文件行结束符。最终 tracked diff --check 无诊断，四个新增测试文件另以 no-index --check 检查，无诊断（新增文件差异返回一）。

本批实现及验收完成，K2 与整体长期目标保持进行中。仍需完整 CST 与其他容器/非文字复杂结构来源编辑、含定义任务项的新建列表项等路径，真实字体连字/停点、复杂脚本/Bidi，以及 K4/E1/E2 的剩余性能和真实平台/IME/读屏/物理 DPI 验收。通用 WebView 继续仅交付 Windows 基础功能、其他平台保留空实现；Document 私有公式/Mermaid/HTML 渲染通道保留，本批未新增对外通信能力。

## K2 含定义列表项 Enter 来源补丁（2026-10-01）

旧 DLL 的 `validation-2026-10-01-list-split-source-before.log` 退出码一，复现带隐藏定义的 `- > 7) [x] abcd` 在正文中间拆列表项返回 UNREPRESENTABLE。本批在私有 Markdown shadow 结束路径增加选中段落来源补丁：保留原项开头、段前定义和未改兄弟拼写，插入同类项目符号/有序分隔符，新编号按列表起始值及项序计算，新任务项清除 CHECKED，尾部可见块及定义跟随新项。松散列表补必要的空白续行；保留原 LF/CR/CRLF、BOM 与末尾无换行。按解析器确认的祖先链生成前缀，不改变统一 Document/SourceStore、可撤销事务及位置映射内核，不增加公开 API。

编号位数变化的早期 case 7 仍拒绝，因为保留旧缩进会改变尾块归属。最终路径只增减已确认的尾部续行缩进，按四列 Tab 边界保留原 Tab 或写出所需剩余空格。候选完整重解析并与整篇结构目标比较；缩进未变时所有定义完整原拼写相同，缩进改变时受影响链接定义的标签/目标/标题原字节相同，其他定义完整原拼写相同。不能证明的多行字段或复杂结构仍回退并可能原子拒绝，本批不声称完成完整 CST 或所有定义形式。

共享 21 组 fixture 覆盖嵌套引用/列表、任意原始有序值与逻辑编号不同、99→100 增宽及九位原标记收缩、Tab、定义前置/重复、尾部段落/引用块与未改兄弟、空首/尾任务项、标题、粗体、实体、引用式链接和全部上述行结束符。Core/DLL 核对完整期望源码，独立加载期望源码后以公开快照比较语义（允许同属性相邻 Text 的不同分段）、块/引用/列表缩进来源及全部行内/定义语法；还检查任务属性、发布光标、未改兄弟及搬移块的稳定身份/逐 UTF-8 边界位置映射、Undo/Redo。21 组 OOM 预算共 4,588 次尝试，各含首次成功；失败时源码、树、标记来源、revision、Dirty、Undo/Redo、通知不变，释放后 live 为零。Windows/Linux Editor 交替派发实际 Enter 事件与 Execute，检查新光标继续输入、两步撤销、重做和 SOURCE 源码投影。

上批九组 Editor 用例仍以非折叠选区替换测试任务项内的拆段；其历史 Enter 拒绝断言由新矩阵替代。core2–core5 的测试构造失败已修正：编译缩进告警、误要求空项没有占位段落、误要求空段落复制范围没有结构换行以及误写公开字段名；最终要求合法 GAP 0 和仅含空 Text 的可选段落。独立 probe 的希腊 UTF-8 C 字面量已分隔，避免十六进制转义延伸。上述日志保留，真实产品缺口证据仍为 before 与编号增宽场景。

以下最终检查均已结束且退出码零，日志位于 `artifacts/xui-document-rebuild/`：

| 验证 | 日志 |
| --- | --- |
| 最终默认 DLL | `validation-2026-10-01-list-split-source-final-build.log` |
| Markdown edit / commands 严格实际 -c GCC analyzer | `validation-2026-10-01-list-split-source-final-edit-analyzer.log`、`validation-2026-10-01-list-split-source-commands-analyzer.log` |
| 公开 DLL 精确来源 probe、独立 Core 与全部 OOM | `validation-2026-10-01-list-split-source-probe3.log`、`validation-2026-10-01-list-split-source-core6.log` |
| 独立 Windows Editor | `validation-2026-10-01-list-split-source-editor-probe.log` |
| 完整 Windows 发布套件 | `validation-2026-10-01-list-split-source-final-suite.log` |
| Linux Core + 652 CommonMark，ASan/UBSan | `validation-2026-10-01-list-split-source-linux-core.log` |
| Linux Renderer、View/Editor、MessageList，ASan/UBSan | `validation-2026-10-01-list-split-source-linux-renderer.log`、`validation-2026-10-01-list-split-source-linux-view.log`、`validation-2026-10-01-list-split-source-linux-message.log` |
| 可选 Windows DLL / 基础 WebView、Document 内部 provider、离线 PNG | `validation-2026-10-01-list-split-source-webview-widget.log`、`validation-2026-10-01-list-split-source-web-provider.log`、`validation-2026-10-01-list-split-source-web-render.log` |

完整 Windows 套件包括独立 Core/实际 DLL 的 21 组来源和失败矩阵、既有 652 CommonMark 解析/源码/原生往返与增量差分（不是 HTML conformance）、284 个公开导出且无旧 Rich API、Renderer/Editor、样式/规模/fractional、表格、剪贴板/对象来源、MessageList 以及原生同步/异步/缺图像素。Linux 明确开启 ASan detect_leaks/halt 与 UBSan halt，未报告 sanitizer 错误，全 UI 保留既有 date-picker 编译警告。原有预算未放宽，MessageList 改宽 shaping 仍为 786528/655376；可选 Windows WebView 仍仅提供基础对外承载，Document 内部公式/Mermaid/HTML 故障恢复、测量、三张 500×350 PNG 和 HTML 沙箱通过。

产品源码/头文件从 final-build 后未改；edit analyzer 早于解释性注释修正，执行代码相同，commands analyzer 亦为空。最后只将 Linux 共享 Editor 文件的混合换行统一为 LF，内容不变。tracked diff --check 与本批四个新增测试文件、两个修改的未跟踪共享 Editor 文件及进度文档的 no-index --check 均无诊断（新增文件差异返回一）。本批实现及验收完成，K2 完整 CST、其他来源编辑、真实字体/复杂脚本/Bidi、K4/E1/E2 剩余性能及真实平台验收仍在进行，整体长期目标保持 active。

## K2 多行链接定义的缩进与完整内容校验（2026-10-01）

`validation-2026-10-01-reference-reindent-before.log` 的旧 DLL 在 sample 21 返回 UNREPRESENTABLE：列表项 `99) abcd` 在正文中间拆分后，新 `100)` 所需续行缩进增加一列，多行标题的连续原字段包含这段容器前缀，旧守卫因此拒绝。新回调 `MD_XUI_REFERENCE_VALUES` 从同一固定版本 MD4C 获取每条定义按语法合并行后的原始标签、目标、标题及标题存在标志，包含未使用与重复定义。普通解析不启用该回调；仅在困难来源候选的受影响原字段比较不等时，前后源码按相同方言、BOM/front matter 起点做无文档树的校验解析。

发布仍须全树等于结构目标，定义数量/种类/顺序不变，尾部外或非链接定义完整原拼写相同，且 writer 仅调整已确认容器缩进。新校验完整比较每个字节，保留实体/转义拼写，拒绝未使用定义目标/标题变化、重复定义后项变化、标签改名/换序、定义增删、无标题和空标题互换；不依赖哈希或只核对渲染出的链接。分配和 TLS/取消保存恢复复用普通适配器，失败或取消直接返回，临时缓冲不形成第二套可写 Document。

额外两次解析与扫描字节计入现有 `iMarkdownParses` / `iMarkdownParsedBytes`。公开 Core/DLL 测试精确要求困难案例三次解析（候选加两次定义校验）及前后源码扫描总字节，同宽/原拼写匹配案例只一次；统计不隐藏成本。当前困难路径临时连续化前后源码并额外扫描，尚不能据此证明大文档结构编辑满足 UI 响应门槛，完整来源片段索引和结构编辑异步优化继续。

共享来源矩阵增至 33 组，新增多行标签/标题、目标另起一行、增长/收缩、Tab、重复未用定义、已用链接、BOM/bare CR、CRLF 引用前缀、front matter、嵌套任务项和未修改的根层未用脚注。Core/DLL 比较精确完整源码、独立公开快照语义和行内/定义/块来源、稳定兄弟与搬移块每个 UTF-8 边界位置映射、光标和 Undo/Redo。33 组 OOM 预算共 7,264 次尝试，各含首次成功，失败时发布源码、树、标记、revision、Dirty、历史和通知原子，释放后 live 为零。Windows/Linux Editor 使用同一矩阵，实际 Enter/Execute、后续光标输入、撤销重做及 SOURCE 投影通过。

Core-only 私有守卫单测另含 18 对相等/不等内容、64 KiB 标题末端差异、17 次 OOM 预算、预先取消及 17 个两轮解析的分配触发取消检查，均无泄漏；该函数不成为公开 DLL 导出。严格 analyzer 指出上游引用行查询可能返回 NULL 而调用方直接使用，现对空行数组和缺失结果防御返回；没有有效输入触达该路径的复现证据，不宣称已复现并修复合法输入崩溃。共享方言 flags 抽取没有改变语法集合。

阶段日志保留：初次新增 BOM C 字面量的十六进制转义延伸和取消测试分配器参数次序已修正；64 KiB 标题构造缺起始引号已改用 sizeof(prefix)，其末端变化现在确实被拒绝。早期 md4c analyzer 的空指针诊断由上述防御检查处理；最终三个产品 TU 的严格实际编译分析为空。真实产品缺口仍由 before 日志证明。

以下检查全部已结束且退出码零，日志位于 `artifacts/xui-document-rebuild/`：

| 验证 | 最终日志 |
| --- | --- |
| 最终默认 DLL（含完整扫描成本统计） | `validation-2026-10-01-reference-reindent-final2-build.log` |
| md4c / markdown / edit 实际 -c -O2 严格 GCC analyzer | `validation-2026-10-01-reference-reindent-final2-md4c-analyzer.log`、`validation-2026-10-01-reference-reindent-markdown-analyzer.log`、`validation-2026-10-01-reference-reindent-edit-analyzer.log` |
| 独立 Core：33 来源/失败矩阵、私有完整内容/64 KiB/OOM/取消、成本统计 | `validation-2026-10-01-reference-reindent-final-core.log` |
| 完整 Windows 发布套件：实际 DLL 的公开矩阵、Editor 及原生绘制 | `validation-2026-10-01-reference-reindent-final-suite.log` |
| Linux Core + 652 CommonMark，ASan/UBSan | `validation-2026-10-01-reference-reindent-final-linux-core.log` |
| Linux Renderer、View/Editor、MessageList，ASan/UBSan | `validation-2026-10-01-reference-reindent-final-linux-renderer.log`、`validation-2026-10-01-reference-reindent-final-linux-view.log`、`validation-2026-10-01-reference-reindent-linux-message.log` |
| 可选 Windows DLL / 基础 WebView、Document 内部 provider、离线 PNG | `validation-2026-10-01-reference-reindent-webview-widget.log`、`validation-2026-10-01-reference-reindent-web-provider.log`、`validation-2026-10-01-reference-reindent-web-render.log` |

Windows 完整套件保留 652 CommonMark 解析/源码/原生往返与增量差分（不是 HTML conformance）、284 项公开导出且无旧 Rich API、Renderer/Editor、样式/规模/fractional、表格、剪贴板/对象来源、MessageList、原生同步/异步/缺图像素验收。Linux 显式 ASan detect_leaks/halt 与 UBSan halt，无 sanitizer 错误，全 UI 的既有 date-picker 警告保留。原预算未放宽，MessageList 改宽 shaping 仍为 786528/655376；可选 Windows WebView 生命周期与内部公式/Mermaid/HTML 故障恢复、测量、三张 500×350 PNG、HTML 沙箱均通过，未增加通用 WebView 对外通信。

产品源码/头文件在 final2-build 后未改。最终 tracked diff --check，以及新增私有测试头、修改的两个未跟踪共享来源头和进度文档的 no-index --check 无诊断（新增文件差异返回一）。本批完成；完整 CST、其余结构来源编辑、复杂脚本/Bidi、K4/E1/E2 性能和真实平台/IME/读屏/DPI 等仍在进行，整体长期目标保持 active。

## K2 定义内容快照缓存与结构校验成本（2026-10-01）

此批替代上一节困难候选的两次额外源码解析。正常 MD4C 解析生成一项对应一条定义的持久内容缓存，包含原始合并行标签、目标、标题及标题存在标志，重复与未使用定义全部保留；脚注为类型占位项。缓存是同一 state 的派生元数据，SourceStore 保持唯一可写源码；缓存不独立序列化，原生 Markdown 重载重新解析恢复。普通正文/来源偏移共享，单条定义的等长或变长字段编辑只替换对应 blob 和路径，复杂编辑仍使用完整解析，Reconcile 和语义投影采用已解析缓存。

列表 Enter 守卫改为完整缓存字节比较，不连续化前后源码、不分配比较缓冲、不额外扫描；长字段每 16 KiB 检查取消。仍须候选全树语义相等、定义数量/顺序/种类不变及尾部外或非链接定义原拼写相同，脚注占位项不证明其正文相等。公开 Core/DLL 精确断言困难 sample 21 与普通 sample 29 都只解析一次、扫描候选源码一次。候选本身仍全文解析，不能把这一改进视为完整 CST 或大文档结构编辑异步验收。

缓存纳入克隆、释放、当前/历史/快照归属和 Prepare 全量/差量发布计费。独立私有 oracle 检查每条缓存的类型、长度、标题存在标志和全部字节，已接入原有增量编辑及 652 CommonMark 的 Prepare/续输入/完整加载比较，不以调用产品比较函数代替 oracle。18 对归一/原拼写测试在分配预算为零时完成缓存比较、解析计数不变，64 KiB 标题末端变化和预先取消通过。九组正文、来源偏移、等长/变长标签/目标/标题、未用重复定义编辑核对根/未变 blob 共享、旧快照、精确 Undo/Redo、完整重载和清空历史后的附加快照存储；脚注正文编辑共享占位项并匹配重载。上一节的两解析器私有观察器保留作独立测试，其 17 次 OOM 与 17 个取消检查仍通过，生产结构编辑不再调用它。

33 组共享来源矩阵的分配预算共 7,128 次尝试，各含首次成功；失败时发布源码、树、语法、revision、Dirty、历史和通知不变，最终 allocator live 为零。统计减少来自移除校验解析，新增缓存分配也纳入扫描预算，未放宽既有失败/规模上界。阶段 core/core2/core3、suite、linux-core/linux-core2 日志保留：新增共享 fixture 起初未满足局部定义的空行边界，调整后显式验证增量共享；另曾误要求诊断用的瞬时 snapshot owner[2] 为常驻计数，现核对公开附加内存及诊断后的标记清理，产品归属代码未因此修改。完整解析回退仍由差分矩阵覆盖。

以下检查全部已结束且退出码零，日志位于 `artifacts/xui-document-rebuild/`。

| 验证 | 日志 |
| --- | --- |
| 最终默认 DLL | `validation-2026-10-01-reference-cache-build.log` |
| reference/store/markdown/edit/incremental 严格实际 -c -O2 GCC analyzer，全部为空 | `validation-2026-10-01-reference-cache-analyzer.log`、`validation-2026-10-01-reference-cache-store-analyzer.log`、`validation-2026-10-01-reference-cache-markdown-analyzer.log`、`validation-2026-10-01-reference-cache-edit-analyzer.log`、`validation-2026-10-01-reference-cache-incremental-analyzer.log` |
| 独立 Core：缓存共享/内容/归属/取消、33 组公开来源/失败/解析成本 | `validation-2026-10-01-reference-cache-core4.log` |
| 完整 Windows 发布套件：Core/实际 DLL、Editor、原生绘制 | `validation-2026-10-01-reference-cache-final-suite.log` |
| Linux Core + 652 CommonMark，ASan leak/halt 与 UBSan halt | `validation-2026-10-01-reference-cache-final-linux-core.log` |
| Linux Renderer、View/Editor，ASan/UBSan | `validation-2026-10-01-reference-cache-linux-renderer.log`、`validation-2026-10-01-reference-cache-linux-view.log` |
| Linux MessageList，ASan/UBSan | `validation-2026-10-01-reference-cache-linux-message.log` |
| 可选 Windows 基础 WebView、Document 内部 provider、离线 PNG | `validation-2026-10-01-reference-cache-webview-widget.log`、`validation-2026-10-01-reference-cache-web-provider.log`、`validation-2026-10-01-reference-cache-web-render.log` |

完整 Windows 套件包含 652 CommonMark 解析/源码/原生往返与增量差分（不是 HTML conformance）、284 项公开导出/无旧 Rich API、Renderer/Editor、样式/规模/fractional/表格、剪贴板/对象来源、MessageList、原生同步/异步/缺图绘制。原预算未放宽，MessageList 改宽 shaping 仍为 786528/655376。可选 WebView 的 12 次重复生命周期、两独立视图及故障重建、Document 内部故障恢复/测量、三张 500×350 PNG、HTML 沙箱均通过；没有扩展通用 WebView 对外通信。Linux 全 UI 保留既有 date-picker 编译警告。

产品源码/头文件从 build 后未改，只补测试与说明。Linux Core/652、Renderer、View/Editor 和 MessageList 均显式开启 ASan detect_leaks/halt 与 UBSan halt，无 sanitizer 错误。最终 tracked diff --check 与本批新模块、私有 oracle、修改的未跟踪增量/来源/私有测试头、进度文档的 no-index --check 无诊断（新增文件差异返回一）。本批实现及验收完成；完整 CST、其他结构来源编辑、复杂脚本/Bidi、K4/E1/E2 剩余性能和真实平台/IME/读屏/DPI 等继续，整体长期目标保持 active。

## K2 代码围栏语言字段来源补丁（2026-10-01）

旧 DLL 的 `validation-2026-10-01-code-language-source-before.log` 退出码一，复现修改语言时通用写回把 `~~~~  c  {.numbers key="a&amp;b"}` 规范化为 `~~~cpp`，丢失额外 info、开闭围栏宽度、CRLF 和尾部空白。新的路径仅接纳 CodeBlock 的单次语言资源变更，父节点、正文、子节点、样式、resource/title 必须不变；使用既有 parser-confirmed 语言范围做一个 SourceStore 替换，再完整解析候选、核对语义及所有未改定义。新的 `&` 与反斜杠用实体保护，UTF-8 文字保留；原围栏、info 其他字节、容器前缀、正文和换行不规范化。

脚注内的语言编辑复用既有所选脚注正文守卫：标签/前缀及其他定义原拼写保持，writer 实际只改一个语言字段。初次脚注拒绝日志 `probe2` 和 `core` 保留；独立 `diagnostic` 的候选/完整加载/目标比较证明语义相等，拒绝来自整条脚注定义原拼写变化，未发现增量解析错误。新增测试最初误用不存在的 ChangeSet 枚举方法，已改为 `GetInfo` 的公开数组。缺少可证明语言字段的 Indented code 等情况仍走通用结构写回；清空语言若会使剩余元数据变成语言，则原子返回 UNREPRESENTABLE，不用通用回退丢弃元数据。调用方可在 SOURCE 中明确编辑整段 info。这一补丁不表示完整 CST、所有围栏转换或大文档结构异步验收已完成。

13 组共享 fixture 覆盖根/引用/列表/已用脚注、增长/收缩、额外 info、空 info/清空 token、原实体与转义、反斜杠/实体新语言、UTF-8、LF/CR/CRLF/BOM、无末尾换行与未闭合围栏。Core/DLL 核对精确完整源码、稳定 CodeBlock ID、独立完整加载的语义和全部来源/定义内容缓存、旧快照及 Undo/Redo；ChangeSet 恰有一个 SOURCE 操作且替换原语言字段。根层与嵌套分配失败共 149 次尝试（各含首次成功），保持源码/树/语法/revision/历史原子，释放后 live 为零；另核对带额外 info 的清空拒绝。Windows/Linux Editor 共享同一矩阵，验证正式语言 API、合法发布光标、继续正文输入、两步撤销/重做及 SOURCE 投影。

以下检查全部已结束且退出码零，日志位于 `artifacts/xui-document-rebuild/`。

| 验证 | 日志 |
| --- | --- |
| 最终默认 DLL | `validation-2026-10-01-code-language-source-final-build.log` |
| edit TU 严格实际 -c -O2 -fanalyzer -Wall -Wextra -Werror，日志为空 | `validation-2026-10-01-code-language-source-final-edit-analyzer.log` |
| 独立 Core：13 来源/149 OOM、metadata-clear 原子拒绝 | `validation-2026-10-01-code-language-source-core2.log` |
| 实际 DLL 独立公开 probe | `validation-2026-10-01-code-language-source-final-probe.log` |
| 完整 Windows 发布套件：Core/DLL/Editor/原生绘制 | `validation-2026-10-01-code-language-source-final-suite.log` |
| Linux Core + 652 CommonMark，ASan leak/halt 与 UBSan halt | `validation-2026-10-01-code-language-source-linux-core.log` |
| Linux Renderer、View/Editor、MessageList，ASan/UBSan | `validation-2026-10-01-code-language-source-linux-renderer.log`、`validation-2026-10-01-code-language-source-linux-view.log`、`validation-2026-10-01-code-language-source-linux-message.log` |
| 可选 Windows 基础 WebView、Document 内部 provider、离线 PNG | `validation-2026-10-01-code-language-source-webview-widget.log`、`validation-2026-10-01-code-language-source-web-provider.log`、`validation-2026-10-01-code-language-source-web-render.log` |

产品实现从 final-build 后未改，只补说明。完整 CST、其余结构来源编辑、复杂脚本/Bidi、K4/E1/E2 性能和真实平台/IME/读屏/DPI 验收仍继续，整体目标 active。通用 WebView 仍只交付 Windows 基础网页承载，Document 私有公式/Mermaid/HTML 通道保留；可选故障恢复、12 次重复生命周期、三张 500×350 PNG 及 HTML 沙箱通过，没有扩展通用控件对外通信。Linux 明确启用 ASan detect_leaks/halt 与 UBSan halt，未报告 sanitizer 错误，UI 保留既有 date-picker 编译警告。

完整 Windows 套件包含 13 组公开语言字段/149 次失败预算与共享 Editor、既有 33 组列表来源/7,128 次预算和单次解析成本断言、652 CommonMark 解析/源码/原生往返及增量差分（不是 HTML conformance）、284 导出且无旧 Rich API、Renderer/Editor、样式/规模/fractional/表格、剪贴板/对象来源、MessageList 以及原生同步/异步/缺图像素验收。原预算未放宽，MessageList 改宽 shaping 仍为 786528/655376。最终 tracked diff --check 与四个新增共享测试/probe、修改的未跟踪 Linux 共享 UI TU、进度文档的 no-index --check 无诊断（新增文件差异返回一）。本批实现及验收完成，整体长期目标继续进行。

## K2 标题正文来源与标记编辑（2026-10-01）

修复前 `validation-2026-10-01-heading-source-before.log` 在首个公开 DLL 样本失败：根层标题调整级别重写正文，丢失实体原拼写、空白与闭合井号。MD4C 新 guarded 正文回调使用已消耗定义、已去掉 Setext 下划线的最终行集，Document 同一节点保存两个相对偏移；公开 `GetBlockSyntax` 增加正文首尾字段，空 ATX 合法，非标题字段为 UINT64_MAX。接口结构体尺寸变化需调用方与 DLL 一起重编译，无兼容包装。独立增量/Prepare 树比较已核对标题正文来源。

标题修改覆盖根/引用/列表/已用脚注：ATX 级别只改开标记，清除标题另去掉可选闭标记和间隙；Setext 一二级转换保持下划线宽度与尾部，转段落删除完整下划线行，单行转 ATX 添加开标记。候选逆序应用独立补丁、拒绝重叠，并比较完整语义与未改定义；多个脚注的标签和其他定义保持原字节。段落转标题仍依赖行内来源索引；多行 Setext 转 ATX 等不能证明的结构继续通用写回，可能原子拒绝。本项不是完整 CST 或所有结构命令的无损保证。

首次 root 补丁绕过原局部解析，大文档回归 `core.log` 在 39,673 次分配触发原门槛；单补丁改为既有 parsed-source patch 后，最终同项为 127 次、首段文字重写为 100 次，少于 512 次且无整篇大分配，解析字节断言也通过。原预算未放宽。`probe.log` 的空标题失败来自测试未填 position.iSize；`suite.log` 与 `linux-core.log` 的取消失败来自测试把事务分配误算为固定一个对象；测试现使用合法 position 和 Begin 前独立 live 基线，最终通过。以上失败日志保留，不混作最终产品故障。

24 组共享 Core/DLL/Editor fixture 覆盖根/引用/列表/脚注、ATX/Setext/多行/空标题、BOM/LF/CR/CRLF/EOF、行内原语法/实体/转义/尾部空白；另覆盖根层与多个脚注的混合选区、70,000 字节下划线。核对精确全文、稳定节点、独立完整加载的语义/来源/定义缓存、旧快照和历史；295 次 OOM 尝试（含三次成功）以及 187 个取消检查点（含两次成功）证明失败原子和释放后零 live。Windows/Linux Editor 使用正式命令，验证发布光标的继续输入、撤销/重做及源码投影。

以下全部已结束、退出码零，日志位于 `artifacts/xui-document-rebuild/`，文件名统一以 `validation-2026-10-01-heading-source-` 开头。

| 验证 | 日志后缀 |
| --- | --- |
| 默认 DLL 与实际公开 probe | `build2.log`、`final-probe.log` |
| 完整 Windows 发布套件 | `final-suite.log` |
| Linux Core/652，ASan detect_leaks/halt 与 UBSan halt | `final-linux-core.log` |
| Linux Renderer、View/Editor、MessageList，ASan/UBSan | `linux-renderer.log`、`linux-view.log`、`linux-message.log` |
| Windows 基础 WebView、Document 内部 provider 与三张 PNG | `webview-widget.log`、`web-provider.log`、`web-render.log` |
| edit/markdown/source 实际严格 analyzer，日志为空 | `edit-analyzer.log`、`markdown-analyzer.log`、`source-analyzer.log` |
| MD4C 无集成宏严格实际编译，日志为空 | `upstream-compile.log` |
| API coverage no-write 与 comment lint | `api-coverage.log`、`comment-lint.log` |

Windows 完整套件包含 284 公开导出且无旧 Rich API、652 CommonMark 解析/源码/原生往返及 Prepare 差分（不是 HTML conformance）、Renderer/Editor、样式/规模/fractional/表格、剪贴板/对象来源、MessageList 和原生同步/异步/缺图像素验收。原有 33 组列表来源分配失败矩阵每条 Core/DLL 路径为 7,129 次尝试（标题派生 blob 增加一次；此前缓存批次为 7,128），单次候选解析断言保持；MessageList 改宽 shaping 仍为 786528/655376。Linux 没有 sanitizer 错误，UI 保留既有未修改 date-picker 编译警告。可选内部渲染验证测量、进程故障恢复、12 次生命周期、三张 500×350 PNG 和 HTML 沙箱，未添加通用 WebView 对外通信。完整 CST、复杂脚本/Bidi、设计性能预算及真实平台/IME/读屏/DPI 等余项仍进行中。最终 tracked 和本批新增共享测试/probe、未跟踪来源/进度/Linux UI 文件的 whitespace 检查无诊断；产品实现自 build2 后未改，只补测试与说明。

## 原生 OpenType 整形基础验收（2026-10-01）

本批集成固定 HarfBuzz 的 C API，第三方独立 C++17 对象与既有 stb 字形栅格化配合；XGE/XUI/Document 集成仍是 C。571 个 vendored src 文件与原包 hash 全部相同。测试字体从项目矩形轮廓、GSUB/GPOS/GDEF 数据生成，不携带 OS 字体。真实字体验证使用本机 Calibri：15 字节 office affinity 整形为 11 字形和 4 个内部字素停点。

以下最终进程均已结束、退出码零，日志在 artifacts/xui-document-rebuild/，表内统一省略 validation-2026-10-01-opentype- 前缀。

| 验证 | 日志后缀 |
| --- | --- |
| 最终默认 DLL、语法检查 | suite2.log 中的自动重建、syntax3.log |
| 完整 Windows Document 发布套件 | suite2.log |
| 公开 XGE 字体/整形/生命周期探针 | native-final.log |
| 原生 Rich/Markdown、VISUAL/SOURCE/LIVE、40/37 字号、停点与 GPU/Editor | document-final2.log |
| 原有 FontFace/字号/XRF/hybrid 字体链路 | text-foundation.log |
| 原有 XGE/XUI 代理 smoke | proxy-xge.log |
| 实际 proxy 严格 -c -O2 -fanalyzer 编译，日志为空 | proxy-analyzer.log |
| Linux 独立 HarfBuzz 对象及 C 链接字体探针 | linux-dependency2.log |
| Linux 动态导入和对象未解析符号记录 | linux-dependency-imports.log |
| 可选 Windows 基础 WebView 控件与生命周期 | webview-widget2.log |
| Document 内部对象 provider、进程故障恢复 | web-provider.log |
| 私有 KaTeX/Mermaid/HTML 测量、沙箱和三张 500×350 PNG | web-render.log |

native-final 验证拉丁/希腊连字、完整/缺失/数量不符/越界 GDEF、GPOS 附标定位、kerning 开关、组合字形整体命中、CRLF、测量一致以及 8 轮 FontFace 共享字号/字体实例引用生命周期。document-final2 验证完整 Text 和三个颜色 Text 节点及 Markdown 同一 ffi 的光标、选择范围、命中、重排和逐像素 alpha 一致。40/37 字号提供整数与非整数停点，三个颜色的实际 GPU 像素遵守 GDEF 分界；局部原生下划线只覆盖中间字素。SOURCE/LIVE 原生光标和命中也遵守同一停点，Editor 方向键及删除/输入、精确纯文本和 Undo 通过。

负向对照保留在 negative-oracle.log：test-only 代理用旧式整字形最后颜色绘制，颜色像素断言确实失败。decoration-before.log 以真实旧 DLL 证明局部下划线原本覆盖整个 ffi。paint-before.log 是旧本地 DLL 被加载时的整形断言失败，不能当成颜色修复的前后证据。document.log 记录 fixture 用 RICH profile 调用 LoadMarkdown 的错误；suite.log / precision2.log 记录测试遗漏段落纯文本末尾换行；document-final.log 记录 SOURCE 模式误调用仅限 LIVE 的 SetActivePosition。它们均为已修正的测试约定，最终路径通过。Linux 初次 dependency.log 缺少 HAVE_PTHREAD，触发上游 std::mutex 导致 C 链接失败；dependency2 明确选择 pthread 后成功，没有禁用线程保护。

完整 Windows suite2 含独立 Core/实际 DLL、652 项 CommonMark 解析/来源/原生往返与差分（不是 HTML/display conformance）、284 导出且无旧 Rich API、Renderer/Editor、样式/规模/fractional/表格、剪贴板/对象、MessageList 和原生同步/异步/缺图绘制。SOURCE/LIVE 和字体生命周期的最后补充分别另以 document-final2/native-final 验证；最终产品执行代码在 suite2 自动重建后未改。原有性能断言保持：列表来源 Core/DLL 7,129 次故障矩阵、候选单次解析与 MessageList 改宽 shaping 786528/655376。

ensure_xge_dll.bat 现检查上游 .cc/.hh 和独立对象脚本；已有 build/document/xge.dll 会刷新，避免其在 Windows DLL 搜索顺序中遮蔽新标准 DLL。最终两份文件 SHA256 均为 9CC6A2C14D6A4D3C9C617B504FDC4AB425BBD2BD8DB5565814A46790978254D8。可选 WebView DLL 单独重编译且通过内部渲染；没有扩展通用 WebView 对外通信。

Linux 只证明依赖对象与字体数据可移植，不包含原生 XGE GPU、Doc UI、IME 或新增 sanitizer 证据；Doc core 本批未改，既有 Linux Core/652 与 UI sanitizer 证据见上一批。本批逻辑 LTR item 整形不能代替完整段落 Bidi，RTL、视觉导航、上下文字体回退和多字形 cluster 的 GDEF 映射仍待完成。公开 GlyphRun ABI 已变化，调用方必须重新编译。整体长期目标保持 active。
最后按正式 build_document_opentype_test.bat 重新运行本批完整原生专项，release-probe.log 退出码零；所有本批进程已收尾。API coverage --no-write 与 comment lint 分别在 api-coverage.log / comment-lint.log 通过，基线未写入。最终 tracked diff --check 及本批新增文本 no-index --check 无 whitespace 诊断（使用仓库认可的 CR 行尾；final-whitespace.log 为空）。第三方原包保持未修改，未将上游源码换行改写为项目格式。
## R1 共享段落方向与原生 RTL 基础（2026-10-01）

本批产品源码和公开字形 ABI 已同步构建；这是 Document 接入方向排版的基础，不是完整混合方向 Document/Editor 交付。所有以下验证进程已结束且退出码零。日志在 artifacts/xui-document-rebuild/，统一前缀 validation-2026-10-01-bidi-：

| 验证 | 日志后缀 |
| --- | --- |
| 完整 Windows Document suite（含新增方向/RTL 探针） | suite2.log |
| 原生 RTL 最终专项（补齐真实字体括号镜像） | native-final2.log |
| Unicode 17 两套完整语料、所有失败点、Linux ASan/UBSan | corpus-linux-final.log（最终源码重跑另见 corpus-linux-final2.log） |
| Linux 无窗口 Renderer / View+Editor / MessageList ASan/UBSan | linux-renderer.log / linux-view.log / linux-message.log |
| 99 个 vendor 文件精确允许修改复核 | vendor.log |
| 真实对象编译 -O2 -fanalyzer | analyzer.log / proxy-analyzer2.log |
| 既有字体 / 代理烟测 | text-foundation.log / proxy.log |
| 可选 Windows WebView 基础控件及浏览器进程生命周期 | webview-widget.log |
| Document provider 和 MessageList 故障卡片/恢复 | web-provider.log |
| 私有 KaTeX/Mermaid/HTML 测量、沙箱及 3 张 500×350 PNG | web-render.log |
| API 检查（不写 baseline） | api-coverage.log / api-lint.log |
| DLL 动态依赖、最终 whitespace | imports.log / final-whitespace.log |

Windows suite2 精确跑完 91,707 条 BidiCharacterTest 与 770,241 条 BidiTest class/direction cases（共 861,948 条），也包含独立 Core/实际 DLL、652 项 CommonMark、284 项 Document 导出/无旧 Rich API、Rich/Markdown Renderer/Editor、样式/规模/fractional/表格、剪贴板/对象、MessageList 和原生同步/异步/缺图绘制。CommonMark 检查仍是解析/来源/原生往返与差分，不是 HTML/display conformance。之前的性能断言没有放宽，MessageList 改宽 shaping 仍为 786528/655376。新段落系统还没有参与这些 Document 正文布局。

UBA 桥接使用逻辑 UTF-8 byte offsets，比较整段基向、L1 后逐字符级别和排除 X9 后的完整 L2 视觉序列；BidiTest 的 23 个代表类包含多字节段分隔、R/AL 和隔离符。额外验证严格 UTF-8、P1/CRLF、NUL、复制输入、释放输入和 kernel 后独立 line 所有权、L1 不改 paragraph、镜像、边界拒绝及深隔离/括号。两组独立逐点扫描包含创建 19/29 点、line 5/5 点，共 58 点，均传播 OOM 且输出为空，已保留结果、存活对象数和字节数不变，最后为零。Linux 加载同一语料并启用 ASan leak/halt、UBSan halt，无 sanitizer 诊断。

首次 core.log 出现无输出的 Windows access violation，linux.log / 带 stacktrace 的诊断定位到上游 SBParagraph 的 _algorithm 未初始化：ProcessParagraph OOM 时 finalizer 读取了无效引用。修复后首套语料通过。追加 BidiTest 后 corpus2.log 的 record 208322 / S RLE PDI R / LTR 暴露 L1 run 在 PDI 的 UTF-8 continuation 内断开；现 L1 不跨非行尾空白/隔离字符携带待处理 BN 链。core/corpus/linux 的各阶段失败日志保留。corpus4.log 的深层 OOM 测试失败来自错误假定第一条镜像一定为右括号；改为保存真实 baseline mirror 并验证其不变，同时比较各 run 字段而非含 padding 的结构 memcmp。语料期望和分配预算没有放宽。

vendor.log 比较固定 v3.0.0 原包的 99 个 Source/Header/LICENSE 文件，只允许相对 include 改写和上述两项修复；不宣称 vendor 未修改。archive SHA256 为 4C3EBD5DCC3424A20A47E0337DB65E19058FF2C7448E66B11FE082C18EE5462E。Unicode 17 character corpus SHA256 为 A3E6E905AB5AFBE318A96DF5401D0372A04CD73EF139AB5E3CF0AE241C255488，class corpus 为 888BDFC8090652272D1F859CDB00AE659E2DC6C26740BE61EF1D03998A687620。全部原始 header/许可保留；源码和本地修复说明见 lib/sheenbidi/README.xge.md，没有向外部提交。

native-final2 用项目自有希伯来 GSUB/GDEF/GPOS fixture，在 40/37 字号对比独立 HarfBuzz 的物理 glyph/advance/offset/pen 与 XGE 的逻辑 cluster 存储。真实 Segoe UI 阿拉伯 glyph 不等于名义字符 glyph，输出与上游一致；RTL 括号使用一次 HarfBuzz 镜像而来源 codepoint/offset 不变。原生探针包括逆向非均匀停点、附标、CRLF、有限/无效/巨大坐标命中、proxy 停点、缺少整形能力时的 UNSUPPORTED、三色 GPU 分段、完整 alpha、局部下划线、右对齐、物理 clip 恢复以及 DrawContext/Spans/Surface 入口一致。negative-oracle.log 的 test-only 忽略 RTL 代理被实际像素一致性断言拒绝（退出码 1 为预期）；不把这个注入测试称为旧 DLL 对照。

suite.log 最终停止于测试重设已初始化的 context proxy；native.log 随后停止于把物理 clipGet 与未取整 float 输入比较。两者均是测试约定错误，现分别使用独立 context 和绘制前实际物理 clip baseline；最终 native-final2 / suite2 全部通过。代理烟测没有绘制 backend，其日志明确跳过 span pixels；实际 GPU 证明来自 native-final2 和 Document OpenType 专项。proxy-analyzer.log 仅因既有公共私有头的 unused-function 告警而失败，按既有工程标志加 -Wno-unused-function 后 proxy-analyzer2 实际编译为空；桥接自身和 vendor 隔离诊断策略没有被额外放宽。

标准 DLL 与 build/document/xge.dll 的 SHA256 均为 80E9A1123A62CA7CBBD504CD749BF78C36414DB31CA5C5CCE35C8B324AE399AC。imports.log 只有系统 C/Win32 依赖，没有新增 C++ 动态 runtime。可选 WebView DLL 已独立重编译；控件 12 轮、独立双控件、隔离浏览器退出/重建、内部 provider 失败卡片/恢复及离线公式/Mermaid/HTML 渲染通过。通用 WebView 对外仍只实现 Windows 基础网页承载，其他平台预留，内部脚本/测量/截图保留。

本批尚未把 UBA 和 RTL items 接入 Document 段落 itemization、逐行视觉索引、选区/命中、SOURCE/LIVE 和视觉方向键；通用 TextLayout 仍是原逻辑布局。Linux UI 验证使用无窗口代理，依赖测试和 UBA conformance 不代表五平台原生字体/GPU/IME/读屏/DPI 已完成。复杂 cluster 的 GDEF 映射、上下文字体回退、K2 完整 CST 和 K4/E1/E2 性能/交互等继续。整体长期目标保持 active。

## R1 Document 双向文本与按需续排接入批次（2026-10-01）

本批把上批共享方向桥接接入实际 Document；没有增加公开 Document API，逻辑位置与原文不倒置。通用 WebView 仍只有 Windows 基础功能，其他四个平台预留；Document 内部脚本、测量、截图保留。以下日志均位于 `artifacts/xui-document-rebuild/`，统一前缀为 `validation-2026-10-01-document-bidi-`，列出的验证已退出码零：

| 验证 | 日志后缀 |
| --- | --- |
| 最终默认 DLL | verified-build.log |
| 完整 Windows Document suite | verified-suite.log |
| 原生 Document Bidi 专项（此前独立运行） | continuation-native.log |
| 原规模专项及 RTL 全行上下文 oracle | continuation-scale3.log |
| Linux 861,948 条 Unicode 17 语料 / ASan、UBSan | verified-linux-corpus.log |
| Linux 无窗口 Renderer | verified-linux-renderer.log |
| Linux View/Editor、128 KiB SOURCE/LIVE 收拢预算 | verified4-linux-view.log |
| Linux MessageList | verified2-linux-message.log |
| 初始 22 点 / 续载 100 点方向 OOM、所有权与回滚 | continuation-failure3.log |
| 实际 -c -O2 -fanalyzer 编译 | final-layout-analyzer.log / verified-renderer-analyzer.log / verified-view-analyzer.log / verified-editor-analyzer.log |
| API coverage --no-write / lint | verified-api-coverage.log / verified-api-lint.log |
| 固定 vendor 99 文件允许修改复核 | verified-vendor.log |
| Windows 基础 WebView 12 轮 / 双控件 / 进程故障恢复 | verified-webview-widget.log |
| Document provider / MessageList 错误卡片与恢复 | verified-web-provider.log |
| 离线 KaTeX/Mermaid/HTML 测量、沙箱与 PNG | verified-web-render.log |
| DLL 系统动态依赖 | verified-imports.log |

完整 Windows suite 覆盖独立 Core/实际 DLL、652 CommonMark 解析/来源/原生往返与差分、284 Document 导出/无旧 Rich API、Renderer/Editor、OpenType 与新 Document Bidi、样式/规模/fractional/表格、剪贴板/对象来源、MessageList 和三项原生绘制烟测。CommonMark 仍不是 HTML/display conformance。Unicode 17 character 91,707 条、class/direction 770,241 条共 861,948 条，逐项核对段落基向、L1 byte levels 与 L2 视觉次序；两个 kernel OOM fixture 是 create 19/29、line 6/6，共 60 点，最后 live owners/bytes 为零。本批 line 自有 byte-level 数组使其较上批各增加一个分配点。

Document 原生专项使用项目自有 GSUB/GDEF/GPOS 字体与真实 Segoe UI，40/37 字号各跑 Spans 和 DrawText-only 两种后端。核对 Rich joined/split 三色与 Markdown emphasis 的相同 alpha、逆向非均匀 caret 停点、局部下划线、方向边界两种 affinity、视觉折行、分离选区、隔离符、HardBreak 段落基向、RLO 下的 discretionary hyphen、RTL 代码/源码、Math 对象合法 GAP，以及 Script 分组后的 Hebrew/Arabic 原生像素。Editor 验证基础视觉左右/Home/End/Shift、RTL 同行选区收拢、逻辑 Backspace/Delete 和 Undo；未据此宣称 Ctrl 词导航或全部复杂跨行行为完成。

长方向段落保留完整 UBA 分析而只逐行续排，所有者计入布局缓存。Linux 新专项扫描全部初始 22 个方向分配点与全部续载 100 个 line 分配点；失败后逐字节核对已发布 fragments、lines、visual permutation 和 paint strings，内核所有者数与保留字节不变，重试成功、已提交行不变，最后释放为零。另覆盖首个 RTL 表格单元格之后的长 LTR 单元格视觉数组扩容、Math GAP、全部不可见方向控制、宽度往返 seed 数量不增长。128 KiB SOURCE/LIVE 全选后 Left 只收拢至起点，整形计数不增加；LIVE 同活动范围不做无意义远端 caret anchoring。

规模专项保留原 Arabic prefix 的输入预算和 lazy/full 几何断言：79,048 字节初始新 run 输入 16,393 字节，整形 49,236 字节；没有安全 cut 的 Arabic 文本仍输入全文。受保护 16 KiB RTL CodeBlock/SOURCE 需要原始 run 度量、方向 seed 和最终方向整形三次；原单次 LTR 计数断言改为逐次完整行 oracle、至少一次 RTL、最多三次，实际两次 RTL，任何人工切小段立即被代理拒绝。其他按需输入预算、缓存上限和 MessageList 改宽 786528/655376 未放宽。

失败阶段保留：native2 暴露 RLI/PDI 被画成 .notdef；invisible-range-before 暴露零宽控制割裂 select-all 矩形，均为产品修复。native3 的 Hebrew SHY 期望违反 HL/BA/HL 不断行规则，改用 RLO Latin 样本；native4 的真实脚本 oracle 忽略 Document 最终像素取整，修正独立基准。final2-suite 发现完整 RTL 布局违反原 prefix 预算，修复为保留 UBA 的 continuation。continuation-scale/scale2 记录旧单次整形断言，scale3 在新整行 oracle 下通过。continuation-failure 首次逐点测试包含每轮深处完整布局，耗时过长后终止；failure2 fixture 长度低于 32 KiB continuation 门槛，failure3 增长至门槛以上并覆盖全部 100 点。verified2-linux-view 的新测试错误设置不存在的 View iMode 字段，verified3-linux-view 缺少冷布局 priming；verified4 修正 fixture，Source/Live 预算和完整 UI 回归通过。此前 failed/stage 日志不是最终验收证据。

本批 Linux 验证使用无窗口代理，未验证平台 WebView、真实 GPU、PNG 剪贴板、IME、读屏或物理 DPI。View/MessageList 编译仍有既有 date-picker 的 maybe-uninitialized 警告，沿用脚本原有非致命策略；改动的 Layout/Renderer/View/Editor 实际 -fanalyzer 编译均无诊断。vendor 精确比对固定 v3.0.0 原包及上批两项修复，第三方边界未扩大。API coverage baseline 未写入，标准 DLL 和 Document-local DLL SHA256 均为 `D68C6FF153737652566DFFCA9E68F5709349D584CF69483EB0C028E66D0F2ACB`，动态导入仅系统 C/Win32 DLL，没有新增 C++ 动态 runtime。

可选 Windows DLL 已以当前源码重建；provider 与 MessageList 浏览器故障/恢复、离线三张 500×350 PNG 和 HTML 沙箱通过。`verified-webview-exports.log` 以 `XUI_WEBVIEW_TEST_EXPORTS=0` 重新构建生产配置，确认基础网页承载和 Document provider 导出存在，14 个内部浏览器符号全部未导出，退出码零。最终 tracked diff 与本批新增文本的 `verified-whitespace.log` 为空；使用仓库认可的 CR 行尾和 safecrlf 检查设置，未改写第三方原包。所有本批进程已结束，产品源码自最终 build 后未改。通用 TextLayout、完整 Script_Extensions/语言 itemization、上下文字体回退、多字形 cluster GDEF、K2 完整 CST、性能及真实平台验收继续；整体长期目标保持 active。

## R1 连字附标停点与 GPOS 度量批次（2026-10-01）

本批实现使用 GDEF 唯一 carrier 的完整字素停点，并计入 GPOS pen/x_offset；零 advance GDEF MARK 可同处一个 cluster，RTL 输出前缘距离。多个 carrier、spacing mark 或不完整/异常列表整体均分。坐标约定依据 [OpenType GDEF](https://learn.microsoft.com/en-us/typography/opentype/spec/gdef) 和 [HarfBuzz ligature caret API](https://harfbuzz.github.io/harfbuzz-hb-ot-layout.html#hb-ot-layout-get-ligature-carets)，后者返回未整形坐标，需加入整形后的 placement。没有变更公开接口或本批 ABI。

Document 联合 RTL 度量现保留 DEFAULT 的字距选项，与实际绘制一致。完整单段 LTR 缓存绘制重用已有 ink width，使用统一整数矩形转换，保留逻辑 advance 外的右侧字形；光标、断行、选择与装饰仍遵循逻辑 advance。行滚动范围不会用旧缓存覆盖已有联合 paint group 的宽度。本批不增加重复 shaping，也未放宽原按需预算。

日志均位于 `artifacts/xui-document-rebuild/`，下表省略共同前缀 `validation-2026-10-01-gdef-mark-`：

| 检查 | 最终日志 | 退出码 / 结果 |
| --- | --- | --- |
| 默认 DLL 构建 | `release-build.log` | 0 |
| Windows 完整发布套件 | `release-suite.log` | 0 |
| 最终 DLL 隔离原生字体专项 | `native-release.log` | 0 |
| 实际 C mapper / 真实字体 ASan + UBSan | `linux-carets-release2.log` | 0 |
| 实际 C mapper GCC 16.2 / 15.2 analyzer | `mapper-analyzer-windows-final2.log` / `mapper-analyzer-linux-final2.log` | 0 / 0，均无诊断 |
| Layout / Renderer 独立 C TU | `document-tu.log` | 0，无诊断 |
| Linux Renderer sanitizer | `linux-renderer-final.log` | 0 |
| Linux View / Editor sanitizer | `linux-view-final.log` | 0 |
| Linux MessageList sanitizer | `linux-message-final.log` | 0 |
| Document Bidi 故障注入 sanitizer | `linux-bidi.log` | 0 |
| Windows 基础 WebView 控件 | `webview-widget.log` | 0 |
| Document 内部 provider / MessageList 故障恢复 | `web-provider.log` | 0 |
| 离线 KaTeX / Mermaid / HTML / PNG | `web-render.log` | 0 |
| 生产 WebView 导出边界 | `webview-exports.log` | 0，14 个内部符号未导出 |
| API 文档 coverage / lint | `api-coverage.log` / `comment-lint.log` | 0 / 0，baseline 未写入 |
| 项目字体确定性重建 | `fixture-reproducibility.log` | 0，SHA256 相同 |
| DLL 动态依赖 | `imports.log` | 0，仅系统 C / Win32 DLL |

项目字体增加 IgnoreMarks 连字、Latin/Hebrew carrier 的 GPOS x placement 与 advance 调整、一个再次展开成两个非 MARK carrier 的 GSUB 歧义样本；旧 glyph ID 和基本 GDEF 数据保持。重建前后 TTF SHA256 均为 `FDD2A74C24E220A0672EA9F9A74AFAC40BC9F82C24A95D5DAE74045D408984D9`，没有引入外部字体字节。

产品 mapper 抽取到同一份 `.inl`，Linux 测试直接包含它，用真实 HB 结果而非复制算法。十组样本覆盖 LTR/RTL、一个/多个附标、kern 开关、缺失/数量不符/越界/多 carrier/spacing mark；26 个实际分配失败点与两次已有停点追加失败验证旧数组、数量、字节和所有者不变，重试恢复且最后为零。新测试分配器用独立固定所有者表核对指针，不为计费增加堆分配。mapper 与 libunibreak C 在 ASan/UBSan 下执行；HB 沿用独立依赖探针的缓存对象，本项不声称第三方所有路径都经过 sanitizer 或证明平台 GPU/IME。

Windows 共享字体测试在 40/37 字号分别使用 Spans 与 DrawText-only。完整 Text、跨四个颜色节点且附标自身独立的 Rich Text、Markdown emphasis、SOURCE 与 LIVE 对照同一轮廓和 GDEF 停点。Latin marked ffi 的字体单位停点为 320/870、整形 advance 900；Hebrew marked ligature 的 RTL 停点为 130/680、advance 900。核对 carets、HitTest、首字素选区宽度、宽度往返、Editor 视觉箭头、完整字素 Backspace 和 Undo，以及完整 RGBA alpha、三种颜色的物理边界。Source/Live 也逐像素核对 alpha。额外验证只露出逻辑终点外右侧 ink 的视口，以及窄布局仍保留该 ink 的滚动宽度；没有用代理像素代替真实 GPU。

Linux Renderer、完整 UI View/Editor 和 MessageList 使用既有无窗口代理与 leak/halt sanitizer，保留原跨对象/跨字体/跨上下标、联合断行收敛、ligature、源码/Live 和规模预算。Bidi 专项扫描初始 22 点与续载 100 点，比较失败恢复后的 fragments、visual order、lines、paint strings 与方向内核所有者。本批没有修改核心或 UBA 实现；完整核心和 652 CommonMark / 861,948 Unicode 17 Bidi 项通过 Windows 发布套件验收。CommonMark 仍为解析/来源/原生往返，不能替代 HTML 或完整排版 conformance。Linux UI 编译保留脚本既有 date-picker 非致命 maybe-uninitialized 警告策略，未扩展成无警告宣称。

失败阶段保留：`before.log` 的旧 DLL 漏用附标连字 GDEF；`document.log` / `document2.log` 在逻辑 advance 外漏画，`document3.log` / `document-diagnostic2.log` 暴露整数矩形直接赋值截断，均由产品修复。初次 `linux-renderer.log` 暴露完整缓存宽度误用于已联合重整形的对象分隔行，最终通过。早期诊断探针曾引用错误字体字段或用浮点格式打印整数，已移除临时 wrapper；`tu.log` 错把引擎聚合 include 当独立 TU，不能作为产品编译证据。早期 analyzer 在测试带头分配器上报空指针/泄漏等路径；补齐 reserve 的非空存储前提、避免 typed pointer 通过 void** 写入、改用独立测试所有者表后，两平台最终 analyzer 和全部 OOM sanitizer 通过，未据此宣称曾存在已证明的产品泄漏。

可选 Windows DLL 已按本批源码重建。基础控件 focus/layout、十二次生命周期、双实例与隔离浏览器进程异常退出后重建通过；Document 内部测量/截图、KaTeX/Mermaid/HTML、主题变化、截图 decode/crop 错误、MessageList 浏览器故障卡片与三次重启预算通过。离线三张 500×350 PNG 与 HTML 沙箱通过。生产配置 `XUI_WEBVIEW_TEST_EXPORTS=0` 保留基础网页承载与 Document provider，14 个内部浏览器符号未导出。通用 WebView 仍仅 Windows 基础功能，其他平台预留，未新增公共脚本或数据通信。

默认、Document-local 与隔离原生专项 DLL SHA256 均为 `C7744701CCE67AE5AC63B093AF8D5A14BB0B6480526320F41388EBA97195702B`，见 `build-hashes-final.log`。全部本批进程已结束；产品实现自 `release-build.log` 构建后未变，完整发布套件使用包含 Source/Live 像素断言的最终共享测试。tracked `whitespace-final.log` 与本批新增文本 `whitespace-new.log` 均无诊断，仓库原有行尾规则保留。GDEF Format 1 的本批唯一 carrier 附标场景已实现；多个 carrier 的源组件映射、contour-point 格式及 device/variation 调整、完整 CST、Script_Extensions/语言/上下文字体回退、复杂导航与性能、真实五平台 GPU/IME/读屏/物理 DPI 仍未全部完成。整体长期目标保持 active。

## R1 轮廓点停点与复合 TrueType 轮廓批次（2026-10-01）

实现边界依据 [GDEF Format 2](https://learn.microsoft.com/en-us/typography/opentype/spec/gdef)、[TrueType 原始点及复合字形](https://learn.microsoft.com/en-us/typography/opentype/spec/glyf) 和 [HarfBuzz contour-point callback](https://harfbuzz.github.io/harfbuzz-hb-font.html#hb-font-get-glyph-contour-point)。固定上游 OT 字体没有默认轮廓点回调，Format 2 不保留回调失败状态；不能把返回零视为合法与否的判断。产品有界 C 读取器保留 off-curve 原始编号，静态未 hint 的 TrueType 通过不可变 OT 子字体提供原始点，GDEF 全列表验证后采用，缺点/未知格式整体回退且合法零坐标保留。共享轮廓转换修正复合矩阵/offset/真实点附着，CFF 沿用原绘制；未改动上游 HB/stb 或公开 API/ABI。

日志位于 `artifacts/xui-document-rebuild/`，下表省略共同前缀 `validation-2026-10-01-gdef-points-`：

| 检查 | 最终日志 | 退出码 / 结果 |
| --- | --- | --- |
| 最终默认 DLL 构建 | `final-build.log` | 0 |
| Windows 完整发布套件及新轮廓点原生专项 | `final-suite.log` | 0，含 Core/DLL、652 CommonMark、861948 Bidi、新字体原生专项及完整 UI |
| Windows 实际 C 读取/转换及真实字体 | `contours-windows-final.log` | 0 |
| Linux 同一 C 读取/转换 ASan + UBSan | `linux-contours-final.log` | 0 |
| 两套 GDEF 字体 / 实际 C mapper ASan + UBSan | `linux-carets-final.log` | 0 |
| 轮廓读取/转换 GCC 16.2 / 15.2 analyzer | `contour-analyzer-windows-final.log` / `contour-analyzer-linux-final.log` | 0 / 0，无诊断 |
| GDEF 验证/mapper GCC 16.2 / 15.2 analyzer | `caret-analyzer-windows-final.log` / `caret-analyzer-linux-final.log` | 0 / 0，无诊断 |
| 可选 Windows 基础 WebView 重建与生命周期 | `webview-widget.log` | 0 |
| Document 内部 provider / MessageList 浏览器故障恢复 | `web-provider.log` | 0 |
| 离线 KaTeX / Mermaid / HTML 与 PNG | `web-render.log` | 0 |
| 生产 WebView 导出边界 | `webview-exports.log` | 0，14 个内部符号未导出 |
| API coverage / comment lint | `api-coverage.log` / `comment-lint.log` | 0 / 0，baseline 未写入 |
| 两套字体与坐标/展开配对确定性重建 | `fixture-reproducibility.log` | 0，四个文件 hash 均保持 |
| 默认 DLL 动态依赖 | `imports.log` | 0，仅系统 C / Win32 DLL |

新项目自有字体以真实 off-curve 点提供 fi/ffi 的 120、250/800 非均匀停点，Greek 原始点经过复合平移得到 300，Hebrew 复合字形保留 250/800；GPOS placement、zero-advance 附标和 RTL 的原期望保持。独立 FontTools 在序列化后导出 502 点坐标、on/off 状态和轮廓终点，实际 C 在短/长 loca 两个编码逐点一致。十组复合/简单特殊轮廓的六个比例共 60 张位图，与独立 FontTools 展开后原 stb 绘制的全部像素完全相同。实际 DLL 另做相同 60 组独立展开比对，验证公开 RasterizeByIndex 已使用新转换。普通字体抽样 Calibri 187、Segoe UI 179、DejaVu 209 个简单字形，与原 stb 独立绘制逐像素相差最多一个 alpha 级别；封闭轮廓起点旋转会改变浮点边的求和次序，此容差不用于项目夹具。

读取/转换的全部 88 次分配失败要求结果为 OOM、输出 NULL/count=0、所有者零；压缩重复、截断、loca/glyf 越界、循环、非法复合 flags、工作量上界和逐字节变异同样通过实际 C 路径。没有把低层 rasterizer 所有内部失败路径或完整 SFNT 输入当成已经审计。GDEF actual-C mapper 对原 Format 1 和新 Format 2 字体各执行十组 LTR/RTL、附标、GPOS、歧义/spacing fallback，分别 26 次分配失败和两次追加回滚。C 读取器、转换、GDEF 验证/mapper 以及 libunibreak 经过 sanitizer；HB 沿用已验证依赖的缓存对象，不声称上游全部路径也带 sanitizer。

原 Windows 共享字体测试另按 `XGE_TEST_CONTOUR_POINTS` 编译，保持原期望值与独立 HB glyph/advance/offset oracle；Rich joined/split 三色、Markdown emphasis、VISUAL/SOURCE/LIVE、40/37 字号及 Spans/DrawText-only 的实际 GPU alpha、caret/hit/range、装饰、reflow、视觉导航、逻辑删除与 Undo 均保持。原测试夹具也继续执行，新专项已加入默认 suite。实际 DLL 从新鲜 FontLoadMemory 检查非法第二轮廓点或未知格式导致两个停点整体等分，合法 x=0 原始点保持有效；每次使用新的 immutable HB face，不在已缓存的字体上修改数据。

失败阶段保留：旧 C7744701 DLL 的 `before.log` 不满足同一 Format 2 原生断言；`native.log` 是早期新 DLL 的专项通过，最终默认 DLL 的证据以 `final-suite.log` 为准。开发时独立像素 oracle 暴露隐含中点四舍五入与原 stb 向下取整的差异，已保留旧取整。第一轮真实字体探针把零面积轮廓当成失败，修正为合法空绘制后 Windows/Linux 通过。首次 Linux mapper analyzer 在 need=0 的抽象路径报 NULL；补齐负数量及 reserve 返回空存储检查后两平台最终无诊断，不据此声称已证明有效输入存在产品崩溃。早期 generator 的共享 glyphOrder 列表导致重复名称，修正后四个输出稳定；上述阶段不替代最终证据。

原字体 SHA256 `FDD2A74C24E220A0672EA9F9A74AFAC40BC9F82C24A95D5DAE74045D408984D9` 保持；新字体为 `A5C0E7FE1E1B5E76DAB7AE587AF8577C41760EF3F2E48C0E100CCC9AA7024A34`，均由项目生成代码创建，不含外部字体字节。默认与 Document-local DLL SHA256 均为 `AD245B6B894B5ABBD0F2BC3DEA8A45253C68D25B98D547DCCA8823A5A061B6C0`，见 `build-hashes.log`。

通用 WebView 保持仅 Windows 基础承载、其他平台预留；保留 Document 私有 KaTeX/Mermaid/HTML 脚本、测量、截图，生产 `XUI_WEBVIEW_TEST_EXPORTS=0` 重建确认 14 个内部符号未导出。实际 provider 与 MessageList 三次浏览器重启/预算及离线 500×350 三张 PNG、HTML 沙箱通过；本批没有扩大公共通信接口。Linux 无窗口 C 验证不替代原生 GPU、WebView、IME、读屏或 DPI。

未验收项仍包括 CFF 原始轮廓点、hinting/变体轮廓、phantom-point 复合附着、超出 64 层/工作量上界的字形、device/variation、完整多 carrier 映射、完整 CST、Script_Extensions/语言/上下文字体回退、复杂导航与性能及真实五平台验收。整体目标保持 active。

本批所有构建、analyzer、sanitizer、原生与 Web 验证进程均已终止，最终结果均为零；产品源码自 final-build 后未改。最终 tracked 和新增文本差异检查见 whitespace-final.log / whitespace-new.log，均无诊断。

## R1 完整字素与同脚本词段字体回退批次（2026-10-01）

本批证据统一在 `artifacts/xui-document-rebuild/validation-2026-10-01-font-fallback-*`。产品新增实际 C SFNT 字体选择器与固定默认可忽略数据；FontFace/HB/最终 GlyphRun/Document 的整形和所有权仍共用原生链路。最后一个产品修复为 XUI 代理字号实例创建的失败回滚；此前的 `release-suite.log` 是已通过阶段，最终产品以 `atomic-suite.log` 为准。

| 检查 | 日志后缀 | 结果 |
| --- | --- | --- |
| 旧 C7744701 DLL 对完整基字/附标的同一字形 oracle | `before.log` | 1，预期缺陷反证 |
| Windows 实际 DLL 完整字体 glyph/GPOS/GDEF/RTL/NFC/生命周期 | `native2.log` | 0，九组完整回退 oracle |
| Windows 原生 Document Spans/DrawText-only 与放大字号 | `document-native.log` / `document-draw-only.log` | 0 / 0 |
| 最后代理回滚修复后的默认 DLL | `atomic-build.log` | 0 |
| 修复前完成的完整 Document 发布套件 | `release-suite.log` | 0，不替代最终 DLL 验证 |
| 最终默认 DLL 的完整 Document 发布套件 | `atomic-suite.log` | 0，652 CommonMark、861948 Bidi、DLL/编辑器/MessageList/原生绘制 |
| 实际 memory-debug DLL 的三个字号创建分配失败点 | `oom-final.log` | 0，输出 NULL、回滚/重试、原字体可用、零活动分配 |
| Linux 实际 C 选择器与 libunibreak ASan/UBSan | `linux-final.log` | 0，40000 字节词段 50002 次字体访问 |
| Windows GCC 16.2 / Linux GCC 15.2 analyzer | `analyzer-windows-final.log` / `analyzer-linux-final.log` | 0 / 0，无诊断 |
| 最终 Windows WebView widget | `atomic-webview-widget.log` | 0，focus/layout、12 生命周期、双视图、浏览器失败后重建 |
| 最终 private Document provider/MessageList | `web-provider.log` | 0，错误反馈、三次浏览器重启、恢复及预算 |
| 离线 KaTeX/Mermaid/HTML 与 HTML 沙箱 | `web-render.log` | 0，三张 500×350 PNG |
| 生产 `XUI_WEBVIEW_TEST_EXPORTS=0` 重建 | `webview-exports.log` | 0，14 个内部符号未导出 |
| 原生 emoji 功能/位图 | `emoji.log` / `emoji-render.log` | 0 / 0，1923 条位图记录 |
| API coverage（no-write）/comment lint | `api-coverage.log` / `comment-lint.log` | 0 / 0 |
| 六个字体子集和默认可忽略数据确定性重建 | `fixture-reproducibility.log` | 0，七个文件内容相同 |
| 可忽略表相同内容再生成 | `generator-mtime.log` | 0，字节与 mtime 都保持 |
| tracked / 本批新增文本 whitespace | `whitespace-final.log` / `whitespace-new.log` | 0 / 0，无诊断 |

真实 DLL 使用 cmap 子集保留相同 glyph ID 和完整字体的轮廓/GSUB/GPOS/GDEF；删除基字、附标或连字组成字符后，最终字形、cluster、advance、placement、caret 和字体身份必须与直接使用完整回退字体一致。九组包括 Latin/Greek/Hebrew RTL、组合附标、fi/ffi 与带附标的连字。字体选择器缺名义 cmap 时实际执行 HB 规范化探测，规范化可组成的主字体保持主字体身份；joiner/variation selector 不被当作必须具有可见 cmap 的字符。run 持有最终字体，调用方与根字体提前释放后仍可测量，释放 run 后引用归零。

原生 Document 同时验证 Rich 单节点/跨颜色节点、Markdown emphasis 与 VISUAL/SOURCE/LIVE、40/37 字号、Spans/DrawText-only。按真实 XGE 代理调整至 60/55.5 后，对 LTR/RTL 继续核对 GPU alpha、caret/hit/range、reflow、基础视觉导航、逻辑 Backspace 与 Undo。不同颜色可保留同一整形组；这些结果不代表不同字体/样式组之间已有完整上下文传递。

独立 memory-debug DLL 直接编译同一 XGE/XUI C，不以 mock 代替 FontCreateSized。注入包装对象、主字体实例、回退字体实例三个实际分配失败点，每次要求 OOM 与输出 NULL、活动分配回到原基线，失败后原字体仍能测量；无故障重试的 ffi 指标与完整字体一致，全部释放后 XRT 活动分配/字节、非法释放与双重释放均为零。默认 profile 不启用该调试模块。HB 使用自己的分配路径，零 XRT 活动分配不能单独证明整个上游 HB 的故障覆盖。

无窗口 sanitizer 直接包含产品选择器并链接真实 HB 对象与 libunibreak C，覆盖完整字素/词段、顺序回退、主脚本与标点边界、NFC、GPOS 字体选择、不可完整匹配的降级、Unicode 默认可忽略属性以及映射分配/字体创建失败。40000 字节不匹配词段的 50002 次访问验证失败词段边界被缓存；不是全面性能基准。Windows/Linux analyzer 均编译实际 C，最终空日志；HB 缓存对象没有重新以 sanitizer 编译。

失败阶段保留：`native.log` 因粗略 emoji 起点范围把 U+200D 截出 OT item 而失败，修复后 `native2.log` 及最终 suite 通过。最初 Linux 独立测试有 misleading-indentation 编译诊断及标点字体预期错误，修正测试后 `linux-final.log` 通过，未据此虚构产品缺陷。最初 `oom.log` 的活动分配基线包含未清除的线程错误对象，补齐 xrtClearError 后 `oom-final.log` 通过；实际代理回滚修复已经在两次 OOM 验证所用 DLL 中。阶段日志不替代最终成功证据。

默认与 Document-local DLL SHA256 均为 `A195004E9CE8C59CAB33C5B2AB98EB4AB39F23A538CFAB524A77605A292E3A3C`；独立 memory-debug DLL 为 `79428680BAE2AF9FE3B050A2B0E93B5FBA9B125D534E4EDF9183841B20C4330E`。原项目字体仍为 `FDD2A74C24E220A0672EA9F9A74AFAC40BC9F82C24A95D5DAE74045D408984D9`；生成的默认可忽略表为 `34850B1344C312C33B6F0632B0420DCCA88CADBAEF7D7370F69F423C5B831CB5`。最终 hash 见 `atomic-build-hashes.log`；字体子集各自 hash 在确定性日志中。UCD 源 SHA256 为 `24c7fed1195c482faaefd5c1e7eb821c5ee1fb6de07ecdbaa64b56a99da22c08`，使用 Unicode License V3，来源与生成器在 `.inc` 中标明。

本批只完成 SFNT 主字体上的完整字素/同主脚本词段回退。词段启发式、既有 Unicode 15.1 字素表、未完成的 Script_Extensions/语言/跨样式组上下文、位图主字体及混合回退、多字体附标合成明确保留。完整 CST、多 carrier、device/variation、复杂导航与性能、真实五平台 GPU/IME/读屏/物理 DPI 继续；无窗口 Linux 不替代平台验收。通用 WebView 仍仅 Windows 基础承载，其他平台预留；本次保留并复核 Document 私有公式/Mermaid/HTML 通道，未扩展公共数据通信。整体目标保持 active。

最终 suite 已正常结束，退出码 0；所用默认与 Document-local DLL 的 hash 在全部检查结束后再次一致。本批没有遗留运行中的构建或验证进程，最终产品源码自 `atomic-build.log` 对应构建后未改；后续只补齐测试中的线程错误清理、生成器的相同内容保留时间和文档。成功表中所有最终构建/运行结果均为 0，开发失败阶段保留且不作为验收证据。

## R1 Unicode 17 字素内核统一批次（2026-10-01）

日志前缀：`artifacts/xui-document-rebuild/validation-2026-10-01-grapheme17-`。导航、实际布局字素 map、SFNT 回退和 GDEF caret 直接编译同一 `xge_unicode_grapheme.h` 状态机/属性；不以 mock 算法替代产品 C，也没有升级 UAX #14 或把 Script_Extensions/语言划分记为完成。

| 检查 | 日志后缀 | 结果 |
| --- | --- | --- |
| 原 A195004E DLL 的普通字母 ZWJ 光标行为 | `before.log` | 1，实际首步 5 字节、预期 4 的缺陷反证 |
| 最终默认 DLL 构建 | `build2.log` | 0 |
| 官方 17.0 全部 766 序列的每字节 map/Next/Prev/Clamp | `portable-final.log` | 0，pointer/callback 均核对独立 ÷/× 边界 |
| 全 1114112 码点 GCB/InCB/EP 原始属性对照 | `all-properties.log` | 0，C 与独立 UCD 三通道展开完全相同 |
| Linux 当前 C 的完整语料/异常 UTF-8/长簇/INT_MAX 及全属性 ASan/UBSan | `linux-final.log` | 0 |
| Windows/Linux 实际导航 C analyzer | `analyzer-windows.log` / `analyzer-linux.log` | 0 / 0，空日志 |
| 实际布局 17.0 map、既有行边界及 scale/failure 预算 | `layout-map.log` | 0，766 字素序列、7654 行边界序列与规模检查 |
| 既有 Unicode word/navigation 小测试 | `word-regression.log` | 0 |
| 完整 Windows Document 发布 suite | `final-suite.log` | 0，652 CommonMark、861948 Bidi、DLL/Renderer/Editor/MessageList/GPU |
| 原生 Unicode 17 场景的强化 caret→hit 字节往返 | `native-hit-final.log` | 0，140 组独立重编译/运行 |
| 字体选择器与两套 GDEF 夹具 Linux sanitizer | `font-linux-final.log` | 0，26 OOM 点/2 次追加回滚继续保持 |
| Unicode 17 数据固定输入重建 | `reproducibility.log` | 0，1631 范围，字节与 mtime 不变 |
| 官方语料注释空白规范化与语料内容相等 | `corpus-provenance.log` | 0，所有非注释记录完全不变，之后再次运行 portable 全语料 |
| 最终 Windows 基础 WebView widget | `webview-widget.log` | 0，12 生命周期、双视图、浏览器失败后重建 |
| Document private provider 与 MessageList | `web-provider.log` | 0，三次浏览器重启、恢复与预算 |
| 离线 KaTeX/Mermaid/HTML、HTML 沙箱与 PNG | `web-render.log` | 0，三张 500×350 图 |
| 生产 `XUI_WEBVIEW_TEST_EXPORTS=0` DLL | `webview-exports.log` | 0，14 个内部符号未导出 |
| API coverage no-write / comment lint | `api-coverage.log` / `comment-lint.log` | 0 / 0 |
| tracked / 本批新增文本 whitespace | `whitespace-final.log` / `whitespace-new.log` | 0 / 0，无诊断 |

每个官方序列直接读取 ÷/× 标记为独立边界 oracle；测试不只检查首尾，而是在所有 UTF-8 字节（包含字符/字素内部）逐一核对正向、反向及 clamp，连续字符串和 callback 两种入口分别调用。原始属性验证器独立扩展 GCB、InCB、EP 三个数组，组合后与 C 查询的完整 0..10FFFF 输出逐项比较，不复用生成器的范围合并算法。全部属性字节 SHA256 为 `621a8732dd0ed162dc783f7f12e116e554ed4463201affaa4f9a6cf1abcd4cf1`。语料和全属性都实际通过 Linux ASan/UBSan；状态机无动态分配，200001 字节组合簇及 INT_MAX 虚拟读取验证进度、局部 ASCII 读取上界和无有符号偏移溢出。

七个原生样本覆盖普通/pictographic ZWJ 后的普通字母、Devanagari/Balinese conjunct、Kirat Rai V、新 Prepend 和不同主脚本 conjoining。Rich 整段/逐码点不同颜色节点、Markdown VISUAL/SOURCE/LIVE、40/37 字号和 Spans/DrawText-only 共 140 组。实际 GPU context 与 XGE proxy 下，首步复制的选区必须是正确整字素；强化 hit 选区也必须是相同字节，再执行 Delete/Backspace、Undo/Redo，检查完整正文和尾部。绘制调用通过真实目标 Surface，夹具不包含全部新脚本真实字形，因此本项不单独证明这些脚本的字体质量、独立像素一致性或整形语言正确性。现有 GDEF/GPOS/Bidi 的真实字体与独立像素专项在 suite 中保持。

`native-hit-final.log` 对最后新增的命中断言重新编译；此前 suite 已通过相同产品 DLL 与 140 组基础导航/编辑场景，不重复执行整个发布套件来替代该精确验证。官方文本复制只去除两行注释的尾随空格，所有非注释记录原字节一致且 portable 766 语料再次通过；suite 和 Linux 全语料的早先运行使用原始副本。原始 SHA256 为 `e2d134d2c52919bace503ebb6a551c1855fe1a1faec18478c78fff254a1793ec`，仓库规范化副本为 `88ead7e1c192b5d9767eb4c54cb62e177ab82afd972efc247e04599fab66b719`，许可及来源见同目录 README。

固定生成输入 GCB / DCP / emoji-data SHA256 分别为 `d6b51d1d2ae5c33b451b7ed994b48f1f4dc62b2272a5831e7fd418514a6bae89`、`24c7fed1195c482faaefd5c1e7eb821c5ee1fb6de07ecdbaa64b56a99da22c08`、`2cb2bb9455cda83e8481541ecf5b6dfda66a3bb89efa3fa7c5297eccf607b72b`，生成器拒绝非固定内容。数据表 SHA256 为 `05C46A531252C3C28685BA7593623658840554FBFDDFB6886A41F17DA0FEA4E0`；默认/Document-local DLL 均为 `838D3CBE6E41BCA85F81E6BCA0AAE2E09CD1EF6979248C95486DEA5B1F52D5FC`。最终哈希在全部验证结束后重读，产品源码自 build2 后未改。

开发阶段保留：首次 DLL 构建中旧 grapheme 宏让 header 的 extern 声明与新静态 wrapper 冲突，移除不再使用的宏后 build2 成功；首次 native 测试漏 TARGET Surface 标志，第二次 SOURCE Position 错用 node 0，修正测试设置后 native3 通过，不把这两项当作产品缺陷。可复现的产品缺陷是旧 DLL 的普通 ZWJ 视觉移动；旧手写 Indic/部分属性和 decoder 的不一致另由完整官方/异常输入验证覆盖。

本批完成 Unicode 17 extended-grapheme 统一；UAX #14 仍为 Unicode 15.0，不能宣称整个文字系统全标准升级。完整 Script_Extensions/语言划分、跨样式/字体组上下文、位图/混合字体回退、全脚本真实字体与复杂导航性能、CST、多 carrier、device/variation 及五平台 GPU/IME/读屏/物理 DPI 仍在整体目标内。通用 WebView 继续 Windows 基础页面承载、其他平台预留；Document 内部渲染通道保留并再次验证，未扩展公共数据通信。整体目标保持 active。

全部本批验证进程已正常结束；表中最终构建/运行检查均为退出码 0，原 DLL 的反证退出码 1 单独保留。全部检查后重读的 default/Document-local hash 相同，产品源码自 build2 之后未改。后续改动限于测试的命中/完整性断言、固定输入/验证器路径、官方数据注释尾空格和文档。tracked 与新增文本最终 whitespace 检查为空，没有遗留运行中的本批 harness。


## R1 Unicode 17 Script_Extensions 原生整形批次（2026-10-01）

本轮完成传入原生整形字符串内的 Script/Script_Extensions 解析、词段回退/覆盖探测接入，以及 Document 首字符主 Script 重复切组的移除。不是跨行、不同字体/字号/方向组的完整段落上下文或语言验收。最终默认和 Document-local DLL 同为 `DFDA6537817F1AE4295D3F978E28335AF739C435CE46C08AD1604B866261DEE7`；没有把此前 `1B96CAA9` 当作含中文括号修正的最终构建。产品源码在 `build-frozen.log` 后保持固定；后续只补测试、夹具和文档。

以下日志均在 `artifacts/xui-document-rebuild/`，前缀为 `validation-2026-10-01-script-extensions-`。本节记录的所有最终进程句柄已取得终态；0 为实际命令退出码。

| 日志后缀 | 终态 | 证据 |
| --- | --- | --- |
| `build-frozen.log` | 0 | 最终默认 DLL，CJK Common 括号策略、Native SCX 及 Document 重复切组修正；没有编译 warning/error |
| `portable-frozen.log` | 0 | 前缀/集合交集、覆盖主 Script、普通/CJK/规范等价括号、段重置、全部 766 字素、200000 字节扫描和 20000 层有界括号栈 |
| `properties-frozen.log` | 0 | 全部 1114112 个实际 C Script/SCX/bracket 查询与独立展开 UCD 一致，dump `4c87487d2f356a2503f170c5397f70c131670b30f45a4814127fed3ead02f659` |
| `analyzer-frozen.log` | 0，空日志 | Windows GCC analyzer 的实际 C 脚本内核测试 |
| `linux-frozen.log` | 0 | Linux ASan/UBSan、全字素语料/压力向量、实际 C 全码点 dump 与独立属性比较，dump 与 Windows 相同 |
| `fallback-probe-final.log` | 0 | Linux 实际 C 词段选择器、Script-specific 缺 cmap GSUB 覆盖、NFC、整字素/词段、分配失败后重试和零测试所有者；40000 字节失败词段为 50002 次字体探测 |
| `native-probe-final.log` | 0 | 最后补齐的 48 个真实 SFNT glyph/font oracle，40/37 与 LTR/RTL，含中文括号及 Tatweel 的真正 GSUB 覆盖探测 |
| `document-frozen.log` | 0 | 400 个真实 Document 40/37、Spans/DrawText-only、Rich 整段/逐码点颜色、Markdown VISUAL/SOURCE/LIVE、完整/回退字体，独立宽度、caret/hit 和逐像素 GPU alpha |
| `bidi-complete-item.log` | 0 | 既有双向原生专项的两种绘制后端，真实 Segoe UI 混合 Hebrew/Arabic、方向 affinity/选区/断行、GDEF/GPOS、Source/Live、颜色像素、删除与 Undo |
| `suite-frozen.log` | 0 | 完整发布套件，652 CommonMark、完整 Bidi 数据、Unicode 17 字素、字体/GDEF/脚本/Renderer/Style/Scale/Editor/MessageList、原生 PNG/异步/占位图 |
| `web-widget.log` | 0 | 用同一最终产品源码重编译可选 WebView2 验证 DLL，基础控件 12 次生命周期、两个独立实例、进程失败及重建 |
| `web-provider.log` | 0 | Document 私有公式/Mermaid/HTML 与 MessageList 内部渲染，三次浏览器恢复及恢复上限 |
| `web-render.log` | 0 | 离线 KaTeX/Mermaid/HTML 测量、3 张 500×350 PNG、HTML sandbox |
| `web-exports.log` | 0 | 重编译生产可选 DLL；基本网页承载与 Document provider 可用，14 个内部浏览器符号不对外导出 |
| `reproducibility-final.log` | 0 | 固定表和三份自有字体重复生成字节一致；固定表内容相同时 mtime 不变 |
| `api-final.log`、`comments-final.log` | 0 | API 注释覆盖与门禁；没有修改公开函数签名或新增通用浏览器通信 API |
| `whitespace-new.log`、`whitespace-tracked-final.log` | 0 | 全部 13 个新增文本的 UTF-8/末尾换行/尾随空格检查，以及 tracked diff 空白门禁 |

完整 suite 编译运行了 44 个基础 native script oracle；其运行过程中，额外增加的人工缺 cmap/Script-specific GSUB 字体验证在 `native-probe-final.log` 重新编译运行至 48 例。最后测试修改没有改变产品 DLL，两份原有 script 字体的生成字节也没有改变。无需把同一套完整套件再次运行作为该新增单一分支的替代证明。

属性表含 176 脚本 tag、283 个扩展集合、1159 范围和 128 条括号记录，SHA256 为 `0cb5eb394a9c27b7b94b9d10e1d89866ea0a87fe9df9f0e02ab6a4ead3ab5896`。原始固定输入 `Scripts.txt` / `ScriptExtensions.txt` / `PropertyValueAliases.txt` / `BidiBrackets.txt` 的 SHA256 分别为 `9f5e50d3abaee7d6ce09480f325c706f485ae3240912527e651954d2d6b035bf`、`ec2107e58825a1586acee8e0911ce18260394ac8b87e535ca325f1ccbeb06bc6`、`64e9a5f76f7a1e8b5a47d6a1f9a26522a251208f5276bdfa1559dac7cf2e827a`、`dadbaf38a0d0246e5b805bf8725cb81b7c621f93d030595635f5ba2c2f179428`。默认 Common/Inherited/Unknown 以空约束集合加主属性表示，独立比较器恢复官方 singleton 值；规范等价括号比较有明确的 U+2329/U+232A 等价覆盖。原始数据与生成表遵循 `lib/unicode/LICENSE`；渲染策略依据固定 [UAX #24 revision 39](https://www.unicode.org/reports/tr24/tr24-39.html) 的实现说明，而非声称官方规定了唯一 script-run 算法。

自有 `xge_script_fixture.ttf` / `xge_script_no_alaph.ttf` / `xge_script_probe_tatweel.ttf` SHA256 分别为 `1346572ad79147bde5647b89b9c24b7bd35da55702880ccfff6fbde72e99959a`、`23f1ad17c9bb3b13144efd11b53138fe4aab4177d94b6f579993980635206311`、`521fc861e73c7d20195cdda82538a6065b2e372ab5037f2bf3601d2fe96684a1`。夹具使用不同 locl 字形/advance 和直接期望字形的 PUA alias；第三份人工字体移除 Tatweel cmap，并仅在 Syriac 下将 `.notdef` 替换为目标字形，因此可以证明非名义字符覆盖使用解析后的脚本。实际 C 选择器与 Windows 原生 shape 均要求保留主字体。测试字体不是生产字体质量证明。

开发过程的失败被保留：`before.log` 与 `document-before.log` 在上轮 `838D3CBE` DLL 上分别复现右括号用了 Greek glyph、40 字号段落宽度 102 而期望 96；`paired-before.log` 复现未加入 Common 配对例外时中文 opener 误取 Bopo 而非外层 Latn。最初 `portable.log` 的 hex escape 和解码函数指针类型已经修复；`build.log` 因 C 盘临时空间不足终止，设置 D 盘临时目录并创建目录后完成构建。`suite.log`、`bidi-frozen.log` 与 `bidi-final.log` 保留了旧参考绘制及中间参考方案的像素失败，没有作为通过证据。

真实 Segoe UI 参考现在按完整字体/方向组走公开 native renderer，不依赖 Document 的分片、分组、视觉索引；单独 Hebrew/Arabic 整形仍核对合并 width 和脚本边界 caret。原来逐脚本裁剪/取整及后续拆串浮点绘制都不能作为同一字体/方向组的逐像素参考。新增自有字体测试另外用 PUA 直接期望字形的逐像素 alpha 和独立 advance 验证脚本字形选择，没有仅用同一脚本算法自比。

当前仅验证 Windows 原生 SFNT/GPU 与 Linux 无窗口 C 内核/选择器；跨行/不同字体/字号/方向组的完整段落 Script/语言/前后上下文、位图/混合字体回退、真实字体矩阵、完整 CST、多 carrier、device/variation、复杂导航/性能、真实五平台 GPU/IME/读屏/物理 DPI 继续。通用 WebView 仍只有 Windows 基础实现和其他平台空后端；Document 私有公式/Mermaid/HTML 通道保留，整体长期目标 active。

## R1 原生段落上下文与语言契约批次（2026-10-01）

本批完成原生 `xge_text_shape_desc_t` 的段落/片段、脚本与语言输入，并让最终整形与真正的 HB 字体覆盖探测使用同一属性和上下文。XUI/Document 仍只传当前字符串，未据此宣称已经完成跨字体、字号、方向与行的整段投影接入。最终 default 和 Document-local DLL 的 SHA256 均为 `41588c9c4a4389dfb00044e4c9041199873373196674a14da35a07d81b60ae57`；产品源码自 `build-final.log` 后固定，后续仅测试/夹具/文档修改。最初的 `FD81A0E8` 是加入显式子段数量检查前的构建，不作为最终构建。

以下日志位于 `artifacts/xui-document-rebuild/`，前缀为 `validation-2026-10-01-paragraph-context-`。所有列出的最终构建、运行进程句柄已取得终态；退出码为实际命令结果。

| 日志后缀 | 终态 | 证据与范围 |
| --- | --- | --- |
| `before-final.log` | 测试退出 1，预期反证 | 独立加载旧 DFDA6537 DLL：三连写 Beh 的首字片段输出孤立 glyph 6，应为起始 glyph 7；首个测试即失败 |
| `build-final.log` | 0 | 最终默认 DLL，包含完整上下文/脚本/语言及显式子段数量检查；没有编译 warning/error |
| `native-final-frozen.log` | 0 | 132 例：128 个独立 upstream HB glyph/GPOS/font/cluster/hit oracle、2 个非零段落偏移 GDEF、2 个释放上下文/调用方字体后的输出生命周期；40/37、LTR/RTL、自动/显式脚本；在正常及 Turkish locale 两个进程中各通过一次 |
| `foundation-final.log` | 0 | 两处使用零初始化描述符的旧 NUL 调用迁移至 `iTextSize=-1` 后，实际字体/字形/缓存/测量 API 通过 |
| `linux-final.log` | 0 | 语言/范围无分配 C 契约、产品实际 C 字体选择器的 Arabic medial GSUB 和 Turkish/English locl、孤立字体回退、每个脚本图/字素图分配失败回滚/重试、零测试所有者，ASan/UBSan 与 Linux analyzer |
| `analyzer-final.log` | 0，空日志 | Windows GCC analyzer：完整语言/上下文契约及实际 C 选择器，没有关闭未初始化告警 |
| `no-hb-syntax-final.log` | 0，空日志 | 整个 `xge.c` 未定义 HB 的分支能编译；仅语法检查，未作为最小 profile 运行验收 |
| `suite-final.log` | 0 | 完整 Document 发布套件：652 CommonMark、全部 766 字素序列、861948 Bidi 数据用例、原生字体/GDEF/脚本、132 新上下文例的两次 locale 运行、400 Doc 脚本 GPU例、布局/样式/缩放/编辑器/消息列表、原生 PNG/异步/占位图 |
| `web-widget.log` | 0 | 同一最终产品源码重编译可选 DLL，12 次基础控件生命周期、两个独立实例、浏览器进程失败和重建 |
| `web-provider.log` | 0 | 保留的 Document 私有公式/Mermaid/HTML 与 MessageList，失败卡片、3 次浏览器恢复和恢复上限 |
| `web-render.log` | 0 | 离线 KaTeX/Mermaid/HTML 测量、3 张 500×350 PNG、HTML sandbox |
| `web-exports.log` | 0 | 同一最终产品源码重编译生产可选 DLL：基础网页承载与 Document provider 可用，14 个内部浏览器符号未对外导出 |
| `reproducibility.log` | 0 | 两份自有 SFNT 重复生成字节完全一致，固定时间戳 |
| `api.log`、`comments.log` | 0 | API 注释门禁与覆盖检查；覆盖 baseline 未重写，没有新增公共 WebView 脚本/消息通道 |
| `whitespace-new.log`、`whitespace-tracked-final.log` | 0 | 7 个新增 UTF-8 文本的末尾换行/尾随空白与 tracked diff 检查 |
| `hashes.log` | 0 | 全部运行后再次核对默认/Doc-local DLL、两个可选 profile 和自有字体 SHA256 |

新原生专项的 128 个 HB oracle 使用独立显式已知 script、language、完整段落和 item 范围，不调用产品的 Script_Extensions 或覆盖 helper；另外对 Arabic 单字和正确 RTL 多字形串、Syriac、语言 locl 使用字面 glyph 期望。GDEF 两例从固定夹具的独立 caret 坐标验证偏移仍为 1/2，不误变为段落偏移 4/5。生命周期两例释放动态上下文、item 文本和调用方字体引用后，输出仍可测量且在释放时清空最终字体引用。C 选择器的 OOM 对每个实际图分配逐点失败并重试；40000 字节不匹配词段仍为 50002 次字体访问。没有将这些样本当作所有脚本/所有上下文的性能或字体质量验收。

两份自有字体 `xge_context_fixture.ttf` / `xge_context_probe.ttf` 的 SHA256 分别为 `ac95df5bf3263b8b470d465c0e27ec0cc8b43cf52289162d20864db1d98d51ac`、`123319a5559835cbd40e8ee187c0fa30668117807b8cd66bc39eb6313c7573f0`。第二份移除 Beh/i cmap，通过 contextual Arabic forms 和 Turkish locl 人工替换 .notdef；English/default/孤立 Arabic 必须选完整回退字体。固定时间戳与生成器/README 保留，未复制外部字体。可选测试 DLL / 生产 DLL SHA256 为 `adddd2386d3ddd259d649809c2b4e63a4099c3d50ca81d77faf3c63a4237b0a1` / `d31236f3e25013c91f19850e8d53e00b4b362897576720a9753b06f03cabe68c`，与默认 profile 的差异来自可选编译配置。

开发失败日志没有删除或作为通过证据：`native.log` 的 LTR Arabic 多字形字面顺序断言、`native-final.log` 的 mark 必须排第一断言、`script-tags.log` 及早期 `suite.log` 的 Common 请求必然绕开 Latin feature 断言，均为测试参考不正确。HB 在显式 LTR 下的 Arabic 次序及同 cluster 内 mark 顺序必须保持原生 oracle；字体没有 DFLT 表时 Common/Inherited 仍可能回退到 latn，不能凭脚本名假定输出。这些参考已修正，最终 glyph/position 仍逐项对独立 HB 核对；132 例的完整 suite 和单独最终专项都通过。`analyzer.log` 的首个子段可能未初始化路径，由显式 `count==0` 返回消除；Windows/Linux analyzer 最终为空，未抑制告警。完整 suite 内的新专项输出仍使用早期的总数说明文字，最终独立专项已改为准确区分 128 HB / 2 GDEF / 2 生命周期；仅打印说明改变，没有修改断言或产品。

原生契约验证输入范围、UTF-8 标量边界及 item 原字节一致；完整上下文由调用者保证有效 UTF-8，不在每次显式 item 上重复完整解码。RFC 5646 语言检查只保证语法与 255 字节实施上限，不验证 IANA 注册。NULL 为固定 `und`，不猜测进程 locale。上下文传递按 [HB buffer API](https://harfbuzz.github.io/harfbuzz-hb-buffer.html) 使用完整文本与 item offset；语法依据 [RFC 5646](https://www.rfc-editor.org/rfc/rfc5646.html)。字体的 script/language table 回退保留 HB 行为。集成 C 与 libunibreak 在 Linux 测试中启用 sanitizer，缓存 HB 依赖对象未声称全量 instrumented。

当前支持证据是 Windows 实际 SFNT 原生输出/既有 Document GPU，以及 Linux 无窗口 C 契约/选择器。XUI 整形与绘制代理、完整 display 投影/来源映射/每段脚本缓存、跨不同样式字体字号方向与行的 context，以及共享文档语言属性仍待实现；不能让逐 item 全段 SCX 重扫成为 Document 的常规路径。全词跨样式字体选择、位图/混合回退、真实字体矩阵、完整 CST、多 carrier、device/variation、复杂导航/性能及真实五平台字体/GPU/IME/读屏/物理 DPI 继续。通用 WebView 只有 Windows 基础实现及其他平台空后端；Document 内部渲染保留，整体长期目标 active。

## R1 XUI 统一文本请求批次（2026-10-02）

实现范围为 `xui_text_item_t`、五个代理入口和公共 `xuiTextShape` 的整体迁移。旧 API 没有包装，代理版本 11 与所有调用方/DLL 一起重编译。调用方属性和 item 字节借用至同步回调返回；`-1` 是 NUL 长度，0 是空 item，context 为空时其 size/offset 必须为 0。XGE/XUI 共用无分配的 RFC 5646/script/字节范围验证。规范化保留 NULL 上下文，保证普通文本不会被误标记成无 HB 后端不支持的显式上下文。

最终默认/Doc-local DLL SHA256 为 `14eb91aedea2f6a982141b268f9c330eedb6a3140ee0a4867b98e5e2dd0598bd`。最终可选测试 WebView DLL / 生产导出边界 DLL 为 `6a9f2fd13795fd66494a96e3d5959e1ebfe4e63d82503be68492bf40afa0818b` / `b891ebbef18486a4b81511c8838fa83a8a1280b021f5c023fd40f9e0f5992169`；无 HB 的独立原生 DLL 为 `4522092412df9913e824e041965e8f40da8642026a0f782ad9b4f2e2d9d456bd`。这些配置不同，哈希不应相同。之前 `a6fe99e8...` / `e16a7f85...` / `295ff780...` 是保留 NULL 上下文修正前的构建，不作为最终构建。

日志在 `artifacts/xui-document-rebuild/`，本批前缀 `validation-2026-10-02-xui-text-item-`。以下条目已有实际终态：

| 日志后缀 | 退出码 | 范围 |
| --- | --- | --- |
| `native-final.log` | 0 | 纯 C 输入契约及 60 组真实 GPU 精确 alpha 对照；40/37，Arabic 首/中/尾、Syriac/显式 script、默认/English/Turkish locl，非 NUL 长度、五代理转发、方向优先级、GPOS/GDEF、非法请求与旧版本拒绝 |
| `no-hb-build.log` / `no-hb-native-final.log` | 0 / 0 | 实际无 HB DLL与五个入口；固定自有 cmap glyph 2、字面 advance 16、三种 GPU 精确 alpha，以及显式全 item/子范围 context、script、language、RTL 拒绝；不宣称复杂文字/Document 段落上下文支持 |
| `linux-contract-final.log` | 0 | ASan/UBSan：四字节描述符、精确非 NUL 两字节缓冲区、借用字段/原请求不变、alias/idempotence、空输入和失败输出原子性 |
| `linux-context.log` | 0 | 共同语言/范围 C 契约及实际 C 字体选择器上下文、每个图分配的 OOM 回滚/重试、40000 字节词段线性访问 |
| `linux-document-final.log` | 0 | 最终源码无窗口 Rich/Markdown Renderer、DocumentView/Editor 三模式、光标/命中/输入/Undo 和 ASan/UBSan；保留 DatePicker 既有 `tValue` maybe-uninitialized warning，未称全量无告警 |
| `analyzer-final.log` | 0，空日志 | 对输入契约测试实际 `-fanalyzer -c`，不是仅语法模式 |
| `consumer-syntax-final.log` | 0 | 46 个迁移 C 调用方与单独配置的 OOM 测试严格语法检查；测试 case header 随其宿主 TU 验证，未作为独立 TU 编译 |
| `tools-syntax.log` | 0，空日志 | AnimForge、ParticleEdit、UIDesign 迁移源码严格语法检查；MapEdit 和 UIDesign component 分别用各自正确 include 根检查通过。没有宣称工具完整运行验收 |
| `corpus.log` | 0 | 652 CommonMark 解析/来源/原生往返及 detached prepare 差分；不等于 HTML 输出兼容性验收 |
| `web-widget-final.log` | 0 | 最终可选构建、12 次基础控件生命周期、独立实例、浏览器失败/重建 |
| `web-provider-final.log` | 0 | Document 私有 KaTeX/Mermaid/HTML、主题/错误卡片、3 次恢复和预算上限，MessageList 集成 |
| `web-render-final.log` | 0 | 离线测量/渲染、3 张 500×350 PNG、HTML sandbox |
| `web-exports-final.log` | 0 | 最终生产构建，基础网页承载与 Document provider 保留，14 个内部浏览器符号未对外导出 |
| `api-final.log` / `comments-final.log` | 0 / 0 | API 注释覆盖与 lint，覆盖 baseline 未重写 |

首次 broad widgets 实际结果为 156/163 通过、7 项链接失败，原日志和 `widgets.json` 保留。MessageList audit/collections 的旧局部源码清单漏掉新 Document 私有依赖；prepare/containers/adapter/drag/tooltip 的完整 XUI 清单又缺 XGE 图像 API 的 import library。已改为共享 release 清单，保留本地浏览器空实现来链接私有 HTML 调用，并补充实际 XGE 图像库及子目录进程的 DLL 搜索路径；没有增加私有 DLL 导出或用假成功函数绕过依赖。初次 audit 全清单尝试仍排除了浏览器模块，`message-audit-final.log` / `message-audit-fixed.log` 退出 1；改为本地完整空后端后 `message-audit-corrected.log` 退出 0，并保留原有显示偏移、规模、懒布局和 Document 样式重排断言。

下一批必须处理的反证：`next-context-window.log` 退出 0 是诊断成功，打印结果是错误差异而非验收通过。40 号、Arabic/`ar`/RTL、自有 context 字体，`Beh + 8 × Fatha + Beh + Beh` 中间 Beh 的 offset 18、length 2 请求得到 glyph 7/advance 28，整段请求同一位置是 glyph 8/advance 32。本地 HB 的两侧 context 长度是 5 个 scalar。此前 132 个与 upstream 同种 item API 的一致性证明不覆盖“片段等于整段”这一长透明上下文性质。Document 跨字体/字号/行段落接入须先补此正式回归并解决差异；本批只证明请求规范化、传递、给定范围的实际原生行为及迁移兼容后的回归。

完整 Document 显示投影/来源映射、按段脚本缓存、共享语言属性、跨样式全词字体选择、完整 CST、多 carrier/device/variation、复杂导航/性能及真实五平台设备验收仍未完成。通用 WebView 仍是 Windows 基础方案和其他平台空后端；Document 私有内部渲染保留，长期目标 active。

最终 widgets 复测已结束：`widget-final-rechecks.json` 的 26 项（前 19 项在最终 DLL 下重跑，以及所有初次失败项）全部退出 0；`widgets-final.json` 保留每项初次状态/日志并合并最终状态，163/163 通过。`final-validation.log` 明确记录 26 项终态，随后串行运行包含官方 CommonMark 语料的完整 Document suite。`message-audit-corrected.log` 的早期独立通过不代替正式 BAT，正式 `widget-final-build_message_list_audit_test.log` 及其余六项修正后的脚本均已实际退出 0。

最终完整 suite 与父级 `final-validation.log` 均已有实际终态，退出码 0。`suite-final.log` 使用最终默认/Doc-local DLL，包含 652 条 CommonMark 解析/来源/原生往返、Unicode 字素与 Bidi 数据、132 例原生上下文、60 组新 XUI GPU 精确 alpha 对照、布局/字体/样式/缩放/编辑器、长 MessageList、原生 PNG/异步发布/缺图占位；不能将此解释为尚未接入的 Document 完整段落上下文正确性验收。所有本批运行句柄已关闭，未留下后台测试。结束后再次核对最终五个 DLL 路径的 SHA256 与本节记录一致，并执行 tracked diff 和新增文本空白检查；终态记录在 `hashes-final.log`、`whitespace-tracked-final.log`、`whitespace-new-final.log`。长期目标仍 active，正式修复长透明上下文反证是下一项必要工作。

## R1 长透明连写上下文修复验证（2026-10-02）

正式专项 `test/build_joining_context_test.bat` 已加入完整 Document suite。原生 oracle 使用独立 HB 的完整段落，不调用产品 context helper 或 HB bounded item API；字形、字体、advance/offset 和 cluster 范围分别核对。Arabic 整段性质使用已解析 RTL；显式 LTR 的 HB 整段反转行为有别于 fragment，仍按既有 upstream item oracle 验证，未混同两种性质。此前原生 oracle 的 132 例仍须通过。

本批日志位于 `artifacts/xui-document-rebuild/`，前缀 `validation-2026-10-02-joining-context-`。以下条目已有实际终态；完整 suite 尚以其真实运行终态为准。

| 日志后缀 | 退出码 | 验证范围 |
| --- | --- | --- |
| `red.log` | 1，预期反证 | 旧 `14eb91ae...` DLL：5 个透明 Fatha 的中间 Beh（offset 12）返回 initial glyph 7，独立整段及字面期望是 medial glyph 8 |
| `dll.log` | 0 | 真实重编译默认原生 DLL，并同步 Doc-local 副本 |
| `properties.log` | 0 | 1114112 个实际 C 查询对独立官方 Unicode 17 原文，以及固定 HB joining table/真实 category fallback；未调用生成器充当 oracle |
| `native.log` | 0 | 2688 组原生全段/字面 glyph、font、advance/offset、item cluster：40/37、自动/显式 script、双侧多种透明标量与 ZWJ/ZWNJ/空格 |
| `native-final.log` | 0 | 在上述基础上加入两组长 pre-context 的真实 GPOS mark 对照，2690 组实际通过 |
| `xui-final.log` | 0 | 84 组两字号/三绘制路径的精确 GPU alpha 对照；长双侧上下文、ZWNJ、测量/塑形/绘制五入口保持属性；跨长标记的 GDEF offset 19 仍保留 item-relative 1/2 停点 |
| `selector.log` | 0 | 产品实际 C 覆盖选择器：512 个双侧标记下选择 medial-only 缺 cmap 夹具，ZWNJ 实际 fallback；逐分配 OOM 回滚/重试、40000 字节失败词段 50002 次字体访问 |
| `portable.log` | 0 | 400006/400002 字节精确非 NUL context、200002/200000 个邻居标量访问、有效/缺失邻居的字面字形、释放借用文本后塑形、附近非法 UTF-8 与失败 HB allocation |
| `linux.log` | 0 | 实际 C buffer integration 的 ASan/UBSan 与逐码点独立官方/HB 属性核对；缓存 HB 对象未宣称全量 sanitizer instrumented |
| `linux-selector.log` | 0 | 上述真实 C 字体选择器含长上下文/故障注入在 Linux ASan/UBSan 通过，零测试所有者 |
| `analyzer.log` | 0，空日志 | Windows 对真实 C buffer integration 的 `-fanalyzer -c`，不只是 syntax-only |
| `web-widget.log` | 0 | 同一最终产品源码重编译可选测试 DLL，12 次生命周期、独立实例、几何/焦点、浏览器失败与重建 |
| `web-provider.log` | 0 | Document 私有公式/Mermaid/HTML 与 MessageList、错误卡片、3 次浏览器恢复及预算上限 |
| `web-render.log` | 0 | 离线 KaTeX/Mermaid/HTML 与 3 张 500×350 PNG、HTML sandbox |
| `web-exports.log` | 0 | 重编译生产可选 DLL，基础网页与 Document provider 保留、14 个内部浏览器符号未导出 |
| `reproducibility.log` | 0 | 三份自有字体和生成的 Unicode 表重复生成字节一致；未变的表保留 mtime |

默认/Doc-local DLL SHA256 均为 `a7932bcd3ca5d489e92464dbe80d2a2aab89522951b43ebdc43d8f20ae860c33`，可选测试/生产 DLL 分别为 `cf7168d3342a49eee140de59cd404e8183b1537f6ec22923e363c459a58c38db` / `dedc5371ccac5886e3c67f54130ab5a675686f1cfb0a3b9e9baa563acebc7b52`。固定 Unicode 17 DerivedJoiningType 原文 SHA256 是 `f39ebe974825d6736aee15582250307aa532b2cfab3caf3f86bd23fddc9c5c4d`；新增自有 medial-only 字体为 `ceb0c5bf22e3fea8e51254c9462e4bd4464d175cab192b5be2b98526194aeea8`，原两份字体字节未变。

开发阶段 `red-oracle-ltr.log` 的失败是初版整段参考错误，不是通过证据；方向范围已更正，正确的 RTL `red.log` 则保留真正产品反证。正式 portable 原生测试只引用公开 HB buffer API；固定 vendor 未补丁或扩大常数窗口。透明数据依据 [Unicode 17 DerivedJoiningType](https://www.unicode.org/Public/17.0.0/ucd/extracted/DerivedJoiningType.txt)，空 context/text 添加行为核对当前固定 HB 的 `hb-buffer.cc` 及 [HB buffer API](https://harfbuzz.github.io/harfbuzz-hb-buffer.html)。

此批证据证明邻接透明串的原生连写修复，不证明 Document 完整段落缓存、不同字体/字号/行之间的显示投影、跨 item ligature/GPOS、跨样式整词字体选择或全脚本/真实设备矩阵。上述内容、共享语言属性、完整 CST、多 carrier/device/variation、复杂导航/性能与真实五平台验收仍待完成。WebView 的对外范围保持 Windows 基础承载，Document 内部通道保留，整体长期目标 active。

最终终态：`suite.log` 实际退出 0，涵盖 652 CommonMark、现有 132 例 context 的两次 locale 运行、2688 例新 joining、84 组 XUI GPU、Document 内核/渲染器/View/Editor、长 MessageList 与原生 PNG/异步/缺图占位。suite 已执行的专项版本还未包含后来加入的两组 GPOS 与长 pre-context GDEF；这些新增断言分别由最终 `native-gate-final.log` 和 `xui-gate-final.log` 的正式 BAT 实际退出 0 证明，前者同时运行官方/HB 全码点与 portable scale，并以 2690 原生例结束，后者含新 GDEF 和 84 GPU 例。没有在产品源码未变时重复整个 suite 来替代必要的新增断言验证。

`api.log` / `comments.log` 实际退出 0，API coverage 使用 `--no-write`，公共头文件实际编译和注释 lint 通过；没有调整覆盖 baseline。本批所有执行句柄（默认 DLL、专项、完整 suite、可选 WebView pipeline、Linux 和最终正式 BAT）均已有实际终态并关闭。`hashes-final.log` 记录最后四个当前 profile 的 SHA256，与本节相同；tracked diff 和 13 个新增/修改 UTF-8 文本的空白/末尾换行检查均退出 0，记录为 `whitespace-tracked.log` / `whitespace-new.log`。旧 LTR 错误参考与真正 RTL red failure 保留，未作为成功证据。整体长期目标继续 active。

## R1 Document 保留段落上下文与脚本缓存验证（2026-10-02）

本批实现保留的完整显示上下文、可选来源边界映射、按段一次解析的 Script_Extensions 缓存，以及共同测量/绘制 item。块与 renderer 快照保有所有者；普通段落、代码、SOURCE/LIVE、表格内段落和安全切点 continuation 使用这套基础。纯 ASCII item 暂保留隐式输入，不能据此宣称跨 ASCII 字体/字号 item 的上下文 GSUB 或无 HB 普通非 ASCII 降级已完成。语言属性仍未进入 Document Schema/继承/事务/持久化。

日志位于 `artifacts/xui-document-rebuild/`，前缀为 `validation-2026-10-02-document-context-`。下表各项均已有实际终态；最后完整发布 suite 使用同时包含原生代理和重复 seed 修正的最终源码。

| 日志后缀 | 退出码 | 范围 |
| --- | --- | --- |
| `suite-last.log` | 0 | 最终完整发布：652 CommonMark、2690 原生连写上下文、84 XUI GPU、400 脚本与 840 Document 完整字面 union、Bidi/字体/GDEF、Renderer/View/Editor/规模、长 MessageList、实际 DLL 导出与原生图像/异步/占位图 |
| `native-red.log` | 1，预期反证 | 旧 `cf7168d3...` DLL：37 号中间 Beh 的 caret advance 22.2，而字面 medial glyph 期望为 29.6；探针因真实几何断言失败，不是通过证据 |
| `renderer-v12-final.log` | 0 | 最终原生代理下的完整 Renderer：652 CommonMark、15470 LIVE 光标边界、源码/按需续排、56 KiB 脚本上下文、真实 Arial 两后端的强制换行/尾随空行/亲和性及全 RGBA 对照 |
| `scale-v12-final.log` | 0 | 修正重复 seed 后的完整规模专项；90003 字节段落首屏整形 16392 字节，小于既有 22500 上限，晚出现高字形/两轮续排/OOM 重试/早期几何、缓存预算、长代码/SOURCE 和大文档首次编辑 |
| `native-union-first.log` | 0 | 两字号与两绘制后端共 840 例；0/5/8/32/512 附标及 ZWNJ、字号/字体/Bidi 分界、Rich/Markdown VISUAL/SOURCE/LIVE/代码、重排/字体失效/释放 live Document；字面 advance、caret/hit 和整个 400×160 画面的精确 alpha |
| `item-v12-final.log` | 0 | 84 组五入口共享上下文/script/language、正负小数绘制偏移的实际 GPU alpha；测量不变，GPOS/GDEF、NaN/Inf/越界原子拒绝，版本 10/11 代理拒绝 |
| `item-no-hb-last.log` | 0 | 真正未编译/链接 HB 的最终独立 DLL；五入口普通隐式输入、字面 cmap advance、三种精确 GPU alpha 与显式 context/script/language/RTL 拒绝 |
| `renderer-linux-last.log` | 0 | 最终布局源码的 Linux 无窗口实际 C Renderer ASan/UBSan；56 KiB 段落 Rich/Markdown 与两代理路径，冷查询与完整布局相等，续排失败保持几何/绘制且可重试，宽度重排复用上下文 |
| `view-linux-last.log` | 0 | 最终布局源码的 Linux 无窗口 DocumentView/Editor 三模式、输入/导航/选择/Undo、源码与命中；ASan/UBSan |
| `factory-linux-v12.log` | 0 | 实际段落工厂和共同显示过滤 helper；字面来源/显示图及脚本 tag、两轮五分配失败/首个成功预算、已有所有者、零分配 reflow、stale/非法 UTF-8/边界/空输入与释放；registry reserve 使用测试适配器 |
| `item-contract-linux-final.log` | 0 | 纯 C 描述符 ASan/UBSan：短描述、精确非 NUL 缓冲区、借用字段、alias/idempotence、空范围及非法偏移失败输出原子性 |
| `web-widget-last.log` | 0 | 最终源码可选构建、12 次基础控件生命周期、两实例、几何/焦点及浏览器失败/重建 |
| `web-provider-last.log` | 0 | 保留的 Document 私有 KaTeX/Mermaid/HTML 与 MessageList，错误卡片、3 次恢复与重启预算 |
| `web-render-last.log` | 0 | 离线 KaTeX/Mermaid/HTML 测量/渲染与三张 500×350 PNG，HTML sandbox |
| `web-exports-last.log` | 0 | 最终生产可选构建；基础网页承载和 Document provider 保留，14 个内部浏览器符号未对外导出 |
| `api-final.log` / `comments-final.log` | 0 / 0 | 覆盖 baseline 未写回；xge 755/755、xui 3255/3255、Document 154/154、Document UI 130/130，公共头编译与注释 lint |

默认与 Document-local DLL SHA256 均为 `2cb73f24f6a2cf271ceb7cdd3ec14845944b1c6207e9b229f73f97d06fc98848`。最终可选测试/生产 DLL 为 `f6a4e418101fb2b62ed4b9ded9b1a9296265e882e8e0160f04cd163584f58d3a` / `10c896208bdb5b6646f934bd975de9d511c61e82e56dd1c5424709ed37a86734`；实际无 HB DLL 为 `35d9886f8806d1c074c51dc783479fb43cc535a47d7af8d769e7ebc34f37d1c3`。这些 profile 配置不同，不要求相同哈希。三份自有 context 字体字节未变：`ac95df5bf3263b8b470d465c0e27ec0cc8b43cf52289162d20864db1d98d51ac`、`123319a5559835cbd40e8ee187c0fa30668117807b8cd66bc39eb6313c7573f0`、`ceb0c5bf22e3fea8e51254c9462e4bd4464d175cab192b5be2b98526194aeea8`。此前 A7932BCD、4FBBE408、5B839A2C/D96C86B9、C86E3A3C 及早期 WebView v12 构建均不是同时含最终代理修正与重复 seed 修正的产物。

开发失败保留，未记为通过：首次 `suite.log` 在既有 Hebrew/Arabic 整段原生 union 像素对照失败；Document 整数矩形和 XGE screen-space 原点二次取整丢掉片段的小数 pen。加入有限 `fDrawOffsetX` 和仅 glyph-run 使用的 `XGE_DRAW_TEXT_SUBPIXEL_X`，保留 Y 基线规则且在转发 quad 前移除文本专用位；代理升为 12，应用/自定义代理/DLL 一起重编译，旧版本不兼容。`bidi-subpixel-first.log` 仅改 XUI 偏移仍失败；`bidi-subpixel-second.log` 因开发补丁声明位置错误编译失败，后续实际修正，未抑制告警或放宽原生像素断言。

随后 `suite-final.log` 在 Renderer 的尾随 NEL/LF 全 RGBA 对照失败：caret 完全相同，但 15 个像素的颜色通道差 1。`mandatory-diagnostic-third.log` 为真实失败；`mandatory-flush-probe.log` 的诊断代理统一提交边界后通过，不能替代产品运行。产品原生代理因此在沿用视口而无需 item 额外裁剪时，保持旧裁剪文本的前后提交边界；正式 `renderer-v12-final.log` 保留全部像素等值并扩展到 drawText-only 后端，实际通过。所有早期 optional WebView 通过均保留为历史，最终仅用表中的 `-last` 重编译和运行记录。

`native-v12-final.log` 的中间字局部 alpha 参考也曾失败：去掉人工 item 墨迹裁剪后，37 号相邻起始字在目标边缘有合法墨迹，单一中间 PUA glyph 参考缺失这部分。没有忽略边缘或放宽容差；正式参考改成首/中/尾独立 PUA alias 及 GPOS mark 的完整 union，字面 advance 与字体 metrics 推导相邻位置，精确比较全帧 alpha。实际 840 例已通过，原生混合脚本整段 union oracle 未修改。

`suite-v12-final.log` 已通过上述原生正确性断言，但在约 90 KiB 的晚出现中文段落中超过既有首屏整形字节上限，实际退出 1；不是完整发布通过证据。完整 cached run 的隔离 advance 现仅作为首次候选计划，省去因为段落有脚本图而额外执行的完整 seed，最终每行仍以完整上下文重新整形、规划，收敛后才发布。跨节点和 continuation seed 继续，任何最终非 ASCII item 都没有恢复旧绕过分支。独立正式 `scale-v12-final.log` 保留首屏/续排字节、晚出现高字形几何、故障重试和缓存预算的原阈值，实际退出 0。

Linux headless sanitizer 编译路径不包含 `src/xui_proxy_xge.c` 或原生 GPU；其通过不能代替 Windows 原生代理的小数 pen/提交修正，也不能证明其他四平台的真实字体、GPU、IME、读屏或物理 DPI。已有 DatePicker `tValue` 的编译器 warning 保留，未宣称全仓库无告警；缓存 HB 对象未宣称全量 sanitizer instrumented。此批也未完成纯 ASCII 跨 item 上下文、无 HB 非 ASCII 文档降级、选择性 SHY 全文变体的重复复制优化、完整控制整形语义或跨样式全词字体选择。完整 CST、位图/混合回退、多 carrier/device/variation、复杂导航/性能及五平台实机验收仍待继续。WebView 对外范围为 Windows 基础承载，其他平台空后端，Document 私有渲染通道保留，整体长期目标保持 active。

最终终态：`suite-last.log` 与 Linux 两项 `-last` sanitizer、实际无 HB `item-no-hb-last.log`、四项 WebView/内部 provider `-last` 验证均实际退出 0。完整 suite 已包含最终 840 全帧参考与 drawText-only 强制换行扩展，没有以早期专项替代新断言。全部本批运行句柄已关闭，无遗留后台测试；再次核对五个当前 DLL 的哈希、三份字体字节、tracked diff，以及本批 28 个 UTF-8 文件的完整尾随空白/末尾换行检查，终态记录为 `hashes-final.json`、`whitespace-tracked-final.log`、`whitespace-new-final.log` 与 `final-state.json`。API coverage baseline 未改写；整体长期目标仍 active。

## R1 Document 共享自然语言属性验证（2026-10-02）

本节日志前缀为 `artifacts/xui-document-rebuild/validation-2026-10-02-document-language-`。最终产品源码包含语言所有权/继承、原生 v7、HTML/复制、独立 Markdown 语言损失位、Editor 待输入状态、Root/容器失效与共享源码预览接管检查；下表均收集了实际进程退出码。历史批次的 DLL 和早期通过日志不替代本节最终验证。

| 最终日志 | 实际退出码 | 验证范围 |
| --- | ---: | --- |
| `suite-final.log` | 0 | 独立 Core、当前默认/Document-local DLL、652 CommonMark、15,470 个 LIVE UTF-8 位置、2690 既有原生连写上下文、84 XUI 请求 GPU、400 Script_Extensions、840 既有 Document union 与 168 新语言全帧参考；Renderer/View/Editor/规模/MessageList/原生图像/异步/占位图 |
| `core-linux-last.log` | 0 | 当前 Core 与 652 CommonMark 的 Linux ASan/UBSan；语言输入复制/大小写 intern/继承/und、范围/混合/清除格式/历史、v7 与旧格式拒绝、HTML/片段跨语言复制、独立转换损失、Markdown 局部解析与 Prepare |
| `renderer-linux-final.log` | 0 | 当前无窗口 Renderer ASan/UBSan；关闭历史后颜色替换释放旧属性、释放 Document、换字体/改宽与两种绘制后端；新增空补丁共享源码预览切换默认语言的原反证 |
| `view-linux-final.log` | 0 | 当前 Rich/Markdown 三模式 View/Editor ASan/UBSan；范围语言、待输入缓冲所有权、und/继承重置、输入前后清除格式保留语言、Undo/Redo 与非法标签原子拒绝 |
| `pool-million.log` | 0 | 1,000,000 个含语言的不同属性：规范值、平衡池及全部释放；插入 1.716 s、释放 1.291 s，为本机样本 |
| `no-hb.log` | 0 | 真正独立构建的无 HB DLL；隐式普通请求、五回调、字面 advance 与三组 GPU alpha；显式 context/script/language/RTL 拒绝 |
| `web-widget-isolated.log` | 0 | 当前可选测试 DLL 单独运行：真实焦点/几何、12 次生命周期、两独立控件和隔离浏览器异常/重建 |
| `web-provider.log` | 0 | 保留的 Document 私有 provider/MessageList：错误卡片、截图解码/裁剪恢复、浏览器真实退出与 3 次恢复预算 |
| `web-render.log` | 0 | 离线 KaTeX/Mermaid/HTML 测量/渲染、HTML sandbox 与三张 500×350 PNG |
| `web-exports.log` | 0 | 当前生产可选 DLL 重新构建；基础网页承载/Document provider 保留，14 个私有浏览器符号未公开导出 |
| `api.log` / `comments.log` | 0 / 0 | 公共头编译、API/注释检查；Document 154/154、Document UI 130/130，coverage baseline 未写回 |
| `whitespace-tracked-final.log` / `whitespace-new-final.log` | 0 / 0 | tracked diff 及本批 34 个完整 UTF-8 文件，含未跟踪新增文件：无 NUL、尾随空白及末尾换行；Git CRLF 提示保留 |

168 个原生案例来自七种文档形式、六轮更新、40/37 两字号与 Spans/DrawText-only 两后端。自有 fixture 的 English/Turkish/default `locl` 分别产生字形 4/5/3，独立参考用字面 PUA alias 与明确 advance 访问期望字形，绕过语言替换。Rich 同字体的相邻语言边界、ZWJ 显示投影、Markdown VISUAL/SOURCE/LIVE、代码自然语言与编程 info 分离均逐项核对宽度/停点及整个画面的精确 alpha。没有用相同语言请求再塑形作为像素 oracle，也没有放宽容差。

Core 分配故障为 48 个预算，其中 10 次失败、38 次成功，验证失败不发布 revision/语言/树/历史且全部释放。255 字节私用标签成功，256 字节以及不终止/不合法样式值被拒绝；原生旧版本 fixture 只移除新增的空语言字段后继续原有版本校验。Schema 版本兼容针对历史数据，未新增旧 API 包装；公开结构已改变，宿主与 DLL 必须同步重编译。

真实开发失败保留：`native-v1.log` 退出 1，Root en → tr 后子块继续使用旧语言宽度；Root 与未独立索引的容器样式现重建目录，最终 168 例通过。`preview-red.log` 退出 1，新增空 SOURCE 补丁共享源码、改变 Root 语言后仍接管旧预览；同一测试在 `renderer-linux-final.log` 和完整最终 suite 通过，接管必须同时满足语言一致。未通过扩大预算、关闭历史或弱化断言隐藏错误；生命周期专项刻意禁用历史，让旧属性真正释放。

`core-initial.log` 的失败是旧 schemaVersion 6 期望；`core-v1.log` 暴露 Root HTML 默认语言在导出子节点时丢失，已补容器导出/导入；`core-v2.log` 为测试传入未指定 profile 的选项结构，已修正测试前提。`core-v3.log`、`core-final.log`、早期 Linux/原生日志及无语料参数的 `suite.log` 仅保留开发过程；最终只使用上述带完整最新断言和语料的记录。

基础 WebView 的 `web-widget.log` 第一次运行实际退出 2，在与默认原生 suite 并行期间失败于 restored browser focus；测试涉及操作系统焦点。suite 完成后同一当前可执行文件独立重跑退出 0，保留原焦点断言及第一次失败日志，本批没有修改 WebView 产品代码。独立重跑结果证明该次完整控件测试通过，不据此声称已经证明首次失败的唯一原因或全部并发焦点行为。其后 provider、离线渲染与生产导出测试顺序执行并分别通过。

最终默认与 Document-local DLL SHA256 均为 `db6a5df7710f1854891a0d40e22f70819438b70027451a52c17394e9b801268a`；可选测试/生产 DLL 为 `fdfc3dfa20c5f47c2849615c5b503852eaf534d4b4ce8d46b259628cf20514b7` / `e5fd33338ad124d01359252a9abe161151647283d3e345eb134276e108f2c8b3`；无 HB 为 `e49ece9159e7d90f350596ded2d12450d1dcda822f13075bd38264055f2fd94c`。profile 配置不同，不要求哈希一致。三份 context 字体仍为 `ac95df5bf3263b8b470d465c0e27ec0cc8b43cf52289162d20864db1d98d51ac`、`123319a5559835cbd40e8ee187c0fa30668117807b8cd66bc39eb6313c7573f0`、`ceb0c5bf22e3fea8e51254c9462e4bd4464d175cab192b5be2b98526194aeea8`，原始字节未改。最终 hash、退出码和空运行句柄集合另存 `hashes-final.json` 与 `final-state.json`。

限制：CommonMark 验证解析/源码/重载及 LIVE 位置，不证明 HTML 或显示规范；Linux 无窗口代理不证明原生 GPU、IME/读屏/WebView 或物理 DPI，依赖 HB 缓存对象未宣称全量 sanitizer。显式语言的长 ASCII 源码/代码行目前采用完整行整形保证正确性，其性能仍待优化；无 HB 最小后端不支持显式语言，任意 Markdown 局部语言语法也未实现。输入能力边界、纯 ASCII 跨 item 上下文、选择性 SHY 全文变体、跨样式全词字体选择、位图/混合字体、完整 CST、多 carrier/device/variation、复杂导航/性能和五平台实机继续验收。通用 WebView 仍限 Windows 基础承载，其他平台空后端；Document 内部公式/Mermaid/HTML 渲染保留。全部本批运行句柄已有终态，整体长期目标保持 active。

## R1 文本输入能力与最小后端 Document 验证（2026-10-02）

本批前缀 `artifacts/xui-document-rebuild/validation-2026-10-02-input-caps-`。代理契约版本 13 新增 context/script/language/RTL 四项独立可选输入能力；Context 缓存声明、测量式 fallback 清除额外能力、公共整形拒绝不支持的显式字段、Document 测量/绘制共用派生输入选择。产品源码最终版本在以下构建后没有再修改。

| 最终日志 | 实际退出码 | 证明范围 |
| --- | ---: | --- |
| `default-build.log` | 0 | 当前默认 HB DLL 构建，代理版本 13 |
| `no-hb.log` | 0 | 实际独立无 HB DLL；既有五回调/三个 GPU alpha 与显式输入拒绝；新 Document 两绘制后端共 10 个原生完整画面参考 |
| `native.log` | 0 | 当前 HB DLL 的 Rich 段落/代码、Markdown VISUAL/SOURCE/LIVE 两后端，共 10 个原生完整画面参考 |
| `suite-final.log` | 0 | 完整 Windows 发布：Core/实际 DLL、652 CommonMark、15,470 个 LIVE 位置、16 种公共能力组合与 8 种 Document context/script/绘制组合、新原生专项及既有 2690 连写/84 请求/400 脚本/840 Document 上下文/168 语言、Renderer/Editor/规模/表格/MessageList/原生图像/异步/占位图 |
| `renderer-linux-final.log` | 0 | 当前 Renderer 实际 C ASan/UBSan；含最新全部 16 种能力组合、能力缓存/无回调拒绝/非法输入优先、测量 fallback、Document 八组合与共享语言/段落生命周期 |
| `view-linux-final.log` | 0 | 当前 Rich/Markdown 三模式 View/Editor ASan/UBSan；复杂字素、格式、输入/撤销、语言待输入及上下文所有权 |
| `web-build.log` | 0 | 当前可选测试 DLL 与基础控件测试重新编译；构建阶段没有运行焦点测试 |
| `web-widget.log` | 0 | 默认原生 suite 结束后独立运行：真实焦点/几何、12 次生命周期、两控件及隔离浏览器失败/重建 |
| `web-provider.log` | 0 | 保留的私有 provider/MessageList：错误卡片、截图失败恢复、真实浏览器进程退出与 3 次重建预算 |
| `web-render.log` | 0 | 离线 KaTeX/Mermaid/HTML 测量、HTML sandbox、三张 500×350 PNG |
| `web-exports.log` | 0 | 当前生产可选 DLL 与导出验证；14 个私有浏览器符号未公开 |
| `api.log` / `comments.log` | 0 / 0 | 公共头编译、API 与注释 lint；coverage baseline 未写回 |
| `whitespace-tracked-final.log` / `whitespace-new-final.log` | 0 / 0 | tracked diff、本批 18 个完整 UTF-8 文件（含未跟踪新文件），无 NUL/尾随空白/缺末尾换行；CRLF 提示保留 |

正式反证 `document-red.log` 实际退出 1：既有无 HB DLL 在 Rich `iiéii` 的 RendererLayout 失败，因为最终 group 自动带上该后端不支持的 context/script。修复后同一场景已在实际无 HB 构建通过，且扩展到代码及三种 Markdown 模式。没有改成跳过非 ASCII 字符、关闭所有后端的段落上下文，或把显式语言/RTL 请求清空。缺少能力返回 UNSUPPORTED；源内容、保存格式与语义语言不变。

20 个新原生案例在 HB/无 HB 两 DLL、五种文档形式和两种绘制后端上比较整个画面的精确 alpha。固定字体四个 i 的预期通过字面 PUA alias 绕开 `locl`；é 刻意缺 cmap，逐字回退 advance 为 600/em。总宽字面参考在 40 号分别为 HB 72、无 HB 56，原文末尾 caret 与 GPU 参考均一致；无 HB 最终绘制请求不含 context/script/language/RTL。此夹具证明能力选择与逐字回退正确，不证明所有脚本或真实字体覆盖质量。源码冻结后，新增 partial-capability header 又加入八个 Document 组合；早期 `renderer-linux.log` 对应只含公共 16 组合的版本，最终以 `renderer-linux-final.log` 和完整 suite 为准。

公共专项故意使用能够接受额外输入的宽松测试代理，证明能力缺失会在调用它之前拒绝；不是把该代理当作所有复杂文字的正确 oracle。16 组合覆盖普通输入与每个显式字段、调用方描述不变、失败输出无 clusters/carets、非法输入错误分类以及改变 getCaps 答复后仍使用已缓存政策。Document 八组合确认自动 context/script 可分别省略，测量后最后一次绘制携带同一选择；显式语言/RTL 仍由公共及实际无 HB 专项验证拒绝。

最终默认/Document-local DLL SHA256 均为 `00fe80c829f23bd32f6bbbde3aea61b6efe04f502d52bace8489682da135ef74`。可选测试/生产 DLL 为 `bac197c43fd0072a7757da3cd5f12de08cc9ae46363fdb366d6fab1e26269268` / `ee138b9934e8b8daccea8f090a0ff1514e62fc0f82eb3d680a44e368180e0649`；无 HB 为 `3accbc8db4eda4a8d7454a8049c97bac13cd9e478d95303c2a2101dad869aacc`。三份 context 字体字节仍与上一批一致。此前 schema v7/代理 v12 的通过产物属于历史，不作为 v13 当前构建证据。终态另外保存为 `hashes-final.json` 与 `final-state.json`，包含实际退出码和空运行句柄集合。

范围限制：本批提供最小后端普通非 ASCII LTR 的显示降级，不提供它缺少的复杂塑形、语言或 RTL。能力声明不保证字体有相应字形/GSUB/GPOS，也不替代真实平台质量矩阵。普通 ASCII 仍保持隐式输入，跨 item 的上下文 GSUB 尚未完成；SHY 全文变体复制、完整控制语义、跨样式整词字体选择、位图/混合字体、完整 CST、多 carrier/device/variation、复杂导航/性能、五平台 GPU/IME/读屏/物理 DPI 继续。CommonMark 与 Linux headless 仍有此前说明的证明边界，既有第三方对象未宣称全量 sanitizer。通用 WebView 仅 Windows 基础承载，其他平台空后端；Document 内部渲染保留。本批所有测试句柄已关闭，长期目标保持 active。

## R1 选择性 SHY 段落变体的内存与借用验证（2026-10-02）

本批前缀 `artifacts/xui-document-rebuild/validation-2026-10-02-shy-context-`。产品的完整显示上下文保持不可变，选择性软连字符只在段落拥有的一份工作缓冲中插入；绘制组存位置，文本回调同步借用，嵌套不同位置使用临时副本。缓冲发布前准备，普通位置移动无分配，缓存预算与释放只计一次。产品源码在最终默认、无 HB、可选测试与生产 DLL 构建后未再修改；公共代理版本 13、原生格式版本 7 保持。

| 最终日志 | 实际退出码 | 证明范围 |
| --- | ---: | --- |
| `factory-final.log` / `factory-linux-final.log` | 0 / 0 | 同一产品 C 工厂；六个新增分配失败点、输出/registry 回滚、UTF-8 边界、借用计数上限、嵌套冻结/OOM/重试、能力 reflow 与 1.25 MiB 来源扫描；Linux ASan/UBSan |
| `renderer.log` | 0 | 当前默认 DLL 构建成功及共同 Renderer；Rich/Markdown × 两绘制后端的一份变体、字面全文/item 比较、颜色边界、深处几何、SHY 失败/重试和改宽复用 |
| `renderer-final-compile.log` / `renderer-final.log` | 0 / 0 | 最新原生断言严格编译并运行；20/40 字号 Greek 上下文 SHY 四场景与字面连字符/HardBreak 的 caret、完整 520×160 精确 RGBA 对照；两后端原六个控制/ASCII SHY 场景及共同 Renderer 保持 |
| `suite-final.log` | 0 | 完整 Windows 发布：实际 Core/DLL、652 CommonMark、15,470 LIVE 位置、字体/字素/方向/上下文/语言/能力、Renderer/Editor/规模/表格、Win32 剪贴板、长 MessageList、原生图像/异步/占位图 |
| `renderer-linux-final.log` | 0 | 最新共同 Renderer 实际 C ASan/UBSan；一份变体、复制量上限、颜色裁剪、续排失败/旧几何/重试、快照寿命与既有全部共享回归 |
| `view-linux.log` | 0 | Rich/Markdown 三模式 View/Editor ASan/UBSan；字素/格式/SHY 编辑、光标、撤销、共享语言与输入能力 |
| `no-hb.log` | 0 | 当前独立无 HB DLL 重建；五回调/三个精确 alpha、显式输入拒绝及两后端 Document 普通非 ASCII LTR 原生专项 |
| `web-build.log` / `web-widget-compile.log` | 0 / 0 | 当前可选测试 DLL 与基础控件测试重新编译；构建阶段没有运行焦点测试 |
| `web-widget.log` | 0 | 发布 suite 结束后的独立运行；焦点/几何、12 次生命周期、双控件、隔离浏览器失败/重建 |
| `web-provider.log` | 0 | Document 私有 provider 与 MessageList：错误卡片、截图失败恢复、真实浏览器退出与三次重建预算 |
| `web-render.log` | 0 | 保留的离线 KaTeX/Mermaid/HTML 测量、HTML sandbox、三张 500×350 PNG |
| `web-exports.log` | 0 | 当前生产可选 DLL 重建；基础承载/Document provider 保留，14 个私有浏览器符号未导出 |
| `api.log` / `comments.log` / `coverage.log` | 0 / 0 / 0 | 公共头编译、API/注释与覆盖率检查；baseline 未写回 |
| `whitespace-tracked.log` / `whitespace-new.log` | 0 / 0 | tracked diff 与本批 11 个完整 UTF-8 文件，包含未跟踪新文件；无 NUL/尾随空白/缺末尾换行，CRLF 提示保留 |

正式反证 `red.log` 实际退出 1：36,864 字节来源、27 像素宽度的前缀保留 910 份全文变体，累计 26,093,340 字节，违反每段一份的 28,674 字节上限。修复后的同一断言在四组合实际通过：Rich 28,674、Markdown 因尾随空格投影为 28,673 字节；完整排版的 4096 个 SHY 位置仍仅一份。前缀复制/移动为 60,490/60,489 字节，完整排版为 172,000/171,999；全序列最大 500,006/500,005，每阶段均断言不超过显示段落的 64 倍。该诊断只统计自有变体复制/移动，不把第三方塑形处理或基础投影工作计入。

大工厂回归的 1,310,720 来源字节投影成 786,432 显示字节，在禁止分配的条件下依次访问 262,144 个插入位置，逐位置核对边界，最后逐字核对基础正文以及全文变体；累计复制/移动等于初始复制加位置间距离。小字面样本覆盖向前/向后/首尾移动、相同位置共享、不同位置嵌套临时副本、临时 OOM 输出不发布和外层正文不变、释放可重复、UTF-8 continuation 拒绝与诊断计数饱和。产品本身不依赖这些诊断值来排版。

原生追加的 Greek λ 让缓存真正包含非 ASCII 脚本和 SHY 工作缓冲，测试明确要求选中位置的绘制组使用它。参考 Document 以字面 `-` 与 HardBreak 替代软连字符，绕过本次插入缓冲；两侧 affinity、字号变化和整幅 RGBA 精确一致，没有弱化原像素断言。完整 suite 的 Renderer 二进制构建在这四个新场景加入之前，故追加验证以最终 `renderer-final.log` 为证；共同 Renderer 的最新内存/颜色/工作量断言已包含在 suite 和 Linux 最终构建中。工厂后来补强的大样本全文不变断言以两份 `factory-*-final.log` 为准。

开发过程保留：初始 `build.log` 退出 1，私有断行 helper 接入能力查询时漏传 Renderer 参数，已同步修复 SOURCE/code 与普通段落调用方，最终默认及各 profile 重建通过。`factory-initial.log`、`factory.log`、`factory-linux.log`、`renderer-linux.log` 为较早测试版本，不代替最终新增断言的证据。没有修改字体、第三方 HB 或取消 OOM、几何、内存预算、像素断言。

默认/Document-local DLL SHA256 均为 `0061e2550838039678f40c61f6ea1bea4c6b88b69125dd1f2ad11fd5ae0f4e03`。可选测试/生产 DLL 为 `64bf4ba3b049b1f7b12f3b181bced19f2bdeed4ad1906b4877d78f8dbe7e5003` / `40c5704b8e0f812fc823022885654f148a8705e71aec82cc11d0fa28057f9959`；无 HB 为 `93f56ee42a9b3dd0e55797123ad813f4fa2695805dd122b53d81a7cc14f470ed`。三份 context 字体仍与前批一致。本批最终文件 hash、源码 hash 与实际退出码/空运行句柄集合分别保存为 `hashes-final.json`、`sources-final.json` 和 `final-state.json`。

限制：本批消除每个选中 SHY 保留全文及常规全文复制，远距离位置交替访问和嵌套不同位置仍可能产生更多移动/临时复制，不能推广为任意访问序列的线性时间。Linux headless 不验收原生 GPU/IME/读屏/WebView/物理 DPI，缓存第三方 HB 对象不宣称全量 sanitizer；CommonMark 不证明 HTML 或显示规范。纯 ASCII 跨 item 上下文、完整控制整形语义、跨样式整词字体选择、位图/混合字体、完整 CST、多 carrier/device/variation、复杂导航/性能和真实五平台继续。通用 WebView 仅 Windows 基础承载，其他平台空后端；Document 内部公式/Mermaid/HTML 通道已保持并重验。本批全部测试已有终态，整体长期目标保持 active。

## R1 整形结果与范围绘制缓存（2026-10-02）

证据前缀：`artifacts/xui-document-rebuild/validation-2026-10-02-ascii-context-`。公开代理版本 **14**，Document 原生格式 **7**。本次目标是缓存几何与实际绘制的一致性，非完整跨样式段落整形验收。

| 日志 | 实际退出 | 证明内容 |
| --- | --- | --- |
| `red.log` | 1 | 旧实际 DLL：全文 `a b` 的 GSUB/几何为 32/36 像素，caret 首行 x=42；绘制子串却为 16/12，带普通 HB context 也未替换 |
| `build-final.log` | 0 | 当前默认 DLL 构建成功；原生测试以 Werror 编译 |
| `native-final.log` | 0 | 新项目字体；Rich/Markdown、软换行、affinity、改宽、释放 Document 后的完整 RGBA、绘制零整形；源字节改写、Context/字体先释放、多载荷及连字部分范围拒绝 |
| `oom.log` | 0 | 独立 memory-debug DLL，9 个真实构建分配失败点/重试，fallback 地址寿命，分配差值与自有字节统计完全相等，最终 live allocation 为零 |
| `language-final.log` | 0 | 更新采集器同时记录缓存范围绘制；保持 168 个 en/tr/und、七模式、字体/语言/颜色失效、来源寿命和精确 alpha 场景 |
| `renderer-linux-final.log` | 0 | C ASan/UBSan：新载荷所有权/OOM/重试/预算/改宽/UNSUPPORTED 回退及全部既有 Renderer 回归 |
| `view-linux.log` | 0 | Rich/Markdown View/Editor 三模式、编辑/Undo、字素/语言/源码场景 ASan/UBSan |
| `no-hb.log` / `no-hb-final.log` | 0 / 0 | 实际无 HB DLL 重建与原五回调/三个 alpha/非 ASCII Document 专项；随后最新断言独立重编译，字体释放后的缓存字形完整 RGBA 通过 |
| `web-build.log` / `web-exports.log` | 0 / 0 | ABI 14 可选测试与生产 DLL 重建；基础承载/Document provider 保留，14 个私有浏览器符号未导出 |
| `web-widget.log` | 0 | 重新编译的基础控件独立运行；焦点/几何、12 次生命周期、双控件、独立浏览器退出/恢复 |
| `web-provider.log` / `web-render.log` | 0 / 0 | Document 内部渲染与 MessageList 错误恢复、真实浏览器退出/三次重建预算；离线 KaTeX/Mermaid/HTML sandbox 和三张 500×350 PNG |
| `api.log` / `comments.log` / `coverage.log` | 0 / 0 / 0 | 公开头、API/注释及 no-write 覆盖率，baseline 未写回 |
| `suite-final.log` | 0 | 当前完整 Windows 发布：652 CommonMark、15,470 LIVE 位置、最新载荷/像素/语言断言、Renderer/Editor/字体/字素/方向/规模/表格、Win32 剪贴板、长 MessageList 与原生图像/异步/占位图 |
| `whitespace-tracked.log` / `whitespace-new.log` | 0 / 0 | tracked diff 与本批 24 个完整 UTF-8 文件，含未跟踪文件；无 NUL/尾随空白/缺末尾换行；CRLF 提示保留，旧三字体 hash 未改 |

完整发布套件已实际退出 0。共同 Windows Renderer 与独立原生新增断言均保留精确像素比较，没有关闭缓存接口来通过语言测试。源码中的 Document 释放后完整 RGBA 断言修改时间为 00:55:50 UTC，suite 中对应原生二进制于 00:59:45 UTC 重编译，已包含最终断言；其独立最终运行也通过。

新字体 `xge_ascii_context_fixture.ttf` 的每个 glyph 有独立 PUA alias。完整源字形 4/5 与连字 glyph 8 的参考画面通过字面 alias 绘制，不使用被测上下文子串整形。生成器拥有矩形轮廓、GSUB 与固定时间戳，重复生成 SHA256 不变：`a82753a2b163295c58e198b2e022443b50a8f04fe08ba859756c6306b088112a`。旧三份 context 字体及第三方 HB 未改。

原生 payload 的自有预算包含 native 后端、按实际容量分配的 glyph/caret 数组、font 指针数组、字素/脚本图及 wrapper 载荷；XUI clusters/carets 单独计费，共享字体、atlas 与其 HB 对象不重复算作段落缓存。实际分配差值断言在 fallback/cache 预热之后执行，避免把共享字体缓存误算为载荷；这不表示第三方所有分配经过 sanitizer。

开发过程保留：`build.log` 退出 1（计费语句误放入释放函数）后已修复；首次原生程序因新增代码对整数 `xui_rect_t` 字段使用浮点 isfinite 而触发 SIGILL，`debug.log` 保留栈，修复后最终像素通过。首个 portable 新测试用了会生成行级 paint group 的窄宽度，未进入被测缓存分支；现以单行/改宽定位拥有式合同，软换行由原生专项覆盖。`suite.log` 实际退出 1：语言采集器只观察旧字符串接口；现复制整形时真实语言输入并同时观察范围接口，所有语言/像素断言保持，最终语言专项通过。

限制：本批接入可打印 ASCII、无删除投影的现有 run 缓存分支，未接管行级 paint group。跨颜色/字体节点的共同字形结果、行级边界语义、非 ASCII/删除投影/RTL 范围绘制、完整控制语义、跨样式整词字体选择、位图/混合回退、完整 CST、多 carrier/device/variation、复杂导航/性能和五平台实机仍继续。Linux headless 不验收原生 GPU/IME/读屏/WebView/物理 DPI；CommonMark 只证明相应解析/来源/原生回存/LIVE 场景。通用 WebView 继续仅 Windows 基础承载与其余平台空后端，Document 内部渲染保留，整体长期目标 active。

默认、Document-local 及本批原生专项 DLL SHA256 均为 `6a15da11a2116ac434fdd3e5360e2e512d37db8deea06843febf788a0f88c138`。memory-debug、可选测试/生产、无 HB 及四份字体的 hash 保存于同前缀 `hashes-final.json`；24 个源码/文档保存为 `sources-final.json`。`final-state.json` 区分最终成功与开发失败，全部本批执行句柄为空。此批完成，整体长期目标保持 active。

## R1 跨样式共享字形与行范围（2026-10-02）

证据前缀：`artifacts/xui-document-rebuild/validation-2026-10-02-shared-span-`。代理 ABI **15**，Document 格式 **7**；此批验收限于同字体/垂直样式/语言/解析脚本/LTR 的可打印 ASCII 共享 span。

| 日志 | 实际退出 | 证明内容 |
| --- | --- | --- |
| `red.log` / `green.log` | 1 / 0 | 旧实际 ABI 14：相同 `a b` 的 a-end 因拆颜色节点从 32 变 16；当前 DLL 两侧均为 32 |
| `build-final.log` | 0 | 当前默认原生 DLL ABI 15 重建 |
| `native-ligature.log` / `native-ligature-plain.log` | 0 / 0 | 各 38 个精确完整 RGBA，40/37 字号；Rich 颜色分段、MD 强调共用字体、上下文 GSUB 换行、分数 X/裁剪、改宽与颜色后缓存复用、Document 释放、fi 完整连字分色及内部断行独立整形 |
| `linux-renderer-initial.log` | 0 | 当前 C ASan/UBSan；joint shape owner、整形/查询错误清理和重试、缓存计费、原输入范围坐标、改宽、Document 释放与 UNSUPPORTED 回退；既有全部 Renderer 断言保持 |
| `linux-view.log` | 0 | 当前 Rich/MD View/Editor 三模式、输入/Undo、字素/语言/来源等 ASan/UBSan |
| `oom.log` / `oom-final.log` | 0 / 0 | 当前 memory-debug DLL：9 点构建失败/重试、fallback 载体、精确自有字节及零 live allocation；最终程序另验证 20 spans 绘制堆分配失败和非法重叠均无像素，GPU clip/live storage 不变，重试完整 RGBA 与预热参考相等 |
| `no-hb.log` | 0 | 实际无 HB DLL 重建与原生基本文本载荷、字体释放、五回调和普通非 ASCII Document 专项 |
| `suite-final.log` | 0 | 完整当前 Windows 发布、原生字体/字素/方向/语言、最新共享字形像素、Renderer/Editor/规模/表格/Win32 剪贴板/MessageList/原生图像和异步示例 |
| `corpus.log` / `live-corpus.log` | 0 / 0 | 独立补跑：652 CommonMark 解析/来源/原生回存，30,940 来源 affinity 位置、Prepare 差分；652 LIVE/15,470 UTF-8 光标边界 |
| `web-build.log` / `web-exports.log` | 0 / 0 | ABI 15 可选测试/生产 DLL，基础承载与 Document provider；14 个私有浏览器符号未导出 |
| `web-widget.log` | 0 | 最新重新编译控件独立运行，焦点/布局、12 次生命周期、双控件、浏览器退出/重建 |
| `web-provider.log` / `web-render.log` | 0 / 0 | Document/MessageList 渲染失败与截图错误恢复、浏览器退出/三次重建预算；离线 KaTeX/Mermaid/HTML sandbox 和三张 500×350 PNG |
| `api.log` / `comments.log` / `coverage.log` | 0 / 0 / 0 | 公共头、API/注释门槛及 no-write 覆盖率，baseline 未写回 |
| `whitespace-tracked.log` / `whitespace-new.log` | 0 / 0 | 全仓 tracked diff、本批 18 个完整 UTF-8 文件（含新文件）、四份字体和 DLL hash 检查 |

本次 suite 未传 corpus 参数，其 CommonMark/LIVE 跳过提示保留。当前 core 与 Renderer 程序另行补跑，实际退出均为 0；不把独立语料结果说成该 suite 自身包含。suite 中共享字形专项已包含 38+38 个最终 fi 断行像素场景，新增 GPU 绘制故障注入由独立最终 OOM 程序执行，不声称该 suite 包含它。

开发失败保留：`native-initial.log` 和 `native-diagnostic.log` 退出 1，暴露颜色修改后再改宽因 shaped_revision 旧值丢弃缓存，修复后原缓存身份断言保持。测试初稿曾给 Markdown 节点设置无法由现有源码表达的任意颜色，被现有属性规则拒绝；当前 MD 用强调节点共用字体，颜色编辑针对 Rich。`native-plain-diagnostic.log` 退出 1，因为采集器误认为普通颜色裁剪也只调用一次范围绘制；现在独立断言三个颜色调用，保留零原始字符串绘制与完整 RGBA。`oom-diagnostic.log` / `oom-diagnostic-detail.log` 退出 1，因为内存基线在清除旧错误诊断前取得；旧诊断清理少一块 120 字节（无 invalid/double free）。改为先清除诊断再取基线，失败前后 live allocation 的相等断言及全部像素/裁剪断言保持。没有因此修改产品或重跑已通过的产品回归。

共享 seed 只拥有 joint result 或按 run 索引借用已有结果，行组按 seed 索引和原输入范围访问；数组扩容、continuation 回滚、错误重试、宽度和纯颜色重排均保留正确寿命。缓存计费包含 offset/advance 表、拥有式字形与几何数组，仍保留且计费各节点的旧结果，不宣称零重复存储。连字内部切点使用原行整形，未接管非 ASCII/RTL/删除投影/SHY；跨字体/字号上下文、整词回退、完整 CST、多 carrier/device/variation、复杂导航/性能和真实五平台验收仍在推进。Linux headless 不证明 GPU/IME/读屏/物理 DPI，CommonMark 不证明 HTML 一致性。通用 WebView 仍仅 Windows 基础承载、其余平台空后端，Document 内部渲染保留。

默认、Document-local 和本批原生专项 DLL SHA256 同为 `eee7fcf60b01d86533c6ec713cfb6aed08812d5ae593b589202dccb9b91b5829`。完整源码及其他 DLL/字体 hash 保存为同前缀 `sources-final.json` / `hashes-final.json`，最终状态见 `final-state.json`；本批执行句柄为空，整体长期目标仍为 active。

## R1 Unicode 最终行字形复用（2026-10-02）

证据前缀：`artifacts/xui-document-rebuild/validation-2026-10-02-line-paint-`。ABI **15**、原生格式 **7** 不变；本批复用已经确定的最终行组结果，不宣称非 ASCII/RTL 的完整段落共同字形已实现。

| 日志 | 实际退出 | 证明内容 |
| --- | --- | --- |
| `red.log` / `green.log` | 1 / 0 | 旧 ABI 15 实际 DLL：Greek 最终行 `(λ)a` 有一次原字符串绘制；当前相同输入零次 |
| `build-initial.log` | 0 | 当前默认 DLL 隔离重建，发布到默认及 Document-local |
| `native-final.log` / `native-plain-final.log` | 0 / 0 | 各 90 个独立 PUA glyph 的完整 RGBA；Greek 括号/Kana locl/Arabic 连写 RTL、Rich 全段/分色、MD VISUAL/SOURCE/LIVE、40/37 字号、换行/裁剪/分数 X、颜色缓存、Document 释放；绘制零原字符串/零整形；完整 RTL 查询及部分 RTL 查询清零/绘制前拒绝 |
| `renderer-final.log` / `linux-renderer-final.log` | 0 / 0 | 当前 portable 合同：Unicode owner、后续行构建 OOM 清理/重试、计费、改宽、Document 释放、UNSUPPORTED 后逐字节相同行输入回退、真实绘制错误传播；Linux C ASan/UBSan；原 SHY、语言/输入能力、来源与深处增量断言保持 |
| `linux-view.log` | 0 | Rich/Markdown View/Editor VISUAL/SOURCE/LIVE 输入、字素/来源/语言、导航/Undo/命中/绘制 ASan/UBSan |
| `oom-context-final.log` | 0 | 新 10 点 context 整形故障/重试、4 KiB/1 MiB Unicode context 均保留 328 字节相同载荷、精确 live 字节计费、原 context 改写/释放后范围查询；原 9 点构建失败、fallback 载体和纯整形最终零 live allocations；20 spans GPU 堆分配失败/非法重叠均无像素、clip/live storage 不变、重试完整 RGBA |
| `no-hb.log` | 0 | 实际无 HB DLL 重建，基本拥有式字形与字体释放、五回调、普通非 ASCII Rich/code/MD 三模式全帧像素 |
| `linux-native-final.log` | 0 | 实际 C fallback helper 的 ASan/UBSan：长透明标记/上下文、完整字素/词、GSUB/locl、NFC/ignorable、逐分配失败/重试及线性 unmatched-word；使用真实 HB 字体和无窗口字体替身 |
| `suite.log` | 0 | 完整当前 Windows 发布；含明确传入的 652 CommonMark/15,470 LIVE 边界、最新 180 行缓存 RGBA、实际 DLL、Context/语言/输入能力/字素/Bidi/字体、Renderer/Editor/表格/规模/剪贴板/MessageList、原生异步及占位图示例 |
| `web-build.log` / `web-exports.log` | 0 / 0 | 当前可选测试/生产 DLL，基础网页承载与 Document provider；14 个私有浏览器符号未导出 |
| `web-widget-compile.log` / `web-widget.log` | 0 / 0 | 最新基础控件重新编译并独立运行，focus/layout、12 次生命周期、两独立控件与浏览器退出/重建 |
| `web-provider.log` / `web-render.log` | 0 / 0 | Document/MessageList 失败、截图解码/裁剪错误与进程三次重建预算/恢复；保留内部 KaTeX/Mermaid/HTML sandbox 的离线测量和三次 500×350 PNG 捕获 |
| `api.log` / `comments.log` / `coverage.log` | 0 / 0 / 0 | 公共头、API 注释门槛及 no-write 覆盖率，baseline 未写回 |
| `whitespace-tracked.log` / `whitespace-new.log` | 0 / 0 | tracked diff 和本批 19 个完整 UTF-8 源码/文档，含未跟踪文件；无 NUL/尾随空白/缺末尾换行，既有四字体 hash 保持，默认/local/独立原生 DLL 字节一致 |

suite 的 Renderer 在补充“UNSUPPORTED 回退输入逐字节相同”断言前已编译；该最终断言由 `renderer-final.log` 和 `linux-renderer-final.log` 的当前二进制独立补验，均退出 0。suite 中新的原生行程序于 02:53 UTC 编译，包含最终 RTL 完整测量/部分拒绝断言。其他成功初次/中间专项日志保留，不重复替换已通过证据。

开发失败保留：`oom.log` 退出 1，新增测试辅助函数把 `xui_proxy` 指针当作结构值访问，编译报错；改成 `p->` 后只重编译测试，最终全部故障/计费/像素断言通过。`linux-native.log` 退出 1，既有 fallback helper 测试替身缺少此前已加入产品的 `iScriptMapBytes` 字段；同步字段后当前 sanitizer 通过，未改第三方源码或放宽断言。

行组拥有式结果随重排、错误和 continuation 清理释放并计费；原生同步脚本/字素图在整形成功后释放，借用的 fallback 指针清空，published glyph/caret/font 结果保持。328 字节只对应该专项的两字节输入和固定字体，不是完整段落或任意字体的通用内存上限。ASCII shared seed 保留，Unicode/RTL 最终行缓存也不等于跨字体/字号的完整段落共享整形。显示投影与 SHY 可进入该拥有式行路径，但本批新增 180 场景不冒充覆盖所有控制字符；既有专项回归保持。

未完成项仍包括跨样式完整上下文/整词回退、非 ASCII/RTL 的段落共同字形、完整行边界语义、完整 CST、多 carrier/device/variation、复杂导航、性能和五平台 GPU/IME/读屏/DPI 实机验收。Linux portable 与 fallback helper 不证明完整原生 GPU/WebView；CommonMark 不证明 HTML 一致性。通用 WebView 仅 Windows 基础承载，其余平台空后端，Document 私有公式/Mermaid/HTML 渲染保留。

默认、Document-local 和本批独立原生 DLL SHA256 同为 `246ede027630d0daaa26d2c4a89494227da98f2876fc3543133f710b04f565a1`；19 源码/文档及其余 DLL/五字体记录于同前缀 `sources-final.json` / `hashes-final.json`。终态与实际退出码见 `final-state.json`；本批完成，整体长期目标仍为 active。

## Unicode LTR 共同字形与 Renderer 生命周期批次（2026-10-02）

日志前缀：`artifacts/xui-document-rebuild/validation-2026-10-02-unicode-span-`。各退出码均观察到实际结束，不以日志末尾或等待超时推定。代理 ABI 保持 **15**，Document 原生格式保持 **7**。

| 日志 | 实际退出 | 证明内容 |
| --- | --- | --- |
| `red.log` / `red-detail.log` | 1 / 1 | 旧实际 DLL 的 Greek `λ μ` 窄行 λ advance=16，独立完整上下文字形应为 32 |
| `brackets-red.log` | 1 | 旧实际 DLL 的双字体 ASCII 括号借用孤立脚本，与 Greek 段落 context/script 的 caret 不一致 |
| `build-final.log` | 0 | 最终生命周期和 Unicode span 产品源码的隔离 DLL 重建，发布到默认与 Document-local |
| `suite-final.log` | 0 | 全程使用最终 DLL 的完整 Windows 发布；明确 652 CommonMark/15,470 LIVE 边界、新 Greek 76 与双字体括号 24 完整 RGBA、既有 ASCII 76/Unicode 行 180、Context/语言/输入能力/字素/Bidi/字体、表格/Editor/规模/剪贴板/MessageList及原生异步/占位图 |
| `brackets-initial.log` / `brackets-plain.log` | 0 / 0 | 最终 DLL 两种后端独立测试，各 12 个 Rich 字体族/Markdown 粗体字体的完整 RGBA，含 Greek 脚本边界、换行/裁剪/分数 X、释放 Document及零 raw draw/reshape；随后 suite 同样重编译执行 |
| `linux-renderer-final.log` | 0 | 生命周期修复后当前 C ASan/UBSan；ASCII/Greek UTF-8 joint seed owner、shape/query 错误清理与重试、精确计费、改宽复用、Document 释放和 UNSUPPORTED 回退；原语言/SHY/来源/深处增量合同保留 |
| `linux-view-final.log` | 0 | 当前 Rich/Markdown View/Editor VISUAL/SOURCE/LIVE、输入/导航/来源/Undo/命中/绘制 ASan/UBSan |
| `oom-final.log` | 0 | 14 点真实 Unicode Document Layout 分配故障/同 Renderer 重试，λ caret=32，每次 Renderer 释放后 live storage 与基线相同；纯整形/Document 最终零 live/invalid/double free；保留 9 点 base/10 点 context故障及 20 spans GPU 故障/重叠/clip/live/零像素/重试 RGBA |
| `no-hb-final.log` | 0 | 实际无 HB DLL以最终源码重建，基本载荷/字体释放/范围回调及非 ASCII Rich/code/MD 三模式绘制 |
| `fixture.log` | 0 | 新 Greek calt/连字/PUA 字体两次生成字节相同，固定时间戳，未改旧字体或第三方源码 |
| `web-build.log` / `web-exports.log` | 0 / 0 | 最终产品源码的可选测试和生产 WebView DLL；14 个私有浏览器符号未导出 |
| `web-widget-compile.log` / `web-widget.log` | 0 / 0 | 最新基础控件编译及完整 suite 后独立运行，focus/layout、12 次生命周期、两个独立控件与浏览器进程退出/重建 |
| `web-provider.log` / `web-render.log` | 0 / 0 | Document/MessageList公式/Mermaid/HTML、调色板/源修改/错误呈现、截图解码/裁剪恢复、三次进程重建预算；保留离线测量、HTML sandbox 与三次 500×350 PNG捕获 |
| `api.log` / `comments.log` / `coverage.log` | 0 / 0 / 0 | 公共头、API 注释门槛及 no-write覆盖率，baseline 未写回 |
| `whitespace-tracked.log` / `whitespace-new.log` | 0 / 0 | tracked diff与本批 16 个完整 UTF-8 源码/文档（包括未跟踪文件），无 NUL/尾随空白/缺末尾换行；旧五字体未变，新字体确定性 hash，默认/local/隔离 DLL字节相同 |

中间证据保留：`suite.log` 虽实际退出 0，但运行时修复了 Renderer 生命周期，后续 build helper 重建 DLL，不能作为全程最终构建的证明。`native-initial.log` / `native-plain-initial.log` 各 38 个精确 RGBA 已验证 Unicode span，但早于生命周期修复；终态使用 `suite-final.log` 的重新编译/执行结果。最终 Linux、OOM、无 HB 和 Web DLL也全部重新构建。没有重写前批 `line-paint-` 或 `shared-span-` 的哈希/终态记录。

开发失败保留：`oom.log` / `oom-diagnostic.log` 退出 1，实际内存调试在 `lib/xrt/xrt.h:100701` 记录非法释放；Document Renderer 将嵌入式/栈上 `xrtMapInit` 对象交给会释放结构本身的 `xrtMapDestroy`。全部 11 个相应路径改成 `xrtMapUnit` 后，原严格 live/invalid/double 计数和完整分配故障重试均通过。`brackets-compile-diagnostic.log` 退出 1，新测试把公开固定数组 `sFontFamily` 当成指针赋值；改为复制字节后两后端通过，未为此改变产品或断言。

共同字形仍限无 Bidi 分析、无删除投影、无 SHY 的 LTR span；字体/字号/垂直样式/语言/解析脚本仍为整形边界。有脚本图时单 run也新建正确段落 context/script 的 owner，不借用孤立 ASCII 括号字形。行范围使用原 UTF-8 坐标，完整 cluster 才共享；切入连字内部仍采用既有最终行路径。各节点旧几何也继续保留并计费，不宣称整个 Document 的所有数据去重。Greek GSUB与字面 PUA画面对照证明固定字体的上下文及几何/绘制一致，不能替代真实复杂字体/平台矩阵。

跨字体/字号上下文与整词回退、完整控制/行边界语义、RTL/Bidi/删除投影的段落共同字形、完整 CST、多 carrier/device/variation、复杂导航、性能矩阵和五平台 GPU/IME/读屏/DPI 实机仍需继续。Linux headless 不证明原生 GPU/WebView；CommonMark 不证明 HTML 一致性。通用 WebView仅 Windows基础网页承载，其余平台空后端；Document 内部公式/Mermaid/HTML渲染保留。

默认、Document-local和本批隔离原生 DLL SHA256 同为 `684de83058c02dd30578f44e61c069ea18d9fa05fe218009ddae9e71668a5899`；新字体为 `4e16221b92fac91bbdfce7c5cc2d4eebdffc5c83c54163965842ea4625c1f720`。16 个源码/文档、七 DLL和六字体的记录见同前缀 `sources-final.json` / `hashes-final.json`，最终实际退出码与空运行句柄表见 `final-state.json`；本批完成，整体长期目标保持 active。

## RTL/Bidi 共同字形与独立缓存分色批次（2026-10-02）

日志前缀：`artifacts/xui-document-rebuild/validation-2026-10-02-rtl-span-`。观察全部实际退出码；等待超时不视作完成。代理 ABI 保持 **15**，原生格式保持 **7**。

| 日志 | 实际退出 | 证明内容 |
| --- | --- | --- |
| `red-final.log` / `range-red-final.log` | 1 / 1 | 最终测试源码/最终字体在封存的旧 DLL：Hebrew א advance=16 应为 32；完整 cluster 的 RTL 子范围拒绝。原 `red.log` / `range-red.log` 也保留 |
| `build-final.log` | 0 | 完整 RTL range、Document 共同 seed和 Renderer 缓存分色产品源码的最终隔离 DLL；实际完成后才作为最终原生验证基线 |
| `native-final-2.log` / `native-plain-final-2.log` | 0 / 0 | 各 86 个完整 RGBA；Rich whole/颜色拆分、Markdown 强调、40/37 字号、GSUB 邻居/连字完整分色和内部断行、裁剪/改宽/颜色缓存/Document释放；五种 RTL 子范围×三种 X偏移、输入改写/字体先释放；独立 `i א ב i` 宽度/视觉 x 与 UBA L1 行尾空白。字符串 spans 开启/关闭均用缓存分色，零 raw/reshape |
| `line.log` / `line-plain.log` | 0 / 0 | 各 120 完整 RGBA，原 Greek/Kana/Arabic 90 保留并新增 Hebrew 30；Rich 整段/分色、Markdown VISUAL/SOURCE/LIVE、40/37 字号、换行/裁剪/小数 X、颜色修改、Document释放、零 raw/reshape。随后 suite 重新编译最终输出标签的同一断言 |
| `linux-renderer.log` / `linux-view.log` | 0 / 0 | 当前实际 C ASan/UBSan；新增 RTL owner、关闭字符串 spans 的缓存路径、shape/query 失败/清理/重试、UTF-8范围、改宽与 released Document，原行载荷/语言/SHY/来源合同及 Rich/MD View/Editor 三模式保持 |
| `oom.log` | 0 | 新 46 点真实 RTL Document Layout故障，同 Renderer重试后 caret=10，每次释放后 live storage与基线相同；原 Greek 14/base 9/context 10点及 20 spans GPU故障/非法重叠/clip/live/零像素/重试精确 RGBA；纯整形/Document最终零 live/invalid/double free |
| `no-hb.log` | 0 | 实际无 HB DLL最新源码重建，基本载荷/字体释放/五回调及非 ASCII Rich/code/MD三模式绘制，明确 RTL等请求仍拒绝 |
| `suite.log` | 0 | 完整当前 Windows发布，全程使用最终产品 DLL；明确 652 CommonMark/15,470 LIVE边界、284个公开函数/无旧Rich API、本批 232个新增完整 RGBA、全部原 Context/语言/输入能力/Unicode/字体/Bidi、表格/Editor/规模/剪贴板/MessageList及原生同步/异步/占位图 |
| `fixture-final.log` | 0 | 最终自有 Hebrew calt/连字/Latin i/PUA字体重复生成字节相同，固定时间戳；旧六字体不改 |
| `web-build.log` / `web-exports.log` | 0 / 0 | 最终产品源码的可选测试/生产 WebView DLL；基础承载和 Document高级 provider，14个私有浏览器符号未导出 |
| `web-widget-compile.log` / `web-widget.log` | 0 / 0 | 最新基础控件编译及完整 suite后独立运行，focus/layout、12次生命周期、两独立控件与专属浏览器进程退出/重建 |
| `web-provider.log` / `web-render.log` | 0 / 0 | Document/MessageList KaTeX/Mermaid/HTML、主题/源编辑/错误显示、截图解码/裁剪恢复、三次进程恢复预算；保留离线测量、HTML sandbox和三次 500×350 PNG捕获 |
| `api.log` / `comments.log` / `coverage-final.log` | 0 / 0 / 0 | 最终头的发布导出、API注释和 GCC语法门禁、no-write覆盖率及前后 baseline SHA相同 |
| `whitespace-tracked.log` / `whitespace-new.log` | 0 / 0 | tracked diff和本批17个完整 UTF-8源码/文档（含未跟踪），无 NUL/尾随空白/缺末尾换行；旧六字体未变、新确定性字体及默认/local/隔离 DLL字节相同 |

开发失败与中间证据保留：`native-initial.log` / `native-diagnostic.log` 退出1，37 px的独立 L1测试把宽度恰设为 `size*1.9`，未计入 native float advance的微小二进制偏差；最终选 `size*1.91` 的明确内部位置，仍排除下一字符，保留相同 L1行尾和独立几何断言。`compile-diagnostic.log`、`diagnostic-compile.log` 各退出1，分别是测试同一行 if的误导缩进和临时诊断把整数 rect字段当 `%g`；仅修复测试写法，后者临时代码已移除。

`native-plain-final.log` / `native-plain-diagnostic*.log` 退出1，37 px的分色边界像素与独立逐 glyph参考不一致。旧 Renderer在字符串 spans关闭时仍空间裁剪，即使缓存 spans已有能力；现优先缓存分色，像素断言不放宽。`native-final.log` 退出0是此修复前的 spans配置中间结果。`native-current.log` 的 spans也退出0，但对应 combined调用实际退出1，因为最终 DLL重建尚未结束，plain使用旧 Renderer不满足新的按 group绘制计数；不是最终构建证据。最终两配置由确认 `build-final.log` 实际退出0后运行的 `native-final-2.log` / `native-plain-final-2.log` 和 suite证明，不重写这些中间日志。

完整 suite运行结束后，仅更新 `xui.h` 的两个回调说明：RTL子范围重定位、缓存分色独立于字符串分色。没有修改公开声明、结构、ABI或任何产品 C源码；该注释更新由最终 public-header GCC语法和 API/注释门禁验证，不为注释重新生成 DLL。suite中的行程序已经包含全部新增 Hebrew样本、完整查询和 UTF-8/连字内部拒绝断言。

共同 span仍排除删除投影/SHY，字体/字号/垂直样式/语言/解析脚本/方向级别仍为边界。RTL用最后逻辑 cluster首 glyph的视觉起点平移原位置，保持原字形/caret/颜色坐标；L1行尾级别不一致时只为该组重新整形。当前固定字体与 232个新增画面不证明所有复杂控制、跨字体上下文/整词回退、多 carrier/device/variation、完整 CST、复杂导航、性能矩阵或五平台实机。真正没有缓存分色能力的空间裁剪后端仍有重叠墨迹颜色限制。Linux headless不证明原生 GPU/WebView；CommonMark不证明 HTML一致性。通用 WebView仅 Windows基础承载，其余平台空后端，Document私有公式/Mermaid/HTML保留。

默认、Document-local和本批隔离原生 DLL SHA256同为 `7a3331434702caf45e1aa3f4194bb08230472b1f5fad5a3d4b524cbc03dee5ae`；新字体为 `bc8194d87a8e0c75dafd1f6dba8beba38d392d69f2c71fc2d85ff128acbb4288`。17源码/文档、七 DLL与七字体见同前缀 `sources-final.json` / `hashes-final.json`，实际退出码和空运行句柄表见 `final-state.json`；本批完成，整体长期目标保持 active。

## 删除投影的共同字形与可见字体缓存键批次（2026-10-02）

日志前缀：`artifacts/xui-document-rebuild/validation-2026-10-02-projected-span-`。全部最终测试已观察实际进程退出；代理 ABI **15**、原生格式 **7** 保持。修改产品源码只涉及 `src/xui_document_layout.c` 的共享投影、可见字体键与 source-run 借用条件；不修改控制字符政策或 Document 源码内容。

| 日志 | 实际退出 | 证明内容 |
| --- | --- | --- |
| `red.log` / `red-glyph.log` / `red-final.log` | 1 / 1 / 1 | 封存旧 DLL 的投影缺少共同 owner；单独几何探针在 λ+ZWSP 窄行得到 advance=16，应为 32。最终测试源码也拒绝同一旧 DLL；字体不变 |
| `build-final.log` | 0 | 最终投影/字体键产品 C 源码的隔离默认 DLL，实际完成后发布到默认和 Document-local |
| `native-final-7.log` | 0 | 两配置各 252 个完整 RGBA：Greek/Hebrew GSUB、ZWSP/WJ/BOM/isolates、Rich 整段/分色/隐形字体/颜色修改、MD VISUAL/SOURCE/LIVE、40/37 px、caret/hit/source 往返、零宽、重排、裁剪、小数 X、Document 释放；删除控制后跨节点连字分色与内部断行回退。零 raw/reshape；不放宽像素容差 |
| `linux-renderer-final-2.log` | 0 | 当前 ASan/UBSan；原 ASCII/Greek/Hebrew 及新 ASCII/Greek ZWSP/BOM 的 duplicate offsets/zero widths、shape/query 失败清理/重试、缓存计费、改宽、Document 释放、UNSUPPORTED 降级；原全部 renderer 合同保持 |
| `linux-view.log` | 0 | 最终产品源码的 Rich/MD View/Editor 三模式 ASan/UBSan；导航、格式字符、来源、语言、输入与撤销回归 |
| `oom.log` | 0 | 新 Greek/Hebrew 投影 Layout 21/53 点真实分配故障、同 Renderer 重试后 caret=32/10，每次释放后 live storage 与基线相同；原 Greek 14、RTL 46、base 9/context 10、GPU spans 故障/非法重叠/clip/live/零像素/重试精确 RGBA 均保留，纯整形/Document 最终零 live/invalid/double free |
| `no-hb.log` | 0 | 最终产品源码实际无 HB DLL；普通非 ASCII Rich/code/MD 三模式与原五回调、字体释放和明确请求拒绝 |
| `suite.log` | 0 | 最终完整 Windows 发布，全程同一产品 DLL；652 CommonMark 解析/原文往返及增量差分、15,470 LIVE 边界、284 导出/无旧 Rich API、新 504 精确 RGBA/caret-hit、全部旧文本/字体/语言/Bidi/Renderer/Editor/表格/规模/剪贴板/MessageList 和原生同步/异步/缺图示例 |
| `suite-gates.json` | 全部 true | 实际 suite 退出 0 后独立核对 11 个必要终态标记，包含两配置的新场景及终末成功行 |
| `web-build.log` / `web-exports.log` | 0 / 0 | 最终产品源码的可选测试/生产 WebView DLL；只公开基础网页承载和 Document provider，14 私有浏览器符号未导出 |
| `web-widget-compile.log` / `web-widget.log` | 0 / 0 | 当前基础控件严格编译、suite 后独立 focus/layout、12 次生命周期、双控件及浏览器进程退出/重建 |
| `web-provider.log` / `web-render.log` | 0 / 0 | 当前 DocumentView/MessageList 公式/Mermaid/HTML、主题、源编辑、错误显示/恢复、截图解码/裁剪与三次重启预算；离线测量/HTML sandbox 和三张 500×350 PNG 保留 |
| `public-api.log` / `comment-lint.log` / `comment-coverage.log` | 0 / 0 / 0 | 284 公开函数、头语法和 API 注释；no-write 覆盖率及 baseline 前后 SHA 相同 |
| `whitespace-tracked-final.log` / `files-final.log` | 0 / 0 | tracked 差异、9 个本批完整 UTF-8 源码/文档、无 NUL/尾随空白/缺末尾换行；默认/local/隔离 DLL 相同、七份字体相对上批不变及旧 DLL 封存校验 |

开发证据按原样保留：`red-build.log` 退出 1，测试误用了不存在的 `block_count`，修正私有检查后旧 DLL 反证成立。`native-first.log` 的批处理返回 0、日志为空，不能作为运行成功；`native-direct.log` 实际退出 -1073741819，GDB 显示新测试在 SOURCE 模式错误读取 `r->blocks[0]`，它使用 source_rows。测试改为 VISUAL 的私有 metadata 检查，以及全部模式共同的公开 caret/hit/source 与回调载荷检查，不改变产品。`native-final.log` 同样是旧批处理包装返回 0，却记录 CHECK 失败，不能作为终态。新脚本改为两配置分别编译/运行及显式非零返回检查；最终 `native-final-7.log` 和 suite 实际退出 0。

`native-cache-diagnostic.log` / `native-final-2.log` 退出 1：测试对 SOURCE 强加了 owner 改宽复用/软换行计数，实际既有策略是改宽丢弃 source rows 缓存、保持水平代码行。改为只对 VISUAL/LIVE 检查共同 owner 复用，并用所有模式的独立字面 glyph advance、完整 RGBA、公开 caret/hit/source 与零绘制 reshape 断言确认正确性。`native-final-3.log`、`native-final-5.log` 退出 1，分别是测试 signedness 和同一行 if 的 misleading-indentation；仅修复写法。`linux-renderer-final.log` 退出 1 也是新测试 signedness，`linux-renderer.log` 退出 0 只覆盖当时原三个 portable 样本，最终以 `linux-renderer-final-2.log` 的五样本为准。`native-final-4.log` / `native-final-6.log` 各退出 0 为 480/504 场景的中间结果，后者尚未加入最后的 hit 断言，最终采用第 7 次及 suite。

投影索引累加过滤后的显示字节，重复边界对应零宽源片段；原文字节、节点和源码映射不重写。投影 span 新建实际显示输入的 owner，不借用原 run 原文范围，字体键采用第一可见片段。字体/字号/垂直样式/语言/解析脚本/方向级别边界与 UBA L1 防误用保持，完整 cluster 才共享，连字内部切点继续独立塑形。SOURCE 缓存策略仍需后续性能工作。含 SHY 的组仍排除共享，不宣称全部复杂控制/PS/行边界、跨字体整词回退、多 carrier/device/variation、完整 CST、复杂导航或五平台实机完成。Linux headless 不证明 native GPU/WebView，CommonMark 不证明 HTML 一致性。通用 WebView 仅 Windows 基础承载，其余平台空后端；Document 私有公式/Mermaid/HTML 通道保留。

默认、Document-local 与本批隔离 DLL SHA256 同为 `b1695c66fbecd0547d50e4f183f07a474a206540e02abc67ba80e6f88168ca81`；七份字体相对上批全部不变。9 源码/文档、七 DLL/七字体和实际退出码见同前缀 `sources-final.json`、`hashes-final.json`、`final-state.json`。本批完成，整体长期目标保持 active。

## R1 SHY 共同字形与候选/精确回退验证（2026-10-02）

本批证据前缀：`artifacts/xui-document-rebuild/validation-2026-10-02-shy-span-`。ABI 15、原生格式 7；以下最终日志均等待进程实际结束，返回码写入同前缀 `final-state.json`。

已观察到的独立成功：`build-final-4.log`、`native-12.log`、`native-plain-final.log`、`oom-final.log`、`shared-focus-2.log`、`linux-renderer-final-4.log`、`bidi.log`、`fonts.log`、`public-api.log`、`comment-lint.log`、`comment-coverage.log` 均实际退出 0。两个 native 配置各 720 完整 RGBA；五个新故障向量分别 25/47/41/35/83 点，原内存向量和像素检查不删减。新字体在另外两个路径重新生成，与发布 fixture 完全一致，原七份 fixture 的 SHA 对上批保持。

旧实际 DLL 的 `native-glyph-red-final.log` 退出 1：同一窄行 λ caret advance=16，字面 alias 应为 32。该反证只去掉产品新私有缓存检查，保留公开位置/独立预设字宽与完整 RGBA，不因旧 DLL 没有新缓存结构而提前失败。旧 DLL 的 SHA 对应上批 `b1695c66...`，封存于 `build/shy-span-old/xge.dll`。

开发失败按原样保留，不能列为终态成功：

- `native-2.log` 退出 1，实际测量拒绝后仍强行留下过宽的 SHY；产品修复统一 rejected cut。
- `native-3.log` 退出 1，SHY-only 行 paint input 为空而几何再插入连字符；产品修复统一显示前缀条件。
- `native-4.log` 退出 1，执行时第 3 次构建尚未结束，使用了上次 DLL；该次不作为最终源码的证据。其后所有复制/测试等到实际构建退出。
- `native-5.log` 和诊断退出 1，是新测试要求了当前固定 HL/BA 引擎禁止的 Hebrew 断点。改为显式检查禁止规则和独立公共字形；Greek RLO 提供实际合法 odd-level 插入，未改变行边界版本。
- `native-6.log` / `native-7.log` 是新增 Greek RLO 字体规则缺少对应输入/查找次序，`native-8` 期间字体生成器报告 FeatureLibError；新 fixture 增加明确镜像 GSUB 模式，并保留独立 glyph aliases 和真实画面对照。
- `native-9.log` / `native-10.log` / `native-11.log` 的 RLO 断行暴露选中 hyphen 前的空格 L1 重置；产品修复前缀级别后第 12 次和普通字符串 spans 关闭配置通过。未选中行的 L1 空白仍不共享不同级别 seed。
- `linux-renderer.log` 的首个循环测试期待 y24，实际代理字体为 20；修正预设行高为 20 后 `linux-renderer-final.log` 通过。本批后来增加额外故障/SHY 合同，以第 4 个最终日志为准。
- `renderer.log` / `renderer-2.log` / `linux-renderer-final-3.log` / `shared-focus.log` 要求 Hebrew SHY 前置向量在 45 宽分成两组，实际固定 HL/BA 规则保留一行。合同显式传入该向量一组，其他旧/新向量仍两组，所有 owner/advance/shape-query failure/UNSUPPORTED 断言保留，`shared-focus-2.log` 通过。
- `oom.log` 为同一 Hebrew 未断行向量误设 caret=10；独立 B36+space10 得 caret46，修改该预设后 `oom-final.log` 的 47 点清理/重试通过。其余四个新向量的公共 caret 仍分别 32/20/32/6。
- `no-hb-shy-compile.log` 退出 1，为测试宏下未使用的 terminal 变量；只移除该配置的变量，两个最终 strict compile 和实际无 HB 160 个完整 RGBA 通过。HB 配置的 calt/终端/独立 glyph 断言保持，最终 suite 两配置验证全部 1440 场景。

| 最终日志/记录（以上述前缀补全） | 实际退出码 | 验证范围 |
| --- | --- | --- |
| `build-final-4.log` | 0 | 冻结后的最终产品 DLL，随后复制到默认与 Document-local；全部最终运行使用该产品或对应选项重建 |
| `suite.log` / `suite-gates.log` | 0 / 0 | 明确 652 CommonMark、15470 LIVE 边界、两配置各 720 新 RGBA、原全部字体/Unicode/Renderer/Editor/表格/规模/MessageList 与同步/异步/缺图示例；14 个 suite/no-HB 必要终态标记全部满足 |
| `linux-renderer-final-4.log` | 0 | ASan/UBSan；八种 ASCII/Greek/Hebrew 基础/ZWSP-BOM/SHY 合同、owner/重复显示边界/shape-query OOM 重试与 UNSUPPORTED；八次重排回退及第九 query/terminal shape 故障清理，后缀 caret36/y20 |
| `linux-view.log` | 0 | Rich/Markdown View/Editor 三模式 ASan/UBSan、输入/格式/来源/导航/Undo 全部原回归；日志中既有 DatePicker maybe-uninitialized 编译警告保留，未改该模块 |
| `bidi.log` / `linux-bidi.log` | 0 / 0 | 新 SHY 前缀 WS 修正、S 分隔符 L1、段落不可变/无 live；91707 字符与 770241 类别/方向官方语料，Linux ASan/UBSan 与严格编译 |
| `oom-build.log` / `oom-final.log` | 0 / 0 | 真正 memory-debug DLL；新 25/47/41/35/83 Layout 故障点与同 Renderer 重试/公共 caret/live storage；原 14/46/21/53、base9/context10/GPU spans clip/live/零像素/重试精确 RGBA 保留，纯 Document 阶段最终零 live/invalid/double free |
| `no-hb-build.log` / `no-hb-shy-compile-final.log` / `no-hb-shy-plain-compile.log` / `no-hb.log` | 全部 0 | 实际无 HB DLL 和五个 Native 程序；原普通非 ASCII/五回调/请求拒绝/字体生命周期/三个绘制检查，以及两配置各 80 新 SHY exact full RGBA、直接 cmap advance、选择/拒绝和无 context recipe |
| `web-build.log` / `web-exports.log` | 0 / 0 | 可选测试与生产 DLL重新构建；仅基础网页承载/Document provider，14 私有浏览器符号未导出 |
| `web-widget-compile.log` / `web-widget.log` | 0 / 0 | 严格编译，suite 和 no-HB 结束后独立运行；focus/layout、12 次生命周期、双控件、浏览器退出/重建 |
| `web-provider.log` / `web-render.log` | 0 / 0 | 按顺序运行当前 DocumentView/MessageList 公式/Mermaid/HTML、主题/编辑、故障恢复/截图解码/裁剪/三次重启预算；离线 HTML sandbox、测量与三张 500×350 PNG |
| `fonts.log` | 0 | 新字体两个其他路径重新生成与发布字节完全一致，原七个 fixture SHA 对上一批不变 |
| `whole-hyphen-oracle.log` | 0 | 审计成功发现待实现差异：最终 DLL 完整 `λ-μ` advance20/6/12，而当前后续行基础 seed 为36；该退出0只表示探针成功，不表示 Document 与完整插入变体一致 |
| `public-api.log` / `comment-lint.log` / `comment-coverage.log` | 全部 0 | 284 公开函数、无旧 Rich API、头语法/注释检查；no-write baseline SHA 前后不变；新 Bidi helper 无公开导出 |
| `whitespace-tracked-final.log` / `files-final.log` | 0 / 0 | tracked diff 与 16 个本批完整 UTF-8 文件/CRLF batches；当前 HB/no-HB 运行程序晚于测试源码，七 DLL/八 fixture 字体与默认/local/隔离 DLL一致性和旧 DLL封存 |

共有结果只用于同字体/字号/垂直样式/语言/脚本/当前方向级别的完整 cluster 范围。选中行拥有真实插入变体，未选中行按重复显示字节索引共享基础 glyph；八次 replan 后 exact candidate 也查询同一 seed，真实 query/shape 错误传播并清理。前缀 WS/BN/format 恢复使用固定属性，S/B L1 和段落不可变保持；这不是全部控制组合显示流 UBA 的完整验收。SOURCE 缓存策略、完整 CST、跨字体整词回退、多 carrier/device/variation、复杂导航/性能和五平台实机仍在长期目标中。Linux headless 不证明 native GPU/WebView。通用 WebView 仅 Windows 基础承载，其余平台空后端，Document 私有渲染通道保留。

选中 SHY 的整词变体尚未向后续行传播。上述完整输入 oracle 的 μ12/当前缓存36 差异是明确待修项；1440 HB 像素场景预设后续行使用基础 glyph，证明该当前行契约的绘制/几何/缓存一致性，并不证明完整插入后的上下文一致性。下一批必须以完整插入变体为独立参照，修改 glyph/caret/断点与缓存所有者后重新完成发布回归；不把整词审计的退出0计成这个待修项已经解决。

默认、Document-local、本批隔离 Native DLL SHA256 同为 `756bc495d15caf8d51c6b920d3edebfb2ad078b4c43d3c5691a33799bdae88b1`，新 fixture 为 `bfd3bc2bdd82197827f4042f36192dd77eed9f7e329e10c9a0511db14ba63185`；16 个本批源码/文档、七 DLL、八 fixture 字体及全部实际退出码见同前缀 `sources-final.json`、`hashes-final.json`、`final-state.json`。本批完成，整体长期目标保持 active。

## R1 SHY 完整插入模式与后续字形传播验证（2026-10-02）

本批前缀 `artifacts/xui-document-rebuild/validation-2026-10-02-shy-variant-`。产品冻结后，默认/local/隔离 Native DLL SHA256均为 `379f01dc2d84a22597d55c805efa88f4f1567715443944900c8784fbe48a8f75`；ABI15、原生格式7保持。旧 SHA756 Native DLL和原八个字体封存不改。本文以下退出码均来自实际结束的进程，日志终态标记另行核对，不能代替进程退出。

| 最终日志（补全上述前缀） | 实际退出码 | 证明范围 |
| --- | --- | --- |
| `build-final.log` | 0 | 最终冻结源码的 Native DLL；复制至默认及 Document-local |
| `public-final-v2.log` | 0 | 仅公共 API：Document后续μ advance12与独立完整 `λ-μ` 相同；旧 DLL在 `public-before.log` 为36，退出1 |
| `suite-final.log` / `suite-gates.log` | 0 / 0 | 652 CommonMark、15470 LIVE边界，HB两配置各760完整 RGBA、四个35007字节/20004片段完整 SHY段落、全部原 Core/DLL/Renderer/Editor/表格/规模/字体/方向/MessageList与原生同步/异步/缺图示例；16项必要终态门槛 |
| `renderer-final.log` | 0 | 原 Native格式控制像素/光标检查、完整Renderer和652项LIVE；旧仅要求单标记recipe的私有所有权断言已支持完整变体，图像与光标比较未删除 |
| `renderer-linux-final.log` | 0 | ASan/UBSan，基础共同 span/删除映射、错误/重试、完整插入模式、单独的未插入候选、最终query故障、零advance连字符、计费与释放；普通无paint后端原回归仍通过 |
| `view-linux.log` / `bidi-linux.log` | 0 / 0 | View/Editor Rich/Markdown三模式输入/历史/导航/命中/绘制；91707字符、770241类别/方向官方Bidi语料及L1/段落不可变/失败回滚；保留未改DatePicker编译警告，不据此改无关模块 |
| `oom-compile-v2.log` / `oom-final.log` | 0 / 0 | 真实memory-debug DLL，SHY五路径25/57/31/47/84点，共244；同Renderer重试、公共caret、完全释放后存活存储一致；原14/46/21/53点与GPU spans clip/零半成品像素/精确RGBA继续通过 |
| `nohb-compile-v2.log` / `nohb-final-v2.log` | 0 / 0 | 真正未链接/启用HB的DLL与五个Native程序；两配置各80个SHY完整RGBA、直接cmap/能力拒绝、缓存/颜色/来源/caret-hit/释放/零raw或reshape；测试程序在最终头文件修改后重新编译 |
| `web-compile.log` / `web-exports.log` | 0 / 0 | 测试及生产WebView2 DLL重建，生产基础公开契约无14个私有浏览器符号 |
| `web-widget.log` | 0 | suite/no-HB实际结束后串行运行，Windows基础控件focus/layout、12次生命周期、双实例与浏览器失败/重建 |
| `web-provider.log` | 0 | 再串行运行DocumentView/MessageList私有公式/Mermaid/HTML测量/显示/主题/编辑、解码/裁剪故障与恢复、浏览器三次重启及耗尽预算 |
| `web-render.log` / `web-artifacts.log` | 0 / 0 | 离线KaTeX/Mermaid/HTML、sandbox与三次500×350 PNG解码；另严格编译的同源探针仅增加原始PNG写出，三张文件已查看，公式/箭头节点/HTML文字与样式实际可见 |
| `syntax.log` / `analyzer.log` | 0 / 0 | 修改布局/渲染/Bidi源码严格 C语法检查；布局真实 `-fanalyzer -c` 无诊断 |
| `public-api.log` / `comment-lint.log` / `coverage.log` | 全部0 | 284个Document公开函数、无旧Rich API、头语法与注释检查、coverage no-write基线不变 |
| `fonts.log` / `whitespace.log` / `files-final.log` | 全部0 | 固定SHY字体两个新路径重复生成与原字节相同、tracked差异检查、完整UTF-8源码/文档和CRLF batches、最新程序、产品冻结指纹与默认/local/隔离DLL一致性、原七字体/SHY字体/coverage/旧DLL不变 |

新像素参照已从上一批“后行基础 glyph”改为完整插入字形。40/37字体下的单SHY后缀和多SHY跨空格上下文均有独立字面glyph和公共caret核对，绘制不得再塑形或调用原字符串。普通字符串spans关闭的配置仍有缓存范围分色接口，不能据它证明完全缺少该接口的后端。连字内部切点和L1级别变化按同一完整模式上下文回退；未穷尽所有控制、字体和cluster组合。

后向影响审计另列，不能混入通过的终态门槛：`future-cached-probe-v2.log`退出0；固定新字体中，第二个连字符把第一 glyph变为46，前行52超过行宽28，当前缓存路径禁止第一SHY并保留第二个，公共advance为16/0/12/20/6/12。`future-no-range-probe-v2.log`退出1，完全关闭缓存范围接口仍保留两个标记，前行20/6与完整模式不一致。字体、生成器及公共探针保存于 `shy-variant-future-probe/`，是下一项可直接复现的剩余正确性问题。最初审计第二词可以整词不换行，原预期不成立；修改审计字体的第二词基础宽度后形成v2，不把初版失败当产品缺陷。单词 `no-range-gap.log`实际退出0，只证明那个简单样本正确；不能外推全部路径。

开发过程退出也保留：两个临时compile-only批脚本最初按搬迁后目录执行，找不到仓库源，退出1，修正工作目录后v2退出0；多SHY探针最初把跨空格的真实GSUB当作两个孤立词，完整输入oracle给122/预期86，退出1，改按完整20/6/36/10/32/6/12参照后通过。首轮`suite.log`退出1在旧私有recipe断言；更新所有权检查后Renderer及完整`suite-final.log`均退出0，产品源码没有为该断言再修改。这些早期日志不算最终通过证据，历史批次检查器不重跑。

原始Web捕获：`shy-variant-web/capture-3.png`为KaTeX，`capture-7.png`为Mermaid，`capture-11.png`为HTML；均为500×350，实际非白内容已检查。源码/七个DLL/八个原字体和审计资产指纹、实际进程退出及未关闭句柄状态见同前缀 `sources-final.json`、`hashes-final.json`、`final-state.json`。本批缓存路径完成，长期目标active：无缓存范围路径正确性、完整控制/跨字体回退/CST/复杂结构命令/性能及真实原生平台验收仍待完成。通用WebView限Windows基础承载，其他平台空后端，Document私有渲染保留；Linux headless不能替代Native GPU/WebView/IME/读屏。


## ABI16基础范围与无缓存SHY批次（2026-10-02）

完整实现及证据边界见 [Document基础范围接口](XUI_DOCUMENT_TEXT_RANGE.md)。最终记录前缀为 artifacts/xui-document-rebuild/validation-2026-10-02-text-range-。默认、Document-local、隔离Native DLL SHA256均为 c4158e7d28dac63a7e7b869d8390873a441819e9a1244ef21bb667561f7e8663，ABI16、原生文档格式7。

原四原生配置共3040个RGBA、新范围188个RGBA，无HB四配置320个SHY和80个范围RGBA；缓存与基础两条内存故障扫描、公共旧退出1/新退出0、完整Windows发布及652/15470检查、Linux Renderer/View/输入sanitizer、analyzer/284个API/注释/不写coverage均通过。内部Web三种渲染截图已检查，浏览器故障/恢复/预算、基础生命周期及不公开14个内部符号通过。

本批basic-plain最初存在37px像素失败，新增输入颜色区间后，在相同字面预期和原字体下通过；无HB首次编译的unused parameter也修正并重新编译运行。所有早期日志保留，不作为最终通过项。实际退出码、完整源码/字体/DLL指纹、产品冻结检查、最终日志门槛见 final-state.json及其关联sources-final.json、hashes-final.json、suite-gates.json和state-check.log，历史检查器不重跑。

本批通过不表示完整设计已完成：控制组合、跨字体整词/CST/导航/性能与真实平台验收继续，通用WebView四个非Windows后端仍按用户决定暂缓，Document内部协议保持。

## R1 ASCII 跨字体脚本与上下文验证（2026-10-02）

实现说明见 [ASCII段落上下文](XUI_DOCUMENT_ASCII_CONTEXT.md)。本批前缀为 `artifacts/xui-document-rebuild/validation-2026-10-02-ascii-script-`。最终Native默认、Document-local和隔离DLL均为 `0d69949692aa25959932a4608a2ce2622e17b361654f79eaf81bf920fd3b16d4`；ABI16、文档格式7保持。上一批隔离Native SHA c4158e7d保持，旧记录和旧字体不重写。

| 终态日志 | 实际退出码 | 证明范围 |
| --- | --- | --- |
| `build.log` / `public-final.log` | 0 / 0 | 当前DLL；12个直接XGE的ASCII/Latin/Common/换行/显式脚本/LTR-RTL字形和cluster检查，加Rich/Markdown公开caret宽度。相同最终探针在旧DLL的`public-before-final.log`退出1 |
| `native-v2.log` | 0 | 108个完整RGBA，40/37字体，Rich跨字体/字号、Markdown小字号粗体、六种缓存/基础/仅drawText/无paint路径、caret/hit、reflow/字体失效/释放Document |
| `full-suite.log` | 0 | 652 CommonMark、15470 LIVE字节边界、284 API/无旧API、3040 SHY与188 RANGE完整RGBA、原上下文/语言/方向/布局/Editor/MessageList/Native示例全部门槛 |
| `context-v3.log` / `context-asan.log` | 0 / 0 | 字面显示投影、ASCII多片段四个工厂失败点及零分配reflow，ASCII SHY插入借用保留单片段无script-map快速路径；Linux ASan/UBSan |
| `renderer-asan-initial.log` / `view-asan.log` / `fallback-asan-v2.log` | 全部0 | Linux Renderer/View/Editor及实际C原生脚本/回退选择器sanitizer；ASCII子item的Latin、必需换行重置、Common、map OOM/重试/缓存；原40000字节不匹配词保持线性检查 |
| `oom-default.log` / `oom-basic.log` | 0 / 0 | 真实memory-debug，新增跨字体/字号布局32/35点，原范围11/2点、SHY/后向影响/Unicode/颜色/clip失败扫描与重试继续通过；相同Renderer重试，释放后live storage相同 |
| 八个 `no-hb-*.log` | 全部0 | 真正不链接/启用HB的DLL；原能力拒绝与派生输入省略，320个SHY和80个RANGE完整RGBA |
| `web-build.log` / `web-widget.log` / `web-provider.log` | 全部0 | 当前测试WebView2 DLL，Windows基础控件12次生命周期与失败重建，Document私有公式/Mermaid/HTML、MessageList、三次浏览器重启及预算 |
| `web-render-final.log` / `web-artifact-final.log` | 0 / 0 | 离线三种500×350 PNG、HTML sandbox；另仅增加原始PNG写出的同源探针，三份截图已查看 |
| `layout-analyzer.log` / `fallback-analyzer.log` / `syntax.log` | 全部0 | 真实GCC analyzer编译及严格C语法 |
| `release-api.log` / `comment-lint.log` / `api-coverage.log` / `fixture.log` / `whitespace.log` | 全部0 | API/头、注释、coverage no-write基线不改，新增字体独立重生成SHA一致，所改tracked文件差异检查 |

新增字体SHA256为 `11daa8330206959251f32a2d08ad423b63498ade2dee81d2dbc7bc490cd0ecb9`，截图为 `ascii-script-web/capture-3.png`（公式）、`capture-7.png`（Start→Finish Mermaid）和`capture-11.png`（Safe HTML样式）。当前源码/产品冻结、DLL/字体/截图指纹、实际退出码与门槛保存于同前缀的`final-state.json`及关联清单。截图查看是视觉检查，单独记录，不混作命令退出。

过程失败保留且不计通过：初版用了没有独立DFLT规则的旧字体，且MD分隔符不成立，退出2，故不作为缺陷证据；新独立字体及有效`(**a**)`的旧DLL复现退出1。初版Native测试复用已绑定代理的context，修正为每个配置独立context后108场景通过。扩展RTL探针原先忘记双字符括号也逐字符镜像，修正字面预期后全部glyph检查通过。Windows GCC缺少UBSan运行库，未安装；同一工厂在Linux ASan/UBSan通过。首个PowerShell 5验证包装器将正常Native stderr诊断转为终止错误，退出1，不代表Native渲染失败；终态包装器等待实际程序退出，两份最终日志退出0，保留该包装器错误记录。此批未重建生产WebView2导出配置；未改公开WebView契约，上批生产14个私有符号不导出的记录保留。

跨item整词字体选择仍未完成：独立`word-context-remaining.log`实际退出1，主字体缺`i`却在完整`ffi`上下文的首个`f`上选主字体。此项作为下一批待修，不加入本轮通过门槛。本轮没有宣称跨字体任意GSUB、完整控制/CST/复杂命令/性能或原生多平台验收完成。长期目标active；通用WebView仅Windows基础承载，其他平台预留，Document内部渲染保留。
## R1 跨 item 整词字体回退验证（2026-10-02）

说明见 [整词字体回退](XUI_DOCUMENT_WORD_CONTEXT.md)。证据前缀 `artifacts/xui-document-rebuild/validation-2026-10-02-word-context-`。默认/Document-local/隔离Native SHA256为`70aa635c18412fd878e716c1e105c4be482b8df9b51360c6505262c5ff7e056f`；ABI16、格式7不变。旧ASCII隔离DLL SHA0d699496保持，旧记录不重写。

| 日志 | 实际退出码 | 证明范围 |
| --- | --- | --- |
| `build-v2.log` / `public-v5.log` | 0 / 0 | 当前DLL；154个字面字体/glyph/advance/cluster与词/脚本/缓存/NFC/释放用例。相同最终程序在旧DLL的`public-before-final.log`退出1 |
| `native-initial.log` / `native-before.log` | 0 / 1 | 新108个独立PUA完整RGBA；旧DLL的同一程序caret/字体宽度失败，作为修复前对照 |
| `joining-initial.log` / `full-suite.log` | 0 / 0 | 2690个既有独立HB整段glyph/GPOS比较；完整Windows发布，652 CommonMark、15470 LIVE边界、284API/无旧API、原3040 SHY/188 RANGE/ASCII108和全部Renderer/Editor/MessageList/Native示例 |
| `linux-v5.log` | 0 | 实际C字体selector ASan/UBSan；新增上下文词段与没有完整覆盖字体时的局部回退、所有分配点失败/同run重试、直接512标记medial覆盖，原40000字节/50002字体访问保持 |
| `renderer-asan.log` / `view-asan.log` / `context-asan-final.log` / `joining-asan.log` | 全部0 | Linux共享Renderer/View/Editor三模式、上下文工厂及实际C joining辅助入口；joining全1114112码点与独立Unicode/HB表 |
| `oom-default.log` / `oom-basic.log` | 0 / 0 | 新整词布局41/44点；context11点，4KiB/1MiB借用上下文最终paint均344字节、精确存储核算；原构建/RANGE/ASCII/Unicode/RTL/SHY/颜色/clip/OOM与重试像素保持 |
| 八个 `no-hb-*.log` | 全部0 | 真正不启用/链接HB的DLL，能力拒绝、普通输入、原320 SHY/80 RANGE完整RGBA |
| `web-widget.log` / `web-provider.log` / `web-render.log` / `web-artifact.log` / `web-html-interaction.log` | 全部0 | 基础WindowsWebView、Document私有provider三次重启/预算、离线公式/Mermaid/HTML测量/PNG、HTML交互面板sandbox/焦点/滚动/链接/历史隔离；三张原始PNG已查看 |
| `web-exports-build.log` / `web-exports.log` | 0 / 0 | 重建生产WebView2配置，基本公开契约保留，14个内部浏览器符号不导出 |
| `native-syntax.log` / `syntax-final.log` / `fallback-analyzer.log` / `layout-analyzer.log` | 全部0 | 完整原生C翻译单元严格语法、实际fallback C入口严格语法/GCC analyzer及布局analyzer |
| `release-api.log` / `comment-lint.log` / `coverage.log` / `fixture.log` / `whitespace.log` | 全部0 | 公开API/头/注释、coverage no-write且基线SHA不改，两份新字体独立重生成逐字节一致，tracked diff检查 |

新主字体SHA256为`ca556c88523f483ea61e712aa99272ce1626da9e98b0456faaf5493f7a4a3fe9`，完整字体为`fcdc97182db7093c7927eacc03ecd23fb7567266aa889f3f8192949b3fd11107`。截图为word-context-web/capture-3.png（分式a²+b²/c）、capture-7.png（Start→Finish）、capture-11.png（Safe HTML）。视觉检查单列，不混作命令退出码。

过程记录保留：初版C helper的misleading indentation被严格编译拒绝；原medial-only用例曾仍按旧item策略期待主字体，现更新整词字体身份并保留独立item覆盖和全部HB glyph/GPOS断言。新增局部回退用例最初未复原后续测试字体链，修正隔离后原线性/字体断言全部通过。公开探针初版先使用显式脚本一致性修正前的DLL；随后独立零advance标记的预期遗漏可见ink宽度及37px向外取整，修正字面预期后154用例通过。错误地单编`src/xge_text_run.c`缺少所属翻译单元的HB类型，`syntax.log`退出1；完整`xge.c`及实际helper入口最终严格检查通过。首次Linux工厂用了未识别的输出环境变量，测试本身退出0；最终改用正确变量重新生成到独立目录。源码通读检查曾把既有Markdown记录的行末空格当成程序源码格式错误；保留历史记录，最终检查完整UTF8及程序源码尾随空白，改动差异由git diff检查。PS5原生stderr诊断可含NativeCommandError包装，包装器使用Continue并等待实际进程退出，本批16个串行运行结果由secondary-exits.json保存，15项退出0，旧DLL反证退出1，包装器本身退出0。

最终清单分开保存最终通过命令、预期失败反证、过程错误、视觉检查、源码/产品/DLL/字体/截图指纹与完整13工作包scope审计。字体选择一致不宣称跨字体任意GSUB、完整UAX词边界、完整CST/命令/性能或多平台实机完成；长期目标仍active，通用WebView只Windows基础承载，Document内部通道保留。


## 2026-10-02 Unicode 17 行边界批次

统一 Document/Rich/Markdown 与共享文本升级到固定 libunibreak8，原始许可证/代码和辅助属性表保留；默认/strict UAX14语料19338/19338、Unicode17字素766/766、官方Line_Break属性1114112/1114112，实际全UTF-8及XUI交集通过，不跳过已知偏差。旧7引擎在同一新语料上1026条不匹配。隔离上游逐序列验证n+1解码及缓冲哨兵；17种规模输入保持已有工作量边界，9个边界构建OOM可重试。

确定性自有字体以字面PUA绘制完整RGBA参照；360个Native与360个真正无HB的Rich整段/分片、Markdown VISUAL/SOURCE/LIVE、六绘制路径、caret/hit、改宽、字体失效和Snapshot生命周期通过。旧封存DLL实际退出1。已有SHY四配置3040个完整RGBA继续核对字面glyph与advance；希伯来BA候选、空格断行与10像素caret按17规则接纳，不放宽宽终端拒绝/像素/所有权。默认和基础两条真实OOM扫描、9程序无HB、完整Windows发布（652项CommonMark、Editor、MessageList及原生示例）、Linux共享文本/Renderer/View/Editor及实际C字体回退的ASan/UBSan均实际退出0。共享文本/控件13程序通过，投影11个构造与9个DPI失败点保留。

可选WebView五个程序均退出0，私有公式/Mermaid/HTML测量及PNG、三次浏览器重启与预算、HTML独立交互保持。生产14个内部浏览器/请求符号与line库符号均不导出；三张PNG已分别检查。默认、Document-local及隔离Native DLL SHA一致为d6bb51e0af3910a1ebb0f320078ba27a10f0380f024156ca07f42066a95ca1ac；37个受保护运行时源文件跨构建与验收未改变，ABI16/格式7不变，API coverage基线不改写。旧日志、字体、DLL与封存清单不重写。

保留开发失败：首次迁移漏掉硬换行扫描旧分类调用（构建1）；属性测试未处理官方ZWJ#紧邻注释（1）；Linux首次命令引用不存在的拆分源文件（1）；旧SHY禁止断点/共享组数预期及首轮完整suite（1）已按官方规则修正；独立Hebrew caret草稿忽略保留空格10像素（1）后按字体度量修正；旧投影夹具在旧/新DLL中均因伪Context查询真实能力而失败，改本地基本能力后0。一次PowerShell逗号参数解析错误与GitHub/Git只读取证网络失败也保留在会话，不作为成功验证。无产品正确性检查被跳过。

最终材料使用artifacts/xui-document-rebuild/validation-2026-10-02-line17-独立前缀，记录真实退出码、源码/资产/DLL指纹和逐项视觉验收。13工作包审核保持整体目标active/未完成；字典分词、语言定制、PS编辑/其余控制、复杂shaping/caret、CST/结构命令、预算性能及真实平台验收继续。通用WebView仅Windows基础承载，其他平台预留；Document内部通道保留。

## 2026-10-02 多层引用选区批次（本批验收通过，整体进行中）

新增实际 DLL 432 个 Rich/Markdown、12 个深度、72 个编辑器及 5,702 个失败点通过；Core 独立入口已通过同一功能与失败扫描，最终 Linux Core ASan/UBSan/652 语料退出零。详细边界与最终封存见 `XUI_DOCUMENT_QUOTE_RANGE.md`。首次扩展套件、列表尾部、元数据及测试自身修复的失败证据保留；最终 Windows 全套的实际退出码已在下方补记，通过凭据均来自关闭后的进程结果。

最终完整 Windows 入口 `validation-2026-10-02-quote-nested-full-suite-v2.log` 实际退出 0；最终 Linux Core/Renderer/View 三条 sanitizer 日志均实际退出 0，Core 包含同一 432/124 层/5702 点专项及 652 项语料。单独实际 DLL Editor 全回归 0；Windows 基础 WebView、私有 provider、离线公式/Mermaid/HTML、原始 PNG 和独立 HTML 交互五程序全部 0，三张 500x350 新截图已查看。生产可选 Web DLL 的 284 API 导出检查 0，14 个浏览器/请求内部函数与私有 Unicode 行符号不导出。两份 GCC fanalyzer、完整原生严格语法检查均 0。默认、Document 本地、隔离 DLL 一致，96 个最终运行期源码指纹一致。

失败记录保留：旧 DLL 多层引用返回 UNSUPPORTED；部分列表项续行尾部、需要新首项编号的列表尾部曾造成候选与目标树不一致；Rich 容器拆分曾丢资源/标题；旧 Editor 用例仍把新增能力断言为禁用，现改为验证成功树/来源/Undo/Redo。测试自身的 Rich 接受掩码、空历史错误码、深层输出容量、shell CRLF 及历史文档 whitespace 范围修复均留原日志。最终通过不覆盖失败日志。当前新包哈希不包含封存/完整性检查进程正在写入的自身日志。整体原始 13 包、完整 CST、复杂结构/定义及其余真实平台继续，目标保持 active。


## 2026-10-03 引用选区原字节与隐藏定义

对应 K2/E1，详见 [引用源码保护](XUI_DOCUMENT_QUOTE_SOURCE.md)。新增 216 个正反 TEXT/GAP 和换行/BOM 场景、12 个深度场景（至 124 层）、4 个 BOM/EOF 黄金文件、隐藏定义空引用，以及 72 个实际 DLL 编辑器场景；9 组完整 OOM 扫描共 5,391 点，源码、树、revision、历史和分配平衡通过。

Windows 完整发布 `validation-2026-10-03-quote-source-full-suite.log`、最终 DLL 专项 `dll-final.log`、Linux Core/Renderer/View sanitizer、两个修改源码的 GCC analyzer 均实际退出 0；652 项 CommonMark parse/source/native-roundtrip 通过，Linux 为无窗口代理。可选 WebView 生产 API（284 个 Document 函数）和私有符号边界通过；5 个运行程序实际退出 0，三张新公式/Mermaid/HTML 截图已检查。所有文件使用本批统一前缀。

原 DLL 的第一例返回 -107，初始候选接受流程的失败已修复并保留过程日志。首次补丁检查受 CRLF 影响失败，LF 的 `changes-v2.patch` 与忽略上下文空白的反向检查通过，新增行单独检查无尾部空白。现有引用内部的多行链接标题仍有 `nonroot-multiline-probe.log` 实际退出 1 的未修复探针：它保持原 tree/source/revision，作为下一项 K2 工作，不计为通过。完整 13 包目标仍 active。

最终默认/本地/隔离 Native DLL SHA256 为 `2ffc4cbf7606a5d3e267905a29d4b049266d2ce493e5d423ec0290b118491661`；93 个其他运行期源码和既有覆盖基线未改。完整终态见本批 checks/source/scope/visual/final-state/hash 清单；不重跑或改写历史封存包。通用 WebView 仍 Windows 基础承载，其他四平台暂缓，Document 私有渲染与独立 HTML 交互保留。


## 2026-10-03 嵌套引用定义值与结果选区

240 个来源场景、80 个实际 DLL 编辑器、12 个深度场景和 20 个五次包裹复合事务已验证；5,139 个分配失败点和 2,413 个取消检查点保持旧文档与分配平衡。已有 Quote/ListItem 的多行链接定义及全部可见子块前缀、选区外脚注来源、Rich/Markdown 结果 GAP 重定位见 [实现与边界](XUI_DOCUMENT_QUOTE_PREFIX.md)。本批 Windows 完整发布、最终 DLL 与已修复公共复现、Linux Core/Renderer/View sanitizer、两份 GCC analyzer、生产接口边界和 5 项浏览器串行程序实际退出 0；三张新 PNG 已实际检查。652 项 CommonMark 为 parse/source/native-roundtrip。证据使用 validation-2026-10-03-quote-prefix-，最终 Native SHA256 为 3bf81a989e7d7d0c69e620fa8a335f10acf0e14194b8647c4b79a8473f72d9a0。选区内部未使用脚注定义的 -107 拒绝保留为下一项 K2 工作，源码/tree/syntax/revision/history 原子。通用 WebView Windows 基础承载、其他四后端暂缓，Document 私有渲染与独立 HTML 交互保留；整体 13 包目标继续。
