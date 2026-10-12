# XGE 文档

这里是 XGE 2.0 的文档入口。函数签名、结构体字段、返回值和编译开关以 [`../include/xge.h`](../include/xge.h) 与 [`../include/xui.h`](../include/xui.h) 为准。

## 使用文档

- [构建与验证](guide/BUILD.md)
- [可裁剪构建与增量编译](guide/BUILD_PROFILES.md)
- [XGE 使用指南](guide/XGE.md)
- [3D C API](guide/3D.md)
- [低成本纹理存储优化](guide/TEXTURE_STORAGE.md)
- [2D 粒子系统](guide/PARTICLES.md)
- [XUI 使用指南](guide/XUI.md)
- [XUI Terminal 使用指南](guide/TERMINAL.md)
- [范例索引](guide/EXAMPLES.md)
- [平台说明](guide/PLATFORM.md)
- [Emoji 支持](guide/EMOJI.md)

## 阅读顺序

1. 第一次使用：从 [构建与验证](guide/BUILD.md) 构建 DLL 并运行基础测试。
2. 编写图形或应用程序：阅读 [XGE 使用指南](guide/XGE.md)，然后从 `examples/xge/` 选择相近范例。
3. 构建界面：阅读 [XUI 使用指南](guide/XUI.md)，再从 `examples/xui/` 选择控件范例。
4. 接入 Android、iOS 或 Web：先确认 [平台说明](guide/PLATFORM.md) 中的适用范围，再在目标环境完成验证。

## 文档约定

- 公共对象只通过头文件声明的创建、查询和释放 API 操作。
- `examples/` 中的代码可作为最小集成参考；测试文件用于回归验证，不是用户 API 教程。
- `guide/` 是使用文档，`reference/` 是样式和接口参考。
- `design/`、`audits/`、`validation/`、`plans/` 保存开发设计、审计、验证和历史计划，不作为公开 API 契约。
