# 平台与图形兼容性

[返回文档中心](README.md)

## 图形 API 基线

XGE 第一阶段基线：

| 目标 | 图形 API |
| --- | --- |
| Windows | OpenGL 3.3 |
| Linux 桌面 | OpenGL 3.3 或 GLES3/EGL |
| macOS | OpenGL，低优先级 |
| Android | OpenGL ES 3.0 |
| iOS | OpenGL ES，低优先级 |
| Web | WebGL2 |
| 小程序 | WebGL2/GLES3 风格桥接 |
| 板卡 Linux | GLES3/EGL |
| 离屏 | EGL + OpenGL/GLES |

## 平台后端

| 目标 | 计划后端 | 状态表述 |
| --- | --- | --- |
| Windows | Sokol | 可本地开发验证 |
| Linux X11/Wayland | Sokol | 待目标环境验证 |
| Linux EGL | EGL | 待目标环境验证 |
| Android | Sokol/NativeActivity/APK | 脚手架与实机分开 |
| iOS/macOS | Sokol/OpenGL | 低优先级兼容路径 |
| Web | Emscripten/WebGL2 | 待浏览器验证 |
| 小程序 | 自定义宿主桥接 | 核心目标，需宿主验证 |
| 板卡 Linux | EGL/GBM/KMS | 面向近年 GLES3 板卡 |
| 离屏 | EGL pbuffer/surfaceless | 工具和测试目标 |

## Shader 兼容

业务 shader 不应写死桌面 OpenGL 版本。使用 `xgeGraphicsShaderHeaderGet` 和 `xgeGraphicsMappingGet` 处理后端差异。

WebGL2 和 GLES3 对 precision、varying/attribute、sampler 和扩展支持更严格。

## 文件和资源

桌面可以使用 exe 相对路径。Web、小程序、Android APK 和 iOS bundle 需要通过打包、assets 或 resource provider 接入。

`res://` 是跨宿主资源协议，不等价于真实文件路径。

## 输入兼容

键盘、鼠标、触控、手柄、文本输入能力由平台后端决定。不是所有平台都有完整输入能力。

文本输入和 IME 特别依赖平台后端。

## 音频兼容

Web 和移动端可能要求用户手势后才能播放音频。小程序音频通常需要宿主桥接。

## 当前验证规则

未实机验证的平台不得标记为已支持。验证结果统一记录在：

```text
dev/docs/平台后端验证结果.md
```
