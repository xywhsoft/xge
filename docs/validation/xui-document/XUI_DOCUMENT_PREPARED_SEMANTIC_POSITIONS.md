# Document Prepare 语义位置映射

2026-10-09。统一内核与现有 VISUAL 异步文本编辑范围已通过本批验收；完整重构继续进行。

## 接口与坐标

`xuiDocumentPrepareSemanticPosition` 创建指定不可变预览中的 TEXT/GAP 位置，带文档身份、共同base revision和输入generation。仅SOURCE的Prepare没有预览语义树，返回UNSUPPORTED。普通snapshot接口继续拒绝带输入generation的位置，防止把预览当作已提交版本。

`xuiDocumentPrepareMapPosition(from,to,...)` 支持同一输入链的共同base（NULL端）、正向、逆向及跨代。SOURCE委托已有Source映射，语义位置复用真实事务操作的offset/affinity/祖先规则。共享generation/源码日志前缀证明输入链，字节相同的独立分叉仍返回STALE。错误文档、版本、generation、非法UTF8均拒绝，输出与quality不变；输入输出可以别名。

Prepare只保存持久日志及不可变预览，不保存全部祖先Prepare，不增加可写文档或独立历史。查询不读取worker的可变draft，在worker RUNNING、取消、替代及owner释放后仍可使用保留句柄。

## 发布与语义对应

worker保留既有完整语义等价门槛；通过后，发布ChangeSet保留预览input_state。`xuiDocumentMapPosition` 对精确generation直接从预览映射到实际解析树，赋发布revision并清除generation；不再重复应用输入补丁。普通提交和Undo/Redo沿原有规则。

对应遍历与语义等价的空Text/Markdown空Paragraph规范化及相邻等样式Text重分段保持一致，不建立全文ordinal或源码猜测映射。父Gap位于合并文本内部时返回Text位置及COLLAPSED；对象自身位置遵循schema，IMAGE包含alt文本，因此自身为TEXT，前后位置为父GAP。ChangeSet独立保留输入树，Prepare先释放仍可映射。

View在接受下一generation时原子映射两端，再退休旧job；取消映回base，外部提交先逆映再应用外部ChangeSet，发布使用精确generation，通知回调可读取发布范围。Renderer接受真实完整语义ChangeSet，跨叶/跨块不伪造单文本操作。相同文本替换可能产生零操作，仍保留有效输入链和零操作delta，不能误报OOM。

## 已验证

新永久私有驱动 `tests/xui/xui_document_prepared_semantic_test.c` 覆盖UTF8、别名、跳代/逆向、取消后保留、restart分叉原子拒绝、Prepare先释放的发布映射、实体导致的文本重分段、内部Gap、图片自身与前后位置，以及5000次查询无分配和零泄漏。

新永久UI驱动 `tests/xui/xui_document_visual_prepared_text_test.c` 覆盖范围新建/克隆/查找/选区、RUNNING5000暖查询、继续/取消、发布回调、共同历史、外部写入、worker分配失败和零操作重试、投影OOM、跨叶/根级跨段落及解析合并文本。原完整Editor的128次混合编辑、1/10MiB及故障扫描保持。

v923独立接受177条实际正例命令，核对两端各137全新正式单元、独立HarfBuzz对象、依赖时间戳、普通/碰撞两端各29全新内核、完整Core和Editor、原Linux View脚本两次sanitizer及原Windows新bat。Linux物理输入在ext4/单链接且字节一致，ASan/UBSan符号和泄漏选项已核对。原旧DLL范围错位负例为编译0/运行1；零操作产品回归、图片位置夹具失败、WSL参数转义失败的原证据均保留。

验收记录：`build/source-document-footnote-local-reference-20261006/document-visual-semantic-v923-paired-acceptance.json`。

## 尚未完成

本批没有把所有结构、表格、对象命令改为异步Prepare，也不宣称富文本异步命令或完整嵌套组合已通过。FORMAT/PAGE、属性/可见范围/子对象、UIA TextPattern/Grid/Table/事件、真实IME/读屏/DPI、固定机器性能、首次投影视口预算仍待完成。

永久矩阵已注册74私有/53公开驱动，整个矩阵与仓库迁移尚未验收。最新集成候选在 `E:\codex-builds\xge-document-visual-integrated-20261009-v921-01a09446`，未合入；主1572保护输入未修改，整体和11工作包仍active。
