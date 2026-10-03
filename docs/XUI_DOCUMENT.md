# 统一 Document 实施与接入

日期：2026-10-02。当前代理 ABI 为 15，Document 原生格式为 7；系统仍在实施，尚未满足整体重构方案的最终发布验收。下文较早批次的版本与限制按该批次记录，新增能力以文末最新记录为准。

本次已经建立可运行的 C 实现：富文本与 Markdown 共用 Document、快照、根事务、历史、结构位置、表格节点、Renderer 和原生编辑控件。新 API 是独立设计，不通过旧 RichDocument/RichEdit 转调。旧实现、公开声明和独立旧测试已退出活动仓库与 DLL；仍须逐项补齐尚未迁移的旧行为和整体方案验收。

## 源码与依赖

| 入口 | 内容 |
| --- | --- |
| `xui_document.h` | 不依赖窗口与 GPU 的文档 API；由 `xui.h` 包含 |
| `xui_document_ui.h` | 共享 Renderer、DocumentView、DocumentEditor；需要显式包含 |
| `src/xui_document_store.c` | 持久化 treap、radix 节点索引、引用计数、分配诊断 |
| `src/xui_document_memory.c` | 共享存储的增量内存归属、历史预算、快照占用诊断 |
| `src/xui_document.c` | 事务、发布、通知、历史、节点操作 |
| `src/xui_document_schema.c` | 节点/属性/表格约束与反序列化校验 |
| `src/xui_document_position.c` | 结构位置、变更映射、源码映射 |
| `src/xge_unicode_grapheme.h`、`src/xge_unicode_grapheme_data.inc` | 编辑导航、布局、SFNT 回退与 GDEF 共用的 Unicode 17 字素规则/属性 |
| `src/xui_document_shape_context.inl`、`src/xui_text_display.h` | Document 保留的段落显示上下文、可选来源映射、脚本缓存及共用显示过滤 |
| `src/xge_unicode_script.h`、`src/xge_unicode_script_data.inc` | 原生 SFNT 整形与词段回退共用的 Unicode 17 Script_Extensions 属性/解析 |
| `src/xge_text_context.h` | 原生整形片段/完整段落契约、无分配 BCP 47 语法校验 |
| `src/xui_document_source.c` | 持久化文本来源片段、实体/转义/换行映射与片段读取 |
| `src/xui_document_commands.c`、`src/xui_document_table.c` | 范围替换、段落操作、根树内的表格操作 |
| `src/xui_document_markdown*.c`、`src/xui_document_reconcile.c` | 解析适配、源码回写与重解析后的身份匹配 |
| `src/xui_document_incremental.c` | 独立段落/标题的局部解析、节点复用和来源元数据更新 |
| `src/xui_document_search.c` | 与可见控件无关的文本投影、字面/正则及大小写折叠查找、原子全部替换 |
| `src/xui_document_io.c`、`src/xui_document_file.c` | 原生 JSON、原子文件写入、打开与保存点 |
| `src/xui_document_html.c` | 转义 HTML 导出；原始 HTML 不执行 |
| `src/xui_document_layout.c`、`src/xui_document_renderer.c` | 共享布局、表格递归、绘制、命中与光标 |
| `src/xui_document_view.c`、`src/xui_document_editor.c` | 原生显示、选择、输入、IME 投影与编辑命令 |
| `src/xui_document_edit_adapter.c` | XUI 通用编辑协议、基础文本辅助技术接口 |
| `examples/xui_document` | 富文本编辑、MD 源码编辑与同文档预览示例 |

`xui_document_sources.bat` 是核心唯一源清单，`xui_sources.bat` 将核心和 UI 模块加入 Windows 主 DLL。独立 core 测试不链接窗口或 Renderer。

Markdown 使用 MD4C 固定提交 `b3c6223903c1df483cef926ba347e531248f0b92`，采用 MIT 许可。解析器包含由宏保护的块语法/引用定义范围回调补丁，分配器包装位于 XUI 适配层；未定义这些宏时仍可独立编译。补丁范围见 `lib/md4c/README.xui.md`。这是相对于设计中“优先验证 cmark-gfm”的实际依赖选择。

标准 Windows DLL 现通过 `build_text_shaping.bat` 单独编译固定版本 HarfBuzz 14.3.1，XGE/XUI 的集成代码仍为 C，第三方对象需要 C++17 编译器。字体字节由既有 FontFace 保有，字体实例缓存不可变整形字体；同一字形结果用于测量和绘制，XUI 代理接收 Unicode 字素停点，完整有效的 GDEF 停点优先，缺失或异常时等分。单个连字伴随一个或多个零 advance 的 GDEF 附标字形时，也使用该唯一 carrier 的完整停点，加入整形后的 pen/GPOS 位移，RTL 转为前缘距离；多个 carrier、带 advance 的附标或无法完整对应字素的列表整体等分，不借用局部 GDEF。跨颜色节点的连字保持原轮廓，以同一停点分割颜色，范围装饰也使用这些停点。完整单段缓存文本保留超出逻辑 advance 的字形绘制与滚动范围，光标、换行和装饰仍使用逻辑宽度。`xge_glyph_run_t` 新增内部停点数组，调用者与 DLL 必须同步重编译。独立构建须定义 `XGE_ENABLE_HARFBUZZ` 并链接第三方对象；未启用的最小构建仍是简单字形路径，不能满足完整整形验收。版本、来源、许可及各平台 mutex 配置见 `lib/harfbuzz/README.xge.md`。

适配器默认仍按 LTR item 输出，新增 `XGE_TEXT_SHAPE_RTL` / `XUI_TEXT_SHAPE_RTL` 可对已解析方向的 item 做真实 RTL 整形。来源 cluster 始终按逻辑顺序排列，`xge_glyph_position_t.fVisualX` 另存物理绘制位置；GDEF 停点按 cluster 前缘距离表示。绘制代理的 `XUI_TEXT_RTL` 支持 DrawContext、Spans 及 Surface 三个入口，包含对齐、裁剪、颜色分段和下划线。不具备 textShape 的代理不会用独立字素测量伪装 RTL 整形。公开字形结构已改变，调用者与 DLL 必须同步重编译。

`src/xui_text_bidi.c` 已建立共享段落 UBA 桥接，固定 SheenBidi 3.0.0 / Unicode 17，以 C 编译、使用 XRT 分配器；保持逻辑 UTF-8 字节位置，逐行输出独立视觉 runs 与镜像记录。依赖来源、许可、两项本地缺陷修复及复核脚本见 `lib/sheenbidi/README.xge.md`。Windows/Linux 已通过 861,948 条完整方向语料、所有分配失败点、生命周期和 Linux ASan/UBSan；原生 XGE 另有希伯来/阿拉伯字形及实际 GPU 探针。

Document 现使用这套桥接进行段落方向解析，含非 ASCII 的显示段落缓存一次 Script_Extensions，按已解析的级别、字体和脚本建立整形组，并在断行后建立独立视觉片段索引；逻辑片段、原文与公开位置保持来源顺序。Rich、Markdown、SOURCE/LIVE 与代码文本共用方向布局、命中、双向 affinity 及选区；不可见方向控制字符保留在来源中参与分析，绘制投影删除它们。HardBreak 保持同一段落基向，代码/源码的实际段分隔按 Unicode P1 解析。基础左右箭头按视觉字素移动，Backspace/Delete 仍按逻辑字素删除；Ctrl 词导航保留原逻辑路径。默认沿用段落现有对齐属性。存在安全切点的长 RTL 段落在 continuation 中保留完整段落 UBA 分析，恢复段落级别后仅续排未提交尾行；失败恢复既有逻辑片段、视觉索引、行描述与绘制文本，缓存计费包含方向内核。没有安全切点的 joining 段落和非 ASCII 代码/源码长行仍完整整形。通用 TextLayout、纯 ASCII 跨 item 的完整上下文、显式语言 itemization、跨样式整词字体选择、多个 carrier 的复杂 cluster 映射及 device/variation 调整的实现或专项验收仍未完成。Linux 原生 XGE、Android、iOS、macOS 的字体/GPU/IME/读屏/DPI 验收也未完成，无窗口代理验证不能替代它们。

静态、未 hint 的 TrueType 字体现支持 GDEF Format 2 轮廓点停点：有界 C 读取器保留原始 on/off-curve 编号，解析复合字形的矩阵、offset、嵌套和真实轮廓点对齐；HarfBuzz 子字体仅提供轮廓点回调，其他 OT metrics/GSUB/GPOS 仍由固定上游实现提供。TrueType 轮廓绘制使用同一读取结果，修正既有 stb 复合变换并保留简单字形的中点取整；CFF 沿用原绘制路径。GDEF 列表逐项检查，轮廓点查询失败或未知格式整体回退，合法零坐标保持有效。不支持 CFF 原始轮廓点、hinting/变体轮廓、phantom-point 附着及超出深度/工作量上界的复合字形；它们不计入已完成范围。细节与独立字体/像素及故障验证见执行清单“轮廓点停点与复合 TrueType 轮廓批次”。

Document 的段落显示文本由块缓存拥有，不再借用排版后即释放的断行投影。非 ASCII item 的测量与最终 `drawText`/`drawTextSpans` 使用同一输入能力策略：支持时传完整显示上下文、精确片段范围、解析脚本和方向；缺少派生 context/script 能力时使用普通输入，显式语言和 RTL 仍须支持或拒绝。它覆盖普通段落、代码及 SOURCE/LIVE 行、表格内段落和安全切点续排。来源位置仍用原字节，SoftBreak 投影为一个空格，HardBreak 为 U+2028，对象为 U+FFFC；删除的方向控制等通过可选边界映射换算，ZWJ/ZWNJ 保留。只有实际改变偏移的段落分配来源映射，单个来源片段的纯 ASCII 可省略脚本图，多个来源片段按需缓存同一解析器的脚本图。宽度重排复用段落上下文和脚本图，字体/快照失效或淘汰时释放；SOURCE 行树目前在改宽时重建行布局。缓存计费包含这些投影、映射、脚本、选择性 SHY 变体和 GDEF caret。

具备 context/script 能力的代理接收 ASCII 跨字体/字号 item 的完整上下文；缺少派生能力时仍走普通输入，保持最小无 HB 构建的简单文本能力。不能把上下文传递或整词选字体等同于跨字体任意 GSUB 替换。无 HB 后端现在可用普通输入排版/绘制非 ASCII LTR 内容，仍不提供复杂文字整形、显式语言/脚本/上下文或 RTL；不能把逐字回退当作完整 Unicode 排版质量。选择性 SHY 在支持 context 的段落中（包括 ASCII）为测量/绘制保留插入短横线的精确上下文，但当前每个候选或保留的 SHY 组会复制全文，极长、多次断词的时间/缓存上限尚未完成。当前显示删除策略也不代表所有控制字符的整形语义已逐脚本验收。Document 自然语言属性已进入继承、事务/Undo、持久化和导入/导出，具体语义见文末；未指定语言固定使用 `und`。

SFNT 主字体带回退链时，整形器现优先选择能够覆盖整个字素或同解析脚本词段的最早字体，避免基字/附标和连字组成字符因单码点回退被拆开。名义 cmap 缺字时实际执行 HB 规范化/GSUB 探测，允许可组成字形的主字体继续使用；Default_Ignorable_Code_Point 使用固定 Unicode 17 数据，不因缺少 joiner/variation selector 的 cmap 而触发无效换字体。探测与最终整形保留传入字符串的前后上下文，测量、绘制、GPOS/GDEF 和字体存活共用最终字形结果。若没有字体完整覆盖词段则退到完整字素；若连字素也无法完整覆盖，沿用头字符的普通回退字体保留 .notdef，尚未实现多字体附标合成。失败词段边界缓存避免逐字素重复遍历整个长词。XUI 调整字号会同时重建主字体和代理已有回退实例，任何分配失败均回滚并返回错误。

这仍是有明确边界的上下文字体回退：词段使用解析后的 Script_Extensions 和字符类别启发式，不是完整 UAX 词边界或语言 itemization。SFNT 上下文子 item 已按完整字素/词段选择字体，并额外核对当前片段覆盖；Rich/Markdown 跨字体/字号专项见 [跨 item 整词回退](XUI_DOCUMENT_WORD_CONTEXT.md)。位图主字体与 SFNT/位图混合回退、跨字体任意 GSUB 替换及所有脚本的真实字体矩阵仍待完成。专门验证见执行清单“完整字素与同脚本词段字体回退批次”。

编辑导航、布局字素边界、SFNT 字体回退与 GDEF 停点现共用 `src/xge_unicode_grapheme.h` 的无分配 Unicode 17 extended-grapheme 状态机和同一生成属性表，替换编辑器手写字符范围与 libunibreak 15.1 字素实现。输入不要求连续内存，Next/Prev/Clamp 保持 UTF-8 字节坐标，处理 GB9c Indic conjunct、GB11 pictographic ZWJ 与 RI 配对；普通文字查询只读取局部上下文，特殊序列回看规则所需前缀。源码中可保留方向控制字符，既有显示投影隐藏它们的覆盖规则仍单独生效。主脚本变化也不能在一个字素内把 SFNT item 切开；该处按需建立同一边界图。官方 17.0 全部 766 条语料的每个 UTF-8 字节、全 1114112 码点属性和实际原生编辑专项均已验证。共享 UAX #14 行边界现由 libunibreak 8.0 升级到 Unicode 17.0，全部19,338条官方语料及1,114,112码点属性通过，原先LB30b已知偏差已消除；WORD应急断行与CHAR仍是显式产品策略。详见 [Unicode17行边界](XUI_DOCUMENT_LINE17.md)。语言定制、PS编辑规则、复杂脚本/字体及真实平台矩阵仍待完成。

原生 SFNT 路径现使用固定 Unicode 17 的 Script/Script_Extensions 集合交集解析，共 176 个脚本标识、283 个集合及 128 个配对括号记录；不会拆开 extended grapheme。共享标点跟随兼容脚本，括号保留外层脚本，段分隔重置上下文。括号栈最多 128 项，溢出丢弃最旧 opener；语言相关引号不按码点猜测。完整 ASCII item 省略脚本图分配；非 ASCII 或需要从周围上下文解析的 ASCII 子 item 使用每字节一个私有脚本 ID，最终整形、词段回退和 HB 覆盖探测共用这张图。Document 使用同一解析器在非 ASCII 及多个来源片段的 ASCII 显示段落缓存脚本图，再按已解析脚本切分 item；不会按首字符主 Script 重复推断。富文本及 Markdown VISUAL/SOURCE/LIVE 的跨度、命中和 GPU alpha 有专项验证；共享语言属性及 ASCII 跨 item 的脚本/上下文已接入，其他脚本/平台矩阵仍待完成。专项见执行清单“Unicode 17 Script_Extensions 原生整形批次”。
原生 `xge_text_shape_desc_t` 已增加 `sContext/iContextSize/iContextOffset`、`iScript` 和 `sLanguage`：字体覆盖探测及最终 HB 整形接收完整段落和当前片段范围，cluster/GDEF caret 仍以片段起点为零。语言按 RFC 5646 校验语法、允许 255 字节，不验证 IANA 注册；NULL 固定为 `und`，不会随进程 locale 改变。显式脚本优先于自动解析；未传脚本时在完整上下文上建立脚本图，显式脚本跳过该分配。`iTextSize=0` 现在明确表示空片段，`-1` 表示 NUL 结尾；描述符 ABI 发生变化，所有宿主和 DLL 必须同步重编译。测试及现有示例已迁移；未启用 HB 的构建对显式上下文/脚本/语言返回 unsupported。

XUI 五个测量/整形/绘制回调共用 `xui_text_item_t`，Document 按代理能力将保留的显示段落、来源映射、已解析 script 与 item 范围传入原生后端，包含 ASCII 跨字体/字号 item；跨字体任意 GSUB 替换仍待完成，SFNT 整词字体选择已使用完整上下文，同字体的颜色/装饰 span 已接入文末的共享字形范围路径。`fDrawOffsetX` 为有限的 [-0.5,0.5] 像素画笔偏移，只影响绘制，不改变测量、整形或矩形裁剪；Document 用它保留脚本 item 之间的小数 pen，原生 glyph-run 通过 `XGE_DRAW_TEXT_SUBPIXEL_X` 保留 X 起点，基线仍吸附像素。原生代理绘制不再把一个 script 边界当作额外的裁剪边界，沿用视口或颜色段裁剪；保留与旧裁剪路径相同的文本提交边界，避免仅改变额外裁剪就改变背景、字形和装饰线的合成结果。`XUI_PROXY_VERSION` 现为 16，旧版本代理拒绝，应用、自定义代理及 DLL 必须一起重编译，不保留旧请求包装。显式语言已作为共享文档属性进入继承、Schema、事务/Undo、序列化和样式失效。原生 132 组独立 HB/GDEF/字体回退及生命周期专项与平台边界见执行清单“原生段落上下文与语言契约批次”。

## 内容所有权与位置

- 一个 live Document 由一个所有者线程读写。所有者获取不可变 Snapshot 后，可以把 Snapshot 交给工作线程；不要在另一个线程获取快照的同时提交同一个 live Document。
- Document、Snapshot、ChangeSet 均有 retain/release。事务 Release 会放弃尚未提交的候选。节点信息中的字符串借用自对应快照。
- 节点 ID、长度和偏移采用 64 位。字符串及偏移使用 UTF-8 字节，事务拒绝落在编码中间的边界。字素导航由编辑器处理。
- TEXT 表示节点文本中的位置，GAP 表示容器子节点之间的位置，SOURCE 表示 Markdown 源码位置。位置带文档身份和 revision；过期位置必须经 ChangeSet 映射。
- 语义命令保留存续节点的 ID，包括跨容器移动、相同内容的节点互换、段落合并后保留的文本片段；Undo/Redo 恢复相应版本的身份和顺序。Markdown 不再用重解析后的源码偏移替代语义身份。
- 表格单元格就在根树中，不拥有第二个 Document 或 Undo 栈。跨不同 Cell scope 的不受支持替换会失败，避免意外合并单元格正文。
- Renderer 保留 Snapshot，借用 context 和传入的字体；释放 Renderer/View/Editor 后再释放这些运行时资源。View/Editor 保留绑定的 Document。

默认限制：256 MiB 文本、1,000,000 节点、128 层结构、256 步历史且历史额外存储不超过 64 MiB；表格最多 1024 列，单元格行/列跨度最多 1024。节点字体尺寸、对象宽高与段后距均为 0–1,000,000 的有限逻辑单位，写入和原生文件读取使用同一边界。默认限制是资源约束，不是已验证的性能承诺。

`xuiDocumentMapPosition` 支持输入与输出使用同一地址。TEXT/GAP 在语义事务中跟随原生 TEXT、SPLIT、MERGE、MOVE、DELETE 等操作；SOURCE 跟随实际源码字节补丁。段落拆分/合并单独记录容器间隙转换，配套子节点 MOVE 带 `XUI_DOC_OP_ANCESTRY_ONLY`，保留祖先关系而不重复移动间隙。调用方应使用 MapPosition，避免自行重复应用源码和语义两组操作。

Markdown 语义编辑删除 Text 节点的全部内容时，使用改变块的结构回写与完整语义核对，避免快速源码替换留下空的加粗、斜体、代码或链接包裹标记。空 Text 在 Markdown 中没有独立可表示的 token；序列化使用过滤空 Text 的私有子节点序列，让相邻标记选择、相同格式合并和共有格式提取看到同一组实际文字，不修改语义树或已有快照。Rich 的空格式节点仍由自己的语义模型保留。两种文本替换 API、Editor 字素 Backspace、合法结果光标、未修改块的原字节、Undo/Redo 及分配失败时源码/树/语法/历史的原子性已有回归。其他部分文本编辑仍使用已有精确源码替换路径。

`xui_doc_change_info_t.iDomain` 表示事务的权威编辑域。相同 `iGroup` 仅在 origin、domain 都相同且提交连续时合并历史；切换 SOURCE/SEMANTIC 形成一个历史边界，仍使用同一个 Undo 栈。这避免把需要不同坐标系统的操作拼成没有中间快照的映射。操作记录和 ChangeInfo 结构已有扩展，使用方需要与 DLL 一起重新编译。

## 历史预算与快照诊断

`xui_doc_desc_t.iHistoryLimit` 和 `iHistoryMaxBytes` 同时约束 Undo/Redo 的总步数及历史额外存储；零值选择默认值，`UINT64_MAX` 取消字节限制，`bDisableHistory` 关闭记录。历史字节由实际可达的存储图计算：当前版本已经需要的数据不重复计费，其余历史数据按共享对象去重，再加历史记录和操作数组；包括 Document 的分配头，不包含系统堆管理开销。

`xuiDocumentSetHistoryLimits(document, max_steps, max_bytes)` 可立即调整预算。超额时先丢弃距离当前位置最远的一步，距离相同优先丢弃最旧的 Undo；只从栈尾移除，保留连续撤销链。单步超出预算仍允许内容提交，但可能无法保留该步；Undo 后原来的大文档变成历史占用，也可能淘汰超额的 Redo。`xuiDocumentClearHistory` 释放全部 Undo/Redo。两者均不分配内存，不改变内容、revision 或保存点；存在写事务或内容回调时返回 BUSY。

启用历史且 Undo/Redo 均为空时，当前根保持一份不计入历史额外字节的待命所有权。第一次可撤销提交将它转给真实历史记录；清空历史或最后一步被预算淘汰时，在释放真实记录前转回当前根。这使后续局部编辑的历史计费只访问发生变化的持久路径。新文档/导入仍需建立当前根的完整计数；待命所有权不延长快照或历史记录的生命周期，也不改变 Undo 可用性。

`GetStats` 以缓存计数提供 `iCurrentBytes`、`iHistoryBytes`、`iHistoryMaxBytes`、`iSnapshotCount`。详细的 `GetMemoryStats` 遍历当前仍保留的快照并集且不分配内存；普通快照获取仅登记句柄，不因此扫描全文。最后一个快照引用释放时，仍会回收它独占的旧存储。多个 Snapshot 句柄或多次 Retain 不重复统计共享内容。

| 详细字段 | 含义 |
| --- | --- |
| `iLiveBytes` | Document 分配器仍存活的总字节 |
| `iCurrentBytes` | 当前已提交版本可达的唯一存储 |
| `iHistoryBytes` | 预算使用的历史额外存储，含记录/操作数组 |
| `iSnapshotBytes` | 所有保留快照的唯一存储及句柄，包含与当前/历史共享的部分 |
| `iSnapshotAdditionalBytes` | 当前和历史之外、快照仍保留的数据及句柄；这些数据也可能被外部 ChangeSet/事务持有 |
| `iOtherBytes` | 剩余控制句柄、事务、ChangeSet、私有候选和临时分配 |

`iLiveBytes = iCurrentBytes + iHistoryBytes + iSnapshotAdditionalBytes + iOtherBytes`。历史预算仅控制上述历史项；当前内容、快照和事务分别具有明确的所有权。淘汰历史不会使外部快照失效，快照占用会转入额外保留项。`xuiDocumentSnapshotGetMemoryStats` 通过保留快照查询同一分配器的诊断，支持读取线程及 live Document 已销毁的情况。

公开描述和统计结构已经扩展，使用方需要与 DLL 一起重新编译。节点属性在同一 Document 分配器内按值共享；节点和快照只读取不可变属性，修改属性时取得新的共享对象。当前版本、历史和快照的内存统计按可达的唯一属性对象计费。不同 Document 的分配器不共用属性对象。池的固定入口桶内使用按哈希排序的 AVL 树，避免大量不同样式退化为长链；相同哈希的极少数对象仍以短链核对完整属性。

## Renderer 布局缓存预算

`xui_doc_renderer_stats_t.iTextRunBytes` 累计成功构造的新文本 run 输入字节，包含显式换行的私有测量文字；复用已有 run 及联合/逐行重复整形不重复增加该计数。`iShapedBytes` 继续记录实际成功整形字节，因此可同时检查前缀材料化范围与字体工作量。失败后重新构造、缓存逐出后再材料化会再次计数；两者都不是当前占用，当前占用用 `iLayoutCacheBytes`。新字段扩展了公开统计结构，宿主与 DLL 需要使用同版头文件重新编译，并设置 `iSize=sizeof(xui_doc_renderer_stats_t)`。

同一段落中的 Text 节点按实际字体、上下标位移、对象及强制换行控制边界划分整形段，每段共用连续文字及逐视觉行的整形结果；对象保留测量得到的宽高和基线，所有文字和对象仍统一确定段落换行与逻辑行基线。LF、CR、CRLF、VT、FF、NEL、LINE SEPARATOR 和 PARAGRAPH SEPARATOR 控制片段使用共享 Unicode 断行决定，宽度为零、参与行高但不进入文字绘制字符串；CRLF 和跨节点 CRLF 保留一个强制换行，空白行保留高度。完整逻辑块末尾的强制控制或 HardBreak 之后保留一行可输入的空行，使用末尾控制的字体行高；文字/代码的 EOF 光标在该行起点，BEFORE/AFTER affinity 均不会落回控制前一行。HardBreak 的末行命中返回其父容器末端 GAP，输入可在该处创建文字；内部代码分片和段落未提交前缀不重复建立末行。这是行内强制断行，PS 不隐式创建 Document 段落节点，结构段落的样式仍由 Paragraph/Heading 等节点决定。

SHY（U+00AD）、ZWSP（U+200B）、WJ（U+2060）和 FEFF 的显示投影由普通文本与 Document 共享。整形与绘制使用删除这些格式字符后的字符串，节点、源码、复制和编辑位置仍指向原始 UTF-8 字节；跨 Text 的颜色跨度用私有显示偏移表映射，不修改内容。删除格式字符后形成的显示字素保持整体换行和端点几何。ZWSP 提供断点，WJ/FEFF 保留两侧不可断行规则。SHY 默认沿用普通文本的 '-' 策略：测量其自身实际字体，只有选择该断点时才在行尾绘制并计入光标/命中宽度；末尾 SHY、代码及 SOURCE 的不折行文本不会凭空显示连字符。可用的联合整形路径在行溢出时，按实际行起点、字体/位移跨度和终端 '-' 重新测量 SHY 候选；正负字距、较窄的连字符及候选拒绝后寻找更早断点已测。候选、绘制或收敛检查发生真实整形错误时返回错误并可重试，临时度量不会进入已发布几何。完整/拆分 Text、解析得到的 Markdown、20/40 字号及改宽均有回归。联合路径已移除 8 KiB/64 run/512 行三个固定阈值，并覆盖长段落的可见前缀与续排；未收敛时采用下述逐行候选测量。其余不支持的 cluster 映射回退路径，以及语言特定连字符形态/拼写变化与复杂脚本仍待补全，不能视作所有语言的断词实现。
代码块的分片/估高和精排使用同一 Unicode 强制换行规则，读取块保留三个前瞻字节以处理跨边界 UTF-8 控制和 CRLF。SOURCE 显示也可在源码物理行内显示 Unicode 强制视觉行，行距与相邻物理 SOURCE 行一样为零；源码索引和文件语法中的 CR/LF/CRLF 行定义不变。普通 Text/代码块仍按配置 fLineGap 排列视觉行。Glyph shaper 的标记不代替 Unicode 的强制换行决定。

SoftBreak/HardBreak 使用与 Text 相同的有效属性和字体解析，支持父级字体大小、字体族和颜色，以及节点自身的格式和上下标。SoftBreak 按所选字体测量一个真实空格；同字体及位移的相邻 Text 和 SoftBreak 共用连续文字整形、颜色跨度及装饰，链接下划线、删除线和背景覆盖空格。HardBreak 使用该字体的行高和逻辑基线，宽度为零并强制换行。测量空格只属于 Renderer 缓存，两个节点仍是文本长度零的结构原子，不改变可编辑偏移、Markdown 来源或保存内容；连续软换行各占一个空格。cluster/字体上下文尚不支持联合整形时，仍保留独立测量和装饰；drawText-only 代理的可用联合路径见下文。

