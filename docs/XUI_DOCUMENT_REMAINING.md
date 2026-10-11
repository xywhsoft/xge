# XUI Document、富文本与 Markdown 当前任务

2026-10-10，基础 IME 状态切换补验：永久桥接入口新增已入队提交之后的只读开关、Document 换绑再换回、控件销毁重建和禁用再启用，四模式新增16组用例均通过；总计3,253项检查、47组会话变化及2组上下文生命周期用例，旧提交不改变文档，新会话仍可正常提交。本批仅改永久测试及本段验收索引，生产源码、13份Windows产物、Linux运行库和21份既有ZIP保持原字节。Windows22条/Linux30条完整基础命令沿用同一运行库的v1216证据，本批未重复全套基础检查。当前试用包仍为 `document-ime-focus-package-v1214/runnable.zip`，增量核对 `document-v1-v1219-native-ime-transition-acceptance.json`。真实IME/键鼠、Windows剪贴板成功互通、DPI/GPU及其他平台发布仍待验收；高级功能继续冻结。

2026-10-10，基础 IME 会话隔离：已修复原生输入事件按消费时焦点投递、导致旧提交修改另一份文档的问题。事件入队时保存私有会话标识；切换焦点、失焦、Escape、选区或 Markdown 模式变化、上下文切换和销毁后拒绝过期提交，新会话仍可正常输入。Rich/Source/Visual/Live 共 2,357 项桥接检查通过，含 31 组会话变化及 2 组上下文生命周期用例；Windows22条/Linux30条完整基础命令、三个原控件消费者和两项原有IME契约入口通过。最新试用包 `document-ime-focus-package-v1214/runnable.zip`，最终核对 `document-v1-v1216-native-ime-focus-acceptance.json`。真实IME/键鼠、Windows剪贴板成功互通、DPI/GPU与其他平台发布仍待验收，HTML/CSS、Document网络动态资源、完整UIA等高级功能继续冻结。以下旧记录对应此前时刻。

2026-10-10，基础IME删除接续：已修复DocumentEditor忽略空文本提交、TSF丢弃非组合直接删除的两处问题；取消恢复原文和空光标插入仍为无操作。四模式565项桥接检查、Windows22条/Linux30条完整基础命令、三个原控件消费者及两项原有输入法契约入口通过。新包 `document-ime-empty-package-v1192/runnable.zip`，核对 `document-v1-v1194-native-ime-empty-acceptance.json`。真实输入法/键鼠、Windows剪贴板成功互通、DPI/GPU和其他平台发布仍待验收；高级功能继续冻结。以下旧记录对应此前时刻。

2026-10-10，先完成基础输入：已修复 DocumentEditor 缺少 Windows TSF 文本快照的问题，原生替换范围经共享编辑投影映射回 Document；Rich及三种Markdown模式的305项桥接检查通过。当前Windows22条（含基础WebView）、Linux30条完整基础命令和三个原控件消费者实际退出0；Linux137全新/零复用构建。新运行包为 `document-native-ime-package-v1180/runnable.zip`，核对记录 `document-v1-v1182-native-ime-bridge-acceptance.json`。真实输入法候选窗/键鼠、Windows剪贴板成功互通、DPI/GPU和其他平台发布仍待验收；HTML/CSS、Document网络动态资源、完整UIA及高级矩阵继续冻结。以下旧批次说明对应此前时刻。

2026-10-10，首版交付包已补齐操作说明，README/网页说明的包内链接回归通过；普通/共享/纯Document三种包装及仓库外运行通过，最新包为 `document-portable-guides-package-v1168/runnable.zip`，核对记录为 `document-v1-v1170-portable-guides-acceptance.json`。本批程序、Document内核及Windows/Linux运行库未改。Windows剪贴板成功路径的隔离验证仍待完成：专用命名窗口站创建被拒绝（错误5），自动命名的新建尝试返回已存在（错误183），两次均在任何剪贴板内容读写之前失败；用户剪贴板序号638未变。基础自动证据可支持接入试用，人工输入法/键鼠、Windows成功互通、跨机器和其他平台发布仍待验收。HTML/CSS、Document网络动态资源、完整UIA等冻结项继续暂缓。

本批最终核对为 `document-v1-v1163-format-menu-acceptance.json`。新版 `document-format-menu-package-v1161/runnable.zip` 的普通/共享运行库两组各6项、仓库外解压6项及三个启动脚本均实际退出0。包内README保持运行验证时字节，本页随后补齐索引；内核运行库与此前ZIP保持原字节，高级范围继续冻结。

