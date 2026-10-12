# Document 基础原生输入与取消关闭后的焦点

日期：2026-10-09。首版继续优先处理常用输入、文件和窗口操作。新增 X11 原生事件队列回归，并修复示例取消关闭后失去编辑器焦点的问题。HTML/CSS 扩展、Document 网络动态资源、完整 UIA 等高级范围保持冻结。

后续[文件确认增量](XUI_DOCUMENT_FILE_CONFIRMATION.md)已补保存/打开后的焦点与提示打开后新 Prepare 的保护。当前文件/关闭 15 场景为 2,682 断言，Linux 启用原生输入后共 108 张图像；下文 v1088 的 2,037 断言/93 图像是该增量之前的记录。

## 修复的行为

有未保存修改时关闭窗口，点击 Cancel 会保留内容并关闭提示。此前提示隐藏后焦点为空，继续输入或 Ctrl+Z 不会到达编辑器。示例的 `close_result` 现在将焦点恢复到记住的 `active_editor`，再显示取消状态；Rich 与 Markdown 编辑器共用这一回调。没有改变通用 MsgBox 的焦点策略、Document 内核、公开 API 或原生文件格式。

修复前的 Linux 记录明确显示 `source_kept=1 editor_focus=0`，原生运行退出 1。使用冻结的旧示例和新的 Windows 焦点断言，也实际退出 1。修复后的相同流程通过；失败记录仍保留为失败。

## 原生队列覆盖

`tests/xui/xui_document_demo_x11_input_test.c` 直接运行实际示例和 Sokol/XGE 循环，使用第二个 Xlib 连接发送 KeyPress/Release、Motion/Button 和 WM_DELETE_WINDOW。三个 Markdown 模式各完成 66 断言，共 198：

- 中文事件在关闭请求之前进入文档；精确源码、revision、脏状态和共享预览一致。
- 原生鼠标取消关闭，窗口保留，内容和编辑器焦点恢复。
- 原生 Ctrl+Z/Y 恢复原内容和保存状态，再恢复修改及历史。
- 再次原生关闭，鼠标选择 Discard 后正常退出，关闭前内容没有被意外修改。

夹具在 XGE 打开连接之前，为私有显示的两个测试键码设置中文 Unicode keysym，并在退出时恢复映射。它不修改用户桌面的键盘映射，也不直接调用 InsertText 或编辑器撤销命令来替代上述输入步骤。每模式实际计数为 text=4、keys=4、mouse=6、quit=2；文本事件计数也包含快捷键产生的字符事件，不能据此声称插入了四个汉字。

这是经过 X11 和 XGE 原生队列的合成事件测试，仍不代表物理键盘、真实输入法组合/候选/替换、窗口管理器或完整键鼠矩阵。Windows 永久关闭夹具也增加了取消后焦点必须回到 Markdown 编辑器的断言，WM_CHAR/WM_CLOSE 路径通过 30 断言。

## 永久运行入口

在 Linux/WSL 仓库根目录运行：

```sh
python3 tests/xui/build_document_linux_workflow_test.py --with-native-input
```

已有当前正常/default 库时，可指定独立的新结果目录和本地中文字体：

```sh
python3 tests/xui/build_document_linux_workflow_test.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results --cjk-font /absolute/path/to/local-font.ttc --with-native-input
```

组合基础套件：

```sh
python3 tests/xui/build_document_linux_basic_suite.py --with-x11-clipboard --with-native-workflows --with-native-input
```

`--with-native-input` 在基础套件中必须与 `--with-native-workflows` 一起使用。需要 X11 开发库、Xvfb、xauth、Mesa 及项目正常构建依赖；字体由本地环境提供。驱动只连接自己的认证显示，禁用 TCP，超时只终止自己启动的进程组，最后关闭服务器、移除认证文件。不要绕过驱动单独在桌面运行键盘映射夹具。

基础套件仍是十组、27 条外层命令。文件/关闭驱动默认 18 条，启用原生输入后为 23 条：多两条编译/依赖检查和三次原生运行。报告分别记录三模式输入通过、原生队列通过和物理 IME 未验证；原有 15 个文件/关闭场景保持。

Windows 回归继续使用：

```bat
call examples\xui\xui_document\build.bat
call tests\xui\build_document_demo_workflow_test.bat
```

## 本次实际证据

证据根目录为 `build/source-document-footnote-local-reference-20261006`。

| 记录 | 实际结果 |
| --- | --- |
| `document-x11-input-diagnostic-v1082` | 修复前内容保留、焦点丢失，原生运行 1；编译 0，私有显示已清理 |
| `document-input-negative-frozen-v1086` | 只收集旧失败的源码、日志、图像和产物；收集 0 不改变原运行 1 |
| `document-x11-input-diagnostic-v1083` | 修复后三模式原生输入均 0，198 断言通过 |
| `document-linux-workflows-positive-v1084` | 永久组合入口 27 条外层命令全部 0，文件/关闭内部 23 条全部 0；15 场景/2,037 断言与三模式原生输入/198 断言通过；剪贴板 34 场景/493 断言通过 |
| 同批图像 | 93 张冻结；直接查看最终三张原生输入图像，覆盖 Source 关闭提示、Visual 与 Live 取消后中文和共享预览；不声称逐张查看全部图像 |
| `document-input-windows-v1085` | 旧示例在新焦点断言实际失败 1；新示例构建、四项原生检查、文件/关闭内部 18 条命令全部 0；最终关闭提示图像已直接查看 |
| `document-v1-v1088-native-input-acceptance.json` | 来源、日志、产物、图像与清理核对；Windows 仅重建示例，内核和 WebView 产物沿用不变的接受版本 |

Linux 复用此前正常构建并验收的运行库，SHA-256 为 `461bc8492ae49a99377a055df2e652c998e878fbde9fe7a20ae4cf0373a13521`，本次没有重建全部 137 个产品单元。示例和测试在当前输入快照重新编译。

基础实现与自动验证可供接入试用。真实 IME、人工键鼠、Windows 原生剪贴板成功交互、桌面应用互通、物理 GPU 和其他平台发布仍待验收；[当前待办](XUI_DOCUMENT_REMAINING.md)保持这些边界。整体高级目标尚未完成。
