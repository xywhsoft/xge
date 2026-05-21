# XUI MsgTip

`MsgTip` 是 XUI 便捷提示服务，用于显示单条短暂的居中提示，例如保存成功、操作失败、轻量警告或确认提示。它不是普通业务控件，也不是 XSON 页面节点。

## 定位

`MsgTip` 和 `MsgBox`、`InputBox`、`Toast` 属于 XUI 便捷层：

- 内部仍使用 `Widget` 接入 XUI 的事件、更新、绘制、裁剪和 Z 序。
- 调用方负责创建并持有一个 overlay host widget，通常挂到 XUI root 或 overlay root。
- 业务界面不通过 XSON 声明 `msgTip` 节点；XSON 页面应通过业务事件触发 C API。
- 一次只显示一条提示；新的 `Show` 会覆盖当前提示。

`MsgTip` 不用于列表式通知队列。需要多条排队通知时使用 `Toast`；需要用户决策时使用 `MsgBox`；需要短文本输入时使用 `InputBox`。

## API

```c
int xgeXuiMsgTipInit(xge_xui_msg_tip pTip, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
void xgeXuiMsgTipUnit(xge_xui_msg_tip pTip);
int xgeXuiMsgTipShow(xge_xui_msg_tip pTip, int iType, const char* sText, float fDuration);
void xgeXuiMsgTipClose(xge_xui_msg_tip pTip);
int xgeXuiMsgTipIsOpen(xge_xui_msg_tip pTip);
void xgeXuiMsgTipSetText(xge_xui_msg_tip pTip, const char* sText);
void xgeXuiMsgTipSetType(xge_xui_msg_tip pTip, int iType);
void xgeXuiMsgTipSetIconTexture(xge_xui_msg_tip pTip, xge_texture pTexture, xge_rect_t tSrc);
void xgeXuiMsgTipSetMetrics(xge_xui_msg_tip pTip, float fMinWidth, float fMaxWidth, float fMinHeight, float fOffsetY);
void xgeXuiMsgTipSetColors(xge_xui_msg_tip pTip, uint32_t iBackground, uint32_t iText, uint32_t iIcon);
int xgeXuiMsgTipEvent(xge_xui_msg_tip pTip, const xge_event_t* pEvent);
int xgeXuiMsgTipEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
void xgeXuiMsgTipUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
void xgeXuiMsgTipPaintProc(xge_xui_widget pWidget, void* pUser);
```

## 类型

```c
XGE_XUI_MSG_TIP_ICON_NONE
XGE_XUI_MSG_TIP_ICON_INFO
XGE_XUI_MSG_TIP_ICON_QUEST
XGE_XUI_MSG_TIP_ICON_WAR
XGE_XUI_MSG_TIP_ICON_ERROR
```

类型决定默认图标和图标颜色。需要产品自定义图标时，调用 `xgeXuiMsgTipSetIconTexture`。

## 行为

- `xgeXuiMsgTipShow` 会设置类型、文本、持续时间并打开提示。
- `fDuration > 0` 时自动关闭。
- `fDuration <= 0` 时保持打开，直到调用 `Close` 或用户点击提示本体。
- 点击提示外部不关闭，也不消费事件；host widget 本体为 input transparent，只保留提示矩形作为命中区域。
- 文本会按 `maxWidth` 自动换行。
- `SetMetrics` 控制最小宽度、最大宽度、最小高度和垂直偏移。
- `SetColors` 控制背景、文字和图标颜色。

## XSON

新口径下不提供 `type: "msgTip"`。

原因是 `MsgTip` 是运行时服务，不是页面结构的一部分。页面声明可以创建按钮、菜单或快捷键，但提示的打开、关闭、内容和 duration 应由业务事件回调调用 C API 完成。

## 范例

```bat
examples\xui_msgtip\build.bat
build\xui_msgtip.exe --frames 5
```

范例覆盖：

- `none/info/question/warning/error` 类型。
- 长文本换行。
- duration 自动过期。
- 点击提示关闭。
- 自定义颜色。
