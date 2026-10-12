# 统一 Document 文本范围与可访问性后端

更新：2026-10-09。文本范围基础后端已实现并通过专项验证；Windows UIA TextPattern/TextRange 尚未接线，整体重构继续进行。

## 当前接续：统一段落范围已通过

2026-10-09，v812—v829：通用文本范围新增 PARAGRAPH 的扩展、范围移动和端点移动，富文本 VISUAL 与 Markdown VISUAL/SOURCE/LIVE 四配置通过。VISUAL 采用同一语义树的 Paragraph/Heading/CodeBlock，保留显式空段落；内部 SoftBreak/HardBreak 和视觉折行不拆段落。SOURCE/LIVE 的范围投影仍是完整原始源码，按物理空行分组并把后续空行归前段；它不是按 Markdown 语义块或 LIVE 绘制行分组，模式切换后边界可随投影域改变。

段落索引是 View 现有 plaintext projection 的派生缓存，随文档版本、投影域和文档替换失效，未增加第二套 Document 或历史。字面黄金、CRLF/UTF8、表格内多段落/单元格分隔、代码块、模式切换/重载、EOF和极限计数通过。四配置各覆盖缓存分配失败的原子性、重试、重复查询无分配和零泄漏；两View共享文档的外部编辑、Undo/Redo、替换/销毁通过。1/10MiB SOURCE 两段落在开头、分隔符、下一段和 EOF 的暖缓存查询不再分配；尚未证明整段投影按视口有界、首次建立固定时延或非ASCII大文档性能。

最新集成候选：`E:\codex-builds\xge-document-paragraph-integrated-20261009-v827-01a09446`，1681来源/脚本/规则输入、180变化、109新文件、1726物理输入，主1572保护输入不变，未合入。v829独立接受16条产品命令，其中14条使用修正后的当前测试、2条是生产输入逐字节相同的两端137单元全新/零复用正式构建。初次历史夹具误用默认语义事务，三端实际运行1的日志/产物保留；仅改为正确的SOURCE事务，产品及黄金断言不变。Linux原View的ASan/UBSan/泄漏、两端完整renderer、Windows两个基础UIA客户端及原Windows永久bat通过，五份匹配运行依赖接入。原正式DLL的段落负例编译0/运行1，错误码记录器首次预设数值错误也保留。详见 [段落范围](XUI_DOCUMENT_PARAGRAPH_NAVIGATION.md)。

FORMAT/PAGE、Prepare generation范围、属性/可见文本范围/子对象、完整UIA TextPattern/事件/Grid/Table、重入和总体缓存规模仍未完成；本次不关闭11个工作包。

v830在确切进程句柄/镜像与创建时间观察时确认v799完整Windows仓库入口和v794原112组诊断仍运行，尚无终态。完整入口绑定此前177项冻结候选，不能作为本180项的全仓库接受；原诊断31组/111090点完成，不能代替当前产品验收。以下历史批次只对应各自输入和观察时刻。

## 此前专项：文本范围祖先与视口裁剪已通过

2026-10-09，v800—v809：通用文本范围 GetRects 现在按实际绘制规则裁到窗口视口、Document 内容区和所有 CLIP/HIDDEN 渲染祖先；VISIBLE overflow 不增加裁剪。隐藏、脱离根/overlay 控件树、零视口及零容器返回零个可见矩形，不改调用方缓冲。可见但 disabled 的文字仍可查询；范围本身仍可读取隐藏/脱离文档的文字。完全不可见时跳过 renderer 几何枚举。未增加第二套文档或历史。

原正式DLL的独立负例实际编译0/运行1：父容器右边界70，返回矩形从40延伸到208。相同裁剪黄金在最新候选的富文本 VISUAL 与 Markdown VISUAL/SOURCE/LIVE 四配置全部通过；另覆盖嵌套容器、VISIBLE、窗口缩小、隐藏/恢复、脱离/重挂、滚动、零尺寸、计数和容量/缓冲尾部。初次夹具诊断 `%g` 与整型 XUI 坐标不匹配的编译失败保留，随后仅修正诊断类型转换；没有放宽断言。

