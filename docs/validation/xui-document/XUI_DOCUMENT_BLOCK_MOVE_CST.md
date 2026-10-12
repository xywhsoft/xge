# Document 块移动的 Markdown 原字节保持

2026-10-07 容器移动最新接续：独立 v73 已修复列表/任务/脚注首段和正文/引用/列表项之间的六个源码保真失败，并解决裸引用标记与列表续行的嵌套列边界；正文沿用同一持久 SourceStore，完整语义和定义原拼写保护保留。四配置每29全新内核、既有336及新180矩阵、80组OOM/取消/22,310未发布失败点、原完整Core程序全部实际0；Linux使用ASan/UBSan。每配置六个1/10MiB正文场景源码/HTML/全NodeId/快照/Undo/Redo通过，来源Blob增长4/0/0字节，未作为RSS/P95/UI物理平台结论。两端各136全新默认共享库/零复用及实际公开336/容器180/Editor252通过，Editor此入口为代理。新原生两端16条检查和32图按此前已查看图像的逐项字节一致接受。八文件永久入口四配置各29全新单元和40条实际检查通过，仍未合入；此前七文件块移动包保留。实际限定终态为 `document-container-move-v73-scoped-terminal.json` 与 `document-container-native-v82-terminal.json`，见[容器移动来源](XUI_DOCUMENT_CONTAINER_MOVE_CST.md)。

主292/1360与候选1373源码保持不变，原75959合入后正式批次仍为Windows。跨父节点首槽/脚注/空容器/非段落及其他命令、完整Renderer/Editor/Web、永久整套接入、实际合入和合入后验收继续；原11包及整体目标active。通用WebView其他四平台及公开通信继续暂缓，Document内部渲染保留。

2026-10-07。此修复已完成隔离专项验收，尚未合入主工作区。整体重构与原 11 个工作包继续 active。

原来的同父节点移动会回退到树序列化，改写未编辑的实体、格式标记、Setext 标题、代码围栏、引用前缀、列表标记和表格拼写。八个独立公开 API 对照确认语义和 Undo 正常，但原字节全部不符。

新实现仍使用同一 Document、事务、SourceStore、解析器和历史。对于可确认来源范围的单次同父 MOVE，在 SourceStore 中旋转原始片段与其尾随间隙；持久序列复用原有 Blob，不复制原文载荷。文件 BOM 固定在文件开头。文件末尾无换行时，只复用前一块分隔区的尾部换行和完整空行，包含引用前缀，避免块直接拼接或松散列表被重解析成紧列表。隐藏定义的字节不进入新增分隔符。

候选仍须经过完整 Markdown 重解析、整树语义投影及有序隐藏定义原拼写保护，才进入同一事务。没有新增内容模型、公共 API 或旧 API 包装。

实际接受证据：

| 项目 | 结果与边界 |
| --- | --- |
| Windows/Linux，普通/引用哈希碰撞四配置 | 每配置 29 个内核单元全新编译；原八例、264 例和扩展 336 例的独立源码/HTML/稳定 ID/旧快照/Undo/Redo 对照通过。336 包含原 264，不能相加当作独立总例数 |
| 四配置故障扫描 | 每配置 56 组逐点 OOM/合作取消，13,630 个未发布失败点；状态、版本、通知、保存状态、失败事务、同一 Document 重试、共同历史、Blob 共享和零泄漏检查通过 |
| 原有完整 Document Core 程序 | 两端普通/碰撞四配置均实际退出 0，测试 C 和断言不变；Linux 使用 ASan/UBSan |
| 新的实际共享库 | 两端各 136 单元全新构建、零复用；同库公开接口 336 例、Editor 252 例通过 |
| 新库原生窗口入口 | 两端原 50093/37329 均实际退出 0，各八条窗口/系统 GL 头、1/10MiB 三模式与首帧光标检查通过；32 张新图与此前逐张查看的同平台图像字节一致，终态为 `document-block-move-native-v65-terminal.json`。Linux 使用 Xvfb/Mesa 软件后端 |
| Editor 专项 | Query/Execute、原字节、HTML、稳定 ID、选区、Undo/Redo 和三模式往返通过；使用代理绘制后端，不作为物理平台或真实窗口验收 |
| 永久入口 | 七文件迁移包已准备。四个内核测试代码/构建文件与先前实际验证包字节一致；两端普通/碰撞永久内核入口及两端永久实际 DLL Editor 入口均通过，尚未进入主 `test_xui` |

终态：`build/source-document-footnote-local-reference-20261006/document-block-move-v55-scoped-terminal.json`。隔离产品为 `E:/codex-builds/xge-document-block-move-20261007-v55-01a09446`，1,373 源闭包；相对已接受 GL/光标/View/永久测试的计划基线，仅一个产品文件 `src/xui/document/xui_document_markdown_edit.c` 改变。主工作区仍为 Document20 合入后的 292 保护输入/1,360 源闭包。

原始失败均保留：纯旋转的 72 个 EOF 差异、首个 EOF 修复的 24 个列表差异，以及定义邻接 EOF 的 48 个差异。测试驱动的相对路径、保存目录、提前准备及 UTF-8 文档读取失败也独立保留，没有改写原终态。保存目录修正使用两端独立工作目录，未修改 Core 断言。

永久迁移包：`build/source-document-footnote-local-reference-20261006/permanent-document-move-v63/tests/xui/`。主要入口为 `build_document_move_test.py` 和 `build_document_move_editor_test.py`，命令见同目录 `DOCUMENT_MOVE_TESTS.md`。

仍需完成：跨父节点及容器首行标记的无损移动、其他通用结构命令的完整来源策略；新候选的完整 Renderer/Editor/Web 与受影响正式门禁；永久入口接入整套、实际合入和合入后验收。原生窗口入口通过不替代完整平台矩阵。不能据此关闭 K2/E1/M1，或宣称物理 IME/DPI/读屏、固定机器性能及整体重构完成。其他四平台通用 WebView 后端和公开通信按用户决定暂缓，Document 内部公式/Mermaid/HTML 通道保留。
