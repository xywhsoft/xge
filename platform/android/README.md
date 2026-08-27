# XGE Android

此目录提供 XGE 的 Android NativeActivity 接入文件。它通过 Sokol Android 入口构建 `libxge_android.so`，使用 OpenGL ES 3；当前 Android 接入以 `XGE_NO_AUDIO` 构建。

完整的发布前验证要求见根目录 [平台说明](../../docs/PLATFORM.md)。

## 构建

使用 Android NDK CMake：

```sh
cmake -S platform/android -B build/android \
	-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
	-DANDROID_ABI=arm64-v8a \
	-DANDROID_PLATFORM=android-18
cmake --build build/android
```

Windows 可运行 `build_android_ndk.bat`。要构建调试 APK，运行 `build_android_apk.bat`；再用 `run_android_apk.bat` 安装并启动。需要已配置 Android NDK、SDK、Gradle 和设备或模拟器。

## 应用入口

`xge_android_main.c` 提供 `sokol_main`。应用应以自身的初始化与帧回调替换或扩展该入口，并在目标设备或模拟器上完成启动、绘制和输入验证。