最新集成候选：`E:\codex-builds\xge-document-text-clipping-integrated-20261009-v807-01a09446`，1679来源/脚本/规则输入、178变化、107新文件、1724物理输入，主1572保护输入不变，未合入。两端各137正式单元全新/零复用和五份匹配运行文件通过；v809独立接受16条实际产品命令，包含原 Linux View 的 ASan/UBSan/泄漏、两端范围与完整 renderer、Windows两个基础UIA客户端和原Windows永久bat。实际UIA截图与已查看图片逐字节相同。完整 UIA TextPattern、Prepare范围、属性/可见范围/对象、重入/缓存/规模和物理平台仍待完成；本次不关闭11个工作包。详见 [文本范围裁剪](XUI_DOCUMENT_TEXT_RANGE_CLIPPING.md)。

v799已运行完整Windows仓库原入口，保留72私有/52公开驱动及后续全部Editor；随后Linux sanitizer仍待Windows终态。它绑定当前裁剪修正之前的177项冻结候选 v798，不能冒充178项的完整仓库接受。v810实际进程句柄/镜像和子进程创建时间确认仍运行，尚无完整CST终态。该入口前部统一Core、依赖和边界专项已输出通过；不是完整仓库通过。

旧归档的 v794 原112组诊断继续运行；v810真实句柄/镜像确认仍存活，18组/57606点完成，尚无完整终态。旧v751中断/缺少退出码以及v610真实分配失败的证据保留，不据此宣称更早失败根因闭环。下方旧批次只对应各自候选和观察时刻；当前状态以本节及冻结证据为准。

## 此前专项：LINE 已接线，完整重构仍未完成

2026-10-09，v783—v790：统一文本范围新增 LINE 的扩展、范围移动和端点移动，富文本 VISUAL 与 Markdown VISUAL/SOURCE/LIVE 四配置通过。永久字面黄金覆盖换行、空段落、终端 HardBreak、并排单元格、CRLF、LIVE 活动边界、非活动标题/隐藏空白、代码围栏及仅有隐藏定义的文档。renderer 的 1/10MiB ASCII 单行在开头、中间和 EOF 查询时，新增塑形字节各不超过16KiB；这不代表通用范围的整段投影内存、非 ASCII 行或固定机器性能已验收。

当前候选为 `E:\codex-builds\xge-document-line-navigation-integrated-20261009-v788-01a09446`：1678 个来源/脚本/规则输入、177 项变化、106 个新文件、1723 个物理输入；主工作区1572个保护输入不变，尚未合入。Windows/Linux 各137正式单元全新编译、零复用，五份匹配运行依赖已接入。v790 独立接受20条实际产品命令，包含两端 renderer/四配置范围、两端公开 Core、Linux 原 View 的 ASan/UBSan/泄漏、Windows 两个实际基础 UIA 客户端和原永久 bat；这不是完整仓库、完整 Editor 或 UIA TextPattern 验收。

仍待完成：FORMAT/PARAGRAPH/PAGE、未发布 Prepare 的范围映射、属性/可见范围/子对象和完整 UIA TextPattern、其余 LIVE 隐藏语法组合、回调/祖先剪裁/缓存规模、最新候选全仓库及完整 Editor、物理平台和最终迁移。所有11个工作包仍有实现或验收余项。专项详见 [LINE 范围方案与证据](XUI_DOCUMENT_LINE_NAVIGATION.md)；接受记录为 `build/source-document-footnote-local-reference-20261006/document-line-navigation-v790-paired-acceptance.json`。

来源索引状态更正：v751 日志停在94个完成组/309226个故障点；v793 实际句柄查询发现两个 PID 已分别被 Edge/WSL 复用，没有原运行退出码，不能再记为存活或接受通过。旧记录未改写。同一原二进制/归档、完整112组顺序和全部断言已在 v794 重新受限运行；v795 观察时两个进程镜像仍匹配且存活，7组/20231点完成，尚无完整终态。512MiB进程/768MiB作业及6GiB提交/3GiB物理启动门槛不变。这是旧失败诊断复测，不替代当前177项候选的全仓库接受。

以下旧批次与“仍在运行”描述仅对应其记录时刻；当前状态以本节及上述冻结证据为准。

## 历史：视觉行查询 v742—v758 与旧失败终态

