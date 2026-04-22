# 最小窗口和 shape 绘制

本案例展示一个最小的 XGE 窗口程序：创建 runtime，启动 Sokol 窗口后端，每帧清屏并绘制基础 shape。

[返回范例解析](README.md) | [第一个 XGE 程序](../guide/first-xge-program.md) | [Core API](../api/core.md)

## 问题

你希望用最少的 XGE API 打开一个窗口，并确认图形后端、主循环和基础绘制都能工作。

这个案例适合：

- 验证 XGE 编译和链接是否正确。
- 验证 Sokol 窗口后端是否能启动。
- 学习 `xgeInit`、`xgeRun`、frame callback 和 shape draw 的最小关系。
- 给纹理、文本、场景和平台冒烟打基础。

## 架构

最小调用链如下：

```text
应用程序
  -> xgeInit
    -> Sokol window/context
  -> xgeRun
    -> frame callback
      -> xgeClear
      -> xgeShapeRectFill
      -> xgeShapeCircleFill
  -> xgeUnit
```

## 步骤 1：准备启动配置

```c
xge_desc_t desc;

memset(&desc, 0, sizeof(desc));
desc.iWidth = 800;
desc.iHeight = 600;
desc.sTitle = "XGE MVP";
desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
desc.iRunMode = XGE_RUN_GAME_LOOP;
desc.iTargetFPS = 60;
```

`XGE_RUN_GAME_LOOP` 表示让 XGE 使用平台后端的主循环。桌面窗口、Android、iOS 和 Web/Sokol 路径通常使用这个模式。

## 步骤 2：编写 frame callback

```c
static int MainFrame(void* pUser)
{
	xge_rect_t rect;

	(void)pUser;
	xgeClear(XGE_COLOR_RGBA(24, 32, 48, 255));

	rect.fX = 24.0f;
	rect.fY = 24.0f;
	rect.fW = 180.0f;
	rect.fH = 96.0f;
	xgeShapeRectFill(rect, XGE_COLOR_RGBA(64, 128, 220, 255));
	xgeShapeCircleFill(256.0f, 72.0f, 48.0f, XGE_COLOR_RGBA(240, 96, 72, 255));
	return 0;
}
```

## 步骤 3：启动并退出

```c
if ( xgeInit(&desc) != XGE_OK ) {
	return 1;
}
xgeRun(MainFrame, NULL);
xgeUnit();
```

`xgeRun` 在默认 game loop 模式下会进入平台主循环，直到窗口关闭、调用 `xgeQuit`，或 frame callback 返回非 0。

## 完整示例

完整示例见：

```text
examples/mvp/main.c
```

构建：

```bat
build_exe.bat
```

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeInit` | 初始化 XGE runtime |
| `xgeRun` | 启动主循环或注册手动 frame callback |
| `xgeClear` | 清屏并提交当前帧 clear 状态 |
| `xgeShapeRectFill` | 绘制填充矩形 |
| `xgeShapeCircleFill` | 绘制填充圆 |
| `xgeQuit` | 请求退出 |
| `xgeUnit` | 释放 XGE runtime |

## 扩展点

你可以在这个案例上继续加入：

- `xgeTextureLoad` 和 `xgeDraw` 绘制图片。
- `xgeSceneSet` 管理多个场景。
- `xgeTextDraw` 显示文本。
- `xgeDebugDumpCaps` 输出平台和 GPU 能力。
- `xgePlatformRuntimeGet` 输出窗口、framebuffer 和 DPI 状态。

## 常见问题

如果窗口没有出现，先运行 `build_test.bat` 确认基础构建，再运行 `check_platform_smoke.bat` 查看平台能力日志。

如果窗口出现但画面为空，检查 `xgeDebugDumpCaps` 中 GL vendor、renderer、version 是否为空，以及平台是否真正创建了 GL context。

如果关闭窗口后进程没有退出，检查 frame callback 是否持续返回 `0`，以及是否调用了 `xgeQuit`。
