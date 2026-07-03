# XUI XSON App Layout Lab

`examples/xui_xson_app_layout_lab` 验证典型应用壳布局的 XSON 声明。它覆盖 dock shell、左侧导航、内容滚动区和底部状态栏。

## 覆盖范围

- dock root、top header、left rail、fill content、bottom status。
- ScrollView content 尺寸映射。
- 内容区 grid cards 的列数和行高。
- layout 后 header、rail、status、content 的关键几何。

## 构建和运行

```bat
examples\xui_xson_app_layout_lab\build.bat
build\xge_xui_xson_app_layout_lab.exe
```

## 通过标准

程序输出 `xui_xson_app_layout_lab ok: dock shell, scroll content, and status layout verified`，并以 `0` 退出。
