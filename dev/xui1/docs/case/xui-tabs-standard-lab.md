# XUI Tabs Standard Lab

`examples/xui_tabs_standard_lab` 是 Tabs 的标准交互验证用例，覆盖 disabled tab、close button、dirty mark、scrollable tab strip、icon 预留、键盘导航和选择/关闭回调。

## 覆盖范围

- Disabled tab：鼠标点击 disabled tab 不消费选择，不改变 selected，也不触发 select 回调。
- Keyboard：Left/Right 在可用 tab 之间循环移动，并跳过 disabled tab。
- Close button：关闭区域点击触发 close 回调，不触发 select 回调。
- Scrollable tab strip：wheel 调整 `scrollX`，选择不可见 tab 时自动滚动到可视区域。
- Dirty/Icon metadata：dirty 数组和 icon 槽位由外部数组驱动，绘制时为文本区域预留空间。

## 运行方式

```bat
examples\xui_tabs_standard_lab\build.bat
build\xge_xui_tabs_standard_lab.exe
```

通过时输出示例：

```text
xui-tabs-standard-lab final-summary create=1 disabled=1 keyboard=1 close=1 scroll=1 meta=1 selected=3 scrollX=210.00 callbacks=3/1 last=3/0
```
