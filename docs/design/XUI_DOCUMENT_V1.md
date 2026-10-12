# XUI Document 首个可用版本

2026-10-10，基础 IME 状态切换补验：永久桥接入口新增已入队提交之后的只读开关、Document 换绑再换回、控件销毁重建和禁用再启用，四模式新增16组用例均通过；总计3,253项检查、47组会话变化及2组上下文生命周期用例，旧提交不改变文档，新会话仍可正常提交。本批仅改永久测试及本段验收索引，生产源码、13份Windows产物、Linux运行库和21份既有ZIP保持原字节。Windows22条/Linux30条完整基础命令沿用同一运行库的v1216证据，本批未重复全套基础检查。当前试用包仍为 `document-ime-focus-package-v1214/runnable.zip`，增量核对 `document-v1-v1219-native-ime-transition-acceptance.json`。真实IME/键鼠、Windows剪贴板成功互通、DPI/GPU及其他平台发布仍待验收；高级功能继续冻结。

2026-10-10，基础 IME 会话隔离：已修复原生输入事件按消费时焦点投递、导致旧提交修改另一份文档的问题。事件入队时保存私有会话标识；切换焦点、失焦、Escape、选区或 Markdown 模式变化、上下文切换和销毁后拒绝过期提交，新会话仍可正常输入。Rich/Source/Visual/Live 共 2,357 项桥接检查通过，含 31 组会话变化及 2 组上下文生命周期用例；Windows22条/Linux30条完整基础命令、三个原控件消费者和两项原有IME契约入口通过。最新试用包 `document-ime-focus-package-v1214/runnable.zip`，最终核对 `document-v1-v1216-native-ime-focus-acceptance.json`。真实IME/键鼠、Windows剪贴板成功互通、DPI/GPU与其他平台发布仍待验收，HTML/CSS、Document网络动态资源、完整UIA等高级功能继续冻结。以下旧记录对应此前时刻。

2026-10-10，基础IME删除修复：原生空文本提交现在删除其明确捕获的选区；TSF不经过组合回调的直接删除也会送达DocumentEditor。无替换范围的结束事件、将预编辑恢复为原文后的取消，以及空光标处的空插入继续保持文档不变。Rich、Source、Visual、Live的565项原生桥接检查通过，覆盖删除、取消、历史及此前跨段/64KB窗口用例；两处修复均有实际编译0/运行1的原实现负例。

当前Windows22条、Linux30条完整基础命令、三个原控件消费者和两项原有XGE输入法契约入口全部实际0。Windows本次最终正常构建1编译/136复用，Linux137全新/零复用。最新[运行包](../guide/XUI_DOCUMENT_RUN_PACKAGE.md)为 `document-ime-empty-package-v1192/runnable.zip`，核对 `document-v1-v1194-native-ime-empty-acceptance.json`。公开API/ABI、Document存储、renderer和WebView后端源码未改；真实IME/键鼠、Windows剪贴板成功互通、DPI/物理GPU与其他平台发布仍待验收，高级范围保持冻结。以下说明对应此前批次。

2026-10-10，基础输入补齐：DocumentEditor 已接入 Windows TSF 文本快照与替换事件，原生 IME 的文档投影坐标转换回统一 Document 位置；Rich、Markdown Source/Visual/Live 共用现有事务和撤销历史，公开 API 与 ABI 未改。永久桥接回归通过305项检查，覆盖反向选区、中文/emoji替换、跨段选择、取消、Undo/Redo，以及64KB长文本快照窗口的UTF-8边界和文档偏移；使用真实TSF文本存储接口和XGE事件队列，绘制使用测试代理，尚不代表物理输入法验收。

