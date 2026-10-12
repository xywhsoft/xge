# Document 的 Windows UI Automation 接入

更新：2026-10-09。基础节点、动作、Document 更换、实际 XGE 自动接入及静态构建迁移已通过。完整文本范围和真实读屏验收尚未完成；E3、P1、H1、V1 及整体重构目标仍为 active。

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


## 当前实现

最新候选为 `E:\codex-builds\xge-document-text-navigation-integrated-20261009-v732-01a09446`（v732输入记录），1675 个源码/脚本/行尾规则输入、169 项变化、103 个新文件，尚未合入主源码。统一 Document 模型、事务、快照、三模式和共同历史保留，没有旧 API 兼容包装。此前109项暂存属于旧候选。

`src/xui/accessibility/xui_native_accessibility.c` 提供 HWND 查询入口、Simple/Fragment/FragmentRoot 节点树、名称、角色、边界、焦点及 Value/Invoke/Toggle；应用可访问性回调独立保留。UIA 客户端在 MTA 查询，Provider 在所属 UI STA 执行。Windows SDK 类型未放进通用公开头。

通用桥接通过 XUI 可访问性协议查询，不再依赖 Document 模块。Widget/context 销毁会断开旧 Provider；Document 更换也显式断开，避免不同文档复用语义 ID 时旧节点指向新内容。同一 Document 的重复设置保持身份。Provider 和客户端各自持有引用，运行 ID 不复用。

普通 `xuiProxyXgePumpInput`/`xuiProxyXgePumpInputRect` 使用输入视口变换自动接入。非 Windows 保持 unsupported 空实现；Linux 空实现已通过八个原始静态 sanitizer 入口，不能据此声称 Linux 读屏后端已实现。通用 WebView 仍只要求 Windows 基础功能。

## 最新文本范围后端

2026-10-09 文本范围导航与64位Unicode v723—v740：共享Unicode边界引擎内部统一64位偏移，现有32位Widget入口委托同一引擎；Document范围新增同View比较、端点比较/移动及交叉折叠、字素/自然词/全文扩展与移动、UTF-8字面正反查找和Unicode大小写。重叠查找、反向选区、组合字符/emoji、INT_MIN/MAX计数、范围随插入和共同Undo/Redo以及失效通过。官方766字素语料在每字节同时核对32/64位入口，虚拟INT_MAX以上、1TiB及INT64_MAX位置的有界读取通过。最新候选1675来源/169变化/103新文件，未合入。两端各137新鲜/零复用正式库在v723生产源码上通过；v732仅修正极限测试夹具中间表达式溢出并接入五份匹配运行依赖，所有生产输入与两端正式库构建逐字节相同。修正后两端正常范围四配置、Linux原View脚本ASan/UBSan/泄漏，以及原Windows Unicode bat和Linux Unicode sanitizer入口通过。v740独立接受30条实际产品命令，其中18条直接针对修正候选、12条沿用生产和测试输入相同的前批命令；两条旧夹具grapheme检查明确排除。原sanitizer夹具失败、预备脚本失败和一次未启动编译的3GiB资源拒绝保留；门槛、产品实现和断言未降低。格式/视觉行/段落/页、属性/可见范围/完整剪裁/缓存与回调矩阵、Prepare generation映射、实际UIA TextPattern、真实系统和全仓库仍待完成。原98950/10513四个已知进程本次实际存活，索引100/112组、339948点通过，原失败原因仍未解释。11工作包及整体目标继续active。详见[XUI_DOCUMENT_TEXT_RANGES.md](XUI_DOCUMENT_TEXT_RANGES.md)。

## 前批桥接实际验证