上下标沿用默认 75% 字号和 30% 行高位移；自定义 onFont 返回相同字体时，位移不同的实际文字仍分别绘制。文字颜色、下划线、链接、删除线、高亮，以及未改变实际字体的粗体/斜体/代码标记，不再单独切断整形；装饰线在文字绘制后按实际文字基线、节点样式与完整字素范围绘制。同色相邻装饰合并，换行分别绘制。空 Text 的字体和位移不切断实际文字的整形，但字素之外的空节点保留自身插入光标度量；绘制使用首个实际文字片段的位置。联合路径的断点、宽度及绘制组数组按实际材料化范围分配，段落字节数、run 数和行数不再触发这三个固定阈值的回退。完整的单 run 字体结果可作为初始度量，跨 run 范围和实际视觉行仍按需重新整形。可见前缀和续排共用此路径；续排只替换未提交行的绘制组，后续整形失败可恢复原光标几何及绘制文字。内存或整形接口整数范围不足时返回错误。

联合断行先做至多八次全局重排；未稳定时改用有限的逐行贪心规划，按源码顺序测量正常候选行的实际显示字串，遇到下一候选过宽便保留上一可容纳断点。首个词过宽时使用字素安全的应急断点；不可拆的单元允许溢出，每行至少消耗一个片段。候选使用实际字体/位移跨度、对象度量及选中 SHY 的终端连字符。选定断点及最终逐行整形一次发布，行组装直接使用这些断点，避免把不同候选上下文的 advance 混用后重新断行。循环与超过八次的慢收敛、Rich 完整/颜色拆分及 Markdown、候选和最终绘制组整形失败后的重试已有回归。该备用策略会重复测量候选前缀，不保证对任意内容依赖整形器的最少整形量，也不寻找非单调宽度下的全局最优断点。源码顺序的合并 cluster 已支持内部字素光标停点（见下文）；其余 cluster 映射仍需处理；drawText-only 的上下文路径见下文。真实字体连字整形与字体停点获取、Bidi 和完整复杂脚本整形仍待完成。

可用联合路径上的一个显示字素若跨越实际字体或上下标不同的 Text 节点，完整字素采用首个有显示内容片段的字体及上下标位移；源属性变化在下一个字素生效。空 Text 和零显示字节的格式片段不选择字形字体。此规则用于种子、实际行候选、最终绘制组和基线/行高度量，避免样式节点中间切断组合字符或 ZWJ/区域指示符字素。Document 节点的原字体/位移属性、UTF-8 内容、Markdown 实体与源码拼写仍保留；不会在字素内新增光标停点。颜色和装饰继续使用各自既有规则，并非把整个字素的全部源样式合并成第一节点。它是 XUI 的明确样式策略，不声称等同全部前端编辑器或复杂字体的回退/连写/Bidi 行为。

`xui_text_shape_t` 末尾增加可选 `iCaretCount` / `pCarets`，元素为 `xui_text_caret_t`。整形器可以给覆盖多个 Unicode 字素的原始 cluster 提供精确内部光标位置：`iTextOffset` 是整形输入的 UTF-8 字节偏移，严格递增；`fAdvance` 是从该原始 cluster 前缘起算的累计 advance，同 cluster 内非递减且位于 `[0, cluster.fAdvance]`。不包含端点；一个 cluster 若提供内部位置，必须提供该 cluster 全部内部字素边界。数组与 `pClusters` 一样使用 XRT 分配器分配，由 `xuiTextShapeFree` 释放；自定义 proxy 不能交付普通 `malloc` 数组。宿主、proxy 和默认/可选 DLL 必须使用同版头文件重新编译，没有旧 ABI 兼容。

Document 在映射回原始源码偏移之前，将有序连续的多字素 cluster 按这些停点拆成内部度量片段；未提供停点的 cluster 使用按 Unicode 字素等分的明确约定，不推断字体特定的光标位置。组合字符和 emoji 字素内部不增加停点，删除的 WJ 等格式字符也不参与等分。缓存 run 和最终行整形使用同一处理，保留原 cluster 总 advance；光标、命中、选区及重排共用结果，字素之间的紧急断行会重新整形实际行。非法偏移、缺失停点、非有限值、逆序或超出 cluster 的坐标返回错误，不静默接受。当前 XGE 后端仍未提供完整 GSUB/GDEF 整形/停点，相关回归使用确定性的代理；无序 cluster、完整 Bidi 和复杂脚本还需单独实现与真实字体验收。XGE→XUI 适配器把 CRLF 的 CR 纳入 LF 强制换行 cluster，保持原始输入范围连续。

`xui_font_metrics_t` 新增 `fUnderlinePosition`、`fUnderlineThickness`、`fStrikePosition`、`fStrikeThickness`。位置是相对基线的有符号偏移，正值向下；XGE proxy 直接传递字体的度量。自定义 `fontGetMetrics` 应先清零完整结构再填写字段；厚度不大于零、字段非有限值或查询失败时，Document Renderer 使用默认装饰度量。普通文字与联合绘制使用同一套规则，避免重复下划线。结构从 5 个 float 扩为 9 个 float，宿主、自定义 proxy 与 DLL 必须使用同版头文件重新编译。`drawLine` 是 `xuiSetProxy` 校验要求的基础回调。

仅提供 `drawText` 的 proxy 也使用可用的联合行整形、SHY 候选及有限收敛路径，断点/光标/范围/绘制共用实际行字符串。一个视觉整形组颜色一致时，只调用一次 `drawText` 绘制完整上下文；不同 Text 节点的相同颜色和装饰不会重新切断绘字。多色组需要 `drawClipGet` / `drawClipSet` / `drawClipClear`，按字素前缘分成不重叠的颜色区间，每区间仍绘制完整行字符串。跨节点组合字素由首个实际文字片段的前景色拥有，零显示字节的空节点/格式字符不分颜色；首尾颜色区间延伸至外部裁剪边缘，避免新增颜色边界裁去外侧像素；drawText 自身的 XUI_TEXT_CLIP 规则仍生效。缺少所需裁剪回调的多色组返回 UNSUPPORTED，单色仍可绘制。

这条回退按视觉区间给字形像素着色，不能等同于字形伸出/重叠处的原生逐字形颜色归属；若需要原生颜色精度和较少绘字工作，应实现 `drawTextSpans`。颜色区间越多，完整字符串重复绘制次数越多，不以本批代理或 Arial 用例宣称统一跨平台帧耗时。两条绘制路径共享原有装饰处理。Renderer 对裁剪查询、入口设置、绘字和最终恢复/清除的错误均返回失败；颜色裁剪结束或失败会尝试恢复外部裁剪，退出时再尝试恢复调用方原状态；恢复本身失败继续返回错误。单次注入错误后，同一 Renderer 可重试。

若宿主的 `textShape` 会让普通可打印 ASCII 的行高随内容变化，应在 Renderer（或 View/Editor 内嵌的 Renderer）描述中设置 `bSourceLineHeightMayVary = 1`。SOURCE/LIVE 的离屏光标、选区与命中会先确认前置源码行的实际高度；超过 8 KiB 的 ASCII 源码行也会完整整形，避免决定行高的字符落在前缀之外。默认零值沿用按字体度量估高和长行前缀排版，以维持普通大文档性能。该选项可能让深处冷查询排版大量前置行；若宿主改变同一字体的整形规则，仍须调用对应的字体失效接口。

`xui_doc_renderer_desc_t.iLayoutCacheBudgetBytes` 为每个 Renderer 单独设置块布局缓存的软预算；零值使用 32 MiB。VISUAL、SOURCE 和 LIVE 使用同一规则。计入块持有的 UTF-8 文本、shaping clusters、片段及对象几何数组的分配容量；不计 Document/Snapshot、块目录与高度索引、字体及 XUI 资源注册表。`xuiDocumentRendererGetStats` 返回当前 `iLayoutCacheBytes`、实际 `iLayoutCacheBudgetBytes` 和累计的预算淘汰次数 `iLayoutCacheEvictions`；普通内容失效不计作预算淘汰。

`RendererLayout` 优先精排可见块，再回收离屏块。缓存接近预算时停止离屏预排，避免下一帧重复 shaping。淘汰会保留原块高度作为估算，因此 `GetSize` 的 `exact` 可能重新变为假；回访时会按需精排并修正高度。可见块必须保留，因而单个很大的块或当前可见块之和可能超过预算。离屏光标/命中查询可以在下一次 `RendererLayout` 前暂时增加缓存。此预算不限制文档本身、外部字体/图片资源或渲染目标表面的内存。

超过 8 KiB 的多行代码块在 Renderer 目录中按完整硬换行分段，只有进入视口或被光标/命中查询的段才进行 shaping；Rich 与 Markdown 仍保留同一个代码节点 ID、全局文本偏移及根历史。分段高度先按行数估算，精排后修正；最后一段才加入段后间距。没有安全断行点的超长单行仍需整段首次 shaping；长 ASCII 及含 Unicode 的自动换行段落可按字素与断行共用的切点进行首屏前缀排版。

已精排的普通块按视觉行保存片段范围与纵向边界。Draw 只扫描裁剪区相交的行，HitTest 只扫描目标行，文本 run 的光标查找按片段偏移二分定位。表格 Cell 会在行高计算后重新定位，Renderer 对含表格的块保守地扫描全部片段。行索引计入上述布局缓存预算，改宽重排时重建。`xuiDocumentRendererGetStats` 的 `iDrawFragmentsExamined`、`iHitFragmentsExamined`、`iCaretFragmentsExamined` 是累计访问量，可用于核对扫描成本；这些数字不代表实际绘制量或 shaping 字节。100,000 字节 ASCII 单段落现可首帧只 shape 可见前缀；行索引继续降低其后续绘制、命中与光标扫描量。

段落/标题在 shaping 各样式 run 之前构造一份连续 UTF-8 断行投影，记录子节点在投影中的范围；文字保留原字节，软/硬换行与对象使用与 Renderer 一致的占位字节。跨 run 的 Unicode 断行边界由同一投影计算，再映射回片段，因此样式边界不会凭空产生断行；分布在相邻 run 中的 CRLF 也只形成一次硬换行，单 run 的代码块使用同一规则。空 alt 图片两侧的结构 gap 光标直接取对象片段的左右边界，不依据 alt 字节长度。长段落的前缀与全量布局沿同一组字素/断行切点 shaping，使已提交片段的 run 级行高不受后文字符改变；含组合字符、emoji、阿拉伯文和中文的专项已通过前缀几何回归。部分段落会在 Renderer 缓存中保留投影、切点和已提交的视觉行；同块滚动、深处文本光标、以文本或同段直接子节点之后的 gap 为终点的选区查询，以及段内可选对象或文本叶节点矩形查询沿安全切点只 shape 新增片段，并从最后一条未提交行继续排版。尚未排版的冷 Renderer 遇到上述文本光标、可识别终点选区或段内节点矩形查询时，先建立最小前缀，再沿同一路径续排。宽度、快照或资源失效及缓存淘汰会释放这份状态；无后续安全切点的靠尾光标/选区、跨容器结构终点和延伸到块尾的文本节点几何查询仍可能补全整块，完整段落 shaping/Bidi 仍待实现。

连续 ASCII 基字母/数字组成的极长词现在也能沿 Unicode 字素边界上的紧急换行点分段；组合附标与 ASCII 基字组成的字素不会拆开。Renderer 优先使用附近的正常/硬换行点，只有缺失时才选紧急切点；前缀与完整排版使用相同切点。为避免破坏连写脚本的字形上下文，非 ASCII 基字的词内紧急切点仍走整块 shaping；代码块的无硬换行长行也尚未实现水平按需 shaping。

长段落前缀后的块高为估算值，`GetSize.exact` 为假。在当前部分块内部滚动或按 Y 命中未排区域时，Renderer 沿切点追加可见内容；跨过该块定位后续块时先补全前置块并重新定位。如果当前块补全后高度收缩，命中会再次查找所在块。离屏 `GetNodeRect` 仍可能给出估算的绝对 Y；需要精确几何的调用方应先进入目标区域或查询目标光标。

`xuiDocumentRendererGetRangeRects` 根据当前快照的非折叠选区返回文档坐标系中的矩形；反向选区得到相同结果，相邻同一视觉行的片段合并为一个矩形，字素簇与对象按绘制路径作为不可分割单元。`rects=NULL, capacity=0` 可先查询总数；缓冲区不足时只写入前 `capacity` 个，`total` 仍返回完整数量。查询按需精排选区覆盖的离屏块，根节点的结构 gap 会定位到相邻子树，避免为单个顶层对象扫描整篇文档。选区之前尚未精排的块仍可能使绝对 Y 使用估算高度；需要稳定的屏幕坐标时，应先让目标视口完成布局。当前验证覆盖 VISUAL 文本/对象、表格与 Markdown SOURCE 行；完整跨 run shaping 和 Bidi 仍待实现。

## 创建和编辑

```c
#include "xui_document_ui.h"

xui_document document = NULL;
xui_doc_desc_t desc = {0};
desc.iSize = sizeof(desc);
desc.iProfile = XUI_DOCUMENT_MARKDOWN; /* 或 XUI_DOCUMENT_RICH */
desc.iMarkdownDialect = XUI_MD_GFM;
int result = xuiDocumentCreate(&desc, &document);
if (result == XUI_OK)
    result = xuiDocumentLoadMarkdown(document, "# Hello\n", 8);

/* 省略调用方对 result 的显示/处理。 */
```

所有公开结构先清零并设置 `iSize`。调用方检查每次操作结果；失败事务不会部分发布。

```c
xui_document_transaction transaction = NULL;
xui_doc_node_desc_t node = {0};
xui_doc_node_id paragraph = 0, text = 0;
int result = xuiDocumentBeginTransaction(rich_document, NULL, &transaction);
if (result == XUI_OK) {
    node.iSize = sizeof(node);
    node.iKind = XUI_DOC_PARAGRAPH;
    result = xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT,
        XUI_DOCUMENT_APPEND, &node, &paragraph);
}
if (result == XUI_OK) {
    node.iKind = XUI_DOC_TEXT;
    node.sText = "Hello";
    node.iTextBytes = 5;
    result = xuiDocumentTxnInsertNode(transaction, paragraph, 0, &node, &text);
}
if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
xuiDocumentTxnRelease(transaction);
```

一次复合操作使用一个根事务。历史、通知和发布所需数据在发布前分配；失败保持原内容、revision、历史和保存状态。通知回调内写入返回 `XUI_DOC_ERROR_BUSY`。空事务不产生历史。`iGroup` 可以显式合并连续、同来源/域的提交；编辑器的交互输入使用同一机制自动分组，不创建独立历史。

`xuiDocumentTxnReplaceRange` 支持原生文本插入、字面节点编辑、段落拆分、同段跨 run 替换、同父段落之间的替换。表格 API 支持插入表格、行列增删、矩形合并与拆分；合并、行列变化和单元格正文共用根历史。

富文本表格的列宽属于 Table 节点，不依附第一行或某个 Cell。`xuiDocumentTxnSetTableColumnWidth(table, column, width)` 以逻辑单位设置首选宽度，`width=0` 恢复自动；`xuiDocumentSnapshotGetTableColumnWidth` 读取不可变版本。单个事务可同时调整多列，行增删、合并/拆分不改变列宽；插入列为新列添加自动宽度，删除列移除对应宽度。原生 JSON 保存完整列宽，HTML 导出使用 `colgroup`。Renderer 在缩放后先分配显式宽度，剩余宽度分给自动列；有显式宽度时自动列最少 24 逻辑单位，空间不足会产生水平溢出。Markdown 方言不能无损表示该富文本属性，事务明确返回 `UNREPRESENTABLE`。富文本 VISUAL Editor 在单元格内部列边界及最右侧外边界的 4 像素热区显示横向缩放光标，指针拖动实时修改首选宽度并合并为一次 Undo；拖动外边界时首次事务还会固定其他自动列的当前显示宽度，使外边界跟随指针，Undo 一步恢复自动列。`xuiDocumentEditorSetTableColumnWidth` 提供相同列宽事务路径的程序调用。VISUAL 富文本 Editor 的 `TABLE_COLUMN_NARROWER/WIDER/AUTO` 命令以当前单元格右侧列为目标，每次增减 8 逻辑单位或恢复自动列宽；合并 Cell 使用跨度最右列。Ctrl+Alt+Shift+左/右箭头调整，Ctrl+Alt+Shift+0 恢复自动。最右侧列首次键盘调整会固定其他自动列的当前显示宽度，使外边界实际移动，整次命令一步 Undo；只读和 Markdown 模式不启用这些命令。跨视图滚动体验仍待实现。

VISUAL Editor 的 `XUI_DOC_EDIT_TABLE_NEXT_CELL` / `XUI_DOC_EDIT_TABLE_PREVIOUS_CELL` 可由命令接口查询与执行；Tab / Shift+Tab 在表格单元格内调用相同路径。导航以当前光标所在的最内层表格为范围，按真实 Cell 顺序跨行，合并单元格只停一次，空的跨度行跳过；移动到已有 Cell 只折叠选区，不改变 Document revision 或 Undo。末尾 Tab 在可编辑状态追加一行，将光标放在新行首格并形成一步 Undo；只读状态仍可在已有 Cell 间导航。方向键先按 Renderer 的文字排版移动；当 ↑/↓ 在单元格末端无法移动时，按当前逻辑列进入上一行或跨过合并跨度进入下一行，目标 Cell 使用相应的末端或起始位置。已验证 Rich 合并单元格、GFM 表格、只读导航和不创建历史；SOURCE/LIVE 模式没有表格导航。

同一命令接口提供 `TABLE_INSERT_ROW_BEFORE/AFTER`、`TABLE_DELETE_ROW`、`TABLE_INSERT_COLUMN_BEFORE/AFTER`、`TABLE_DELETE_COLUMN`、`TABLE_SPLIT_CELL`、`TABLE_MERGE_CELLS` 与富文本列宽命令 `TABLE_COLUMN_NARROWER/WIDER/AUTO`。它们在 VISUAL 模式通过一个根事务执行，行列命令支持 Rich 与 GFM/EXTENDED Markdown；拆分、合并仅支持 Rich。插入命令把光标放在新位置，删除整张表后仍保持合法结构光标；VISUAL 模式下 Undo 恢复编辑前的普通文字或矩形单元格选区；Redo 清除已提交操作后的矩形选区。合并命令接受普通选区两端 Cell 围成的矩形，也接受 Alt+拖动、Alt+Shift+方向键或 API 产生的单元格矩形选区；查询时拒绝切穿已有跨度或只包含一个 Cell 的区域，执行时由内核再次验证。

`xuiDocumentTxnPasteTableMatrix` 接受 UTF-8 TSV 矩阵；双引号包围的字段可包含 Tab、换行，并用 `""` 表示字段内的双引号。未闭合引号或闭合引号后出现非分隔符返回 FORMAT。Rich Cell 保留字段内换行；Markdown 表格无对应表达方式，明确返回 UNREPRESENTABLE 且不提交。粘贴从指定 Cell 开始，短行缺项清空对应目标格，必要时在表格右侧或底部补列补行。目标矩形不能碰到合并 Cell；全部 Cell 在一个根事务中替换，Markdown 经过影子树回写与语义核对。VISUAL Editor 的普通粘贴在折叠光标位于表格内且剪贴板文本含 Tab 时使用此路径，粘贴后光标停在最后一格，Undo 一步恢复。

`xuiDocumentSnapshotCopyTableMatrix` 从 Rich 或 Markdown 的 Cell 矩形生成可往返粘贴的带引号 TSV，合并 Cell 需要完整落在矩形内，覆盖的格子导出为空字段。VISUAL View/Editor 使用 Alt+拖动选择完整单元格，着色显示选区；松开时自动扩展到包含相交的合并 Cell。Ctrl+C、通用 `xuiEditCopy` 和 Editor Copy 命令共享复制结果。`xuiDocumentTxnClearTableMatrix` 在一个根事务中清空矩形内每个完整 Cell 的正文，保留表格网格和合并跨度，切穿跨度时原子拒绝；Markdown 走结构回写与语义核对。矩形 Cut 先复制带引号 TSV 再清空，Delete、Backspace、通用 `xuiEditDeleteSelection` 与空文本替换也执行整块清空；Undo 一步恢复。对选中矩形粘贴从左上角开始；粘贴单个字段或键入文字只替换左上角 Cell。Rich 文档的矩形 Enter 只替换左上角 Cell 并在其中插入段落分隔，其他 Cell 不变，整个动作可一步 Undo；Markdown GFM Cell 不能表示段落分隔，命令查询和执行均明确返回 `XUI_DOC_ERROR_UNREPRESENTABLE`，不修改文档。新选择、模式切换或文档修改会清除矩形选区。

`xuiDocumentViewSetTableSelection` 允许宿主工具栏与自动化在 VISUAL View/Editor 中按 Table ID 和网格坐标指定单元格矩形；它与 Alt+拖动共用合并 Cell 的完整跨度展开规则。设置时普通光标定位在左上角 Cell 的内容起点，嵌套表格不会夺走外层表格的命令上下文；传入 `NULL` 清除矩形并保留普通光标。无效表格或越界范围原子拒绝，原选区和 Document revision 不变；只读状态仍可选取，禁用选区或 SOURCE/LIVE 模式拒绝设置。`GetTableSelection` 返回展开后的范围。

VISUAL View/Editor 在 Cell 内使用 Alt+Shift+方向键选择完整单元格：首次按键以当前 Cell 为固定起点，之后的方向键移动终点，可扩展或收缩矩形；跨越合并 Cell 时按完整跨度选择。Alt+拖动和程序化选区也保存起点/终点，后续键盘操作沿该方向继续；普通 Shift+方向键仍选择文字。只读 View/Editor 可以选择，SOURCE/LIVE 不建立表格矩形；Escape 清除矩形。矩形内执行结构命令后，Undo 恢复选区及键盘延伸方向，整个选择过程不修改 Document revision。

`xuiDocumentTxnCopySubtree` 从保留的 Snapshot 复制一个完整语义子树到目标 Document，逐节点分配新的目标 NodeId，复制文字、marks、资源描述及富文本表格列宽；来源文档释放后，Snapshot 仍可作为复制输入。`xuiDocumentTxnCopyRange` 接受非折叠语义选区，按最近共同祖先的子节点形成结构片段：同一 Text 内按 UTF-8 边界裁剪，跨 run 保留 marks/链接/资源，跨段落复制首尾裁剪后的块和中间完整子树；完整选中的图片等原子节点可复制。调用方提供可容纳这些片段根节点的目标父节点及插入位置，返回首个新 NodeId 与根节点数。部分选择表格、Row、Cell 或非 Text 正文节点明确拒绝，表格矩阵仍使用专门接口。复制、同一事务内其他修改和撤销共用根历史；Markdown 目标先写入影子语义树，再生成源码并重新解析核对，不可表达的属性返回 `UNREPRESENTABLE` 且不发布。新顶层块插入时补齐原来源所需的 LF/CRLF 块分隔符。这些入口是原生 fragment 的复制原语。

`xuiDocumentFragmentCreateRange` 将非折叠语义范围捕获为独立、只读的 Rich Document 片段；调用后可释放来源文档与快照。`FragmentSerialize/Deserialize` 使用版本化 16 字节头部和原生 Document JSON，保留文字、属性、资源、完整表格及列宽；损坏输入须通过头部、JSON 与文档 schema 校验。`TxnInsertFragment` 把多个片段根节点一次插入可容纳它们的目标父节点。`TxnReplaceRangeWithFragment` 在同一事务内替换选区，在行内 Text 中分裂 run，在块片段落点分裂段落并返回新光标；Rich 与 Markdown 都使用统一事务，失败不发布。VISUAL Editor 的 Copy/Cut 同时发布纯文本与原生片段，Paste 优先读取原生片段；只读 View 也可发布原生片段，空 alt 图片选区仍保留图片节点和资源描述。原生数据损坏或缺失时尝试 HTML、PNG，再回退纯文本，SOURCE/LIVE 与表格矩形仍使用原有文本/TSV 路径。部分 Row/Cell 片段仍不支持捕获，复杂列表/表格光标可能因目标 schema 不兼容而拒绝；完整 HTML5/CSS 导入、跨应用矩阵及图片资产持久化仍待实现。

`xuiDocumentFragmentExportHtml` 把片段中的实际选中节点导出为 UTF-8 HTML；行内片段只输出行内节点，块片段保留段落等结构。VISUAL Editor 和只读 View 的语义选区复制可同时发布纯文本、原生 fragment 与 `text/html`，空 alt 图片也能以 `<img>` 导出。调用方用 `xuiDocumentFreeBuffer` 释放结果。HTML 导出会转义正文与属性，并过滤不安全的链接/图片 URL；字号、段距、图片尺寸与表格列宽使用与进程数字区域设置无关的十进制写法。HTML 中图片携带资源描述及可选的外层链接；单张已注册图片的选区复制会额外发布 PNG 像素。

`xuiDocumentFragmentImportHtml(html, bytes, out)` 先在私有 Rich Document 中把 UTF-8 HTML 转成统一 schema，再提交校验并返回独立 fragment；失败不会改动目标文档。当前识别段落、标题、引用、列表、代码块、规则线、表格及跨度/列宽，行内 marks、链接、图片和部分颜色/字号/字体属性；内联 CSS 的 font-weight、font-style、text-decoration(-line) 映射为粗斜体、下划线与删除线，段落中的这些 marks 传给正文；body、div、article、section、main、aside、列表与表格容器上的内联文字颜色、字号、字体、粗斜体和对齐逐层传给后代段落与正文，子级 normal 等声明可覆盖祖先样式；容器背景色和段后间距不传递；同一 style 中只有当前支持且值有效的声明才参与后写及基本 !important 优先级比较；无效或尚不支持的值不覆盖先前有效声明。颜色可导入 CSS 命名色（含 `rebeccapurple`）、`transparent`、3/4/6/8 位十六进制、常见 `rgb()`/`rgba()` 和 `hsl()`/`hsla()` 的逗号或空格语法，以及 `hwb()` 的空格语法；HSL 色相接受无单位角度、deg、grad、rad、turn，现代语法接受饱和度/亮度数字或百分比和斜杠透明度，旧式逗号语法要求饱和度/亮度百分比；HWB 允许百分比或以 0–100 为参考范围的数字，白黑合计达到或超过 100% 时转为比例灰度，不接受逗号，数值仅接受 CSS 十进制形式（可带指数），颜色、字号、字重、图片尺寸与表格列宽的数值解析不依赖进程数字区域设置。`background-color:currentColor` 保留关键字，在绘制时取该元素的当前文字色；`color:currentColor` 在 Document 祖先可表达时保留继承语义，HTML 容器被压平且已提供颜色时实体化为 RGBA，以保持导出外观。字号可导入 px/pt，段后间距可导入 px/pt 与无单位零；尚不解析 `lab()`/`color()` 等颜色表达式、`calc()` 或缺失分量 `none`；零 RGBA 由属性标记区分显式透明黑和未设置，导入、导出均保留；二值 marks 不能保留完整数字字重或装饰线样式；实体被解码，脚本、样式表及其他活动元素的内容被丢弃，不安全 URL 不进入链接或图片节点。VISUAL Editor 粘贴依次尝试原生片段、HTML、PNG 图片、纯文本，成功后仍使用一个事务和一步 Undo；SOURCE/LIVE 与表格矩形继续使用原有文本路径。它不是完整的 HTML5 树构建器或 CSS 引擎：畸形标签自动修复、浏览器样式层叠、复杂表格与任意网页内容仍需后续实现和跨应用验收。

