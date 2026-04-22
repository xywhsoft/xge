# XUI 表单

本案例展示用 XUI 组合一个设置表单。

[返回范例解析](README.md) | [XUI 控件教程](../guide/xui-controls-intro.md) | [XUI API](../api/xui.md)

## 结构

```text
panel
  label + input
  toggle
  slider
  button row
```

## 代码骨架

```c
xgeXuiWidgetSetLayout(panel, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetPaddingPx(panel, 16.0f, 16.0f, 16.0f, 16.0f);

xgeXuiInputInit(&name_input, &ui, &name_widget, &font);
xgeXuiToggleInit(&fullscreen_toggle, &ui, &fullscreen_widget);
xgeXuiSliderInit(&volume_slider, &ui, &volume_widget);
xgeXuiButtonInit(&apply_button, &ui, &apply_widget);
```

## 提交

```c
static void OnApply(void* pUser)
{
	const char* name;

	name = xgeXuiInputGetText(&name_input);
	SaveSettings(name, xgeXuiToggleGetChecked(&fullscreen_toggle), xgeXuiSliderGetValue(&volume_slider));
}
```

## 常见失败原因

表单高度不对：子控件没有设置 size 或 measure。

输入法候选框错位：没有把 Input candidate rect 传给宿主。
