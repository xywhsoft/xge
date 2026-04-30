# XUI XSON Layout Gallery Lab

`examples/xui_xson_layout_gallery_lab` 验证 XUI layout gallery 迁移到 XSON 后的几何结果。它不打开窗口，直接加载声明式布局并检查关键 rect。

## 覆盖范围

- absolute、row、column、stack、grid 等布局声明。
- padding、gap、grid row height、size unit 等布局字段。
- 迁移后的关键控件几何关系。

## 构建和运行

```bat
examples\xui_xson_layout_gallery_lab\build.bat
build\xge_xui_xson_layout_gallery_lab.exe
```

## 通过标准

程序输出 `xui_xson_layout_gallery_lab ok: migrated layout gallery geometry verified`，并以 `0` 退出。
