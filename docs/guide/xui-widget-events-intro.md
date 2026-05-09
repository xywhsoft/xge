# XUI Widget 事件入门

本教程说明 XUI Widget 基础事件的使用边界。它面向自定义控件和后续控件重构，不替代 Button、Input、ListView 等控件自己的业务回调。

[返回教程索引](README.md) | [XUI API](../api/xui.md)

## 事件分层

XUI 事件分为三层：

- 原始 XGE 事件：`XGE_EVENT_MOUSE_DOWN`、`XGE_EVENT_KEY_DOWN`、`XGE_EVENT_TEXT` 等，是平台输入进入 XUI 的入口。
- Widget 基础语义事件：MouseEnter、MouseLeave、Click、DoubleClick、ContextMenu、HotKey、Command、Drag、Focus、Capture 和状态变化等，由 Widget 基础层统一合成和路由。
- 控件业务事件：Button 的 click、Input 的 text change、ListView 的 selection change 等，由具体控件解释自身状态后触发。

自定义控件应该优先接入 Widget 基础语义事件。只有在控件确实需要更底层信息时，才直接处理原始 XGE 事件。

## 路由和处理结果

Widget 事件按 tunnel、target、bubble 分发。事件对象会保留 original target、current target、phase、pointer id、capture 和 modifier 信息，控件代理转发时不能丢失这些字段。

处理结果有三种：

- `XGE_XUI_EVENT_CONTINUE`：当前回调不处理，继续路由；无人处理时允许回落到 XGE。
- `XGE_XUI_EVENT_HANDLED`：已处理，但允许当前路由继续；最终阻止 XGE fallback。
- `XGE_XUI_EVENT_CONSUMED`：已消费，立即停止当前路由。

## 热路径规则

MouseMove、hover、tooltip、hotkey 和 drag 都必须按需启用。没有注册对应事件、没有 tooltip、没有 capture、没有 drag interest 时，Widget 基础层不会为了这些能力扫描整棵树。

注册事件使用按类型的入口：

```c
xgeXuiWidgetSetEventHandler(widget, XGE_EVENT_XUI_CLICK, OnClick, user);
xgeXuiWidgetSetEventHandler(widget, XGE_EVENT_XUI_DRAG_BEGIN, OnDrag, user);
```

设置 tooltip 或动态 resolver 会自动加入 tooltip interest；清空 tooltip 后会移除 interest。

## Click、Command 和业务回调

Widget 的 `XGE_EVENT_XUI_CLICK` 只表示一个基础点击手势成立。它不等于 Button 的业务 click，也不理解 checked、selected、value、open 等控件状态。

多个入口触发同一业务动作时，优先使用 Command：

```c
xgeXuiWidgetSetEventHandler(widget, XGE_EVENT_XUI_COMMAND, OnCommand, user);
xgeXuiHotKeyRegisterCommand(&ui, widget, 'S', XGE_KEY_MOD_CTRL, 7001, "file.save", user);
xgeXuiCommandDispatch(&ui, widget, widget, 7001, "file.save", user);
```

HotKey 由 context 注册表匹配 key 和 modifier，不在每次按键时遍历 widget tree。隐藏、禁用、移除或销毁的 widget 不会触发 HotKey；移除或销毁时会清理对应注册项。

## Drag 和 Capture

基础 Drag 默认不开启。只有 widget 显式启用 drag 或注册 Drag 事件时，基础层才会在按下后监控移动距离；超过阈值后触发 DragBegin，并通过 pointer capture 接管后续 Move/End/Cancel。

```c
xgeXuiWidgetSetDragEnabled(widget, 1);
xgeXuiWidgetSetEventHandler(widget, XGE_EVENT_XUI_DRAG_MOVE, OnDrag, user);
```

地图、画布、节点编辑器这类控件通常需要完整掌控左键、右键和拖拽语义，不应该默认依赖 ScrollView 内容拖拽或基础 Drag 抢占行为。

## 状态清理

隐藏、禁用、移除或销毁 widget 时，基础层会清理指向该 widget 子树的 focus、hover、tooltip、capture、click/context press 和 drag state。移除或销毁还会清理 HotKey/Command 注册并刷新父级 subtree event mask。

控件自己的 active、pressed、selection、editing 等业务状态仍然由控件负责在 CaptureLost、CaptureCancel、DragCancel、VisibleChanged、EnabledChanged 等事件中收尾。

## 调试

`xgedbg` 提供 debug-only 事件追踪：

```c
xgedbgXuiEventTrace(&ui, &event, buffer, buffer_size);
```

该接口只在 `XGE_DEBUGMODE=1` 下声明并由 `xgedbg` 导出。它预览事件命中、capture、eventMask/subtreeEventMask、drag 状态和 HotKey/Command 匹配结果，不实际派发事件，不改变运行状态。
