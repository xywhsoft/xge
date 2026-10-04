# 提醒块头部来源与取消引用保真

日期：2026-10-04。范围：原设计 K2/E1；整体 Document/Rich/Markdown 目标仍未完成。

## 问题与实现

旧实现只记录提醒类型的规范化语义值，丢失原始 `[!TYPE]` 头部范围。取消引用无法通过前缀补丁的完整语义检查，退到整体序列化，改写正文实体、围栏和附加 info。上一批独立失败复现已保留，本批同一源码通过；上一批实际 DLL 再次复现退出 1。

MD4C 在识别提醒标签、将头部行变成解析器空行之前，记录原始标签范围及完整行结束位置。标签复用 Quote 的 secondary marker，行结束复用 kind-specific tail；不增加公共函数、结构字段或旧 API 包装。Document 检查原始标签与语义类型按 ASCII 大小写相等，并验证周围空白和 LF/CRLF/单 CR 范围。公共 `GetBlockSyntax` 的 secondary 范围现在能读取原始大小写；普通 Quote 与 CommonMark/GFM 的字面标签没有该范围。

取消引用只删除已确认的头部行和本层引用前缀，继续复制正文原来源。如果头部行含父列表首标记，将该前缀保留到第一条正文行，去掉其重复的 continuation 前缀；按新物理列调整前导 Tab，保留正文代码中的 Tab。空提醒块保留必要的父容器标记及换行；BOM 保留。原有前/后分隔候选、完整语义及有序定义值校验继续生效，失败不发布。

同一事务可以连续取消内外两层提醒块，只有一个最终版本和一步 Undo。Rich 仍使用同一语义命令；Markdown 原始头部仅属于来源系统，不转换成正文文本。原生 Markdown 数据保存源码，重新加载后重新解析头部元数据；独立文档的 NodeId 不要求相同，原文档的快照、增量版本与移动后子节点身份保持。

## 开发验证与维护入口

当前 Core、实际 DLL 与 Linux Core ASan/UBSan 已通过：

- 17 组独立来源黄金文件、五种提醒类型及不同大小写，共 1,470 个 TEXT/GAP、双 affinity、LF/CRLF/BOM/混合换行场景；完整重载、节点及后代 ID、稀疏补丁、全部 UTF-8 边界映射、定义值共享和 Undo/Redo。
- 36 个空提醒块场景，包含根/父引用/列表/相邻段落、BOM 与 EOF；CommonMark/GFM 的字面 `[!NOTE]` 保留。
- 增量 SOURCE 前置插入后的相对范围位移、旧快照不变与原生格式往返；四个脚注内提醒块的去缩进、列表、Tab、混合换行加载与原生往返。
- 根和 1/8/32/64/123 层父引用、内外两种提醒的复合事务；35 个 Rich 等价场景。
- 4 KiB/128 KiB/1 MiB 隐藏正文的 `HistoryBytes` 均为 8,821 字节，正文 SourceStore payload 与有序定义值缓存保持共享。
- 3,267 个分配失败点与 3,219 个取消点；已发布的源码、树、语法、revision 和历史不变，释放后无泄漏。

维护用例：`test_xui/xui_document_admonition_source_test.c`；Windows 专项已加入完整套件，Linux Core ASan/UBSan 也已接入。实际 DLL Editor 的维护用例覆盖五类型、三种选区、两种 affinity 和三种初始模式，共 1,530 个 VISUAL 执行与 1,020 个 LIVE/SOURCE 原子拒绝场景，全部实际通过。拒绝用例不代表这些模式已实现结构编辑。

证据前缀 `artifacts/xui-document-rebuild/validation-2026-10-04-admonition-source-`。91 项冻结输入保持不变；完整 Windows 套件、Linux Core/Renderer/View ASan/UBSan、三个修改模块 analyzer、无宏 MD4C、Web 测试/生产构建、五个串行浏览器程序和生产 API 边界均实际退出 0。三张新截图已查看；最终 checks/sources/hashes/scope/baselines/visual 与剩余失败探针记录随证据前缀保存。

## 范围与后续

脚注内部的结构编辑、部分范围取消引用、完整 CST/token/trivia、LIVE 结构编辑和其他原工作包仍未完成。脚注内的加载/来源范围测试不等于脚注内结构命令已实现。历史独占固定不证明完整重解析的 P95 耗时达标；Linux UI 代理不替代真实平台、IME、读屏或物理 DPI 验收。

当前脚注内取消提醒块的独立黄金复现 `validation-2026-10-04-admonition-source-footnote-probe-v10.log` 实际退出 1：命令返回成功，但脚注正文实体、围栏和附加 info 仍被整体序列化改写。源码、日志及失败终态保留，作为下一项 K2/E1 来源正确性证据。

通用 WebView 仍仅 Windows 基础网页承载，其他四后端和公开通信延期；Document 私有公式/Mermaid/HTML 渲染与独立 HTML 交互保留。原 13 个工作包及整体设计 SHA 继续追踪，目标保持 active。
