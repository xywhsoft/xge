# XUI Slider

Slider 是基础数值选择控件，职责是把一个连续或离散数值映射到可拖动的轨道和滑块。它不负责文本展示、不内建复杂刻度，也不承担滚动容器语义；需要显示数值时应与 Label 等控件组合。

## 设计原则

- 使用 Widget 负责启用/禁用、焦点、背景、边框、布局和事件分发，Slider 只负责轨道、已选区域和滑块的绘制。
- 默认关闭 Widget 背景和边框，避免基础控件出现多余容器感；用户需要外框时可以直接设置 Widget 样式。
- 支持水平和垂直方向。水平从左到右递增，垂直从下到上递增。
- 支持 step 和 pageStep。鼠标拖动、键盘方向键、PageUp/PageDown、Home/End 都走同一套值域约束。
- 提供轨道颜色、填充颜色、滑块颜色、滑块边框色、禁用色、焦点色，满足应用 UI 和游戏 UI 的基础定制。
- 内置渲染缓存。默认 auto 模式会缓存静态外观，数值、状态、尺寸或样式变化时自动失效。

## API

```c
int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiSliderUnit(xge_xui_slider pSlider);
void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser);
void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax);
void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue);
float xgeXuiSliderGetValue(xge_xui_slider pSlider);
void xgeXuiSliderSetOrientation(xge_xui_slider pSlider, int iOrientation);
void xgeXuiSliderSetStep(xge_xui_slider pSlider, float fStep, float fPageStep);
void xgeXuiSliderSetMetrics(xge_xui_slider pSlider, float fTrackSize, float fKnobSize, float fTrackRadius, float fKnobRadius);
void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);
void xgeXuiSliderSetKnobBorderColor(xge_xui_slider pSlider, uint32_t iColor);
void xgeXuiSliderSetCacheMode(xge_xui_slider pSlider, int iMode);
```

`iOrientation` 使用 `XGE_XUI_SEPARATOR_HORIZONTAL` 或 `XGE_XUI_SEPARATOR_VERTICAL`。

`xgeXuiSliderSetStep` 中 `fStep <= 0` 表示不吸附，`fPageStep <= 0` 时键盘 Page 步长退化为值域的 10%。

`xgeXuiSliderSetMetrics` 中半径传 `-1` 表示自动取半高/半宽形成圆角。

缓存模式使用 `XGE_XUI_CACHE_AUTO`、`XGE_XUI_CACHE_OFF`、`XGE_XUI_CACHE_FORCE`。

## XSON

```json
{
  "type": "slider",
  "min": 0,
  "max": 100,
  "value": 50,
  "orientation": "horizontal",
  "step": 5,
  "pageStep": 20,
  "trackSize": 4,
  "knobSize": 14,
  "trackRadius": -1,
  "knobRadius": -1,
  "trackColor": "#D7E8F4FF",
  "fillColor": "#2E7CD6FF",
  "knobColor": "#FFFFFFFF",
  "knobBorderColor": "#9AAEC6FF",
  "disabledColor": "#B4BAC4FF",
  "cacheMode": "auto"
}
```

`orientation` 可为 `horizontal` 或 `vertical`。`cacheMode` 可为 `auto`、`off`、`force`。

## 测试覆盖

专属范例：

- `examples/xui_slider`
- `examples/xui_slider_xson`

范例覆盖默认水平 Slider、禁用状态、自定义配色、自定义尺寸、cache off、cache force、垂直 Slider、垂直禁用状态，以及 XSON 声明式加载。
