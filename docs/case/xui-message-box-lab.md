# XUI Message Box Lab

`examples/xui_message_box_lab` 覆盖 `MessageBox` 的打开关闭、按钮结果、键盘确认/取消和一次性回调。

## 覆盖 API

- `xgeXuiMessageBoxInit`
- `xgeXuiMessageBoxSetText`
- `xgeXuiMessageBoxSetType`
- `xgeXuiMessageBoxSetButtons`
- `xgeXuiMessageBoxSetResult`
- `xgeXuiMessageBoxSetColors`
- `xgeXuiMessageBoxSetOpen`
- `xgeXuiMessageBoxGetResult`

## 构建和运行

```bat
examples\xui_message_box_lab\build.bat
build\xge_xui_message_box_lab.exe --frames 5
```

## 自动检查点

- `init=1`：消息框控件完成初始化，标题、正文、类型和按钮组可用。
- `click=1`：鼠标点击按钮能写入 result 并触发回调。
- `escape=1`：Escape 可产生取消结果。
- `enter=1`：Enter 可产生默认确认结果。
- `once=1`：一次关闭只触发一次结果回调。

## 通过标准

程序自动退出，并打印 `xui-message-box-lab final-summary`，其中 `init`、`click`、`escape`、`enter`、`once` 均为 `1`。

