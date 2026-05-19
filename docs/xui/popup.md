# XUI Popup

Popup 是 XUI 的通用弹层基础设施。它不是普通业务控件，而是让内容以 overlay 身份出现在根界面之上的控制器。

当前设计以 [Viewport / Scroll](scrollview.md) 为准：Popup 始终使用 `ScrollView` 承载内容，不再维护“普通 widget 容器”和“滚动容器”两套路径。

## 职责边界

Widget 负责节点、盒模型、布局、裁剪、基础绘制和事件路由。

Popup 负责：

- 挂载到 `overlayRoot`。
- 绑定 owner、显式 anchor、cursor 或 fixed 坐标。
- 打开、关闭、置顶和可见性。
- 根据 anchor point 和 popup direction 计算位置。
- 在空间不足时执行方向回退和最终窗口内 clamp。
- 管理 outside click、owner 区域点击、Escape、modal/consume 策略。
- 打开时处理焦点，关闭时恢复焦点。
- owner hidden 或 disabled 时自动关闭。
- 使用内部 `ScrollView` 承载内容溢出。

Popup 不负责业务内容。Menu 的菜单项、ComboBox 的选项列表、ColorPicker 的颜色面板、DatePicker 的日历面板都由具体控件实现。

## 尺寸策略

硬规则：Popup 不缩放内容。

控件申请的 content size 是业务内容设计尺寸。窗口放不下时，缩小的是可见 viewport，不是 content。内容通过内部 `ScrollView` 滚动查看。

```text
contentSize = 控件申请大小
viewportSize = min(contentSize, windowSize - 必要滚动条预留)
popupSize = viewportSize + 可见滚动条预留 + popup 边框
```

当 `contentSize <= viewportSize` 时，内部 `ScrollView` 不显示滚动条；当内容超出 viewport 时，`ScrollView` 自动显示需要的滚动条。

滚动条预留不应制造无意义的另一轴滚动条：

- 横向内容本来能放下、只有纵向内容溢出时，Popup 宽度增加纵向滚动条预留，保持内容 viewport 宽度不变，不显示横向滚动条。
- 纵向内容本来能放下、只有横向内容溢出时，Popup 高度增加横向滚动条预留，保持内容 viewport 高度不变，不显示纵向滚动条。
- 只有窗口硬限制导致 `contentSize + 必要滚动条预留` 仍放不下时，才允许另一轴滚动条出现。

## 结构

```text
overlayRoot
  popupWidget            // overlay surface / viewport owner
    scrollViewWidget     // Popup 内部唯一承载路径
      contentWidget      // 业务内容，尺寸等于 contentSize
```

具体控件只把业务内容放进 `contentWidget`。裁剪、滚动条区域、滚动偏移和事件坐标映射由 ScrollView 统一处理。

## 锚点和方向

Popup 的位置由两个参数决定：

- `anchorPoint`：以 owner、anchor rect、cursor 或 fixed rect 的哪个点作为锚点。
- `direction`：popup 从锚点向哪个象限展开。

默认：

```text
anchorPoint = bottomLeft
direction = rightDown
```

常用组合：

| anchorPoint | direction | 用途 |
| --- | --- | --- |
| `bottomLeft` | `rightDown` | ComboBox、普通下拉 |
| `bottomRight` | `leftDown` | 右对齐下拉 |
| `topLeft` | `rightUp` | 向上弹出 |
| `topRight` | `leftUp` | 右对齐向上弹出 |
| `cursor` | `rightDown` | 右键菜单 |
| `fixed` | 调用方指定 | 固定坐标浮层 |

旧 placement 入口不作为新实现口径。需要表达弹出位置时，使用 `anchorPoint + direction`。

## 回退策略

Popup 先按 `anchorPoint + direction` 计算理想位置。如果 viewport 无法完整放进窗口，按以下顺序尝试：

1. 垂直对称回退。
2. 水平对称回退。
3. 对角回退。
4. 使用原方向计算结果，将 viewport clamp 到窗口内。

回退只移动 viewport，不缩放 content。

最终 clamp 允许遮盖 owner。此时窗口空间已经无法满足理想弹出关系，保证可操作比保证相对位置更重要。

## 关闭策略

Popup 需要显式区分三个区域：

- `inside`：popup viewport 内部。
- `owner`：owner 区域。
- `outside`：既不在 popup，也不在 owner。

策略：

| 策略 | 说明 |
| --- | --- |
| `outsideClose` | outside click 关闭并消费事件 |
| `outsideIgnore` | outside click 不关闭，事件继续传播 |
| `outsideConsume` | outside click 不关闭，但消费事件 |
| `ownerPassthrough` | owner click 交给 owner |
| `ownerClose` | owner click 关闭 |
| `ownerToggle` | owner click 关闭，并允许 owner 自己完成 toggle |
| `ownerConsume` | owner click 不关闭，但消费事件 |
| `escapeClose` | Escape 关闭并消费事件 |
| `escapeIgnore` | Escape 不处理 |

`consumeInside` 控制内部区域在子控件未消费时是否继续消费 pointer 事件。Menu、ComboBox、ColorPicker 这类交互弹层应打开；Tooltip 这类说明弹层可以关闭。

## 焦点策略

| 策略 | 说明 |
| --- | --- |
| `none` | 不改变焦点 |
| `popup` | 焦点给 popup surface |
| `firstChild` | 焦点给第一个可聚焦子项 |
| `custom` | 调用方指定焦点 widget |

关闭时优先恢复显式设置的 restore widget；否则恢复打开前焦点。restore 目标 hidden 或 disabled 时不恢复。

## 当前实现状态

Popup 已按新 viewport 口径恢复实现：

- 默认进入编译路径，不再受 viewport quarantine 隔离。
- 内部只使用 `ScrollView` 一种承载路径。
- 不在 Popup 内部重复实现滚动条、scroll offset 或 content 坐标转换。
- `xgeXuiPopupSetContentWidget` 会把业务内容挂到内部 `contentWidget`。
- `xgeXuiPopupSetContentSize` 设置业务内容尺寸；窗口放不下时只缩小 viewport，由内部 `ScrollView` 显示滚动条。
- `xgeXuiPopupGetViewportRect` / `xgeXuiPopupGetContentRect` / `xgeXuiPopupGetScroll` 提供公开检查入口，范例不再读取内部滚动字段。
- XSON `popup` 已恢复，`children` 会自动挂到内部 `contentWidget`。
- `PopupUnit` 只释放 Popup 创建的内部 ScrollView 结构；业务内容会保留在 popup widget 树下，避免控件 Unit 顺序造成悬空引用。

Menu、ComboBox、ColorPicker 这类弹层控件必须接入同一套 Popup 行为，不能在各自控件内重复实现弹层坐标、窗口回退或滚动条。ColorPicker 已按 context 级共享弹层接入；后续控件也应保持同一口径。
