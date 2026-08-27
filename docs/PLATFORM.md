# 平台说明

## Windows

Windows / MinGW-w64 是仓库提供完整构建脚本的主要验证入口。按 [构建与验证](BUILD.md) 生成 DLL、导入库和测试程序。

## Android

`platform/android/` 提供 NativeActivity、CMake 与 Gradle 接入文件。可用根目录 `build_android_ndk.bat` 构建 NDK 库，或使用 `build_android_apk.bat`、`run_android_apk.bat` 构建并安装调试 APK。需要已配置 Android NDK、SDK、Gradle 和设备/模拟器。

## iOS

`platform/ios/` 提供 iOS Sokol 接入文件。macOS 上使用 `build_ios_sim_exe.sh` 构建模拟器应用，使用 `run_ios_sim_exe.sh` 安装并启动。需要 Xcode 命令行工具和可用模拟器。

## Web

`platform/web/` 提供 Emscripten/WebGL2 的 HTML shell。通过相应 Web 范例的构建脚本生成产物，再运行 `serve_web_exe.bat` 或 `serve_web_exe.sh` 以本地 HTTP 服务打开。

## 发布前要求

除 Windows 本机构建外，任一目标平台都应在实际目标环境完成构建、启动和关键输入/渲染路径验证后再交付。平台 scaffold 是接入起点，不替代目标环境验证。