2026-10-10，原生编辑示例新增 Format 菜单：段落、三级标题、三类列表、引用、代码块、下划线、删除线、行内代码和清除格式共13项命令。菜单复用现有能力查询、事务与历史；按当前模式、只读和选区禁用不支持的操作，关闭后恢复原编辑器焦点。Windows/Linux 的 Rich、Source、Visual、Live 各通过3,624项查找/格式检查（含3,241项菜单检查、28图像），两端实际窗口缩放各2,728项/32图像，原示例四项和文件/关闭流程复验通过。见[格式菜单](XUI_DOCUMENT_FORMAT_MENU.md)。本批仅改示例与永久验证入口，运行库、内核、renderer、editor与WebView后端源码未改；未重复全部基础/内核套件，沿用v1152的Windows21/Linux30完整基础证据。高级范围继续冻结，物理IME/键鼠、DPI、Windows剪贴板成功互通和其他平台发布仍待验收。

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

2026-10-10，修复 DocumentEditor 原生查找/替换弹窗在缩小宿主窗口时超出 viewport 的问题；弹窗与内部控件按可用空间调整，窗口保持打开时查询与焦点保留。Windows/Linux 的 Rich、Source、Visual、Live 各四组真实窗口尺寸验证通过：每端2,472项检查、32张缩放/查找图像。当前运行库重新通过 Windows 全部21条基础命令（含基础WebView）、Linux 含五组原生扩展的30条外层命令，以及原有三个控件消费者。见[查找入口](XUI_DOCUMENT_NATIVE_FIND.md)、[缩放验证](XUI_DOCUMENT_NATIVE_RESIZE.md)。本批改动包含 editor 的 Find UI，内核、renderer、公开API与WebView后端源码未改；上述基础套件是本批实际重跑结果。真实IME、物理键鼠/GPU、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；HTML/CSS、Document动态网络资源、完整UIA等高级范围继续冻结。

2026-10-10，修复原生编辑示例固定1280×780布局：绘制目标、viewport和输入映射随窗口尺寸更新，工具栏换行，窄窗口纵向排列三个视图。Windows/Linux的Rich与三种Markdown模式各完成四组真实窗口尺寸、2,228项检查和16张缩放图像，覆盖缩放后的原生指针命中、内容/revision与焦点保持、继续编辑和撤销重做；两端原示例四项检查、查找383项和文件/关闭2,682项复验通过。见[原生窗口缩放](XUI_DOCUMENT_NATIVE_RESIZE.md)，最终核对 `document-v1-v1138-native-resize-acceptance.json`。新版[运行包](XUI_DOCUMENT_RUN_PACKAGE.md)的普通/共享运行库两组检查与独立解压、启动脚本均通过；此前ZIP保持原字节。本批独立执行受影响入口，未重复未修改内核及全部基础套件。原生尺寸/指针队列自动验证已完成，真实IME、物理键鼠、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，原生示例补齐 Find/Replace 按钮与 Ctrl+F/H 的使用说明；Rich、Markdown Source/Visual/Live 四模式在 Windows/Linux 各通过383项查找、替换、历史和焦点检查，各20张图像。新增按钮后的原示例四项检查与两端文件/关闭流程复验通过（各2,682项）。Windows运行包同时包含编辑器、原生消息和独立基础网页三个入口；普通/共享运行库两种包装各6项检查、仓库外解压后的6项检查与3个启动脚本均实际退出0。共享运行库遗漏DLL及加载器的失败证据保留，修复后重新验证。见[原生查找与替换](XUI_DOCUMENT_NATIVE_FIND.md)、[运行包说明](XUI_DOCUMENT_RUN_PACKAGE.md)，最终核对为 `document-v1-v1125-native-find-acceptance.json`。本批独立验证受影响入口，未重复未修改内核与全部基础套件；前批接受包保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功互通、物理GPU和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，补齐原生 Document 消息示例：Rich、Markdown 和共享草稿直接嵌入 MessageList，使用真实 XGE renderer。Windows与私有X11/Mesa两端各28项检查、各6张图像通过，覆盖命中拖选、中文输入同步、三模式/历史、消息追加后的行高与后续行位置、尾部滚动和窄逻辑视口；已直接查看当前两端图像，并修正示例列表与工具栏重叠。永久原生入口已注册基础套件，本批独立验证该入口，未重复全套基础/内核。见[原生消息接入与证据](XUI_DOCUMENT_NATIVE_MESSAGES.md)，最终核对 `document-v1-v1114-native-messages-acceptance.json`；前批[独立运行包](XUI_DOCUMENT_RUN_PACKAGE.md)保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功交互、原生窗口缩放、物理GPU与完整平台发布仍待验收，高级范围继续冻结。

