# Document 文本范围的可见矩形裁剪

更新：2026-10-09。四配置祖先/窗口裁剪专项已通过，完整重构继续进行。

## 当前接续：统一段落范围已通过

2026-10-09，v812—v829：通用文本范围新增 PARAGRAPH 的扩展、范围移动和端点移动，富文本 VISUAL 与 Markdown VISUAL/SOURCE/LIVE 四配置通过。VISUAL 采用同一语义树的 Paragraph/Heading/CodeBlock，保留显式空段落；内部 SoftBreak/HardBreak 和视觉折行不拆段落。SOURCE/LIVE 的范围投影仍是完整原始源码，按物理空行分组并把后续空行归前段；它不是按 Markdown 语义块或 LIVE 绘制行分组，模式切换后边界可随投影域改变。

段落索引是 View 现有 plaintext projection 的派生缓存，随文档版本、投影域和文档替换失效，未增加第二套 Document 或历史。字面黄金、CRLF/UTF8、表格内多段落/单元格分隔、代码块、模式切换/重载、EOF和极限计数通过。四配置各覆盖缓存分配失败的原子性、重试、重复查询无分配和零泄漏；两View共享文档的外部编辑、Undo/Redo、替换/销毁通过。1/10MiB SOURCE 两段落在开头、分隔符、下一段和 EOF 的暖缓存查询不再分配；尚未证明整段投影按视口有界、首次建立固定时延或非ASCII大文档性能。

最新集成候选：`E:\codex-builds\xge-document-paragraph-integrated-20261009-v827-01a09446`，1681来源/脚本/规则输入、180变化、109新文件、1726物理输入，主1572保护输入不变，未合入。v829独立接受16条产品命令，其中14条使用修正后的当前测试、2条是生产输入逐字节相同的两端137单元全新/零复用正式构建。初次历史夹具误用默认语义事务，三端实际运行1的日志/产物保留；仅改为正确的SOURCE事务，产品及黄金断言不变。Linux原View的ASan/UBSan/泄漏、两端完整renderer、Windows两个基础UIA客户端及原Windows永久bat通过，五份匹配运行依赖接入。原正式DLL的段落负例编译0/运行1，错误码记录器首次预设数值错误也保留。详见 [段落范围](XUI_DOCUMENT_PARAGRAPH_NAVIGATION.md)。

FORMAT/PAGE、Prepare generation范围、属性/可见文本范围/子对象、完整UIA TextPattern/事件/Grid/Table、重入和总体缓存规模仍未完成；本次不关闭11个工作包。

v830在确切进程句柄/镜像与创建时间观察时确认v799完整Windows仓库入口和v794原112组诊断仍运行，尚无终态。完整入口绑定此前177项冻结候选，不能作为本180项的全仓库接受；原诊断31组/111090点完成，不能代替当前产品验收。以下历史批次只对应各自输入和观察时刻。

## 当前行为

`src/xui_document_accessible_text.inl` 的 `getRects` 使用同一 Document 的跟踪范围、plaintext projection 和 renderer。矩形转换到 XUI context 整数像素后，只报告窗口视口、控件内容区和所有 CLIP/HIDDEN 渲染祖先内的可见部分。VISIBLE overflow 容器保留窗口范围。父链采用实际渲染的 pParent；Popup 焦点 owner 不会被误算为额外渲染祖先。

隐藏控件/祖先、脱离 root/overlay 控件树、零视口/容器均返回成功和0个矩形，调用方缓冲不变。disabled 控件仍可被读取。可见范围裁剪不删除文档内容，getText 在隐藏或脱离时仍可读取原范围。完全不可见时不进入 renderer 的范围几何枚举；这不等于全文 projection 已按窗口流式化。

布局前后重新计算可见裁剪，滚动偏移只用于文档到context的转换。祖先裁剪不改变根事务、历史、来源位置或范围映射。空间查询仍不负责重叠窗口的视觉遮挡，也没有实现原生UIA TextPattern。

## 永久黄金和真实失败

`test_xui/xui_document_text_clip_cases.h` 对富文本 VISUAL 和 Markdown VISUAL/SOURCE/LIVE 四种配置设置独立的窗口、父容器和控件坐标，核对固定的几何边界：

| 情况 | 预期 |
| --- | --- |
| CLIP 子容器 + HIDDEN 祖先 | 所有矩形不越过子容器右边70和底部75 |
| 子容器改为 VISIBLE | 祖先右边100仍有效 |
| 两个祖先均 VISIBLE | 仅窗口右边160有效 |
| 窗口缩为55×38 | 不越过窗口右边55和底部38 |
| 隐藏父容器或Document | 零矩形、缓冲未改、内容仍能读取 |
| disabled但可见 | 仍返回可见文字矩形 |
| 脱离/重挂内层容器 | 脱离时零矩形，重挂后恢复 |
| Document滚动 | 仍符合祖先边界 |
| 零窗口或零容器宽度 | 零矩形且缓冲未改 |
| 容量0/1/32 | 总数一致，容量外的sentinel未改 |

原正式DLL通过编译后实际运行失败，返回 `[40,19,168,14]`，右边208超出固定边界70。原日志、exe、DLL和负例证明保留在 v802；夹具源字节保存并与运行前散列核对。最初的v801仅是整型诊断格式导致的编译失败，没有启动产品。最终黄金保持同一边界和断言。

## 实际接受

最新候选：`E:\codex-builds\xge-document-text-clipping-integrated-20261009-v807-01a09446`，1679来源、178变化、107新文件、1724物理输入；尚未合入主源码。

接受记录：`build/source-document-footnote-local-reference-20261006/document-text-clipping-v809-paired-acceptance.json`。

- Windows/Linux各137正式单元全新编译、零复用，五份匹配运行依赖接入。
- 共16条实际产品命令：Windows正式库及范围/renderer/两个实际基础UIA客户端9条，Linux正式库及范围/renderer5条，原Linux View sanitizer1条，原Windows永久bat1条。
- LinuxASan、UBSan、泄漏参数保持；原View的两个PNG失败即abort stub未替换Document实现。实际ext4、单链接、源字节、产物和导入/导出、日志均独立核对。
- Windows原生UIA图片与之前实际查看图片逐字节相同；这不是物理读屏或完整TextPattern接受。

## 尚未完成

1. 当前178项候选的完整仓库/Editor和合入后验收。正在运行的v799完整入口绑定此前177项候选，其结果须按依赖范围使用。
2. Prepare generation、回调重入/销毁、属性/可见文本范围/子对象、UIA TextPattern/事件/Grid/Table。
3. 整段projection和几何枚举的内存规模、深祖先性能、复杂非ASCII/Bidi/对象组合及固定机器性能。
4. 真实读屏、IME、DPI/主题/GPU矩阵以及最终迁移。

本专项不关闭 E3/R1 或整体11个工作包。
