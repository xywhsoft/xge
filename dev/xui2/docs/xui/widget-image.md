# XUI Image Widget

`Image` 控件用于把一个 proxy `surface` 作为控件内容渲染。XUI2 版本保持 XUI1 的核心语义，但绘制路径适配新的缓存体系：控件只在自身缓存失效时把图像绘制到缓存，后续界面刷新由合成层搬运缓存完成。

## 职责边界

- `Image` 不拥有 `surface`，调用方或资源管理层负责加载和释放。
- `Image` 不负责图片解码；从文件、内存或 RGBA 数据创建 `surface` 属于 proxy/resource 层。
- `Image` 不参与文本、九宫格、图集切片或动画序列逻辑；这些能力后续应由专门控件或资源层组合实现。
- XSON 绑定暂缓，当前只提供 C API 和代码范例。

## 公开能力

| 能力 | API |
| --- | --- |
| 创建控件 | `xuiImageCreate` |
| 设置 surface | `xuiImageSetSurface` / `xuiImageGetSurface` |
| 设置源矩形 | `xuiImageSetSource` / `xuiImageSetSourceRect` / `xuiImageClearSource` / `xuiImageGetSource` |
| 设置颜色调制 | `xuiImageSetColor` / `xuiImageSetTint` / `xuiImageGetColor` |
| 设置模式 | `xuiImageSetMode` / `xuiImageGetMode` |
| 设置对齐 | `xuiImageSetAlign` / `xuiImageGetAlign` |
| 设置自定义目标矩形 | `xuiImageSetCustomRect` / `xuiImageGetCustomRect` |
| 查询最终绘制矩形 | `xuiImageGetDrawRect` |

## Image Mode

| 模式 | 行为 |
| --- | --- |
| `XUI_IMAGE_NATURAL` | 使用源图尺寸，按 `align_x` / `align_y` 在内容区域内对齐。 |
| `XUI_IMAGE_STRETCH` | 拉伸到完整内容区域，忽略对齐。 |
| `XUI_IMAGE_CONTAIN` / `XUI_IMAGE_FIT` | 等比缩放到内容区域内完整可见，按对齐放置。 |
| `XUI_IMAGE_COVER` | 等比缩放直到覆盖内容区域，超出部分由控件缓存自然裁剪。 |
| `XUI_IMAGE_SCALE_DOWN` | 图像大于内容区域时按 contain 缩小，否则按 natural 显示。 |
| `XUI_IMAGE_CUSTOM` | 使用相对内容区域的自定义目标矩形。 |

源矩形宽高为 0 时表示使用完整 `surface`。控件测量尺寸来自源矩形或完整 `surface` 尺寸，显示模式只影响最终绘制矩形，不改变固有尺寸。

## 缓存与失效

`Image` 使用 `XUI_CACHE_POLICY_SELF` 和 `XUI_CACHE_CLEAR_ON_UPDATE`。以下修改会触发缓存失效：

- `surface`、源矩形、颜色调制、模式、对齐、自定义目标矩形。
- 影响内容区域的 widget 布局或 padding 变化。
- 样式属性变化：`image.color`、`image.tint`、`image.mode`、`image.align_x`、`image.align_y`。

当 `surface` 为空时控件缓存保持透明，不绘制占位图。这个行为和资源失败态解耦，调用方可以自行决定是否叠加错误提示或默认图。

## 验证

- 单元测试：`test_xui\build_image_test.bat`
- 代码范例：`examples\xui_image\build.bat`
- 自动运行：`build\xui_image.exe --frames 360`
