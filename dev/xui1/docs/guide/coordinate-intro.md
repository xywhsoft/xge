# 坐标、Camera、Viewport 与 Clip 入门

本教程说明 XGE 中像素坐标、逻辑坐标、camera、viewport 和 clip 的关系。

[返回教程索引](README.md) | [Graphics API](../api/graphics.md) | [绘制教程](drawing-intro.md)

## 学习目标

读完后应能判断：

- 什么时候使用逻辑坐标，什么时候使用像素坐标。
- Camera 和 viewport 分别控制什么。
- Clip 适合解决哪些 GUI 和局部绘制问题。
- 高 DPI、离屏和小程序环境下如何避免坐标偏移。

## 默认坐标

XGE 默认使用 2D 友好的 y 轴向下坐标：

```text
(0, 0) ----------------> x
  |
  |
  v y
```

普通绘制 API 使用 `float` 逻辑坐标：

```c
xgeShapeRectFill((xge_rect_t){ 20.0f, 20.0f, 180.0f, 72.0f }, xgeColorRGBA(80, 140, 220, 255));
xgeDraw(&texture, 64.0f, 120.0f);
```

像素坐标 API 带 `Px` 后缀，适合像素精确的工具、编辑器网格或低分辨率像素风游戏：

```c
xgeDrawPx(&texture, 10, 20);
xgeShapeRectFillPx((xge_rect_t){ 0.0f, 0.0f, 64.0f, 64.0f }, xgeColorRGBA(255, 255, 255, 255));
```

## Camera 控制坐标映射

Camera 决定世界坐标如何变成屏幕坐标。默认 camera 与窗口大小重合。

```c
xge_camera_t camera;

camera = xgeCameraDefault((float)xgeGetWidth(), (float)xgeGetHeight());
camera.tPosition.fX = 100.0f;
camera.tPosition.fY = 60.0f;
camera.tScale.fX = 2.0f;
camera.tScale.fY = 2.0f;
xgeCameraSet(&camera);
```

世界坐标和屏幕坐标之间可显式转换：

```c
xge_vec2_t screen;
xge_vec2_t world;

screen = xgeWorldToScreen((xge_vec2_t){ 100.0f, 100.0f });
world = xgeScreenToWorld(screen);
```

## Viewport 控制输出区域

Viewport 决定图形输出写到 framebuffer 的哪个区域。它和 camera 独立，但默认重合。

```c
xgeViewportSet((xge_rect_t){ 0.0f, 0.0f, 400.0f, 300.0f });
DrawMiniMap();
xgeViewportClear();
```

典型用途：

- 分屏视图。
- 小地图。
- 编辑器中的预览窗口。
- 离屏目标的一部分区域绘制。

## Clip 控制裁剪区域

Clip 用于禁止绘制超出矩形范围的内容。

```c
xgeClipSet((xge_rect_t){ 20.0f, 20.0f, 300.0f, 180.0f });
DrawScrollableContent();
xgeClipClear();
```

XUI 的滚动列表、输入框、面板和局部刷新都会依赖 clip。

## 高 DPI

窗口逻辑尺寸不一定等于 framebuffer 尺寸。需要像素级计算时读取 runtime：

```c
xge_platform_runtime_t rt;

if ( xgePlatformRuntimeGet(&rt) == XGE_OK ) {
	/* rt.iWindowWidth / rt.iFramebufferWidth / rt.fDpiScale */
}
```

业务 UI 通常使用逻辑坐标；截图、读回和底层 framebuffer 操作使用像素尺寸。

## 常见错误

不要把 camera 当成 viewport。移动 camera 会改变坐标映射，不会限制绘制区域。

不要把 clip 当成布局。Clip 只负责裁剪，不负责计算子控件大小。

不要在高 DPI 平台用窗口尺寸创建全屏 RenderTarget，应使用 framebuffer 尺寸。

## 下一步

- 继续学习 [Premultiplied Alpha 与 Blend 入门](blend-alpha-intro.md)。
- 需要 2.5D 透视时读 [2.5D 入门](2.5d-intro.md)。
