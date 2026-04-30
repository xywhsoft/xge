# XUI Scroll Standard Lab

`examples/xui_scroll_standard_lab` 是 ScrollView 的标准交互验证用例，覆盖 offset clamp、横向/纵向 wheel、内容拖拽、滚动条轨道页跳、ensure visible、scrollbar policy、nested scroll policy、focusable/clip 和 disabled widget。

## 覆盖范围

- Offset clamp：`SetOffset` 会把负值限制到 0，并把超过内容边界的值限制到最大滚动范围。
- Mouse wheel：在内容区内消费事件，并同时支持 `fDX` 横向滚动和 `fDY` 纵向滚动；内容区外不消费事件。
- Content drag：内容区按下后捕获鼠标，移动时按拖拽距离更新 offset，抬起后释放 capture。
- Scrollbar track：点击垂直/水平滚动条轨道时按可视区域尺寸翻页。
- Ensure visible：`xgeXuiScrollViewEnsureRectVisible` 和 `xgeXuiScrollViewEnsureChildVisible` 会把目标矩形/子 widget 调整到可视区域内。
- Scrollbar policy：`AUTO` 只在内容超出时显示滚动条，`HIDDEN` 隐藏滚动条但保留 wheel/drag 滚动能力。
- Nested scroll policy：`PASS_EDGE` 在内容已到边界且 wheel 无法改变 offset 时返回 continue，便于父级滚动容器接管。
- Disabled widget：widget disabled 后不消费 wheel，也不改变 offset。

## 运行方式

```bat
examples\xui_scroll_standard_lab\build.bat
build\xge_xui_scroll_standard_lab.exe
```

通过时输出示例：

```text
xui-scroll-standard-lab final-summary create=1 clamp=1 wheel=1 drag=1 bars=1 ensure=1 policy=1 focus=1 offset=20.00/20.00
```
