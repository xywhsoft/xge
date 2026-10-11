# Document 基础剪贴板与段首删除正确性

## Windows 原生成功路径的独立验证入口（待环境验收）

2026-10-10，新增 `test_xui/build_document_windows_clipboard_test.py` 与对应 C 测试。入口先确认管理员组已启用，然后以 `CWF_CREATE_ONLY` 创建全新的非交互窗口站及桌面；访问权限仅授予当前用户，所有剪贴板操作和子进程限定在此窗口站。创建失败时不会复用已有窗口站或转到交互剪贴板。

```powershell
python test_xui/build_document_windows_clipboard_test.py --runtime-dir build
```

输出目录自动生成且必须为新目录；也可用 `--out-dir` 指定新位置。条件满足后，入口执行 Rich、Markdown Source/Visual/Live 的复制、剪切、粘贴和共同历史检查，计划运行12组成功用例与12个原生Win32读写子进程，使用真实XGE剪贴板适配器；绘制代理仍为确定性测试代理。这是基础Unicode文本互通验证，不代表桌面应用人工互通或完整HTML/CSS转换验收。

退出0表示全部原生成功用例通过；退出77表示私有窗口站隔离不可用，不能记为成功；其他非零表示构建、保护条件或用例失败。该入口不加入默认基础套件的必过项，避免把特权环境要求当作普通运行要求。另有两个固定保护检查：误指定 `WinSta0` 的读取和写入均须在调用 `OpenClipboard` 前拒绝。

当前结果：C编译实际退出0，两个交互窗口站保护检查按预期退出1；主测试实际退出77。当前进程的管理员组未启用，没有创建私有窗口站、调用剪贴板内容接口或启动互通子进程；交互剪贴板序号638未变。原生成功用例运行数为0，Windows成功路径仍待验证。证据为 `document-windows-native-clipboard-v1221/report.json`，准备与保护核对为 `document-v1-v1223-windows-clipboard-gate-readiness.json`；生产源码、运行库及现有试用包未改。此前v1220编译/不可用记录保留。

命名与创建限制依据[CreateWindowStationW官方文档](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowstationw)，权限检查依据[CheckTokenMembership官方文档](https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership)。不能通过清空用户剪贴板来替代隔离验收。真实IME/键鼠、多DPI/GPU和其他平台发布仍待验收，高级范围继续冻结。

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

2026-10-10，修复 DocumentEditor 原生查找/替换弹窗在缩小宿主窗口时超出 viewport 的问题；弹窗与内部控件按可用空间调整，窗口保持打开时查询与焦点保留。Windows/Linux 的 Rich、Source、Visual、Live 各四组真实窗口尺寸验证通过：每端2,472项检查、32张缩放/查找图像。当前运行库重新通过 Windows 全部21条基础命令（含基础WebView）、Linux 含五组原生扩展的30条外层命令，以及原有三个控件消费者。见[查找入口](XUI_DOCUMENT_NATIVE_FIND.md)、[缩放验证](XUI_DOCUMENT_NATIVE_RESIZE.md)。本批改动包含 editor 的 Find UI，内核、renderer、公开API与WebView后端源码未改；上述基础套件是本批实际重跑结果。真实IME、物理键鼠/GPU、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；HTML/CSS、Document动态网络资源、完整UIA等高级范围继续冻结。

2026-10-10，修复原生编辑示例固定1280×780布局：绘制目标、viewport和输入映射随窗口尺寸更新，工具栏换行，窄窗口纵向排列三个视图。Windows/Linux的Rich与三种Markdown模式各完成四组真实窗口尺寸、2,228项检查和16张缩放图像，覆盖缩放后的原生指针命中、内容/revision与焦点保持、继续编辑和撤销重做；两端原示例四项检查、查找383项和文件/关闭2,682项复验通过。见[原生窗口缩放](XUI_DOCUMENT_NATIVE_RESIZE.md)，最终核对 `document-v1-v1138-native-resize-acceptance.json`。新版[运行包](XUI_DOCUMENT_RUN_PACKAGE.md)的普通/共享运行库两组检查与独立解压、启动脚本均通过；此前ZIP保持原字节。本批独立执行受影响入口，未重复未修改内核及全部基础套件。原生尺寸/指针队列自动验证已完成，真实IME、物理键鼠、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，原生示例补齐 Find/Replace 按钮与 Ctrl+F/H 的使用说明；Rich、Markdown Source/Visual/Live 四模式在 Windows/Linux 各通过383项查找、替换、历史和焦点检查，各20张图像。新增按钮后的原示例四项检查与两端文件/关闭流程复验通过（各2,682项）。Windows运行包同时包含编辑器、原生消息和独立基础网页三个入口；普通/共享运行库两种包装各6项检查、仓库外解压后的6项检查与3个启动脚本均实际退出0。共享运行库遗漏DLL及加载器的失败证据保留，修复后重新验证。见[原生查找与替换](XUI_DOCUMENT_NATIVE_FIND.md)、[运行包说明](XUI_DOCUMENT_RUN_PACKAGE.md)，最终核对为 `document-v1-v1125-native-find-acceptance.json`。本批独立验证受影响入口，未重复未修改内核与全部基础套件；前批接受包保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功互通、物理GPU和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，补齐原生 Document 消息示例：Rich、Markdown 和共享草稿直接嵌入 MessageList，使用真实 XGE renderer。Windows与私有X11/Mesa两端各28项检查、各6张图像通过，覆盖命中拖选、中文输入同步、三模式/历史、消息追加后的行高与后续行位置、尾部滚动和窄逻辑视口；已直接查看当前两端图像，并修正示例列表与工具栏重叠。永久原生入口已注册基础套件，本批独立验证该入口，未重复全套基础/内核。见[原生消息接入与证据](XUI_DOCUMENT_NATIVE_MESSAGES.md)，最终核对 `document-v1-v1114-native-messages-acceptance.json`；前批[独立运行包](XUI_DOCUMENT_RUN_PACKAGE.md)保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功交互、原生窗口缩放、物理GPU与完整平台发布仍待验收，高级范围继续冻结。

