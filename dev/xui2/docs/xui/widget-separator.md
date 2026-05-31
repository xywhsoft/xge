# XUI Separator Widget

`Separator` 控件用于在 widget content rect 内绘制一条静态分隔线。XUI2 版本保持 XUI1 的核心行为：水平/垂直方向、颜色、厚度、对齐方式以及 solid/dot/dash/dash-dot 四种线型。区别是 XUI2 的绘制进入控件缓存，后续窗口刷新只合成缓存。

## 职责边界

- 控件只绘制分隔线，不负责背景、标题、图标、装饰图案或复杂矢量路径。
- 控件尺寸由布局系统决定，`thickness` 只表示线条厚度。
- 横向分隔线的厚度作用在高度上，纵向分隔线的厚度作用在宽度上。
- 非实线使用多个矩形段绘制，不要求 proxy 提供专用虚线接口。
- XSON 绑定暂缓，当前只提供 C API 和代码范例。

## 公开能力

| 能力 | API |
| --- | --- |
| 创建控件 | `xuiSeparatorCreate` |
| 设置颜色 | `xuiSeparatorSetColor` / `xuiSeparatorGetColor` |
| 设置厚度 | `xuiSeparatorSetThickness` / `xuiSeparatorGetThickness` |
| 设置方向 | `xuiSeparatorSetOrientation` / `xuiSeparatorGetOrientation` |
| 设置对齐 | `xuiSeparatorSetAlign` / `xuiSeparatorGetAlign` |
| 设置线型 | `xuiSeparatorSetLineStyle` / `xuiSeparatorGetLineStyle` |
| 查询最终线条矩形 | `xuiSeparatorGetLineRect` |

## 常量

| 常量 | 说明 |
| --- | --- |
| `XUI_SEPARATOR_HORIZONTAL` | 横向分隔线。 |
| `XUI_SEPARATOR_VERTICAL` | 纵向分隔线。 |
| `XUI_SEPARATOR_SOLID` | 实线。 |
| `XUI_SEPARATOR_DOT` | 点线。 |
| `XUI_SEPARATOR_DASH` | 虚线。 |
| `XUI_SEPARATOR_DASH_DOT` | 点划线。 |

`align` 使用通用的 `XUI_ALIGN_START`、`XUI_ALIGN_CENTER`、`XUI_ALIGN_END`。横向时对应上/中/下，纵向时对应左/中/右。

## 缓存与失效

`Separator` 使用 `XUI_CACHE_POLICY_SELF` 和透明清屏。以下修改会触发失效：

- `color`、`align`、`line_style`：缓存和渲染失效。
- `thickness`、`orientation`：布局、缓存和渲染失效。
- widget 尺寸或 padding 变化：通过布局系统更新最终线条矩形。

线条矩形会在缓存绘制前做整数像素对齐，避免 1px 线条落在半像素坐标后被吞掉或变成 2px。

## 样式属性

| 属性 | 类型 | 影响 |
| --- | --- | --- |
| `separator.color` | color | 线条颜色。 |
| `separator.thickness` | float | 线条厚度。 |
| `separator.orientation` | int | 横向或纵向。 |
| `separator.align` | int | start/center/end。 |
| `separator.line_style` | int | solid/dot/dash/dash-dot。 |

## 验证

- 单元测试：`test_xui\build_separator_test.bat`
- 代码范例：`examples\xui_separator\build.bat`
- 自动运行：`build\xui_separator.exe --frames 360`
