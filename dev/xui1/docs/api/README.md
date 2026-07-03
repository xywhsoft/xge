# XGE API 索引

> 当前正式 API 参考入口。这里按“你会怎样使用 XGE”组织，而不是按源码文件顺序堆放。

[返回文档中心](../README.md)

---

## 1. 基础类型与运行时

| 模块 | 文档 | 稳定性 | 说明 |
| --- | --- | --- | --- |
| Core | [core.md](core.md) | 稳定 | 初始化、退出、运行循环、时间、内存释放和调试统计 |
| Platform | [platform.md](platform.md) | 平台待验证 | 平台后端、图形后端、能力报告、runtime 快照和平台冒烟 |
| Scene | [scene.md](scene.md) | 稳定 | 场景栈、生命周期、事件派发和 update 策略 |
| Input | [input.md](input.md) | 平台待验证 | keyboard、mouse、touch、text input、gamepad 状态 |

## 2. 渲染与图形资源

| 模块 | 文档 | 稳定性 | 说明 |
| --- | --- | --- | --- |
| Graphics | [graphics.md](graphics.md) | 实验 | GL/GLES/WebGL 映射、shader header、blend、camera 和 viewport |
| Texture | [texture.md](texture.md) | 稳定 | image、texture、sampler、fallback、upload queue 和 readback |
| Drawing | [drawing.md](drawing.md) | 稳定 | sprite、shape、text draw、sprite batch 和 2.5D draw |
| Render Target | [render-target.md](render-target.md) | 实验 | RenderTarget、RenderPass、Buffer 和 readback |
| Shader / Material | [material.md](material.md) | 实验 | 自定义 shader、material、uniform 和 pipeline state |

## 3. 资源、异步、字体和音频

| 模块 | 文档 | 稳定性 | 说明 |
| --- | --- | --- | --- |
| Resource | [resource.md](resource.md) | 稳定 | 文件、内存、`res://`、custom provider 和 xpack provider |
| Async | [async.md](async.md) | 实验 | 异步 image/texture/font/sound 加载和取消 |
| Font / Text | [font-text.md](font-text.md) | 实验 | UTF-8、TTF、XRF、glyph、text measure 和 draw |
| Audio | [audio.md](audio.md) | 实验 | sound、music、stream、group、fade、3D position 和 listener |

## 4. 跨平台与桥接

| 模块 | 文档 | 稳定性 | 说明 |
| --- | --- | --- | --- |
| Mini Program | [miniprogram.md](miniprogram.md) | 平台待验证 | 小程序 WebGL2 Canvas 桥接、touch、resource 和 audio command |
| Offscreen / EGL | [offscreen.md](offscreen.md) | 平台待验证 | EGL pbuffer/surfaceless、板卡 Linux 和离屏读回 |
| XUI Bridge | [xui.md](xui.md) | 内部孵化 | XGE 内部孵化期 XUI host/backend、事件和 paint |

## 稳定性说明

| 标记 | 含义 |
| --- | --- |
| 稳定 | API 方向已经明确，后续只做兼容扩展。 |
| 实验 | 功能目标明确，但实现和细节可能随 MVP 调整。 |
| 内部孵化 | 当前用于 XGE 内部验证，未来可能迁移到独立项目。 |
| 平台待验证 | API 存在或脚手架存在，但需要目标平台实机验证。 |

## 推荐阅读顺序

1. [core.md](core.md)
2. [platform.md](platform.md)
3. [graphics.md](graphics.md)
4. [texture.md](texture.md)
5. [drawing.md](drawing.md)
6. [scene.md](scene.md)
7. [resource.md](resource.md)
8. [audio.md](audio.md)

## API 文档编写标准

每个 API 模块页必须覆盖分配给该模块的公开 `XGE_API` 函数，并且每个函数都要说明：

- **功能**：这个函数解决什么问题，什么时候使用。
- **函数原型**：从 `xge.h` 复制的准确 C 原型。
- **参数**：输入/输出方向、是否可为 `NULL`、生命周期、所有权、单位和范围。
- **返回值**：成功/失败语义和错误码。
- **资源归属**：谁分配、谁释放、用哪个 `Free/Unit/Close/Reset` 函数清理。
- **补充说明**：调用顺序、线程安全、后端差异和兼容性注意事项。
- **范例代码**：尽量给出可直接学习的小代码段。

新页面请从 [API_PAGE_TEMPLATE.md](API_PAGE_TEMPLATE.md) 开始填充。