2026-10-09，基础原生X11剪贴板修复已通过：Rich/GFM可视、Source/Live纯文本复制/剪切/粘贴支持完整字节传输；读取失败、超出单次协议预算和含不受支持对象的可视复制/剪切均明确返回错误，失败保持文档。最终34场景/493断言通过；Linux正常137全新/零复用构建及九组26条基础命令通过。Windows沿用18入口/3消费者和文件/关闭18子命令的完整基线；最后Linux专用改动在普通/WebView2配置的三个受影响单元中预处理结果相同，当前另补8项构建/接口/原生示例/WebView复验。Linux原生示例10项检查符合预期，8图像与此前核对版本一致。见[原生X11剪贴板范围与证据](XUI_DOCUMENT_X11_CLIPBOARD.md)，最终核对为`document-v1-v1073-x11-clipboard-acceptance.json`。

当前基础实现与自动验收可供接入试用。X11文本跨进程、Linux原生XUI文件/关闭对话框与三模式基础输入队列已有私有显示自动证据；Windows系统剪贴板成功路径、真实IME/人工键鼠、桌面应用互通、物理GPU及其他平台仍待验证。HTML/CSS扩展、Document网络动态资源、完整UIA等高级范围继续冻结，整体长期目标保持未完成。下文旧批次的结果与待验描述对应记录时刻。

日期：2026-10-09。本文记录首个可用版本中的基础修复，不恢复 HTML/CSS 转换、Document 网络动态资源、完整 UIA 或高级规模矩阵。

最新删除空白补充：除段首ASCII空格外，TAB、段尾空格和格式旁Unicode空白已修复；永久删除入口扩展为33种源码/594组合，新增完整语义前缀的折叠光标检查，最终240,513断言通过。当前18项基础与3个原控件回归复验通过，详见[删除空白修正](XUI_DOCUMENT_WHITESPACE.md)。下文12种源码/216组合及v1011接受记录保留为该批历史证据。

## 已修复的行为

| 问题 | 修复后的行为 |
| --- | --- |
| SOURCE/LIVE 复制失败仍返回成功，剪切可能删除未复制的内容 | XGE 代理通过结果型剪贴板接口返回实际失败；剪切失败保留文档、选择、保存点和历史 |
| 原生剪贴板读取失败被当作空文本 | Win32 数据接口与 XUI 代理传播读取错误，不把错误作为可粘贴的空字符串 |
| 粘贴空文本替换当前选区 | 有效空剪贴板直接无操作；长度查询后内容变为空则返回 STALE，原选区内容保留 |
| Markdown 可视模式段首剪切留下空格时返回 UNREPRESENTABLE | 同一 Text 节点删除进入已有 CST 源码范围规划器，保留语义空格、未编辑源码和引用定义，通过私有候选的完整语义校验后提交 |

例如 `keep 中文 😀 tail\n` 选择并剪切 `keep 中文 😀` 后，语义内容应为 ` tail`。原始空格放在 Markdown 段首会被解析器当作缩进；现在仅将必要的边界字节写为 `&#32;tail\n`。链接标签允许保留普通前导空格，保持 `[ tail](local.path "title")` 的最小源码修改；不是所有空格都强制转换成实体。

产品修改涉及 `src/xge_render.c`、`src/xui_proxy_xge.c`、`src/xui_document_editor.c`、`src/xui_document_markdown.c` 与 `src/xui_document_markdown_edit.c`。公开 API、代理 ABI 和 Document 原生格式版本保持。没有另建 Markdown 编辑内核。