2026-10-09 视觉行查询 v742—v758：新增公开 renderer GetLineRange，片段保留真实行边界；共用光标字素/换行亲和性判断，混合字号、显式换行、并排/嵌套表格、Bidi、空行、SOURCE CRLF、LIVE 活动/非活动内容与 1/10MiB ASCII 行开头查询的独立端点黄金通过。两端各137全新正式单元/零复用，完整renderer、原四配置范围与导航、Linux原View脚本ASan/UBSan/泄漏、两个Windows实际UIA客户端及原永久bat通过；v758独立接受16条实际产品命令。初次两处夹具初始化/绑定顺序错误和控制器输出编码错误保留；没有降低产品实现、门槛或断言。最新集成候选1676来源/174变化/104新文件、1721物理输入，五份匹配运行依赖已接入，主1572保护输入不变，未合入。通用LINE扩展/移动尚未接线；FORMAT/PARAGRAPH/PAGE、Prepare范围、完整UIA、缓存/回调/物理系统及全仓库仍待完成。详见[XUI_DOCUMENT_VISUAL_LINES.md](XUI_DOCUMENT_VISUAL_LINES.md)。

原98950/10513已经结束：v745依据原状态、日志及散列确认完整Windows仓库入口和来源索引诊断均实际1，Linux后续未启动。来源索引101/112组、344736点后出现真实malloc(393264)失败与CreateSourceIndex返回-5，当时系统可用提交量5500928字节；不据此证明更早v434根因。新v751以同一二进制/归档、原112顺序和全部断言，在512MiB进程/768MiB作业限额、6GiB提交/3GiB物理启动门槛下复测并记录峰值/余量。本次两个实际进程句柄确认存活；尚无完整终态。旧“原进程仍运行”的文字仅对应各自历史观察时刻。全部11工作包及整体目标继续active。


## 实现与数据来源

最新源码候选：`E:\codex-builds\xge-document-text-navigation-integrated-20261009-v732-01a09446`；输入记录为`document-text-navigation-v732-inputs.json`。1675个源码/脚本/行尾规则输入、169项变化、103个新文件，未合入主工作区。

`src/xui/accessibility/xui_accessibility_text_internal.h`定义通用内部文本范围协议；OS桥接无需依赖Document模块。可访问性类型adapter新增`createTextRange`回调，通用访问入口负责操作保护、重入检查和查询后的Provider/内容变化检查。其余控件的adapter改用指定字段初始化；所有正式单元均已重新编译。

`src/xui/document/xui_document_accessible_text.inl`在Document编辑adapter中实现范围。使用现有plaintext projection读取内容，端点为64位UTF-8字节偏移；实际保存的是Document结构/来源位置。范围并不拥有第二份Document或第二套历史。

| 能力 | 当前行为 |
| --- | --- |
| 创建范围 | 全文、当前选区、光标、语义子节点、世界坐标命中点 |
| 内容与端点 | 端点查询/修改、克隆、文本尺寸查询与复制；拒绝UTF-8续字节边界；容量不足不改写调用方缓冲 |
| 几何 | 使用同一renderer的范围几何，转换到XUI世界坐标并裁剪到控件内容视口 |
| 交互 | 通过统一Document选区设置与滚动接口执行 |
| 事务与历史 | 已持有范围按同一个change set更新；插入和Undo/Redo后仍指向原内容 |
| 生命周期 | 同一Document重复绑定保留范围；不同Document、View或context销毁使旧范围失效 |

查询和动作属于UI线程。范围原子引用允许其他线程Retain/Release；View持有链表引用，已无客户端引用的范围在UI线程创建/变更时回收。销毁先断开owner，再释放View引用；外部保留的范围返回STALE，最终释放不访问已销毁的Widget/context。

## 导航、查找与64位Unicode