2026-10-09，基础原生X11剪贴板修复已通过：Rich/GFM可视、Source/Live纯文本复制/剪切/粘贴支持完整字节传输；读取失败、超出单次协议预算和含不受支持对象的可视复制/剪切均明确返回错误，失败保持文档。最终34场景/493断言通过；Linux正常137全新/零复用构建及九组26条基础命令通过。Windows沿用18入口/3消费者和文件/关闭18子命令的完整基线；最后Linux专用改动在普通/WebView2配置的三个受影响单元中预处理结果相同，当前另补8项构建/接口/原生示例/WebView复验。Linux原生示例10项检查符合预期，8图像与此前核对版本一致。见[原生X11剪贴板范围与证据](XUI_DOCUMENT_X11_CLIPBOARD.md)，最终核对为`document-v1-v1073-x11-clipboard-acceptance.json`。

当前基础实现与自动验收可供接入试用。X11文本跨进程、Linux原生XUI文件/关闭对话框与三模式基础输入队列已有私有显示自动证据；Windows系统剪贴板成功路径、真实IME/人工键鼠、桌面应用互通、物理GPU及其他平台仍待验证。HTML/CSS扩展、Document网络动态资源、完整UIA等高级范围继续冻结，整体长期目标保持未完成。下文旧批次的结果与待验描述对应记录时刻。

更新：2026-10-09。按用户最新决定，先完成首个可用版本，高级功能分期恢复。**基础实现已合入，自动基础验收已通过，可供试用。** 具体范围以[首版方案](XUI_DOCUMENT_V1.md)为准。原整体设计保留，但原来的11个工作包不再全部构成首版门槛。

当前接入入口是`build\xui_document.exe`，可试用富文本与Markdown共享内核、源码/可视/Live、共享预览、文件往返和关闭保存保护。基础剪贴板、段首与Unicode空白删除、格式命令的修正均已有永久回归；近期证据以本文顶部和[首版范围](XUI_DOCUMENT_V1.md)为准。

## 基础阶段后续

1. 在实际使用环境验证中文输入法组合/替换、鼠标选择、常用快捷键和窗口操作。
2. 补Windows真实系统剪贴板成功交互、桌面应用互通、物理GPU与跨机器验收；X11三模式基础输入队列已通过合成原生事件回归，物理键鼠和IME仍待验证。
3. 接入应用并处理发现的基础问题；macOS/Android/iOS发布验证尚未完成。

这些平台和人工验收继续保留为待办。自动基础检查已通过，不以恢复高级矩阵作为试用门槛。

## 首版推进

| 工作 | 当前状态 | 接下来的完成条件 |
| --- | --- | --- |
| 统一内核和已有正确性修复 | 已合入196项变化；主仓库Core、来源/往返、652样本与现有Editor回归通过；基础剪贴板及段首删除修正已复验 | 后续只针对新失败或新的基础用例复测 |
| 正常构建 | 当前Windows正常依赖构建通过；Linux独立快照137全新/零复用；298公开函数审核与3个控件回归入口通过 | 新使用环境按正常入口构建验证 |
| 富文本/Markdown使用流程 | 三模式、共享预览、打开/另存为、文件重开、Unicode编辑、撤销重做、失败保留通过；实际文件对话框、关闭保存/放弃/取消、待处理编辑保护已补齐 | 人工验证真实IME、键鼠使用及其他机器；自动对话框测试不代表物理输入验收 |
| Windows原生IME桥接 | TSF快照与替换接入共享Document投影；四模式3,253项检查通过，含空提交、直接删除、跨段、64KB窗口、会话隔离和只读/换绑/重建/启停；47组会话变化及2组上下文切换/销毁用例通过，公开API不变 | 真实输入法候选、组合、替换、取消与窗口焦点人工验收 |
| 原生窗口布局 | 工具栏换行、三视图适配；双端四模式/四尺寸和原生指针队列通过 | 物理拖动窗口、多DPI与其他平台验证 |
| 查找、替换与基础格式入口 | Find/Replace、Ctrl+F/H与Format菜单均接入原生示例；双端四模式3,624检查通过，其中格式菜单3,241检查；禁用项、菜单焦点与共同历史已验证 | 人工键鼠使用检查 |
| MessageList | Document绑定及原生Rich/MD/共享草稿示例通过；Windows/Linux各28检查、6图像，覆盖拖选提取和行高/滚动 | 长会话与复杂对象留到高级阶段 |
| Windows基础WebView | 生产导出、HTML/URL导航、焦点、布局、可见性、缩放、关闭和自动加载器打包通过 | 实际应用网页与键盘输入验证；其他四平台继续暂缓 |
| 可解压交付包 | 三个启动入口及包内查找/缩放/格式说明具备；普通与共享包各5份说明/10条链接，纯Document包4份说明/9条链接；实际ZIP和仓库外运行通过 | 在目标用户机器试用，记录真实IME、键鼠、剪贴板与环境问题 |
| 基础交付验收 | v1208当前Windows22条、v1209当前Linux30条完整基础命令及3个原控件消费者通过；Windows注册默认21入口、含WebView22入口；原生消息/查找/格式/缩放/文件流程均重跑；原有TSF存储与输入法模式契约通过 | 可供接入试用；X11私有跨进程文本已验证，Windows真实剪贴板成功交互、人工验收和完整平台发布仍待验证 |

