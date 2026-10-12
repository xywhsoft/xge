# Document 原生窗口验证

`build_document_native_window_test.py` 使用已有 profile 的实际动态库和
`xge_build_config.h`，不通过代理模拟排版或绘制。当前入口支持 Windows 与 Linux。

入口会检查系统 GL 与 XGE 私有 GL 头文件的两种包含顺序及指针 ABI，再运行
同一套公共 Document API：富文本/Markdown 加载、原生绘制与光标几何、编辑、
逐像素精确 Undo/Redo、保存重载，以及 Markdown 的视觉/源码/实时三模式。
每次生成十二帧检查、四张 PNG、完整日志与带输入散列的 `report.json`。
像素对照发生在同一运行环境内，不要求不同平台字体产生相同像素。

先使用 `tools/build/build.py` 构建启用 XUI Document、原生文字及 HarfBuzz 的
`default` 或 `full-dev` profile，再执行：

```powershell
python tests/xui/build_document_native_window_test.py --build-dir build --out-dir build/document/native-window-win
```

```sh
python3 tests/xui/build_document_native_window_test.py --build-dir build --out-dir build/document/native-window-linux --headless
```

Linux 默认使用系统 DejaVu 的常规、粗体、斜体和等宽字体；需安装
`fonts-dejavu-core`、`fonts-dejavu-extra`。Windows 默认使用系统 Arial 与 Consolas。
`--font-dir` 可指定包含四个 DejaVu 字体文件的目录。缺少字体会在运行前明确失败。
头文件检查需要平台 GL 开发头文件；Linux 无显示设备时使用 `--headless`，
需要 Xvfb/xauth，且明确选择 Mesa 软件 GL。物理显示设备可以省略此参数。

构建和输出路径均可为绝对路径；`--source-root` 用于验证独立源码副本。
输出目录必须尚不存在，防止覆盖已有失败证据。脚本只编译两个测试文件，
不会修改产品源码、已有构建配置或库。原生窗口检查不能证明物理 DPI、
真实输入法、读屏、硬件性能或交互 Editor 的全部行为；这些必须另外验收。

## Actual Editor scale and caret visibility

The companion `build_document_native_scale_test.py` measures a 10,000-paragraph
Rich transaction and a 100 KiB Markdown prepare/run/publish sequence. It then
uses the actual XGE Editor and running worker for 1/10 MiB Markdown input in
VISUAL, SOURCE and LIVE modes, with exact source checks and shared Undo/Redo.
All twelve publication/history caret checks query the renderer owned by the
actual widget and require the caret rectangle to remain inside its viewport.
Six PNGs and a report with input hashes are written. Timing includes GPU draw
completion; it is diagnostic and does not include OS key dispatch or display
presentation. The test uses an internal inspector to check geometry, not a
separately created renderer or a weakened public-input assertion.

```powershell
python tests/xui/build_document_native_scale_test.py --build-dir build --out-dir build/document/native-scale-win
```

```sh
python3 tests/xui/build_document_native_scale_test.py --build-dir build --out-dir build/document/native-scale-linux --headless
```

`--source-root`, absolute build/output paths and new output directory rules are
the same as the window entry. `--font` selects one font for the scale entry;
its defaults are Windows Arial and Linux DejaVu Sans. The worker is never
frozen. Fixed-machine release thresholds, physical IME, DPI and readers still
require separate acceptance.

The full existing Renderer and Editor tests remain part of validation. Their
working directory must contain `res/xui/assets/msgbox_info.png` and `build/document` for
image loading and save/reload cases. The Renderer image wait uses the shared
five-second deadline and thread yield on both platforms, so a tight fixed poll
count cannot finish before the decoder worker gets scheduled.