同一 XUI 之间的 HTML 片段会写出并读回 `data-xui-object` 元数据：行内/块公式、Mermaid 图形、HTML 源码、front matter、扩展节点、脚注/引用以及任务列表勾选状态保留对应节点类型；对象的资源、info 和标题为转义后的描述字段，不承载运行时对象。段落/标题的对齐与显式段后间距使用受限 CSS 往返，字体族的 CSS 十六进制转义也会解码。导入器限制输入和单段解码缓冲为 256 MiB，显式对象缺少结束标签返回 FORMAT；这仍不等于任意浏览器 HTML 的语义无损往返。

富文本转 Markdown 须显式调用 `xuiDocumentSnapshotAnalyzeMarkdownConversion(snapshot, dialect, losses, capacity, total)`。分析结果按文档顺序列出受影响 NodeId、节点类型与损失原因，覆盖不支持的节点/方言、marks、文字/块样式、图片或表格几何、Cell 内容、被忽略的元数据与空段落；已知属性无损时还会实际执行一次私有转换，若重新解析的语义不同则报告根节点 `ROUNDTRIP`。可先以 `losses=NULL` 查询数量，再读取列表。`xuiDocumentSnapshotConvertToMarkdown` 仅执行严格无损转换，任何已报告损失或语义不一致都返回 `UNREPRESENTABLE`，不产生结果文档；成功时返回独立、干净且可编辑的 Markdown Document，其 SourceStore 可保存为 `.md`。该转换不修改来源快照或富文本 Document，也不隐式发生在模式切换或富文本 `SaveFile(MARKDOWN)` 中。若调用方明确接受报告中的 `MARK`、`TEXT_STYLE`、`BLOCK_STYLE` 或 `GEOMETRY` 位，可调用 `xuiDocumentSnapshotConvertToMarkdownWithPolicy`：它在私有 Rich 副本中仅清除被授权的显示属性，再运行严格转换和语义核对；未授权损失、合并 Cell、表格正文换行及解析差异仍原子拒绝。几何策略仅可舍弃图片尺寸、表格列宽等显示几何，不会展平合并结构。Markdown→Rich 可先调用 `xuiDocumentSnapshotAnalyzeRichConversion`，读取原始源码字节、引用定义和行内语法数量，再调用 `xuiDocumentSnapshotConvertToRich(snapshot, accepted_reasons, out)`；只有显式接受报告中的源码写法及来源层引用定义损失才会生成独立、干净且可编辑的 Rich Document。语义节点、marks、链接资源和表格结构逐节点复制，原 Markdown 快照不变；Rich Document 不保留 `.md` 的原始定界符、空白写法或未使用的引用定义。更广泛的结构内容降级策略与面向用户的损失确认界面仍待实现。

行内结构写回器可以把格式 run 首尾空格及相邻普通文字的边界字符写成数字字符引用，使 `alpha` + 加粗的 ` beta`、加粗的 `alpha ` + `beta` 以及两侧加粗、中间普通的片段无损转成 Markdown。相同规则已覆盖 CommonMark 斜体、GFM 删除线，以及 EXTENDED 方言的高亮、下标和上标；这些格式与普通文字相邻时的首尾空格、斜体单独 run 的首尾空格、两段同格式间隔普通文字及部分相邻混合格式均经过语义往返测试。连续两段加粗在这种边界上交替使用 `**` 和 `__`，避免解析器把相邻定界符重新绑定；拆分的普通文字节点、Unicode 边界与三段重复加粗/斜体结构也经过往返核对。相邻文字 run 共同具有加粗或斜体、另一种格式在其间切换时，写回器把共同格式提到外层，仅在切换处写入另一种定界符；加粗外层使用斜体 `_`，斜体外层使用加粗 `__`，并对必要的文字边界使用数字字符引用。两种嵌套方向、无空格边界和拆分 run 已通过严格语义往返。该改写共用于富文本转换和 Markdown 语义结构写回；更复杂的格式交错仍可能因往返不一致而被拒绝。

## Markdown 行为

`xuiDocumentTxnReplaceText` 编辑 Markdown 图片节点的 alt 时，按图片 `![...]` 内的普通 Markdown 文字转义新内容；例如输入 `]` 写为 `\]`，前导横向空白可写为数字字符实体。解析器也为 `![]` 记录零宽 alt 来源位置，首次输入和 `TxnUpdateImage` 更新图片属性都可走局部补丁。`TxnUpdateImage` 仅改变 alt 时只替换图片方括号内的来源字节，保留原来的内联或引用式图片写法；改变 URL 或标题时仍会改写目标图片语法。精确来源位置的补丁保留图片外的实体、换行和引用定义原文，并在提交前重新解析核对图片语义。图片 URL、标题和尺寸仍由图片属性命令处理。

| 方言 | 已接入的解析配置 |
| --- | --- |
| `XUI_MD_COMMONMARK` | CommonMark 基础解析 |
| `XUI_MD_GFM` | 加表格、删除线、任务列表、自动链接 |
| `XUI_MD_EXTENDED`（默认） | 加公式、脚注、admonition、front matter、Mermaid 围栏节点，以及 `==高亮==`、`~下标~`、`^上标^` |

EXTENDED 的 `[!NOTE]` 等提醒块在统一语义树中是带类型 `info` 的引用块；HTML 导出使用 `blockquote` 的 `data-xui-info` 保存该类型，导回 Rich 后仍可无损转换回 EXTENDED Markdown。原生 Renderer 用独立标题行显示类型并保留引用线；标题不进入正文，点击标题映射到引用起点，空提醒块也能定位光标。类型专用图标/配色、本地化与真实辅助技术验收尚待实现。

根层、引用、列表项和已用脚注中的标题修改先尝试来源补丁。现有 ATX 标题直接使用解析器确认的开/闭标记和原始正文范围：调整级别只替换开头井号，转普通段落时去掉开标记及其间空白、可选闭标记和其间空白，保留正文拼写及原行结束符。Setext 的一/二级转换保留下划线长度、缩进、尾随空白和换行；转普通段落删除完整下划线行，支持多行正文；单行 Setext 转三至六级插入 ATX 开标记并删除下划线行。段落转标题仍利用行内来源索引定位开头，不宣称已有完整段落 CST。多段补丁按原来源位置排序并逆序应用，拒绝重叠；单补丁优先使用既有局部解析，多补丁完整解析，候选必须与完整目标语义相等。所有未改引用定义保持原拼写；所选脚注允许标题正文修改，其标签和其他定义继续保护。不能证明等价的修改回退通用结构写回，可能原子返回 UNREPRESENTABLE；多行 Setext 转 ATX 等不保证原字节保真。

`GetBlockSyntax` 的 `iHeadingContentStart/End` 返回解析器最终确认的 ATX/Setext 原始行内正文范围，排除标题标记及其周围空白；空 ATX 标题有合法空范围。Setext 范围来自已消耗链接定义、已去掉下划线的正文行集，多行范围包含中间原行结束符和容器前缀。非标题节点这两个字段为 `UINT64_MAX`。内部保存相对块起点的两个偏移，源码平移不重写旧快照；它们是同一文档内核的派生来源元数据，不是第二份可写正文。扩展后的结构体需调用方与 DLL 一起重新编译，本项目不保留旧 API/ABI 兼容包装。

`.md` 保存直接取已提交源码，不从语义树重新生成整篇。有效 UTF-8 的 BOM、换行、空白、转义、实体和未闭合语法保留在 SourceStore 中。原生 Markdown 工程文件保存源码和方言，读取时重建语义树。

SOURCE_TEXT、LIVE_MARKDOWN 与 VISUAL 是同一文档的三个视图模式，共用时间顺序的 Undo/Redo。SOURCE_TEXT 输入会重新解析；VISUAL 文本修改会生成源码补丁并重解析，将结果与预期语义核对。无法表达时返回错误并禁止提交。

`xuiDocumentSnapshotGetSourceLine(snapshot, source_offset, out)` 对不可变 Markdown SourceStore 返回原始行的五个字节边界：行首、前导空格/Tab 结束、尾随空格/Tab 开始、正文结束、行结束。相邻区间完整覆盖一行；最后一段保留 LF、CR 或 CRLF 原拼写。可从偏移 0 起按 `iLineEnd` 顺序遍历，也可按行内任意字节定位；空源码和 EOF 返回 `NOT_FOUND`。

`xuiDocumentSnapshotGetBlockSyntax(snapshot, node_id, out)` 读取解析器确认的块标记字节范围。ATX 标题返回起始井号与可选闭合井号；Setext 标题返回下划线；围栏代码/Mermaid 返回起始围栏、可选闭合围栏与开围栏同行的原始尾部范围（包含语言/其他信息以及空格、Tab，可为空范围），还返回 MD4C 剥离首尾 ASCII 空格后的有效信息串和首个语言 token 范围；空串对应合法空范围，语言名后的元数据继续保留在原始尾部。分隔线返回去掉尾随空白后的标记范围。列表项返回无序符号或有序数字加分隔符、其后原始水平空白范围、可选的任务框 `[ ]`/`[x]` 及任务框后的空白范围，并返回 `iListIndentCount`；这两个首行空白范围可以为空，未出现任务框时后一对坐标为 `UINT64_MAX`。引用块返回首行 `>` 范围及 `iQuotePrefixCount`。GFM/EXTENDED 表格返回确认的分隔线内容范围及 `iTableTokenCount`。范围按当前快照的 SourceStore 偏移表示，旧快照、Undo/Redo 与局部解析后缀平移各自独立；不存在的第二标记及非围栏节点的尾部/信息字段以 `UINT64_MAX` 表示。普通段落等无已分类标记的节点返回 `NOT_FOUND`，Rich 快照返回 `UNSUPPORTED`。

`xuiDocumentSnapshotGetCodeIndent(snapshot, code_id, index, out)` 枚举非围栏代码块最终保留的每一行。返回前缀水平空白的原始字节范围、该行在原文中的逻辑缩进起始列、四列代码缩进的内容边界列和完整缩进终点列；Tab 边界可以位于同一原字节内部。代码块内部空行仍有记录，但标记为 `XUI_DOC_CODE_INDENT_BLANK`，其内容边界列为 `UINT32_MAX`；块前后的空行已由解析器剔除。`GetBlockSyntax` 的 `iCodeIndentCount` 给出记录数，主标记是首个保留行的缩进。围栏代码由单独的围栏来源接口表示。

`xuiDocumentSnapshotGetQuotePrefix(snapshot, quote_id, index, start, end)` 按源码顺序枚举该引用块每个由 MD4C 确认的显式 `>`，包含首行和续行，索引范围为 `[0, iQuotePrefixCount)`；没有 `>` 的懒续行不生成标记。多行引用只保存后续前缀相对引用块起点的紧凑偏移，单行引用无需额外数组。

`xuiDocumentSnapshotGetListContinuationIndent(snapshot, item_id, index, out)` 按来源顺序返回该列表项续行中被解析器实际接受的原始水平空白字节范围。起始项目符号所在行不计入；缩进不足而以懒续行归属的行也不计入。范围从该层最近的引用 `>` 后开始，直到首个非空白字节；嵌套列表项可分别拥有重叠的同一行范围，Tab 按原字节保留。`iIndentStartColumn`、`iContentColumn`、`iIndentEndColumn` 给出 MD4C 按四列 Tab 停靠计算的逐层逻辑列：本层消耗前两者之间的列，余量到第三者，内容边界可落在一个 Tab 字节内部。没有实际缩进但由空行规则归属列表的行不产生记录。首行标记和可选任务框后的空白分别在 `GetBlockSyntax` 的 `iListMarkerGapStart/End`、`iTaskMarkerGapStart/End` 中查询；其他块内 trivia 尚未形成完整 CST。

含隐藏链接定义的多层列表/引用中，单个嵌套段落的替换、拆分或合并可使用段落来源补丁。补丁按上述解析器确认的列表标记、任务框和引用前缀构造完整祖先链：首行保留原始标记及空白，后续行将每层列表标记转为等宽缩进、保留引用标记、去掉任务框；Tab 仍按四列停靠计算，只有移除任务框改变其停靠宽度时才用等宽空格替代。行内定界符交由行内写回器处理，隐藏定义与补丁外源码保留原字节，沿用原 LF/CRLF。候选必须重新解析，正文语义及全部链接定义原拼写均匹配才能发布。已有更小的直接源码补丁仍优先使用；同属性相邻 Text 的节点分段可以与完整重解析不同，文字和属性须语义等价。

`xuiDocumentTxnSplitListItem` 与 VISUAL 的列表项 Enter 还提供独立来源补丁：拆分直接位于列表项内的段落/标题时，保留原项开头和拆分段落前的定义/正文，插入相同无序项目符号或相同有序分隔符的新项，新有序编号按列表起始值及项序计算，新任务项清除勾选。松散列表增加必要的空白续行，避免改变列表 tightness；原 LF/CR/CRLF、BOM 和无末尾换行保持。后续块跟随新项，未触及的兄弟标记、实体和定义正文不规范化。有序标记位数改变时，仅增减解析器确认的后续缩进列；保留可保留的 Tab，边界落在 Tab 内时改写为所需空格。候选必须完整重解析并匹配结构目标；缩进未改时核对全部定义原拼写，缩进改变时优先核对受影响链接定义的标签、目标和标题原始字节，其他定义保持完整原拼写。多行字段包含变化的容器前缀时，逐条比较快照缓存的解析器合并行后的原始标签、目标、标题及标题存在标志，覆盖未使用与重复定义并保留实体/转义拼写；不再额外解析或连续化前后源码。困难与普通案例均只解析一次候选，成本仍记录在现有解析统计中。不能证明等价的结构继续走通用路径，并可能原子返回 UNREPRESENTABLE。已覆盖嵌套、有序、任务、空首/尾项、标题、引用式链接及多行链接定义的文字拆分；本条不表示任意定义形式、所有容器编辑或完整 CST 已完成。

链接定义内容缓存是同一 Document state 中的私有不可变派生元数据，由正常 MD4C 解析回调生成，按定义顺序保存完整字节而非哈希；无标题与空标题有独立存在标志，脚注保存对应占位项。SourceStore 仍是唯一可写源码，缓存不另行序列化，加载原生 Markdown 文档时从源码恢复。持久序列与快照/历史共享，普通正文编辑及定义来源偏移沿用未变缓存；单条定义的增量字段编辑仅替换对应内容 blob 和序列路径，复杂定义增删由完整解析生成新缓存。缓存存储纳入当前文档、历史、快照和 Prepare 发布预算的内存归属计费；失败或取消不发布半成品。脚注正文仍受原拼写与现有依赖校验保护，占位项不充当正文等价证明。缓存去掉了额外定义扫描，但结构候选仍须完整重解析，尚不据此宣称大文档结构编辑已满足 UI 响应门槛。

`xuiDocumentTxnSetCodeBlockLanguage` 与 `xuiDocumentEditorSetCodeBlockLanguage` 修改 Markdown fenced code 时，只替换解析器确认的语言 token，保留 info 的前导/分隔/尾部空白与额外元数据、原围栏长度/字符、容器前缀、字面正文及换行。新语言中的 `&` 和反斜杠分别使用实体拼写，避免改变解码后的语言值；UTF-8 语言保留文字。根层、引用、列表和已用脚注内均使用同一来源字段路径。候选须整篇重解析、核对语义以及全部未修改定义原拼写；所选脚注仅允许该语言字段改变正文，其标签和其他定义继续保护。节点身份和共同历史保留，ChangeSet 报告一个语言字段的源码替换。空语言可清除不含额外内容的 token；若保留的元数据会成为新的语言，返回 UNREPRESENTABLE 且不发布，调用方可明确在 SOURCE 中编辑整段 info。Indented code 和缺少可证明字段范围的情况仍走通用结构写回，不把本路径当作完整 CST 或任意围栏转换的验收。

`xuiDocumentSnapshotGetTableToken(snapshot, table_id, index, out)` 按来源顺序查询 GFM/EXTENDED 表格的未转义单元格分隔 `|` 与分隔线中每列的 `-`/对齐冒号串。`iRow=0` 是标题行，`1` 是分隔线，`2+` 是正文行；`iOrdinal` 是该行同类 token 的序号。分隔线 token 的 flags 指明左右对齐冒号，外围空白和 CRLF 仍由原始行分区保留。表格之外或 CommonMark 方言没有这些 token。当前这些接口与原始行分区共同提供部分块内来源定位；围栏语言名后的元数据细分、Tab 的逐层逻辑列界和其余 trivia 尚未形成完整 CST。

`xuiDocumentSnapshotGetBreakSyntax(snapshot, break_id, out)` 对 Markdown SoftBreak/HardBreak 节点返回 MD4C 已确认的换行来源：软换行、反斜杠硬换行、行尾至少两个空格的硬换行，以及解析器配置强制的硬换行。结果给出原行尾水平空白、硬换行标记和 LF/CR/CRLF 的原始字节范围；没有标记时标记端点为 `UINT64_MAX`。源码事务、语义编辑、快照与 Undo 使用同一来源元数据；Rich 文档返回 `UNSUPPORTED`。图片 alt 内的换行不生成独立 Break 节点，不由此接口枚举。

VISUAL 在同一文字节点首尾插入空格、Tab 或 MD4C 识别的 Unicode 横向空白时，会把新插入的边缘标量写为数字字符实体，避免段落缩进或强调定界符的语义变化；CR/LF 仍按结构换行处理。端点可精确映射时，局部补丁保留范围外的实体、换行和引用定义原文；完整重解析不等价时不提交该候选。

跨段落精确文字选区也使用同一边界空白规则：仅将插入文本首尾的 MD4C Unicode 空白写为数字字符实体，普通 Unicode 文字保持原始 UTF-8；候选仍经全文语义核对。此规则覆盖来源精确的文字端点，不扩大复杂容器、不可精确映射端点或完整块内 CST 的可编辑范围。

列表标记与受支持的非段落首块同行时，标记识别使用有限的前缀缓冲区，行结束符则在块来源范围内分块查找。因此围栏信息串或块正文超过该前缀长度时，仍可保留原标记与 LF/CRLF；CRLF 跨扫描块边界也按两个原字节保留。超长标记前空白等无法确认前缀的情况仍不进入这条精确来源路径。

跨父容器的精确文字选区合并也会检查右端段落之前的容器源码。若其间只有完整链接定义与容器空白，补丁把原始字节移到合并段落之后，并在私有候选中核对全文语义及全部定义拼写。右引用块在文字移走后若仍含链接定义，即使没有可见子节点也保留空 Quote，因为同一源码重解析仍会生成该节点；普通空容器仍清理。已覆盖右侧另有后段、只剩前置或后置定义，以及定义被引用的情况。其他复杂语法间隙和完整 CST 尚无通用无损保证。

若右段落之后只有独立的 `>` 标记行和空白，合并后也保留空 Quote，以匹配 Markdown 对这些未选中来源字节的解析。没有定义或尾部标记的普通空 Quote 仍会清理；判断不了来源边界的操作仍按全文重解析结果回退或拒绝。

右侧唯一可见段落位于列表项内连续的 Quote 链、段落之后仍有裸引用标记行时，精确补丁可从被替换行中提取原列表与多层引用前缀，在合并段落之后补出空列表项的首行；原尾行保持原字节，新增首行沿用原 LF/CRLF。补丁要求目标 Quote 链只剩空容器，且原前缀中的 `>` 数量与 Quote 层数一致。测试覆盖一至三层 Quote、无序与单数字有序列表标记、引用块内的隐藏链接定义；实现保守接纳最多九位数字，候选仍需全文重解析、语义与定义原文核对。嵌套列表、任意容器前缀及完整 CST 尚无通用无损保证。

若 Quote 位于多层列表中，精确补丁可移动被选中文本之前的列表来源路径，包括原始空行、分行的列表标记与引用前缀。父列表项若有被选区完整覆盖的前导段落，则按列表标记与段落行结束符删去正文及行内定界符，保留原标记和换行；列表中被完整选中的目标前可见兄弟项按原语法范围跳过，只含链接定义的空兄弟项则保留其容器和定义原文，目标后的兄弟项保持原字节。若前置项有一个或多个可见块，以段落开头并在末块的完整空白间隙之后包含链接定义，删除这些块时保留首段落的列表标记与行结束符、尾随定义原文，同时删去会让列表分裂的空白行并更新列表的 tightness；支持多条 LF/CRLF 空行、含空格/Tab 的空白行、两段落及段落加围栏代码块。项内若有更早的隐藏定义则不使用这条精确路径。定义紧随可见正文而没有空行时属于可见段落，随选区删除。路径必须能按结构与来源范围明确证明，候选仍经完整解析、语义及定义拼写核对。两层/三层列表、可见父项、有序标记、CRLF、隐藏定义和前后兄弟已有回归；前置兄弟项的首块非段落、非空白来源间隙与完整 CST 仍无通用无损保证。

父列表项若有被选区完整覆盖的可见块，然后才是通往右端段落的内层列表，跨父范围替换可删除这些前导块并保留原列表标记、内层列表前缀与未选中的后继兄弟。首块为段落时保留原标记和段落行结束符；非段落首块现覆盖围栏代码、Quote、标题、分隔线、原始 HTML、GFM 表格和 Mermaid 围栏。独占一行的列表标记按原字节保留，与块起始同一行的标记则只保留标记及原首行 LF/CRLF。只有每个块的来源范围有序、块间只含空白和解析器记录的完整 LINK 定义时才使用该精确路径；定义原字节移到保留的父项标记之后，略去会拆分列表的空白，候选仍须全文重解析并与目标语义和定义原文核对。删除前导块后若空父项由 loose 变为 tight，结构目标同步更新。已覆盖两段落、段落加围栏代码、上述首块类型、已使用/未使用定义、双定义、跨行标题定义、CRLF 有序列表与前后兄弟；未验证的首块类型、非 LINK 定义及完整 CST 仍需逐项补齐。

Markdown VISUAL 的后台待提交输入现覆盖段落/标题内单一文本叶节点及相邻同格式文本叶节点的精确来源范围。加粗、斜体、删除线、高亮、上下标和目标一致的链接标签文字可先在私有语义快照中显示输入，同组光标处继续输入；worker 重解析源码补丁并核对完整语义后才发布。带格式选区内若含原始定界符、转义符或可能改变行内语法绑定的来源字符，则不建立该快速候选，交给同步结构回写路径。行内代码、跨格式或跨块选区以及不精确来源仍不属于这条异步路径。

Markdown 表格中显式写出的空 Cell 有解析器提供的零宽源码锚点；语义 GAP 映射到该锚点时返回 `SYNTAX`，两种亲和性均可从源码位置回到同一 Cell。对此 GAP 输入文字时，VISUAL 编辑优先在该锚点插入已转义的 Markdown 字节，并用私有重解析核对完整语义及引用定义拼写；成功时保留该表其他单元格、分隔线、空格和 LF/CRLF 的原始字节。锚点位于原空白之后时，新文字也插在原空白之后。短行中为补齐列数而合成的 Cell 没有独立源码锚点，其语义 GAP 会沿同级与祖先寻找邻近来源边界，返回 `APPROXIMATE`；此时 VISUAL→SOURCE 仍有合法落点，但再次切回不保证回到原 Cell。非空文本原有的精确片段映射不受影响。

短行补齐的合成空 Cell 虽无原始锚点，在其语义 GAP 输入文字时可按该行真实的结尾分隔竖线，只向这条物理行补出缺少的空 Cell 和目标内容。输入的 Markdown 标点先转义，候选重解析并核对完整语义与引用定义原文后才发布；其他行、分隔线和原有尾随空格保持原字节。成功后目标 Cell 成为有源码范围的真实 Cell，Undo 恢复原短行。无法证明行边界或候选语义时仍回退通用结构路径。

视觉回写使用共同的结构写回器：先在私有语义树执行并记录原生操作，再生成受影响顶层块的源码补丁，统一解析一次并比较完整语义。核对成功后保留命令树的身份和文本分段，从解析树投影新的源码范围；空文本等被规范化掉的节点也记录删除。源码编辑重新解析时，语义未变化且已匹配的子树继续保留原有分段，避免后续 SOURCE 编辑抹掉视觉编辑建立的身份。段落/标题、嵌套列表与引用中的跨 run 替换、段落拆分/合并、marks、可表达的节点移动/删除，以及链接、图片和代码语言等资源属性修改已接入。代码块、图片、公式、HTML、脚注等节点可随所在结构一起回写，但仍须通过语义核对。

跨文本叶节点的 `ReplaceRange` 在两个端点都能精确映射源码时，先尝试只替换端点之间的原始字节。两个端点可位于同一段落/标题，或位于同一父容器下连续的段落/标题。跨块路径接纳纯空白块间来源；引用块的空行也可含 `>` 前缀。根层、列表项或引用块的相邻段落之间若只有完整的链接引用定义和容器空白，还可将这些原始字节移到合并后的段落之后。脚注正文的跨段落编辑同样可走精确来源补丁：脚注定义的正文属于被编辑范围，核对时保留其标签与来源前缀，并严格核对其他所有定义的原始拼写。私有候选仍重解析并核对整篇语义，成功后才接纳；语法绑定不同则回退共同结构写回器，内存/取消/限制错误直接返回。该路径能保留选区外的实体、引用定义和换行原拼写，覆盖正反向选区、删除、转义字符插入及跨多个段落合并。

不同父容器的文字选区按两端段落/标题的最近公共块容器处理：左端以前、右端以后的兄弟子树保留，两端之间的完整子树删除；右段落的剩余行内节点移入左段落，再使用已有合并操作。两端可位于不同顶层块，也可位于同一列表的不同列表项或引用块内的不同分支；穿越表格行列时明确拒绝，避免破坏单元格结构。Markdown 对精确端点先尝试局部源码补丁；根层两个顶层块之间、以及左端引用块/列表项等容器尾部的完整链接定义及空白会按原字节移到合并块之后。候选必须完整重解析、语义相等且所有定义保持原拼写才提交。这条路径覆盖端点容器中未选中的前后段落、被完整选中的中间块、反向选区以及选区外的实体与换行。非文字端点的复杂结构选区、端点映射不精确、其余容器内部隐藏定义及复杂语法间隙仍没有完整无损保证；完整块内 CST 尚未实现。

GFM/EXTENDED 的 `InsertTable`、`InsertTableRow`、`DeleteTableRow`、`InsertTableColumn`、`DeleteTableColumn` 已接入同一写回器；支持一个事务内连续操作，保留列对齐，首行变化时维护表头。Markdown 表格要求第一行为表头、无合并跨度、单元格为单段内容。Rich 的合并/拆分单元格能力保持不变。通用 `InsertNode` 仍用于 Rich 文档，Markdown 不支持逐节点搭建未完成结构。

VISUAL Editor 的 `xuiDocumentEditorInsertTable(editor, rows, columns)` 在折叠光标处插入表格：段落起点/终点直接插在段落前/后，段落中间先拆分段落，再把表格放在两段之间；空容器 gap 可直接插入。插入和必要的段落拆分共用一次 Undo，随后光标进入首格。Rich 与 GFM/EXTENDED 使用同一接口；CommonMark 无表格语法时明确返回 `UNREPRESENTABLE`。跨块选区、SOURCE/LIVE 模式与只读 Editor 不执行该操作。

