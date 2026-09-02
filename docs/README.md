# XGE 文档

这里是 XGE 2.0 的发布文档入口。文档只描述当前可用的公开能力；函数签名、结构体字段、返回值和编译开关以 [`../xge.h`](../xge.h) 与 [`../xui.h`](../xui.h) 为准。

## 使用文档

- [构建与验证](BUILD.md)
- [XGE 使用指南](XGE.md)
- [XUI 使用指南](XUI.md)
- [XUI Terminal 使用指南](TERMINAL.md)
- [范例索引](EXAMPLES.md)
- [平台说明](PLATFORM.md)
- [Emoji 支持](EMOJI.md)

## 阅读顺序

1. 第一次使用：从 [构建与验证](BUILD.md) 构建 DLL 并运行基础测试。
2. 编写图形或应用程序：阅读 [XGE 使用指南](XGE.md)，然后从 `examples/xge_*` 选择相近范例。
3. 构建界面：阅读 [XUI 使用指南](XUI.md)，再从 `examples/xui_*` 选择控件范例。
4. 接入 Android、iOS 或 Web：先确认 [平台说明](PLATFORM.md) 中的适用范围，再在目标环境完成验证。

## 文档约定

- 公共对象只通过头文件声明的创建、查询和释放 API 操作。
- `examples/` 中的代码可作为最小集成参考；测试文件用于回归验证，不是用户 API 教程。
- 仓库不再把设计草案、开发计划、审计记录和历史实现作为发布文档维护。
