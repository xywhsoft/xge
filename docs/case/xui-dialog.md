# XUI 对话框

本案例展示模态对话框的打开、关闭和事件拦截。

[返回范例解析](README.md) | [XUI 控件教程](../guide/xui-controls-intro.md) | [XUI API](../api/xui.md)

## 创建

```c
xge_xui_dialog_t dialog;

xgeXuiDialogInit(&dialog, &ui, &dialog_widget);
xgeXuiDialogSetTitle(&dialog, &font, "Confirm");
xgeXuiDialogSetClose(&dialog, OnDialogClose, NULL);
xgeXuiDialogSetOpen(&dialog, 0);
```

## 打开

```c
xgeXuiDialogSetOpen(&dialog, 1);
xgeXuiRefreshRequest(&ui);
```

## 关闭

```c
static void OnDialogClose(void* pUser)
{
	(void)pUser;
	xgeXuiDialogSetOpen(&dialog, 0);
}
```

## 常见失败原因

背景还能点击：dialog 事件没有拦截到底层控件。

关闭后仍绘制：widget visible/open 状态没有同步。

APP 模式不刷新：打开或关闭后没有请求 refresh。
