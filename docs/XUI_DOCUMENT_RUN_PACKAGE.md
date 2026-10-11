# XUI Document 首版运行包

当前交付包：`build/source-document-footnote-local-reference-20261006/document-ime-focus-package-v1214/runnable.zip`。它修复了输入法会话切换后旧提交误改文档的问题，保留 Document、MessageList 与独立基础 WebView 三个入口，以及统一文档和共同历史。Rich/Source/Visual/Live 的2,357项桥接检查、Windows22条/Linux30条完整基础命令、三个原控件消费者及两项原IME契约入口通过。普通/共享/纯Document三种包、包内说明路由、独立解压和启动脚本的实际检查见 `document-ime-focus-package-v1214/proof.json`，最终核对 `document-v1-v1216-native-ime-focus-acceptance.json`；自动桥接不代替真实IME人工验收，高级范围继续冻结。

前批交付包：`build/source-document-footnote-local-reference-20261006/document-ime-empty-package-v1192/runnable.zip`。它包含Windows原生IME空文本提交与直接TSF删除的修复，以及Document、MessageList和独立基础WebView三个启动入口；富文本与Markdown Source/Visual/Live共用Document和历史。565项原生桥接检查、Windows22条/Linux30条基础命令、三个原控件消费者及两项原有输入法契约入口通过，真实输入法仍需人工验证。普通/共享运行库各6项、纯Document包4项、仓库外解压6项及三个启动脚本按本批 `document-ime-empty-package-v1192/proof.json` 核对；三份ZIP另验证包内说明路由。最终核对 `document-v1-v1194-native-ime-empty-acceptance.json`。以下记录对应此前包。

前批交付包：`document-native-ime-package-v1180/runnable.zip`，接入Windows TSF文本快照与替换事件，原生IME桥接305项自动检查、Windows22条/Linux30条完整基础命令通过，核对 `document-v1-v1182-native-ime-bridge-acceptance.json`；真实输入法未记作通过。

此前交付包：`document-portable-guides-package-v1168/runnable.zip`。新增包内操作说明及相对链接重写，普通/共享运行库各6项、纯Document包4项、仓库外解压6项及三个启动脚本均实际退出0。三份ZIP另通过只读取归档内容的说明路由回归：带网页包各5份说明/10条本地链接，纯Document包4份说明/9条链接。原v1161缺页和越出包目录的失败记录保留在 `document-package-links-negative-v1165`；最终核对为 `document-v1-v1170-portable-guides-acceptance.json`。该批程序和内核运行库未改，包内说明为验证时版本。

解压包中的操作说明：[查找与替换](XUI_DOCUMENT_NATIVE_FIND.md)、[窗口缩放](XUI_DOCUMENT_NATIVE_RESIZE.md)、[格式菜单](XUI_DOCUMENT_FORMAT_MENU.md)。打包器将这些说明放入 `docs/`，并把本页及网页说明中的链接改为包内相对路径；每次生成包都会检查链接目标，运行示例不需要源码目录。

本批最终核对为 `document-v1-v1163-format-menu-acceptance.json`。新版 `document-format-menu-package-v1161/runnable.zip` 的普通/共享运行库两组各6项、仓库外解压6项及三个启动脚本均实际退出0。包内README保持运行验证时字节，本页随后补齐索引；内核运行库与此前ZIP保持原字节，高级范围继续冻结。

本批格式菜单运行包为 `document-format-menu-package-v1161/runnable.zip`。点击 Rich 或 Markdown 编辑区后打开 **Format**，可选择段落、三级标题、三类列表、引用、代码块与常用文字格式；不可用项按当前模式和选区禁用。Source/Live 需要结构格式时切换到 Visual；插入代码块应先取消整段选区并放置光标。菜单关闭后可直接继续编辑，Ctrl+Z/Y使用原文档历史。当前示例已完成Windows/Linux受影响入口验证；以下v1152等包是此前批次记录。

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

本批运行包：`document-find-popup-package-v1150/runnable.zip`。编辑器和查找/替换弹窗随宿主窗口尺寸调整；连续缩放保留查询、焦点、内容和撤销历史。普通运行库与启用WebView的共享运行库均按本页命令验证，实际包内、独立解压与启动脚本结果记录在本批 `proof.json`。当前运行库已通过 Windows21条与Linux30条基础外层命令，以及三个原有控件消费者；此处下方v1138/v1125/v1102描述保留为此前批次证据。

