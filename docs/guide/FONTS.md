# 系统字体与字体加载策略

XGE 不会在 `xgeInit()` 中隐式加载字体。纯游戏、品牌应用或已经内嵌字体的程序不会因此增加系统字体内存。需要平台默认字体时，应用可以直接调用 `xgeFontLoadSystem()`；使用 XUI 时，也可以由 context 的字体策略管理这次加载。

## XGE：显式加载

```c
xge_font_t font = {0};
xge_system_font_desc_t desc = {0};

desc.iSize = sizeof(desc);
desc.iRole = XGE_SYSTEM_FONT_UI;
desc.fPixelSize = 16.0f; /* 0 使用系统配置的 UI 字号 */

if (xgeFontLoadSystem(&font, &desc) != XGE_OK) {
    /* Web 或缺少平台字体服务时，回退到应用内嵌字体。 */
}
```

`xgeFontLoadSystem()` 与 `xgeFontLoad()` 的所有权一致：成功后由调用方执行 `xgeFontFree()`。系统字体不是全局单例，也不会在引擎初始化时预加载。

可选角色：

- `XGE_SYSTEM_FONT_UI`：按钮、菜单、表单等界面文字。
- `XGE_SYSTEM_FONT_DOCUMENT`：正文内容。
- `XGE_SYSTEM_FONT_MONOSPACE`：代码和终端。

## XUI：Context 策略

旧的 `xuiCreate()` 和 `xuiCreateOwned()` 默认使用 `XUI_SYSTEM_FONT_AUTO`。新程序可用扩展构造函数明确策略：

```c
xui_context_desc_t desc;
xui_context context = NULL;

xuiContextDescDefault(&desc);
desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;

if (xuiCreateEx(&desc, &context) != XUI_OK) {
    /* handle error */
}
```

| 策略 | 行为 | 适用场景 |
| --- | --- | --- |
| `XUI_SYSTEM_FONT_AUTO` | Proxy 绑定后仍不加载；首次调用 `xuiGetDefaultFont()` 或 `xuiEnsureDefaultFont()` 时加载。 | 普通桌面工具，默认推荐。 |
| `XUI_SYSTEM_FONT_DISABLED` | 永不请求系统字体；`xuiEnsureDefaultFont()` 返回 `XUI_ERROR_UNSUPPORTED`。 | 游戏、品牌字体、严格内存预算。 |
| `XUI_SYSTEM_FONT_EAGER` | Proxy 绑定时立即加载；失败会使绑定失败并回滚。 | 启动阶段需要尽早发现字体环境错误的应用。 |

`AUTO` 的关键内存语义是：如果应用在第一次文字使用前调用 `xuiSetDefaultFont()`，系统字体不会被加载。自动加载成功后，字体由 context 持有；应用随后替换默认字体不会造成泄漏，context 销毁时会释放自动字体。应用自己交给非 Owned context 的字体仍由应用释放。

```c
/* AUTO，但是在任何控件读取默认字体之前设置应用字体。 */
xui_context context = NULL;
xui_font appFont = NULL;
xui_proxy_t proxy = xuiProxyXge();

xuiCreate(&context);
xuiSetProxy(context, &proxy);
proxy.fontLoadFile(&proxy, &appFont, "res://fonts/ui.ttf", 16.0f,
                   XUI_FONT_FORMAT_TTF);
xuiSetDefaultFont(context, appFont);
```

自定义 Proxy 可以不实现 `fontLoadSystem`。此时 `AUTO` 首次解析返回空字体，显式 `xuiEnsureDefaultFont()` 返回 `XUI_ERROR_UNSUPPORTED`；`DISABLED` 不会调用该回调。`fontLoadSystem` 是追加的可选 Proxy 字段，旧 Proxy 结构前缀仍可绑定。

## 平台实现

| 平台 | 解析方式 | 说明 |
| --- | --- | --- |
| Windows | `SystemParametersInfoW` + DirectWrite | 使用系统消息字体、字号、字重和真实字体集合索引。 |
| Linux | Fontconfig（运行时加载） | 匹配 `sans-serif`/`monospace`，可使用 BCP-47 语言标签；缺少 Fontconfig 时返回 `UNSUPPORTED`。 |
| Apple | CoreText | 使用平台 UI/文档/等宽角色；字体集合通过 PostScript 名称定位正确 face。 |
| Android | NDK 系统字体匹配 API | API 29 及以上可用；较旧系统返回 `UNSUPPORTED`，应用应提供内嵌字体。 |
| Web | 不支持读取系统字体文件 | 返回 `UNSUPPORTED`；必须使用应用打包字体。 |

无论平台策略如何，应用都应准备一个内嵌字体作为 Web、精简 Linux 镜像、旧 Android 或损坏系统字体配置的回退路径。

## 范例与测试

- `examples/xge/xge_emoji`：直接调用 `xgeFontLoadSystem()`，并从系统字体创建多个字号实例。
- `examples/xui/xui_label`：使用默认 `AUTO`，首次需要文字时加载系统字体。
- `examples/xui/xui_input`：使用 `DISABLED`，只保留应用显式选择的字体。
- `tests/xge/build_system_font_test.bat`：验证平台字体能加载并返回有效度量。
- `tests/xui/build_system_font_policy_test.bat`：验证三种策略、惰性加载、失败回滚及资源释放。
