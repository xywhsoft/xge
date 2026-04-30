# XUI Accordion Lab

`examples/xui_accordion_lab` 覆盖 `Accordion` 的 section、展开状态、single/multiple 模式、内容高度和选择回调。

## 覆盖 API

- `xgeXuiAccordionInit`
- `xgeXuiAccordionSetFont`
- `xgeXuiAccordionSetSelect`
- `xgeXuiAccordionSetMetrics`
- `xgeXuiAccordionSetMode`
- `xgeXuiAccordionGetSectionCount`
- `xgeXuiAccordionIsExpanded`
- `xgeXuiAccordionGetContentHeight`

## 构建和运行

```bat
examples\xui_accordion_lab\build.bat
build\xge_xui_accordion_lab.exe --frames 5
```

## 自动检查点

- `init=1`：section 数量和初始展开状态正确。
- `multi=1`：多展开模式下多个 section 可同时展开。
- `single=1`：单展开模式下新 section 展开会收起其他 section。
- `selected`：输出最后选择的 section。
- `height`：输出内容区域高度，便于人工核对布局。

## 通过标准

程序自动退出，并打印 `xui-accordion-lab final-summary`，其中 `init`、`multi`、`single` 均为 `1`。

