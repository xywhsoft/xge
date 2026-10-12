# Document 统一文本范围的段落操作

更新：2026-10-09。PARAGRAPH 四配置专项已通过；完整重构继续进行，产品候选尚未合入主源码。

## 行为与文档边界

富文本 VISUAL 和 Markdown VISUAL 的段落单位来自现有语义树中最近的 Paragraph、Heading 或 CodeBlock。视觉折行、SoftBreak、HardBreak、代码块中的空行不创建新段落。显式空 Paragraph 独立归属；多个段落的单元格保持各自边界，末段的单元格 TAB/行 LF 属于前段。

Markdown SOURCE/LIVE 的可访问文本投影是原始源码；段落按物理空行分组，内部非空行属于同一单位，后续空行归前段，起始空行单独成组。空白行只含 ASCII 空格或 Tab；原 CR、LF、CRLF 字节保留。代码围栏、标题或列表若没有空行，SOURCE/LIVE 不额外按 Markdown 语法拆分。LIVE 绘制采用混合行，段落范围仍查询源码投影。这一差异属于投影域约定，不是第二套文档。

EOF 扩展到最后一个有内容的单位，空投影范围为0..0。Expand/Move/MoveEndpoint 复用既有端点排序、跟踪与交叉折叠；零计数保持范围，INT_MIN/MAX 在真实文档边界停止。分配/查询失败不修改范围，实际移动数保持0。未发布 Prepare 范围目前仍明确不支持。

## 实现

`src/xui/document/xui_document_accessible_paragraphs.inl` 为当前 View 的 edit_projection 建立派生边界索引。两次读取先计数再填充，使用 Document 的 allocator 一次精确分配；仅在完整成功后发布缓存。缓存按偏移二分查找，后续查询不再分配。`doc_view_invalidate_edit` 先释放索引，再释放投影和快照；文档版本、投影域、替换与销毁遵循现有 View 生命周期。

没有引入新 Document、语义 NodeId、事务或撤销历史。现有 GRAPHEME/WORD/LINE/ALL、查找、renderer 和基础 UIA 客户端保持回归。

## 独立黄金与故障检查

| 场景 | 字面边界 |
| --- | --- |
| Rich折行与显式空段落 `abcdef\n\nxy\n` | 0, 7, 8, 11 |
| 内部Soft/HardBreak及显式空段落 `a\nb\n\n\nz\n` | 0, 5, 6, 8 |
| 单元格内两个段落 `a\nb\tc\n` | 0, 2, 4, 6 |
| 代码内空行 `one\n\nsecond\nx\n` | 0, 12, 14 |
| SOURCE/LIVE `# head\r\n\r\nabcdef\r\n` | 0, 10, 18 |
| 同文档VISUAL `head\nabcdef\n` | 0, 5, 12 |
| 起始空行和空格/Tab空行 | 0, 3, 10, 11 |
| 希腊/中文/emoji及空行分隔 | 0, 4, 9, 13 |

永久测试在每个合法UTF8起始位置核对扩展、前后移动、端点交叉和EOF。另覆盖模式切换、同Document重载、仅空白/隐藏定义和空源码。四配置各针对Expand、Move及MoveEndpoint分别注入索引分配失败，验证原范围/版本/存活分配不变、重试成功、暖缓存不分配且最终零泄漏。

两个View在段落缓存建立后观察同一外部事务、Undo/Redo、相同Document重复绑定、替换与销毁。1/10MiB SOURCE 两段落在开头、分隔符两字节、下一段和EOF核对固定边界，并验证建立索引仅新增一次Document分配、后续查询及极限移动不分配。这不是全投影内存有界、首次建立固定时延或固定机器性能验收。

## 实际验收与保留失败

候选：`E:\codex-builds\xge-document-paragraph-integrated-20261009-v827-01a09446`。1681来源、180变化、109新文件、1726物理输入；主1572保护输入不变，尚未合入。

接受记录：`build/source-document-footnote-local-reference-20261006/document-paragraph-v829-paired-acceptance.json`。

- Windows/Linux 正式运行库各137单元全新编译、零复用。初次产品测试在新增历史夹具处失败：默认语义事务不能执行源码替换。v823仅修正夹具事务域，生产源码/头文件/脚本/规则逐字节相同；保留并复用本批全新库，重新编译和运行全部消费者，没有复用失败客户端。
- 共16条正向产品命令：14条当前消费者/原View sanitizer/原永久bat，以及2条生产相同的既有正式构建；失败的三个测试运行和旧段落负例均不计入正向通过。
- 四配置段落、既有范围/行/导航/裁剪、两端完整renderer、Windows两个实际基础UIA客户端通过。Linux原View入口保留ASan/UBSan/泄漏参数及两个PNG失败即abort stub；ext4、单链接、源字节、日志、产物和构建报告独立核对。五份匹配运行依赖接入。
- 原正式DLL的负例实际编译0/运行1，确切输出`Unit=4 expand at 0: result=-7`。v813记录器预设错误数值导致记录器1，v814仅修正预期数值；测试断言不变，原记录保留。负例所有输入在运行前已保存原字节。
- Windows实际UIA图片与此前查看图片逐字节相同；基础客户端通过不代表完整TextPattern或物理读屏验收。

## 尚未完成

FORMAT/PAGE、Prepare generation范围映射、属性/可见文本范围/对象、完整UIA TextPattern/事件/Grid/Table、回调重入/销毁、总体缓存和复杂非ASCII性能仍待完成。当前180项完整仓库/Editor及合入后验收未完成；v799完整入口绑定旧177项候选，须按依赖范围使用。11个工作包与整体目标保持active。