2026-10-09 文本范围导航与64位Unicode v723—v740：共享Unicode边界引擎内部统一64位偏移，现有32位Widget入口委托同一引擎；Document范围新增同View比较、端点比较/移动及交叉折叠、字素/自然词/全文扩展与移动、UTF-8字面正反查找和Unicode大小写。重叠查找、反向选区、组合字符/emoji、INT_MIN/MAX计数、范围随插入和共同Undo/Redo以及失效通过。官方766字素语料在每字节同时核对32/64位入口，虚拟INT_MAX以上、1TiB及INT64_MAX位置的有界读取通过。最新候选1675来源/169变化/103新文件，未合入。两端各137新鲜/零复用正式库在v723生产源码上通过；v732仅修正极限测试夹具中间表达式溢出并接入五份匹配运行依赖，所有生产输入与两端正式库构建逐字节相同。修正后两端正常范围四配置、Linux原View脚本ASan/UBSan/泄漏，以及原Windows Unicode bat和Linux Unicode sanitizer入口通过。v740独立接受30条实际产品命令，其中18条直接针对修正候选、12条沿用生产和测试输入相同的前批命令；两条旧夹具grapheme检查明确排除。原sanitizer夹具失败、预备脚本失败和一次未启动编译的3GiB资源拒绝保留；门槛、产品实现和断言未降低。格式/视觉行/段落/页、属性/可见范围/完整剪裁/缓存与回调矩阵、Prepare generation映射、实际UIA TextPattern、真实系统和全仓库仍待完成。原98950/10513四个已知进程本次实际存活，索引100/112组、339948点通过，原失败原因仍未解释。11工作包及整体目标继续active。详见[XUI_DOCUMENT_TEXT_RANGES.md](XUI_DOCUMENT_TEXT_RANGES.md)。

范围协议提供compare、compareEndpoints、setEndpoint、findText、expand、move、moveEndpoint。比较只接受同View/provider的范围；端点按起止排序，跨越另一端时折叠。查找在原范围内返回新的跟踪范围，反向查找保留重叠命中，Unicode忽略大小写沿用Document FindEx的regex引擎。未知或尚未实现单位原子返回UNSUPPORTED/INVALID_ARGUMENT，失败不修改端点。字素单位仍不是已经完成的UIA Character控制字符策略。

## 前批基础范围实际验证

- Windows和Linux默认运行库分别137单元全新编译，零对象复用；同一永久C测试在富文本VISUAL及Markdown VISUAL/SOURCE/LIVE四配置均通过。
- 覆盖中文、emoji、组合字符、UTF-8非法边界/反向端点拒绝、克隆、选区/光标、子范围、命中/裁剪几何、滚动、事务和共同Undo/Redo、模式切换、独立多视图、替换/销毁及跨线程释放。
- Linux沿原`build_document_view_sanitizer.sh`参数编译同一永久范围测试；ASan、UBSan及泄漏检查实际0。独立包装只为原无窗口PNG路径提供失败即abort的两个已有stub，不替换实际文本/范围/Document实现。该检查不证明图片编码或物理平台后端。
- Windows两个既有系统UIA客户端在新正式库通过；真实XGE普通输入泵自动接入保持通过，PNG与此前已实际查看图片逐字节一致。
- 五份canonical运行依赖已接入候选，原`build_document_accessible_text_test.bat`实际编译/运行0；独立复核实际新导入符号、可执行文件、源码、日志、两端库和Linux ext4/单链接。

证据：`build/source-document-footnote-local-reference-20261006/document-text-ranges-v717-paired-acceptance.json`和`document-text-ranges-v720-integrated-acceptance.json`。当前候选累计12条实际命令接受；不能叠加此前不同源码的50条而称其全部针对当前候选。

构建仍保留已有非范围模块警告：Linux默认库的未使用变量、原View sanitizer的DatePicker maybe-uninitialized。新增测试按原严格参数通过，没有取消范围断言。

## 下一步与未完成范围

1. ITextProvider/ITextProvider2/ITextRangeProvider的实际UIA接线、UTF-16长度/查找、完整COM行为与系统客户端测试。
2. 格式、页单位及UIA Character控制字符策略；属性和最近包围元素/子对象。字素/词/视觉行/段落/全文、范围比较、端点与查找已通过专项，具体边界与覆盖范围见各记录。
3. 可见范围、祖先/宿主完整剪裁、大文档几何按需查询、Provider/范围缓存预算及回调销毁/重入矩阵。目前范围几何仍可能精排整个所选范围，不能声称完整大文档性能已验收。
4. Prepare未发布输入与generation的范围维护。目前明确返回UNSUPPORTED，不能将已提交书签直接夹进候选字节。需要同时覆盖继续输入、发布、取消与失败恢复。
5. 文本/选区/属性事件、Grid/Table及真实读屏、物理IME、DPI/GPU和固定机器性能。
6. 最新候选全仓库/Editor回归、索引失败闭环、实际合入及合入后验证。

原完整112组来源索引诊断和完整仓库入口继续各原会话；专项通过不覆盖原失败。全部11工作包与整体目标继续active。通用WebView仍只要求Windows基础功能，其他四平台与公开通信暂缓，Document内部渲染保留。
