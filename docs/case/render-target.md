# RenderTarget 与离屏绘制

本案例展示把一组绘制命令先画到 RenderTarget，再把结果作为纹理绘制回窗口。

[返回范例解析](README.md) | [RenderTarget 教程](../guide/render-target-intro.md) | [RenderTarget API](../api/render-target.md)

## 问题

RenderTarget 适合做小地图、后处理、动态头像、缓存复杂 UI、阴影贴图和截图前的离屏合成。

## 渲染流程

```text
RenderTarget
  -> xgePassBegin
  -> draw scene into texture
  -> xgePassEnd
Window
  -> draw RenderTarget texture
```

## 步骤 1：创建 RenderTarget

```c
static xge_render_target_t g_target;

static int InitTargets(void)
{
	return xgeRenderTargetCreate(&g_target, 256, 256);
}
```

RenderTarget 的尺寸不需要和窗口一致。对 UI 缓存或小地图，使用固定尺寸通常更稳定。

## 步骤 2：离屏绘制

```c
static void DrawToTarget(void)
{
	xge_pass_t pass;

	xgePassInit(&pass, &g_target, XGE_CLEAR_COLOR, xgeColorRGBA(20, 24, 32, 255));

	if ( xgePassBegin(&pass) != XGE_OK ) {
		return;
	}

	xgeShapeCircleFill(128.0f, 128.0f, 72.0f, xgeColorRGBA(80, 180, 120, 255));
	xgeShapeLine(32.0f, 224.0f, 224.0f, 32.0f, 4.0f, xgeColorRGBA(255, 255, 255, 255));

	xgePassEnd(&pass);
}
```

## 步骤 3：绘制到窗口

```c
static int MainFrame(void* pUser)
{
	xge_texture pTexture;

	(void)pUser;

	DrawToTarget();

	xgeClear(xgeColorRGBA(10, 12, 18, 255));
	pTexture = xgeRenderTargetTexture(&g_target);
	xgeDraw(pTexture, 32.0f, 32.0f);
	return 0;
}
```

## 步骤 4：窗口大小变化

如果 RenderTarget 用作主画面缓冲，窗口变化时应调用：

```c
xgeRenderTargetResize(&g_target, iFramebufferWidth, iFramebufferHeight);
```

如果 RenderTarget 是固定大小缓存，不要跟随窗口重建，否则会增加 GPU 分配抖动。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeRenderTargetCreate` | 创建离屏目标 |
| `xgeRenderTargetResize` | 调整离屏目标尺寸 |
| `xgePassInit` | 初始化一次 render pass |
| `xgePassBegin` | 切换到目标并应用 clear |
| `xgePassEnd` | 结束 pass 并恢复默认目标 |
| `xgeRenderTargetTexture` | 取得可绘制纹理 |
| `xgeRenderTargetReadPixels` | 读取像素用于截图或测试 |

## 常见问题

如果画面上下颠倒，检查平台纹理坐标和 framebuffer 原点约定，不要在业务层反复做临时翻转。

如果读取像素很慢，确认不是每帧调用 `xgeRenderTargetReadPixels`。读回会造成 GPU/CPU 同步，只适合截图、测试和少量离屏任务。
