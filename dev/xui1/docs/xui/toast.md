# XUI Toast

`Toast` 是绑定在 `xge_xui_context` 上的轻量通知服务，用于保存成功、失败提示、后台任务提醒等非阻塞消息。它不是普通业务控件，也不是 XSON 页面节点；业务代码通过 C API 在运行时显示通知。

## 设计口径

- `xge_xui_context` 持有唯一 Toast 运行时状态、内部 overlay widget、当前可见队列和等待队列。
- `xgeXuiToastShow` 创建一条通知并返回稳定 `toastId`。如果当前可见数量已满，通知进入等待队列。
- 默认显示时长为 3 秒，并且从通知真正进入可见队列时开始计时，而不是从入队时开始计时。
- Toast 文本按配置宽度自动换行；高度由标题、正文和最小高度共同决定。
- 点击关闭按钮只关闭通知，不触发点击回调。
- 点击通知正文会触发可选点击回调并关闭通知；没有点击回调时点击正文等同关闭。
- Toast 点击事件只登记关闭请求，实际回调和移除在下一次 `xgeXuiUpdate` 中完成。这样可以避免事件派发过程中重排或隐藏 hit widget，业务回调里再次 `show/close/clear` 也不会破坏当前事件栈。
- `Page Loader` 不识别 `type:"toast"`。XSON 页面可以声明按钮、菜单或快捷键，但 Toast 的内容、队列和生命周期由业务事件通过 C API 控制。

## API

```c
typedef void (*xge_xui_toast_click_proc)(xge_xui_context pContext, int iToastId, void* pUser);
typedef void (*xge_xui_toast_close_proc)(xge_xui_context pContext, int iToastId, int iReason, void* pUser);

int xgeXuiToastShow(xge_xui_context pContext, int iType, const char* sTitle, const char* sMessage, float fDuration, xge_xui_toast_click_proc procClick, void* pUser);
int xgeXuiToastClose(xge_xui_context pContext, int iToastId);
void xgeXuiToastClear(xge_xui_context pContext);
int xgeXuiToastGetActiveCount(xge_xui_context pContext);
int xgeXuiToastGetPendingCount(xge_xui_context pContext);
void xgeXuiToastSetPlacement(xge_xui_context pContext, int iPlacement);
void xgeXuiToastSetDirection(xge_xui_context pContext, int iDirection);
void xgeXuiToastSetMetrics(xge_xui_context pContext, float fWidth, float fMargin, float fGap, int iMaxVisible);
void xgeXuiToastSetFont(xge_xui_context pContext, xge_font pFont);
void xgeXuiToastSetColors(xge_xui_context pContext, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iMutedText, uint32_t iInfo, uint32_t iSuccess, uint32_t iWarning, uint32_t iError);
void xgeXuiToastSetClose(xge_xui_context pContext, xge_xui_toast_close_proc procClose, void* pUser);
```

## 类型和关闭原因

通知类型：

- `XGE_XUI_TOAST_TYPE_INFO`
- `XGE_XUI_TOAST_TYPE_SUCCESS`
- `XGE_XUI_TOAST_TYPE_WARNING`
- `XGE_XUI_TOAST_TYPE_ERROR`

关闭原因：

- `XGE_XUI_TOAST_CLOSE_TIMEOUT`
- `XGE_XUI_TOAST_CLOSE_CLICK`
- `XGE_XUI_TOAST_CLOSE_BUTTON`
- `XGE_XUI_TOAST_CLOSE_CLEAR`
- `XGE_XUI_TOAST_CLOSE_CAPACITY_DROP`
- `XGE_XUI_TOAST_CLOSE_API`

## 布局

`xgeXuiToastSetPlacement` 控制堆叠基准位置：

- `TOP_RIGHT`
- `TOP_LEFT`
- `BOTTOM_RIGHT`
- `BOTTOM_LEFT`
- `TOP_CENTER`
- `BOTTOM_CENTER`

`xgeXuiToastSetDirection` 控制通知堆叠方向：

- `AUTO`：顶部位置向下堆叠，底部位置向上堆叠。
- `DOWN`：强制向下堆叠。
- `UP`：强制向上堆叠。

`xgeXuiToastSetMetrics` 的 `iMaxVisible` 控制同时可见数量。传 `0` 时按窗口高度和最小高度自动估算，但最多不超过 `XGE_XUI_TOAST_VISIBLE_CAPACITY`。等待队列容量为 `XGE_XUI_TOAST_QUEUE_CAPACITY`，超出后丢弃最早的等待项并触发 `CAPACITY_DROP` close reason。

## 使用示例

```c
static void OnToastClick(xge_xui_context ui, int toastId, void* user)
{
	(void)ui;
	(void)toastId;
	/* open details, focus panel, or run app-specific command */
}

static void OnToastClose(xge_xui_context ui, int toastId, int reason, void* user)
{
	(void)ui;
	(void)toastId;
	(void)reason;
	(void)user;
}

xgeXuiToastSetPlacement(&ui, XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT);
xgeXuiToastSetDirection(&ui, XGE_XUI_TOAST_DIRECTION_AUTO);
xgeXuiToastSetMetrics(&ui, 320.0f, 18.0f, 8.0f, 3);
xgeXuiToastSetClose(&ui, OnToastClose, NULL);

xgeXuiToastShow(&ui, XGE_XUI_TOAST_TYPE_SUCCESS, "Saved", "Project settings were saved.", 3.0f, NULL, NULL);
xgeXuiToastShow(&ui, XGE_XUI_TOAST_TYPE_INFO, "Build finished", "Click to open the report.", 5.0f, OnToastClick, NULL);
```

## 范例

`examples/xui_toast` 覆盖 context 队列、可见数量限制、点击回调、关闭原因、关闭按钮、自动过期和运行时按钮触发。
