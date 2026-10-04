# 取消列表的来源保护与列表首行元数据

日期：2026-10-04。范围：整体重构设计 K2/E1；原 13 个工作包继续，整体目标未完成。

后续同日批次已修复本页记录的 DLL Editor 结果选区失败和跨空列表边界；正式 DLL 专项通过，完整发布验收尚未完成。后续实现与证据见 [取消列表选区与跨空边界](XUI_DOCUMENT_UNLIST_SELECTION.md)。本页原始失败记录与当时冻结输入保留。

## 问题与正式实现

上一批正式 DLL 的取消列表命令会经整段规范化序列化改写实体、围栏及 info，部分选区甚至会改写未选兄弟项的正文。包含隐藏多行链接定义的一个场景返回 -107；八个独立黄金场景全部失败。同一维护测试链接上一批正式 DLL 仍实际退出 1，原始证据保留。

MD4C 现在为每个列表项记录首行标记、标记/任务间隙与逻辑开始列、消费内容列和原始结束列。适配器将首行记录放入 LIST_ITEM 的私有 syntax_aux；公开 continuation 记录仍仅包含延续行空白。普通列表标记后的额外代码缩进保持逻辑宽度，任务首行间隙按解析器实际消费处理，无新增公共 API 或旧接口包装。

取消列表先核对被删除项的每个子树确实原样提升到列表父节点，随后复制正文、隐藏定义和未选区间，仅删除解析器确认的所属标记与缩进，并补必要的块边界分隔。前缀 Tab 按原逻辑宽度处理，字面正文不推断为前缀。提升到父列表项的多个块同时更新 Markdown 松散列表语义。

只删除空项而保留头、尾两个列表时，Markdown 会合并同种标记的相邻列表；新后缀列表因此仅调整自己的 bullet/ordered delimiter，正文与任务标记保持原字节。所属脚注正文值可以随结构改变，其他全部有序定义值与标签仍校验。完整重解析必须与要求的整篇语义一致；已经识别提升操作却无法证明来源时原子拒绝，不退回整段规范化重写。

## 已取得与待确认的证据

正式源码 Core 已通过 30 个独立源码黄金扩展出的 1,380 个列表/项 GAP、跨叶 TEXT、折叠 TEXT、正反方向、双 affinity 和 LF/CRLF/BOM 场景。覆盖实体、任务、任意围栏/info、字面代码 Tab、表格、HTML、嵌套父引用/列表、脚注、重复/未用定义和 EOF；核对所有 UTF-8 来源边界映射、稳定提升与未选子树 ID、SourceStore 共享、完整重载、缓存和 Undo/Redo。

正式 Core 专项还通过 2,268 个分配失败点、2,214 个取消点；发布源码、语义、语法范围、revision、历史和旧快照保持原子，释放后无泄漏。候选实现的完整既有 Core 回归通过，包括空项、跨列表和 Rich 行为。

维护入口为 test_xui/xui_document_unlist_source_test.c，已加入 Windows Core/实际 DLL Editor 和 Linux Core sanitizer。实际 DLL Editor 的目标为 1,380 次 VISUAL 执行、920 次 LIVE/SOURCE 原子拒绝和三模式共同历史；本轮实际运行失败，不能计为完成。追踪记录显示 ci=11、kind=1、VISUAL、正向 GAP 选区执行命令成功，读取结果选区成功，但 ViewSetSelection 返回 -2。原始 Windows 套件与独立 DLL 追踪均退出 1，证据为 validation-2026-10-04-unlist-source-full-suite.log 和 validation-2026-10-04-unlist-source-editor-trace.log。拒绝用例不代表 LIVE/SOURCE 的结构编辑已实现。

95 项源码、头文件与维护测试输入已冻结。证据前缀 validation-2026-10-04-unlist-source-；Linux Core/Renderer/View sanitizer、四个修改模块 analyzer、无宏 MD4C、Web 测试/生产构建均实际退出 0。Windows 完整套件在 DLL Editor 专项失败后停止，后续套件项目不能计为本批通过；本批串行浏览器验收尚未启动。Linux 无窗口代理通过不代表真实 XGE 平台验收。

## 保留范围

本批不代表完整 CST/token/trivia、所有跨复杂容器结构命令、LIVE 结构编辑或全平台验收已经完成。正式源码的四列表跨空项选区探针仍实际返回 -107，原子拒绝且源码、历史保持不变；原始记录为 validation-2026-10-04-unlist-source-cross-empty-probe-v2.log。独立候选现在按最终相邻列表证明新后缀标记归属，并避免重复添加分隔空行；candidate-v2-exits.json 记录该复现、完整 Core 和既有取消列表专项的编译与运行全部退出 0。候选位于 build/list-empty-boundary-20261004，尚未落入正式源码，复杂父容器、DLL Editor 和完整发布验收仍待完成。完整重解析临时内存、大正文缓存历史与 P95 门槛仍在原目标内。

通用 WebView 仍仅 Windows 基础承载，其他四后端与公开通信延期；Document 内部公式、Mermaid、HTML 渲染及独立 HTML 交互保留。Linux 无窗口代理不能代替真实 XGE、输入法、读屏和物理 DPI 验收。
