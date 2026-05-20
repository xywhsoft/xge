# MsgBox 控件

MsgBox 是基于 `Window` 控件拼装的弹窗控件，用于显示标题、图标、说明文本和一组操作按钮。

## 设计思路

MsgBox 直接使用 `Window` 的标题栏、关闭按钮、Z 序和 overlay 能力。这样它和普通窗口、工具窗口保持一致的视觉和事件模型。

MsgBox 可以设置为 modal。modal 打开时，弹窗外的鼠标和键盘事件会被消费，调用方必须先关闭弹窗才能继续操作下面的 XUI 空间。非 modal 打开时，它只是一个浮动窗口。

内容文本会按窗口可用宽度自动换行。弹窗自动尺寸以根窗口宽度的 80% 作为最大宽度目标，避免长文本把窗口撑得过宽。

## API

```c
int xgeXuiMsgBoxInit(xge_xui_msg_box pBox, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiMsgBoxUnit(xge_xui_msg_box pBox);
void xgeXuiMsgBoxSetText(xge_xui_msg_box pBox, xge_font pFont, const char* sTitle, const char* sMessage);
void xgeXuiMsgBoxSetType(xge_xui_msg_box pBox, int iType);
void xgeXuiMsgBoxSetIconTexture(xge_xui_msg_box pBox, xge_texture pTexture, xge_rect_t tSrc);
void xgeXuiMsgBoxSetButtons(xge_xui_msg_box pBox, int iButtons);
void xgeXuiMsgBoxSetCustomButtons(xge_xui_msg_box pBox, xvalue arrButtons);
void xgeXuiMsgBoxSetResult(xge_xui_msg_box pBox, xge_xui_select_proc procResult, void* pUser);
void xgeXuiMsgBoxSetModal(xge_xui_msg_box pBox, int bModal);
void xgeXuiMsgBoxSetOpen(xge_xui_msg_box pBox, int bOpen);
int xgeXuiMsgBoxGetResult(xge_xui_msg_box pBox);
```

公开 API 统一为 `xgeXuiMsgBox*`。XSON 类型统一使用 `msgBox`。

## 图标

```c
XGE_XUI_MSG_BOX_ICON_NONE
XGE_XUI_MSG_BOX_ICON_INFO
XGE_XUI_MSG_BOX_ICON_QUEST
XGE_XUI_MSG_BOX_ICON_WAR
XGE_XUI_MSG_BOX_ICON_ERROR
```

如果需要游戏或产品自己的图标，使用 `xgeXuiMsgBoxSetIconTexture` 设置纹理。

## 按钮与返回值

```c
XGE_XUI_MSG_BOX_BUTTON_OK
XGE_XUI_MSG_BOX_BUTTON_OK_CANCEL
XGE_XUI_MSG_BOX_BUTTON_YES_NO_CANCEL
XGE_XUI_MSG_BOX_BUTTON_CUSTOM
```

返回值：

```c
XGE_XUI_MSG_BOX_RESULT_CLOSE  // -1
XGE_XUI_MSG_BOX_RESULT_OK     // 0
XGE_XUI_MSG_BOX_RESULT_CANCEL // 1
XGE_XUI_MSG_BOX_RESULT_YES    // 0
XGE_XUI_MSG_BOX_RESULT_NO     // 2
```

关闭窗口返回 `-1`。自定义按钮返回按钮描述中的 `id`。

自定义按钮使用 `xvalue` 数组，每个元素可以是文本，也可以是 table：

```c
{ text: "Retry", id: 10, style: "primary" }
{ text: "Delete", id: 20, style: "danger" }
```
