# Document 视觉行范围查询

更新：2026-10-09。renderer 的视觉行内容查询已实现并通过本节限定验收；通用可访问文本范围的 LINE 扩展/移动与 UIA TextPattern 接线仍待完成。

## 当前接续：文本范围祖先与视口裁剪已通过

2026-10-09，v800—v809：通用文本范围 GetRects 现在按实际绘制规则裁到窗口视口、Document 内容区和所有 CLIP/HIDDEN 渲染祖先；VISIBLE overflow 不增加裁剪。隐藏、脱离根/overlay 控件树、零视口及零容器返回零个可见矩形，不改调用方缓冲。可见但 disabled 的文字仍可查询；范围本身仍可读取隐藏/脱离文档的文字。完全不可见时跳过 renderer 几何枚举。未增加第二套文档或历史。

原正式DLL的独立负例实际编译0/运行1：父容器右边界70，返回矩形从40延伸到208。相同裁剪黄金在最新候选的富文本 VISUAL 与 Markdown VISUAL/SOURCE/LIVE 四配置全部通过；另覆盖嵌套容器、VISIBLE、窗口缩小、隐藏/恢复、脱离/重挂、滚动、零尺寸、计数和容量/缓冲尾部。初次夹具诊断 `%g` 与整型 XUI 坐标不匹配的编译失败保留，随后仅修正诊断类型转换；没有放宽断言。

最新集成候选：`E:\codex-builds\xge-document-text-clipping-integrated-20261009-v807-01a09446`，1679来源/脚本/规则输入、178变化、107新文件、1724物理输入，主1572保护输入不变，未合入。两端各137正式单元全新/零复用和五份匹配运行文件通过；v809独立接受16条实际产品命令，包含原 Linux View 的 ASan/UBSan/泄漏、两端范围与完整 renderer、Windows两个基础UIA客户端和原Windows永久bat。实际UIA截图与已查看图片逐字节相同。完整 UIA TextPattern、Prepare范围、属性/可见范围/对象、重入/缓存/规模和物理平台仍待完成；本次不关闭11个工作包。详见 [文本范围裁剪](XUI_DOCUMENT_TEXT_RANGE_CLIPPING.md)。

v799已运行完整Windows仓库原入口，保留72私有/52公开驱动及后续全部Editor；随后Linux sanitizer仍待Windows终态。它绑定当前裁剪修正之前的177项冻结候选 v798，不能冒充178项的完整仓库接受。v810实际进程句柄/镜像和子进程创建时间确认仍运行，尚无完整CST终态。该入口前部统一Core、依赖和边界专项已输出通过；不是完整仓库通过。

旧归档的 v794 原112组诊断继续运行；v810真实句柄/镜像确认仍存活，18组/57606点完成，尚无完整终态。旧v751中断/缺少退出码以及v610真实分配失败的证据保留，不据此宣称更早失败根因闭环。下方旧批次只对应各自候选和观察时刻；当前状态以本节及冻结证据为准。

## 当前状态：LINE 已接线，完整重构仍未完成

2026-10-09，v783—v790：统一文本范围新增 LINE 的扩展、范围移动和端点移动，富文本 VISUAL 与 Markdown VISUAL/SOURCE/LIVE 四配置通过。永久字面黄金覆盖换行、空段落、终端 HardBreak、并排单元格、CRLF、LIVE 活动边界、非活动标题/隐藏空白、代码围栏及仅有隐藏定义的文档。renderer 的 1/10MiB ASCII 单行在开头、中间和 EOF 查询时，新增塑形字节各不超过16KiB；这不代表通用范围的整段投影内存、非 ASCII 行或固定机器性能已验收。

当前候选为 `E:\codex-builds\xge-document-line-navigation-integrated-20261009-v788-01a09446`：1678 个来源/脚本/规则输入、177 项变化、106 个新文件、1723 个物理输入；主工作区1572个保护输入不变，尚未合入。Windows/Linux 各137正式单元全新编译、零复用，五份匹配运行依赖已接入。v790 独立接受20条实际产品命令，包含两端 renderer/四配置范围、两端公开 Core、Linux 原 View 的 ASan/UBSan/泄漏、Windows 两个实际基础 UIA 客户端和原永久 bat；这不是完整仓库、完整 Editor 或 UIA TextPattern 验收。

