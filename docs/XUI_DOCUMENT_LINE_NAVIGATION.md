# Document 统一文本范围的 LINE 操作

更新：2026-10-09。LINE 的基础扩展与移动已通过两端专项；完整可访问性和11个重构工作包继续进行。

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

## 统一内核与行为

范围仍保存同一 Document 的结构/来源位置，使用同一事务、change set 和 Undo/Redo。没有另建 Markdown 文档或兼容旧 Rich API。通用范围已支持 GRAPHEME、WORD、LINE、PARAGRAPH、ALL；FORMAT、PAGE 尚未完成。

LINE 使用 renderer 的真实视觉行边界。SOURCE 返回完整物理源码行并归属原行终止符；VISUAL 使用排版行；LIVE 活动区使用源码行，非活动区使用语义行并映射到所属根块的源码分区。分区复用 renderer 现有块边界，隐藏空白、围栏和定义不会通过相邻块猜测归属。

公开 GetLineRange 只返回实际行内容。通用 LINE 单位额外处理 plaintext projection 合成的段落 LF/单元格 TAB，终端 HardBreak 后的空行独立归属。EOF 扩展最后一个有文本单位的行；零长度全文范围不产生虚构文本。范围移动保留现有规范化与端点交叉折叠约定。布局/查询失败时原范围保持不变，实际移动计数为零。

长 ASCII 未换行源码/code 行优先查询行元数据，避免为了得到远端行边界先塑形整个水平前缀。来源映射读取 provenance-aware 范围；不使用可能来自旧 store 的裸节点 source 字段。

## 永久独立黄金

`test_xui/xui_document_text_line_cases.h` 对每个合法 UTF-8 标量位置检查同一个字面行分区，并验证正反移动、EOF、零计数、INT_MIN/MAX、端点交叉折叠。以下偏移均为该 View 的 UTF-8 plaintext projection 字节偏移。

| 内容 | 预期行分区端点 |
| --- | --- |
| Rich 换行与空段落 `abcdef\n\nxy\n` | 0, 3, 7, 8, 11 |
| Rich 终端 HardBreak `abc\nde\n\n` | 0, 4, 7, 8 |
| 并排单元格 `left\tright\n` | 0, 5, 11 |
| MD SOURCE `# head\r\n\r\nabcdef\r\n` | 0, 8, 10, 18 |
| 同文档 VISUAL `head\nabcdef\n` | 0, 5, 12 |
| LIVE 活动首块 | 0, 8, 10, 18 |
| LIVE 活动后块，前标题与空白非活动 | 0, 10, 18 |
| LIVE 标题、围栏、两条代码行、尾部空行与段落 | 0, 4, 5, 15, 18, 23, 28 |
| LIVE 仅隐藏定义 `[id]: /target\n` | 0, 14 |

`test_xui/xui_document_visual_line_cases.h` 保留真实 renderer 的混合字体、Bidi、表格、空行和模式端点黄金，并对1/10MiB ASCII 单行检查开头、中间、EOF，聚合新增塑形不超过16KiB。通用范围仍可能投影整个选区；这个优化不构成范围内存/RSS/P95预算的证明。

## 当前候选与证据

候选：`E:\codex-builds\xge-document-line-navigation-integrated-20261009-v788-01a09446`。1678来源、177变化、106新文件、1723物理输入，主1572保护输入不变；产品尚未合入主源码。

冻结接受：`build/source-document-footnote-local-reference-20261006/document-line-navigation-v790-paired-acceptance.json`。

- Windows/Linux 各137单元全新正式构建、零对象复用，五份匹配运行依赖已接入。
- 独立复核20条实际产品命令：Windows正式库与四配置范围/renderer/两个基础UIA客户端9条，Linux正式库与范围/renderer5条，原Linux View sanitizer1条，两端公开Core各2条，原Windows永久bat1条。
- 原Linux sanitizer保留ASan、UBSan和泄漏参数，仅使用原View入口的两个失败即abort的PNG stub。实际ELF/导入/导出、日志、ext4单链接和源码散列已核对。
- 早期 LIVE 活动边界、非活动标题空白和仅定义源码文档的真实失败保留，对应永久黄金在最终候选通过。一次控制器准备顺序失败也保留，未启动产品。

## 仍需完成

1. Prepare 输入/generation、预览/取消/发布、复杂 LIVE 行内/表格/脚注隐藏语法和对象组合。
2. FORMAT/PAGE、属性/可见范围/子对象、剪裁重入/销毁与总体缓存规模；基础祖先/窗口裁剪和段落单位已通过专项。
3. Windows UIA TextPattern/TextRangeProvider 的实际接线、UTF-16映射、事件、Grid/Table及真实读屏。
4. 非 ASCII 长行、通用范围投影内存与固定机器性能，真实字体/IME/DPI/GPU。
5. 最新候选完整仓库与Editor、旧来源索引失败闭环、最终源码/示例/依赖迁移和合入后验证。

本专项通过不能关闭 E3/R1/M1 或整体重构。
