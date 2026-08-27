# 构建与验证

## Windows / MinGW-w64

需要 `gcc` 与 `windres` 在 `PATH` 中。所有脚本在仓库根目录执行，并把结果写入本地 `build/` 目录。

```bat
build_dll.bat
build_test.bat
build_dbg_dll.bat
build_dbg_test.bat
```

`build_dll.bat` 生成 `build/xge.dll` 与 `build/xge.lib`；该 DLL 同时包含 XGE 和 XUI。`build_dbg_dll.bat` 生成带调试 API 的 `xgedbg` 变体。

## 推荐验证集

在变更 XGE 核心后：

```bat
build_test.bat
build_dbg_test.bat
test\build_input_event_queue_test.bat
test\build_native_input_coordinates_test.bat
test\build_clipboard_win32_test.bat
test\build_zstd_resource_test.bat
```

在变更 XUI 后，至少执行：

```bat
test_xui\build_context_test.bat
test_xui\build_widget_test.bat
test_xui\build_layout_test.bat
test_xui\build_input_test.bat
test_xui\build_text_test.bat
```

每个 `test_xui/build_*.bat` 都可独立构建并运行一个 XUI 回归测试。需要观察实际界面时，运行相应 `examples/xui_*/build.bat`；范例默认持续运行，支持的 `--frames` 和 `--seconds` 参数见其命令行帮助。

## 源码集成

应用可链接本脚本生成的 DLL/导入库，也可把 `xge.c` 与 `xui_sources.bat` 列出的实现文件纳入自己的 C 构建。两种方式都应：

- 使用 `xge.h` 和（需要界面时）`xui.h`；
- 为 DLL 使用与构建脚本一致的 `XGE_DLL` / `XUI_DLL` 宏；
- 链接脚本列出的系统库；
- 不把 `src/` 中的私有实现文件当作稳定 API。

## 生成目录

`build/`、`artifacts/` 以及根目录下由测试临时写入的图像均可重新生成，已由 `.gitignore` 排除。发布源包不应包含这些目录。
