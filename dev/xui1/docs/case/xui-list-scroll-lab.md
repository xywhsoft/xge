# XUI List Scroll Lab

`examples/xui_list_scroll_lab` exercises the scrolling and list-style XUI controls in one auto-exit example. It covers `ScrollView` and `ListView`, including content sizing, offset and scroll getters/setters, disabled rows, hover tracking, pointer selection, thumb dragging, and keyboard navigation with `Up/Down/Home/End/PageUp/PageDown`.

## Covered API

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`
- `xgeXuiScrollViewEvent`
- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetEnabledItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`
- `xgeXuiListViewSetScroll`
- `xgeXuiListViewGetScroll`
- `xgeXuiListViewSetSelect`
- `xgeXuiListViewSetColors`
- `xgeXuiListViewSetDisabledTextColor`
- `xgeXuiListViewEvent`

## Build And Run

```bat
examples\xui_list_scroll_lab\build.bat
build\xge_xui_list_scroll_lab.exe --frames 5
```

## Verification

- Console prints `xui-list-scroll-lab final-summary`.
- `scroll=1` means `ScrollView` default values matched implementation, content size and color overrides stuck, and wheel, free drag, page jump, plus thumb drag all changed offsets as expected.
- `list=1` means `ListView` items, font, colors, disabled text color, mouse selection, and callback wiring all worked.
- `disabled=1` means clicking a disabled row was consumed but did not change the selected item.
- `hover=1` means pointer hover updated the row highlight index and pointer leave cleared it.
- `keys=1` means keyboard selection covered `Up/Down`, `PageUp/PageDown`, `Home/End`, and activation through `Enter` or `Space`, while still skipping disabled rows.
- `scroll_ops=1` means both `ScrollView` and `ListView` scroll getters/setters clamped correctly and scrollbar page-jump or thumb-drag paths changed the stored scroll values.
