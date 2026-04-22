# 运行模式入门

> 状态：中文初稿已生成，待审阅。

XGE 有两种主要运行方式：默认游戏循环和手动刷新。游戏通常使用默认循环；工具、编辑器、GUI APP、小程序宿主和某些离屏任务更适合手动模式。

## 你要先理解的 3 个动作

| 动作 | API | 说明 |
| --- | --- | --- |
| 推进运行时 | `xgeFrame` | 轮询平台事件、更新输入、推进 scene/update。 |
| 提交渲染 | `xgeRender` | 执行一帧渲染。 |
| 显式批量提交 | `xgeBegin` / `xgeEnd` / `xgeFlush` | 手动控制绘制命令的开始、结束和刷新。 |

默认 `xgeRun` 会在内部完成这些动作。手动模式下，宿主需要自己决定什么时候调用。

## 默认游戏循环

适合普通 2D 游戏。XGE 控制主循环，每帧调用你的 frame callback。

```c
static int MainFrame(void* pUser)
{
	(void)pUser;
	xgeClear(xgeColorRGBA(20, 24, 32, 255));
	return 0;
}

int main(void)
{
	xge_desc_t desc;
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = 800;
	desc.iHeight = 600;
	desc.sTitle = "XGE Game";
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;

	if ( xgeInit(&desc) != XGE_OK ) {
		return 1;
	}
	xgeRun(MainFrame, NULL);
	xgeUnit();
	return 0;
}
```

调用顺序：

```text
xgeInit
  -> xgeRun
    -> MainFrame
  -> xgeUnit
```

## 手动刷新模式

适合宿主已有自己的循环，例如编辑器、APP、工具窗口、小程序或离屏任务。

```c
desc.iRunMode = XGE_RUN_MANUAL;
if ( xgeInit(&desc) != XGE_OK ) {
	return 1;
}

while ( host_is_running() ) {
	xgeFrame();
	xgeRender();
}

xgeUnit();
```

手动模式下，XGE 不接管主循环。宿主可以在窗口 resize、UI dirty、定时器或外部事件触发时决定是否渲染。

## APP 模式和局部刷新

APP/GUI 工具不一定每帧都要重绘。可以把需要刷新的区域标记为 dirty，再集中刷新。

```c
xge_rect_t dirty;
dirty.fX = 0.0f;
dirty.fY = 0.0f;
dirty.fW = 320.0f;
dirty.fH = 180.0f;

xgeInvalidateRect(dirty);
xgeBegin();
xgeClear(xgeColorRGBA(0, 0, 0, 255));
/* draw UI */
xgeEnd();
xgeFlush();
xgeDirtyRectClear();
```

这套模式是 XUI/RMGUI 的基础：控件状态变化后请求刷新，宿主只在必要时提交绘制。

## 什么时候显式 begin/end

普通游戏里通常不需要手动调用 `xgeBegin` 和 `xgeEnd`。以下情况才考虑显式控制：

- 宿主已经有自己的主循环。
- 只想在 UI dirty 时刷新。
- 需要把多次绘制命令作为一个批次提交。
- 离屏渲染或工具导出需要确定提交边界。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 手动模式下窗口不刷新 | 只调用 `xgeFrame`，没有调用 `xgeRender` 或 `xgeFlush` | 手动模式需要宿主显式推进渲染。 |
| 游戏循环里重复 begin/end | 默认循环已经管理帧边界 | 普通 frame callback 里直接提交绘制命令即可。 |
| APP 模式 CPU 占用高 | 仍然每帧全量刷新 | 使用 dirty rect 和宿主事件驱动刷新。 |

## 下一步

- 想画纹理，继续看 [资源与纹理入门](texture-intro.md)。
- 想查运行时 API，看 [Core API](../api/core.md) 和 [Graphics API](../api/graphics.md)。

[返回教程入口](README.md)
