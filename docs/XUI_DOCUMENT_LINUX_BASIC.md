# Document 首版的 Linux 基础验证

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

2026-10-10，修复 DocumentEditor 原生查找/替换弹窗在缩小宿主窗口时超出 viewport 的问题；弹窗与内部控件按可用空间调整，窗口保持打开时查询与焦点保留。Windows/Linux 的 Rich、Source、Visual、Live 各四组真实窗口尺寸验证通过：每端2,472项检查、32张缩放/查找图像。当前运行库重新通过 Windows 全部21条基础命令（含基础WebView）、Linux 含五组原生扩展的30条外层命令，以及原有三个控件消费者。见[查找入口](XUI_DOCUMENT_NATIVE_FIND.md)、[缩放验证](XUI_DOCUMENT_NATIVE_RESIZE.md)。本批改动包含 editor 的 Find UI，内核、renderer、公开API与WebView后端源码未改；上述基础套件是本批实际重跑结果。真实IME、物理键鼠/GPU、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；HTML/CSS、Document动态网络资源、完整UIA等高级范围继续冻结。

2026-10-10，修复原生编辑示例固定1280×780布局：绘制目标、viewport和输入映射随窗口尺寸更新，工具栏换行，窄窗口纵向排列三个视图。Windows/Linux的Rich与三种Markdown模式各完成四组真实窗口尺寸、2,228项检查和16张缩放图像，覆盖缩放后的原生指针命中、内容/revision与焦点保持、继续编辑和撤销重做；两端原示例四项检查、查找383项和文件/关闭2,682项复验通过。见[原生窗口缩放](XUI_DOCUMENT_NATIVE_RESIZE.md)，最终核对 `document-v1-v1138-native-resize-acceptance.json`。新版[运行包](XUI_DOCUMENT_RUN_PACKAGE.md)的普通/共享运行库两组检查与独立解压、启动脚本均通过；此前ZIP保持原字节。本批独立执行受影响入口，未重复未修改内核及全部基础套件。原生尺寸/指针队列自动验证已完成，真实IME、物理键鼠、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，原生示例补齐 Find/Replace 按钮与 Ctrl+F/H 的使用说明；Rich、Markdown Source/Visual/Live 四模式在 Windows/Linux 各通过383项查找、替换、历史和焦点检查，各20张图像。新增按钮后的原示例四项检查与两端文件/关闭流程复验通过（各2,682项）。Windows运行包同时包含编辑器、原生消息和独立基础网页三个入口；普通/共享运行库两种包装各6项检查、仓库外解压后的6项检查与3个启动脚本均实际退出0。共享运行库遗漏DLL及加载器的失败证据保留，修复后重新验证。见[原生查找与替换](XUI_DOCUMENT_NATIVE_FIND.md)、[运行包说明](XUI_DOCUMENT_RUN_PACKAGE.md)，最终核对为 `document-v1-v1125-native-find-acceptance.json`。本批独立验证受影响入口，未重复未修改内核与全部基础套件；前批接受包保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功互通、物理GPU和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，补齐原生 Document 消息示例：Rich、Markdown 和共享草稿直接嵌入 MessageList，使用真实 XGE renderer。Windows与私有X11/Mesa两端各28项检查、各6张图像通过，覆盖命中拖选、中文输入同步、三模式/历史、消息追加后的行高与后续行位置、尾部滚动和窄逻辑视口；已直接查看当前两端图像，并修正示例列表与工具栏重叠。永久原生入口已注册基础套件，本批独立验证该入口，未重复全套基础/内核。见[原生消息接入与证据](XUI_DOCUMENT_NATIVE_MESSAGES.md)，最终核对 `document-v1-v1114-native-messages-acceptance.json`；前批[独立运行包](XUI_DOCUMENT_RUN_PACKAGE.md)保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功交互、原生窗口缩放、物理GPU与完整平台发布仍待验收，高级范围继续冻结。

2026-10-09，基础原生X11剪贴板修复已通过：Rich/GFM可视、Source/Live纯文本复制/剪切/粘贴支持完整字节传输；读取失败、超出单次协议预算和含不受支持对象的可视复制/剪切均明确返回错误，失败保持文档。最终34场景/493断言通过；Linux正常137全新/零复用构建及九组26条基础命令通过。Windows沿用18入口/3消费者和文件/关闭18子命令的完整基线；最后Linux专用改动在普通/WebView2配置的三个受影响单元中预处理结果相同，当前另补8项构建/接口/原生示例/WebView复验。Linux原生示例10项检查符合预期，8图像与此前核对版本一致。见[原生X11剪贴板范围与证据](XUI_DOCUMENT_X11_CLIPBOARD.md)，最终核对为`document-v1-v1073-x11-clipboard-acceptance.json`。

当前基础实现与自动验收可供接入试用。X11文本跨进程、Linux原生XUI文件/关闭对话框与三模式基础输入队列已有私有显示自动证据；Windows系统剪贴板成功路径、真实IME/人工键鼠、桌面应用互通、物理GPU及其他平台仍待验证。HTML/CSS扩展、Document网络动态资源、完整UIA等高级范围继续冻结，整体长期目标保持未完成。下文旧批次的结果与待验描述对应记录时刻。