仍待完成：FORMAT/PARAGRAPH/PAGE、未发布 Prepare 的范围映射、属性/可见范围/子对象和完整 UIA TextPattern、其余 LIVE 隐藏语法组合、回调/祖先剪裁/缓存规模、最新候选全仓库及完整 Editor、物理平台和最终迁移。所有11个工作包仍有实现或验收余项。专项详见 [LINE 范围方案与证据](XUI_DOCUMENT_LINE_NAVIGATION.md)；接受记录为 `build/source-document-footnote-local-reference-20261006/document-line-navigation-v790-paired-acceptance.json`。

来源索引状态更正：v751 日志停在94个完成组/309226个故障点；v793 实际句柄查询发现两个 PID 已分别被 Edge/WSL 复用，没有原运行退出码，不能再记为存活或接受通过。旧记录未改写。同一原二进制/归档、完整112组顺序和全部断言已在 v794 重新受限运行；v795 观察时两个进程镜像仍匹配且存活，7组/20231点完成，尚无完整终态。512MiB进程/768MiB作业及6GiB提交/3GiB物理启动门槛不变。这是旧失败诊断复测，不替代当前177项候选的全仓库接受。

以下旧批次与“仍在运行”描述仅对应其记录时刻；当前状态以本节及上述冻结证据为准。

## 接口与统一数据来源

新增 `xuiDocumentRendererGetLineRange(renderer, position, range)`。它读取同一个 Document snapshot、结构位置和 renderer 排版结果，不建立另一份内容、位置索引或历史。

排版片段保留本行的逻辑片段结束索引。查询从同一光标位置取得所属行，使用与 GetCaretRect 共用的字素和换行亲和性判断，再返回该行的逻辑内容端点。行归属不依赖 glyph 高度重叠，因此字号不同、Bidi 视觉重排和并排/嵌套表格单元格不会被合成一行。独立对象和空单元格也保留自己的片段边界。

| 情况 | 范围语义 |
| --- | --- |
| 软换行 | BEFORE 属于前一行，AFTER 属于后一行，与实际光标一致 |
| 混合字号/样式 | 使用真实排版行；返回逻辑顺序端点，不使用屏幕左右顺序 |
| 显式 HardBreak | 范围包含 break atom；终端空行可返回折叠范围 |
| 表格 | 每个单元格的行独立，嵌套表格和空单元格保留自己的范围 |
| SOURCE | 返回完整物理源码行，包含原 LF/CR/CRLF 终止符 |
| LIVE | 活动源码行返回源码行；非活动语义内容映射回 LIVE 的来源位置 |
| 长 ASCII code 行 | 已知单行边界可包含尚未塑形的后缀 |
| 无效/过期位置 | 返回错误，调用方 range 输出不变 |

该接口返回实际行内容，**不附加 plaintext projection 合成的段落/单元格分隔符**。后续通用文本单位需要明确处理这些分隔符，不能直接以 GetLineRange 的内容端点冒充完整 UIA Line 单位。

## 当前源码与实际验证

集成候选：`E:\codex-builds\xge-document-visual-lines-integrated-20261009-v756-01a09446`。1676 个来源/脚本/规则输入、174 项变更、104 个新文件、1721 个物理输入；主工作区 1572 个保护输入保持不变。源码和五份匹配运行依赖均在独立候选，尚未合入主工作区。

验收记录：`build/source-document-footnote-local-reference-20261006/document-visual-lines-v758-paired-acceptance.json`。