- v686 集成库：6 个新单元、131 个正式对象复用，7 条构建命令实际0；作为原始批处理的匹配库。
- v697 两个永久系统客户端：编译/运行4条实际0。测试代理客户端覆盖语义节点、中文、emoji、Toggle/Undo/Invoke、共同历史、应用回调、Document更换及Widget/context销毁。
- 实际XGE客户端使用正常输入泵，没有显式桥接绑定；覆盖富文本/Markdown角色和原生边界、UTF-16替换、Undo及LIVE。实际字体为Segoe UI/Consolas，GL为AMD Radeon RX5700；RGBA中两列分别4155/5345墨色像素，PNG实际查看。
- v688/v703：Windows21次原始批处理入口实际0，包括五种基础样式参数、静态文本/拖放/弹窗/容器/表格/终端，以及两个永久UIA入口。聚合源码表也由实际编译覆盖。
- v701：Linux8个原始脚本实际0，ASan/UBSan和泄漏选项保持；可访问性、文本编辑、拖放、弹窗及Document渲染通过。1715物理文件和8份日志/产物在实际ext4复核，单链接。迁移脚本统一LF，并补`tests/xui/.gitattributes`，解决直接`sh`入口的CRLF退出。
- v704：Windows默认库137单元全新编译、零复用，两个永久UIA客户端在该库上通过。正式库与原始批处理PNG均与已实际查看的图逐字节一致。
- v705独立复核上述45条实际命令、源文件、运行库、复用对象、日志及产物，全部24迁移入口的依赖覆盖成立。保留各入口的行为/规模/样式覆盖；这不是完整仓库或发布矩阵验收。
- v706/v708/v709/v710：Linux默认库也137全新/零复用，公开Core初次缺少测试目录的实际1保留；新输出根补目录后，用同一库和同一可执行文件通过原断言。Windows同一公开Core编译/运行也通过。v710独立复核两端137/零复用、实际报告与完整编译列表、库/消费者/日志、ext4/源码，累计接受50条实际产品命令。

接受记录：`build/source-document-footnote-local-reference-20261006/document-native-accessibility-v705-paired-acceptance.json`及`document-native-accessibility-v710-paired-acceptance.json`。

旧状态getter在系统客户端返回成功的诊断保留于v687/v689；永久测试分别验证旧节点失效与旧Toggle/Invoke动作不可用，并检查替换文档的任务和调用计数未改变。v691准备锚点错误、v693严格缩进诊断、v698 drvfs读取错误及v699脚本CRLF失败均保留。后继修正没有取消旧动作、内容或身份断言。

## 尚未完成

1. ITextProvider/ITextProvider2/ITextRangeProvider：全文/可见范围、选区、光标、词/行导航、范围几何、编辑后位置、滚入视口及UTF-8/UTF-16映射。
2. Grid/Table模式、标题级别、对象子范围、完整属性/文本/选区事件和大文档Provider缓存预算。
3. Widget与语义子节点混合导航、真实系统焦点、视口剪裁、密码节点及复杂更换/回调矩阵。
4. 最新候选的完整仓库回归、全部Editor矩阵、实际合入及合入后验证仍待完成；两端137正式库和公开Core通过不替代这些范围。
5. 实际读屏软件、物理IME、DPI、主题、GPU矩阵与固定机器性能。已通过本机原生GL/UIA测试不能替代完整物理平台验收。

本次观察（2026-10-08T17:25:04.472540+00:00）确认旧完整仓库原98950和索引原10513的四个已知进程实际存活；索引前75组251286点通过，尚无112组终态，原失败原因未解释。完整11工作包和整体目标继续active。

接续观察（2026-10-08T17:34:20.637146+00:00）：上述两端正式库/公开Core已接受；旧完整98950和索引10513四个已知进程仍实际存活，索引前78组260558点通过，原失败根因仍未解释。

## 统一文本范围后端接续

2026-10-09 统一文本范围后端 v713—v721：富文本VISUAL和Markdown三模式的全文/选区/光标/子节点/命中范围、64位UTF-8端点、复制与选区/滚动已实现；范围沿同一Document change set映射插入及Undo/Redo，同一Document保持、替换/销毁失效、跨线程Retain/Release和UI线程回收通过。Windows/Linux各137单元全新、零复用，四配置范围永久测试两端通过；Linux原View脚本ASan/UBSan/泄漏参数及断言保持，新范围测试通过。Windows原系统UIA与真实XGE自动接入保持通过，PNG与此前已查看图相同。v717独立核对11条实际命令，v718接入五份匹配运行依赖，原永久bat也实际0；v720独立接受累计12条当前候选命令。最新1674来源/165变化/102新文件，源码在独立候选，未合入。该后端尚未接线UIA TextPattern；词/行/格式单位、属性/子对象/可见范围、未发布输入映射、回调/完整剪裁和缓存规模等仍待完成。原完整98950和索引10513四个已知进程本次仍实际存活；索引94/112组、309226点通过，原失败原因未解释。11工作包与整体目标仍active。详见[XUI_DOCUMENT_TEXT_RANGES.md](XUI_DOCUMENT_TEXT_RANGES.md)。

基础桥接公开模式仍为Value/Invoke/Toggle。新的通用文本范围协议和Document实现已经通过上述范围验证，尚未向UIA客户端公布TextPattern，不能据此声称完整读屏可用。未发布Prepare输入明确返回UNSUPPORTED，其映射、文本单位和属性等仍在实现范围内。
