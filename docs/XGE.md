# XGE 使用指南

XGE 负责窗口、帧循环、渲染、资源、文本、输入、音频和平台能力。应用从 `xge.h` 引入 API。

## 生命周期

1. 零初始化 `xge_desc_t`，设置窗口尺寸、标题、初始化标志和运行模式。
2. 调用 `xgeInit`。成功后才能创建依赖运行时的资源或窗口对象。
3. 游戏循环模式下调用 `xgeRun` 并提供帧回调；手动模式下由应用调用 `xgeFrame` 和 `xgeRender`。
4. 退出时先释放应用持有的资源，最后调用 `xgeUnit`。

典型窗口程序：

```c
#include "xge.h"
#include <string.h>

static int on_frame(void* user) {
    (void)user;
    xgeClear(0x20242cff);
    return XGE_OK;
}

int main(void) {
    xge_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.iWidth = 960;
    desc.iHeight = 540;
    desc.sTitle = "My XGE App";
    desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
    desc.iRunMode = XGE_RUN_GAME_LOOP;
    if (xgeInit(&desc) != XGE_OK) return 1;
    xgeRun(on_frame, NULL);
    xgeUnit();
    return 0;
}
```

返回 `XGE_OK` 表示成功；失败时检查 API 的返回值，并可通过 `xgeSetErrorCallback` 接收可恢复错误信息。

## 常用能力

独立 CPU 粒子模拟、资源配置与批绘制参见 [2D 粒子系统](PARTICLES.md)，实际集成参见 [七个粒子场景](../examples/xge_particles/README.md)。

| 需求 | 公开 API / 范例 |
| --- | --- |
| 绘制基础形状、精细路径和渐变 | `xgeShape*`、`xgeShapeEx*`；`examples/xge_shape*`。 |
| 图像、纹理与采样状态 | `xgeImage*`、`xgeTexture*`；`examples/xge_svg`、`examples/yuv420p_texture_lab`。 |
| 字体、文本和 Emoji | `xgeFont*`、`xgeText*`；[Emoji 支持](EMOJI.md)。 |
| 键盘、鼠标、触摸、IME 和拖放 | `xgeInput*`、`xgeDrag*`、`xgeDataObject*`。 |
| 场景与更新策略 | `xgeScene*`、`xgeSceneUpdateStrategy*`。 |
| 异步图像和资源加载 | `xgeAsync*`、`xgeResource*`。 |
| 离屏/EGL 渲染 | `xgeOffscreen*`、`xgeEGL*`。 |

## 资源和所有权

- 由 `Create`、`Load` 或 `Init` 取得的对象，使用对应 `Destroy`、`Free` 或 `Unit` API 释放。
- 查询函数返回的指针、字符串或平台句柄通常是借用值；除非头文件明确要求，不要释放它们，也不要跨运行时生命周期保存。
- 不访问不完整资源类型的字段。资源布局、缓存和后端对象均为内部实现，不能作为 ABI 依赖。
- 未指定线程安全的 API 应在创建 XGE 的线程调用。异步请求使用 `xgeAsyncPoll` 在调用线程提交完成结果。

## XGE 与 XUI

XUI 运行在 XGE 的窗口和渲染循环之上。先完成 XGE 初始化，再创建 XUI context；在帧回调中驱动 XUI 更新、布局和绘制。具体流程见 [XUI 使用指南](XUI.md) 与对应范例。
