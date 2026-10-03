# 旧富文本专项向统一 Document 的行为迁移

旧 `xuiRich*` API 与实现已退出公开头、DLL 和工作树。下表记录被移除专项的行为去向；旧文件仍可在 Git 历史中查看。**旧文件删除不代表其全部行为已经验收**。统一 Document 的验收以当前可运行测试和 `XUI_DOCUMENT_VALIDATION.md` 为准。

| 旧专项 | 已由新体系验证 | 尚需用统一 Document API 验证或实现 |
| --- | --- | --- |
| `xui_rich_document_test.c` | `xui_document_test.c` 的结构事务、稳定 NodeId、范围替换、Undo/Redo、富表格同根历史、原生保存、HTML 片段导入/导出、Markdown 解析与显式格式转换；复杂富表格内嵌子表、链接、图片、公式、Mermaid、原始 HTML 和代码块的原生及 HTML 整树往返 | 更多 HTML5 表格纠错、CSS 与外部应用导入组合；旧测试的子文档指针身份语义已由同根 Cell/NodeId 模型替代，不应照搬 |
| `xui_rich_edit_test.c` 及宽度、惰性、分数坐标附属用例 | `xui_document_editor_test.c` 的选区、命令、链接/图片、表格、IME、剪贴板和三模式历史，以及普通容器内嵌 Editor 的坐标、点击焦点、行尾空白与文档下方端点、共享 View 更新与父节点销毁；统一 Document 命令到 XUI Toolbar 的创建、指针执行、状态/语言同步，补验 Rich/Markdown 的引用切换、规则线与空代码块按钮、单步 Undo 和禁用状态；内建 XUI Menu 的右键/键盘调用、命令执行、选区边界、只读及运行时语言更新；内建 XUI 查找窗口的结果表、单项/全部替换及 Markdown SOURCE/LIVE 模式；`xui_document_renderer_test.c` 的短行/混合字号/邻 Cell 命中与容器内 View；`xui_document_fractional_test.c` 的分数坐标 | 更完整的格式菜单及工具栏完整命令集合/自动宿主同步、真实 IME 与跨应用粘贴矩阵 |
| `xui_style_rich_edit_test.c` | `xui_document_style_test.c` 的正文/背景/普通外框/代码/高亮/链接/选区/光标颜色、显式文字颜色、运行时主题切换与清除后回退，以及全部/当前查找命中的双层颜色、文档 revision 和模式变化后的结果重算；新增顶层引用线、规则线、段落底色、表格表头/普通单元格及边框、缺失图片占位/边框/alt 文字、焦点边框的独立主题色，并验证换色不重新 shape；命名图片在缺失、有效 surface、错误资源类型、替换与移除之间切换时，实际图像/占位及对象选中叠加由代理逐态核对 | 复杂嵌套块、加载失败的完整错误状态和真实 GPU 的主题像素矩阵 |
| `xui_rich_edit_scale_test.c` | `xui_document_scale_test.c` 的 10,000 段富文本可见区测量/热重绘/换宽/局部编辑与撤销；`xui_document_fractional_test.c` 的分数几何 | 与逐条参考绘制的独立对照、细粒度缓存内存预算、嵌入 Widget 惰性实例化、完整故障注入规模矩阵 |
| `xui_rich_edit_large_perf_test.c` | `xui_document_scale_test.c` 的 10,000 段和大单段样本，`xui_document_editor_test.c` 的中部输入/跨段替换及 Undo/Redo；实际 DLL Editor 与跨平台 Core 现在对 20,001 节点文档的首次可撤销编辑检查历史独占、LiveBytes 增量和新增分配数，避免全量复制 | 固定测试机上的可重复 P95 与总内存门槛、真实 GPU/IME 响应；当前存储门禁不代表首次编辑 CPU 计费时长已验收 |

下一步应优先补齐编辑器宿主交互，再补独立绘制/规模基准和外部应用互操作。所有新用例必须使用 `xui_document.h` / `xui_document_ui.h` 与实际产品 DLL，不能重新引入旧兼容接口。