被重写的顶层容器允许规范化拼写；其他源码及被修改顶层块之间的空白、链接引用定义保持原字节。普通 `ReplaceRange` 在单独列表项末尾增加空段、单元格多段及某些 delimiter 组合仍可能返回 `UNREPRESENTABLE`。正式的 `SplitListItem` 命令将尾段和后续块移入新兄弟项，`ExitListItem` 将空项移出当前列表；VISUAL Enter 根据列表项内容选择命令。`IndentListItem` 把当前项移入前一兄弟项的子列表；`OutdentListItem` 把嵌套项提升一级，并把后续子项留在被提升项下；顶层项提升为普通块，必要时拆分前后列表。有序列表在拆分后延续编号。同一列表的非折叠选区可用 `IndentListRange` / `OutdentListRange` 一次调整两端之间的全部列表项，并保持原选区方向。同一父容器下相邻的不同列表也可作为一个范围调整：空列表项包含在内，缩进将各组选中项放入前方保留项的子列表，提升从后向前处理兄弟组以维持顺序；有序列表移走前缀后顺延剩余项的起始编号。VISUAL 的 Tab / Shift+Tab 对这些选区调用对应命令；SOURCE/LIVE 保持源码输入与焦点处理。Markdown 回写后重解析验证完整语义；无法表达的结构原子拒绝。夹杂普通段落或跨不同父容器的列表范围仍明确拒绝。重写单个引用块或列表时，写回器提取各项内定义所在的原始语法空隙，保持链接引用定义原字节与全局顺序，再重解析核对完整语义和所有定义原文。非任务项的首个空隙若含定义，保留原列表标记并将定义移到正文之前；定义在后续空隙时可保留生成的列表标记或任务复选框，并将原始定义空隙移到该项正文之后。嵌套列表/引用中单段文字修改、拆分或相邻段落合并可只补丁受影响的段落语法范围，保留外围定义及其他源码；候选仍需整篇语义和定义原文核对。加粗、斜体和链接段落也按行前缀与行内语法分离后局部回写；两段间的引用定义可按原字节移到合并段落之后；含其他非空白来源的合并、其余嵌套结构重写及无法保持合法列表结构的操作仍原子拒绝，直接正文补丁与 SOURCE 编辑可用。

`xuiDocumentTxnSetHeading(range, level, after)` 将语义选区中的段落和标题统一转换：`level=0` 为段落，`1..6` 为对应级别标题；保留节点身份、行内内容、marks 和原选区方向。它接受普通跨块选区以及根或其他容器上的完整子块结构选区；可插入标题的折叠容器 gap（包括空文档根位置）会创建空标题。富文本在事务树中直接改变块类型；Markdown 对候选树统一回写并重解析核对，结构选区跳过表格内部段落以保留 GFM 表格。VISUAL Editor 提供段落和六级标题命令以及选区混合状态，Ctrl+A 后的根结构选区也可执行；SOURCE/LIVE 不执行该视觉格式命令。

`xuiDocumentTxnCreateListRange(range, flags, start, list_id, after)` 把同一父容器中连续的段落/标题变成一个列表，原块与行内 NodeId 保持不变；折叠的可插入容器 gap 会创建一个空项。`flags=0` 为项目符号，`XUI_DOC_ORDERED` 为有序列表（`start>=1`），`XUI_DOC_TASK` 为任务列表；任务列表在 Markdown 中要求 GFM/EXTENDED。完整块结构选区和反向选区保持方向，整个操作只占一步 Undo。Markdown 候选会经源码回写和语义重解析校验；跨父容器、夹杂非段落块的范围拒绝执行。VISUAL Editor 的 `BULLET_LIST`、`NUMBER_LIST`、`TASK_LIST` 命令以及 Toolbar 列表组共用该事务路径。

VISUAL DocumentEditor 的任务列表复选框可用指针点击：按下和松开必须命中同一个复选框，移出后松开不会切换。折叠光标位于任务项中时，`XUI_DOC_EDIT_TOGGLE_TASK` 可查询勾选状态并切换；宿主也可用 `xuiDocumentEditorToggleTaskItem(editor, item_id)` 指定任务项。三种入口及辅助技术的 Toggle 动作共用 Editor 事务与一步 Undo，保持原选区；Markdown GFM/EXTENDED 只改对应任务框的来源字节，Rich 直接修改节点属性。只读以及 SOURCE/LIVE 模式禁用这些 Editor 入口；Document 事务 API 仍可由宿主直接使用。

已有列表可用 `xuiDocumentTxnSetListStyleRange` 转换选中的连续项，必要时拆成原样式前段、新样式中段、原样式后段；有序列表后段延续原编号，原有块、列表项和行内节点身份不变。跨多个列表转换为有序列表时，每个选中列表段分别从传入的 `start` 起号。`xuiDocumentTxnUnlistRange` 把所选项的子块提升到列表父容器，前后未选项保持列表结构。Markdown 中没有语义正文的空项（包括空任务项）直接消失，来源回写后不残留空段落；富文本空项仍产生可编辑的空段落。两条命令接受单个列表内，或同一父容器下连续相邻兄弟列表之间的文字、列表项和完整列表结构选区；完整列表可用父容器上的 gap 选取，取消列表后返回的 gap 覆盖提升出来的块；若没有可提升的块，返回折叠的父容器 gap。嵌套列表的直接 gap 选区按子列表处理，不会提升外层列表项。中间不得夹杂非列表块，跨父容器仍拒绝。反向选区与一步 Undo 保留；Markdown 回写后重解析核对。Editor 列表按钮显示当前类型的激活/混合状态；选中已有列表项时点击另一类型执行转换，再次点击已激活类型则取消列表。

`xuiDocumentTxnMoveBlockRange(range, down, after)` 将同父容器的连续块或列表项上移/下移一位。文字光标在列表项内移动该列表项，在表格单元格内移动该单元格的段落；父容器 gap 可选中多个完整兄弟块，包括列表、引用和表格。存续节点 ID、文字选区方向和结构选区范围保持，边界或跨父容器范围明确拒绝；Markdown 候选回写后重解析核对。VISUAL Editor 提供 `MOVE_BLOCK_UP/DOWN` 命令与 Alt+上/下方向键，只读及 SOURCE/LIVE 模式禁用。

`xuiDocumentTxnSetBlockStyleRange(range, fields, style, after)` 对富文本选区内的段落/标题一次应用对齐与段后间距，保留块和行内节点身份；空容器 gap 会创建带样式的段落。`XUI_DOC_SPACING_EXPLICIT` 区分“显式零间距”与“使用 Renderer 默认间距”，序列化/加载保留该标记。VISUAL Editor 提供左/中/右/两端对齐命令与状态，以及任意非负间距的查询/设置和恢复默认。Markdown 正文没有这两类属性的无损语法，命令明确禁用，直接事务返回 `XUI_DOC_ERROR_UNREPRESENTABLE`；GFM 表格列对齐仍由表格属性处理。其他块种类转换及夹杂普通块、跨不同父容器的列表混合选区尚待补齐。

VISUAL Editor 的行内命令现覆盖加粗、斜体、删除线、下划线、代码、高亮、上标和下标，非折叠选区可查询 active/mixed 状态。富文本可使用这些 marks；Markdown 按所选方言启用可表达的命令：下划线不启用，CommonMark 不启用删除线，高亮/上标/下标仅限 EXTENDED。切换上标和下标会清除另一种 mark；Markdown 代码命令会清除与代码语法冲突的其他 marks。EXTENDED 的行内定界符有来源范围，整段清除优先删除原定界符；结构回写后仍重解析并核对语义。单块回写保留原块末尾 LF/CRLF 数量，避免撤销后再次格式化时多出空行。Renderer 提供可配置的高亮底色，并调整上/下标的字形大小与基线；完整的 Markdown 编辑器命令集仍待实现。

`xuiDocumentTxnSetTextStyle(range, fields, style)` 对富文本非空语义选区一次修改文字颜色、底色、字号和字体族；颜色零值在 `iExplicitFields` 中设置对应位时表示显式透明黑，不设置则清除该颜色并恢复 Renderer 默认值；字号零值/空字体族恢复默认，未选中的字段保持原样。跨 run/段落时拆分边界并保留未选中文本属性。`SnapshotQueryTextStyle` 返回光标上下文或选区首个 run 的属性及逐字段混合状态，`iExplicitFields` 标出颜色/底色是否显式设置（包含零值）；VISUAL Editor 的 `QueryTextStyle` / `SetTextStyle` 对选区形成单步撤销，折叠光标维护后续输入的待用样式，可与待用 marks 同时应用。`iCurrentColorFields` 可给文字色和底色指定 `currentColor`；它与同字段的 `iExplicitFields` 互斥，颜色输入值须为零，查询保留关键字状态。字号参与原生字形度量，颜色/底色参与绘制，HTML 导出包含四种属性。自定义字体族由 Renderer 的 `onFont` 回调解析；宿主没有提供解析器时使用默认字体。Renderer 与 SnapshotQueryTextStyle 使用同一套祖先样式解析：文本节点未设置颜色、字号或字体族时从最近祖先取值；未显式设色的链接仍使用主题链接色。段落底色按块绘制，渐进排版只覆盖已确定的行，表格选择色位于段落底色之上。Markdown 不能无损表示这些富文本属性，编辑器禁用命令，直接事务明确返回 `XUI_DOC_ERROR_UNREPRESENTABLE`。

Rich Document 在纯文字/段落颜色属性变化且不拆分节点时，Renderer 保留已测量块的 shaping、断行和光标几何，只刷新 run 的有效前景/底色与段落背景框颜色；背景框在无底色时也保留几何，因此后续启用底色不需重排。Text run 内的局部选区改色仍会拆分节点并重新排版受影响块，这一性能要求尚未完全验收。

`xuiDocumentTxnClearFormatting(range)` 在同一语义事务中清除非空选区内的行内 marks、链接目标/标题、文字颜色/底色、字号和字体族；块样式与嵌入对象保持不变。已无行内格式的片段不拆分，也不产生历史步骤。`XUI_DOC_EDIT_CLEAR_FORMATTING` 在 VISUAL Editor 中既支持选区，也支持折叠光标，富文本和 Markdown 共用。选区操作保留未选中片段格式，成功形成一个 Undo 步骤；光标操作只设置待用清除状态，不改变 revision，后续文字输入在同一输入事务中清除继承的格式。跨行输入通过事务位置映射限定新插入的多节点范围；纯换行不执行空范围清除，待用状态留给后续正文。单行及跨行输入都可继续叠加用户显式设置的待用 marks，富文本还可叠加文字样式；清除、marks 和样式在一个根事务中提交。移动光标或改变投影会清除待用状态。Markdown 在语义变更后回写源码并核对结果，无法表达的复杂组合会原子拒绝。复杂来源语法及真实平台输入仍待验收。

`xuiDocumentTxnSetLink(range, uri, title)` 在一个根事务内拆分选区边界、设置或清除链接 mark 与目标资源；空 URI 解除链接并清除原目标/标题。`TxnInsertLink(range, label, uri, title, caret)` 可以在折叠光标插入自定义单行文字，也可用链接文字替换选区；文字与链接格式在同一次根事务中提交，返回最终光标。`SnapshotQueryLink` 返回全选区链接状态、混合状态与统一目标，字符串借用自所查询的 Snapshot。VISUAL Editor 的 `InsertLink` 接受自定义文字，`SetLink` 在折叠光标处以 URI 作为显示文字，均形成单步撤销。通用 `SetMarks` 不接受 LINK 位，避免只改变 mark 却遗留或缺失目标资源。Markdown 对语义结果回写并重解析核对；完整链接解除优先删除来源定界符，单个精确文字片段创建链接时优先做局部源码补丁，保留同段未选中的实体与 CRLF。复杂跨 run 的链接由结构写回器规范化，不能无损表达时明确拒绝。Renderer 默认将链接绘成带下划线的主题可配置颜色；View 命中可通过 `onActivate` 把目标交给宿主，宿主自行决定打开或处理。链接编辑的完整弹窗和全部 CST 保真仍属后续工作。

`XUI_DOC_LIVE_MARKDOWN` 已实现块级语法显隐：光标所在的顶层块显示原始源码，其余块使用原生排版。列表、引用和表格以整个顶层容器为激活单位；不创建第二个 Document。该模式的命中、选区、复制和输入采用 SOURCE 位置，格式工具栏命令目前仅用于 VISUAL。`xuiDocumentRendererSetActivePosition` 供无控件宿主指定活动位置，View/Editor 自动跟随光标。

`xui_doc_node_info_t.iSourceStart/iSourceEnd` 记录正文来源，`iSyntaxStart/iSyntaxEnd` 记录解析器提供的块语法范围（半开 UTF-8 字节区间，含标记和结束行换行）；缺少范围时为 `UINT64_MAX`。后者覆盖空标题、分隔线、空引用/列表项、完整围栏、Setext 下划线、front matter 和脚注定义，不再由正文范围反推块边界。保留语法边界的空节点也可以转换光标位置。

`iSourceSegmentCount` 和 `xuiDocumentSnapshotGetSourceSegment` 提供节点正文的有序来源片段：DIRECT、ESCAPE、ENTITY、NORMALIZED、SYNTHETIC。片段将语义 UTF-8 区间映射到原始源码区间，随快照持久化；语义节点保留原身份或重新分段时，也投影这些来源。一个实体解码为多个码点并跨文本节点时，用 PARTIAL_START / PARTIAL_END 明确标记边界不完整，避免伪造精确偏移。

`xuiDocumentSourceToPositionEx` 接受 BEFORE/AFTER 亲和性。EXACT 表示可精确定位的字节或完整解码边界；COLLAPSED 表示实体/转义/换行归一化片段内部折叠到边界；SYNTAX 表示非正文区域向内容边界投影；APPROXIMATE 表示来源信息仍不足。原 `SourceToPosition` 默认 AFTER。视觉文本编辑优先使用精确片段边界生成局部补丁，例如修改 `a&amp;b` 中的 `b` 可以保留相邻实体的原拼写。

`xuiDocumentSnapshotGetSourceInfo` 查询行内语法和引用定义的数量；`GetInlineSyntax` 返回语法种类、嵌套父索引、开始/正文/结束字节范围和所用引用定义索引。索引只在当前快照来源树中有意义，不是语义 NodeId，缺失关系为 `UINT64_MAX`。范围直接来自解析器回调，覆盖拆分的 `***`、下划线、删除线、多反引号及裁剪空格、链接/图片、自动链接、公式和脚注引用。来源层随 SOURCE 编辑、语义投影、Undo/Redo 一起替换，旧快照不变。

`GetReferenceDefinition` 保留完整定义范围（含容器前缀和脚注续段）及 label、destination、title 的原始范围。`iKind` 区分链接定义和脚注定义；脚注没有 destination/title，这四个端点均为 `UINT64_MAX`。链接未提供 title 时为 `UINT64_MAX`，显式空标题保留零长度范围。重复定义和未使用的脚注定义也保留；链接/图片和已解析脚注引用的 `iDefinitionIndex` 均指向解析器实际选用的定义，支持标签大小写折叠，不在 XUI 中另行猜测匹配。`GetReferenceCandidate` 查询目前查无定义、但语法有效的链接/图片/脚注标签；返回记录的 `source` 是待查的方括号范围，`content` 是原始标签范围。完整引用写法可能有多种回退查询，因此不同类型的候选可以共用同一源码范围；相同记录去重。候选随快照、Undo、源码局部重解析及后缀位移保持一致；独立链接定义目标或标题字段内的替换、删除与插入，在定义未被使用或只影响一个独立文本块且引用额度安全时可局部解析，标签改变、跨块使用和脚注依赖仍回退完整解析。

清除整段或多个完整行内范围的一种格式（加粗、斜体、删除线、代码标记，以及 EXTENDED 的高亮、上标、下标），优先删除已记录的定界符，保留正文中的实体、转义、引用拼写、CRLF 及容器内部定义；跨行选区同步修改换行节点的格式。源码候选只解析一次，并与命令生成的完整语义树核对后接纳。局部范围或去除代码标记后会形成新语法时，候选不发布，交由结构写回器处理。仅清除 `__alpha &amp; beta__` 中 `alpha` 的加粗时，可把选区末字母及保留加粗区的首空格写成数字字符引用，得到语义等价的 `alph&#97;__&#32;&amp; beta__`。同样支持仅清除 `__alpha beta__` 中 `beta` 的加粗，把保留加粗区的末空格与普通区首字符写成数字引用；清除中间词时右侧加粗区改用另一种 strong 标记，避免定界符重新绑定。斜体、GFM 删除线以及 EXTENDED 的高亮、下标、上标也可用同一局部路径清除测试中的首段、末段与中间段，并保留未选中 `&amp;` 的原始拼写及原定界符。所有局部候选均须通过完整语义核对；已覆盖 `**`、`_`、Unicode、连续空格、CRLF、引用定义和 Undo/Redo，更复杂的交错 marks 拼写仍可能被拒绝。

对于 `***alpha &amp; beta***`，斜体来源范围包裹着加粗范围；清除 `alpha` 或 `beta` 的斜体时，局部补丁会识别完整包裹的子定界符，保留加粗和未选中实体，并只移动外层斜体定界符。只清除中间 `&` 的斜体时，局部补丁把加粗定界符移到外层，将斜体拆成两段，选中部分的 `&amp;` 或 `\&` 源码拼写原样复制；`***` 与 `___` 均有精确源码和 Undo/Redo 测试。这些候选都须重新解析并与目标语义完全一致后才提交。其他复杂交错和块内来源无损回写仍需继续实现。

目前仍不是完整 CST；块内缩进/表格等独立 token、未解析引用的跨块依赖失效、扩展语法交叠来源及完整无损结构回写仍待补。LIVE 使用块语法范围，脚注按来源顺序参与分区。652 项 CommonMark 语料已消除全部 40 项整篇源码回退，并对 15,470 个 UTF-8 边界验证光标几何。扩展语法出现重叠或缺失范围时仍保留安全回退，`bLiveSourceFallback` 报告回退，`GetActiveSourceRange` 查询实际范围；这不等同于完整 CST 或 HTML/排版规范验收。

源码编辑在独立顶层块的边界可证实时使用局部解析，其他情况回退全量解析。SOURCE 与 LIVE_MARKDOWN 编辑器在源码或本次输入达到默认 100 KiB 门槛时使用 prepare 工作线程；小文档和大部分 VISUAL 输入仍同步；大 Markdown 文档中普通段落/标题单文本叶节点内部、来源位置精确的连续输入可以先显示语义候选，再由 worker 解析。LIVE 候选在编辑仍局限于活动块时显示该块的新源码，其他块保留已提交语义排版；跨块候选临时显示完整候选源码。源码补丁先批量应用，再对最新候选解析一次；诊断统计提供解析次数和扫描源码字节数。

## Markdown prepare 与发布

`xuiDocumentPrepareSource` 在 Document 所有者线程创建唯一待提交候选，接受一批顺序应用的 UTF-8 字节补丁。偏移从已提交版本开始，后一个补丁相对前一个补丁执行后的源码；可以用 `xui_doc_txn_desc_t` 指定 base revision、origin 和 group，domain 必须为 SOURCE。调用复制补丁输入并构建持久化候选源码，不执行解析。成功创建新候选才会淘汰旧代次，创建失败保留旧候选。

后续按键使用 `xuiDocumentPrepareContinueSource(document, previous, patches, count, &next)`：`previous` 必须仍是该 Document 的当前候选，补丁偏移从其待输入源码开始。新候选继承 base revision、origin、group 和此前所有补丁，源码和操作日志共享持久化存储；创建时不复制整份源码或累计操作数组。操作数组在 worker Run 时展开，最终所有输入仍通过一次根事务发布和共同历史生效，遵循正常分组/预算规则。`iPatchCount` 为跨代次累计补丁数，包括没有产生变化的调用补丁。

延续 QUEUED、RUNNING、READY 候选均可；它只读取前一代的不可变源码和日志，不读取正在构建的语义树。未取消的 Run 失败候选也可继续修改或用零补丁重试；已经取消、被替代、发布或因外部提交失效的句柄不能延续。创建失败保留旧候选；成功才向旧代次发出 STALE。持久化日志避免了逐键复制全部先前操作，但最终解析、调和及展开日志仍有相应成本。

Run 或 Publish 的非取消/过期失败会保留当前候选，SaveFile 继续返回 BUSY。失败的代次本身不能重新 Run，通过 ContinueSource/StreamSource 创建新代次重试或修改；明确放弃使用 CancelPrepare。这使解析或发布分配失败后的输入仍可显示、复制和恢复。

`xuiDocumentPrepareCopySource` 可在工作线程解析期间读取不可变的待输入源码；`xuiDocumentPrepareReadSource` 支持无分配的字节范围读取，不添加终止符，并允许读取 UTF-8 字符内部的字节切片。候选是带 document ID、base revision 和 generation 的输入投影，不包含可写语义树或独立历史。普通 Snapshot、其他视图和保存点继续表示最后一个已提交版本。

`xui_doc_position_t.iInputGeneration` 为零时属于已提交内容；非零时仅用于对应候选的 SOURCE 位置。`xuiDocumentPrepareSourcePosition` 构造并校验该位置；Snapshot/Transaction API 拒绝候选位置，Renderer 拒绝来自其他代次的位置。输入发布后 Editor 将选区转换为新 revision 的已提交位置，撤销书签保存已提交位置。不能只改 revision 把候选光标当作普通文档位置。

宿主将候选派发给自己的执行器，在 worker 调用 `xuiDocumentPrepareRun`，完成后通知所有者线程调用 `xuiDocumentPreparePublish`。Run 是调用线程上的同步函数，内部不创建线程；每个候选只能运行一次。派发前 Retain，在 worker 返回后 Release，所有者保留另一份引用到处理结果结束。`GetInfo` 的 QUEUED / RUNNING / READY / PUBLISHING / COMMITTED / FAILED 状态可跨线程查询；取消请求另有标志，运行中的取消会在正常清理完成后报告失败。

后台 builder 只持有不可变输入、共享分配器和私有事务，没有 live Document 指针。分配器中的原子 NodeId 序列保证后台候选和普通事务同时构建时不复用编号；放弃候选消耗的编号不会回收。取消在解析检查点、解析器分配和语义树遍历中协作退出，不承诺单次扫描、排序或释放的硬实时期限。自定义分配器须能处理 worker 与所有者线程的并发分配/释放，用户上下文必须比全部候选和快照活得更久。

Publish 核对文档身份、base revision 和当前代次，再走现有根事务的原子发布点。源码、节点树、来源元数据、位置映射、共同 Undo 与通知一起生效；没有第二套提交协议。分配失败、取消或过期均不发布半成品。通知始终在 Publish 的所有者线程发生；已开始发布的结果不再被晚到的取消撤销。

Run 还会预建 ChangeSet、历史记录以及存储计费变更表：只读保留的历史内容，在私有表中计算提交后的共享可达存储与预算裁剪。提交基线仍在已计费历史内时，准备阶段读取受锁保护的当前/历史可达计数，只沿新增、移除及所有者类别变更的持久树路径推演，未变共享子树停止遍历；没有这个基线时使用完整计费。候选不修改 live 计数，读取单项计数只短时持锁。若发布时历史栈、步数/字节额度仍与捕获时相同，内核不再分配或遍历源码/节点树，而是应用按地址排序的计费更新并切换版本。`iPreparedPublishes`、`iPreparedStorageUpdates` 和 `iPreparedAccountingVisits` 分别记录预计算发布、实际更新项和成功计费计划的可达访问次数；首次建立历史或重建大面积内容仍可能产生大表。

准备期间调整额度或清空历史不会丢弃输入，但会使计费计划失效，Publish 回到普通提交路径重新核算。若这类变化与增量计费重叠，后台准备也可使用完整计费重算，保留可发布的候选。两个路径均保留同样的预算、共同历史和原子失败规则。保留的历史记录使用引用计数，使工作线程能安全读取已被所有者淘汰的历史内容；这些暂存引用不是另一套可写历史。

| 动作 | 当前内核规则 |
| --- | --- |
| PrepareSource | 从已提交源码新建候选，成功后旧候选收到 STALE |
| PrepareContinueSource | 从当前候选源码继续应用补丁，保留原始提交基线和此前输入；成功后旧候选收到 STALE |
| PrepareStreamSource | 从当前候选追加流式字节；不完整字符暂存，创建失败保留全部此前输入 |
| 普通事务 / Undo / Redo | 成功发布后淘汰待输入；失败或 Abort 不影响候选 |
| PreparePublish 遇到现有 writer 或通知回调 | 返回 BUSY，保持候选可重试 |
| PrepareCancel(handle) | 跨线程请求取消，不等待、不自动解除 Document 的待提交引用 |
| CancelPrepare(document) | 所有者线程解除待提交引用并请求取消；writer/通知期间返回 BUSY |
| SaveFile | 存在待提交候选时返回 BUSY，由上层选择先发布或明确取消；显式 SnapshotExportFile 仍导出指定快照 |
| 释放 Document | 解除候选引用并取消；worker 持有的候选/输入仍安全，宿主负责等待自行派发的 worker 后再卸载库 |

没有变化的候选可以完成发布，但不增加 revision、通知或 Undo。显式 origin/group 的历史合并沿用根事务规则。候选、计费缓存及已淘汰历史的暂存引用计入内存诊断的 OtherBytes；历史 64 MiB 额度不包含这些待提交或外部保留存储。

最后一次 PrepareRelease 可能释放大量候选或已淘汰的图。需要控制 UI 延迟的宿主，应保留自己的候选引用，在发布/取消后把该引用的最终释放交给执行器；内核不会自动启动回收线程。创建新代次前直接丢掉所有宿主引用，会使 Document 解除旧候选时承担最终回收成本。验证样本分别计时准备、发布和 worker 回收；10 MiB 局部修改的回收仍约数十毫秒，不能忽略。

### 流式 UTF-8 输入

`xuiDocumentPrepareStreamSource(document, previous, desc, utf8, bytes, final_chunk, &next)` 为追加流提供同一套候选机制。`previous == NULL` 时从已提交源码末尾开始，允许传入 SOURCE 事务描述；存在 previous 时继承原有描述，desc 必须为 NULL。每次追加包含此前未完整的字符字节，在候选内最多保留三个尾字节。完整部分进入持久化 SourceStore，尾字节不进入解析器或语义树。

`GetInfo.iBufferedUtf8Bytes` 报告尾字节数，`iSourceBytes` 和 Copy/ReadSource 只覆盖完整部分；两者之和受 Document 字节上限约束。UTF-8 过长编码、代理区、超出 U+10FFFF 和已经非法的不完整前缀立即拒绝，整次调用不接受任何字节。`final_chunk != 0` 要求调用结束时没有残缺字符；可以传零字节确认流结束，截断时返回 UTF8 错误并保留旧候选，允许补齐或明确取消。

含尾字节的候选保持 QUEUED；Run、Publish 和普通 ContinueSource 返回 BUSY，宿主应继续 StreamSource 补齐。SaveFile 同样保持 BUSY。不能把“完整部分可显示”当作整次输入已提交，也不能静默丢掉尾字节。显式 CancelPrepare 或从已提交版本重新 PrepareSource 会按既有规则丢弃旧候选。

宿主可以连续接收分片形成最新候选，每帧/批次仅在 `iBufferedUtf8Bytes == 0` 时派发最新代次的 Run。提交后下一批以 previous == NULL 开始，使用相同 origin/group 可按根历史规则合并流式撤销。StreamSource 生成零到两个补丁，累计 `iPatchCount` 不是网络分片数。内核没有计时器、worker 队列或独立流式历史；最终回收仍需交给宿主执行器。

