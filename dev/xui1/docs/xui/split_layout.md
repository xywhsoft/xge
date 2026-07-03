# XUI SplitLayout

`xge_xui_split_layout_t` 是用于把一个容器切分为多个可拖拽 pane 的布局控件。它是布局基础设施，分隔条只是 SplitLayout 的内部交互部件，不作为独立控件暴露。

## 设计边界

- SplitLayout 负责 pane 的尺寸分配、分隔条命中、拖拽提交和内部 pane 容器创建。
- pane 本身是普通 `widget`，可以继续设置布局、padding、gap、背景、边框，也可以继续放入任意子控件。
- 分隔条不是独立控件，不应该被外部添加到布局树中。外部只通过 SplitLayout API 设置分隔条尺寸、颜色和拖拽方式。
- `orientation=vertical` 表示垂直分隔条，pane 从左到右排列；`orientation=horizontal` 表示水平分隔条，pane 从上到下排列。
- SplitLayout 使用 widget 的边框、背景、裁剪、启用/禁用等基础能力；自身只负责内部 pane 和 divider 的布局。

## Pane 模型

每个 pane 使用独立结构保存状态：

```c
typedef struct xge_xui_split_layout_pane_t {
    xge_xui_widget pWidget;
    float fWeight;
    float fFixedSize;
    float fResolvedSize;
    float fMinSize;
    float fMaxSize;
    int iMode;
    int bCollapsed;
} xge_xui_split_layout_pane_t;
```

`iMode` 支持两种模式：

```c
#define XGE_XUI_SPLIT_PANE_GROW   0
#define XGE_XUI_SPLIT_PANE_FIXED  1
```

- `GROW`：按权重分配剩余空间。
- `FIXED`：按固定像素占用空间。
- `minSize` 和 `maxSize` 会参与布局求解，也会限制拖拽范围。
- 如果固定空间和最小空间超过可用空间，SplitLayout 会按比例降级分配，保证布局不会生成负尺寸。

## Divider 模型

分隔条拆成三个矩形：

```c
typedef struct xge_xui_split_layout_divider_t {
    xge_xui_widget pWidget;
    xge_rect_t tLayoutRect;
    xge_rect_t tVisualRect;
    xge_rect_t tHitRect;
} xge_xui_split_layout_divider_t;
```

- `dividerSize`：布局占位尺寸。
- `dividerVisualSize`：实际绘制的可见线条尺寸。
- `dividerHitSize`：鼠标命中和拖拽区域尺寸。

这样可以做到视觉上细线分割，但交互上有足够大的拖拽热区。

## 拖拽方式

- `shadowDrag=true`：拖拽时只显示阴影线，松开后提交尺寸。适合复杂界面，避免连续布局成本。
- `shadowDrag=false`：拖拽时实时调整 pane 尺寸。适合轻量布局，反馈直接。

拖拽提交后，如果注册了 change 回调，会回调被拖动的 divider 索引。

## API

```c
int xgeXuiSplitLayoutInit(xge_xui_split_layout split, xge_xui_context context, xge_xui_widget widget);
void xgeXuiSplitLayoutUnit(xge_xui_split_layout split);

void xgeXuiSplitLayoutSetOrientation(xge_xui_split_layout split, int orientation);
void xgeXuiSplitLayoutSetPaneCount(xge_xui_split_layout split, int count);
int xgeXuiSplitLayoutGetPaneCount(xge_xui_split_layout split);
xge_xui_widget xgeXuiSplitLayoutGetPaneWidget(xge_xui_split_layout split, int index);

void xgeXuiSplitLayoutSetPaneWeight(xge_xui_split_layout split, int index, float weight);
float xgeXuiSplitLayoutGetPaneWeight(xge_xui_split_layout split, int index);
void xgeXuiSplitLayoutSetPaneMode(xge_xui_split_layout split, int index, int mode);
int xgeXuiSplitLayoutGetPaneMode(xge_xui_split_layout split, int index);
void xgeXuiSplitLayoutSetPaneFixedSize(xge_xui_split_layout split, int index, float size);
float xgeXuiSplitLayoutGetPaneFixedSize(xge_xui_split_layout split, int index);
void xgeXuiSplitLayoutSetPaneMinSize(xge_xui_split_layout split, int index, float minSize);
void xgeXuiSplitLayoutSetPaneMaxSize(xge_xui_split_layout split, int index, float maxSize);
float xgeXuiSplitLayoutGetPaneSize(xge_xui_split_layout split, int index);

void xgeXuiSplitLayoutSetDividerSize(xge_xui_split_layout split, float size);
void xgeXuiSplitLayoutSetDividerVisualSize(xge_xui_split_layout split, float size);
void xgeXuiSplitLayoutSetDividerHitSize(xge_xui_split_layout split, float size);
void xgeXuiSplitLayoutSetShadowDrag(xge_xui_split_layout split, int enabled);
void xgeXuiSplitLayoutSetColors(xge_xui_split_layout split, uint32_t divider, uint32_t hover, uint32_t active, uint32_t shadow);
void xgeXuiSplitLayoutSetChange(xge_xui_split_layout split, xge_xui_split_layout_change_proc proc, void* user);
```

## XSON

`splitLayout` 是容器控件，但不用 `children` 直接描述子节点，而是使用 `panes[].children`。这样 XSON 结构和运行时 pane 边界完全一致。

```json
{
  "type": "splitLayout",
  "orientation": "vertical",
  "dividerSize": 10,
  "dividerVisualSize": 3,
  "dividerHitSize": 14,
  "shadowDrag": true,
  "panes": [
    {
      "mode": "fixed",
      "size": 150,
      "minSize": 120,
      "layout": "column",
      "padding": [12, 10, 12, 10],
      "children": [
        { "type": "label", "text": "Fixed pane" }
      ]
    },
    {
      "mode": "grow",
      "weight": 1,
      "minSize": 180,
      "layout": "column",
      "children": [
        { "type": "button", "text": "Action" }
      ]
    }
  ]
}
```

支持字段：

- `orientation`: `vertical` / `horizontal`
- `paneCount`: 未使用 `panes` 时可手动设置 pane 数
- `dividerSize`, `dividerVisualSize`, `dividerHitSize`
- `shadowDrag`
- `dividerColor`, `dividerHoverColor`, `dividerActiveColor`, `shadowColor`
- `panes[]`: pane 描述数组
- `panes[].mode`: `grow` / `fixed`
- `panes[].weight`, `panes[].size`, `panes[].fixedSize`, `panes[].minSize`, `panes[].maxSize`
- `panes[].layout`, `panes[].padding`, `panes[].gap`, `panes[].background`, `panes[].borderColor`, `panes[].borderWidth`
- `panes[].children`: 当前 pane 的子控件

## 范例

- `examples/xui_split_layout`：手写 API 版本，展示 fixed/grow pane、垂直/水平分割、阴影拖拽/实时拖拽、视觉分隔条和命中热区分离。
- `examples/xui_split_layout_xson`：XSON 版本，展示同一结构如何用 `panes[].children` 描述和加载。
