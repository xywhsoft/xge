# Document 原生查找与替换入口

本批最终核对为 `document-v1-v1163-format-menu-acceptance.json`。新版 `document-format-menu-package-v1161/runnable.zip` 的普通/共享运行库两组各6项、仓库外解压6项及三个启动脚本均实际退出0。包内README保持运行验证时字节，本页随后补齐索引；内核运行库与此前ZIP保持原字节，高级范围继续冻结。

2026-10-10，原生编辑示例新增 Format 菜单：段落、三级标题、三类列表、引用、代码块、下划线、删除线、行内代码和清除格式共13项命令。菜单复用现有能力查询、事务与历史；按当前模式、只读和选区禁用不支持的操作，关闭后恢复原编辑器焦点。Windows/Linux 的 Rich、Source、Visual、Live 各通过3,624项查找/格式检查（含3,241项菜单检查、28图像），两端实际窗口缩放各2,728项/32图像，原示例四项和文件/关闭流程复验通过。见[格式菜单](XUI_DOCUMENT_FORMAT_MENU.md)。本批仅改示例与永久验证入口，运行库、内核、renderer、editor与WebView后端源码未改；未重复全部基础/内核套件，沿用v1152的Windows21/Linux30完整基础证据。高级范围继续冻结，物理IME/键鼠、DPI、Windows剪贴板成功互通和其他平台发布仍待验收。

在独立查找驱动中添加 `--with-format` 可同时验证新格式菜单；默认仍是383项查找/替换、20张图像。启用该选项完成3,624项、28张图像，其中3,241项来自格式菜单；Windows基础入口与Linux `--with-native-find` 自动启用。菜单四组尺寸是逻辑viewport边界检查，实际OS窗口缩放另由[窗口驱动](XUI_DOCUMENT_NATIVE_RESIZE.md)验证，不将其记为物理窗口/DPI验收。

本批最终验收为 `document-v1-v1152-find-popup-acceptance.json`。新版 `document-find-popup-package-v1150/runnable.zip` 的普通与共享运行库各6项检查、仓库外独立解压6项检查及三个启动脚本均实际退出0；包内README保持验证时字节，本页随后补齐验收索引。高级范围继续冻结，整体长期目标仍未完成。

2026-10-10 更新：查找/替换窗口在打开和宿主 viewport 改变时按可用尺寸调整，并将位置约束在宿主内。查询、匹配结果与输入焦点保持，不创建新文档或历史。可用 `build_document_demo_resize_test.py --with-find` 验证打开状态下的连续缩放，见[缩放验证](XUI_DOCUMENT_NATIVE_RESIZE.md)。本批修改 editor 的 Find UI；以下383项入口证据是先前批次，本批 Windows/Linux 基础套件也已复验该入口。

日期：2026-10-10。DocumentEditor 已有原生查找窗口、快捷键与统一查找/替换内核。本批为原生示例增加 Find 与 Replace 工具栏按钮，打开最后活动编辑器已有的窗口，不另建搜索模型或编辑历史。

## 使用

先点击富文本或 Markdown 编辑区，再点击 Find/Replace，或按 Ctrl+F/H。窗口提供上一处、下一处、结果列表、大小写/整词/正则/选区选项，以及当前替换和全部替换。结果、高亮与选择仍由 DocumentView 管理；替换进入编辑器事务与共同撤销历史。Escape 关闭窗口并返回编辑区，后续输入继续进入原文档。

富文本和 Markdown 可视模式按语义文字查找；Markdown Source/Live 按源码查找。切换模式后仍使用相同 Document，查找结果按当前投影更新。原文件和关闭保护流程保持。

## 永久验证

```powershell
python tests/xui/build_document_demo_find_test.py --out-dir D:\path\to\new-results --with-demo-verification
```

```sh
python3 tests/xui/build_document_demo_find_test.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results --cjk-font /absolute/path/to/local-font.ttc --with-demo-verification
python3 tests/xui/build_document_linux_basic_suite.py --runtime-dir /absolute/path/to/runtime --with-native-find
```

四种模式逐一启动真实 XGE 窗口，以公开控件 API 查找子控件，并从实际位置发送指针与键盘事件。测试不访问系统剪贴板。Linux 使用独立认证的 Xvfb/Mesa 软件 OpenGL，完成后终止自己的显示进程、移除认证文件；它不是物理 GPU 或真实输入法验收。

验证覆盖工具栏打开、查询和结果、上一处/下一处、大小写/整词/正则选项、非法模式保持内容、当前替换、全部替换、撤销重做、关闭恢复焦点、Ctrl+F/H 和继续中文输入。每模式保存查找、非法表达式、当前替换、全部替换、恢复编辑五张图像。可选 `--with-demo-verification` 另构建原生示例并运行原有渲染、异步、图片占位和文件往返四项检查。

## 当前证据

Windows `document-find-windows-v1117` 与 Linux 导出 `document-find-linux-v1118` 均完成 Rich/Source/Visual/Live 四模式，各383项检查、20张查找图像；两端原示例各四项原生检查也通过，另各6张图像。Windows使用仅含系统目录的执行PATH；Linux复用此前正常构建的库，依赖路径和库/配置哈希一致。

更新示例后的文件/关闭流程已复验：Windows `document-find-windows-workflows-v1120` 的18条命令、Linux `document-find-linux-workflows-v1119` 的23条命令全部实际退出0；两端各15场景/2,682检查，Linux原生X11输入198检查、Windows消息队列关闭/Unicode30检查通过。没有把这些增量记录称为未修改内核的全套重新验收。

源码和公开API的Document、renderer、editor与WebView后端保持不变，只有示例入口、永久验证注册和运行包改动。HTML/CSS转换、动态资源、完整UIA等高级范围继续冻结。真实IME/物理键鼠、Windows剪贴板成功互通、物理GPU与完整平台发布仍待验收。

最终记录为 `build/source-document-footnote-local-reference-20261006/document-v1-v1125-native-find-acceptance.json`。当前运行包可选包含独立 MessageList 示例，见[运行包说明](../../guide/XUI_DOCUMENT_RUN_PACKAGE.md)；旧接受包保持原字节。

更新后的 `document-find-package-v1123/runnable.zip` 包含三份启动脚本。普通与共享启用网页运行库各6条包内检查、仓库外中文/空格目录6条检查与3份启动脚本均实际退出0。打包器按应用角色分别复制DLL；启用网页的Document运行库加载器也在Document目录携带。v1121的DLL遗漏和v1122的加载器遗漏记录保留；修复后采用新目录复验。程序与库在所有验证期间保持冻结，之后仅补充文档证据。
