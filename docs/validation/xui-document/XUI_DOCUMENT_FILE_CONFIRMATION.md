# Document 首版的文件确认与恢复编辑

日期：2026-10-09。继续补齐打开、保存和取消之后的基础交互，修复示例文件流程中的焦点与待发布编辑保护。统一内核、公开 API、原生格式和通用对话框策略没有改变；高级范围保持冻结。

## 修复的行为

文件对话框关闭后，此前焦点为空，继续输入或快捷键不能到达编辑器。示例现在恢复发起文件操作的编辑器；保存成功、取消、Escape、确认打开及拒绝放弃修改均覆盖 Rich 与 Markdown。若还需显示放弃修改的确认框，该框继续正常取得模态焦点，决定结束后才回到编辑器。

“放弃修改并打开文件”的提示显示期间，宿主可能启动新的待发布编辑。此前 Yes 回调直接替换绑定，会将当前候选留在不再显示的旧文档上。现在确认时再次查询当前 Document 的 Prepare 和编辑器待处理输入；忙时提示 `XUI_DOC_ERROR_BUSY`，释放待打开的文件候选，保持当前文档、共享预览、revision、保存点、历史和未发布编辑。完成或取消编辑后，可重新发起打开操作。

这一检查补充文件选择确认时已有的保护：两次确认之间仍可能出现新编辑，不能只在文件对话框 OK 时检查一次。修复前两条独立原生探针均编译 0、运行 1，分别证明失焦和错误切换文档。

## 固定回归

原永久文件/关闭夹具已扩展，三种 Markdown 模式各执行五种关闭结果，共 15 场景、2,682 断言：

- Rich 与 Markdown 的保存取消、保存成功、拒绝丢弃、Escape、打开完成之后，焦点必须回到正确编辑器。
- 保存和重开后，通过 XUI 输入路由输入中文，再用 Ctrl+Z，内容和保存点恢复；不再靠测试手工重新设置焦点掩盖问题。
- 放弃提示打开后启动外部 Prepare，再确认 Yes：原文档和共享预览保持，候选仍在当前文档上，界面报告忙；取消 Prepare 后，键盘撤销仍能回到保存内容。
- 原有文件往返、关闭保存/放弃/取消、待发布文件确认和关闭保护继续通过。

源码输入采用正常 `xuiUpdate` 等待发布，并设五秒截止时间。快捷键辅助函数在释放按键时也清除 Ctrl 快照，避免后续普通文字被当作按住 Ctrl 的输入；早期测试错误记录保留，没有改写成通过。

在仓库根目录运行：

```bat
call examples\xui\xui_document\build.bat
call tests\xui\build_document_demo_workflow_test.bat
```

```sh
python3 tests/xui/build_document_linux_workflow_test.py --with-native-input
```

Linux 可用 `--runtime-dir`、`--out-dir` 和 `--cjk-font` 指定当前正常库、新结果目录与本地字体。需要 Xvfb、xauth、X11 开发环境和 Mesa。所有 Linux 输入及图像只在驱动自己的认证显示中执行，不访问用户桌面或剪贴板。详细环境见[Linux 文件入口](XUI_DOCUMENT_LINUX_WORKFLOWS.md)。

## 实际验收

证据根目录：`build/source-document-footnote-local-reference-20261006`。

| 记录 | 实际结果 |
| --- | --- |
| `document-file-dialog-negative-v1089` | 修复前失焦和待发布编辑脱离显示绑定的两个独立探针，编译均 0、原生运行均 1 |
| `document-file-dialog-linux-v1090`、`document-file-dialog-windows-v1091` | 首个 Markdown 连续输入断言失败；Windows 示例构建和四项原生检查先已通过，原失败保留 |
| `document-file-dialog-linux-v1092`、`document-file-dialog-windows-v1093` | 加入正常发布等待后，仍在未释放 Ctrl 的文字输入断言失败；原失败保留 |
| `document-file-dialog-linux-v1094` | 修正事件序列后，永久驱动内部 23 条命令全为 0；15 场景/2,682 断言及三模式原生队列/198 断言通过；108 张图像冻结，直接查看其中三张 |
| `document-file-dialog-windows-v1095` | 永久文件/关闭内部 18 条命令全为 0；15 场景/2,682 断言及 WM_CHAR/WM_CLOSE 的 30 断言通过；103 张流程图像冻结，直接查看其中一张；示例构建和四项原生检查沿用同源码的 v1091 成功前缀 |
| `document-v1-v1096-file-confirmation-acceptance.json` | 当前来源、日志、库、可执行文件、图像与私有显示清理核对；保留前批核心与 WebView 基线 |

本次只重新编译受影响示例/夹具并运行文件与原生输入流程，没有重复未改变的全部内核、renderer 或高级矩阵。Linux 运行库沿用已验收版本 `461bc8492ae49a99377a055df2e652c998e878fbde9fe7a20ae4cf0373a13521`，Windows 主库和 WebView 产物不变。

这些是实际 XGE 渲染和 XUI/系统事件路由的自动证据，不代表真实输入法组合/替换、物理键鼠或全平台发布。Windows 原生剪贴板成功交互、人工与平台验收仍在[待办](XUI_DOCUMENT_REMAINING.md)中。HTML/CSS 扩展、Document 网络动态资源和完整 UIA 等继续冻结，整体长期目标未声明完成。
