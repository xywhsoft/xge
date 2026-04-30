# XUI XSON Property Grid Lab

`examples/xui_xson_property_grid_lab` 验证 PropertyGrid 的 XSON item 数据、状态标记和风格字段。

## 覆盖范围

- PropertyGrid item 数量、visible count 和 selected。
- readonly、defaultChanged、error、color editor 等 item 标记。
- row height、name width、value color 等风格字段。

## 构建和运行

```bat
examples\xui_xson_property_grid_lab\build.bat
build\xge_xui_xson_property_grid_lab.exe --frames 2
```

## 通过标准

程序输出 `xui-xson-property-grid-lab final-summary`，且 `load=1`、`items=1`、`flags=1`、`style=1`。
