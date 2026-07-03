# XUI XSON Page Lab

`examples/xui_xson_page_lab` 是 XSON 页面加载的最小结构验证。它不打开窗口，直接从内存加载页面声明，并检查 root、toolbar、content 的节点查找和样式映射。

## 覆盖范围

- XSON 内存加载和错误输出。
- `name` 节点查找。
- column/root、toolbar height、grid columns 等基础样式字段映射。

## 构建和运行

```bat
examples\xui_xson_page_lab\build.bat
build\xge_xui_xson_page_lab.exe
```

## 通过标准

程序输出 `xui_xson_page_lab ok`，并以 `0` 退出。
