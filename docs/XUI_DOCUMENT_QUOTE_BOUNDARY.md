# 取消引用的相邻块分隔与来源保真

日期：2026-10-04。范围：原设计 K2/E1 的整块取消引用；统一 Rich/Markdown Document 内核。整体重构仍未完成。

## 修复的问题

取消引用会把引用内的块提升到父容器。如果前面已有段落，提升后的段落可能和它合并；如果末块是需要空行结束的 HTML，后继正文可能被吸入 HTML。旧 DLL 的公开复现返回 `XUI_DOC_ERROR_UNREPRESENTABLE`，未能执行取消引用。

引用去前缀路径现在先尝试原有最小补丁；仅当完整解析的语义不符时，依次尝试前分隔、后分隔、两端分隔。每个候选都从同一事务草稿创建，完整重新解析，核对目标语义和全部链接/脚注定义的有序值后才能接纳。没有通过的候选不发布，也不进入历史；分配失败和取消保持原子性。

## 来源与共同语义

- 分隔行的祖先引用、列表和任务标记来自解析器确认的来源范围。祖先引用标记保留，列表/任务首行标记转换成等宽空白，避免生成额外列表项。不扫描正文中的字面 `>`。
- 新增分隔行使用对应边缘的换行，支持 LF、CRLF、单 CR、混合换行与 BOM；已有换行、实体、围栏 info、HTML 和定义正文按原字节保留。
- 原来源复制证明继续产生稀疏 SOURCE 操作，未改正文共享旧 SourceStore payload 和定义值缓存。节点移动后，原子节点及后代 ID 保持。
- 如果提升出的段落与列表项原有段落相邻，共同命令同步该列表的 loose 属性；Rich 使用同一事务命令。Markdown 候选必须重新解析为同一目标树。
- 单次操作只提交一个最终版本；同一事务连续取消两个引用仍只有一步共同 Undo。

## 验证与维护入口

维护用例位于 `test_xui/xui_document_quote_boundary_test.c`；Windows 入口 `test_xui/build_document_quote_boundary_test.bat` 已加入完整 Document 套件，Linux Core ASan/UBSan 入口也已接入。

当前 Core、实际 DLL 与 Linux Core ASan/UBSan 已通过：

- 20 组独立来源黄金文件，展开为 348 个 TEXT/GAP、双 affinity、LF/CRLF/BOM/混合换行场景；逐字节源码、完整重载树/语法、节点身份、来源位置映射及 Undo/Redo 核对。
- 根层和 1/8/32/64/123 层父引用、单事务两个引用，以及 7 个 Rich 等价场景。
- 4 KiB/128 KiB/1 MiB 隐藏定义正文，历史独占 `HistoryBytes` 均为 8,724 字节；实际来源 payload 和定义值缓存继续共享。
- 6,205 个分配失败点、6,157 个取消点；失败时已发布内容、节点树、语法、revision 和历史不变，释放后无泄漏。

完整 Windows 套件实际退出 0，实际 DLL Editor 完成 360 个 VISUAL Query/Execute/Caret 场景、240 个 LIVE/SOURCE 原子拒绝及三模式历史；Linux Core/Renderer/View ASan/UBSan、两个修改模块 GCC analyzer、Web 测试/生产构建、五个串行浏览器程序与生产导出审计均实际退出 0。88 项冻结输入保持不变，三张新截图已查看；最终 checks/sources/hashes/scope/baselines/visual 记录随证据前缀保存。

证据前缀：`artifacts/xui-document-rebuild/validation-2026-10-04-quote-boundary-`。旧 DLL 复现、开发失败日志和各次修正记录保留；不覆盖此前引用、脚注、Tab 和来源共享批次。

## 未由本批完成的范围

本批支持解析器来源可确认的整块取消引用。提醒块头部、脚注内部结构编辑、部分范围取消引用、完整 CST/token/trivia 和其他复杂结构命令继续追踪。LIVE/SOURCE 当前明确拒绝该结构命令；拒绝测试不算这两种模式的结构编辑实现。

独立提醒块黄金复现 `validation-2026-10-04-quote-boundary-admonition-probe-v9.log` 在当前源码编译后实际退出 1：取消引用命令返回成功，但 `&amp;` 被改写，原始围栏及附加 info 被规范化。该失败保留为下一项 K2/E1 来源正确性证据，不纳入本批通过的来源范围。

完整重解析仍需临时缓冲和扫描；固定历史占用不证明编辑耗时达标。Linux UI 的无窗口代理不能代替真实 XGE 后端、输入法、读屏、物理 DPI 和其他平台验收。652 项 CommonMark 语料覆盖解析/来源/原生往返和差分，不证明全部 HTML 输出符合标准。

通用 WebView 保持 Windows 基础网页承载；其他四后端和公开通信延期。Document 私有公式/Mermaid/HTML 渲染与独立 HTML 交互继续保留、单独回归。原 13 个工作包保持，整体目标不因本批专项通过而完成。
