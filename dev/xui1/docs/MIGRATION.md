# 从 XGE V1 迁移到 XGE V2

[返回文档中心](README.md)

## 迁移原则

XGE V2 不做 1:1 API 兼容。迁移应按功能重新映射，而不是机械替换函数名。

V1 来自 DirectDraw 和大量 CPU bitmap 操作时代；V2 以 GPU 加速、现代渲染管线、异步资源、音频、字体和 GUI bridge 为核心。

## 废弃内容

| V1/旧思路 | V2 策略 |
| --- | --- |
| GDI 绘制 | 废弃。 |
| DirectDraw surface | 使用 texture、render target、image。 |
| bitmap blend | 使用 GPU blend、shader、material。 |
| IOCP 网络 | 由 xrt 提供外围能力。 |
| xpack 内建内核 | 作为可选 resource provider。 |
| V1 消息回调模型 | 使用 lifecycle、scene、input event。 |
| 旧函数名 | 不保留兼容 wrapper。 |

## 常见映射

| 旧需求 | V2 模块 |
| --- | --- |
| 打开窗口 | `xgeInit` / platform backend |
| 主循环 | `xgeRun` 或手动 `xgeFrame` / `xgeRender` |
| 绘制图片 | `xgeTextureLoad` + `xgeDraw` / `xgeDrawEx` |
| CPU 图像访问 | `xgeImage*`，必要时 `xgeTextureReadPixels` |
| 离屏画布 | `xgeRenderTarget*` / `xgeOffscreen*` |
| 字体 | `xgeFont*` / `xgeText*` / XRF |
| 声音 | `xgeSound*` / `xgeMusic*` / `xgeStream*` |
| 场景切换 | `xgeSceneSet` / `xgeScenePush` / `xgeScenePop` |
| GUI | XUI bridge |

## Bitmap 操作迁移

优先迁移为：

- GPU texture 更新：`xgeTextureUpdateRGBA`
- RenderTarget 合成：`xgePassBegin` / `xgePassEnd`
- Shader/Material 效果：`xgeMaterialDraw`

只有确实需要 CPU 访问时，才使用 image pixels 或 readback。

## 坐标迁移

V2 默认 y 轴向下，保留 2D 游戏和 GUI 的直觉。需要中心坐标或 2.5D 时，通过 camera 修改。

## 建议迁移顺序

1. 跑通最小窗口和清屏。
2. 替换图片加载和 sprite 绘制。
3. 替换输入系统。
4. 替换音频。
5. 用 scene stack 重建流程。
6. 把 GUI 迁移到 XUI。
7. 用 RenderTarget/shader 替代旧 bitmap 特效。