## 永久验证入口

从仓库根运行：

```bat
call test_xui\build_document_native_clipboard_test.bat
call test_xui\build_document_boundary_delete_test.bat
call test_xui\build_document_basic_suite.bat --with-webview
```

剪贴板入口覆盖 Rich VISUAL、GFM VISUAL/SOURCE/LIVE 四配置各八项，共32个场景、610次断言：失败复制/剪切/粘贴、有效空粘贴、长度查询后变为空及成功复制/剪切/粘贴。成功路径与空内容路径使用确定性的 TestProxy，原生失败路径使用真实 XGE 回调并由另一线程的窗口持有 Windows 剪贴板。所有原生写入尝试均在该锁内失败，不读取或覆盖用户剪贴板有效载荷；测试核对剪贴板序列未变化。这不代表真实系统剪贴板成功路径或跨应用 HTML 粘贴已验收。

段首删除入口覆盖12种源码、CommonMark/GFM/Extended三方言、可视 Cut/TxnReplaceRange/TxnReplaceText三入口及正反向选择，共216组。包含中文/emoji、Quote、List、Heading、Bold、Code、直接链接/引用链接、多空格、已编码空格、BOM/CRLF及未编辑引用定义。核对精确源码、字面语义、保存点、单次 Undo/Redo、保留快照及模式切换。两个事务入口分别扫描到首次成功的140个分配预算点，总计278次 OOM 失败和2次成功，失败不改 revision、保存点、历史、通知或快照，释放后无遗留分配；含分配器检查共49,679次断言。

## 实际验收证据

证据根目录为 `build/source-document-footnote-local-reference-20261006`。

| 记录 | 实际结果 |
| --- | --- |
| `document-native-clipboard-negative-v994/proof.json` | 原v988运行库编译0/运行1，四配置12个失败场景；保留复制误报、剪切误删与空粘贴的原日志 |
| `document-clipboard-fix-v995/proof.json` | 正常构建3个受影响单元重新编译、134复用，实际0 |
| `document-clipboard-positive-v998/proof.json` | 永久剪贴板入口实际0，610次断言，核对实际加载主库路径 |
| `document-clipboard-baseline-v999/proof.json` | 17项基础子入口实际0；外层消费者使用CMD正斜杠路径导致实际1，不能把外层失败记为通过 |
| `document-clipboard-consumers-v1000/proof.json` | 仅修CMD路径，三个原控件消费者均实际0，输入与运行库未变 |
| `document-md-boundary-cut-v1002/proof.json` | 当时运行库编译0/运行1，可视段首Cut返回-107，原文未丢失；这是基础缺陷，不归入高级冻结 |
| `document-boundary-delete-fix-v1003/proof.json` | 正常构建2个Markdown单元重新编译、135复用，实际0 |
| `document-boundary-delete-negative-v1008/proof.json` | 永久新回归对原运行库编译0/运行1，能捕获段首Cut缺陷 |
| `document-boundary-delete-positive-v1009/proof.json` | 同一永久回归对修复后运行库实际0 |
| `document-basic-boundary-baseline-v1010/proof.json` | 当前18项基础入口、3项原控件消费者与文件/关闭内部18条命令均实际0；1711项输入及主库/导入库/配置运行前后不变 |
| `document-v1-v1011-basic-acceptance.json` | 独立核对最终1712项输入、负例/正例日志、运行库、导出、构建报告与图像，冻结交付产物 |

最终基础套件覆盖完整现有 Core/Editor、652个 CommonMark 样本的解析/来源/原生往返/Prepare对照、renderer、格式命令、两个新回归、通用文本范围、MessageList、示例与四个原生检查、实际文件/关闭流程、298个公开Document导出及Windows基础WebView。WebView版本按依赖重编译相同两个Markdown单元、复用135；加载器自动打包，14个内部浏览器接口未公开导出。本轮增量构建不能描述为新的137全新构建。

此前夹具与收集器错误原记录保留：隔离 WindowStation 被系统拒绝、初始化后更换代理、缺失的假定 sibling DLL、测试初始化告警和CMD路径错误均独立修正。v1005/v1006段首回归最初过度要求链接标签实体拼写，改为精确最小源码黄金并继续检查实际字面语义，产品未为这个黄金修订改变；原夹具/失败可执行文件保留。没有将这些失败记作产品通过。

## 首版交付边界

主仓库基础实现与当前自动验收可供接入试用。SOURCE/LIVE/文件以及保存、放弃和关闭提示的原生图像已经直接查看。真实物理 IME、人工键鼠、其他机器和本轮新改动的Linux/移动平台复验仍待完成；没有把自动合成输入当作人工验收。全部高级设计目标仍保持未完成，原高级实现及测试保留，后续按范围逐项恢复。
