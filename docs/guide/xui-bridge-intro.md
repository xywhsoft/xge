# XUI 桥接入门

> 状态：中文初稿已生成，待审阅。

XUI 当前在 XGE 内部孵化，目标是形成可独立工作的 DUI 系统。XGE 给 XUI 提供输入、时间、资源和渲染能力；XUI 负责布局、控件、事件响应和绘制请求。

> 当前 guide 描述第一版 bridge。维护者设计 XUI 基础层时，以 `dev/docs/XUI Widget V2基础设计.md` 为准；剥离到独立仓库必须等 Widget V2 的 clip、Z、事件、焦点、滚动、IME 和盒模型成熟后再执行。

## XGE 和 XUI 的边界

| 模块 | 职责 |
| --- | --- |
| XGE | 平台输入、时间推进、纹理/字体、绘制 API、clip、刷新请求。 |
| XUI | widget 树、布局、focus/capture、控件状态、事件响应、paint。 |

XUI 不应该强绑定 XGE。当前先在 XGE 内孵化，是为了开发效率和测试便利；成熟后再剥离到 XUI 仓库。

## 最小集成顺序

```text
xgeXuiInit
  -> xgeXuiSetHost
  -> 创建 root 子控件
  -> xgeXuiDispatchEvent
  -> xgeXuiUpdate
  -> xgeXuiPaint
  -> xgeXuiUnit
```

## Host bridge

Host 是 XUI 调用宿主绘制能力的桥。

```c
static void HostDrawRect(xge_rect_t rect, uint32_t color, void* user)
{
	(void)user;
	xgeShapeRectFill(rect, color);
}

static void HostRequestRefresh(void* user)
{
	(void)user;
	/* wake host or mark dirty */
}
```

设置 host：

```c
xge_xui_host_t host;
memset(&host, 0, sizeof(host));
host.draw_rect = HostDrawRect;
host.request_refresh = HostRequestRefresh;
xgeXuiSetHost(&ui, &host);
```

真实项目还应提供 `draw_image`、`draw_text_rect`、`measure_text`、`clip_set` 和 `clip_clear`。

## 创建 widget

```c
xge_xui_widget root = xgeXuiRoot(&ui);
xge_xui_widget button_widget = xgeXuiWidgetCreate();

xgeXuiWidgetSetRect(button_widget, (xge_rect_t){ 20.0f, 20.0f, 160.0f, 36.0f });
xgeXuiWidgetSetFocusable(button_widget, 1);
xgeXuiWidgetAdd(root, button_widget);
```

控件对象通常绑定在 widget 上：

```c
xge_xui_button_t button;
xgeXuiButtonInit(&button, &ui, button_widget);
xgeXuiButtonSetText(&button, &font, "OK");
```

## 事件、更新和绘制

```c
xgeXuiDispatchEvent(&ui, &event);
xgeXuiUpdate(&ui, xgeGetDelta());
xgeXuiPaint(&ui);
```

APP 模式下，控件状态变化后可以请求刷新：

```c
if ( xgeXuiRefreshNeeded(&ui) ) {
	xgeBegin();
	xgeXuiPaint(&ui);
	xgeEnd();
	xgeFlush();
	xgeXuiRefreshClear(&ui);
}
```

## 文本输入和 IME

第一版不实现完整内置 IME，依赖系统 IME。XUI input 负责：

- 接收提交后的文本。
- 管理光标和选择区。
- 通过 widget/focus 机制提供候选框矩形给宿主。

```c
xge_rect_t rc = xgeXuiGetImeCandidateRect(&xui);
```

宿主拿当前 context 的矩形交给系统 IME；自定义文本控件可通过 `xgeXuiWidgetSetImeCandidateRect` 注册自己的候选框解析器。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 控件不刷新 | host 没实现 `request_refresh` 或未标记 dirty | 实现刷新回调，必要时调用 `xgeXuiRefreshRequest`。 |
| 文本测量不准 | host 没实现 `measure_text` | 用 XGE Font/Text API 实现测量。 |
| 批量修改布局卡顿 | 每次修改都触发布局 | 用 `xgeXuiLayoutBatchBegin` / `xgeXuiLayoutBatchEnd` 包住批量操作。 |

## 下一步

- 查完整函数看 [XUI API](../api/xui.md)。
- 看完整例子用 [XUI 桥接范例](../case/xui-bridge.md)。

[返回教程入口](README.md)
