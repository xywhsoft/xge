# Document 消息的原生接入示例

日期：2026-10-10。首版已有 MessageList 文档绑定回归，但原生消息示例仍只展示普通字符串。本批增加 `examples/xui_document_messages`，直接使用公开 Document 与 MessageList API，在真实 XGE 后端展示富文本、Markdown 和共享编辑草稿；不修改文档内核或浏览器后端。

## 接入方式

富文本消息通过统一事务创建段落与文字格式。Markdown 消息包含标题、粗体、表格、代码块与中文引用，由同一 Document renderer 绘制。每条消息用 `xuiMessageListSetNodeDocument` 绑定；MessageList 持有文档和 renderer，应用销毁控件后释放自己的文档与字体。

下方草稿编辑区和第三条消息共用一个 Markdown Document，Source/Visual/Live 三模式沿用共同历史。编辑发布后，消息显示与可选择文字来自同一个已提交版本。第二条消息的本地追加使用 SOURCE 域事务；该文档提交触发行高与后续行位置更新，无须重新绑定消息。

示例仅使用本地内容和系统字体。它展示接入方式，草稿只保留到本次运行结束；已有独立 Document 示例继续承担原生文件、Markdown 文件及关闭保存流程。

## 重复验证

```powershell
python test_xui/build_document_messages_test.py --out-dir D:\path\to\new-results
```

```sh
python3 test_xui/build_document_messages_test.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results --cjk-font /absolute/path/to/local-font.ttc
python3 test_xui/build_document_linux_basic_suite.py --runtime-dir /absolute/path/to/runtime --with-native-messages
```

独立入口保护源码与运行库哈希，保留真实编译/运行日志和六张图像。Windows 基础套件已注册此入口；Linux 作为可选组注册，仍可只运行无窗口基础回归。Linux 原生入口使用独立认证 Xvfb 和 Mesa 软件 OpenGL，结束后终止自己的显示进程并移除认证文件。

验证覆盖：三条消息的 Rich/MD 文档绑定、真实命中与拖选、选中文字提取、中文输入同步、三模式无损切换、撤销重做、追加后的行高与后续行位置、绑定保留、尾部滚动和窄逻辑视口。图像分别为初始显示、选区、共享草稿、追加回复、滚动末尾和窄视口。

自动验证只向自己的缓冲区提取选中文字，没有读写系统剪贴板；不能记为 Windows 原生剪贴板成功验收。输入为合成 XUI 事件，窄视口为逻辑布局调整；真实输入法、物理键鼠、原生窗口缩放、物理 GPU 与跨机器发布仍待验证。MessageList 高级对象交互、HTML/CSS 转换、Document 网络动态资源及 UIA 的冻结范围保持。


## 当前验收记录

Windows 永久入口位于 `document-messages-windows-v1112`，编译和运行两条命令均实际退出0；Linux 位于导出的 `document-messages-linux-v1113`，编译、实际依赖核对和运行三条命令均实际退出0。两端均完成28项检查和6张图像。Linux复用此前正常137全新/零复用构建的库，库与配置哈希不变；所有原12项Windows运行产物不变，另交付 `build/xui_document_messages.exe`。

当前Windows末尾滚动、窄视口及Linux中文共享草稿三张图像已直接查看。首次原生结果的人工图像复核发现列表与工具栏重叠，修正列表高度后新增区域边界断言；最终末尾滚动及窄视口中消息裁剪在列表区域，工具栏与编辑区不重叠。编译接口错误、节点输出参数错误、源码事务域错误等早期示例失败日志保留，不计为最终通过。

Windows基础入口注册后含18项默认检查、启用WebView时19项；Linux的 `--with-native-messages` 新增一个可选组/一条外层命令。本轮实际执行永久消息入口，未把注册后的完整19项Windows套件或新增Linux组合描述成已整批重跑。原内核、renderer与编辑器等接受基线继承，整体高级目标尚未完成。

最终核对记录为 `build/source-document-footnote-local-reference-20261006/document-v1-v1114-native-messages-acceptance.json`，记录源码、平台日志、运行库、可执行文件、当前实际查看图像及文档补记。此前v1102运行包未被改写，本例单独从仓库构建/运行。
