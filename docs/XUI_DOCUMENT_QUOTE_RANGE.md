# Document 多层引用选区的结构边界

更新：2026-10-03（本批 2026-10-02 启动）。E1/K2 本批实现完成；最终 Windows 整体发布、Linux Core/Renderer/View sanitizer 与 Web 回归均通过。长期整体重构保持进行中，ABI 16、原生格式 7 不变。

`xuiDocumentTxnWrapQuoteRange` 现在可以提升选区的两端到最近的合法公共父容器，并逐层拆分 Quote、List、ListItem。Text 端点仍扩展到其所属完整段落/标题；GAP 端点表示子节点边界。正向、反向、混合 Text/GAP、同一列表中段、不同深度的两端、列表项的部分子块都使用同一规划。全覆盖边缘继续直接提升。表格等其他容器的部分边界仍受其 schema 和 Markdown 表达能力约束。

原实现只能各拆一个直接边缘容器：跨两层引用到根段落会返回 `UNSUPPORTED`。新规划以左右边界相邻的稳定 ChildId 作为锚点；第一端拆分后，第二端从该 ChildId 的新父节点重新定位。两端完成后再解析一次锚点，避免第二端克隆共同祖先时丢失第一端。编辑器 Query/CanExecute 与实际执行共用这个结构预检，预检不改树、来源、revision 或历史。

有序列表的新后缀延续编号。预检把左端新增的列表项也计入右端编号，`UINT64_MAX` 溢出在结构变更前返回 `LIMIT`。任务项只有前缀保留 TASK/CHECKED，后缀清除这两个标记。拆分的新容器复制全部属性以及 resource、info、title；原有未选子树和选中叶节点继续使用其 NodeId。

Markdown 的保留边界查找也递归穿过 Quote/List/ListItem，保留未选前缀中的实体、原始缩进、换行和 BOM。可直接独立重载的后缀原样保留；部分列表项后缀从续行开始，需要新列表标记，有序列表尾部的原始首项编号与新编号不一致时也必须生成新标记。这些情况使用受影响容器的有限重写，外部来源保持原字节。全部候选仍与目标语义树比较，失败不发布，也不建立撤销记录。

这次没有添加公共 API 或旧接口兼容层。SOURCE/LIVE 模式仍通过同一 Document 切换视图；引用结构按钮当前在 VISUAL 模式可用，SOURCE/LIVE 直接执行该按钮仍返回 UNSUPPORTED。专项验证检查模式切换不创建新 Document、不重导入、不丢失共享历史。

## 验证与证据

专项程序：`test_xui/xui_document_quote_nested_test.c`。Windows 维护入口：`test_xui/build_document_quote_nested_test.bat`，已加入完整 Document 发布套件；Linux Core sanitizer 入口也加入同一程序。

- 432 个 Rich/Markdown 场景：9 种树，2 个方向，4 种 Text/GAP 组合，2 个 profile，3 种 LF/CRLF/混合换行+BOM；逐例验证 Abort、未选前缀和外部来源、节点归属、任务数量、4/5/6 编号、独立完整重载以及精确 Undo/Redo。Rich 额外逐个检查克隆容器的 resource/info/title。
- 12 个深度场景：1/2/8/32/64/124 层引用，两个方向，完整语义树与来源语法重载及精确撤销。该检查验证深度正确性，不是深层规范化输出体积或延迟的性能验收。
- 72 个实际 DLL 编辑器场景：Query/CanExecute/Execute 一致、Text/GAP/反向选区、结果选区可重新设置、SOURCE/LIVE/VISUAL 切换与共享 Undo/Redo；另有两端列表项拆分导致的有序溢出禁用检查。
- 9 组逐分配失败扫描：464、721、526、964、601、684、582、580、580 个失败点，共 5,702 个；每点验证旧 tree/source/revision/history 原子、成功重试和零泄漏。
- 最终 Linux Core ASan/UBSan 与同一专项通过，652 项 CommonMark 0.31.2 parse/source/native-roundtrip 通过；该语料不是 HTML conformance 测试。

独立旧 DLL 复现、每次失败尝试、源码差异、最终 DLL 和验证日志使用 `artifacts/xui-document-rebuild/validation-2026-10-02-quote-nested-*` 新包保存；此前 Unicode17 等封存包保留。封存前核对 96 个运行期源码指纹，默认、Document 本地和隔离 DLL 必须一致。

尚未完成的目标：完整 CST/token/trivia、包含隐藏定义的所有复杂容器改写、其余结构命令及表格/对象/样式完整组合、异步/IME/无障碍矩阵、复杂文本塑形与性能、多设备和真实平台验收。完整 13 包范围保留，不以本批通过代替整体完成。通用 WebView 保持 Windows 基础网页承载、其他四平台后端暂留空；Document 私有公式、Mermaid、HTML 渲染与 HTML 独立交互通道保留。