日期：2026-10-10。本包用于在 Windows 上试用统一 Document 的富文本与 Markdown，以及独立的基础 WebView 示例。自动运行证据不代替真实输入法、人工键鼠或跨机器发布验收；HTML/CSS 转换、Document 网络动态资源、完整 UIA 等高级范围继续冻结。

更新：原生编辑示例的绘制目标、输入viewport和视图随窗口尺寸调整，工具栏换行；宽度小于900像素时三个视图纵向排列。缩放保留内容、绑定和历史，见[原生窗口缩放](XUI_DOCUMENT_NATIVE_RESIZE.md)。本次运行包核对为 `document-v1-v1138-native-resize-acceptance.json`；下方v1125与v1102内容是此前批次记录。

此前包为 `document-resize-package-v1136/runnable.zip`：普通与共享运行库各6条包内检查、仓库外6条检查和三份启动脚本均实际退出0。对应源码/产物/日志核对为 `document-v1-v1138-native-resize-acceptance.json`。此ZIP已冻结，包内README为运行验证时版本，本页证据随后补齐。

## 解压与运行

完整解压 ZIP 后，双击 `Open Document.cmd`。它进入 Document 子目录再运行示例，资源与诊断输出不依赖源码仓库的当前工作目录。左侧是富文本，中间是 Markdown，右侧是共享预览；Source/Visual/Live 三种模式共用文档和撤销历史。Open/Save as 支持原生 `.xdoc` 与 Markdown `.md`，取消和确认结束后恢复编辑焦点，关闭时提供保存/放弃/取消。较新版本的 Find/Replace 按钮或 Ctrl+F/H 打开当前编辑器的查找窗口，支持结果、导航、匹配选项和替换；Escape 返回编辑器。

包内若有 `Open Messages.cmd`，可运行独立的 Document 消息示例：富文本与 Markdown 在 MessageList 中显示，下方临时草稿与一条消息共享文档，支持三模式、选择和共同历史；Append reply 演示本地追加与行高更新。此示例的草稿只保留到本次运行结束。

包内若包含 `Open WebView.cmd`，可单独启动网页示例。初始页面使用自包含 HTML，支持文字输入、选择、链接和折叠内容；工具栏支持输入 URL、Go、Local HTML、Focus 与缩放。它独立于 Document/MessageList，只使用公开网页 API。

运行包携带 XGE DLL、示例和本地 PNG；不需要 GCC、Python 或源码目录来运行示例。字体使用本机 Windows 字体，不捆绑字体。WebView 可选包携带 `WebView2Loader.dll`，仍需本机安装 WebView2 Runtime；加载器不是浏览器运行时。普通网页示例使用 `%LOCALAPPDATA%\XGE\WebViewDemo` 的独立 profile，自动验证使用进程专属临时 profile。没有自动下载引擎或字体。

可以直接运行程序，或在对应子目录执行自动检查：

```bat
"Open Document.cmd" --verify
"Open Messages.cmd" --verify
"Open WebView.cmd" --verify
```

根目录启动脚本会切换到正确目录；也可先进入对应子目录，再执行程序。Document 的图像检查会写入其 `artifacts/xui-document-rebuild` 子目录；文件往返检查使用临时文件并在成功后删除。

## 基础人工检查

1. Rich 和 Markdown 三模式分别测试中文输入法组合、候选、替换与取消；切换模式确认内容和撤销历史没有丢失。
2. 测试鼠标选择、滚动、Ctrl+A/C/X/V、Ctrl+Z/Y、加粗/斜体；用本地样例检查表格、图片、链接和缺失资源占位。
3. 打开、另存为、重开并比较内容；取消文件和关闭提示后直接继续输入；验证关闭保存、放弃和取消。
4. WebView 单独测试本地页面输入、页面链接、URL 导航、焦点、缩放和窗口操作。此检查不要求应用数据通信。

记录机器、Windows 版本、输入法、失败步骤和涉及文件。自动验证的合成事件不能填写成上述人工项目已经通过。Windows 系统剪贴板成功交互和其他平台发布仍待验收。

## 重新打包

