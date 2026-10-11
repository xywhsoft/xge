# 标题合并的统一来源计划

2026-10-07：本批修复仍在独立候选中，主工作区未合入。Windows17/Linux34 限定验证与永久 Windows4/Linux8 测试入口均已实际通过，289/1,357/26 独立物理冻结完成。全新DLL/Native98原会话84542已实际退出0：136单元全新编译/零复用、294公开符号、实际DLL的2,544黄金/32公开OOM及新增864 Editor场景零差异/零拒绝通过。完整842原会话64761已启动，当前Core128；最终完整回归和合入后验收仍必需。K2/K4/E1及整体重构没有关闭。

标题与段落合并沿用同一 Document 事务、解析器、来源索引和共同历史，不新增文档模型或公开 API。

## 问题与原始证据

旧实际 Native98 DLL 的十个严格结构对照检出六处源码差异。标题的正文范围与整个语法范围不同：ATX 的闭合井号、Setext 的下划线以及引用/脚注前缀属于块语法，不能把普通段落合并的接缝直接用于标题。

隔离 v1 保留左侧标题结构并移除右侧标题结构，十个严格对照和 600 个来源黄金通过。继续扩展的 18 个对照仍检出六处真实源码缺陷：四个无末尾换行场景多出换行，两个混合换行场景错误保留左侧最终换行。语义、重解析、位置、历史和失败原子性通过不能替代逐字节保真。

原始驱动、失败日志和证据均保留，没有调整失败黄金：

- `build/source-document-footnote-local-reference-20261006/join-structure-diagnostic-v1-proof.json`
- `build/source-document-footnote-local-reference-20261006/join-heading-edge-diagnostic-v1-proof.json`
- `build/source-document-footnote-local-reference-20261006/join-heading-linux-v1-terminal.json` 只接受旧矩阵的 Linux26，不覆盖新增失败。

## v2 实现

来源计划只在同父级块的纯合并接缝、左侧最后 Text 末尾到右侧首个 Text 开头的适用范围内启用。范围来自事务旧状态中的公开块语法和来源行查询；不从可见文本长度猜测原始标记。

1. 左侧为标题时，保留解析器确认的正文尾部到最后物理行 `iContentEnd` 的全部结构字节。ATX 闭合井号、Setext 的内部换行与下划线、容器前缀均按原字节保留。
2. 右侧正文之后的原标题结构被消费；最终换行取右侧最后物理行的 `iContentEnd..iSyntaxEnd`。右侧原来没有 EOF 换行时，结果也没有。
3. 被移动的隐藏定义及原间隙逐字节复制。右侧 EOF 没有换行时，仅新建必要的分隔 LF；旧间隙的 CR/LF/CRLF 不改写。
4. 在独立影子候选中先应用尾部补丁再删除接缝，完整重解析并核对整树 Markdown 语义投影和定义拼写后才发布。取消、分配失败、重试、旧快照及 Undo/Redo 沿用统一事务机制。

v2 写入器来源为独立工作区 `E:/codex-builds/xge-document-join-heading-draft-20261007-v2-01a09446/src/xui_document_markdown_edit.c`。未改主工作区或正在运行的旧候选输入。

## 已取得的限定证据

Windows v2 的 17 条实际产品命令为 0：18 个严格对照全部通过；既有 600 个 Join/正反 TEXT/GAP 来源黄金与 20 组公开 OOM；新增 1,440 个来源黄金与 24 组 OOM、24 组取消；父继续输入 504 黄金/44 故障扫描及既有 1,200 边界黄金通过。

新增矩阵覆盖 ATX/Setext、左右两侧标题、引用、隐藏定义、脚注首行和续行、列表引用；左侧内部三种换行与右侧三种最终换行/无最终换行独立组合，包含 BOM、直接 Join 及正反 TEXT/GAP 选区。

永久候选已接入同一 CST、私有故障与 Editor 入口。Windows 原会话 16024 已实际退出 0：同一入口 2,544 黄金（原段落 360、继续输入 144、标题 600、新边界 1,440），32 组公开 OOM 和 92 组私有 OOM/取消扫描通过。候选相对父 Join 产品只有一个生产写入器与五个测试文件的变化，289 受保护输入、1,357 源闭包、26 辅助输入；没有使用硬链接。

扩展后的同一 Editor 驱动保留原216场景，新增864场景，覆盖Backspace/Delete/跨块选区删除、精确接缝位置、共同撤销和三模式切换。旧实际DLL的指定负例编译为0、运行为1：原216通过，新864检出864处源码差异且零拒绝。这是旧产品负例。随后相同散列驱动在全新实际DLL上原216和新864均零差异/零拒绝，Native98原会话84542实际0。连同继承的其他Editor专项，本次实际DLL接受3,312个新增Editor场景；不代表物理IME验收。

Windows 最初的扩展控制器在私有驱动的嵌套 `main` 宏冲突上退出 1，前两条 1,440 矩阵产品命令均为 0；修正版按相同散列继承这两条，不重复产品运行，原失败保留。永久 Windows 入口曾在异步准备尚未完成、清单尚未发布时提前调用并退出 1，未启动任何产品命令；原准备 64635 实际 0 后才执行接受批次，未重启准备。

当前证据包括：

- `join-heading-windows-v2-terminal.json`
- `join-heading-permanent-v2-inputs.json`
- `join-heading-permanent-windows-v2-proof.json`
- `join-heading-editor-old-v2-proof.json`
- `join-heading-native-v2-terminal.json`
- `join-heading-permanent-linux-v2-terminal.json`
- `join-heading-v2-frozen-inputs.json`

以上文件位于 `build/source-document-footnote-local-reference-20261006/`。

## 尚未完成

普通/碰撞Linux34专项原会话12508已实际退出0；永久Linux8原会话11241亦已实际0，与Windows4合计12条接受命令，两配置均2,544/32/92。全新DLL/Native98原会话84542实际0，其专项不能替代仍在运行的完整Core/两平台常规/分析。完整842原会话64761当前Core，最终合入与合入后正式/Web验收仍需终态。代理Editor和合成输入不证明物理输入法、读屏、字体/DPI或固定机器P95；本批也不覆盖所有复杂跨容器或多行结构命令。
