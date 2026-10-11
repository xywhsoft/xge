# Document 首版示例的 Linux 原生运行

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

2026-10-10，修复 DocumentEditor 原生查找/替换弹窗在缩小宿主窗口时超出 viewport 的问题；弹窗与内部控件按可用空间调整，窗口保持打开时查询与焦点保留。Windows/Linux 的 Rich、Source、Visual、Live 各四组真实窗口尺寸验证通过：每端2,472项检查、32张缩放/查找图像。当前运行库重新通过 Windows 全部21条基础命令（含基础WebView）、Linux 含五组原生扩展的30条外层命令，以及原有三个控件消费者。见[查找入口](XUI_DOCUMENT_NATIVE_FIND.md)、[缩放验证](XUI_DOCUMENT_NATIVE_RESIZE.md)。本批改动包含 editor 的 Find UI，内核、renderer、公开API与WebView后端源码未改；上述基础套件是本批实际重跑结果。真实IME、物理键鼠/GPU、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；HTML/CSS、Document动态网络资源、完整UIA等高级范围继续冻结。

2026-10-10，修复原生编辑示例固定1280×780布局：绘制目标、viewport和输入映射随窗口尺寸更新，工具栏换行，窄窗口纵向排列三个视图。Windows/Linux的Rich与三种Markdown模式各完成四组真实窗口尺寸、2,228项检查和16张缩放图像，覆盖缩放后的原生指针命中、内容/revision与焦点保持、继续编辑和撤销重做；两端原示例四项检查、查找383项和文件/关闭2,682项复验通过。见[原生窗口缩放](XUI_DOCUMENT_NATIVE_RESIZE.md)，最终核对 `document-v1-v1138-native-resize-acceptance.json`。新版[运行包](XUI_DOCUMENT_RUN_PACKAGE.md)的普通/共享运行库两组检查与独立解压、启动脚本均通过；此前ZIP保持原字节。本批独立执行受影响入口，未重复未修改内核及全部基础套件。原生尺寸/指针队列自动验证已完成，真实IME、物理键鼠、多DPI、Windows剪贴板成功互通和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，原生示例补齐 Find/Replace 按钮与 Ctrl+F/H 的使用说明；Rich、Markdown Source/Visual/Live 四模式在 Windows/Linux 各通过383项查找、替换、历史和焦点检查，各20张图像。新增按钮后的原示例四项检查与两端文件/关闭流程复验通过（各2,682项）。Windows运行包同时包含编辑器、原生消息和独立基础网页三个入口；普通/共享运行库两种包装各6项检查、仓库外解压后的6项检查与3个启动脚本均实际退出0。共享运行库遗漏DLL及加载器的失败证据保留，修复后重新验证。见[原生查找与替换](XUI_DOCUMENT_NATIVE_FIND.md)、[运行包说明](XUI_DOCUMENT_RUN_PACKAGE.md)，最终核对为 `document-v1-v1125-native-find-acceptance.json`。本批独立验证受影响入口，未重复未修改内核与全部基础套件；前批接受包保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功互通、物理GPU和其他平台发布仍待验收；高级范围继续冻结。

2026-10-10，补齐原生 Document 消息示例：Rich、Markdown 和共享草稿直接嵌入 MessageList，使用真实 XGE renderer。Windows与私有X11/Mesa两端各28项检查、各6张图像通过，覆盖命中拖选、中文输入同步、三模式/历史、消息追加后的行高与后续行位置、尾部滚动和窄逻辑视口；已直接查看当前两端图像，并修正示例列表与工具栏重叠。永久原生入口已注册基础套件，本批独立验证该入口，未重复全套基础/内核。见[原生消息接入与证据](XUI_DOCUMENT_NATIVE_MESSAGES.md)，最终核对 `document-v1-v1114-native-messages-acceptance.json`；前批[独立运行包](XUI_DOCUMENT_RUN_PACKAGE.md)保持原字节。真实IME/人工键鼠、Windows系统剪贴板成功交互、原生窗口缩放、物理GPU与完整平台发布仍待验收，高级范围继续冻结。

2026-10-09，基础原生X11剪贴板修复已通过：Rich/GFM可视、Source/Live纯文本复制/剪切/粘贴支持完整字节传输；读取失败、超出单次协议预算和含不受支持对象的可视复制/剪切均明确返回错误，失败保持文档。最终34场景/493断言通过；Linux正常137全新/零复用构建及九组26条基础命令通过。Windows沿用18入口/3消费者和文件/关闭18子命令的完整基线；最后Linux专用改动在普通/WebView2配置的三个受影响单元中预处理结果相同，当前另补8项构建/接口/原生示例/WebView复验。Linux原生示例10项检查符合预期，8图像与此前核对版本一致。见[原生X11剪贴板范围与证据](XUI_DOCUMENT_X11_CLIPBOARD.md)，最终核对为`document-v1-v1073-x11-clipboard-acceptance.json`。

当前基础实现与自动验收可供接入试用。X11文本跨进程、Linux原生XUI文件/关闭对话框与三模式基础输入队列已有私有显示自动证据；Windows系统剪贴板成功路径、真实IME/人工键鼠、桌面应用互通、物理GPU及其他平台仍待验证。HTML/CSS扩展、Document网络动态资源、完整UIA等高级范围继续冻结，整体长期目标保持未完成。下文旧批次的结果与待验描述对应记录时刻。

日期：2026-10-09。本轮修复示例把字体路径写死为Windows路径的问题，并验证真实XGE后端的原生渲染和文件往返。统一Document内核、公开API、原生格式、Windows WebView及高级冻结范围保持。