当前源码已重新通过 Windows22条基础命令（默认21条，另含基础WebView）、Linux30条基础命令及三个原控件消费者。Windows正常构建86编译/51复用，Linux独立ext4源码快照137全新/零复用；两端原生消息、查找/格式、缩放与文件流程均在本批重跑。当前[运行包](../guide/XUI_DOCUMENT_RUN_PACKAGE.md)为 `document-native-ime-package-v1180/runnable.zip`，最终核对 `document-v1-v1182-native-ime-bridge-acceptance.json`。基础实现可供接入试用；真实IME/键鼠、Windows剪贴板成功互通、多DPI/物理GPU及其他平台发布仍待验收。HTML/CSS、Document动态网络资源、完整UIA等高级范围继续冻结。下文旧批次说明对应各自记录时刻。

2026-10-10，首版交付补齐包内查找、缩放和格式说明，并修复README及网页说明指向缺页或源码目录的问题。最新运行包为 `document-portable-guides-package-v1168/runnable.zip`，说明路由回归和普通/共享/纯Document包装、仓库外解压及启动脚本通过，见[运行包](../guide/XUI_DOCUMENT_RUN_PACKAGE.md)；核对记录为 `document-v1-v1170-portable-guides-acceptance.json`。本批只改打包器、永久归档回归和说明，13份Windows程序产物均未变化，原Windows/Linux基础与原生入口证据继承。Windows剪贴板隔离窗口站探针未能启动（命名站错误5、自动命名的新建站错误183）；编译通过、运行失败，未读取或写入用户剪贴板内容，序号638保持不变。此尝试不能记为系统剪贴板成功路径通过。首版可供接入试用；人工IME/键鼠、Windows剪贴板成功互通、DPI/GPU与其他平台发布仍待验收，高级范围继续冻结。

本批最终核对为 `document-v1-v1163-format-menu-acceptance.json`。新版 `document-format-menu-package-v1161/runnable.zip` 的普通/共享运行库两组各6项、仓库外解压6项及三个启动脚本均实际退出0。包内README保持运行验证时字节，本页随后补齐索引；内核运行库与此前ZIP保持原字节，高级范围继续冻结。

2026-10-10，原生编辑示例新增 Format 菜单：段落、三级标题、三类列表、引用、代码块、下划线、删除线、行内代码和清除格式共13项命令。菜单复用现有能力查询、事务与历史；按当前模式、只读和选区禁用不支持的操作，关闭后恢复原编辑器焦点。Windows/Linux 的 Rich、Source、Visual、Live 各通过3,624项查找/格式检查（含3,241项菜单检查、28图像），两端实际窗口缩放各2,728项/32图像，原示例四项和文件/关闭流程复验通过。见[格式菜单](../validation/xui-document/XUI_DOCUMENT_FORMAT_MENU.md)。本批仅改示例与永久验证入口，运行库、内核、renderer、editor与WebView后端源码未改；未重复全部基础/内核套件，沿用v1152的Windows21/Linux30完整基础证据。高级范围继续冻结，物理IME/键鼠、DPI、Windows剪贴板成功互通和其他平台发布仍待验收。

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

2026-10-10，修复 DocumentEditor 原生查找/替换弹窗在缩小宿主窗口时超出 viewport 的问题；弹窗与内部控件按可用空间调整，窗口保持打开时查询与焦点保留。Windows/Linux 的 Rich、Source、Visual、Live 各四组真实窗口尺寸验证通过：每端2,472项检查、32张缩放/查找图像。当前运行库重新通过 Windows 全部21条基础命令（含基础WebView）、Linux 含五组原生扩展的30条外层命令，以及原有三个控件消费者。见[查找入口](../validation/xui-document/XUI_DOCUMENT_NATIVE_FIND.md)、[缩放验证](../validation/xui-document/XUI_DOCUMENT_NATIVE_RESIZE.md)。本批改动包含 editor 的 Find UI，内核、renderer、公开API与WebView后端源码未改；上述基础套件是本批实际重跑结果。真实IME、物理键鼠/GPU、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；HTML/CSS、Document动态网络资源、完整UIA等高级范围继续冻结。

