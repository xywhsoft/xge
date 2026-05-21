# InputBox 便捷弹窗

InputBox 是基于 `Window`、`Input` 和 `Button` 拼装的便捷输入弹窗，用于短文本输入。它属于 XUI 便捷层，不建议作为普通业务控件或 XSON 页面节点使用。

## 设计思路

InputBox 使用和 MsgBox 相同的窗口基础设施：标题栏、关闭按钮、Z 序、overlay 和 modal 行为都由 `Window` 承担。控件本身只负责提示文本、输入框、确定按钮和取消按钮的布局与结果管理。

默认布局：

```text
窗口标题
提示内容                  OK
输入框                    Cancel
```

打开时会自动选中输入框中的全部内容，用户可以直接输入新值替换旧值。

## API

```c
int xgeXuiInputBoxInit(xge_xui_input_box pBox, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
void xgeXuiInputBoxUnit(xge_xui_input_box pBox);
void xgeXuiInputBoxSetText(xge_xui_input_box pBox, xge_font pFont, const char* sTitle, const char* sPrompt, const char* sInitial);
void xgeXuiInputBoxSetResult(xge_xui_input_box pBox, xge_xui_text_submit_proc procResult, void* pUser);
void xgeXuiInputBoxSetModal(xge_xui_input_box pBox, int bModal);
void xgeXuiInputBoxSetOpen(xge_xui_input_box pBox, int bOpen);
int xgeXuiInputBoxGetResultCode(xge_xui_input_box pBox);
char* xgeXuiInputBoxGetResult(xge_xui_input_box pBox);
```

## 返回值

`xgeXuiInputBoxGetResultCode` 返回和 MsgBox 相同的基础结果：

```c
XGE_XUI_MSG_BOX_RESULT_CLOSE  // -1
XGE_XUI_MSG_BOX_RESULT_OK     // 0
XGE_XUI_MSG_BOX_RESULT_CANCEL // 1
```

只有用户点击 OK 或在输入框中按 Enter 时，`xgeXuiInputBoxGetResult` 才会返回一份新分配的字符串副本。调用方负责释放该字符串。内存由 xrt 分配，内部代码可以使用 `xrtFree`；只链接 `xge.dll` 的应用也可以使用 `xgeMemoryFree` 释放。

## 行为

`modal=1` 时，InputBox 会阻塞弹窗外的 XUI 输入。`modal=0` 时，它作为普通浮动窗口显示。

关闭窗口返回 `-1`，点击 Cancel 或按 Escape 返回 `1`。OK 回调中可以直接读取输入字符串，也可以在回调后调用 `xgeXuiInputBoxGetResult` 获取可长期持有的副本。

## XSON

新设计口径下，InputBox 由业务事件通过 C API 打开，不作为推荐的 XSON 声明节点。历史 loader 中若仍存在 `inputBox` 声明路径，视为待清理的历史资产，不作为后续文档和范例口径。
