# XGE Sokol 跨平台冒烟测试计划

本文档用于验证 Sokol 窗口后端在非 Windows 平台的构建和运行状态。未完成本文档对应平台的实机验证前，`XGE_V2_SPEC.md` 中对应平台项不得关闭。

实际执行结果必须记录到 `docs/平台后端验证结果.md`。未写入验证结果的口头通过不作为关闭平台任务的依据。

可运行 `check_platform_results.*` 汇总当前哪些平台结果已满足关闭条件。
可运行 `record_platform_result.*` 写入目标平台的实际执行结果。

## 通用验证项

每个平台至少验证以下内容：

- 能完成编译和链接。
- 能启动窗口或平台等价 surface。
- 能创建 OpenGL/OpenGL ES/WebGL context。
- `xgeDebugDumpCaps` 能输出平台后端、Sokol target、图形后端和 GL 信息。
- `xgePlatformRuntimeGet` 或 platform smoke 输出的 `Platform Runtime` 能报告 window/framebuffer/DPI。
- 对键盘、文本、鼠标、触摸、gamepad、resize 的实际操作会让对应 runtime 事件计数增加。
- `xgeRun` 或等价循环能持续运行 30 秒。
- `xgeQuit` 或窗口关闭能正常退出。
- keyboard/mouse/touch 中该平台可用的输入能进入 XGE event。
- 最小纹理绘制、shape 绘制、text 绘制至少各验证一次。

通用平台冒烟示例：

```sh
./examples/platform_smoke/build.sh
```

Windows:

```bat
examples\platform_smoke\build.bat
```

该示例启动后会调用 `xgeDebugDumpCaps` 输出平台能力，定期打印 `Platform Runtime` 运行态快照，并绘制基础 shape 与程序生成纹理。Web、macOS、Linux X11/EGL 的平台脚本默认使用该示例；Android/iOS scaffold 在入口帧内输出 caps 日志并绘制基础 shape。

该示例支持自动退出参数，便于目标机或 CI 采集可解析日志：

```sh
./build/xge_linux_x11 --frames 180 > smoke.log
./check_platform_smoke_log.sh smoke.log --require-final
```

也可以用环境变量：

```sh
XGE_SMOKE_SECONDS=30 ./build/xge_linux_x11 > smoke.log
```

Windows:

```bat
build\xge_platform_smoke.exe --frames 180 > smoke.log
check_platform_smoke_log.bat smoke.log --require-final
```

Windows 也可以直接执行完整包装脚本：

```bat
check_platform_smoke.bat
```

Linux/macOS 目标机优先执行对应包装脚本：

```sh
./check_linux_x11_smoke.sh
./check_linux_egl_smoke.sh
./check_macos_smoke.sh
```

在切换到新平台前可以先运行工具链检查脚本：

```sh
./check_platform_backends.sh
```

Windows:

```bat
check_platform_backends.bat
```

也可以运行批量准备检查脚本，它会依次执行 scaffold、工具链和结果状态检查：

```bat
check_platform_all.bat
```

Linux/macOS:

```sh
./check_platform_all.sh
```

也可以先运行 scaffold 完整性检查，确认平台入口文件和关键配置没有缺失：

```bat
check_platform_scaffold.bat
```

Linux/macOS:

```sh
./check_platform_scaffold.sh
```

## Linux X11

构建命令：

```sh
./examples/platform_smoke/build_linux_x11.sh
./build_test.sh
./build_dll.sh
./examples/mvp/build.sh
./examples/shape/build.sh
./examples/texture/build.sh
./examples/text/build.sh
./examples/input/build.sh
```

完整冒烟包装：

```sh
./check_linux_x11_smoke.sh
```

系统依赖：

- X11 开发包。
- Xi 开发包。
- Xcursor 开发包。
- OpenGL 开发包。
- pthread/dl/m。

通过标准：

- `xgePlatformCapsGet` 报告 `bSokolLinuxX11 = 1`。
- `sSokolTargetName` 为 `linux-x11`。
- `build/linux_x11` 或 `build/xge_linux_x11` 对应的 platform smoke 窗口可以打开和关闭。
- 键盘、鼠标、滚轮可用。

## Linux Wayland

当前 vendored 的 `lib/sokol/sokol_app.h` 未包含原生 Wayland 后端。Linux 桌面 Sokol 路径实际是 X11，可选择 GLX 或 EGL。

如果需要验证 Linux EGL/GLES3 路径，可以先使用 X11/EGL：

```sh
./examples/platform_smoke/build_linux_egl.sh
```

完整冒烟包装：

```sh
./check_linux_egl_smoke.sh
```

X11/EGL 系统依赖：

