# 统一 Document 实施与接入

日期：2026-09-16。当前为正在实施的新系统，尚未满足整体重构方案的最终发布验收。

本次已经建立可运行的 C 实现：富文本与 Markdown 共用 Document、快照、根事务、历史、结构位置、表格节点、Renderer 和原生编辑控件。新 API 是独立设计，不通过旧 RichDocument/RichEdit 转调。旧实现暂时仍在活动构建中，等待功能迁移完成后移除。

## 源码与依赖

| 入口 | 内容 |
| --- | --- |
| `xui_document.h` | 不依赖窗口与 GPU 的文档 API；由 `xui.h` 包含 |
| `xui_document_ui.h` | 共享 Renderer、DocumentView、DocumentEditor；需要显式包含 |
| `src/xui_document_store.c` | 持久化 treap、radix 节点索引、引用计数、分配诊断 |
| `src/xui_document.c` | 事务、发布、通知、历史、节点操作 |
| `src/xui_document_schema.c` | 节点/属性/表格约束与反序列化校验 |
| `src/xui_document_position.c` | 结构位置、变更映射、源码映射 |
| `src/xui_document_commands.c`、`src/xui_document_table.c` | 范围替换、段落操作、根树内的表格操作 |
| `src/xui_document_markdown*.c`、`src/xui_document_reconcile.c` | 解析适配、源码回写与重解析后的身份匹配 |
| `src/xui_document_search.c` | 与可见控件无关的文本投影、字面查找、原子全部替换 |
| `src/xui_document_io.c`、`src/xui_document_file.c` | 原生 JSON、原子文件写入、打开与保存点 |
| `src/xui_document_html.c` | 转义 HTML 导出；原始 HTML 不执行 |
| `src/xui_document_layout.c`、`src/xui_document_renderer.c` | 共享布局、表格递归、绘制、命中与光标 |
| `src/xui_document_view.c`、`src/xui_document_editor.c` | 原生显示、选择、输入、IME 投影与编辑命令 |
| `src/xui_document_edit_adapter.c` | XUI 通用编辑协议、基础文本辅助技术接口 |
| `examples/xui_document` | 富文本编辑、MD 源码编辑与同文档预览示例 |

`xui_document_sources.bat` 是核心唯一源清单，`xui_sources.bat` 将核心和 UI 模块加入 Windows 主 DLL。独立 core 测试不链接窗口或 Renderer。

Markdown 使用 MD4C 固定提交 `b3c6223903c1df483cef926ba347e531248f0b92`，采用 MIT 许可。供应商文件保持上游原样，分配器包装位于 XUI 适配层。详见 `lib/md4c/README.xui.md`。这是相对于设计中“优先验证 cmark-gfm”的实际依赖选择。

## 内容所有权与位置

- 一个 live Document 由一个所有者线程读写。所有者获取不可变 Snapshot 后，可以把 Snapshot 交给工作线程；不要在另一个线程获取快照的同时提交同一个 live Document。
- Document、Snapshot、ChangeSet 均有 retain/release。事务 Release 会放弃尚未提交的候选。节点信息中的字符串借用自对应快照。
- 节点 ID、长度和偏移采用 64 位。字符串及偏移使用 UTF-8 字节，事务拒绝落在编码中间的边界。字素导航由编辑器处理。
- TEXT 表示节点文本中的位置，GAP 表示容器子节点之间的位置，SOURCE 表示 Markdown 源码位置。位置带文档身份和 revision；过期位置必须经 ChangeSet 映射。
- 表格单元格就在根树中，不拥有第二个 Document 或 Undo 栈。跨不同 Cell scope 的不受支持替换会失败，避免意外合并单元格正文。
- Renderer 保留 Snapshot，借用 context 和传入的字体；释放 Renderer/View/Editor 后再释放这些运行时资源。View/Editor 保留绑定的 Document。

默认限制：256 MiB 文本、1,000,000 节点、128 层结构、256 步历史；表格最多 1024 列。默认限制是资源约束，不是已验证的性能承诺。历史内存预算尚未实现。

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

一次复合操作使用一个根事务。历史、通知和发布所需数据在发布前分配；失败保持原内容、revision、历史和保存状态。通知回调内写入返回 `XUI_DOC_ERROR_BUSY`。空事务不产生历史。`iGroup` 可以显式合并连续、同来源的提交；编辑器尚未自动合并连续键入。

`xuiDocumentTxnReplaceRange` 支持原生文本插入、字面节点编辑、段落拆分、同段跨 run 替换、同父段落之间的替换。表格 API 支持插入表格、行列增删、矩形合并与拆分；合并、行列变化和单元格正文共用根历史。

## Markdown 行为

| 方言 | 已接入的解析配置 |
| --- | --- |
| `XUI_MD_COMMONMARK` | CommonMark 基础解析 |
| `XUI_MD_GFM` | 加表格、删除线、任务列表、自动链接 |
| `XUI_MD_EXTENDED`（默认） | 加公式、脚注、admonition、front matter、Mermaid 围栏节点 |

`.md` 保存直接取已提交源码，不从语义树重新生成整篇。有效 UTF-8 的 BOM、换行、空白、转义、实体和未闭合语法保留在 SourceStore 中。原生 Markdown 工程文件保存源码和方言，读取时重建语义树。

SOURCE_TEXT、LIVE_MARKDOWN 与 VISUAL 是同一文档的三个视图模式，共用时间顺序的 Undo/Redo。SOURCE_TEXT 输入会重新解析；VISUAL 文本修改会生成源码补丁并重解析，将结果与预期语义核对。无法表达时返回错误并禁止提交。

