# Document 原生基础格式菜单

本批最终核对为 `document-v1-v1163-format-menu-acceptance.json`。新版 `document-format-menu-package-v1161/runnable.zip` 的普通/共享运行库两组各6项、仓库外解压6项及三个启动脚本均实际退出0。包内README保持运行验证时字节，本页随后补齐索引；内核运行库与此前ZIP保持原字节，高级范围继续冻结。

日期：2026-10-10。首版示例提供原生 Format 菜单，复用统一 DocumentEditor 的格式命令、能力查询、事务和撤销历史；没有新增文档类型、转换层或公开API。

先点击Rich或Markdown编辑区，再打开Format。包含Paragraph、Heading 1/2/3、Bullet/Numbered/Task list、Block quote、Code block、Underline、Strikethrough、Inline code、Clear formatting，共13项。菜单显示当前格式状态，并按模式、只读、选区和格式可表达能力禁用不支持项；Source/Live可切换Visual执行结构命令。Code block需放置光标，不在整段选区上插入。选择后或Escape关闭菜单恢复原编辑器焦点，Undo/Redo沿用原文档历史与共享预览。

```powershell
python test_xui/build_document_demo_find_test.py --out-dir D:\path\to\new-results --with-format --with-demo-verification
```

```sh
python3 test_xui/build_document_demo_find_test.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results --cjk-font /absolute/path/to/local-font.ttc --with-format --with-demo-verification
```

Windows基础入口和Linux `--with-native-find` 自动启用菜单检查；默认独立驱动保持原查找范围。当前双端四模式各3,624项、28图像通过：查找383项，格式菜单3,241项，覆盖13项命令、实际菜单行点击/键盘Enter、能力与勾选状态、禁用项无编辑、只读、撤销/重做和焦点/文档/预览保持。可用的代码块插入使用折叠光标额外验证。四组逻辑viewport检查菜单边界、owner、Escape焦点和revision；这是逻辑布局检查。实际OS窗口缩放由独立驱动完成，每端2,728项、32图像。两端原示例四项和文件/关闭流程（15场景、2,682项）复验通过；Windows原生Unicode/关闭30项，LinuxX11输入198项通过。

菜单缺失的旧示例负例 `document-format-menu-negative-v1153-corrected` 实际编译退出0，Rich运行因找不到Format按钮退出1；旧示例、测试、驱动、DLL和日志冻结。首次v1153编译错误属于测试中菜单API名称和缩进的错误，不作为产品缺陷。最终四模式入口为Windows `document-format-menu-windows-v1155-caret`、Linux导出 `document-format-menu-linux-v1156-caret`；先前v1155/v1156包含原示例四项验证，示例/运行库未变，后续仅补测试中的折叠光标代码块路径。

本批示例和永久验证源已改，内核、renderer、editor和WebView后端及DLL未改。未重跑未变化的全部基础/内核套件；完整基础证据沿用v1152。已直接查看Windows菜单与Linux禁用菜单图像。真实IME、物理键鼠/GPU、完整DPI和平台发布仍待验收；HTML/CSS转换、Document网络动态资源、完整UIA与高级矩阵继续冻结。