- X11 开发包。
- Xi 开发包。
- Xcursor 开发包。
- EGL/OpenGL 或 GLES 开发包。

原生 Wayland 后续需要升级 Sokol 或实现独立平台后端。

Wayland 关闭标准：

- 原生 Wayland session 下可打开窗口。
- `xgePlatformCapsGet` 报告对应 Wayland 能力，或明确报告 fallback。
- `sSokolTargetName` 为 `linux-wayland`。
- pointer/keyboard/text input 路径可用。

## macOS

macOS 为低优先级平台，第一版只验证 OpenGL 路径，不引入 Metal。

```sh
./examples/platform_smoke/build_macos.sh
```

完整冒烟包装：

```sh
./check_macos_smoke.sh
```

通过标准：

- `examples/platform_smoke/build_macos.sh` 能生成 `build/xge_macos`。
- `xgePlatformCapsGet` 报告 `bSokolMacOS = 1`。
- `sSokolTargetName` 为 `macos`。
- 窗口和 OpenGL context 可创建。
- 输入、纹理、文本基础示例可运行。

## Android

Android 需要 NDK 工程或 Gradle/NativeActivity 集成。

当前已提供 `platform/android` NativeActivity scaffold，可先验证 CMake shared library：

```sh
./build_android_ndk.sh
```

Windows:

```bat
build_android_ndk.bat
```

如果目标机器有 Android SDK/Gradle，也可以直接构建 debug APK：

```sh
./build_android_apk.sh
```

Windows:

```bat
build_android_apk.bat
```

安装并启动：

```sh
./run_android_apk.sh
```

Windows:

```bat
run_android_apk.bat
```

最低验证内容：

- NDK 构建通过。
- `libxge_android.so` 能被 NativeActivity 加载。
- `app-debug.apk` 能安装到设备或模拟器。
- GLES3 context 创建成功。
- touch event 可进入 XGE。
- asset/resource loading 可从 APK 或外部文件路径读取。
- audio 示例能在设备上播放。

通过标准：

- `xgePlatformCapsGet` 报告 `bSokolAndroid = 1`。
- `sSokolTargetName` 为 `android`。
- 真机或模拟器运行 30 秒无崩溃。

## iOS

iOS 为低优先级平台，第一版只验证 OpenGL ES 路径。

当前已提供 `platform/ios` Sokol scaffold 和 iOS simulator 构建脚本：

```sh
./build_ios_sim_exe.sh
```

安装并启动到已 boot 的 simulator：

```sh
./run_ios_sim_exe.sh
```

最低验证内容：

- Xcode 工具链可构建 `build/ios_sim/XGE.app`。
- `run_ios_sim_exe.sh` 能安装并启动 app。
- GLES3 context 创建成功。
- touch/text input 基础事件可用。

通过标准：

- `xgePlatformCapsGet` 报告 `bSokolIOS = 1`。
- `sSokolTargetName` 为 `ios`。
- 真机或模拟器运行 30 秒无崩溃。

## Web/Emscripten

Web 路径验证 WebGL2。该目标不同于小程序后端，小程序不走 Sokol。

当前已提供 `platform/web/shell.html`，构建脚本会将其作为 Emscripten shell：

```sh
./examples/platform_smoke/build_web.sh
```

Windows:

```bat
examples\platform_smoke\build_web.bat
```

生成后从本地 HTTP server 打开：

```sh
./serve_web_exe.sh
```

Windows:

```bat
serve_web_exe.bat
```

通过标准：

- `examples/platform_smoke/build_web.sh` 或 `examples\platform_smoke\build_web.bat` 能生成 `build/web/xge_web.html`。
- Emscripten 构建通过。
- 浏览器中 WebGL2 context 创建成功。
- `xgePlatformCapsGet` 报告 `bSokolWeb = 1`。
- `sSokolTargetName` 为 `web`。
- keyboard/mouse/touch 中浏览器可用事件能进入 XGE。

## spec 关闭规则

- Linux X11：需要 Linux X11 实机或虚拟机完成构建和窗口冒烟。
- Linux Wayland：当前 vendored Sokol 未提供原生 Wayland 后端；需要升级平台库或实现独立后端后，在 Wayland session 完成构建和窗口冒烟。当前 spec 中“能力检测”已完成不等于运行路径完成。
- macOS：需要 macOS 设备或 CI runner 完成构建和窗口冒烟。
- Android：需要 Android 工程/NDK 构建和真机或模拟器冒烟。
- iOS：需要 Xcode 工程和真机或模拟器冒烟。
- Web/Emscripten：需要 Emscripten 构建和浏览器 WebGL2 冒烟。
- 所有平台：必须在 `docs/平台后端验证结果.md` 中记录环境、脚本、观察结果和关键日志。
