# XUI Widget OwnerDraw 入门

OwnerDraw 用于让业务代码接管 Widget 的视觉内容，同时继续复用 Widget 基础设施。它不是 `xgeXuiWidgetSetPaint` 的别名：`SetPaint` 是低层绘制回调，适合框架内部控件；OwnerDraw 是公开自绘入口，带有明确的替换范围和稳定绘制上下文。

## 基本边界

Widget 自绘分三层使用：

| 机制 | 用途 |
| --- | --- |
| `xgeXuiWidgetSetPaintBefore` / `xgeXuiWidgetSetPaintAfter` | 在默认绘制链前后追加装饰绘制，不替换内容 |
| `xgeXuiWidgetSetOwnerDraw` | 替换 Widget 内容、内容加子树，或完整替换 Widget 绘制 |
| 控件级 item/cell OwnerDraw | List、Table、Tree、PropertyGrid 等复合控件的行、单元格、片段自绘 |

普通自绘优先使用 Widget OwnerDraw；复合控件不要用 Widget OwnerDraw 接管整个控件，否则会把滚动、选择、虚拟化、命中测试和 tooltip 逻辑也一起绕开。复合控件的 item/cell OwnerDraw 会在控件重构时逐个接入。

## OwnerDraw Mode

`xgeXuiWidgetSetOwnerDraw(widget, mode, proc, user)` 支持三个替换范围：

| Mode | 行为 |
| --- | --- |
| `XGE_XUI_OWNER_DRAW_CONTENT` | 默认推荐模式。保留背景、clip、children、border、focus ring、disabled overlay、debug outline，只替换 Widget 内容绘制 |
| `XGE_XUI_OWNER_DRAW_CONTENT_AND_CHILDREN` | 接管内容和子树绘制；Widget 仍绘制背景、边框、焦点、禁用遮罩和调试线 |
| `XGE_XUI_OWNER_DRAW_FULL` | 完整接管 Widget 绘制链；框架只负责调用回调和清理 dirty 状态 |

传入 `NULL` 回调会关闭 OwnerDraw，非法 mode 会退回 `XGE_XUI_OWNER_DRAW_NONE`。

## PaintInfo

OwnerDraw 回调签名：

```c
static void myOwnerDraw(const xge_xui_paint_info_t* pInfo, void* pUser)
{
	if ( pInfo == NULL ) {
		return;
	}
	/* 使用 pInfo->pContext、pInfo->tContentRect、pInfo->iState 等字段绘制。 */
}
```

`xge_xui_paint_info_t` 提供：

| 字段 | 说明 |
| --- | --- |
| `pContext` / `pWidget` | 当前 XUI 上下文和 Widget |
| `iRole` / `iState` | Widget role 和聚合状态；状态包含 disabled、hover、focus、active |
| `iOwnerDrawMode` / `iPart` | 当前自绘 mode 和绘制部位 |
| `tOuterRect` / `tBorderRect` / `tPaddingRect` / `tContentRect` | Widget 盒模型矩形 |
| `pStyle` / `fDipScale` | 当前样式和 DIP scale |
| `pControl` | 控件对象指针，由 `xgeXuiWidgetSetOwnerDrawControl` 设置 |
| `pItemData` / `iItemIndex` / `iRow` / `iColumn` | 预留给复合控件 item/cell OwnerDraw |

Widget 级 OwnerDraw 当前会使用 `XGE_XUI_PAINT_PART_CONTENT` 或 `XGE_XUI_PAINT_PART_WIDGET`。item/cell/header 等 part 会由后续复合控件重构接入。

## Dirty 与绘制链

启用 OwnerDraw 后，Widget 仍遵守统一 dirty 规则。`CONTENT_AND_CHILDREN` 和 `FULL` 会在跳过子树绘制时清理子树 paint dirty，避免子节点长期保持 dirty 导致后续刷新无法正确冒泡。

`xgeXuiWidgetSetOwnerDrawControl` 会标记 Widget 需要重绘，因为传入的 control 指针通常会影响回调绘制结果。

推荐做法：

- 普通控件自绘内容，使用 `XGE_XUI_OWNER_DRAW_CONTENT`。
- 需要自己绘制内部子元素的画布或编辑器，使用 `XGE_XUI_OWNER_DRAW_CONTENT_AND_CHILDREN`。
- 极少数完全自管视觉链的场景，才使用 `XGE_XUI_OWNER_DRAW_FULL`。
- List、Table、Tree 这类控件，等待控件级 item/cell OwnerDraw，不要用 FULL 接管整个控件。
