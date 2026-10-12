# Document 原生窗口缩放

本批最终核对为 `document-v1-v1163-format-menu-acceptance.json`。新版 `document-format-menu-package-v1161/runnable.zip` 的普通/共享运行库两组各6项、仓库外解压6项及三个启动脚本均实际退出0。包内README保持运行验证时字节，本页随后补齐索引；内核运行库与此前ZIP保持原字节，高级范围继续冻结。

2026-10-10 格式菜单接续：原生示例新增第12个Format按钮，常规窗口下保持原视图位置，窄窗口仍自动换行。永久驱动检查全部12按钮；Windows/Linux当前各2,728项、32张缩放/查找图像通过。格式菜单另通过四组逻辑viewport的边界、原owner、Escape焦点与revision保持检查，见[格式菜单](XUI_DOCUMENT_FORMAT_MENU.md)。下方2,472/2,228项是原11按钮批次。本批内核、renderer、editor和WebView运行库未变。

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

2026-10-10 接续：修复 DocumentEditor 查找/替换弹窗超出缩小后的宿主窗口。打开时限制尺寸与位置；窗口保持打开且 viewport 改变时调整内部控件，保留查询、焦点与结果。此接续修改包含 editor 的 Find UI；下方v1138的“editor未改”仅描述当时的示例宿主修改。

Windows `document-find-resize-windows-basic-v1146` 全部21条基础命令和 Linux 导出 `document-find-resize-linux-basic-v1147` 全部30条外层命令通过；原有三个控件消费者复验通过。新增 `--with-find` 选项后，两端Rich/Source/Visual/Live各四个尺寸、2,472项检查、32张图像通过，覆盖弹窗打开时的查询/焦点保持、可见控件的正尺寸与边界。两端查找383项、文件/关闭15场景与2,682项也通过。Windows标准入口自动启用此选项，Linux `--with-native-resize` 同样启用；默认独立缩放驱动仍可只运行宿主布局检查。

修改前 `document-find-resize-negative-v1139` 在640×760窗口中因查找弹窗边界超出宿主而实际失败，旧Find UI、测试、DLL和日志已冻结。当前已直接查看Windows及Linux的窄窗口查找图像。640×760以下的完整可用布局、多DPI、真实IME/物理键鼠与其他平台发布不在这批证据内。

日期：2026-10-10。修复原生编辑示例的固定尺寸布局：窗口变化时，绘制目标、XUI viewport、输入映射、编辑区和共享预览同步调整。工具栏自动换行；宽度小于900像素时，三个视图改为纵向排列。初始1280×780布局保留。

布局变化不创建或替换Document，不提交编辑事务；三个视图保留原文档、焦点和历史。新的绘制目标创建成功后才释放旧目标。此项修改属于示例宿主，Document内核、renderer、editor与WebView后端保持不变。

## 运行与验证

```powershell
python tests/xui/build_document_demo_resize_test.py --out-dir D:\path\to\new-results --with-find --with-demo-verification
```

```sh
python3 tests/xui/build_document_demo_resize_test.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results --cjk-font /absolute/path/to/local-font.ttc --with-find --with-demo-verification
python3 tests/xui/build_document_linux_basic_suite.py --runtime-dir /absolute/path/to/runtime --with-native-resize
```

测试只操作自己创建的窗口：Windows通过SetWindowPos改变客户区尺寸并发送原生指针消息；Linux通过独立认证的Xvfb/Mesa显示上的XResizeWindow和X11指针事件操作自己的窗口。它不会访问用户剪贴板，不代表物理键鼠、真实输入法、物理GPU或DPI矩阵验收。

Rich、Markdown Source/Visual/Live分别覆盖640×760、960×640、1440×900和恢复1280×780。检查绘制目标/根控件/viewport尺寸一致、工具栏与视图边界和互不重叠、文档内容与revision不因布局变化而修改、共享预览绑定、焦点保留、缩放后的原生指针命中，以及继续中文编辑和撤销重做。每模式保存四张当前绘制目标图像。

## 先前v1138证据

`document-resize-windows-v1132`与导出的`document-resize-linux-v1133`各四模式、16张缩放图像、2,228项检查通过；原示例各四项渲染、异步、图片占位与文件往返检查也通过，另各6张图像。已直接查看当前Windows窄窗口/三栏布局与Linux窄窗口。

受影响查找入口双端各383项通过；文件/关闭流程双端各15场景、2,682项通过，Windows原生Unicode/关闭30项和Linux原生X11输入198项通过。基础套件注册新的独立入口，当前Windows默认20入口、含WebView21入口；本批运行受影响入口，没有重新运行全部基础或未修改内核。

修改前的`document-resize-negative-v1129`在真实窗口改为640×760后因根控件仍是1280×780而失败；旧示例源码、可执行文件和日志保留。`v1128`是测试首次编译时误用viewport成员名的失败，不作为产品缺陷证据。最终核对为`document-v1-v1138-native-resize-acceptance.json`。

HTML/CSS转换、Document网络动态资源、完整UIA及高级性能矩阵继续冻结。小于上述尺寸的完整可用布局、多DPI、物理窗口操作与其他平台发布仍未声称通过。

新版 `document-resize-package-v1136/runnable.zip` 的普通与共享运行库两组各6条检查，仓库外中文/空格目录6条检查和三份启动脚本均实际退出0。ZIP与验证输入已冻结，本页随后补齐证据；整体目标仍未完成。