流式 UTF-8 缓冲已接入 SOURCE 和 LIVE_MARKDOWN Editor 的 `xuiDocumentEditorAppendStreamSource`。该接口从 Markdown 源码末尾追加；完整字符立即投影到编辑器候选，残缺 UTF-8 字节只在 Prepare 内缓冲。LIVE 的末尾块显示候选源码，其余块保留语义排版；若不能保持同一活动块则临时显示候选全文源码。`Flush` 可以发布完整帧，流仍未结束时返回 BUSY；各帧使用同一 Undo 组。流期间 SaveFile、模式切换和其他编辑保持 BUSY，收到的交互事件按顺序排队，最终分片提交后才回放。`CancelInput` 丢弃尚未发布的分片和队列并解除屏障，已发布帧保留，可通过 Undo 撤销。VISUAL 已有受限的同叶节点连续插入、删除及选区替换候选；同一无格式段落内跨纯文本和软换行的选区也可异步替换，并在原编辑光标继续插入。其他跨叶节点编辑、结构编辑和流式投影仍未完成。多类独立顶层块已接入以下增量路径。本机 1/10 MiB 的 SOURCE 准备/发布样本不代表完整 UI 响应验收通过。

### 独立块增量解析

同步 SOURCE 事务和后台 PrepareRun 共用增量入口。存在其他可复用块、全部待解析补丁都落在同一个顶层块内、前后由不变空行或文档边界隔开时，只展开并解析该块的源码。当前可处理段落、标题、规则线、围栏/缩进代码、Mermaid、HTML、引用块、列表和表格；新结果仍须是一个完整的受支持块。文档已有全局定义、但编辑不触及任何定义时，目标块没有 `[` 可直接复用旧定义表；若含其他方括号引用写法、目标块新源码不含 `[^` 且旧块没有已解析脚注引用，则将未变的链接和脚注定义原始行按原顺序附加到局部解析输入，验证解析出的全部定义位置与旧记录逐项对应，再保留旧表并平移后续定义的来源坐标。局部与整篇解析器的引用展开额度也需同时满足保守上界，否则回退全量。围栏/HTML/容器类块另外按最多 64 KiB 的窗口重解析左右相邻块，对比其语义、来源范围及片段，确认没有吞并或改变邻块。列表等容器的语法范围可能已经包含分隔空行；仅确认该行确实空白时接受紧邻下一块的结束位置。无法证明边界独立、末尾围栏未闭合或邻块过大时回退完整解析。

含脚注定义的文档，若编辑块与脚注用法无依赖，方括号链接仍可使用未变定义的虚拟后缀局部解析；解析结果必须保留原定义表及完整块语义。旧块已有脚注引用、候选新源码出现 `[^` 或脚注正文行内语法不再按源码排序时，回退完整解析。脚注节点按首次引用顺序附于根，可能与定义来源顺序不同；块定位只搜索普通块前缀，变长编辑按定义实际来源调整脚注节点，无有序索引时由所有子块重算根来源包络。

没有任何已渲染脚注节点时，顶层未引用脚注定义的正文内部单补丁编辑可单独重解析该定义。新片段必须仍仅有同一脚注定义，且标签、定义顺序、独立空行边界不变；若字节长度变化，则平移后续定义、块与行内语法/候选来源，并保守核对全篇链接引用额度。

已引用脚注定义的独立正文内部单补丁编辑可通过“定义窗口＋虚拟脚注引用＋未变的外部链接定义”局部重建该脚注子树。重解析必须只得到同一脚注与一个虚拟引用块；编辑后的定义标签、来源边界和链接定义表逐项核对。复用全量协调器的节点身份规则，按首次引用顺序替换该脚注的行内语法，重映射其链接定义索引；其他脚注虽可按不同于源码的顺序附于根，来源平移仍按各自定义的实际位置决定。未解析链接候选、后续普通块与所有脚注的根来源包络一并更新。新增或既有脚注正文出现 `[^`、脚注标签/标记改变、定义包含在容器内、多补丁续接无法取得对应旧源码窗口，以及不确定的定义边界仍回退完整解析；OOM、取消和过期错误直接传播。

全篇引用展开额度使用当前不可变源码版本中 `[` 的精确计数。源码补丁只扫描被删和新增字节来更新该计数，连续候选保留各自版本的值，因此含外部链接定义的局部编辑无需再为额度检查读取整篇源码。定义字段变长时，旧/新整篇额度也复用该计数，并把被删的每个字节都当作潜在 `[`，保持原有保守上界；定义本身和目标块仍按上述安全规则核对。

未引用的脚注定义同样参与上述范围检查；不能通过“当前没有脚注节点”推断没有依赖。独立链接定义的标签、目标或标题字段内编辑可先重解析该定义：来源边界与定义顺序不变时，目标/标题编辑依已解析引用定位独立依赖块；标签编辑还可能使未解析候选生效或改变重复定义的优先级，因此保守地重解析所有含 `[` 的独立块。依赖列表按需增长，不再以固定 32 块为界；未被使用的定义保留原树。变长编辑会平移后续定义、块与行内语法/候选来源，并对旧、新整篇的引用展开额度作保守校验。外部定义所影响的独立引用块、列表和表格可按顶层块局部重解析：校验定义表、完整块语法边界和容器块级结构，允许引用解析变化导致的内联节点与父容器来源包络变化。列表语法若已包含分隔空行，虚拟定义后缀复用该空行，避免把后缀的额外换行错误计入列表。容器中的定义、链接定义修改涉及脚注文档或跨脚注依赖、代码或 HTML 等不能证明独立的场景仍全量解析。解析器保留内部定义存在标志，快照/身份协调/语义投影保持这一标志。CR/LF 在窗口边界重新组合时也回退，避免错误地保留实际已经消失的空行分隔。回退是语法决策，OOM、取消和过期错误直接传播，保持事务原子性。

增量候选沿用原有的 NodeId 匹配及逻辑文字片段保留规则；按有序来源范围定位顶层块和行内语法，字节长度变化时在根子块序列中记录未变后缀的延迟来源偏移，正文来源片段与行内语法也共享旧存储并在读取时解析有效坐标。根节点或依赖块替换前会保存旧版索引资格，替换后验证改变的块及前后缀交界，再恢复有序来源索引；不能证明时关闭索引并使用完整定位路径。完成后仍通过根事务发布、通知和共同 Undo/Redo。没有第二套可写文档、历史或发布路径。

`GetStats.iMarkdownParses` 统计解析请求，窗口失败后回退完整解析仍算一次请求；`iMarkdownParsedBytes` 累加实际尝试的窗口/整篇字节，回退可能同时计入两者。`iMarkdownIncrementalParses` 统计完成的局部候选，包含尚未发布的候选。统计结构 ABI 变化，调用方须与 DLL 一起重新编译。

当前仍不是对所有 Markdown 结构都按修改量计算的解析流水线：跨块结构、脚注引用/标签和跨脚注依赖、含内部定义或不确定边界的容器，以及过大或不确定的邻块仍可能完整解析。清空历史后的首次局部编辑使用当前根的待命历史所有权，避免为保留旧根逐对象登记；初次完整加载仍需遍历文档。固定 10 MiB 样本的独立块和单定义依赖编辑只解析数百字节，但真实交互延迟、定义变更的完整依赖驱动失效与完整来源层仍待验收。

## View、Editor 与宿主接口

SOURCE 和 LIVE_MARKDOWN 模式提供带候选代次的输入投影。`xuiDocumentRendererSetSourceInput` 保留真实已提交 Snapshot；SOURCE 绘制候选全文，LIVE 混合候选活动块源码与其他已提交语义块。块内单补丁可只重建受影响行及邻行，保留其余行和语义块的排版缓存；无法证明候选仍在活动块时回退为候选全文源码。输入、光标命中、选区、复制/粘贴、字素删除、滚动和通用 `xuiEdit*` 使用候选内容；其他视图继续读取已提交内容。

`xui_doc_editor_desc_t.iAsyncSourceThresholdBytes` 对 SOURCE、LIVE_MARKDOWN 及受限的 VISUAL 文本输入生效；为零时默认 100 KiB，设为 1 强制异步，设为 UINT64_MAX 保持同步。SOURCE/LIVE 在源码规模或输入量达到门槛时异步，连续按键可延续候选；VISUAL 在源码规模或本次输入量达到门槛时，对无格式段落/标题同一文本叶节点内部的精确来源范围，以及同一无格式段落中由纯文本和软换行组成、两端来源精确的跨叶节点范围异步处理。它显示私有语义候选，已提交 Document 和其他视图保持原值；worker 解析并核对候选语义后，统一发布和记录历史。VISUAL 待提交期间，同一输入组内、来源可精确定位的同文本叶节点插入、删除和选区替换可延续候选；删除专用候选的连续 Backspace/Delete 使用原始未改动前缀/后缀的快速映射；上述跨叶节点候选仅允许在原编辑光标续输。交互式导航会结束输入组，后续编辑先等待发布；程序批次仍可在同组内移动选区后继续编辑。其他跨叶节点或改变 Undo 组的操作等待发布后回放，直接程序调用可能返回 BUSY。发布时保留能精确映射的候选选区，Undo/Redo 书签指向编辑结束光标。其他位置及结构编辑走同步路径。首次需要异步时创建一个工作线程，最多运行一份解析并排队最新一代。淘汰候选交给 worker 回收。一个 Document 仍只有一个 pending；其他 Editor 在它存在期间的写入返回 BUSY，显式 Document 外部写入按原规则淘汰待输入。

`xuiDocumentEditorFlush` 非阻塞地派发/发布输入并处理等待事件，BUSY 时宿主继续 `xuiUpdate` 并重试。模式切换、换 Document、查找和只读状态改变先 flush；保存仍通过 Document 的 BUSY 检查。键盘 Undo/Redo 等命令等待发布时，后来的文字、导航、IME、选择和剪贴板按控件收到的顺序执行，不再覆盖上一条等待命令，也不把后续文字并入将要撤销的旧候选。

生产者调用 `xuiDocumentEditorAppendStreamSource(editor, bytes, length, final_chunk)` 时，失败调用不接受该分片，可修正后重试。`final_chunk=1` 仅关闭流；若候选仍在后台解析，继续 Flush 至 OK 后才能保存或切换模式。未发最终分片、但本帧没有待提交 Prepare 时，`GetPendingWork.iResult` 仍为 BUSY，表示流会话与保存屏障尚未结束。调用 `CancelInput` 可以显式结束会话；它不会回滚已经发布的帧。

等待队列最多接受 4,096 个事件，每次 Flush 最多处理 64 个，并在事件之间检查 4 ms 预算。单条原子命令或宿主回调不能被中断，因此这不是整个 UI 帧耗时的上限。拖动期间相邻 move 合并；普通悬停不进入队列；编辑器不处理的快捷键继续向父级传递。焦点和指针捕获按事件到达时更新，稍后的选择回放不重新抢夺焦点。排队 Paste 在实际执行时读取剪贴板，以保留排队 Copy/Cut/Paste 的顺序。

有等待事件或正在回放时，同一 Editor 的直接输入/命令/选区/滚动/缩放 API 返回 BUSY，调用未被接受，宿主应完成 Flush 后重试；宿主回调也遵守此规则，不继承内部事件处理权限。其他 View/Editor 仍可读取和选择已提交内容，其他 Editor 的写入保持 BUSY。入队 OOM 或超限通过 onError 报告，新事件未被接受，旧队列不变。回放遇到 OOM/容量失败保留当前事件和后续事件，RetryInput 恢复；不支持的命令报告错误后只跳过该命令。

剪贴板回调中的同 Editor 直接 API 同样返回 BUSY，新到达事件入队；显式 CancelInput、Document 外部写入或销毁会中止回调后的剪切删除/粘贴写入。直接命令被此类回调中断时返回 STALE；队列重放将它视为已取消，不继续执行旧队列，也不误删回调重新排入的事件。仅复制不会取消现有 IME 预编辑。

`xuiDocumentEditorGetPendingWork` 报告候选是否存在、排队事件数及当前结果。`GetPendingInput` 和 Document 的 `HasPrepare` 只检测 prepare，不代表控件队列已空。例如 Undo 已完成、后续文字创建候选发生 OOM 时，可能没有 prepare，但仍有已接受的事件；DocumentSaveFile 会继续返回 BUSY。关闭和保存以 Flush 完成为准，不能仅检查 HasPrepare。CancelInput 显式取消候选、预编辑与队列，即便当前只有预编辑也会清理；外部 Document 成功写入淘汰其依赖的等待事件。失焦保留已接受输入并继续按序执行。

源码 IME 使用单独的不可变显示候选，其代次不会替代当前已接受输入，也不触发解析或历史。组合期间暂停该 Editor 的发布，确认时按独立撤销单元进入同一输入协调器；旧单元未完成时保留预编辑并等待，取消只撤销组合投影。Widget 销毁会取消尚未发布的输入并等待工作线程退出；需要保留输入时，宿主先完成 Flush。普通输入、取消、替换和发布不等待线程。

交互文字事件和同方向 Backspace/Delete 在光标连续、模式/格式不变的情况下自动合并。`iUndoGroupTimeoutMs` 为零时使用 1,000 ms，按事件到达时间判断相邻操作；UINT32_MAX 禁用自动合并。排队后再回放不会抹去到达时的停顿，后台发布也不会拆开连续输入。改选区、导航、失焦、模式/格式变化、Undo/Redo 和外部写入结束当前组；当前版本被标为保存点后，新输入另起一组。复制与普通滚动不主动拆组。

Paste、Cut、Enter/含换行输入和 IME 确认分别形成独立撤销单元。SOURCE 的旧候选尚未发布时，新单元先等待它完成，后续事件仍由同一 FIFO 保留并阻止保存；预编辑继续显示，失败重试保留原替换范围。该边界仍可能等待完整解析或增量候选的后缀/计费工作，不能把局部投影或解析字节减少视为分组边界的完整响应验收；完整窗口性能仍待补齐。

合并后的 Undo 恢复首笔修改前的选区，Redo 恢复末笔修改后的选区。后台发布同时保留用户已经移动的当前光标，不让历史书签把它拉回旧位置。公开 InsertText/Execute 属于程序事务，不跨已发布版本自动分组；未发布的 SOURCE 程序调用仍可共享其当前批次。宿主需要交互输入语义时，应派发 text/key/IME 事件；显式复合业务编辑继续使用 Document 事务与 iGroup。

SOURCE 输入投影已支持局部行更新：候选必须是当前投影的直接后继，且仅含零或一个补丁；扫描修改行及两侧上下文，正确处理 CRLF 拆合和终止空行，复用其余行的排版缓存并调整源码偏移。跳过代次、多补丁和不匹配基线回退完整重建；失败前不更改旧目录或缓存所有权。发布快照与当前投影共享同一 SourceStore 时，直接保留布局并切换为已提交坐标。

SOURCE 行目录现由 Renderer 持有可局部拼接的行树：每个行节点保留自己的排版缓存，子树汇总行数、高度、最大宽度和未精排数量；源码后缀偏移采用延迟平移。单补丁直接后继只重新扫描并创建编辑点邻近的行，拆分/拼接不再复制整个行块数组或重建 Fenwick 高度索引；尺寸、偏移/y 定位、光标、命中和绘制均使用行树。更新前完成所有新行分配，失败不改变旧目录；多补丁、跳过代次及不匹配基线仍完整重建。LIVE 也只重扫活动块内的编辑行及上下文，并复用其他源码行与语义块的排版缓存；受影响行数不变时原位交换已准备的行并更新高度索引，行数改变时重建数组和索引。LIVE 的后续源码行在字节数变化时仍逐行平移。VISUAL 继续使用数组和高度索引。

Renderer stats 的累计 `iSourceBytesScanned`、`iSourceIncrementalUpdates`、`iSourceReusedBlocks` 和新增 `iSourceRowsCreated` 分别记录扫描、局部更新、复用和成功更新所创建的行节点。1/10 MiB 各 128 次首/中/尾换行编辑只创建 446 个新行节点，差分结果与完整重建一致；这是 SOURCE Renderer 的独立局部目录证据。Document 的独立块来源偏移现按顶层块版本化，但首次可撤销编辑的历史所有权登记仍会扫描大量旧存储。通用 `xuiEditGetText` 和订阅全文的编辑事件仍会展开整篇源码；完整 GPU/预览/后台完成延迟及平台 P95 尚未验收。Renderer stats 结构新增字段，调用方须与 DLL 同时重新编译。

LIVE 的候选行更新还用 96 次连续插入、删除、替换及 CR/LF 改动与从同一已提交文档重建的混合投影比较，逐步核对尺寸、语义锚点、源码光标和命中。没有可排版语义块、只有链接定义等源码的文档仍按源码行定位光标。在本机冻结解析线程的代理样本里，1/10 MiB LIVE 文档各 64 次待输入编辑的输入、View 更新与代理绘制 P95 在原位行交换后分别为 0.074/0.147 ms，源码目录扫描均为 897 字节；此前逐次重建 LIVE 数组和索引时的两次样本约为 0.74/8.61 ms。该样本不包含 GPU、系统输入派发或后台发布时间，不作为最终平台门槛。换行、跨块及首次后台发布的响应仍需独立测量。

Document 的局部 Markdown 解析现在对未变后缀共享节点及来源片段序列：根的顶层块持久序列记录版本化来源偏移，节点保存其建立时的块位置和基线；`GetNode`、`GetSourceSegment`、源码/语义位置映射、语义回写和 LIVE 布局按当前快照解析有效坐标。行内语法的持久序列同样共享未变后缀字节块，以树上的延迟偏移记录来源坐标和嵌套父索引变化。源码到语义位置的查询先校验各顶层块的有序语法边界及其全部可命中来源段，再二分定位并按距离扩展邻近块；不能证明范围封闭时仍使用完整树搜索，双亲和性与相同距离的节点选择保持原语义。旧快照仍读取旧版本；跨块修改回退完整解析时会将来源元数据重新归一化。完整 CST、复杂跨块依赖和任意来源对象的通用检索索引尚未交付；历史开启时首次登记旧根所有权仍可能耗时较长。

后台发布的精确计费现在可从已计费的当前根作差额遍历，刚清空历史或禁用历史时不再因为缺少历史基线而先扫描旧/新整棵树。若本次提交创建历史记录，系统仍须为它准确登记旧根的存储；大型文档的第一次可撤销编辑因此可能执行大量计费访问和发布更新。禁用历史的固定 1 MiB 局部编辑只访问 220 个计费对象；这不代表启用撤销后的首次编辑具有同样开销。

VISUAL 待提交期间其他跨叶节点或来源不精确的编辑、结构编辑与流式异步化、LIVE 在跨块候选时保留混合排版、后续结构/对象命令的协调和真实平台 IME 继续保留为待完成项。Editor 描述结构新增 iUndoGroupTimeoutMs，使用方须与 DLL 同时重新编译。大文档的单次撤销占用可能超过默认 64 MiB 历史额度并被淘汰；应用可以显式配置更高额度，待输入存储及外部快照不计入该额度。

```c
xui_doc_editor_desc_t editor_desc = {0};
xui_widget editor = NULL;
editor_desc.iSize = sizeof(editor_desc);
editor_desc.tView.iSize = sizeof(editor_desc.tView);
editor_desc.tView.pDocument = document;
editor_desc.iMode = XUI_DOC_SOURCE_TEXT; /* 富文本使用 XUI_DOC_VISUAL */
int result = xuiDocumentEditorCreate(context, &editor_desc, &editor);
```

一个 Document 可以绑定多个 View/Editor。每个视图独立持有宽度、滚动、缩放与布局缓存。View 有选择、复制、命中、链接回调和可选自动高度；Editor 复用它并增加输入、字素删除、键盘导航、IME 和编辑命令。`bAutoHeight` 精确测量当前宽度下的全部内容，并将普通滚轮事件交给父级；它适合外部滚动容器，但没有长消息估算高度/按需精排的完整宿主协议。

DocumentView 与 DocumentEditor 已接入 XUI 样式表。注册控件类型后可设置颜色属性 `document.text.color`、`document.background.color`、`document.border.color`、`document.code.background_color`、`document.highlight.color`、`document.link.color`、`document.selection.color`、`document.caret.color`、`document.find.result_color` 和 `document.find.active_color`。`document.border.color` 绘制 View/Editor 内容区域的普通外框；设置为显式零值可隐藏外框，编辑器的焦点外框颜色另由 `document.border.focus_color` 指定并覆盖普通外框。未设置时使用 View/Editor 描述中的颜色或 Renderer 默认值；`document.background.color` 的显式零值表示透明。`xui_doc_view_desc_t.iBackgroundColor` 可设置不使用样式表时的背景。显式写入文档文字的前景/底色仍优先于主题色；查找命中颜色覆盖文字底色，当前命中颜色覆盖选区色。运行时改色只刷新绘制颜色，不修改 Document revision 或 Undo，也不因颜色本身重新测量文本；MessageList 内嵌 Document 的默认文字色变化同样保留 shaping 缓存。规则线、引用边框和代码块的已缓存几何会读取当前颜色。context 默认字体对象变化已单独触发度量失效和固定高度 View 阅读锚点；富文本 VISUAL 视图对字号语义修改及经字体回调解析的字体族变化已保留可见文字锚点；其他模式与真实平台字体的完整联动仍待验收。

`xuiDocumentViewGetRenderStats` 将当前 View/Editor 的 Renderer 统计返回给宿主，包括块数、累计已测块、布局次数和 shaped 字节。计数器随同一 Renderer 累积，改变缩放并重建 Renderer 后从新实例重新计数；它适合测量可见区排版工作量，不等于全进程的内存或耗时分析。

`xuiMessageListGetNodeDocumentRenderStats(list, message_id, stats)` 返回绑定消息当前 Renderer 的同一套计数，供宿主区分新增 run 输入与联合重整形工作。它不触发同步、排版或字体更新，因而可能反映尚未同步的旧布局；调用方先按正常流程更新/绘制消息，再读取统计。统计随 Renderer 重建而重新计数，不宜直接对重建前后的计数做差。输出 `iSize` 须为新版结构大小；无消息或无 Document 绑定返回 `XUI_ERROR_NOT_FOUND`，无效参数返回 `XUI_ERROR_INVALID_ARGUMENT`，错误时不修改输出。

主题绘制还支持 `document.border.focus_color`、`document.quote.border_color`、`document.rule.color`、`document.paragraph.background_color`、`document.table.border_color`、`document.table.header_color`、`document.table.cell_color`、`document.image.placeholder_color`、`document.image.border_color` 和 `document.image.text_color`。表格表头由 Cell 的 `XUI_DOC_HEADER` 标记判定；节点显式背景色优先于段落/单元格主题底色，显式透明背景也抑制主题回退。无可用图片资源时，Renderer 绘制占位底色、边框和居中裁剪的 alt 文本；未设主题时默认分别为 RGBA(238,241,245,255)、RGBA(170,180,192,255)、RGBA(90,100,112,255)。顶层及嵌套引用的边线随可见子块绘制。编辑器获得焦点且设定焦点边框颜色时绘制边框；IME 预编辑投影使用同一主题和普通外框。运行时更换这些颜色只刷新绘制，不修改 Document 或重新 shape 文本。

固定高度 View 调用 `xuiDocumentViewSetZoom`、XUI 虚拟 DPI 改变或未显式指定字体时的 context 默认字体对象变化，会记录视口中的文档位置及其在行内的相对高度；新 Renderer 以相同语义位置精排附近内容并校正滚动。缩放和虚拟 DPI 已覆盖 Rich、Markdown VISUAL、SOURCE 与 LIVE；默认字体变化已覆盖 Rich VISUAL 和 Markdown SOURCE/LIVE 中正在阅读的段落或源码行。LIVE 模式同时保留活动源码区。度量变化时，曾经精排但已被缓存淘汰的锚点前缀块和对象依赖块也会补测，避免旧估算高度移动可见文本。SOURCE 的离屏行高估算可能改变绝对滚动数值，因此应以可见内容位置而非往返后的 `scroll_y` 数字判断阅读锚点。自动高度 View 的外层滚动仍由父容器负责；它会在默认字体变化后重新测量。富文本 VISUAL 对字号修改已覆盖视口上方已测量与未测量段落、Undo 和段内部分文字拆分；经回调解析的字体族变化也已覆盖前置段落与 Undo。真实平台字体和物理显示器 DPI 切换尚未验收。

若宿主的 `onFont` 回调在字体对象指针、DPI 和视图宽度均未变化时改用另一套字形度量，宿主须在改变回调状态后调用 `xuiDocumentRendererInvalidateFonts(renderer)`、`xuiDocumentViewInvalidateFonts(view)` 或 `xuiMessageListInvalidateNodeDocumentFonts(list, message_id)`。独立 Renderer 会先构造新的 VISUAL/SOURCE/LIVE 布局目录，成功后再替换旧目录，重建离屏高度估算并清除旧字体布局缓存；失败时保留旧目录。固定高度 View 会以当前可见的语义位置重新定位阅读行；MessageList 会重新测量绑定消息及后续消息的位置。Document revision、内容和 Undo 历史不变。若更换的是 context 默认字体对象或虚拟 DPI，现有自动失效仍生效；显式通知用于宿主掌握、XUI 无法自动探测的字体回调状态变化。

视觉模式改变布局宽度时，旧视口附近、Document revision、字体对象和虚拟 DPI 未变的块保留文本 run 的 shaped clusters，仅重建断行、片段及对象几何；离屏块释放原缓存。正文或资源失效会清除相关缓存，缩放变化会重建 Renderer。虚拟 DPI 改变会使独立 Renderer 的 VISUAL/SOURCE 已测量布局、shaping 和内部创建的按字号字体失效；context 默认字体对象改变时，未显式指定 normal 字体的 Renderer 也执行同样的失效。固定高度 View 为保留阅读锚点重建 Renderer。显式 normal 字体不随 context 默认字体变化。富文本 VISUAL 字号及经回调解析的字体族变化的固定高度 View 阅读锚点已测；部分文字样式拆分复用未受影响块的布局。真实平台字体与物理 DPI 仍待验收。10 万字节单段落加一个样式 run 的专项中，首屏只 shape 8,197 字节；靠尾光标补全整块后，往返改宽可复用完整 shape，切换字体对象后则重新 shape。宽度变化后的总高度和抽样光标坐标与全新 Renderer 一致。这是改宽重排复用与 Unicode 字素/断行切点上的首屏前缀排版；可见区、深处文本光标、文本/同段直接子节点 gap 终点选区与段内可选对象及文本叶节点矩形均可保留投影与已提交行逐段续排；完整 Bidi、跨容器结构终点、复合节点几何和连写脚本等需保持 shaping 上下文的长行及代码块无硬换行长行的按需排版仍未实现。

行距只加在相邻文本行之间，单行段落不产生末尾行距。Renderer 使用 double 累计块与行坐标，在最终绘制时对矩形各边独立对齐像素；同一段落或标题内的正文视觉软换行边界（包括跨 Text、颜色和字号节点），`XUI_DOC_BEFORE` 光标留在上一行末端，`XUI_DOC_AFTER` 光标移到下一行起点；强制控制跨 Text、跨节点 CRLF 或跟随空 Text 时，也遵循此规则。下一行的插入位置采用首个字素或空样式节点的度量，不把字号不同的空节点当成普通字形丢弃；完整逻辑块末尾没有其他内容时，两种 affinity 都定位到可输入尾行。SOURCE 的 CR/LF/CRLF 物理行位置继续按行目录定位。只含强制控制的空行命中控制的起点，HardBreak 空行命中父容器对应的 CHILD_GAP；点击结果的光标停留在实际点击行，并可直接输入。

