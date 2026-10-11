# Document 未发布输入的通用文本范围

## 当前接续：VISUAL Prepare 语义位置与文本范围已接入

2026-10-09，v895—v923：在已有 SOURCE/LIVE 范围基础上，统一 Document 内核增加 `xuiDocumentPrepareSemanticPosition` 与 `xuiDocumentPrepareMapPosition`。语义 Text/Gap 端点使用真实不可变语义操作日志，在共同 base 与同一继续输入链之间正向、逆向、跳代映射；SOURCE 仍委托既有映射。精确 generation、UTF-8、分叉拒绝、别名及失败输出原子性保持；查询不分配，不另建 Document、可写内容或历史。

发布后的 ChangeSet 保留已证明等价的预览语义树，使用结构对应遍历映射节点重排及相邻文本重分段；不是按全文序号或猜测源码位置定位。空 Text/Markdown 空段落按既有语义等价规则规范化。被合并文本的内部 Gap 转为 Text 位置并返回 COLLAPSED；图片自身使用 TEXT，图片前后由父节点 GAP 表示。释放 Prepare 后，保留 ChangeSet 仍可映射精确输入 generation，清除标签且不重复移位。

View/Editor 已接入现有 VISUAL 异步文本编辑的范围生命周期和 renderer 实际 ChangeSet。既有/新建范围、继续、取消、发布回调、外部提交、共同 Undo/Redo、失败 worker、零操作重试和投影 OOM 通过；跨叶及根级跨段落删除/插入后，解析合并文本的范围通过。真实 RUNNING worker 阻塞期间5000次暖查询没有新增 Document 分配。完整 Editor 的128次混合编辑检出零操作日志被误判OOM，修复后两端完整入口和1/10MiB用例通过。

最新集成候选：`E:\codex-builds\xge-document-visual-integrated-20261009-v921-01a09446`，1689来源/脚本/规则、194变化、117新文件、1734物理输入，五份匹配运行依赖接入，主1572保护输入不变，未合入。v923独立接受177条实际正例产品命令：Windows/Linux各137全新/零复用正式库与完整Editor、公开完整Core/输入映射/renderer；两端普通/强制碰撞各28 Document+1 XRT全新内核及新语义专项/完整Core；原Linux View脚本两次ASan/UBSan/泄漏、原Windows新增永久bat。实际对象、独立HarfBuzz与依赖时间戳、Linux ext4/单链接、日志和产物散列均核对；Windows实际XGE图片本轮已查看。

原旧DLL负例编译0/运行1，范围预期cd实际bc；完整Editor零操作回归、私有图片GAP夹具失败及WSL参数转义失败的原日志保留。图片夹具改为合法TEXT，未放宽黄金；WSL只改脚本传输，产品与断言不变。新专项注册后永久矩阵为74私有/53公开，但完整矩阵尚未验收。

**仍待完成：** 全部结构/对象命令的异步Prepare支持及复杂嵌套矩阵；FORMAT/PAGE、属性/可见范围/子对象、完整UIA TextPattern/Grid/Table/事件；真实IME/读屏/DPI/固定性能；最新完整仓库、源码合入及合入后验收。首次plaintext投影仍可能复制全文，本批不证明视口有界。富文本仍沿同一Document同步事务编辑，不能把语义映射通过扩大为全部富文本命令已支持异步Prepare。整体与11工作包继续active。详见[Prepare语义位置映射](XUI_DOCUMENT_PREPARED_SEMANTIC_POSITIONS.md)。

以下历史批次仅对应各自冻结输入和观察时刻；顶部本节为当前状态。

2026-10-09。本批完成 SOURCE/LIVE 的 Prepare 范围界面接入；整体重构未完成。

## 实现

范围端点沿已有 Prepare 输入链映射。接受新的输入 generation 时，保留的范围先完成两端原子映射，再退休旧 job；取消映回共同 base。外部提交先逆映旧输入，再应用外部 ChangeSet。精确 generation 的发布直接使用 ChangeSet 映射，清除 generation，不重复执行补丁；提交回调在刷新 renderer 前清除旧输入投影。范围只保留 View 所属链表及端点，没有新 Document 或历史。

源码 plaintext 的 origin 来自实际 Prepare，不再取可能暂时过期的 selection。候选端点按 Source/UTF8 边界投影和反投影。LIVE 非活动行转换到提交坐标后定位，只搜索编辑区域之前或之后的语义块，再转换边界到输入坐标；共用 renderer 的既有转换规则。

## 验证

最终永久头为 `test_xui/xui_document_text_prepared_cases.h`，由原通用范围驱动最先调用。两个模式各测试旧范围、新范围、查找/克隆/选区/光标、矩形/命中点、行/段落及 UTF8 字素；覆盖继续、取消、发布、多视图、共同历史、模式切换、外部修改、失败 worker 和重试、投影 OOM、空流无变更发布、分段 UTF8 流及销毁/替换。实际 RUNNING worker 各5000次暖查询不调用 Document allocator，清理后零泄漏。

v890独立接受29条实际正例产品命令，包含两端137单元全新/零复用正式库和实际对象/依赖核对、两端完整公开 Core/renderer/输入映射、最终范围与完整Editor、原Linux View sanitizer及原Windows范围/Editor入口。源码与测试头冻结后核对；前一测试头的四条消费者命令不计为最终范围接受。

旧正式DLL对最终测试实际编译0/运行1，范围创建返回UNSUPPORTED。发布回调负例实际编译0/运行1、返回STALE(-102)；错误码记录器第一次预设错误保留，恢复仅核对原日志及头文件声明。初次夹具的头路径、第二View销毁、空流假设及保存输出目录失败均保留；保存夹具只补目录，重跑同一二进制和库，断言未放宽。

证据：`build/source-document-footnote-local-reference-20261006/document-prepared-v890-paired-acceptance.json`。

## 边界与下一步

已提交的 VISUAL 语义范围切到 SOURCE/LIVE 的映射已测试。未发布 VISUAL 语义预览跨 Text 分拆/合并、跨叶/跨块、Gap 和对象的通用映射仍须在统一内核中实现。一般语义 Prepare 范围不能据本批关闭。

FORMAT/PAGE、属性/可见范围/子对象和完整UIA TextPattern/Grid/Table/事件仍待完成。第一次源码投影仍可复制全文，暖查询无分配不证明视口有界、首次建立固定时延或物理性能。完整Editor入口通过也不等于最新73/53全部驱动或真实IME/读屏/平台发布矩阵通过。

最新候选在 `E:\codex-builds\xge-document-prepared-integrated-20261009-v888-01a09446`，尚未合入主工作区；主1572保护输入未修改。完整目标和11个工作包保持active。
