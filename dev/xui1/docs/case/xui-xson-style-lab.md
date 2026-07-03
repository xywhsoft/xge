# XUI XSON Style Lab

`examples/xui_xson_style_lab` 验证 XSON 样式 token、父级样式继承和内联覆盖。它不打开窗口，适合在 smoke 中快速确认声明式样式链路没有退化。

## 覆盖范围

- root 继承布局、padding、gap、background。
- toolbar 的父级样式和内联覆盖。
- button 控件尺寸、正常态颜色和文本颜色映射。

## 构建和运行

```bat
examples\xui_xson_style_lab\build.bat
build\xge_xui_xson_style_lab.exe
```

## 通过标准

程序输出 `xui_xson_style_lab ok: inherited tokens and inline overrides verified`，并以 `0` 退出。