2026-10-10，修复原生编辑示例固定1280×780布局：绘制目标、viewport和输入映射随窗口尺寸更新，工具栏换行，窄窗口纵向排列三个视图。Windows/Linux的Rich与三种Markdown模式各完成四组真实窗口尺寸、2,228项检查和16张缩放图像，覆盖缩放后的原生指针命中、内容/revision与焦点保持、继续编辑和撤销重做；两端原示例四项检查、查找383项和文件/关闭2,682项复验通过。见[原生窗口缩放](../validation/xui-document/XUI_DOCUMENT_NATIVE_RESIZE.md)，最终核对 `document-v1-v1138-native-resize-acceptance.json`。新版[运行包](../guide/XUI_DOCUMENT_RUN_PACKAGE.md)的普通/共享运行库两组检查与独立解压、启动脚本均通过；此前ZIP保持原字节。本批独立执行受影响入口，未重复未修改内核及全部基础套件。原生尺寸/指针队列自动验证已完成，真实IME、物理键鼠、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，原生示例补齐 Find/Replace 按钮与 Ctrl+F/H 的使用说明；Rich、Markdown Source/Visual/Live 四模式在 Windows/Linux 各通过383项查找、替换、历史和焦点检查，各20张图像。新增按钮后的原示例四项检查与两端文件/关闭流程复验通过（各2,682项）。Windows运行包同时包含编辑器、原生消息和独立基础网页三个入口；普通/共享运行库两种包装各6项检查、仓库外解压后的6项检查与3个启动脚本均实际退出0。共享运行库遗漏DLL及加载器的失败证据保留，修复后重新验证。见[原生查找与替换](../validation/xui-document/XUI_DOCUMENT_NATIVE_FIND.md)、[运行包说明](../guide/XUI_DOCUMENT_RUN_PACKAGE.md)，最终核对为 `document-v1-v1125-native-find-acceptance.json`。本批独立验证受影响入口，未重复未修改内核与全部基础套件；前批接受包保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功互通、物理GPU和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，补齐原生 Document 消息示例：Rich、Markdown 和共享草稿直接嵌入 MessageList，使用真实 XGE renderer。Windows与私有X11/Mesa两端各28项检查、各6张图像通过，覆盖命中拖选、中文输入同步、三模式/历史、消息追加后的行高与后续行位置、尾部滚动和窄逻辑视口；已直接查看当前两端图像，并修正示例列表与工具栏重叠。永久原生入口已注册基础套件，本批独立验证该入口，未重复全套基础/内核。见[原生消息接入与证据](../validation/xui-document/XUI_DOCUMENT_NATIVE_MESSAGES.md)，最终核对 `document-v1-v1114-native-messages-acceptance.json`；前批[独立运行包](../guide/XUI_DOCUMENT_RUN_PACKAGE.md)保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功交互、原生窗口缩放、物理GPU与完整平台发布仍待验收，高级范围继续冻结。

2026-10-09，基础原生X11剪贴板修复已通过：Rich/GFM可视、Source/Live纯文本复制/剪切/粘贴支持完整字节传输；读取失败、超出单次协议预算和含不受支持对象的可视复制/剪切均明确返回错误，失败保持文档。最终34场景/493断言通过；Linux正常137全新/零复用构建及九组26条基础命令通过。Windows沿用18入口/3消费者和文件/关闭18子命令的完整基线；最后Linux专用改动在普通/WebView2配置的三个受影响单元中预处理结果相同，当前另补8项构建/接口/原生示例/WebView复验。Linux原生示例10项检查符合预期，8图像与此前核对版本一致。见[原生X11剪贴板范围与证据](../validation/xui-document/XUI_DOCUMENT_X11_CLIPBOARD.md)，最终核对为`document-v1-v1073-x11-clipboard-acceptance.json`。

