# Document 引用选区的原始源码与隐藏定义保护

更新：2026-10-03。对应整体重构的 K2/E1；整体目标仍在进行。ABI 16、原生格式 7、统一 Document 内核和公开 API 集合保持不变。

原来的多层 Quote/List/ListItem 结构边界可以正确拆分，但当受影响容器中有链接定义时，通用 Markdown 写回仍可能返回 `UNREPRESENTABLE`。这些定义包括未使用的标签、重复定义和多行标题，不能从可见语义节点重新生成，也不能在结构编辑时丢失。

`WrapQuoteRange` 新增私有来源适配路径。根层新引用的选中子树，如果边缘只需拆分 Quote，则从稳定节点取得原始语法范围；完整选中的列表、表格、代码等块也保留原始范围。改动只添加每行的引用前缀和必要的空白边界，不重新拼写已有文本、实体、标记、围栏、列表编号或链接定义。选区两端之外的源码仍是原字节；多行标题中新增的引用容器前缀属于本次结构变化。

候选源码在私有 transaction 中完整解析。核对目标语义树后，还按解析顺序比较全部链接定义的解析值，包括未使用定义与重复定义；因此外部链接的解析结果也不能意外改变。通过后接入现有统一来源投影和稳定 NodeId 接受流程，再由原事务一次性发布源码、语义树、来源记录、操作与历史。Abort、分配失败和不等价的候选都不发布。

新路径不是另一套可写文档或历史，没有添加公共接口、旧 API 包装或 WebView 通信。非根层的引用、需要新列表项标记的边缘、包含脚注定义的选中来源仍使用现有受检适配路径；不能安全表达的操作继续原子拒绝。完整 CST/token/trivia 与所有隐藏定义结构编辑尚未完成。独立公共 API 探针 `nonroot-multiline-probe.log` 已确认：在现有引用内部再包裹包含多行链接标题的选区，旧路径仍返回 `-107`，保持原源码与 revision。该失败保留为下一项 K2 工作，不计入本批通过。

## 维护入口与专项

专项为 `test_xui/xui_document_quote_source_test.c`；Windows 的 `test_xui/build_document_quote_source_test.bat` 同时运行独立 C 内核与实际 DLL/Editor，已加入完整发布套件。Linux Core ASan/UBSan 入口运行同一专项及完整 CommonMark 语料。

- 216 个场景：9 类结构、正反向、4 种 TEXT/GAP 组合、LF/CRLF/混合换行+BOM；验证 Abort、稳定选中节点、外部字节、定义顺序、任务数量、完整来源/语义重载以及精确 Undo/Redo。
- 12 个深度场景：1/2/8/32/64/124 层引用，正反向，内部多行链接标题。另有 4 个 BOM/EOF 精确黄金文件与仅有隐藏定义的空引用容器。
- 72 个实际编辑器场景：Query/CanExecute、Wrap/Execute、有效结果选区、SOURCE/LIVE/VISUAL 三种投影、共同历史。已有引用中的按钮仍保持切换语义；显式 Wrap API 用于增加引用层。
- 9 组完整分配失败扫描：553/808/535/447/466/526/792/657/607，共 5,391 点；每点保持已提交的 tree/source/revision/history，不产生泄漏，最后成功重试。

## 本轮证据

新包使用 `artifacts/xui-document-rebuild/validation-2026-10-03-quote-source-*`，旧验证包保持。旧 DLL 的独立公共 API 复现 `before.log` 实际退出 1，首例返回 `-107`；旧 DLL、import library 和复现程序留在 `build/quote-source-before`。

最终 Windows 完整发布套件、最终 DLL 专项、Linux Core/Renderer/View ASan/UBSan、两份修改源码的 GCC analyzer、生产接口与私有符号检查，以及 5 项可选 WebView 串行回归均实际退出 0。652 项 CommonMark 检查覆盖 parse/source/native-roundtrip，不是 HTML conformance；Linux Renderer/View 使用无窗口代理，不代替真实平台 GPU/IME/读屏验收。三张新公式/Mermaid/HTML 截图已实际检查。完整门槛、所有保留尝试、源码与输出指纹见本批 `checks-final.json`、`sources-final.json`、`hashes-final.json` 和 `final-state.json`。

本批未扩大通用 WebView 范围：仍只要求 Windows 基础网页承载，其他四个平台后端暂留空；Document 私有公式、Mermaid、HTML 的脚本、测量、截图和独立 HTML 交互通道保留。完整 13 个工作包继续按原设计跟踪，尚未满足整体完成条件。

最终默认、Document 本地和 `build/quote-source-native-final/xge.dll` 指纹一致：`2ffc4cbf7606a5d3e267905a29d4b049266d2ce493e5d423ec0290b118491661`。首个隔离构建保留在 `build/quote-source-final`；发布套件中重建默认 DLL 后，以最终 DLL 再运行完整专项，包含 72 个编辑器场景。93 个其他运行期源码指纹与既有 API 注释覆盖基线保持不变。本轮差异使用 `changes-v2.patch`，CRLF 混合工作区的反向检查通过；首次补丁检查失败日志保留。
