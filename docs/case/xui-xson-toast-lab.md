# XUI XSON Toast Lab

`examples/xui_xson_toast_lab` 验证 Toast 的 XSON item、placement 和风格字段。

> 该范例是 Toast 重构前的历史 XSON 覆盖项。新设计口径下，Toast 和 MsgTip/MsgBox/InputBox 一样属于 C API 便捷服务，不作为新的 XSON 页面节点方向；后续 Toast 重构时会清理该历史范例。

## 覆盖范围

- Toast item 数量。
- success 类型 item。
- top-right placement。
- toast width 和 success color 等风格字段。

## 构建和运行

```bat
examples\xui_xson_toast_lab\build.bat
build\xge_xui_xson_toast_lab.exe --frames 2
```

## 通过标准

程序输出 `xui-xson-toast-lab final-summary`，且 `load=1`、`items=1`、`placement=1`、`style=1`。