普通软换行的行尾命中使用 BEFORE，编辑器 End/Shift-End、上下与翻页导航保留命中的视觉 affinity；极大横坐标的距离舍入相同时按真实边缘选择行尾。字素内部的原始位置仍先按 affinity 吸附到该完整字素的端点，不跨到另一视觉行。空样式 Text 若仍处于上行，AFTER 可沿共享逻辑基线移到下行并保留自己的字号；若它已独占下一行、位于段落开头或紧跟强制控制，则保留其自身可输入行。下游定位不跨越段落或 Cell。

命中测试先限定指针所在的最内层 Cell，再按视觉行的垂直位置与共享基线选行，最后按水平位置取字素端点；短行右侧空白不会跳到下一行或邻 Cell，混合字号同一行仍按点击的横向位置定位。分数缩放、段后距和滚动的 48 段专项及嵌入式 Editor 指针专项已覆盖这些规则；完整段落 shaping/Bidi 与对象几何仍需继续验收。

`xuiDocumentRendererHitTestCell` / `GetCellRect` 和 View 对应接口返回 `xui_doc_cell_hit_t`：表格/单元格 ID、行列、跨度及单元格外边界。调用方先填写 `iSize`；Renderer 使用文档坐标，View 使用控件局部坐标并计入内容区偏移与滚动。命中包含单元格内边距，嵌套表格优先返回最内层 Cell；按 Cell ID 查询可按需测量所在块，包括当前视口之外的 Cell。专项覆盖合并跨度、分数滚动下的最终绘制矩形、嵌套表格与 1024 行跨度边界。完整辅助技术语义尚未完成。

Editor 连续按上/下方向键时保留首次移动的目标横坐标，经过短行后进入长行仍尽量回到原列；跨段落间距时寻找下一条实际视觉行，避免停在原行。Shift 扩选沿用同一目标列；水平键、Home/End、指针定位、程序设置选区、输入、文档提交和模式切换会重置目标列，布局宽度变化则在下次移动时重新取值。SOURCE、Markdown VISUAL 和富文本 VISUAL 的长—短—长行及较大段后距已在实际 DLL 中验证；复杂 Bidi、混合字形高度与真实平台输入仍需专项验收。

SOURCE 模式的字素与自然词导航直接从持久文本按需读取 1 KiB 缓存块，删除也使用同一路径；IME 源码替换范围不需要复制整篇源码。Renderer 的 SOURCE 行树汇总尺寸与行高，稳定布局下显露光标不扫描全部行。通用 `xuiEditSetSelection/GetSelection` 在 SOURCE/LIVE 模式直接读写源码位置；显式 `xuiEditGetText` 仍按旧协议返回完整文本。Document 已为独立块编辑共享未变后缀并延迟平移来源范围；跨块依赖与其他编辑形态仍待补齐。

`xuiDocumentEditorQueryCommand` 返回命令可用、已激活和混合状态。状态查询检查模式、profile、选区、剪贴板后端与历史；折叠光标在文档起点时 Backspace 不可执行，在终点时 Delete 不可执行，空文档两者均不可执行；非空选区仍可删除。程序化 `Execute` 对这两条命令返回相同的禁用原因。实际 Markdown 提交仍要验证语义可表达性。控件只读不会阻止宿主通过 Document 事务更新内容。

宿主可用 `xuiDocumentEditorSetupToolbar(editor, toolbar, groups)` 为现有 XUI Toolbar 填充历史、剪贴板、粗斜体/下划线/删除线、对齐、段落/前三层标题、引用切换、分隔线和空代码块插入，以及列表创建和缩进命令；`groups=0` 选择默认组，列表组需显式选择。工具项的 `iValue` 是统一 Document 命令编号。宿主在 Toolbar 选择回调中调用 `xuiDocumentEditorExecuteToolbarItem`，在选区、模式、文档、只读或语言变化后调用 `xuiDocumentEditorSyncToolbar`；后者根据 `QueryCommand` 更新启用、勾选和当前语言提示。适配层不持有两种 Widget 的生命周期，查找窗口已由 Editor 自身提供，其他对象属性仍需宿主按对应 API 接入。

DocumentEditor 的右键和键盘菜单键现打开内建 XUI Menu，列出 Undo/Redo、Cut/Copy/Paste/Delete 与 Select All。位于 VISUAL 表格 Cell 或矩形选区时，菜单还列出行列插入/删除及 Cell 合并/拆分；每项启用状态取自统一命令查询，因此 Markdown 表格不能表示的合并/拆分显示为禁用。普通正文及 SOURCE 模式不显示表格命令。菜单在首次打开时创建，由 Editor 持有；`xuiDocumentEditorOpenMenu(editor, x, y)` 可在世界坐标处主动打开，`GetMenuWidget` 供宿主读取或调整。启用状态每次打开都按统一命令重新查询；右键点在已有选区内保留选区，点在选区末端或外部则移动光标，键盘菜单键锚定当前光标。菜单译文复制到 Editor 自有内存，运行时替换语言包不会使已打开菜单引用失效，下次打开更新译文。只读模式禁用修改命令；Rich、Markdown VISUAL/SOURCE 共用这一路径。更完整的格式菜单与工具栏自动宿主同步仍待实现。

IME 预编辑使用未发布候选的独立渲染投影，确认形成一个根提交。取消、外部内容修改、选区/模式变化会撤销预编辑。输入测试覆盖预编辑、取消、确认、替换范围与跨视图选区映射；尚未完成真实 Windows 输入法交互验收。

通用 `xuiEdit*` 支持文本投影、选区、复制/粘贴、历史、只读、光标位置；它的旧协议使用 `int` 偏移，越界返回 LIMIT。原生 Document API 仍采用 64 位结构位置。编辑事件在 `xuiUpdate` 中合并发布。通用编辑协议提供基本文本框值/选区/编辑动作；VISUAL 模式另外通过 Document 无障碍 provider 暴露表格、链接与对象语义节点，详见下文。

`xuiDocumentSnapshotFind` 提供区分大小写的 UTF-8 字面查找，可跨样式片段；`FindEx` 的 `XUI_DOC_FIND_REGEX` 启用 XRT UTF-8 正则，`XUI_DOC_FIND_IGNORE_CASE` 启用 Unicode 大小写不敏感匹配，`XUI_DOC_FIND_WHOLE_WORD` 使用 XUI Unicode 自然词边界，三项可组合。整词边界检查针对完整文本投影，不把限定范围的端点误当词边界；该选项当前限制投影长度不超过 `INT_MAX` 字节，超限返回 LIMIT。`FindEx` 返回不重叠匹配，零宽正则命中返回折叠位置；传入范围时，正则锚点以该范围为边界。`xuiDocumentViewSetFindQueryEx` 在 View/Editor 中保存查询及选项，零结果也有效；`SetFindScope/GetFindScope` 可设置非空选区作为查找范围，该范围随文档 revision 和视图模式映射。`GetFindResultCount/GetFindResult` 返回全部范围与当前索引，`ClearFind` 同时清除查询和范围。`xuiDocumentViewFindEx` 按选区向前/向后导航，可选回绕；`ActivateFindResult` 直接选中指定结果。无 Ex 后缀的接口保留区分大小写的字面行为。全部匹配与当前匹配按两种主题色绘制。文档提交后按 revision 重新计算，模式切换后按 VISUAL 语义文本或 SOURCE/LIVE Markdown 源码重新计算；切换绑定文档清除查询。编辑器的未发布输入投影不绘制旧 revision 的命中。`TxnReplaceAllEx` 和 Editor `ReplaceAllEx` 在一个根事务里完成，不依赖可见区控件；默认按字面量插入替换文本，同时设置 `XUI_DOC_FIND_REGEX | XUI_DOC_REPLACE_EXPAND` 时可用 `$0`、`$1` 等编号组、`${name}` 命名组和 `$$` 字面美元符展开每一处匹配。无效模板在发布前原子拒绝。

`xuiDocumentEditorOpenFind` / `OpenReplace` 懒创建 Editor 持有的 XUI 查找窗口，`GetFindWindow` 允许宿主读取窗口；若宿主销毁该窗口，下次打开会重建。`Ctrl+F` / `Ctrl+H` 从 Editor 打开，窗口内 Enter/Shift+Enter 或 F3/Shift+F3 前后导航，Esc 关闭。查找框变化更新高亮与虚拟结果表；“查找全部”重新扫描，“替换当前”只替换精确选中的匹配项，未选中时先导航到下一项，“全部替换”仍是一条 Undo。窗口提供“区分大小写”（默认开启）、“整词”、“正则”和“仅选区”复选框。启用“仅选区”时捕获当前非空选区，后续结果导航不会重新定义范围；正则替换按上述捕获组模板展开。无效正则或模板拒绝修改，正则错误清除过时命中并显示提示。窗口随 Document revision、模式及语言包变化刷新；只读 Editor 允许查找并禁用两个替换按钮。搜索结果虚拟表对大文档仍需 UI 响应性能验收。

对象测量和绘制可通过 Renderer 的 `onObjectMeasure/onObjectDraw` 接入。`XUI_DOC_IMAGE` 现在有原生 surface 路径：将节点的 `sResource`（Markdown 的图片地址）作为名称，在 View/Renderer 所属的 XUI context 用 `xuiResourceSet` 注册 `XUI_RESOURCE_SURFACE`，Renderer 就按 surface 的实际尺寸测量，并通过代理 `drawSurface` 绘制。图片只借用注册表的 handle，不在 Document 或 Renderer 中持有 surface；宿主须保证注册资源的生命周期，并可用资源的 `onDestroy` 释放 surface。节点的 `fWidth/fHeight` 可覆盖固有尺寸；仅给一个维度时保持原始宽高比，超出段落可用宽度时等比缩小。自定义对象回调优先，返回 `XUI_ERROR_UNSUPPORTED` 时图片改用上述默认路径；没有匹配的 surface 或注册名称对应非 surface 资源时绘制占位底色、边框和 alt 文本。

XUI 资源注册表提供 `xuiResourceGetRegistryGeneration(context)`。注册、替换、移除或 `Touch` 命名 surface 后，Renderer 在下次排版/绘制时自动重测相关图片；View 在更新帧请求布局和缓存刷新，无需宿主逐个调用失效接口。命名图片在构建块目录时登记，即使尚未进入可见区也能响应后续资源变化；没有命名图片的视图不会因无关资源变化而重排。自定义对象 provider 自行改变尺寸时仍调用 `xuiDocumentViewInvalidateObjects(view)`（或独立 Renderer 的 `xuiDocumentRendererInvalidateObjects(renderer)`）。对象失效只释放含对象块的布局缓存；固定高度 View 重新测量视口上方的对象，并以块高度和已排版片段修正阅读位置。Document 内容和撤销历史不变。宿主可用 `xuiDocumentImageResourceLoadMemory/File` 同步提交编码图片或授权文件路径，默认限制编码 16 MiB、解码后 1600 万像素，失败替换保留原资源。

图片剪贴板当前以内存 PNG 进入 Document：XGE 可从 RGBA8 编码并正确转换预乘 alpha，Win32 优先读取注册剪贴板格式 PNG；没有 PNG 表示或延迟渲染失败时，将常见 24 位 BI_RGB `CF_DIB` 或 32 位 BI_RGB/BI_BITFIELDS `CF_DIBV5` 转换为同一 PNG 输入，按掩码提取颜色和透明度，并拒绝截断数据或重叠掩码。向 Windows 复制有效 PNG 时，XGE 同时提供带显式 alpha 的 32 位顶向下 `CF_DIBV5`，供只识别系统位图格式的应用读取；超出 1600 万像素或无法解码的 PNG 不附加 DIB，原有剪贴板项目照常发布。手动运行的 XGE 实例使用不显示的顶层窗口持有剪贴板。VISUAL View/Editor 仅在选区恰为一张图片且 context 中存在同名 surface 时附加 PNG 字节；没有可读 surface 仍复制原生片段和 HTML 资源描述。VISUAL Editor 在原生片段和 HTML 缺失时可从 PNG 或上述 DIB 表示粘贴，限制 16 MiB 编码数据和 1600 万像素，创建唯一的 context 命名资源和图片节点，失败不发布文档，成功可一步 Undo。资源由 context 持有，Undo 不删除它；宿主保存文档时仍需把这类图片归档为持久资产并维护 URI 映射。ICC 色彩管理、压缩/调色板 DIB、跨进程应用矩阵及其他平台仍需验收。

Win32 DIB 回退在首次 PNG 长度查询后暂存一次转换结果，供紧接着的内容读取复用，避免 Document 粘贴把同一位图解码、编码两遍。暂存上限为 16 MiB；完整读取、XGE 写剪贴板或 `xgeUnit` 会释放，下一次 DIB 回退会按系统剪贴板序号淘汰过期数据。超出上限的图片仍可能重复转换。这只是减少重复计算，未提供跨进程快照或真实 UI 延迟保证。

需要避免 UI 线程解码时，宿主可调用 `xuiDocumentImageResourceLoadFileAsync` 或 `LoadMemoryAsync`：后者复制编码字节，调用方可随即释放原缓冲。XGE worker 读取/检查尺寸并解码 CPU 像素；所有者线程轮询 `xuiDocumentImageResourcePoll`，完成时会回收同 context 的已完成 worker、上传 GPU、替换命名资源并推进队列。每个 context 最多运行 4 个 worker、保留 64 个未完成请求及 64 MiB 已复制编码输入，可用 `xuiDocumentImageResourceGetAsyncStats` 查询。取消、同名新请求和资源外部变化分别阻止旧结果发布；请求必须在 context 销毁前 Release，Release 可能等待正在解码的 worker。Markdown URI 不会自行触发本地或网络访问。文件/网络 URI 的宿主解析与授权、已发布 surface 的缓存驱逐及 GPU 预算、恶意载荷的完整解码防护、任意样式度量变化、物理 DPI 与复杂重排场景下的滚动锚点仍需继续实现。

`xui_doc_image_desc_t` 携带图片地址、UTF-8 alt、标题、可选宽高，以及独立的 `sLinkTarget` / `sLinkTitle`。`sLinkTarget=NULL` 表示无外层链接，非空指针（包括空串）表示带链接；图片地址与外层链接目标在统一 Image 节点中分开保存。`xuiDocumentTxnInsertImage(range, image, image_id, caret)` 在折叠光标插入图片，也可用图片替换语义选区；根/容器结构间隙可创建段落。`TxnUpdateImage(image_id, image)` 一次更改地址、alt、标题、链接和尺寸，保持图片 NodeId。VISUAL Editor 的 `InsertImage` / `UpdateImage` 使用同一事务，形成单步撤销，并遵守只读状态。富文本的显式宽高参与原生排版且写入 HTML 导出的图片样式；Markdown 不表达宽高，非零尺寸明确返回 `XUI_DOC_ERROR_UNREPRESENTABLE`。Markdown 解析 `[![alt](image)](target)` 及引用式外层链接，HTML 导入/导出 `<a><img></a>`，原生格式与 fragment、Rich→Markdown 转换均保留两个独立资源。在已解析文字内插入图片、替换同一文字 run 的选区，以及更新已有图片时，优先做局部源码补丁并重解析核对完整语义；未改动外层链接时保留其引用写法。不能证明局部补丁等价时回退到结构写回；引用/复杂语法不能保证语义时拒绝，不承诺所有结构命令保留原块标点拼写。SOURCE/LIVE 模式仍可直接编辑原始 Markdown 图片语法；图片资产持久化、跨应用兼容矩阵、拖放和完整来源无损改写尚未完成。

VISUAL View/Editor 点击图片、公式、Mermaid、HTML 或扩展对象时选中其父容器中的完整子节点范围；`xuiDocumentViewSelectObject(view, node)` 为工具栏和宿主提供相同的程序入口。空 alt 图片也有非折叠结构选区，选中覆盖层绘制在对象图像之上。编辑器的字素级方向键把对象视为单个替代字符；Shift+Right 可从对象前方整体选中图片或公式。辅助技术可按节点选中对象，并读取已选中状态。复制可携带原生片段与 HTML，Delete/Cut 使用统一 Document 事务，Undo 恢复原节点；SOURCE/LIVE 不提供此结构对象选区。拖放、Bidi 视觉导航和更多对象辅助技术动作仍需补齐。

VISUAL Editor 的 `xuiDocumentEditorUpdateObjectSource` 可替换现有公式、Mermaid 图、HTML 与代码块节点的完整 UTF-8 源码，保留 NodeId，并与普通编辑共用根事务和 Undo。块级 Markdown 节点的 `SnapshotCopyText` 包含结尾换行，调用方应把读取的完整 payload 用作编辑初值。Markdown 改写后会验证解析语义；若 Mermaid 内容中有可关闭围栏的独立 `~~~` 或反引号行，回写会改用足够长的波浪线围栏，而行中普通围栏字符保持原围栏。破坏对象语义的输入会被拒绝且不提交。SOURCE/LIVE 模式仍直接编辑 Markdown 源码；本接口只解决对象源码编辑，不提供公式排版、图形绘制或 HTML 交互。

`xuiDocumentTxnInsertObject(range, kind, flags, utf8, bytes, object_id, caret)` 和 VISUAL Editor 的 `xuiDocumentEditorInsertObject` 现可创建公式、Mermaid 和 HTML 节点。公式为段内对象，可用 `XUI_DOC_BLOCK` 表示展示公式；HTML 可选段内或块级；Mermaid 为块级。段内对象可替换选区，块级对象要求折叠光标，位于段落内部时先拆分段落；插入、拆分和光标位置共用一次 Undo。Markdown 的公式/Mermaid 限 EXTENDED 方言，HTML 适用于 CommonMark/GFM/EXTENDED；块级 HTML 和 Mermaid 正文若缺末尾 LF，会补入一个 LF，以符合解析器的对象正文语义。候选重解析并核对完整语义后提交；例如无法保留对象种类的 HTML 片段会原子拒绝。共享写回器将拆分后段落的边缘空格写成数字字符引用，使空格附近插入块对象仍能保留原文字内容。Rich 保留调用方传入的对象源码。

`xuiDocumentTxnInsertCodeBlock` / `xuiDocumentTxnInsertRule` 及对应 VISUAL Editor 接口提供代码块和分隔线创建。两者只接受折叠语义光标；位于段落内部时先在同一事务中拆分。代码块要求语言为不含空白或围栏字符的单个 UTF-8 token，Markdown 内容缺少末尾 LF 时补齐，并通过重解析验证；`mermaid` 在 EXTENDED 方言中会解析为图形而不是普通代码块，因此普通代码块插入会被拒绝。代码块创建后光标位于正文末尾，`xuiDocumentEditorUpdateObjectSource` 可更新完整代码正文；分隔线创建后光标位于其后。两者均可一步撤销。`xuiDocumentTxnSetCodeBlockLanguage` 和 `xuiDocumentEditorSetCodeBlockLanguage` 可修改或清除代码语言，保留 NodeId 与正文；语言非法或改写后不再解析为代码块时原子拒绝。

`xuiDocumentTxnWrapQuoteRange` 与 `xuiDocumentEditorWrapQuote` 可把当前段落，或同父连续完整块（包括规则线、列表、代码块和表格）包进一个引用块，保留原块 NodeId 和选区方向。跨父范围在边缘容器完整覆盖时提升至共同父容器；直接位于共同父容器下的 Quote 或 List 被部分覆盖时，可在子块或列表项边界拆开，原容器留下未选一侧，选中一侧连同相邻块进入新 Quote；同一 List 中间的完整列表项也可拆成前、中、后三段再包裹中段，有序列表的起始编号随拆分调整。外层 Quote 中的 List 可在项边界与该 Quote 内的段落边界一起拆分，或把同一 List 的中段包进内层 Quote；Markdown 写回保留未选的 Quote 前后子块及列表边缘的原始来源。宽松列表被拆成仅含简单段落项的列表时，按保留项之间的原始空行重新判定紧凑属性：单项或项间无空行变为 tight，项间仍有空行保持 loose。写回结果须经独立全文重解析校验。从根层 Quote 或 List 沿单一 Quote/List/ListItem 祖先链可达的目标 Quote，可从其内层 List 的项边界包到同 Quote 段落；局部来源写回只在被选行有解析器确认的显式 `>` 时增加一级引用标记，其他行原字节保留。选区内的 LINK 定义可随引用层级移动，但定义数量、顺序及标签、目标、标题原始 token 必须保持；旧 Quote 标记补丁路径对缺少显式标记的 lazy continuation 保持拒绝。`xuiDocumentTxnUnwrapQuote` 与 `xuiDocumentEditorUnwrapQuote` 在折叠光标处移除最近一层引用，把其所有子块提升到父容器；两者都是单步 Undo。同一 Quote 内的同父完整块范围沿用普通包裹规则；同一 ListItem 内选中的连续完整子块也可独立包进 Quote，来源写回沿解析器记录的续行缩进插入标记，保留列表标记、未选实体、空行和定义原文；多子块之间的空行生成引用空行，文件末尾无换行也可处理。选中段落的 lazy continuation 可沿同项已确认的续行前缀补齐；选区内的 LINK 定义须保留原标签、目标和标题 token。候选仍经全文语义核对。列表项内部单个块的部分内容、跨多个 ListItem 的内部子块和同一边缘跨多层容器同时拆分仍不支持；Markdown 无法保持引用内语义的 Front Matter 与脚注定义明确拒绝。

列表项首个完整子块被选中时，普通/有序列表可从解析器记录的列表标记宽度推导续行前缀，对 lazy continuation 补齐缩进和引用标记；Tab 和外层 Quote 前缀保留原字节，结果仍需全文重解析验证。任务列表首个子块直接变为 Quote 无法保留任务标记所依附的开头段落，因此 Markdown 命令预检返回 `XUI_DOC_ERROR_UNREPRESENTABLE`；选中任务列表后续完整子块仍可包裹。

`XUI_DOC_EDIT_BLOCK_QUOTE` 在折叠光标处包裹段落或取消最近一层引用，状态查询以 `bActive` 表示光标处已有引用；连续兄弟段落/标题选区可执行包裹。`XUI_DOC_EDIT_INSERT_RULE` 与 `XUI_DOC_EDIT_INSERT_CODE_BLOCK` 仅在折叠的 VISUAL 语义光标处启用，后者创建无语言的空代码块。三个命令和对应 Toolbar 按钮共用上述事务入口；只读及 SOURCE/LIVE 模式禁用。工具栏状态是预检，Markdown 的实际语义往返仍可能拒绝不可表达的结构。

`xuiDocumentTxnInsertFootnote` 在语义选区创建脚注引用和定义，返回引用后及定义正文中的两个光标；VISUAL Editor 的 `xuiDocumentEditorInsertFootnote` 把光标放在定义正文，空定义可直接输入。标签可显式指定且必须满足 MD4C 脚注标签语法，省略时选择未占用的 `fnN`；已使用和仅保留在 Markdown 来源中的未使用定义都参与占用检查，比较遵循解析器的 Unicode 大小写折叠规则，不按原始字节长度猜测标签是否相同。Markdown 仅支持 EXTENDED 方言，写回后重新解析并核对完整语义；定义在语义根中的顺序随正文首次引用顺序排列，已有定义不会因新增较早引用而错位。改写脚注正文时，定义自身的来源范围允许重写，块内其他引用定义仍受来源保护。Rich 和 Markdown 均可创建多段正文；Markdown 解析器将顶层及任意列表/引用嵌套中的“空行＋足额缩进续段”归入同一脚注，使用共用块解析器建立段落、列表、围栏/缩进代码与引用块，并保留各块来源范围。定义首行为空、CRLF 和嵌套容器均已验证；列表空白分隔行可省略缩进，引用前缀必须显式保留。语义不能往返的视觉编辑会原子拒绝。

`xuiDocumentTxnRemoveFootnoteReference` 和 VISUAL Editor 的 `xuiDocumentEditorRemoveFootnoteReference`可在一个事务中删除指定脚注引用，并将光标移至原引用位置。删除后从正文引用出发计算脚注定义的可达性，只清理本次变为不可达的定义；共享定义保留，脚注自引用与依赖链可连带清理，原本未使用的 Rich 定义不受影响。Markdown 通过语义重解析校验结果，失败不发布半成品。

启用 WebView2 的 Windows DLL 可选用 `xuiDocumentWebProviderCreate` 为公式、Mermaid 和块级原始 HTML 节点提供静态显示。宿主传入 XUI context、仍存活的父 widget、`res/xui_document_web` 的绝对路径以及视图失效回调；可选的 `sUserDataFolder` 指定 WebView worker 的独立 profile，创建时复制，未指定时使用运行时默认位置。Renderer 的 `onObjectMeasure` / `onObjectDraw` 分别指向 `xuiDocumentWebObjectMeasure` / `xuiDocumentWebObjectDraw`，`pUser` 指向 provider。每帧在 `xuiUpdate` 之后调用 `xuiDocumentWebProviderUpdate`，回调里调用 `xuiDocumentViewInvalidateObjects`；销毁父 widget/context 之前释放 provider。它使用一个不可输入的 WebView worker，在本地页面内运行打包的 KaTeX/Mermaid/DOMPurify、度量结果并截取 PNG；浏览器排版始终保留完整 viewport，缓存时再把实际内容矩形裁成 XGE surface 供共享 Renderer 绘制。缓存按文档身份、revision、NodeId、可用宽度、缩放与调色板代次区分；Document 事务修改源码后，新版本重新渲染，浏览器不持有另一份历史。worker 保留 1×1 可见区域以驱动 WebView2 绘制，父控件必须位于实际 XGE 窗口树内。`xuiDocumentWebProviderSetPalette` 接受不透明的 XUI RGBA 前景/背景色，切换时取消旧请求、丢弃旧缓存并通过失效回调重新测量；默认是浅色。真实窗口已验证暗色公式/Mermaid 与运行时切换，但多个不同主题的视图需分别持有 provider。当前支持静态公式、Mermaid 与块级原始 HTML；DPI、对象辅助技术和生产级错误呈现仍需补齐。禁用 WebView2 的普通构建不提供此显示能力。

需要操作原始 HTML 时，在已经设置根控件和 `xuiInputViewport` 的 Windows XUI context 中，对 Rich/Markdown 的 `XUI_DOC_HTML` 节点调用 `xuiDocumentHtmlInteractionCreate`。DocumentView/MessageList 的对象 `onActivate` 回调可将 NodeId 交给该入口。它建立独立的 XUI Window 与子 WebView；源码经转义放入禁脚本、禁表单提交的 sandbox iframe，保留表单控件输入和局部 DOM 状态。面板外层处理文内 `#锚点` 点击，避免 iframe 导航清空原内容。其他链接会被拦截；仅 HTTP(S) 的绝对地址经私有 WebView 来源及页面代次核验后交给描述中的 `onOpenLink` 回调，由宿主决定如何打开。未提供回调或协议不被接受时不导航。宿主在 UI 线程每帧布局/更新后调用 `xuiDocumentHtmlInteractionUpdate`：无关的 Document revision 不重载页面，目标节点源码变化才重载；浏览器中的输入不会写入 Document 或产生 Undo 历史。`xuiDocumentHtmlInteractionShow` 可重开面板并在宿主布局完成后聚焦浏览器。源码编辑仍用 Document 事务；面板在载入 HTML 前安装私有 WebView2 子资源过滤器，在 WebView2 请求事件中放行 `about:`/`data:`，并给其余 URL 返回 403；若运行时不支持所需的 `ICoreWebView2_22` 接口，面板返回 `UNSUPPORTED`。销毁 context 前调用 `xuiDocumentHtmlInteractionRelease`。这是静态对象的独立交互入口，网页任意脚本应用仍交给通用 WebView。该入口目前已验证实窗焦点、裁剪、源码隔离、重载、文内锚点、长内容滚动、外链回调，以及系统键盘输入到 iframe 单个表单控件且不修改 Document；受控网络/文件资源加载、可访问性及复杂表单仍需专门验收。

