# XUI Pager

`Pager` 是轻量分页切换控件，XSON 类型名为 `pager`。它只负责页码切换，不内置每页条数下拉框、跳页输入框和总数文本；这些业务组合应由外层界面使用 Label、Input、ComboBox 等控件自行拼装。

## 定位

- 页码使用 1-based 语义。
- 控件以单个 Widget 自绘实现，分页按钮共享边框，避免多个 Button 组合造成的边框叠线和额外内存。
- 固定包含上一页、首页、页码窗口、尾页、下一页；必要时在页码窗口两侧显示不可点击的 `...`。
- 支持鼠标点击和键盘 `Left` / `Right` / `Home` / `End`。
- 页码变化时先更新内部当前页，再触发 `onChange` 回调；业务侧如需拒绝切换，可在回调里调用 `xgeXuiPagerSetCurrent(pager, oldPage, 0)` 回滚。

## C API

```c
xge_xui_pager_t pager;
xgeXuiPagerInit(&pager, &xui, widget);
xgeXuiPagerSetPageCount(&pager, 80);
xgeXuiPagerSetCurrent(&pager, 6, 0);
xgeXuiPagerSetWindowSize(&pager, 5);
xgeXuiPagerSetChange(&pager, OnPagerChange, user);
```

常用 API：

| API | 说明 |
| --- | --- |
| `xgeXuiPagerSetPageCount` | 设置总页数，最小为 1。 |
| `xgeXuiPagerSetTotal` | 按总条数和每页条数计算总页数。 |
| `xgeXuiPagerSetCurrent` | 设置当前页，第三个参数决定是否触发回调。 |
| `xgeXuiPagerSetWindowSize` | 设置中间页码窗口数量，会自动规范为奇数。 |
| `xgeXuiPagerSetText` | 设置首页、尾页、上一页、下一页文本。 |
| `xgeXuiPagerSetMetrics` | 设置高度、页码宽度、文本按钮宽度、前后翻页宽度、省略号宽度。 |
| `xgeXuiPagerSetColors` | 设置背景、边框、文字、hover、active、当前页和禁用文本色。 |

默认尺寸：

| 项 | 默认值 |
| --- | --- |
| 高度 | `28` |
| 页码宽度 | `38` |
| 首页/尾页宽度 | `56` |
| 上一页/下一页宽度 | `46` |
| 省略号宽度 | `32` |

默认颜色：

| 项 | 默认值 |
| --- | --- |
| 背景 | `#FFFFFFFF` |
| 边框 | `#D6E2ECFF` |
| 文字 | `#243648FF` |
| Hover | `#EFF7FCFF` |
| Active | `#E4F0F8FF` |
| 当前页 | `#22B8AAFF` |
| 当前页文字 | `#FFFFFFFF` |
| 禁用文字 | `#A0ACB8FF` |

## XSON

```json
{
  "type": "pager",
  "pageCount": 80,
  "currentPage": 6,
  "windowSize": 5,
  "width": 470,
  "height": 28
}
```

可用属性：

| 属性 | 说明 |
| --- | --- |
| `pageCount` / `pages` | 总页数。 |
| `total` + `pageSize` | 按总条数和每页条数计算总页数。 |
| `currentPage` / `current` / `page` / `value` | 当前页。 |
| `windowSize` | 中间页码窗口数量。 |
| `firstText` / `lastText` / `prevText` / `nextText` | 命令按钮文本。 |
| `itemHeight` / `pageWidth` / `textWidth` / `navWidth` / `ellipsisWidth` | 尺寸配置。 |
| `backgroundColor` / `borderColor` / `textColor` | 基础颜色。 |
| `hoverColor` / `activeColor` / `currentColor` / `currentTextColor` / `disabledTextColor` | 状态颜色。 |
| `focusColor` | 键盘焦点边框色。 |

`onChange` 暂不通过 XSON 绑定；业务层应在 C 侧取得控件实例后调用 `xgeXuiPagerSetChange`。

## 范例

- `examples/xui_pager`
- `examples/xui_pager_xson`
