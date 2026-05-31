# XUI Progress Widget

`Progress` 控件用于展示一个数值在范围内的完成比例。XUI2 版本对齐 XUI1 的核心能力：范围和值、填充方向、模板文字、track/fill 颜色、track/fill nine-patch，以及 fill patch 的 stretch/reveal 模式。绘制进入控件自身缓存，后续刷新由合成层搬运缓存完成。

## 职责边界

- 控件只负责进度条视觉，不负责异步任务、计时器或数据绑定。
- `surface` 和 `font` 不由控件拥有，调用方或资源管理层负责释放。
- 文本模板只接收一个百分比参数，和 XUI1 一样用当前进度百分比格式化。
- XSON 绑定暂缓，当前只提供 C API 和代码范例。

## 公开能力

| 能力 | API |
| --- | --- |
| 创建控件 | `xuiProgressCreate` |
| 范围和值 | `xuiProgressSetRange` / `xuiProgressGetRange` / `xuiProgressSetValue` / `xuiProgressGetValue` / `xuiProgressGetRate` |
| 文本模板 | `xuiProgressSetText` / `xuiProgressSetTextTemplate` / `xuiProgressGetTextTemplate` / `xuiProgressGetDisplayText` |
| 字体和文字色 | `xuiProgressSetFont` / `xuiProgressSetTextColor` / `xuiProgressSetFillTextColor` / `xuiProgressSetTextFlags` |
| 轨道和填充色 | `xuiProgressSetColors` / `xuiProgressGetTrackColor` / `xuiProgressGetFillColor` |
| 填充方向 | `xuiProgressSetFillDirection` / `xuiProgressGetFillDirection` |
| 九宫格 | `xuiProgressSetTrackPatch` / `xuiProgressSetFillPatch` / `xuiProgressSetFillPatchMode` |
| 几何查询 | `xuiProgressGetFillRect` |

## 常量

| 常量 | 说明 |
| --- | --- |
| `XUI_PROGRESS_LEFT_TO_RIGHT` | 从左向右填充。 |
| `XUI_PROGRESS_RIGHT_TO_LEFT` | 从右向左填充。 |
| `XUI_PROGRESS_BOTTOM_TO_TOP` | 从下向上填充。 |
| `XUI_PROGRESS_TOP_TO_BOTTOM` | 从上向下填充。 |
| `XUI_PROGRESS_FILL_STRETCH` | fill patch 拉伸到当前填充矩形。 |
| `XUI_PROGRESS_FILL_REVEAL` | 简单 fill patch 按进度裁剪源图；带 slice 的九宫格仍按当前填充矩形绘制。 |

## 缓存与失效

`Progress` 使用 `XUI_CACHE_POLICY_SELF` 和透明清屏。以下修改会触发失效：

- value/range、文本模板、字体、颜色、文本 flags：缓存和渲染失效。
- fill direction：布局、缓存和渲染失效。
- track/fill patch、fill patch mode：缓存和渲染失效。
- widget 尺寸或 padding 变化：通过布局系统更新 content rect 和 fill rect。

填充矩形按 content rect 的累计边界做整数像素对齐，避免 0.5 坐标导致边缘线被吞掉或变粗。

## 样式属性

| 属性 | 类型 | 影响 |
| --- | --- | --- |
| `progress.track_color` | color | 轨道颜色。 |
| `progress.fill_color` | color | 填充颜色。 |
| `progress.text_color` | color | 普通文字颜色。 |
| `progress.fill_text_color` | color | 填充区域内的文字颜色。 |
| `progress.text_flags` | int | 文本对齐与裁剪 flags。 |
| `progress.fill_direction` | int | 填充方向。 |
| `progress.fill_patch_mode` | int | fill patch stretch/reveal 模式。 |
| `text.color` | color | 可继承的默认文字颜色。 |
| `text.flags` | int | 可继承的默认文本 flags。 |
| `font.name` | string | 可继承的字体名。 |

## 验证

- 单元测试：`test_xui\build_progress_test.bat`
- 代码范例：`examples\xui_progress\build.bat`
- 自动运行：`build\xui_progress.exe --frames 360`