## 实现

`examples/xui_document/main.c`现在选择Windows原有系统字体或Linux常见DejaVu字体，并允许环境变量覆盖：

| 变量 | 用途 |
| --- | --- |
| `XUI_DOCUMENT_FONT` | 正文字体；必需，加载失败明确报告路径并退出 |
| `XUI_DOCUMENT_FONT_BOLD` | 加粗字体 |
| `XUI_DOCUMENT_FONT_ITALIC` | 斜体字体 |
| `XUI_DOCUMENT_FONT_MONO` | 源码/代码等宽字体 |
| `XUI_DOCUMENT_CJK_FONT` | 可选中文回退字体；显式配置无效路径时报告错误并退出 |

Windows未设置覆盖变量时保留Segoe UI/Consolas及微软雅黑/宋体回退。Linux默认路径为`/usr/share/fonts/truetype/dejavu`中的DejaVuSans、Bold、Oblique及Mono文件；字体安装位置不同可覆盖。DejaVu默认字体不保证中文覆盖，显示中文应提供本机字体路径。字体不打包进仓库，不能据此宣称其他三个平台已经验收。

示例的异步本地图片轮询改用现有跨平台`xgeSleep(1)`，保留原Windows等待行为，避免非Windows端连续忙轮询。POSIX文件检查使用真实进程ID生成临时文件名，减少同时运行示例时的冲突。

新增Linux构建入口：

```sh
python3 examples/xui_document/build_linux.py
./build/linux-native/xui_document
```

入口默认调用正常`tools/build_profile.py default`生成`build/linux-native/libxge.so`，再构建同目录示例，不覆盖Windows DLL。需要Python 3、GCC/G++及正常Linux构建使用的系统开发库，运行时需要可用的X显示和OpenGL。已经构建过正常/default运行库时，可用`--runtime-dir /absolute/path/to/runtime --output /absolute/path/to/xui_document`复用；本轮正式验证采用此前137全新/零复用构建的已核对运行库，并核对实际加载路径。

设置本机中文字体后可运行原有检查：

```sh
export XUI_DOCUMENT_CJK_FONT=/absolute/path/to/chinese-font.ttc
./build/linux-native/xui_document --verify
./build/linux-native/xui_document --verify-async
./build/linux-native/xui_document --verify-image-placeholder
./build/linux-native/xui_document --verify-files
```

这些命令检查Source/Live、共享预览、提交前后的图像、本地图片/缺失占位、Rich/Markdown保存重开、Unicode文件路径与编辑、Undo/Redo和失败打开保留。它们不验证人工输入或Linux打开/保存对话框。

## 实际验证

在WSL2 Ubuntu新建独立ext4源快照，以私有Xvfb显示和认证文件运行，禁止TCP监听，测试结束后服务器已退出，认证文件已清除。未使用或写入用户的活动显示/剪贴板。`glxinfo`确认Mesa llvmpipe OpenGL 4.5：这是实际X11/OpenGL后端的软件渲染，不能作为物理GPU或性能矩阵的证据。

原示例在同一正常库上编译0、运行1，实际报`C:\Windows\Fonts\segoeui.ttf`不存在。修复后默认Linux字体启动及四项原生检查全部退出0；配置不存在的正文/中文回退字体分别按预期退出1，并报告所配置路径。通过新Linux构建入口重新编译和重跑得到相同结果。

取回冻结8张Linux截图；直接查看Source、Live、异步未发布/已发布、缺失占位和中文文件编辑共6张。最终入口产物截图与已查看版本的8张哈希全部一致，表格、图片、预览和中文显示正常。CJK验证采用本机WSL可访问的Windows字体，仅记录来源与哈希，没有打包字体文件。

受影响Windows示例重新构建，四项原生检查、文件/关闭永久入口及其内部18条命令均实际0；Source/Live、中文编辑图像复验正常。主DLL、导入库、配置和WebView等既有产物保持不变。本轮不重复未改变的Core/Editor全部18项基础套件，不把增量示例检查描述成整套重新运行。

## 证据

证据根目录：`build/source-document-footnote-local-reference-20261006`。

- `document-linux-native-negative-v1031/linux-proof.json`：原示例实际字体失败，正常库与原来源哈希保留。
- `document-linux-native-positive-v1033/proof.json`：直接编译的Linux示例及原生检查通过，截图取回冻结。
- `document-example-windows-v1034/proof.json`：当前Windows示例构建/原生四检查/关闭文件18子命令通过，内核产物未变。
- `document-linux-native-positive-v1035/proof.json`：新永久Linux构建入口实际使用，原生四检查和两项预期错误通过，1716项来源输入与内核运行库保持。
- `document-v1-v1037-native-linux-acceptance.json`：最终来源、入口、日志、产物和图像独立接受。

v1032收集器误用了不存在的异步截图文件名；渲染/异步命令本身退出0，收集器失败记录保留，修正文件名后v1033/v1035通过，没有减弱产品断言。Windows私有窗口站创建仍被当前环境拒绝（错误5）；本轮探针未访问剪贴板有效载荷，用户剪贴板序号576前后相同，不把它算作原生剪贴板成功验收。

## 剩余边界

Linux原生X11/OpenGL渲染路径及文件API往返已验证；Linux实际文件对话框、真实系统剪贴板成功交互、物理IME、人工键鼠和物理GPU/跨机器仍待验证。macOS/Android/iOS和Linux WebView未新增实现或验收。高级功能继续冻结，整体设计目标尚未完成。
