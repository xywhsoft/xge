# XUI XSON Virtual List Lab

`examples/xui_xson_virtual_list_lab` 验证 VirtualList 的 XSON 声明和 itemTemplate 复用。它不打开窗口，直接检查窗口化列表的首项、可见数量和 slot 复用。

## 覆盖范围

- VirtualList 控件实例、event/paint/layout proc 绑定。
- item count、item height、scrollY、background 映射。
- itemTemplate 生成 label slot。
- 滚动后 slot 复用和 label 数量稳定。

## 构建和运行

```bat
examples\xui_xson_virtual_list_lab\build.bat
build\xge_xui_xson_virtual_list_lab.exe
```

## 通过标准

程序输出 `xui_xson_virtual_list_lab ok: itemTemplate windowing and slot reuse verified`，并以 `0` 退出。