当前基础实现与自动验收可供接入试用。X11文本跨进程、Linux原生XUI文件/关闭对话框与三模式基础输入队列已有私有显示自动证据；Windows系统剪贴板成功路径、真实IME/人工键鼠、桌面应用互通、物理GPU及其他平台仍待验证。HTML/CSS扩展、Document网络动态资源、完整UIA等高级范围继续冻结，整体长期目标保持未完成。下文旧批次的结果与待验描述对应记录时刻。

范围修订：2026-10-09。按用户最新要求，先完成基础使用流程，再逐步完善高级功能。统一架构和富文本、Markdown 同时交付的要求保持。本文是当前阶段的范围与验收依据；原整体设计中的高级功能不再全部作为首版门槛。

## 首版必须交付

| 范围 | 必须具备的行为 |
| --- | --- |
| 统一 Document | RICH 与 MARKDOWN 使用同一存储、快照、事务、位置映射和撤销历史；失败不能部分提交，已提交内容可可靠保存 |
| 富文本 | 段落、标题、常用文字格式、列表、基础表格和本地图片的显示；输入、选择、复制、剪切、粘贴纯文本、撤销/重做 |
| Markdown | 常用 CommonMark/GFM 内容的解析与显示；源码、可视和 Live 三种模式共享文档与历史；保存源码，不能因模式切换丢失内容 |
| 编辑与持久化 | 常用键鼠操作、中文/Unicode 输入路径、共享预览；原生富文本文件与 Markdown 文件的打开、编辑、保存、重开；失败保留当前内容 |
| DocumentView | 独立只读显示、选择复制、滚动、链接与资源缺失占位；本地图片使用宿主资源接口 |
| MessageList | 绑定 Document、测量绘制、命中选择、复制、更新高度和基本滚动锚点；与独立 View 共用 renderer |
| Windows WebView | 独立网页控件的 HTML/URL 显示、导航、输入焦点、尺寸/可见性和销毁；自动打包 WebView2Loader.dll |
| 交付入口 | 主仓库源码、可运行示例、基础验证入口、公开 API 与构建/运行说明一致 |

Markdown 可视命令以现有能力查询为边界。不支持的操作必须明确返回结果，用户仍可切换源码编辑。Markdown 无法表示的富文本属性不应静默转换；富文本首版使用原生文件保存。

## 冻结的高级功能

| 功能 | 本阶段处理 |
| --- | --- |
| HTML/CSS 转换 | 暂停扩大导入/导出、CSS 级联、复杂跨应用 HTML 粘贴和转换损失矩阵；保留已有实现 |
| 网络动态加载渲染 | 暂停 Document 的远程资源自动加载、动态内容刷新、网络缓存与复杂生命周期；本地资源和缺失占位继续验收 |
| UIA | 暂停完整 TextPattern/TextRange、属性、事件、Grid/Table、PAGE 和读屏适配；保留已有基础接口与已验证的通用文本范围 |
| 高级排版和性能 | 暂停极端规模、深层组合、首次全文投影预算、完整 Bidi 视觉导航、全部对象格式范围和全平台固定机器性能矩阵 |
| 高级嵌入 | 暂停 MessageList 深层 Widget/复杂对象交互与高级读屏；先保留基本 Document 消息显示 |
| WebView 扩展 | macOS/Linux/Android/iOS 后端及对外 JS/消息通信继续暂缓 |

“网络动态加载”冻结针对 Document 的扩展资源系统，Windows WebView 正常 URL 导航仍属于基础功能。Document 内部既有公式/Mermaid 静态渲染通道按此前用户决定保留，作为可选能力；不要求首版新增完整的高级对象编辑器。冻结不等于删除代码、丢弃源数据或把数据正确性问题隐藏起来。

## 验收方法

基础验收以实际运行结果为准：