对象脚本返回错误时，provider 保留最多 255 字节的 UTF-8 错误原因，过滤控制字符；测量回调会给失败对象返回按默认字体宽度分行的卡片尺寸，绘制回调在 DocumentView/MessageList 中显示对象类型和原因。宿主需设置 context 默认字体，并在 `onInvalidate` 中失效对象布局。错误结果只属于当前文档版本和调色板代次；修改对象源码后正常重新排队，不会把失败状态带到新版本。渲染脚本、截图请求和截图任务超时会结束当前对象等待并显示原因；真实 WebView2 窗口已注入验证脚本无响应、PNG 解码失败和内容裁剪失败。解码或裁剪接口在创建 surface 后报错时，provider 会释放该资源；裁剪失败不会把完整浏览器 viewport 当成对象缓存。截图请求/捕获、尺寸和缓存额度等其余故障分支仍待专项验证。最多绘制 32 行，极窄容器或过长的错误可能截断；对象辅助技术仍待完善。

WebView worker 在创建后无法映射本地资源、页面导航/依赖加载失败或启动超时时，provider 将待渲染对象转成同一种错误卡片；此后新版本的对象也直接显示原因，不会留在永不完成的队列。`xuiDocumentWebProviderUpdate` 对已经记录的永久 worker 失败返回 `XUI_OK`，以便窗口继续布局和绘制。浏览器进程失败或意外关闭则在显示错误卡后按 1、2、4 秒间隔最多连续自动重建三次内部 worker；页面稳定运行 60 秒后重置重启次数。仅因 worker 故障失败的对象在新页面就绪后重新排队，已完成缓存和独立的语法错误不受影响。宿主通过 `xuiDocumentWebProviderGetStats` 的 `bWorkerFailed`、`sWorkerError`、`bRestartPending`、`bRecovering` 与 `iRestartAttempts` 读取状态。单对象 KaTeX/Mermaid/HTML 语法错误只增加 `iFailed`，不会触发重启。已完成的静态缓存可继续显示，切换调色板后会清空并按当前故障状态重新生成卡片。创建时同步返回的后端初始化错误仍由 `xuiDocumentWebProviderCreate` 返回；耗尽重启次数或其他永久 worker 故障后，恢复需要释放并重建 provider。

普通 Markdown 围栏代码块通过 `xuiDocumentTxnReplaceText` 修改字面文本时使用相同的围栏安全检查。只有新增内容能关闭当前开围栏时才重写受影响块；例如原文使用四个波浪线，正文中的独立三个波浪线仍可局部编辑，并保留开围栏的空格等原始写法。来源范围经前方编辑平移后也按当前坐标读取。

`xui_doc_web_provider_desc_t.iTimeoutMs` 可设置内部浏览器的启动、渲染响应、等待绘制及截图上限，零值默认 10 秒。上限使用单调时钟的实际经过时间，由每帧的 `xuiDocumentWebProviderUpdate` 检查，不再把“600 次更新”当成固定时长。超时只终结当前对象并显示原因；启动超时使 worker 进入故障状态。宿主应持续调用 Update，停帧期间不会主动触发检查。

## MessageList 文档正文

MessageList 的消息正文可绑定富文本或 Markdown Document，使用与 DocumentView 相同的共享 Renderer 绘制与命中。消息行仍由 MessageList 管理头像、标题、气泡、可折叠辅助节点和外层滚动；Document 负责正文结构、布局及历史。绑定会保留 Document 并订阅提交，文档修改后重新测量该消息及其后续行；替换消息、解绑或销毁列表时取消订阅并释放 Renderer。绑定排版失败时保留旧绑定与消息状态。

可访问树按 `List → 消息 → Document → 文档节点` 枚举。文档根、段落、标题、文字、链接、列表、表格及对象沿用 DocumentView 的语义角色和值；保留不变文档节点的 MessageList 可访问 ID，解绑后旧 ID 失效。段落、标题与文字节点可用局部 UTF-8 字节偏移选择正文并沿用消息级复制；链接可调用绑定描述中的激活回调，有实际边界的节点可滚入列表视口。文档提交和绑定变化通知树结构更新。折叠的辅助消息不枚举隐藏文档子树。任务项支持回调驱动的标记点击和可访问 TOGGLE；深层嵌套场景和真实平台读屏桥接仍待验收。

列表、引用、脚注、表格、行与单元格支持按自身 `sValue` 的 UTF-8 字节偏移选择，反向与跨子节点范围会裁剪到各节点；图片、公式、Mermaid、HTML 与扩展对象按所在父容器的结构边界整体选中。MessageList 单元格的 `SET_SELECTION(NULL)` 选择包含该 Cell 的完整表格矩形；携带局部范围 payload 时仍选择文字。`xuiMessageListSetNodeDocumentTableSelection` 按 Table ID 和网格坐标设置矩形，自动扩展以覆盖相交的合并 Cell；`GetNodeDocumentTableSelection` 返回展开后的范围。矩形与普通文字选区互斥，选中的完整 Cell 报告 `SELECTED`，Renderer 着色显示整块，消息级复制输出可往返粘贴的 TSV。传入 `NULL` 清除矩形；清除文本选区、重新选择文字、文档提交或解绑也会清除矩形，纯选择不改变 Document revision。MessageList 支持 Alt+拖动、Alt+Shift+方向键从当前 Cell 扩展或收缩矩形，Escape 清除；键盘延伸时把目标 Cell 滚入视口。深层嵌套场景、平台读屏及大文档节点查询性能仍待验收。

```c
xui_message_document_desc_t body = {0};
body.iSize = sizeof(body);
body.pDocument = document;
body.onActivate = on_document_activate;
body.onTaskToggle = on_document_task_toggle;
body.pUser = host;
xuiMessageListSetNodeDocument(message_list, "answer-1", &body);
/* NULL 解绑，并恢复该消息的 sText 绘制。 */
xuiMessageListSetNodeDocument(message_list, "answer-1", NULL);
```

`onTaskToggle` 在 UI 线程收到任务 ListItem 的 NodeId 和目标勾选状态；鼠标点击复选标记和可访问 `TOGGLE` 动作共用这条回调。MessageList 不直接修改文档。宿主可在回调中读取节点属性，用 `xuiDocumentTxnSetAttributes` 设置或清除 `XUI_DOC_CHECKED` 并提交事务；Document 观察器会刷新正文和可访问状态，Undo 由 Document 处理。未设置回调时任务项保持只读，指针不会触发切换。`xuiDocumentRendererHitTaskMarker` 也公开相同标记区域的文档坐标命中，便于自定义容器复用。

`xuiMessageListHitNodeDocument` 接受与 `xuiMessageListGetNodeAt` 相同的世界坐标；`GetNodeDocumentSelection` 返回该消息在列表级拖选中的本地 Document 结构范围，完全位于选区内的中间消息返回全文范围。正文按内容高度扩展，不单独处理滚轮；列表滚动后仍按同一文档位置命中。可跨 Document 与普通文本消息双向拖选、复制；系统和折叠消息跳过，消息间以换行分隔，Document 片段保留自身语义换行。文档事务提交会映射选区端点；重新绑定选区覆盖的消息会清除选区。链接/对象激活回调仍走绑定的 Document，字体与列表主题色变化会刷新嵌入 Renderer。`onActivate` 的资源字符串只在回调期间有效。MessageList 的文本导入/导出格式仍记录消息自身的 `sText`，文档内容应单独持久化并在恢复消息后重新绑定。

消息绑定描述的 `tRenderer` 也可设置同一个高级对象 provider 的测量/绘制回调。使用 context 命名 surface 的图片在资源注册、替换或移除后，可见的 MessageList 会在下一次 `xuiUpdate` 检测资源代次，刷新可见消息的 Renderer、行高和后续消息位置；Document revision 不变。高级 provider 的异步对象完成或尺寸改变时，在 provider 的失效回调中对相应消息调用 `xuiMessageListInvalidateNodeDocumentObjects(list, message_id)`；MessageList 会丢弃该消息 Renderer 的对象布局、重新测量行高并刷新后续行。该接口仅适用于仍绑定 Document 的消息；解绑后返回 `XUI_ERROR_NOT_FOUND`。调用方应保证 provider 与消息绑定的生命周期顺序。

多块长文档先按块高估算消息高度，不精排离屏块；进入可见区后只精排附近块，修正本消息及后续消息坐标。列表自身保留消息位置；若当前视口位于同一 Document 消息的正文深处，默认字体、虚拟 DPI、命名资源或显式对象失效引起重排时，再以可见文档位置和行内相对高度校正滚动，并补测此前已测量的前缀块及对象依赖块。显式 `ScrollToEnd` 会在末尾精排后再次定位底部。离屏消息的 `GetNodeRect` 高度可能仍是估算值。MessageList 已提供列表与消息级语义树：消息暴露发送者或辅助节点标题、正文纯文本、时间、选中与折叠状态，并可按消息选择、滚入视口或展开；绑定 Document 的正文值随文档版本更新，消息 ID 在原位更新时保持稳定。消息正文已支持局部 UTF-8 字节范围的正反向选择；普通文本和绑定 Document 使用现有选区与复制路径，非法字节边界原子拒绝。拖动、清除、Document 更新/解绑时的选区变化，以及辅助消息折叠引起的状态、正文与几何变化会在外层更新点合并通知可访问性事件订阅方。绑定 Document 已枚举根、段落、标题、文字等内部语义节点；表格矩形已支持程序化、指针与键盘选择、绘制和 TSV 复制；任务项标记点击与可访问 TOGGLE 已交给宿主；深层嵌套场景、嵌入交互 Widget 与真实平台读屏仍待实现或验收。对有字素/断行切点的极长段落可首屏按需 shape 前缀，并在共享 Renderer 中随可见区滚动续排；90 KiB 单段落消息的两次同块滚动、追加 shaping、绘制与命中已通过专项；默认字体、虚拟 DPI、列表宽度与同块命名图片高度往返时，90 KiB 单段落消息的深处阅读行已通过专项；富文本字号语义变更已覆盖前置段落及同块部分文字的阅读锚点；字体族、物理 DPI、其他对象尺寸及更复杂同块重排仍待验收，完整 Bidi 和连写脚本等受保护长行和代码块无硬换行长行仍待实现。

## 保存与打开

```c
/* 成功后才标记对应内容已保存。 */
int result = xuiDocumentSaveFile(document, "notes.md", XUI_DOC_FILE_MARKDOWN);
/* 富文本及 Markdown 工程状态均可保存为新原生格式。 */
result = xuiDocumentSaveFile(document, "notes.xdoc", XUI_DOC_FILE_NATIVE);

xui_document opened = NULL;
result = xuiDocumentOpenFile("notes.xdoc", XUI_DOC_FILE_NATIVE,
    NULL, 0, &opened);
```

输入/输出路径为 UTF-8；文件层使用 XRT 的同目录临时文件和原子替换。Open 返回新的干净文档，没有导入撤销项。SaveFile 只接受能作为文档保存的 native/Markdown 格式；纯文本与 HTML 使用 SnapshotExportFile，导出不会修改保存点。原生 Markdown 文件必须记录 `dialect`，加载时以文件声明为准；缺失或非法方言会拒绝加载，避免根据调用方默认值重新解释源码。单独的 `.md` 文件没有这个元数据，打开时仍由调用方描述指定方言。

`XUI_DOC_EXTENSION` 可携带不透明二进制 payload：`sInfo` 指定类型，`iExtensionVersion` 指定版本（零默认 1），`bExtensionRequired` 指明它是否为必需扩展；Document 内核不解释未知类型或版本。每节点最多 16 MiB，所有扩展 payload 的合计受文档 `iMaxTextBytes` 的数值上限约束，且与普通文本字节分别计数。`xuiDocumentSnapshotCopyExtensionPayload` 按实际字节长度查询和复制，不附加终止符；`xuiDocumentTxnSetExtensionPayload` 原子替换内容与版本，可撤销。原生文件当前写出 `schemaVersion: 6`：版本 2 引入扩展 payload，版本 3 引入显式零颜色标记，版本 4 引入 `currentColor` 标记，版本 5 引入显式左对齐标记，版本 6 引入图片外层链接目标/标题；仍可读取版本 1–5，但版本 1/2 不能携带显式零标记，版本 1/2/3 不能携带 `currentColor` 标记，版本 1–4 不能携带显式左对齐标记，版本 1–5 不能携带图片外层链接字段，以免旧读取器静默改变语义。旧读取器会拒绝新版文件。原生文件、原生 fragment 和 XUI 标记的 HTML fragment 保留类型、版本、必需标志及 payload（文件/HTML 中使用严格 Base64）；格式错误、超限或 payload/必需扩展缺少非空类型时拒绝，不发布半成品。普通 Markdown 无法表达任意扩展 payload，不能把模式切换当作格式转换。

表格 Cell 的对齐由其中未设置对齐的段落继承；GFM 列的居中/右对齐因此使用同一布局路径，富文本编辑器的对齐命令状态也按这个有效值报告。富文本段落的 `text-align:left` 或 `SetBlockStyleRange` 中 `ALIGNMENT + iAlignment=0` 会设置 `XUI_DOC_ALIGNMENT_EXPLICIT_LEFT`，覆盖 Cell 或其他祖先的对齐；`bAlignmentInherited=1` 且 `iAlignment=0` 可清除此覆盖并恢复继承。Cell 的前景色参与自身 `background-color:currentColor` 的绘制；明确透明的背景保留为透明，不使用表格默认底色。

后台保存应由所有者取得 Snapshot，工作线程导出该 Snapshot；成功后回到所有者线程调用 `MarkSaved(document, snapshot)`。保存期间有新修改时，新文档仍为 dirty。不要在后台线程直接调用 live Document 的 SaveFile。

## 语义可访问性

VISUAL 模式的 DocumentView 与 DocumentEditor 通过 XUI 无障碍 provider 按文档顺序暴露语义树，节点 ID 使用稳定的 Document NodeId。段落、标题、列表与任务项、链接、图片、表格/行/单元格和扩展对象有独立角色；文字值与图片 alt 在显式查询时按节点读取。根节点支持文本选区，链接/图片沿用 View 的激活回调，任务项切换通过统一 Document 事务进入 Undo，有绘制边界的离屏节点可执行滚动到视口。SOURCE/LIVE 当前仍按单个文本框暴露。普通绘制和更新不会为无障碍树展开整篇文档；首次枚举才建立快照节点索引，文档变化后失效。

VISUAL 根节点的 `sValue` 和 `iTextStart/iTextEnd` 使用同一份 Edit 语义文本投影，根节点 `SET_SELECTION` 的局部字节偏移与它一致；报告反向选区方向，选区非空时置 SELECTED。根节点文本只在显式查询时生成，并随 Document revision 失效。这个全文值遵循 `xuiEditGetText` 的纯文本规则（包括块分隔换行）；各语义子节点仍分别报告其自身的可访问值。

Renderer 的 `xuiDocumentRendererGetNodeRect` 返回 VISUAL 节点的文档坐标边界；对未测量的前置块，离屏 Y 坐标可能采用估计高度。无直接绘制边界的元数据节点仍可枚举并标记为离屏。Text/Link、代码块及脚注引用叶节点，以及段落/标题、列表/列表项、引用、脚注、表格/行/单元格容器，报告节点局部 UTF-8 字节 anchor/caret；反向选择保留方向，跨节点选择在各节点内裁剪。段落/标题的 `sValue` 连续拼接文字和换行，内联图片等对象以 U+FFFC 占位；多块容器把相邻子块用换行连接，同行单元格用制表符连接，因此表格值按行换行、按列制表。子对象仍单独报告 alt/源码和语义角色；代码块有独立角色。`SET_SELECTION` 接受节点局部的 `xui_accessible_selection_t`，传 `NULL` 选择整个节点文本；对象占位的完整字节范围映射到对象两侧的 Document 结构位置，落在 UTF-8 字符内部的偏移被拒绝。只读 View 可以选择，禁用选区的 View 不暴露该动作。图片、公式、Mermaid、HTML 与扩展对象也暴露按节点选中动作和已选中状态。当前文本投影在节点查询时构造，未完成大文档读屏性能与真实平台辅助技术验收。

VISUAL 可访问树的 Cell 节点报告逻辑行列和行/列跨度。启用选区时，Cell 暴露可选择状态及按节点选择动作：携带局部范围的 payload 选择单元格内文字，传 `NULL` 沿用完整 Cell 的表格矩形选区；合并 Cell 一次选中其完整跨度。当前矩形完全覆盖 Cell 时，节点报告已选中；只读 View 也允许选择，禁用选区的 View 不暴露此动作。离屏 Cell 的滚动动作使用单元格自身边界。读屏平台桥接和真实读屏验收仍需后续完成。

## 验证

在仓库根目录，GCC/windres 位于 PATH：

```bat
call test_xui\build_document_suite.bat artifacts\xui-document-rebuild\commonmark-0.31.2.json
```

不传参数时跳过官方语料，仅运行本地测试。官方语料来自 `https://spec.commonmark.org/0.31.2/spec.json`；SHA256 为 `d431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20`，包含 652 项。

Windows 完整套件在 DLL 构建后运行 `test_xui/check_document_release.ps1`：逐项核对 `xui_document.h` 和 `xui_document_ui.h` 的公开函数是否实际导出，并检查活动源码、示例、测试、头文件及 DLL 中没有旧 RichDocument/RichEdit API。该检查需要 MinGW 的 `objdump` 和系统 PowerShell。

完整套件默认运行属性池 2.5 万至 20 万种属性、交错增删及人工碰撞链查找/删除门禁。百万档单独运行，避免每次常规构建都占用较多内存：

```bat
call test_xui\build_document_attribute_pool_scale_test.bat
build\document\xui_document_attribute_pool_scale_test.exe --million
```

Linux 下可独立验证纯 C Document 内核，并启用 ASan/UBSan；脚本从 `xui_document_sources.bat` 读取与 Windows 构建相同的核心源码清单：

```sh
bash test_xui/build_document_core_sanitizer.sh artifacts/xui-document-rebuild/commonmark-0.31.2.json
bash test_xui/build_document_renderer_sanitizer.sh
bash test_xui/build_document_view_sanitizer.sh
bash test_xui/build_document_shape_context_sanitizer.sh
bash test/build_opentype_carets_sanitizer.sh
```

首条命令省略 JSON 参数时只运行 Core 测试；第二条在无窗口测试代理上检查 Rich/Markdown 共享 Renderer 的布局、光标、命中与绘制；第三条从完整 XUI 源码清单构建 View/Editor，检查 Rich 与 Markdown 三模式的基本输入、撤销和绘制。后两条不接入实际 GPU 后端或平台 WebView，也不验收 PNG 剪贴板和真实 IME。

第四条验证实际段落上下文工厂、来源映射、脚本缓存与分配失败后的所有权回滚。第五条使用真实 HB 字形和项目字体数据执行产品同一份 C 停点映射，验证 LTR/RTL 连字附标、GPOS 位移、非法或多 carrier 回退及分配失败回滚；它不包含原生 GPU。第三方 HB 对象来自独立依赖脚本，测试字体可由项目生成器重建，默认保留 leak/halt 的 ASan/UBSan 检查。

完整套件还运行 `test_xui\build_document_footnote_perf_test.bat`：以约 10.9 MiB 的 Markdown 样本连续编辑已引用脚注，校验单次、同事务后续补丁和同一定义内的 Prepare 批量补丁均保持局部解析，并打印进程 CPU 耗时。该计时只用于跟踪内核退化，不替代真实平台输入与绘制 P95。

验证目标包括：独立核心、同一测试链接实际 DLL、官方语料、Renderer、Editor，以及真实 XGE 后端 Source/Live 三栏像素检查。核心测试包含随机编辑、不可变快照、并发读取、保存重载、表格跨度、过期位置、源/语义历史、分配失败逐点扫描。语料测试检验解析成功、源码保持和原生重载，**不是 HTML 输出符合规范测试**。另外对 652 项语料的 15,470 个有效 UTF-8 位置验证 LIVE 活动块与光标几何；该项也不代表排版符合规范。

本机结果、性能样本和缺项见 [验证记录](XUI_DOCUMENT_VALIDATION.md)。

## 最终验收仍未完成的项目

1. Markdown 块内 trivia/token、含内部定义容器的完整无损回写、跨不同列表的范围操作与完整对象命令、其余块转换、其他边界空白和复杂 delimiter、扩展语法来源覆盖、跨块与全局依赖的增量失效；VISUAL 待提交期间其他跨叶节点或来源不精确的编辑、结构编辑与流式 Editor 接入，以及 LIVE 跨块候选的语义混合显示。已解析依赖、未解析标签候选与未使用脚注定义已有来源记录；独立块无查找时可复用现有定义，安全的链接引用查找可借原始定义行局部解析。独立链接定义标签/目标/标题的替换、删除与插入已能保守定位按需增长的受影响顶层块，包含无内部定义的独立引用块、列表和表格；脚注、含内部定义容器及其他跨块依赖的完整失效仍未完成。Document 独立块后缀已共享并延迟平移来源，跨块编辑仍需完整解析。
2. 完整段落 shaping/Bidi、缺少自然或安全紧急切点的极长段落及代码块无硬换行长行的按需排版、其余跨容器结构终点范围与复合节点几何的分段续排及 MessageList 同块复杂度量变化锚点、其余复杂样式联动、真实平台字体、物理 DPI 与复杂同块重排下的阅读锚点；深处文本光标、文本/同段直接子节点 gap 终点选区与段内可选对象或文本叶节点矩形已有安全切点续排，虚拟 DPI 和 context 默认字体替换的固定高度 View 锚点已有代理回归，当前缓存预算只涵盖块内布局缓存。
3. 编辑器的块命令 UI、复杂原生片段、完整 HTML5/CSS、图片剪贴板跨应用矩阵和资产持久化、完整辅助技术；Windows 常见 `CF_DIB(V5)` 输入与 `CF_DIBV5` 输出已有内建测试，矩形选择可由 Alt+拖动、Alt+Shift+方向键或 API 建立，常规拖动仍是正文选区。
4. 可选 Windows provider 已让公式、Mermaid 和块级原始 HTML 在真实 DocumentView、MessageList 中静态排版、绘制和随源码事务/调色板更新；单对象脚本错误已有可见卡片并可在源码修正后恢复。独立 HTML 交互面板已验证 Rich/Markdown、脚本沙箱、焦点、裁剪、文内锚点及滚动；仍需 DPI、不同主题多视图、准确宽度与基线、启动/超时诊断、受控外部子资源加载和对象辅助技术；外链点击策略已由独立面板处理。通用 WebView 当前仅验收 Windows 基础网页承载，已有同窗口连续 12 轮、双控件独立关闭和隔离 profile 浏览器主进程异常退出后的失败通知、释放与重建测试；Document provider 已在真实浏览器主进程退出后验证排队对象的 MessageList 错误卡片及重建后重新渲染。中文 IME、物理 DPI/跨屏、主框架渲染进程失败与更大规模/跨窗口并发矩阵仍未验收；脚本/消息/截图只供 Document 内部 provider 使用。
5. MessageList 已有共享 Renderer 的正文高度、绘制、命中、跨 Document/普通文本消息的双向选区与复制、滚动和激活；多块长文档的离屏估算与可见区局部精排已实现。单个长段落消息的部分字号变更已保留深处文字行，列表与消息级语义节点已接入。其他对象和复杂样式变化下的阅读锚点、长消息估算精度、绑定 Document 内部语义子树及文本/对象选区已接入；任务项动作已接入，嵌入交互 Widget 与真实平台读屏仍需补齐；消息正文的局部 UTF-8 文本范围已可由辅助技术选择。旧富文本 API 已退出公开头和 DLL，但旧测试中尚未迁移的行为仍需以新 API 补测。
6. 真实平台 IME、读屏、多 DPI、其他平台和大文档 P95 验收。

这些缺口尚未由测试验收，不能将当前实现描述为已达到成熟前端 Markdown 编辑器的全部功能。

## 共享自然语言属性（2026-10-02）

`xui_doc_attributes_t.sLanguage` 保存 BCP 47 自然语言标签，使用与 XGE 整形输入相同的 RFC 5646 语法检查，最多 255 字节；不推断注册表成员资格。输入由内核复制，属性池按小写规范化并按内容去重。NULL/空串表示继承最近祖先，`und` 显式停止继承、使用未指定语言。`GetNode` 返回的指针由 Snapshot 拥有。语言字节放在属性的同一分配尾部，没有给每个属性增加 256 字节数组；分配故障、释放和内存计费沿用统一属性池。

Rich 的 Root、块及行内节点可保存语言。`XUI_DOC_TEXT_STYLE_LANGUAGE` 支持行内范围设置、混合状态查询和 Editor 待输入状态；`xui_doc_text_style_t.sLanguage[256]` 是独立的查询/待输入值，调用方的输入缓冲可立即改写或释放。清除格式保留语言；空语言样式清除本地覆盖，`und` 显式重置。CodeBlock 的自然语言通过节点属性设置，`sInfo` 继续表示编程语言，二者分开保存。

Markdown 支持 Root 默认语言作为文档元数据，源码替换、局部解析、Prepare 发布与 Undo/Redo 保留它，不往源码插入标记。原生格式输出 schemaVersion 7：Rich 每个 attributes 保存 `language`，Markdown 顶层保存 `language`；1–6 版的原有数据仍可读取，带新增语言字段却标成旧版本的数据被拒绝。纯 `.md` 保存原始源码，不编码这项元数据；需要保留语言时使用原生格式或宿主配置。局部 Markdown 语言覆盖仍不能从当前原生 Markdown 语法表达，相关修改被拒绝，不能把模式切换当作自动转换。

Rich → 普通 Markdown 分析单独报告 `XUI_DOC_MD_LOSS_LANGUAGE`，只有显式接受该损失位才移除语言；接受字体/颜色损失不会顺带移除语言。HTML 使用 `lang`，导入支持 `xml:lang`，空标签转换为 `und`，非法标签忽略并继承；代码元素的自然语言与 `data-language` 编程语言分别读取。Root 导出使用可识别的 `data-xui-document` 容器，复制或剪贴板片段进入另一语言环境时保留源语言，包括原来由祖先继承的值。

Renderer 的首次投影整形、联合分组、绘制和 SOURCE/LIVE 原始行使用同一解析语言；同字体的语言边界也拆分 item，跨节点字素继续使用首个可见片段的语言。语言变化使测量缓存失效；Root/无独立块的祖先样式会重建受影响目录；共享源码的待提交输入只有在默认语言一致时才可直接接管。绘制分组从当前 run 读取语言，不额外借用旧属性指针。带显式语言的长 ASCII 源码/代码行暂时保留完整行整形，以保证内容相关字形与高度正确。

共享语言语义不等于任意后端都有本地化整形能力：无 HB 的最小代理仍拒绝显式语言请求；真实五平台字体、复杂局部 Markdown 语言语法及性能矩阵继续验收。通用 WebView 仍限 Windows 基础网页承载，Document 私有公式/Mermaid/HTML 通道保留。

## 文本输入能力契约（2026-10-02）