日期：2026-10-09。补验当前主仓库的统一 Document、富文本/Markdown 编辑与显示内核，覆盖近期格式命令、候选回写和删除空白修正。HTML/CSS 扩展、动态网络资源、完整 UIA、高级矩阵和其他平台 WebView 继续冻结；本轮没有修改产品内核。

后续原生示例验证已完成：修正Windows固定字体路径，新增Linux示例构建入口，在私有X11/Mesa软件OpenGL显示上通过渲染、异步、图片占位和文件往返，中文截图已查看。见[原生Linux验证](XUI_DOCUMENT_LINUX_NATIVE.md)。下文“没有Linux原生GPU窗口”是v1025—v1029批次边界；当前已验证原生X11/OpenGL路径，物理GPU、真实IME/系统剪贴板和人工交互仍待验证。

## 可重复运行的入口

在 Linux 或 WSL 中，从仓库根目录运行：

```sh
python3 test_xui/build_document_linux_basic_suite.py
```

需要 Python 3、GCC/G++ 和正常 XGE Linux 构建使用的 X11/Xi/Xcursor/OpenGL 开发库。入口调用 `tools/build_profile.py default`，产物、测试可执行文件、日志及实际退出记录放在新的 `build/document-linux-basic-*` 目录。已有正常/default 运行库可显式复用：

```sh
python3 test_xui/build_document_linux_basic_suite.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results
```

运行库目录须含 `libxge.so` 和 `xge_build_config.h`。每个库消费者用绝对 RPATH 链接，并用 `ldd` 核对实际加载路径；测试前后核对运行库及配置哈希。原 Core 文件测试需要的 `build/document` 目录也由入口创建。结果目录必须为新目录，失败保留实际日志。Windows 原基础入口保持，用于原生窗口、文件/关闭流程和 Windows WebView。

## 实际结果

本次在 WSL2 Ubuntu 的独立 ext4 快照上验证，源码来自当前 MAIN，1713 项已有来源/规则/文档输入及38项实际存在的本地测试数据逐项核对。没有复制旧运行库或对象；正常入口生成137个全新编译单元、零复用，136个普通对象和1个既有可选 HarfBuzz 共享对象，实际退出0。构建阶段及其子进程限制为2 GiB地址空间；后续永久基础入口没有据此宣称固定机器性能验收。

随后新增永久 Linux 基础入口，以该正常构建产物执行25条命令，全部实际退出0：

| 基础组 | 验证结果 |
| --- | --- |
| Document Core | 完整现有原测试通过，包括事务、快照、源码/语义、原生持久化与历史 |
| Renderer | 完整现有代理布局、绘制、命中、资源占位等测试通过 |
| Editor | 完整现有输入/选择、代理剪贴板、编辑命令、IME投影和三种Markdown模式测试通过 |
| 格式命令 | 55,959断言；109/219个分配预算点通过 |
| 删除边界 | 33源码×3方言×3入口×2方向，共594组；240,512断言、8组分配预算扫描通过 |
| 通用文本范围 | 四种文档/编辑模式的已有范围回归通过，不能扩大为完整UIA |
| MessageList | Document绑定、测量绘制、选择复制与更新的已有基础回归通过 |
| CommonMark | 652解析/来源/原生往返样本、30,940来源位置及Prepare对照通过；不代表HTML一致性 |

Linux删除断言比Windows少1次，仅因Windows特有运行库路径检查在Linux不编译；所有语义、历史和光标检查保持。另有12个表格/脚注公开可视Cut探针，在Windows和Linux均编译/运行0；它们是额外探针，不替代永久回归的Undo、光标与分配失败检查。

正常 Linux 库 SHA-256：`a523d8ce1501a4e313ffa418add1305c670b13862812b7f0deca34b18884b7af`。Windows既有主DLL、导入库、配置、示例和WebView产物在本轮前后保持，不重复描述为本轮重跑Windows全部18项入口/3消费者。

## 证据与失败记录

证据根目录：`build/source-document-footnote-local-reference-20261006`。

- `document-linux-basic-v1025/linux-proof.json`：正常137全新/零复用构建通过；第一次Core运行因测试输出目录缺失退出1，整次记录保留为失败。
- `document-linux-basic-v1026/proof.json`：只补测试入口创建目录，再执行原测试；永久入口25条命令实际0，原1713项输入和Windows产物未变。
- `document-linux-basic-collected-v1028/proof.json`：源快照、运行库、日志、可执行文件和额外12探针独立核对；Linux产物与日志已取回冻结副本。
- `document-v1-v1029-linux-basic-acceptance.json`：最终新增入口、说明文档、Windows原接受产物和Linux证据的独立接受记录。

v1024准备阶段要求两个MAIN不存在的可选Bidi数据文件，尚未启动构建；收缩为本轮所需且实际存在的数据后继续。v1027收集器错误匹配探针输出摘要，探针本身编译/运行均0；修正收集器后v1028通过。这些失败保留，不修改产品或减弱测试断言，不记为产品通过。

## 尚未验收

此结果确认当前 Linux 文档基础代码可构建，原库消费者回归可通过。Renderer/Editor使用TestProxy，IME仅覆盖已有协议投影；没有Linux原生GPU窗口、物理IME、真实系统剪贴板成功交互、读屏或Linux WebView的验收。Windows专用的原生剪贴板锁定失败测试仍由Windows入口验证。macOS/Android/iOS及跨机器发布继续待验证。首版可用于接入试用，整体高级设计目标保持未完成。
