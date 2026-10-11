# Document 原生窗口验收入口

`build_document_native_suite.py` 顺序运行系统 GL 头文件/真实窗口、1/10 MiB 三模式编辑、复杂文字光标与首帧稳定性三个入口。它使用现有构建产物，记录输入、运行库和配置散列，保留各子入口报告、日志和实际输出图像。每次必须选择一个新输出目录，避免覆盖既有结果。

Windows 示例，在仓库根目录执行：

```powershell
python tools/build_profile.py default build/native-document
python test_xui/build_document_native_suite.py --build-dir build/native-document --out-dir build/document-native-acceptance
```

Linux 示例，在仓库根目录执行；安装系统编译器、项目构建依赖，以及 Xvfb 和 Mesa 后：

```sh
python3 tools/build_profile.py default build/native-document
python3 test_xui/build_document_native_suite.py --build-dir build/native-document --out-dir build/document-native-acceptance --headless
```

有实际显示服务器时可以省略 `--headless`。不同配置使用不同构建和输出目录；验证 `full-dev` 时将第一个命令的 profile 改为 `full-dev`，第二个命令引用对应产物。可通过 `--font` 指定测试字体。默认 Windows 使用 Arial、Linux 使用 DejaVu Sans；窗口测试还需要项目约定的 regular/bold/italic 测试字体。

入口共执行八条产品编译/运行检查，产生十六张原生图像。窗口测试覆盖 Rich/Markdown、嵌套表格、三模式、几何、编辑、共同 Undo/Redo 与保存重载。规模测试覆盖 1/10 MiB 的 SOURCE/VISUAL/LIVE 输入、发布和共同历史，不冻结异步 worker。光标测试断言真实 widget 的640×400内容视口，每端24次光标可见和12次发布/Redo首帧稳定性检查，并核对完整源码和共同历史。

`report.json` 汇总子入口实际退出码和报告散列。只有全部子入口产品检查通过、输入未改变、十六张图像齐备时才记为 `passed`。图像存在不等于视觉验收；需要实际查看或证明与已查看图像的字节一致。

该入口补充 `build_document_suite.bat` 和 Core/Renderer/View sanitizer 入口，不能替代它们。Linux `--headless` 结果属于 Xvfb/Mesa 软件后端，不证明硬件、物理 DPI、真实输入法、读屏或固定机器性能门槛；这些仍需单独完成系统验收。