合入证据：`build/source-document-footnote-local-reference-20261006/document-v1-delivery-v956/source-landed-proof.json`。
主构建证据：`document-v1-main-build-v957/proof.json`。源码已落地，不能再按历史记录中的“仍在隔离候选”描述当前状态。
首版自动接受：`document-v1-v963-acceptance.json`，不包含冻结的高级矩阵或人工输入法验收。

文件/关闭流程增量：`document-v1-workflow-v974/proof.json`。永久bat入口及内部18条命令、最终示例构建和四个原生检查全部退出0；1699项来源/规则输入前后不变，主DLL未改变。发现并修复对话框打开后启动Prepare时“确认打开”直接替换文档的问题，原失败记录`document-demo-pending-file-v972`保留。新流程已注册到基础套件；本轮运行其独立入口与受影响原生检查，未重复执行未改变的Core/renderer等全部基础入口。

独立增量接受：`document-v1-v976-workflow-acceptance.json`。最终目录的保存、拒绝覆盖和关闭提示截图已直接查看并核对，当前示例可执行文件有独立冻结副本。v975收集器错误要求不同输出目录的截图完全相同，失败保留；文件对话框显示不同目录路径，因此改为直接查看最终截图，没有修改产品或测试断言。

基础格式接续：已修复代码块/图片字段的格式命令状态、GFM单词内部删除线与Markdown候选回写。v985重跑当前运行库的全部16个基础入口，均实际退出0；v987仅扩展永久测试，109/219个分配预算点通过，运行库未变。最终输入/日志/运行库/图像独立核对为`document-v1-v988-format-acceptance.json`，详见[格式命令](XUI_DOCUMENT_FORMAT_COMMANDS.md)。首版可试用；尚待真实IME、人工键鼠、跨机器与新的平台验证。冻结范围不自动恢复，整体长期目标未宣称完成。

## 本阶段冻结

- HTML/CSS复杂转换、跨应用HTML粘贴和完整转换损失矩阵。
- Document网络动态资源加载、刷新、缓存及复杂对象生命周期扩展。
- 完整UIA TextPattern/TextRange、属性、Grid/Table、事件、PAGE与读屏适配。
- 极端规模、深层结构/对象全组合、首次全文投影预算、完整视觉Bidi及所有平台固定性能矩阵。
- MessageList高级Widget/对象交互、深层重排与读屏；其他平台WebView及公开JS/消息通信。

已有实现和测试保留。数据正确性、常用编辑、保存失败和崩溃不能因冻结而跳过。Document内部已有公式/Mermaid静态渲染继续保留为可选能力。Windows WebView正常URL导航仍属于基础功能。

## 耗时作业与历史

旧范围v926、v799完整矩阵及v794旧诊断已按本次范围调整停止，实际进程退出记录保存在`document-v1-delivery-v956/cancelled-exhaustive-jobs.json`。原日志和部分成功结果保留，停止不算通过，不重新启动相同高级矩阵作为首版门槛。

[范围调整前清单](XUI_DOCUMENT_REMAINING_HISTORY_20261009.md)和[执行进展](XUI_DOCUMENT_REBUILD_PROGRESS.md)保留历史证据。“当前”“正在运行”“未合入”等旧文字只对应记录时刻。首版完成不等于全部高级目标完成。
