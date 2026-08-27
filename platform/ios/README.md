# XGE iOS

此目录提供 XGE 的 iOS Sokol 接入文件。它通过 `sokol_main` 使用 OpenGL ES 3，并可构建为模拟器应用。完整的发布前验证要求见根目录 [平台说明](../../docs/PLATFORM.md)。

## 构建模拟器应用

在 macOS 且已安装 Xcode 命令行工具的环境中：

```sh
./build_ios_sim_exe.sh
```

可选环境变量：

```sh
IOS_SDK=iphonesimulator
IOS_ARCH=arm64
IOS_DEPLOYMENT_TARGET=12.0
```

生成的应用位于 `build/ios_sim/XGE.app`。使用 `run_ios_sim_exe.sh` 安装并启动；可通过 `IOS_SIMULATOR`、`IOS_APP_DIR` 与 `IOS_BUNDLE_ID` 覆盖默认值。交付前应在目标模拟器或设备上完成启动、绘制和输入验证。
