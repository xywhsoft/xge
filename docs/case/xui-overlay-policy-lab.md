# XUI Overlay Policy Lab

`examples/xui_overlay_policy_lab` 是 XUI overlay 体系的聚合策略验证用例，覆盖 Popup、Dialog、Menu、Tooltip 在无窗口环境下的关闭、焦点、owner 和启用策略。

## 覆盖范围

- Popup：默认关闭策略、outside/escape 自动关闭、owner 区域穿透、打开后的可见性和焦点。
- Dialog：modal 与 non-modal outside click 行为、Escape 开关、outside close、关闭回调、重新打开后的焦点、focus restore 和 z base。
- Menu：弹层创建、打开后 list 焦点、关闭后 owner 焦点恢复、Escape 关闭。
- Tooltip：owner capture 接管与旧 capture 链接、文本/启用门控、owner 事件打开和关闭。
- Top overlay：打开 Popup 后再打开 Menu，Escape 只关闭当前聚焦的 Menu，底层 Popup 保持打开。
- Public policy：Popup placement/anchor/offset、screen clamp、z base/top overlay、modal outside consume、owner disabled 自动关闭。

## 运行方式

```bat
examples\xui_overlay_policy_lab\build.bat
build\xge_xui_overlay_policy_lab.exe
```

通过时输出示例：

```text
xui-overlay-policy-lab final-summary create=1 popup=1 dialog=1 menu=1 tooltip=1 top_escape=1 policy=1 popup_close=3 dialog_close=2 legacy=2
```
