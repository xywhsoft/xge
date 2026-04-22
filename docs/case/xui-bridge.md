# XUI 桥接

本案例展示 XGE 如何把输入、时间和绘制能力提供给 XUI，并由 XUI 管理布局、控件事件和 GUI 绘制。

[返回范例解析](README.md) | [XUI 教程](../guide/xui-bridge-intro.md) | [XUI API](../api/xui.md)

## 问题

XGE 负责窗口、输入、GPU 和主循环；XUI 负责 retained-mode GUI 的布局、控件、焦点、事件和绘制。两者需要紧密配合，但不能把 XUI 设计成只能依赖 XGE 的组件。

当前阶段先在 XGE 内孵化 XUI，成熟后剥离到独立仓库，再保留高性能桥接层。

## 桥接边界

```text
XGE platform/input
  -> xge_event_t
  -> xgeXuiDispatchEvent
XGE frame time
  -> xgeXuiUpdate
XUI retained tree
  -> xgeXuiPaint
  -> XGE drawing API
```

## 步骤 1：初始化 XUI

```c
static xge_xui_context_t g_ui;

static int InitUI(void)
{
	if ( xgeXuiInit(&g_ui) != XGE_OK ) {
		return XGE_ERROR_INVALID_STATE;
	}

	xgeXuiSetDipScale(&g_ui, 1.0f);
	return XGE_OK;
}
```

## 步骤 2：构建控件树

```c
static xge_xui_widget_t g_panel;
static xge_xui_widget_t g_button_widget;
static xge_xui_button_t g_button;

static void BuildUI(void)
{
	xge_xui_widget root;

	root = xgeXuiRoot(&g_ui);

	xgeXuiWidgetSetLayout(root, XGE_XUI_LAYOUT_VERTICAL);
	xgeXuiWidgetSetPaddingPx(root, 16.0f, 16.0f, 16.0f, 16.0f);

	xgeXuiWidgetAdd(root, &g_panel);
	xgeXuiWidgetSetSize(&g_panel, xgeXuiSizePercent(100.0f), xgeXuiSizeContent());
	xgeXuiWidgetSetBackground(&g_panel, xgeColorRGBA(32, 38, 48, 255));

	xgeXuiWidgetAdd(&g_panel, &g_button_widget);
	xgeXuiButtonInit(&g_button, &g_ui, &g_button_widget);
	xgeXuiButtonSetText(&g_button, &g_font, "Start");
}
```

## 步骤 3：接入事件

```c
static int UIEvent(const xge_event_t* pEvent)
{
	if ( xgeXuiDispatchEvent(&g_ui, pEvent) ) {
		return 1;
	}
	return 0;
}
```

业务层应先让 XUI 处理鼠标、触摸、键盘和文本输入。XUI 未消费的事件再交给游戏逻辑。

## 步骤 4：更新和绘制

```c
static int MainFrame(void* pUser)
{
	(void)pUser;

	xgeClear(xgeColorRGBA(18, 20, 26, 255));
	DrawGameWorld();

	xgeXuiUpdate(&g_ui, xgeGetDelta());
	xgeXuiPaint(&g_ui);
	return 0;
}
```

APP 模式下可以用 `xgeXuiRefreshNeeded` 判断是否需要刷新，再手动 `xgeBegin`、`xgeXuiPaint`、`xgeEnd`、`xgeFlush`。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeXuiInit` | 初始化 XUI context |
| `xgeXuiRoot` | 获取根控件 |
| `xgeXuiWidgetAdd` | 建立控件父子关系 |
| `xgeXuiDispatchEvent` | 分发输入事件 |
| `xgeXuiUpdate` | 更新 GUI 状态 |
| `xgeXuiPaint` | 绘制 GUI |
| `xgeXuiRefreshNeeded` | 查询是否需要刷新 |

## 常见问题

如果按钮没有响应，检查控件是否 visible、enabled、focusable，以及事件是否先被其他系统消费。

如果 APP 模式下 CPU 占用高，确认没有每帧无条件刷新，而是只在 XUI 请求 repaint 或外部状态变化时提交渲染。
