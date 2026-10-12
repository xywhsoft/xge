# Document 首版的 Linux 文件与关闭流程

日期：2026-10-09。补齐原生 XGE/XUI 文件对话框与关闭保护的 Linux 自动验收，使用既有跨平台示例与交互夹具。产品内核、公开 API、原生格式和 Windows WebView 均未改变；HTML/CSS 扩展、网络动态资源、完整 UIA 等高级范围继续冻结。

后续基础增量已补原生队列选项 `--with-native-input`，并修复示例取消关闭后编辑器失焦。最终三模式输入 198 断言和原有文件/关闭 2,037 断言通过，见[原生输入与焦点](XUI_DOCUMENT_X11_INPUT.md)。下文 v1080 是该增量之前的文件/关闭证据。

## 覆盖范围

`tests/xui/xui_document_demo_workflow_test.c` 直接运行实际示例的回调、文件操作和 XGE renderer；输入通过 XUI pointer/key dispatch 合成。Source、Visual、Live 三种 Markdown 模式各执行 cancel、save、discard、clean、pending 五种关闭结果，共 15 场景、2,682 断言：

- 富文本 `.xdoc` 与 Markdown `.md` 保存、重新打开、内容比较、脏状态及共享预览绑定。
- 实际 XUI 保存对话框取消、打开未保存文档时拒绝或确认丢弃、Escape 取消，以及关闭时保存/放弃/取消。
- 文件对话框打开后出现待发布编辑，确认不能替换当前文档或保存未完成候选；原文、revision、保存点和共享预览保持。
- 关闭保护依次处理两份脏文档；取消保存保持窗口和修改，未发布编辑须完成或明确取消。
- 文件与放弃确认结束后恢复正确编辑器焦点；后续中文输入/键盘撤销与提示打开后才启动的 Prepare 保护通过。新增修复与最终证据见[文件确认](XUI_DOCUMENT_FILE_CONFIRMATION.md)；下文旧批次的 2,037 断言保留对应历史范围。

这是原生 XGE/XUI 控件的自动交互验证，不是 GTK/系统原生文件选择器，也不是物理键鼠或真实输入法验收。X11 原生键鼠/窗口关闭事件队列的完整交互仍未在本轮验证。

启用新增 `--with-native-input` 可验证中文输入、取消/放弃关闭与 Ctrl+Z/Y 的原生 X11 队列路径；没有扩展为完整键鼠或物理 IME 验收。

## 永久入口

在 Linux/WSL 仓库根目录运行：

```sh
python3 tests/xui/build_document_linux_workflow_test.py
```

默认使用正常/default 构建生成独立运行库；已有当前运行库时：

```sh
python3 tests/xui/build_document_linux_workflow_test.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results --cjk-font /absolute/path/to/local-font.ttc
```

`--cjk-font` 可选，覆盖 `XUI_DOCUMENT_CJK_FONT`；其余字体配置沿用示例环境变量。字体由运行环境提供，配置缺失的字体会明确失败，报告记录显式字体的文件哈希。

Linux 基础套件新增可选组合入口：

```sh
python3 tests/xui/build_document_linux_basic_suite.py --runtime-dir /absolute/path/to/runtime --with-x11-clipboard --with-native-workflows
```

默认仍是八组/25 条命令；加剪贴板为九组/26 条，加两个选项为十组/27 条外层命令。文件/关闭驱动内部另有 18 条命令：运行库依赖、编译、可执行文件依赖及 15 次原生运行。

两条入口均可加 `--with-native-input`；基础套件要求同时指定 `--with-native-workflows`。启用后文件/关闭内部为 23 条，原生输入增加三个 Markdown 模式和六张图像；组合入口外层仍为 27 条。

需要 Linux 编译环境、`Xvfb`、`xauth` 和 Mesa 软件 OpenGL。驱动创建独立认证的私有显示并禁用 TCP，不连接用户桌面或访问用户剪贴板。超时只终止自己启动的进程组；结束时停止自己的服务器并移除认证文件。软件渲染线程数固定为一，仅用于确定性测试。

## 实际结果与证据

证据根目录：`build/source-document-footnote-local-reference-20261006`。

| 记录 | 实际结果 |
| --- | --- |
| `document-linux-workflow-diagnostic-v1077/proof.json` | Linux 原生 source/cancel 探针编译、运行及依赖检查通过，证明现有夹具可直接使用 |
| `document-linux-workflows-positive-v1078/linux-proof.json` | 十组基础回归 27 条外层命令全部退出 0；原生文件/关闭内部 18 条命令全部退出 0，15 场景/2,037 断言通过 |
| 同批 `native_report` | 原生 X11 文本剪贴板 34 场景/493 断言再次通过；其内部四条命令全部退出 0 |
| 同批 `workflow_report` | 87 张 PNG 已冻结，直接查看其中五张，覆盖 Markdown 保存、Rich 保存、关闭提示、重开后共享预览及带中文的丢弃确认；不将全部 87 张写成逐张目检 |
| `document-windows-clipboard-isolation-v1079.json` | Windows 隔离环境探针编译 0、运行 1；具名对象创建返回 Win32 5，匿名 create-only 返回 183，桌面剪贴板 sequence 保持 591→591，未执行剪贴板内容写入或读取 |
| `document-v1-v1080-linux-workflow-acceptance.json` | 最终来源、驱动、日志、图像、复用库及进程清理独立核对；Windows 的 11 项产物保持原接受哈希 |

本轮复用此前正常 137 全新/零复用构建并验收的 Linux 库，SHA-256 为 `461bc8492ae49a99377a055df2e652c998e878fbde9fe7a20ae4cf0373a13521`；没有声称本轮重新构建 137 个产品单元。新入口和基础套件集成已实际执行，既有产品源码保持。

## 仍待验证

Windows 原生跨进程剪贴板成功路径仍待验收。微软说明，剪贴板属于 WindowStation，具名 WindowStation 创建受管理员组限制；当前进程未启用管理员组，不能创建新的隔离对象。测试拒绝复用已有匿名对象，也没有改用桌面剪贴板。[WindowStation](https://learn.microsoft.com/en-us/windows/win32/winstation/window-stations)，[CreateWindowStationW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowstationw)。这是测试环境限制，不是产品失败结果。

真实输入法组合/替换、人工键鼠、桌面应用互通、物理 GPU、跨机器和 macOS/Android/iOS 发布仍待验收。基础实现可供接入试用；整体高级设计未声明完成。
