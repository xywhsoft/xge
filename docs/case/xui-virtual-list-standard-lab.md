# XUI Virtual List Standard Lab

`examples/xui_virtual_list_standard_lab` 是 VirtualList 的标准交互验证用例，覆盖 ensure index visible、slot reuse 生命周期、selected item persistent、variable item height API、滚动 clamp、focusable/clip 和 disabled widget。

## 覆盖范围

- Ensure visible：`xgeXuiVirtualListEnsureVisible` 会把指定 index 调整到可视区域内。
- Slot reuse：滚动只复用已创建 slot，slot index 改变时重新 bind；`Refresh` 会强制下一次布局重新 bind 现有 slot。
- Selected persistent：`Refresh` 和 slot reuse 不清空 selected；只有 item count 小于 selected 时才会失效。
- Variable height：`xgeXuiVirtualListSetItemHeightProc` 可以按 index 返回 item height，命中、ensure、scroll range 和 slot rect 都使用该高度。
- Keyboard / pointer：方向键移动选择并触发 select；鼠标点击可视行会选择对应真实 item index。
- Clamp / disabled：滚动 offset 会 clamp 到合法范围；widget disabled 后不消费键盘事件。

## 运行方式

```bat
examples\xui_virtual_list_standard_lab\build.bat
build\xge_xui_virtual_list_standard_lab.exe
```

通过时输出示例：

```text
xui-virtual-list-standard-lab final-summary create=1 ensure=1 reuse=1 selection=1 variable=1 clamp=1 focus=1 first=2 visible=6 selected=6 scroll=1728.00 create=6 bind=30 select=3 last=6
```
