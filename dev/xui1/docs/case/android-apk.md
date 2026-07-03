# Android APK 工程

本案例说明 Android APK 分发工程的文档化要求。

[返回范例解析](README.md) | [Android 教程](../guide/android-intro.md) | [平台冒烟](platform-smoke.md)

## 状态

APK 工程属于分发脚手架。文档只说明目标结构，实际 Gradle/NDK 工程以仓库实现为准。

## 推荐结构

```text
android/
  app/
    build.gradle
    src/main/
      AndroidManifest.xml
      cpp/
      assets/
```

## 需要打通

- C 入口和 XGE 初始化。
- assets 到 XGE resource provider。
- GLES3 context。
- 触控和文本输入。
- 音频初始化。
- 日志输出到 logcat。

## 常见失败原因

NDK ABI 与设备不匹配。

assets 没有进入 APK。

AndroidManifest 权限或 Activity 配置不完整。
