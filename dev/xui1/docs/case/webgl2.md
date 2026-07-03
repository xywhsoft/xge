# WebGL2 范例

本案例说明 WebGL2/Emscripten 目标的验证重点。

[返回范例解析](README.md) | [Web 教程](../guide/web-intro.md) | [Platform API](../api/platform.md)

## 状态

WebGL2 是计划基线。具体浏览器、Emscripten 版本和资源打包方式需要在实现中持续验证。

## 目标

```text
Emscripten
  -> canvas
  -> WebGL2 context
  -> XGE runtime
```

## 验证重点

- canvas 创建和 resize。
- WebGL2 context。
- shader header 和 precision。
- 虚拟文件系统或资源 provider。
- 鼠标、键盘、触控。
- 用户手势后的音频播放。

## 常见失败原因

写了阻塞式主循环。

资源没有被打包进虚拟文件系统。

浏览器阻止自动播放音频。

使用了 WebGL2 不支持的桌面 GL 语法。