1. 正常构建入口生成主仓库 DLL，公开声明与导出一致，旧 RichDocument/RichEdit API 已退出活动调用。
2. 统一 Document core、CommonMark/GFM、renderer、完整现有 Editor、通用文本范围与 MessageList 基础回归通过。
3. 原生 XGE 示例实际渲染，三模式、共享预览、未发布输入与图片占位检查通过；增加文件保存/重开检查。
4. Windows WebView 独立构建和基础生命周期/输入/布局验证通过；关闭后端的构建仍可用，运行目录自动携带加载器。
5. 实际XUI文件/确认对话框与窗口关闭保护通过自动交互检查；提供人工验证清单：真实输入法组合/替换、鼠标选择、常用快捷键、打开/保存与窗口交互。自动合成输入不能标记为人工输入法验收。

新基础入口与原完整套件分开。原完整套件和耗时故障/复杂组合检查仍保留，后续高级阶段再运行；它们的中止记录与部分结果不能记为通过。发现基础路径中的数据损坏、崩溃、保存失败或常用操作错误时，仍应修复并复测。

## 当前落地状态

2026-10-09，最新原生示例接续：Linux字体路径与构建入口已补齐，Source/Live渲染、异步编辑、图片占位、Rich/Markdown保存重开及中文显示在私有X11/Mesa软件OpenGL环境通过。Windows受影响示例构建、四项原生检查与文件/关闭内部18条命令通过；内核库和WebView产物未变。该结果不是物理GPU、真实IME/键鼠或系统剪贴板成功验收。见[Linux原生运行](../validation/xui-document/XUI_DOCUMENT_LINUX_NATIVE.md)，最终接受`document-v1-v1037-native-linux-acceptance.json`。

2026-10-09，最新补验：当前源码在WSL2 Ubuntu独立ext4快照完成正常137全新/零复用构建；新增永久Linux基础入口，八组基础回归的25条命令全部实际0，另有表格/脚注12探针通过。近期格式、Markdown回写与删除空白改动已获得当前Linux基础证据。Windows原接受产物未变；Linux原生GPU、物理IME、真实系统剪贴板及其他三平台仍待验收。见[Linux入口与证据](../validation/xui-document/XUI_DOCUMENT_LINUX_BASIC.md)。下文旧批次的“Linux未复验”对应当时状态，高级冻结范围保持。

2026-10-09，已将此前隔离候选中验证过的 196 个变化文件合入主工作区；全部 1691 个来源/规则输入与接受候选逐项一致。合入前该候选已有 Windows/Linux 正式构建、Editor/Core/范围/renderer 与 Linux sanitizer 证据。本次合入记录为 `build/source-document-footnote-local-reference-20261006/document-v1-delivery-v956/source-landed-proof.json`。

旧范围的三个耗时作业已停止，保留实际进程退出记录及原日志。主工作区正常入口完成137全新/零复用编译。基础套件14个入口检查全部通过，另有3个原有控件回归入口通过；中文字体回退修正后，示例重新构建和4个原生检查全部通过。652个CommonMark样本的解析、来源、原生往返与Prepare对照通过，298个公开Document函数导出审核通过。Windows WebView生产版本通过基础检查，加载器由构建器自动打包，14个内部浏览器通信/截图接口未出现在公开导出中。

**基础实现和自动验收已完成，可供接入与试用。** 原接受记录为 `build/source-document-footnote-local-reference-20261006/document-v1-v963-acceptance.json`。后续文件/窗口关闭增量记录为 `document-v1-workflow-v974/proof.json`：永久批处理入口通过，内部18条编译/运行命令全部退出0，覆盖三模式各五种关闭结果、文件取消/拒绝覆盖、确认时出现待处理编辑及Windows消息队列Unicode/关闭；示例重新构建和四个原生检查全部通过。主DLL和构建输入保持一致。原生截图已查看，中文、共享预览、Live和对话框显示正常。真实输入法组合/替换、人工键鼠操作及其他机器/平台仍需人工或平台验证；不将自动合成输入记作人工验收。首版阶段不等于整体设计全部完成，也不改变Document原生格式与代理ABI的版本号。

