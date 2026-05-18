# XUI 渲染与刷新入门

本教程说明 XUI 如何通过 XGE host 绘制，并在 APP 模式下使用 dirty/refresh 降低无效刷新。

[返回教程索引](README.md) | [XUI API](../api/xui.md) | [运行模式教程](run-mode-intro.md)

## 绘制链路

```text
XUI widget tree
  -> layout
  -> paint callbacks
  -> xge_xui_host_t
  -> XGE draw/shape/text/clip
```

XUI 不直接绑定某个图形 API。XGE 是默认 host，但未来 XUI 可以接入其他宿主。

## 设置 Host

```c
xge_xui_host_t host;

memset(&host, 0, sizeof(host));
host.draw_rect = HostDrawRect;
host.draw_text_rect = HostDrawTextRect;
host.draw_image = HostDrawImage;
host.clip_set = HostClipSet;
host.clip_clear = HostClipClear;
host.request_refresh = HostRequestRefresh;
xgeXuiSetHost(&ui, &host);
```

在 XGE 内部孵化期，可以使用默认 XGE host，不必每个应用手写。

## 游戏循环中绘制

```c
static int MainFrame(void* pUser)
{
	(void)pUser;

	xgeClear(xgeColorRGBA(18, 20, 26, 255));
	DrawGame();
	xgeXuiUpdate(&ui, xgeGetDelta());
	xgeXuiPaint(&ui);
	return 0;
}
```

游戏连续刷新时，不需要依赖 dirty rect 控制帧率。

## APP 模式刷新

```c
if ( xgeXuiRefreshNeeded(&ui) ) {
	xgeBegin();
	xgeClear(xgeColorRGBA(18, 20, 26, 255));
	xgeXuiPaint(&ui);
	xgeEnd();
	xgeFlush();
	xgeXuiRefreshClear(&ui);
}
```

APP、编辑器、工具面板不一定需要每帧刷新。状态变化、输入、动画和异步资源完成时再请求刷新。

## 脏区

```c
xgeInvalidateRect(widget_rect);
```

脏区用于减少低功耗场景的绘制面积。第一版可以先把脏区作为刷新提示，不必承诺所有后端都做真实局部 swap。

## Clip

XUI 第一版的 ScrollView、ListView 和输入框都依赖 host clip：

```c
xgeClipSet(content_rect);
DrawChildren();
xgeClipClear();
```

自定义 host 必须正确实现 clip，否则滚动容器会把内容画出边界。clip 不再只是个别控件手动调用 host 的局部策略，而是 PaintContext 的基础能力：paint 会按 widget overflow、content rect、ScrollFrame viewport 和 overlay layer 维护 clip stack；嵌套 clip 会与父级 clip 相交，普通 Control 的文本和图片绘制会自动裁剪到自身 content rect，文字、边框和子树都必须遵守当前 clip。同父级绘制顺序使用 `layer > z > treeOrder`，与 hit test 和事件目标选择保持一致。`hitTestVisible` 会把 widget 子树整体排除出命中测试，`inputTransparent` 只让 widget 自身穿透输入，不禁用子节点命中。

## 常见错误

不要把 retained-mode XUI 当 IMGUI 每帧重建。应保持树稳定，只更新改变的状态。

不要在 APP 模式无条件 `xgeFlush`。这会让低功耗刷新失效。

不要在控件 paint 里做资源加载。资源应提前加载或走异步完成后请求刷新。

## 下一步

- 看完整例子用 [XUI 桥接范例](../case/xui-bridge.md)。
- 学习具体控件读 [XUI 控件入门](xui-controls-intro.md)。
