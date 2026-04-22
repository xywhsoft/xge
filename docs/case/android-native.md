# Android NativeActivity

本案例说明 Android NativeActivity 方式的接入路径。

[返回范例解析](README.md) | [Android 教程](../guide/android-intro.md) | [Platform API](../api/platform.md)

## 状态

该页面描述 NativeActivity 脚手架策略，具体设备和系统版本需要人工实机验证。

## 目标

```text
NativeActivity
  -> Android app glue / Sokol
  -> GLES3
  -> XGE runtime
```

## 验证重点

- Activity 创建和销毁。
- Surface 创建、销毁和重建。
- GLES3 context。
- 触控输入。
- pause/resume。
- 音频是否需用户手势或权限。

## 生命周期建议

进入 pause 时暂停 update 和音频；surface 重建后恢复 GPU 资源或重新上传。

## 常见失败原因

在 surface 尚未创建时初始化 GL。

后台线程直接调用 GL。

APK asset 路径没有通过 resource provider 接入。