`xuiGetProxyCaps` 现分别报告 `XUI_PROXY_CAP_TEXT_CONTEXT`、`XUI_PROXY_CAP_TEXT_SCRIPT`、`XUI_PROXY_CAP_TEXT_LANGUAGE`、`XUI_PROXY_CAP_TEXT_RTL`，表示五个文本回调支持对应输入；它们是可选能力，普通 XUI Context 不要求全部具备。能力在 `xuiSetProxy` 时缓存；没有 `textShape` 的测量式代理会清除这四位。原生 HB 构建报告四位，无 HB 构建不报告。字体实际拥有的字形、GSUB/GPOS 或语言规则仍由字体决定，能力位不保证所有字体/脚本质量。

`xuiTextShape` 先校验输入，再依据缓存能力拒绝不支持的显式字段，不能因后端回调宽松而静默忽略；非法输入仍返回 INVALID_ARGUMENT。Document 自动解析的非 ASCII context/script 可以按能力选择是否附带，最终绘制从同一已测量 group 复用该选择；不改变保存的内容、来源和有效语言，也不去掉语言或 RTL 来假装成功。因而最小原生后端可以继续显示普通非 ASCII LTR 文本，显式语言及 RTL 文档仍可能返回 UNSUPPORTED。这项降级不宣称复杂文字整形质量、ASCII 跨 item 上下文或五平台验收完成。

## 软连字符的段落上下文缓存（2026-10-02）

U+00AD 在普通显示段落中仍不可见；被断行选中的位置仅为当前 item 插入 `-`。具有非 ASCII 脚本缓存且支持 context 输入的段落，在所有者发布前准备一份可复用的插入缓冲；绘制组只保存插入位置，不再各自保留完整段落变体。基础显示文本、来源映射和脚本图保持不变，缓存预算只对该缓冲计费一次，重排复用它。ASCII 或不支持 context 的后端不分配这份缓冲。

测量、Spans 绘制、普通绘制和颜色裁剪均在同步文本回调期间借用变体，回调返回后释放借用。连续位置通过移动两位置之间的字节更新，不重新复制整段，也不增加常规分配。若嵌套回调请求不同位置，则临时复制变体，避免改写外层仍在使用的文本；临时分配失败返回 OUT_OF_MEMORY，保持外层数据并允许重试。这是文本缓冲的借用保护，不是对任意重入修改 Renderer 几何或多线程绘制的授权。

正式回归使用 36,864 字节 Rich/Markdown 段落与两种绘制后端：原前缀保留 910 份全文变体、共 26,093,340 字节；现在仅保留 28,674 字节缓冲。完整 4096 个断行位置继续共享一份缓冲；逐字节上下文、深处 caret 与完整排版、改宽、颜色边界、整形失败重试和快照生命周期保持。工厂测试另外覆盖逐分配失败、嵌套借用、UTF-8 边界，以及关闭分配后的连续插入位置扫描。

上述内存上限不代表任意调用顺序都有线性时间：交替访问远距离位置仍需移动相应区间，嵌套不同位置需要临时全文副本，第三方塑形本身的上下文处理也另行计费。纯 ASCII 跨 item 上下文、完整控制整形语义、跨样式整词字体选择和其他整体缺项继续验收；本批不改变公开 API、代理版本 13 或原生 schemaVersion 7。

## 整形结果与范围绘制缓存（2026-10-02）

Document 的现有单 run 缓存分支过去保留整段整形宽度，但绘制软换行子串时又调用整形器；具有 ASCII 上下文 GSUB 的字体会让绘制字形与 caret、选区的几何不同。HarfBuzz 的普通 pre/post Unicode context 不等于把邻居字形加入 GSUB 查找，单纯附带 `sContext` 无法修复此问题。

本批引入代理版本 **14**，后续共享范围接口已升为 **15**，均需重编译代理与调用方；原生 schemaVersion 保持 **7**。`XUI_TEXT_SHAPE_RETAIN_PAINT` 是可选请求，后端可在 `xui_text_shape_t` 返回不透明 `pPaint`、自有存储字节数 `iPaintBytes` 与 `paintFree`。该结果保存整形时的字形和位置，输入字符串不必继续存活；`xuiTextShapeFree` 同时释放几何数组与绘制载荷。字素停点细分不改变载荷的原始输入字节坐标，删除投影则暂不接入这条路径。

可选 `drawTextShapeRange` 按原始输入字节范围绘制完整 cluster，不再次整形；目前原生实现接受 LTR、左上定位、CLIP/UNDERLINE。内部二分定位字形边界，只准备所选范围的 atlas 字形。若范围切在连字内部、包含强制换行，或要求其他尚未实现的方向/定位，返回 UNSUPPORTED，且不提交像素或改变裁剪状态，调用方使用现有文本绘制。没有载荷或回调的后端继续使用原路径。

Document 仅为不含删除格式、强制控制或非 ASCII 的可打印缓存 run 请求该载荷；在未由行级 paint group 接管的绘制分支复用它。字形数组、停点、原生私有容量、字素/脚本缓冲与载荷本身计入布局预算，共享字体/atlas 不重复计费。原生载荷持有字体 wrapper 生命周期：调用方销毁字体后，主字体与 fallback 的地址仍保持到最后一个载荷释放，避免 XGE run 的引用计数与借用字体地址产生悬空。

验证包含项目生成字体的 Rich/Markdown 软换行与两侧 affinity、改宽重排、完整 RGBA 字面字形对照，以及绘制期间零整形调用；9 个原生分配失败点均可重试，实际分配字节与统计完全一致，最终无残留。无 HB 后端也支持缓存基本字形，并通过字体释放后的完整 RGBA 对照。跨颜色/字体节点的共同字形缓存、行级重整形的完整边界语义、非 ASCII/删除投影/RTL 范围绘制仍需继续，不把本次缓存分支修复视作完整段落整形验收。通用 WebView 的平台和通信范围保持既定收缩，Document 私有渲染保留。

## 跨样式共享字形与行范围（2026-10-02）

同一字体、字号、语言、解析脚本和方向的 span 现可共享完整整形结果。文字颜色和普通装饰不增加整形边界：Rich 的颜色节点及 Markdown 的粗体/斜体节点在解析到同一字体时，共用字形、逻辑 advance 和字素停点。字体、垂直样式、语言、脚本或方向改变仍产生独立 span。当前原生保留请求限于可打印 ASCII、无删除投影、无 SHY 和 LTR。

代理 ABI **15** 新增两个可选回调：`textShapeRangeMeasure` 查询原输入内完整 cluster 范围的绘制宽高；`drawTextShapeRangeSpans` 直接绘制该范围，接收完整输入坐标中的颜色 spans 及有限的 [-0.5,0.5] X 偏移。测量不提交像素，UNSUPPORTED 清零输出；绘制的 UNSUPPORTED 在改变裁剪/提交像素前返回。原生仅支持左上定位及 CLIP/UNDERLINE，RTL、强制换行与连字内部切点继续拒绝。普通 `drawTextShapeRange` 委托此入口，偏移和 spans 为零。

Document 的 width-independent seed 拥有跨节点字形结果；完整单 run 则按索引借用既有结果，避免复制同一载荷。行绘制组保存 seed 索引和原输入字节范围，seed 数组扩容不使引用悬空；片段边界索引避免每行重新扫描全文。改宽保留 seed，真正切入连字内部的行才回到行级整形。缺少可选回调或无有效载荷的后端沿用既有路径。可选查询报错中止布局；新 owner 和 offsets 均随失败、淘汰及 continuation 回滚释放。载荷、几何数组、advance 表和边界表计入缓存预算。不同节点原有几何结果仍保留并计费，不声称整个 span 只有一份所有几何数据。

颜色 spans 使用原输入坐标，支持连字内部按停点分色；普通 DrawText-only 后端按颜色裁剪同一共享字形，保持分数 X 起点。纯颜色修改同步布局缓存的修订号，后续改宽不会因为旧号重新整形。原生专项使用独立 PUA glyph 的完整 RGBA 对照验证这些行为；Linux headless 专项验证 joint owner、整形/范围查询错误清理、重试、改宽、范围偏移和快照释放，不代替 GPU 验收。

Markdown 任意文字颜色不能由现有 Markdown 源码表达；此批 MD 验证针对粗体/斜体节点共用字体，颜色修改回归针对 Rich。跨字体/字号的完整上下文、非 ASCII/RTL/删除投影的共享字形、行边界的完整脚本语义、整词字体选择、完整 CST 和真实五平台验收仍未完成。通用 WebView 仍为 Windows 基础网页承载，其余平台空后端；Document 私有公式/Mermaid/HTML 渲染通道保留。

## Unicode 最终行的字形复用（2026-10-02）

实际完成整形且与片段映射一致的最终行组，现保留该次整形的字形结果。未能引用 ASCII shared seed 的 Unicode、RTL、显示投影及选中 SHY 行组可使用自己的 `shape`；Renderer 将完整行输入范围和原颜色 spans 传给范围绘制，普通文本颜色裁剪也复用该载荷。行组结果只属于已经确定的这一行，改宽后重新确定的行组会释放旧结果；此前的 ASCII whole-span seed 继续跨宽度复用。没有保留结果或可选绘制入口的后端仍走原路径，UNSUPPORTED 回退到同一行输入，其他错误直接返回。

原生范围接口现在支持整个 RTL shaped item，以其现有 item-local visual positions 绘制和查询宽高。部分 RTL 范围仍在提交像素前返回 UNSUPPORTED，查询输出清零；不以简单裁剪冒充正确的 RTL 范围重定位。代理 ABI 保持 **15**，原生 schemaVersion 保持 **7**。

原生整形完成后释放同步阶段的 Script_Extensions 和字素图，清除借用的 fallback 词边界指针。已发布 glyph/caret 数组足以支持绘制、命中和范围查询，不保留全文临时脚本图或原输入地址。行载荷、cluster/caret 数组纳入现有缓存计费，布局失败、行重排、continuation 回滚和块淘汰均释放 owner。

独立 PUA glyph 的 180 个完整 RGBA 场景涵盖 Greek 脚本括号、Kana locl、Arabic 连写 RTL、Rich 整段/分色、Markdown VISUAL/SOURCE/LIVE、两种字号、换行、裁剪、分数 X、颜色缓存和释放 Document；行缓存绘制期间无原字符串绘制或再次整形。实际 memory-debug 验证 4 KiB/1 MiB Unicode context 的两字节行均只保留 328 字节原生载荷，原 context 释放后范围查询仍有效；不把该数值视作任意字体或完整 Document 的内存上限。

这一步统一最终行的几何和绘制，尚未让非 ASCII/RTL 或跨字体 span 复用完整段落的同一字形结果。完整段落边界语义、跨样式整词回退、完整 CST、复杂导航、性能矩阵和五平台实机验收继续；通用 WebView 仍限 Windows 基础承载，Document 内部公式/Mermaid/HTML 渲染保留。

## Unicode LTR 共同 span 的上下文字形（2026-10-02）

无删除投影、无 SHY、无需 Bidi 分析的 LTR span，现可保留同字体/字号/语言/解析脚本的完整 Unicode 字形结果。颜色和普通装饰仍不切断 span；换行按 UTF-8 原输入中的完整 cluster 范围引用共同结果，不把 GSUB 的邻居重新缩成孤立子串。连字内部断点继续使用单独的最终行结果。改宽保留共同 owner 和范围索引，新的行组重新选择范围，计费与释放沿用统一机制；ABI **15**、原生格式 **7** 不变。

完整单 run 的结果仅在段落没有脚本图时借用；含 Unicode 的段落可能把单独的 ASCII 括号解析成 Greek 等脚本，不能借用其孤立 LATN 整形来代替段落上下文。此时即使 span 只有一个 run，也按该 span 的段落 context/script 生成共同结果。字体改变仍分隔 span，原有跨字体字素选择策略保持，不宣称已经完成跨字体整词共同字体选择。

独立生成的 Greek GSUB/连字字体保留字面 PUA aliases：旧 DLL 的 `λ μ` 窄行 λ advance 为 16，应为 32；当前 Rich 整段/分色、Markdown 强调、宽窄切换、连字完整分色和内部断行在两种绘制后端的 **76** 个完整 RGBA 场景通过。另有 **24** 个双字体括号完整 RGBA 验证 Rich 字体族和 Markdown 粗体边界的 Greek 脚本，包含换行、裁剪、分数 X、Document 释放及绘制零整形。字面 glyph 参考独立于被测上下文子串；这些固定字体测试不代替真实字体/平台矩阵。

新增真实 Document 内存调试发现 Renderer 把 `xrtMapInit` 初始化的嵌入式/栈上索引用 `xrtMapDestroy` 释放，产生非法释放诊断。全部相应路径改为 `xrtMapUnit`，只释放键值与桶存储，不释放所属结构。14 个 Unicode Document 布局分配失败点均可清理并重试，保持 λ caret 32，释放每个 Renderer 后 live storage 与基线相同，纯整形/Document 阶段最终零 live allocation、零 invalid/double free。

段落含 RTL/Bidi、删除投影或 SHY 时仍使用既有最终行整形/载荷路径。完整控制语义、跨字体/字号上下文与整词回退、完整 CST、复杂导航、性能和五平台实机验收继续。WebView 平台及公开基础功能范围保持，Document 内部渲染通道保留。

## RTL/Bidi span 共同字形与缓存分色（2026-10-02）

共同字形已扩展到按 UBA 分析的 LTR/RTL span，仍以同字体、字号、垂直样式、语言、解析脚本和方向级别划分。完整 cluster 的子范围可复用共同 glyph/caret/font 结果；RTL 按选中范围最后一个逻辑 cluster 的首个 glyph 计算左起点，绘制起点相应平移，原字形和原 UTF-8/颜色偏移不修改。平移后的起点不另做整数吸附，避免破坏分数 X；mark 在 cluster 内的位移保持。完整 item 仍使用原位置，连字内部切点和包含强制换行的范围仍在提交像素前返回 UNSUPPORTED。ABI **15** 和原生格式 **7** 保持。

UBA L1 会在行尾重置空白的方向级别。行组仅在当前级别与 seed 一致时共享结果，方向已改变的空白独立整形；同一 seed 的相邻有效字形仍可共享。Bidi 段落不借用孤立 run 的 shape，避免方向或段落脚本不一致。改宽、颜色修改、释放 Document 后继续显示、计费及失败清理沿用统一机制；删除投影和 SHY 的共同 span 路径尚未启用。

缓存字形的颜色 spans 与原字符串的 `drawTextSpans` 是独立能力。只要 retained range 回调和载荷可用，Renderer 优先直接分色，避免因关闭字符串 spans 而退到空间裁剪。没有缓存分色能力时保持原空间裁剪降级；这条降级不能承诺相邻样式边界的重叠墨迹与逐字形分色完全相同。UNSUPPORTED 后才选择可用的普通降级，真实错误保持传播，不调用空回调。

新的自有 Hebrew calt/连字字体含独立 PUA aliases 和固定时间戳。旧 DLL 的 `א ב` 窄行 א advance 为 16，完整上下文应为 32；当前两配置各 86 个完整 RGBA 验证 whole/split Rich、Markdown 强调、RTL 连字分色/内部断行、五种范围重定位、原输入改写/字体先释放、分数 X、裁剪、改宽缓存，以及 `i א ב i` 的独立 L1 行尾位置。配置分别为字符串 spans 开启/关闭，二者都验证原生缓存分色；不把后者冒充没有任何分色能力的代理。

原 180 个 Unicode 最终行画面保留，并新增 60 个 Hebrew Rich/Markdown VISUAL/SOURCE/LIVE 画面，共 **232** 个本批新增精确 RGBA 场景。绘制均不得重新整形或调用原字符串；Linux portable 同时覆盖 RTL owner、shape/query 错误/重试和关闭字符串 spans 的缓存路径。真实 memory-debug 新增 **46** 点 RTL Document Layout 分配故障，重试后 caret=10，释放每个 Renderer 后 live storage 与基线相同；原 Greek 14 点、base/context 9/10 点及 GPU spans 故障回归保持，纯整形/Document 最终零 live/invalid/double free。

这完成同一解析 span 的 RTL 范围复用，不等于完整控制、跨字体/字号上下文、整词回退或所有行边界语义已完成。完整 CST、多 carrier/device/variation、复杂导航、性能和真实五平台验收继续；通用 WebView 仅 Windows 基础承载，其余平台空后端，Document 内部公式/Mermaid/HTML 保留。

## 删除投影的共同字形与原文位置（2026-10-02）

已有显示过滤策略删除的隐形控制字符，现在也能在同一 LTR/RTL span 内复用完整 glyph/caret 结果。原文、节点文字、语义位置与 SOURCE 偏移不变；seed 的每个片段边界改用过滤后的显示 UTF-8 字节，隐形片段前后边界可以相等，宽度为零。段落 source_map 仍把原文映射到显示上下文，不把显示索引写回 Document。完整 cluster 才共享，连字内部切点继续采用独立最终行整形；UBA L1 的方向级别检查保持。ABI **15**、原生格式 **7** 不变。

删除投影不能借用原 run 的原文字节 shape，否则原文与显示范围不一致；它拥有按过滤后输入生成的共同结果。前导隐形片段可以来自其他字体，seed 的字体/垂直样式键采用第一个实际可见片段，与 paint_input 相同；只有全空 span 才使用首片段的字体。字体、字号、垂直样式、语言、解析脚本和方向级别仍是可见 span 边界，颜色与普通装饰不切断它。

项目自有 Greek/Hebrew GSUB 字体的字面 PUA glyph 对照覆盖 ZWSP、WJ、BOM 和 LRI/RLI/PDI，Rich 整段/分色与 Markdown VISUAL/SOURCE/LIVE、40/37 字号、宽窄布局、原文映射、点击命中与光标回映、隐形字体、颜色修改、裁剪、小数 X 和 Document 释放；再覆盖删除控制后形成连字的完整分色及内部断行，两种绘制配置共 **504** 个完整 RGBA 场景。绘制不得调用原字符串或再次塑形。真实 Native 内存调试新增 Greek/Hebrew **21/53** 个布局分配故障点，重试后的 caret 为 32/10，每次释放 Renderer 后 live storage 与基线相同。

VISUAL/LIVE 在本批重排中复用共同 owner；SOURCE 行仍按已有策略在改宽时丢弃缓存，源码模式也保持代码行的水平布局规则。此批不改变控制字符的删除政策、PS 段落语义或 SHY 选择性插入；含 SHY 的组仍使用既有最终行路径。所有复杂控制、跨字体/字号上下文与整词回退、完整 CST、多 carrier/device/variation、复杂导航、性能矩阵及五平台实机验收仍需继续。通用 WebView 仅 Windows 基础网页承载，其他平台空后端，Document 内部公式/Mermaid/HTML 渲染通道保留。

### SHY 基础字形与选中行变体（2026-10-02）

含 SHY 的 span 现在也保留过滤后的共同 glyph 结果。未选中的 SHY 占零显示字节，原始 UTF-8 位置不变，显示范围索引允许重复边界；完整 cluster 子范围继续共享 owner。选择 SHY 的行按插入 `-` 后的真实输入重新塑形并拥有独立结果，不把插入 glyph 写进基础缓存。字体、字号、语言、脚本和方向级别的分组边界保持，颜色仍属于绘制属性。ABI **15**、原生格式 **7** 不变。

候选与发布共用“本行确有显示前缀”的插入条件。过宽 SHY 被候选测量拒绝后，强制消费第一个完整字素时不会再次强行插入连字符；下一行仅含删除控制时也不产生孤立连字符或额外显示行。单个不可再分字素仍可超出行宽。

终端 glyph 可改变前文字宽，全局重排因此可能来回切换。八次快速重排后的有限精确回退，测量未插入的完整范围时使用同一基础 seed；它不再用孤立子串宽度选断点，再发布更宽的共同 glyph。范围查询的真实错误向调用者返回；仅 UNSUPPORTED 允许独立塑形降级。选中行仍按真实插入变体测量。

选中连字符使其前面的 WS/BN/format 尾部不再属于行尾空白。候选塑形及最终几何读取同一方向级别修正；分类来自现有 SheenBidi 属性，段落方向结果不可变，S/B 分隔符保留 L1 重置。普通未选中行继续执行 L1，级别改变的空白独立塑形。此处不是对全部控制组合重新运行显示流 UBA 的完整实现。

新增自有固定字体及两种配置的 1440 个完整 RGBA 场景覆盖 Greek LTR、Greek RLO 和 Hebrew、可缩窄/过宽的终端替换、SHY 三个位置、Rich 整段/逐标量分色、Markdown VISUAL/SOURCE/LIVE、40/37 字号、改宽、颜色修改、源码映射、caret/hit、裁剪、小数 X 和释放 Document。参照为独立字面 PUA glyph 与预设 advance；绘制不得重新塑形或调用原字符串。普通字符串 spans 关闭的配置仍具备缓存 range spans，不代表完全没有缓存分色能力的后端。

新增 Native 分配故障覆盖未选中 Greek/Hebrew、选中 Greek、拒绝 Greek 和选中 RLO 共 231 点，逐点重试并核对公共 caret 及相同 live storage。可移植测试另覆盖重复显示索引、shape/query 错误清理、UNSUPPORTED 降级与实际八次重排回退中的 terminal-shape/query 失败。完整发布、Linux 与 Document 内部 Web 的最终结果见本批验证记录。

实际无 HB DLL 另有两配置各 80 个完整 RGBA 场景，使用直接 cmap 字宽和字面 glyph，验证基础路径选中/拒绝 SHY、相同缓存/颜色/caret-hit/source 与生命周期条件。该后端不期待 calt 替换，插入行拥有独立结果但不创建不受支持的 context recipe。固定字体的两次独立路径生成一致，原七份 fixture 不改；完整 Windows suite、Linux Renderer/View/Bidi sanitizer 和基础/私有 Web 渲染均已通过。

当前插入只重建选中行，后续行继续引用未插入的基础 glyph。独立整词输入 `λ-μ` 的最终 Native 对照给 μ advance=12，而当前后续行保留 36；本批画面断言检查当前行契约，不能替代完整插入变体的整词 GSUB/GPOS 一致性。此差异已列为下一项内核正确性工作，须传播完整变体、验证相应断点收敛及独立画面后才能验收完整 SHY 语义。

仍需完成跨字体/字号的完整上下文和整词回退、全部控制与行边界语义、完整 CST、多 carrier/device/variation、复杂导航、性能矩阵及真实平台验收。通用 WebView 保持 Windows 基础网页承载和其他平台空后端，Document 私有公式/Mermaid/HTML 通道保留。

### SHY 完整插入模式与后续行传播（2026-10-02）

具备共同字形和缓存范围接口的布局路径，现为选中的 SHY 集合建立不可变 `doc_render_paint_variant`。它拥有完整显示文本、原始控制流替换后的 Bidi 分析、两套片段字节边界、均匀字体/字号/垂直样式/语言/脚本/方向 span 的字形和范围索引。同一段落的选中行与后续行共同引用该结果，未选中基础 owner 仍可在改宽时复用；颜色不是塑形边界。ABI **15**、原生格式 **7** 不变。

候选只加入已经选定的前行 SHY 和本候选的末尾 SHY。尝试不换行的另一个候选时，不带入仅暂时考虑的连字符，避免插入后的窄字形错误地使未插入文本也被接受。全部断点确定后，用最终完整模式再测所有行；若后续插入使前面的选中行过宽，禁止该标记并重试，普通行可以收紧到更早的合法边界。禁止集合只增长、行上界只减小，每行至少消费一个片段。无法再分的 Unicode 单元仍允许溢出，但不能强行恢复已拒绝的连字符。

绘制引用完整变体的 cluster 范围。连字内部断点或 UBA L1 改变方向的行范围，继续以同一完整变体上下文取得独立行结果；真实测量/塑形错误直接传播，只有 UNSUPPORTED 走能力回退。空字形范围不向代理传入空 paint。选中的零 advance 连字符也保留精确零值，不恢复成预估连字符宽度。变体的文本、方向所有者、范围数组和 glyph 只计费一次；失败清理和块释放覆盖整组，续排回滚保留旧所有者链头。

后面的 SHY 可以通过 OpenType lookahead 改变前行字形，含 SHY 且有缓存范围接口的长段落因此先确定完整模式，再发布段落布局。普通段落的按需续排保持。这个正确性取舍没有证明多 SHY 的大文档耗时已达目标：当前多次候选可能反复塑形完整 span，仍需独立性能验收。

公共 API 的独立参照 `λ-μ` 给后续 μ advance=12；本次 Document 也为12，上一批 DLL 为36。新多 SHY 样本还覆盖跨空格的替换，完整 `λ-μ λ-μ` 的 advance 为20/6/36/10/32/6/12，不能按两个独立词的20/6/12简单拼接。原生完整像素、光标和缓存断言相应改用完整插入参照。另一个固定字体审计让第二个连字符把第一字形加宽至46，缓存路径会拒绝第一个 SHY并保留第二个；完全关闭缓存范围接口的路径仍错误地保留两个标记，该审计的退出码为1，属于明确的剩余正确性工作。

本节替代上一批关于缓存路径“后续行继续基础 glyph”的当前限制，历史记录不改写。完全没有共同缓存范围接口的后端、完整能力回退语义、全部控制组合、跨字体整词回退、完整 CST、复杂导航、性能及真实平台矩阵尚未完成。通用 WebView 仅 Windows 基础网页承载，其他平台空后端；Document 私有公式/Mermaid/HTML 渲染通道保留。最终验收与实际退出码见最新验证记录。


### 基础文本接口选择完整字形范围（2026-10-02）

ABI16新增完整输入 RANGE、同步借用字形和颜色范围，使缓存范围回调缺失时仍能保持 Document 的基础 span、完整 SHY 模式与实际绘制一致。接口与所有权、无持久 paint 路径、原生及无 HB 验证边界见 [基础范围接口说明](XUI_DOCUMENT_TEXT_RANGE.md)。整体长期目标仍在进行，通用 WebView 与 Document 内部渲染边界保持。

## 多层结构边界的引用包裹

引用包裹现使用稳定 ChildId 边界锚点，逐层拆分 Quote/List/ListItem，可处理不同深度的两端和列表项部分子块，并保留任务标记、Rich 容器元数据与可表达的未选来源边缘。结构预检、编号规则、有限来源重写与验证范围见 [多层引用选区](XUI_DOCUMENT_QUOTE_RANGE.md)。完整 CST、其余复杂结构和整体平台验收继续。

2026-10-03：根层引用包裹的 Quote 边缘现支持原字节前缀补丁，保留完整选中块和隐藏链接定义。实现边界与证据见 [引用源码保护](XUI_DOCUMENT_QUOTE_SOURCE.md)。


## 嵌套引用前缀与稳定结果选区

240 个来源场景、80 个实际 DLL 编辑器、12 个深度场景和 20 个五次包裹复合事务已验证；5,139 个分配失败点和 2,413 个取消检查点保持旧文档与分配平衡。已有 Quote/ListItem 的多行链接定义及全部可见子块前缀、选区外脚注来源、Rich/Markdown 结果 GAP 重定位见 [实现与边界](XUI_DOCUMENT_QUOTE_PREFIX.md)。本批 Windows 完整发布、最终 DLL 与已修复公共复现、Linux Core/Renderer/View sanitizer、两份 GCC analyzer、生产接口边界和 5 项浏览器串行程序实际退出 0；三张新 PNG 已实际检查。652 项 CommonMark 为 parse/source/native-roundtrip。证据使用 validation-2026-10-03-quote-prefix-，最终 Native SHA256 为 3bf81a989e7d7d0c69e620fa8a335f10acf0e14194b8647c4b79a8473f72d9a0。选区内部未使用脚注定义的 -107 拒绝保留为下一项 K2 工作，源码/tree/syntax/revision/history 原子。通用 WebView Windows 基础承载、其他四后端暂缓，Document 私有渲染与独立 HTML 交互保留；整体 13 包目标继续。
