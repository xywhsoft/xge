# XGE Android NativeActivity Scaffold

This folder contains the first Android/Sokol integration scaffold.

## Scope

- Uses Sokol's Android entry path. Android does not support `SOKOL_NO_ENTRY`.
- Builds XGE as a `NativeActivity` shared library.
- Uses OpenGL ES 3 through `SOKOL_GLES3`.
- Disables XGE audio in the initial scaffold with `XGE_NO_AUDIO`; audio device validation is a later target-device task.

## Build Outline

Use Android NDK CMake to validate the native shared library:

```sh
cmake -S platform/android -B build/android \
	-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
	-DANDROID_ABI=arm64-v8a \
	-DANDROID_PLATFORM=android-18
cmake --build build/android
```

Windows equivalent:

```bat
build_android_ndk.bat
```

The generated library name is `libxge_android.so`, matching `AndroidManifest.xml`.

To build a debug APK with the Gradle scaffold:

```sh
./build_android_apk.sh
```

Windows:

```bat
build_android_apk.bat
```

The APK path is `platform/android/gradle/app/build/outputs/apk/debug/app-debug.apk`.

Install and launch on a connected device or emulator:

```sh
./run_android_apk.sh
```

Windows:

```bat
run_android_apk.bat
```

## App Entry

`xge_android_main.c` provides `sokol_main`. It initializes XGE with a minimal scene callback. Real apps should replace that file or provide a project-specific variant.

## Validation

This scaffold is not marked as Android support complete until it is packaged into an APK and run on a device or emulator.
