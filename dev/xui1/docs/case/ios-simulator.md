# iOS Simulator

本案例说明 iOS Simulator 路径的限制和验证重点。

[返回范例解析](README.md) | [Apple 平台教程](../guide/ios-macos-intro.md) | [Platform API](../api/platform.md)

## 状态

iOS 是低优先级兼容目标。该页面只描述验证策略，不承诺当前已完整支持。

## 验证重点

- Sokol/iOS 后端是否能创建窗口和 GL ES context。
- Retina scale 下 framebuffer 尺寸是否正确。
- 触控事件是否进入 XGE。
- 前后台切换是否稳定。

## 注意

Simulator 与真机 GPU/驱动不同。Simulator 通过不等于真机通过。

## 常见失败原因

OpenGL ES 路径在目标 SDK 上受限。

Info.plist 或入口配置不完整。

触控坐标没有处理 scale。
