# 从零开始写第一个 XGE 程序

> 状态：中文初稿已生成，待审阅。

这篇教程带你写一个最小 XGE 程序。目标不是一次学完所有能力，而是先把最重要的调用顺序跑通：初始化引擎、创建窗口、进入循环、绘制基础图形、退出并释放资源。

## 你要先理解的 4 个对象

| 对象 | 你可以把它理解成 | 生命周期 |
| --- | --- | --- |
| `xge_desc_t` | 引擎启动配置 | 初始化前填写，调用 `xgeInit` 后可丢弃 |
| XGE runtime | XGE 的全局运行环境 | `xgeInit` 创建，`xgeUnit` 销毁 |
| frame callback | 每帧被调用的用户函数 | 通过 `xgeRun` 传入 |
| render command queue | 绘制命令队列 | `xgeDraw`、shape、text 等 API 提交，帧末 flush |

最小调用顺序是：

```text
xgeInit
  -> xgeRun
    -> frame callback
      -> xgeClear
      -> xgeShapeRectFill / xgeDraw / xgeTextDraw
  -> xgeUnit
```

## 第一步：写 frame callback

frame callback 返回 `0` 表示继续运行，返回非 `0` 表示请求退出。

```c
static int MainFrame(void* pUser)
{
	xge_rect_t rect;

	(void)pUser;
	xgeClear(XGE_COLOR_RGBA(24, 32, 48, 255));

	rect.fX = 40.0f;
	rect.fY = 40.0f;
	rect.fW = 240.0f;
	rect.fH = 120.0f;
	xgeShapeRectFill(rect, XGE_COLOR_RGBA(64, 128, 220, 255));

	xgeShapeCircleFill(360.0f, 100.0f, 48.0f, XGE_COLOR_RGBA(240, 96, 72, 255));
	return 0;
}
```

## 第二步：初始化 XGE

```c
int main(int argc, char** argv)
{
	xge_desc_t desc;

	(void)argc;
	(void)argv;
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = 800;
	desc.iHeight = 600;
	desc.sTitle = "Hello XGE";
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iTargetFPS = 60;

	if ( xgeInit(&desc) != XGE_OK ) {
		return 1;
	}
	xgeRun(MainFrame, NULL);
	xgeUnit();
	return 0;
}
```

## 完整代码

```c
#include "xge.h"

#include <string.h>

static int MainFrame(void* pUser)
{
	xge_rect_t rect;

	(void)pUser;
	xgeClear(XGE_COLOR_RGBA(24, 32, 48, 255));

	rect.fX = 40.0f;
	rect.fY = 40.0f;
	rect.fW = 240.0f;
	rect.fH = 120.0f;
	xgeShapeRectFill(rect, XGE_COLOR_RGBA(64, 128, 220, 255));
	xgeShapeCircleFill(360.0f, 100.0f, 48.0f, XGE_COLOR_RGBA(240, 96, 72, 255));
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t desc;

	(void)argc;
	(void)argv;
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = 800;
	desc.iHeight = 600;
	desc.sTitle = "Hello XGE";
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iTargetFPS = 60;

	if ( xgeInit(&desc) != XGE_OK ) {
		return 1;
	}
	xgeRun(MainFrame, NULL);
	xgeUnit();
	return 0;
}
```

## 编译

仓库已经提供最小示例：

```bat
examples\mvp\build.bat
```

如果你要运行平台后端冒烟示例：

```bat
check_platform_smoke.bat
```

## 你刚刚完成了什么

1. `xgeInit` 创建 XGE runtime。
2. `xgeRun` 进入默认 game loop。
3. frame callback 每帧提交绘制命令。
4. `xgeClear` 清空当前帧。
5. shape API 绘制矩形和圆形。
6. `xgeUnit` 释放引擎资源。

## 常见问题

### 为什么没有手动调用 `xgeBegin` / `xgeEnd`

默认游戏循环会在内部管理每帧提交和 flush。只有在 APP/manual 模式或你需要显式批量刷新时，才需要主动使用 `xgeBegin` / `xgeEnd`。

### 为什么示例只画 shape

shape 不依赖外部资源，适合作为第一个程序。纹理、字体、音频等能力见后续教程。

### 什么时候使用 `XGE_RUN_MANUAL`

当 XGE 被嵌入 GUI 工具、编辑器或宿主 APP 中，且不希望引擎自己控制主循环时，使用 `XGE_RUN_MANUAL`。

## 下一步

- 继续阅读 [运行模式入门](run-mode-intro.md)。
- 想查函数细节时看 [Core API](../api/core.md) 和 [Drawing API](../api/drawing.md)。

[返回教程入口](README.md)
