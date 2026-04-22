# XUI 内部孵化

本案例说明为什么 XUI 先在 XGE 内实现，再剥离到独立仓库。

[返回范例解析](README.md) | [XUI 桥接教程](../guide/xui-bridge-intro.md) | [XUI API](../api/xui.md)

## 决策

XUI 长期应是独立 DUI 项目，但初期先在 XGE 内孵化。这样能直接复用 XGE 的输入、渲染、字体、资源和测试脚手架，减少早期“空中搭桥”的成本。

## 边界

```text
当前阶段：XGE 内部实现 XUI
成熟后：剥离 XUI 仓库
保留：XGE <-> XUI 高性能 bridge
```

## 剥离条件

- 布局、控件、输入、paint API 稳定。
- XGE 内回归测试覆盖常见 GUI 场景。
- host 接口足够清晰，不依赖 XGE 私有状态。
- 剥离后性能不明显下降。

## 常见风险

如果 XUI 直接读取 XGE 内部渲染状态，后续剥离会困难。应通过 host 和公开资源对象交互。

如果 bridge 过度抽象，会影响 GUI 性能。桥接层应薄而直接。
