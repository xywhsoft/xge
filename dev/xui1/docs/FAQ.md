# XGE 常见问题

[返回文档中心](README.md)

## XGE 是 C 还是 C++ 引擎？

XGE 公开接口只提供 C API。类型命名使用 `xge_texture_t` 这类风格，函数使用 `xge` 前缀。

## XGE 是否兼容 XGE 1.x？

不做 API 兼容。XGE V2 只继承功能目标，不复刻 DirectDraw 时代接口和 bitmap blend 等旧实现。

## 为什么不是 GLES2？

为了换取更完整的现代 2D 能力，XGE 当前基线是 GLES3 / WebGL2 / OpenGL 3.3 风格。现代手机和近年板卡通常可以接受这个基线。

## 是否只使用 OpenGL Core Profile？

API 风格采用现代 Core Profile 思路，但不会把 Core Profile 桌面 API 绑定成唯一后端。后端需要覆盖 OpenGL 3.3、GLES3、WebGL2、小程序和离屏路径。

## XGE 和 XUI 是一个项目吗？

当前 XUI 先在 XGE 内孵化，方便开发和测试。长期 XUI 会成为可独立工作的 DUI 项目，XGE 是默认接入框架。

## 为什么布局系统放在 XUI 而不是 XGE？

游戏底层通常不需要完整布局系统。GUI、工具和 APP 模式才需要布局，因此布局属于 XUI。

## 是否支持小程序？

小程序是第一版核心目标之一。正式支持状态取决于桥接层和目标平台实测结果。

## 是否支持 Metal？

第一阶段不实现 Metal。iOS/macOS 是低优先级兼容路径，暂时走 OpenGL/OpenGL ES，并保留后端扩展空间。

## 是否内建资源缓存？

第一版不内建全局资源缓存。XGE 提供资源加载、provider、fallback 和引用计数，上层项目负责缓存策略。

## 为什么默认 premultiplied alpha？

它更适合现代 2D 渲染，能减少透明边缘黑边，并让字体、UI、粒子混合更稳定。

## 是否支持多线程渲染？

支持专用 render thread 设计。拥有 GL context 的线程执行 GPU 命令，其他线程提交请求或做 IO/解码。

## 文档什么时候有英文版？

中文文档先人工审阅。稳定后再生成同名 `.en.md` 和根目录 `README.en.md`。