先通过仓库正常构建入口生成 Document 和可选 WebView2 运行库/示例，再从仓库根目录运行：

```powershell
python tools/package_xui_document.py --out-dir D:\path\to\new-package --verify
python tools/package_xui_document.py --out-dir D:\path\to\new-package-with-browser --webview-dir build/webview-basic --verify
python tools/package_xui_document.py --out-dir D:\path\to\new-package-with-messages --webview-dir build/webview-basic --messages-exe build/xui_document_messages.exe --verify
```

`--runtime-dir` 与 `--document-exe` 可指定已经构建的当前版本。WebView 示例通过 `python examples/xui_webview/build.py` 构建；启用后端的说明见仓库 `examples/xui_webview/README.md`。消息示例先通过 `python examples/xui_document_messages/build.py` 构建，再用 `--messages-exe` 明确加入包。输出目录、相邻 ZIP 和报告必须均不存在，失败证据不会被覆盖。

打包器记录输入和文件 SHA-256、PE 架构、真实退出码及日志。`--verify` 从包自己的子目录运行四项 Document 检查和可选网页/消息检查，PATH 仅保留 Windows 系统目录，清除字体覆盖。`manifest.json` 用于核对解压文件；邻接 `.report.json` 和 ZIP 哈希用于核对打包结果。没有启用 `--verify` 的包不会声明已通过自动运行检查。

包内说明路由检查始终执行，未带 `--verify` 也会拒绝缺失目标或越出包目录的链接。永久归档回归入口只读取ZIP，无需以源码目录补齐链接目标：

```powershell
python test_xui/build_document_package_docs_test.py --archive D:\path\to\new-package.zip --out-dir D:\path\to\new-guide-check
```

当前增量验收记录为仓库 `build/source-document-footnote-local-reference-20261006/document-v1-v1125-native-find-acceptance.json`，对应增加查找/替换按钮及可选消息示例的运行包。打包器允许 Document 与 WebView 使用同一运行库目录，分别复制两份应用目录所需的 DLL；Document 运行库目录若带有 WebView2Loader.dll，也复制到 Document 目录并携带 SDK 声明。普通禁用网页后端的 Document 运行库不需要加载器。此前按来源键去重而遗漏 Document DLL、以及共享启用网页的运行库时遗漏 Document 加载器的失败记录均保留。旧交付包和记录不被覆盖。未修改的内核/renderer/WebView 基线继承，不声明整体高级目标完成。


当前更新包为 `document-find-package-v1123/runnable.zip`。普通和共享启用网页运行库各6条包内检查，仓库外中文/空格目录6条检查、3份启动脚本均实际退出0；解压前后清单和不可变程序/加载器哈希核对通过。v1121与v1122两条包装失败记录保留。当前ZIP已冻结；本页验收说明随后补齐，包内README为运行时冻结版本。

## 首轮交付验收历史（v1102）

本批 `document-run-package-v1101/runnable.zip` 为 Windows x64 运行包。包内四项 Document 检查及一项独立 WebView 检查全部实际退出0；另在源码仓库外的中文/空格目录解压，先核对所有清单文件，再运行相同五项检查和两份根目录启动脚本，七项全部实际退出0。两组运行均使用仅含 Windows 系统目录的 PATH，没有编译器、Python 或源码路径依赖。

禁用网页后端的独立示例实际退出1并报告不支持；纯 Document 包也已成功生成，未请求验证时清单与报告均不声明自动验收通过。首轮包装验证因 Python 对 CMD 带空格脚本名称的引号转义方式错误而失败，该记录保留；更正测试调用后完整复验通过，没有修改启动脚本来规避问题。

Document 的六张解压后图像与此前已接受基线逐字节一致，本批另实际查看渲染和文件往返两张。WebView 的工具栏 PNG 仅来自 XGE 工具栏表面，本批已查看；它不能证明浏览器子窗口像素或网页人工交互通过。原11项 Windows构建产物不变，另新增公开API网页示例；既有Linux与内核基线继承，本批没有重新运行完整内核或Linux套件。

打包后的 ZIP 与日志保持原字节。本页与示例 README 的验收记录随后补齐，因此包内 README 是运行时冻结的说明版本；最终验收记录明确区分这些文档变化与已验证的程序输入。整体长期目标仍未完成，高级功能按首版范围冻结。
