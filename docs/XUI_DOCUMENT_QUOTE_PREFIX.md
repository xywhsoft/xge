# Document 嵌套引用前缀、定义值与结果选区

更新：2026-10-03。对应整体重构 K2/E1，统一 Document 内核、ABI 16、原生格式 7 与公开函数集合保持。整体目标仍在进行。

本批解决上一批公共 API 探针的失败：在已有 Quote 内，将 `take [a]` 至 `also [a]` 再包一层引用，中间含多行链接标题时，旧 DLL 返回 `UNREPRESENTABLE (-107)`。已有列表项、引用中的任务列表项、多行标签/目标/标题、未使用与重复链接定义也使用同一校验路径。

## 内核改动

原前缀适配器比较链接定义的原始字段跨度。多行字段包含容器前缀，新增 `>` 虽然不改变定义值，原字节比较仍判为不同。现在只在确认逐行复制非前缀字节的 Quote/ListItem 适配路径中，按顺序比较全部不可变定义缓存的标签、目标、标题与标题存在性。检查包含未使用及重复定义，不只核对可见链接。完整候选解析还必须与目标语义树等价。

脚注没有同类定义值缓存，因此完整脚注来源仍必须字节相同。本批保护选区外已使用和未使用脚注；不把脚注正文迁移等价性简化为标签比较。校验前使用 `doc_state_clone` 保存旧状态的共享根：仅 retain 事务 draft 不够，因为私有来源补丁会修改同一个 draft 的 source 字段。该克隆只创建状态包装，来源、索引与缓存保持共享，并在所有出口释放。

现有 Quote 的全部可见子块也允许走逐行前缀补丁；开头与末尾隐藏定义继续留在原外层引用。空 Quote 仍由现有受检写回生成实际开头标记。

共同结构命令还修复了返回选区。混合 TEXT/GAP 选区的 GAP 原来仍指向旧父容器及旧 child offset；一次包裹后偏移可能数值合法但指向其他块，后续又变成非法位置。现在 GAP 端点重定位到新 Quote 的对应边缘，普通 TEXT 端点继续保留原节点与字节位置；已拆分的块选区保持现有完整块返回规则。Rich 与 Markdown 共用此修复，可以在同一事务继续使用结果选区。

所有修改仍经原 Document transaction 发布一次 tree/source/来源记录/操作/历史。没有新增可写文本副本、第二套历史、公共 API 或旧接口包装。失败、取消和 Abort 保持已发布文档不变。

## 维护测试

入口为 `test_xui/xui_document_quote_prefix_test.c`。Windows 的 `build_document_quote_prefix_test.bat` 同时运行独立内核与实际 DLL/Editor，已加入完整发布套件；Linux Core sanitizer 运行同一专项。

- 240 个文档场景：10 类来源、正反向、4 种 TEXT/GAP、LF/CRLF/混合换行+BOM。包含实际黄金源码、选区外原字节、稳定节点、任务属性、完整来源/语义重载、定义缓存的独立 oracle、Abort、精确 Undo/Redo。
- 12 个深度场景：已有引用内 1/2/8/32/64/124 层、正反向，多行标题及选区外字节保护。
- 20 个复合事务：10 个 Markdown 与 10 个 Rich，每次连续五层 Wrap，复用返回选区，最终一次发布、一次 Undo；Markdown 再独立完整重载。
- 80 个实际 DLL 编辑器场景：Query/CanExecute/Execute、有效选区、完整重载、SOURCE/LIVE/VISUAL 投影与共同历史。
- 10 组分配失败扫描共 5,139 点；5 组取消扫描共 2,413 点。每个失败核对已发布 tree/source/revision/history 和恢复后的 live allocation，最终成功重试并释放为零。

## 验证证据与剩余范围

本批使用独立前缀 `artifacts/xui-document-rebuild/validation-2026-10-03-quote-prefix-*`。旧 DLL 与公共复现保存在 `build/quote-prefix-before`，原失败实际退出 1。多行字段、外部脚注状态捕获和连续包裹的过程失败日志保留，不覆盖历史记录。

Windows 完整发布、最终 DLL 专项、两个已修复公共 API 探针、Linux Core/Renderer/View ASan/UBSan、两份修改源码的 GCC analyzer、严格 C 编译及生产导出检查均实际退出 0。652 项 CommonMark 覆盖 parse/source/native-roundtrip，不是 HTML conformance；Linux UI 使用无窗口代理，不代替真实平台 GPU/IME/读屏验收。5 项可选 WebView 串行程序实际退出 0，三张新 500×350 公式/Mermaid/HTML PNG 已实际查看。最终结果见本批 `checks-final.json`、`final-state.json`、`sources-final.json` 和 `hashes-final.json`。

这是确认前缀路径的来源与选区正确性补全。完整 CST/token/trivia、其他复杂容器和脚注改写、结构编辑的完整异步/三模式行为、性能及真实平台验收继续按原 13 个工作包追踪。

新增实际 DLL 公共探针 `inside-footnote-final.log` 确认：选区内部的未使用脚注定义确实被识别为 FOOTNOTE，但增加引用层仍返回 `-107`；源码、语义树、来源语法、revision 与历史保持原样。该实际退出 1 的用例作为下一项 K2 工作，不计入本批通过。

通用 WebView 仅 Windows 基础网页承载，其他四平台后端预留。Document 私有公式、Mermaid、HTML 的脚本、测量、截图和独立 HTML 交互保留；不扩大通用 WebView 的公开通信范围。

最终默认、Document 本地与 `build/quote-prefix-native-final/xge.dll` SHA256 一致：`3bf81a989e7d7d0c69e620fa8a335f10acf0e14194b8647c4b79a8473f72d9a0`。首个隔离 Native 构建也为该指纹；旧 DLL `2ffc4...` 及两个实际退出 1 的旧公共复现另存，历史封存包不重跑。93 个其他源码/脚本输入、公开头文件和 API 注释覆盖基线保持。首次导出检查未识别当前 objdump 的表格式而失败，修正后对非空完整导出表检查通过；该过程日志保留。最终差异与反向校验见 `changes.patch`、`diff-check-final.log`。
