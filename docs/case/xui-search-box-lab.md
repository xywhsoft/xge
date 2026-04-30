# XUI Search Box Lab

`examples/xui_search_box_lab` 覆盖 `SearchBox` 的搜索文本、placeholder、提交、清空和预留 suggestion 状态。

## 覆盖 API

- `xgeXuiSearchBoxInit`
- `xgeXuiSearchBoxSetPlaceholder`
- `xgeXuiSearchBoxSetSubmit`
- `xgeXuiSearchBoxSetClear`
- `xgeXuiSearchBoxSetText`
- `xgeXuiSearchBoxGetText`

## 构建和运行

```bat
examples\xui_search_box_lab\build.bat
build\xge_xui_search_box_lab.exe --frames 5
```

## 自动检查点

- `init=1`：文本初始化成功，suggestions 预留状态存在。
- `submit=1`：Enter 或提交路径可触发 submit 回调。
- `clear=1`：清空按钮可清除文本并触发 clear 回调。
- `sub` / `clr`：输出提交和清空回调次数。

## 通过标准

程序自动退出，并打印 `xui-search-box-lab final-summary`，其中 `init`、`submit`、`clear` 均为 `1`。

