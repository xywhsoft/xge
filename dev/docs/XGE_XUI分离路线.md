# XGE / XUI 分离路线

本文档固化 XUI 从 XGE 内部孵化到独立仓库的路线。当前阶段以开发效率和可测试性优先，成熟后再拆分。

> 2026-05-07 口径更新：XUI 剥离前必须完成 Widget V2 基础层，并把旧基础层上的控件按新口径重新验收。不得把 clip、Z、事件、焦点、滚动、IME 等仍靠局部补丁维持的状态带入独立仓库。

## 当前决策

XUI 长期是独立 DUI/RMGUI 项目，不并入 XGE 内核。

当前先在 XGE 仓库内实现 XUI 的布局、控件、事件和绘制桥接，原因：

- 可以直接复用 XGE 的输入、字体、纹理、shape、text、clip、dirty rect 和 APP 模式。
- 可以用 XGE 示例和测试快速验证 XUI。
- 避免早期在两个不稳定仓库之间设计过厚抽象层。
- 方便性能调优，确认 bridge 不能成为瓶颈。

## 模块边界

XGE 负责：

- 平台后端和窗口。
- 输入事件采集。
- 时间和主循环。
- GPU 资源和绘制 API。
- 字体、纹理、RenderTarget。
- APP 模式刷新和脏区提交。

XUI 负责：

- retained widget tree。
- 布局系统。
- 控件状态。
- 焦点、捕获、事件消费。
- 文本编辑核心和 IME 候选框位置。
- dirty layout / dirty paint。
- 通过 host 绘制。

Bridge 负责：

- 将 `xge_event_t` 输入交给 XUI。
- 将 XUI 绘制调用映射到 XGE draw/shape/text/clip。
- 将 XUI refresh 请求映射到 XGE APP 模式刷新。
- 保持薄层，不做复杂虚拟化。

## 孵化阶段

目标：

- 在 XGE 内完成基础布局和控件。
- 建立 XUI case 文档和示例。
- 建立人工验证流程。
- 确认控件树、布局、输入、paint API 稳定。

允许：

- XUI 使用 XGE 公开类型，例如 `xge_rect_t`、`xge_vec2_t`、`xge_texture`、`xge_font`。
- XUI 通过 XGE host 绘制。
- XUI 示例放在 XGE 仓库内。

禁止：

- XUI 直接访问 XGE 私有渲染队列。
- XUI 依赖 Sokol、GL 或平台后端私有状态。
- XUI 在控件逻辑里直接加载资源。

## 剥离阶段

剥离到 XUI 仓库前需要满足：

- Widget V2 基础层完成：box model、clip、PaintContext、layer/zIndex/treeOrder、event route、focus、tab order、pointer capture、IME、ScrollViewBase、VirtualScrollViewBase。
- 布局、控件、文本输入、paint API 完成 MVP。
- 现有控件在 Widget V2 上重新验收，重点覆盖文字不溢出、裁剪正确、Z 序正确、事件目标和上浮/下沉正确、焦点和 IME 策略正确。
- XGE 内部 XUI 示例通过人工验证。
- `xge_xui_host_t` 或其后继接口足够稳定。
- XUI 对 XGE 私有符号依赖为 0。
- XUI 有独立构建脚本和基础测试。

剥离步骤：

1. 将 XUI 纯逻辑代码迁移到 XUI 仓库。
2. 保留 XGE 侧 bridge 适配层。
3. 在 XGE 中 vendoring 或引用 XUI 单头版本。
4. 运行 XGE 中积累的 XUI 回归测试。
5. 将正式文档拆成 XGE bridge 文档和 XUI 独立文档。

## 性能原则

Bridge 必须薄。

控件绘制应尽量批处理，不因抽象层导致每个控件都强制 flush。

布局不每帧全树重算，dirty layout 只传播必要分支。

大型 ListView 不为每行创建复杂控件树，优先虚拟化或轻量行绘制。

## 文档同步

正式使用者文档：

- XGE 文档只描述 XGE 如何接入 XUI。
- XUI 独立文档描述布局、控件和 DUI 设计。

维护者文档：

- 当前孵化实现细节放在 `dev/docs/`。
- 剥离路线变化时先更新本文档，再改代码。