当前视觉回写可用范围：可定位的文本修改、部分单 run marks、顶层段落/标题内的跨 run marks 和段落范围替换，包括 Enter、跨段替换和局部取消格式。被重写的段落允许规范化拼写；范围外源码保持原样。嵌套列表/引用/表格的通用视觉结构重写尚未完成，部分 delimiter 组合或特殊来源仍会明确拒绝。

`XUI_DOC_LIVE_MARKDOWN` 已实现块级语法显隐：光标所在的顶层块显示原始源码，其余块使用原生排版。列表、引用和表格以整个顶层容器为激活单位；不创建第二个 Document。该模式的命中、选区、复制和输入采用 SOURCE 位置，格式工具栏命令目前仅用于 VISUAL。`xuiDocumentRendererSetActivePosition` 供无控件宿主指定活动位置，View/Editor 自动跟随光标。

目前来源记录不是完整 CST；实体/转义/某些复合节点映射会返回 `APPROXIMATE`。LIVE 无法确定可靠的顶层来源边界时会整篇回退到源码显示，`bLiveSourceFallback` 明确报告回退，`GetActiveSourceRange` 可查询实际范围。652 项官方语料中有 40 项发生此回退；因此三模式的入口、共享内容和历史已建立，完整来源覆盖仍未验收。

源码编辑仍为全量解析，未交付增量或异步 prepare。源码全部替换先批量应用补丁，再统一解析一次；诊断统计提供解析次数和扫描源码字节数。

## View、Editor 与宿主接口

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

`xuiDocumentEditorQueryCommand` 返回命令可用、已激活和混合状态。状态查询检查模式、profile、选区、剪贴板后端与历史；实际 Markdown 提交仍要验证语义可表达性。控件只读不会阻止宿主通过 Document 事务更新内容。

IME 预编辑使用未发布候选的独立渲染投影，确认形成一个根提交。取消、外部内容修改、选区/模式变化会撤销预编辑。输入测试覆盖预编辑、取消、确认、替换范围与跨视图选区映射；尚未完成真实 Windows 输入法交互验收。

通用 `xuiEdit*` 支持文本投影、选区、复制/粘贴、历史、只读、光标位置；它的旧协议使用 `int` 偏移，越界返回 LIMIT。原生 Document API 仍采用 64 位结构位置。编辑事件在 `xuiUpdate` 中合并发布。辅助技术目前只接入基本文本框值/选区/编辑动作，未交付完整表格、链接与对象语义树。

`xuiDocumentSnapshotFind` 与 View Find API 提供区分大小写的 UTF-8 字面查找，可跨样式片段。`TxnReplaceAll` 和 Editor ReplaceAll 在一个根事务里完成，不依赖可见区控件。正则、大小写折叠、查找面板和全部匹配高亮尚未实现。

对象测量和绘制可通过 Renderer 的 `onObjectMeasure/onObjectDraw` 接入。当前默认高级对象只有保留源码/占位的行为，不代表公式、Mermaid、HTML 或图片资源已经有生产可用的默认渲染器。

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

输入/输出路径为 UTF-8；文件层使用 XRT 的同目录临时文件和原子替换。Open 返回新的干净文档，没有导入撤销项。SaveFile 只接受能作为文档保存的 native/Markdown 格式；纯文本与 HTML 使用 SnapshotExportFile，导出不会修改保存点。

后台保存应由所有者取得 Snapshot，工作线程导出该 Snapshot；成功后回到所有者线程调用 `MarkSaved(document, snapshot)`。保存期间有新修改时，新文档仍为 dirty。不要在后台线程直接调用 live Document 的 SaveFile。

## 验证

在仓库根目录，GCC/windres 位于 PATH：

```bat
call test_xui\build_document_suite.bat artifacts\xui-document-rebuild\commonmark-0.31.2.json
```

不传参数时跳过官方语料，仅运行本地测试。官方语料来自 `https://spec.commonmark.org/0.31.2/spec.json`；SHA256 为 `d431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20`，包含 652 项。

验证目标包括：独立核心、同一测试链接实际 DLL、官方语料、Renderer、Editor，以及真实 XGE 后端 Source/Live 三栏像素检查。核心测试包含随机编辑、不可变快照、并发读取、保存重载、表格跨度、过期位置、源/语义历史、分配失败逐点扫描。语料测试检验解析成功、源码保持和原生重载，**不是 HTML 输出符合规范测试**。另外对 652 项语料的 15,470 个有效 UTF-8 位置验证 LIVE 活动块与光标几何；该项也不代表排版符合规范。

本机结果、性能样本和缺项见 [验证记录](XUI_DOCUMENT_VALIDATION.md)。

## 最终验收仍未完成的项目

1. Markdown 完整来源层、复杂视觉结构命令、消除 LIVE 来源回退、增量和异步解析；富文本与 Markdown 的显式格式转换。
2. 完整段落 shaping/Bidi、缓存内存预算、主题/DPI 与资源变化下的阅读锚点。
3. 编辑器的表格矩形选择/矩阵粘贴/列宽拖动、块命令 UI、原生片段与 HTML/图片剪贴板、完整辅助技术。
4. 公式、Mermaid、HTML 等真实 provider；通用 WebView 仍是独立依赖。
5. MessageList 的高度、选择、滚动与交互接入，以及旧调用方迁移和旧富文本 API 删除。
6. 真实平台 IME、读屏、多 DPI、其他平台和大文档 P95 验收。

这些缺口尚未由测试验收，不能将当前实现描述为已达到成熟前端 Markdown 编辑器的全部功能。