从仓库根目录运行 `build\xui_document.exe` 试用。重跑基础检查使用 `tests\xui\build_document_basic_suite.bat --with-webview`，详见[示例说明](../../examples/xui/xui_document/README.md)。本地图片仍由宿主提供，示例文件不内嵌资源包。示例窗口关闭提供保存、放弃和取消；保存依次处理两份脏文档，取消保存保持窗口，待处理编辑完成后才允许保存退出。文件确认会重新检查对话框打开后启动的编辑，忙时保留当前文档；其他宿主可复用该流程。

最新独立增量接受记录：`build/source-document-footnote-local-reference-20261006/document-v1-v976-workflow-acceptance.json`。该记录核对本轮来源、日志、运行库、可执行文件与最终截图；原未变化的内核/renderer验收沿用v963，不将本轮增量检查描述成全套重新运行。

## 基础删除空白补充验收

2026-10-09，已修复删除暴露TAB、段尾空格、格式分隔符旁Unicode空白时的操作拒绝；保持原格式、完整文本与引用键。当前v1018全部18项基础及3个原控件消费者实际0；永久删除回归扩展33种源码/594组合，8组分配预算扫描和折叠光标检查通过。主库和WebView均正常增量1重编译/136复用，不变更公开API。最终接受为`document-v1-v1022-whitespace-acceptance.json`，见[删除空白与光标验证](../validation/xui-document/XUI_DOCUMENT_WHITESPACE.md)。本轮仍不包含物理IME/其他平台发布或冻结高级功能。

## 基础剪贴板与段首删除接续验收

2026-10-09，已修复SOURCE/LIVE复制失败误报成功及剪切误删、原生剪贴板读取失败被当作空内容、空粘贴删除选区的问题。Markdown同一Text节点删除接入统一CST源码规划器，修复段首剪切留下空格时拒绝操作；只转义解析确实需要保护的边界字节，链接标签等保持最小源码修改。

新增永久剪贴板与段首删除入口。当前v1010全部18个基础入口、3个原控件消费者及文件/关闭内部18条命令均实际退出0，1711项输入和运行库前后不变。剪贴板32场景/610断言，段首删除216组合及两个入口各140分配预算点通过。主库正常构建先3重编译/134复用、再2重编译/135复用；Windows基础WebView重编译2/复用135后通过。最终独立核对为`document-v1-v1011-basic-acceptance.json`。详见[基础剪贴板与删除修正](../validation/xui-document/XUI_DOCUMENT_CLIPBOARD.md)。真实系统剪贴板成功路径、物理IME、人工键鼠及本轮新改动的其他平台复验未计作通过。

## 基础格式命令接续验收

2026-10-09，已补齐代码块/图片字段的行内格式禁用状态，修复 GFM 单词内部删除线，并将 Markdown 快捷回写改为私有候选语义校验后采用。原公开探针的九个不一致全部消失。更新运行库后，v985 全部16个基础构建/验证入口实际退出0；永久格式回归另补109/219个小规模分配预算点，v987独立入口通过，原文/保存点/历史/快照/通知和释放正确。最终独立记录为 `document-v1-v988-format-acceptance.json`，详细契约与永久入口见[格式命令修正](../validation/xui-document/XUI_DOCUMENT_FORMAT_COMMANDS.md)。本次正常构建按依赖重编译三个受影响单元，不能描述成新的137全新构建；Linux旧证据不替代这些改动的复验。

## 后续顺序

首版先用于接入试用并收集基础使用问题。用户恢复高级范围后，依次完善复杂结构命令与剪贴板、排版/长文档性能、HTML/CSS 转换和网络资源、完整辅助技术以及其他平台 WebView。每项单独定义范围和验收，避免重新把所有高级功能绑定成一个交付门槛。