- Windows/Linux 正式默认库各 137 个单元全新编译、零复用；实际公开接口消费者和完整 renderer 回归均通过。
- 独立逻辑端点黄金覆盖宽度重排、换行前后亲和性、混合字号、HardBreak/终端空行、并排表格、嵌套单元格、Hebrew Bidi、空文档、SOURCE CRLF 和 LIVE 活动/非活动内容。
- 1/10 MiB ASCII 单行 code 在开头位置查询时返回完整行边界；SetSnapshot/Layout/查询期间新增塑形字节不超过 16 KiB。这是该路径的塑形检查，**不代表任意远端位置、非 ASCII 行、RSS 或固定机器 P95 已验收**。
- 既有富文本 VISUAL 与 Markdown VISUAL/SOURCE/LIVE 四配置的跟踪范围、导航、事务/共同历史和失效测试保留并通过。
- Linux 沿原 `build_document_view_sanitizer.sh` 参数运行实际永久测试，ASan/UBSan/泄漏检查通过。仅使用原 View 测试已有的两个失败即 abort 的 PNG stub，没有替换 Document/文本/renderer 实现。
- Windows 两个实际系统 UIA 客户端通过；真实 XGE GL/自动输入泵接线截图与此前已查看图片逐字节相同。该检查仍不证明 UIA TextPattern 或物理读屏完成。
- 五份 canonical 运行依赖接入后，原 `build_document_accessible_text_test.bat` 实际编译/运行通过；新可执行文件确实导入新行查询接口，旁边没有旧 DLL 覆盖。

独立复核共接受 16 条实际产品命令：Windows 当前消费者 8 条、生产输入相同的前批正式构建 1 条、Linux 新正式库与消费者 5 条、原 sanitizer 1 条、原 Windows bat 1 条。Linux 两份来源树、日志和 ELF 在实际 ext4、单链接下核对；两端来源/运行依赖/产物散列、编译单元列表、真实导入与 sanitizer 符号均复核。

初次测试夹具漏填 stats.iSize、随后夹具未先绑定 Markdown snapshot 就切换模式的失败保留在 v744/v750。还保留 v744 控制器最终打印的控制台编码错误。修正仅涉及夹具初始化/绑定顺序和后续工具输出；生产代码、门槛和断言没有为通过测试而降低。Windows 正式库与最终候选所有生产输入逐字节相同。

## 后续工作

1. 通用范围 LINE 已通过上述永久黄金；其余 LIVE 隐藏语法、复杂容器/对象、回调和故障组合仍须扩展验收。
2. 显式 Markdown VISUAL、Prepare generation、继续输入/发布/取消、对象与控制字符的新增行查询矩阵。
3. 任意远端长行查询、复杂样式/缓存失效、完整祖先剪裁、回调重入/销毁与规模预算。
4. FORMAT/PARAGRAPH/PAGE、属性/可见范围/子对象、完整 UIA TextPattern/事件与真实读屏。
5. 最新候选完整仓库/Editor 回归、来源索引失败闭环、实际合入与合入后验证。

本专项不关闭 R1/E3/M1 或整体 11 个工作包。

## 来源索引与完整仓库终态更正

v745 已依据原日志散列和控制器终态确认：原 v610 来源索引诊断编译 0、运行 1；101/112 组、344736 点通过后，在 point=3448 的 failed-command-snapshot 阶段出现真实 malloc(393264) 失败，CreateSourceIndex 返回 XUI_ERROR_OUT_OF_MEMORY(-5)，当时系统可用提交量 5500928 字节。该次失败具有直接真实分配失败证据；**不据此宣称更早 v434 失败的根因已经证明**。

原 v647 完整 Windows 仓库入口亦实际 1，停在来源索引断言；Linux 后续阶段未启动。旧 98950/10513 进程已经结束，下方其他历史文档中的“仍运行”仅对应各自观察时刻。失败记录：`document-terminal-failures-v745-observation.json`。

新 v751 使用字节相同的原 context.exe、原归档、原 112 组顺序及全部断言复测；Windows 进程 512 MiB/作业 768 MiB 限额，启动要求 6 GiB 可用提交量/3 GiB 物理余量，额外记录实际作业峰值和系统余量。此复测不覆盖旧失败记录，尚未取得完整终态，不视为内核正确性通过。跟踪：`document-source-context-bounded-v751-current.json`。
