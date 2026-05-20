# XGE API 命名规则

本文档固化 XGE V2 公开 API、类型、常量和内部符号的命名规则。

## 总原则

- XGE 只提供 C API。
- 公开函数统一使用 `xge` 前缀。
- 公开常量和宏统一使用 `XGE_` 前缀。
- 公开类型使用 `xge_*_t` snake_case 形式。
- 内部函数使用 `__xge` 前缀。
- 不兼容 XGE V1 的旧命名，如 `DrawEx`、`SetCoodr` 等。

## 公开函数

公开函数使用 lower camel case：

```c
xgeInit
xgeTextureLoad
xgeSoundPlay
xgeXuiWidgetSetRect
```

模块名放在 `xge` 后面，动词放在模块名后面：

```c
xgeTextureLoad
xgeTextureFree
xgeSoundPlay
xgeSoundStop
xgeScenePush
xgeScenePop
```

这样阅读时可以先看到函数所属模块，再看到动作。

## Get / Set 规则

属性访问优先使用 `Get` / `Set`：

```c
xgeGetWidth
xgeGetHeight
xgeCameraGet
xgeCameraSet
xgeBlendGet
xgeBlendSet
xgeXuiWidgetGetRect
xgeXuiWidgetSetRect
```

允许少量高频全局函数省略模块名，例如：

```c
xgeGetWidth
xgeGetHeight
xgeGetDelta
xgeGetFPS
```

新 API 默认不再新增 `xgeWidth`、`xgeHeight` 这类短名。

## 生命周期函数

模块对象生命周期使用以下动词：

- `Init`：初始化用户传入的透明结构体。
- `Unit`：反初始化上下文类对象。
- `Create`：创建或初始化资源/对象。
- `Load`：从文件或资源加载。
- `LoadMemory`：从内存加载。
- `AddRef`：增加引用计数。
- `Free`：释放资源。
- `Clear`：清空状态或缓存。

示例：

```c
xgeXuiInit
xgeXuiUnit
xgeTextureCreateRGBA
xgeTextureLoad
xgeTextureLoadMemory
xgeTextureAddRef
xgeTextureFree
xgeTextureFallbackClear
```

资源释放采用 `xgeXxxFree`，不采用 `xgeFreeXxx`。

## 模块函数顺序

推荐顺序：

```text
xge + 模块名 + 子模块名 + 动词
```

示例：

```c
xgeXuiWidgetSetRect
xgeXuiScrollViewSetOffset
xgeXuiListViewSetItems
xgeShaderVariantSetInit
```

如果模块名已经足够明确，可以省略子模块：

```c
xgeAudioInit
xgeAudioUnit
xgeAudioSetVolume
```

## 类型命名

公开类型使用 snake_case：

```c
xge_texture_t
xge_font_t
xge_sound_t
xge_xui_context_t
xge_render_target_t
```

指针别名保留短形式：

```c
typedef struct xge_texture_t xge_texture_t;
typedef xge_texture_t* xge_texture;
```

透明对象尽量保持字段可读，避免为了读取普通字段强制走 getter。

## 结构体字段命名

结构体字段遵守 xrt 风格匈牙利前缀：

- `i`：整数、枚举、标志位。
- `f`：浮点数。
- `b`：布尔值。
- `s`：字符串。
- `p`：指针。
- `arr`：数组。
- `t`：值类型结构体。
- `obj`：较大的临时对象。
- `proc`：回调函数。

示例：

```c
int iWidth;
float fDelta;
int bInitialized;
const char* sTitle;
xge_texture pTexture;
xge_rect_t tRect;
xge_event_proc procEvent;
```

## 常量与宏

常量和宏使用大写 snake_case，并带 `XGE_` 前缀：

```c
XGE_OK
XGE_ERROR_INVALID_ARGUMENT
XGE_INIT_WINDOW
XGE_BLEND_ALPHA
XGE_EVENT_MOUSE_DOWN
XGE_XUI_LAYOUT_ROW
```

模块常量按模块分组：

```text
XGE_TEXTURE_*
XGE_RENDER_TARGET_*
XGE_PASS_*
XGE_AUDIO_*
XGE_XUI_*
```

位标志使用十六进制值，普通枚举值可使用连续整数。

## 回调命名

回调类型使用 `_proc` 后缀：

```c
xge_scene_proc
xge_scene_event_proc
xge_xui_event_proc
xge_xui_paint_proc
```

回调字段使用 `proc` 前缀或语义化生命周期名：

```c
procEvent
procPaint
onEnter
onLeave
onUpdate
onDraw
```

## 内部符号

内部函数使用两条前导下划线：

```c
__xgeSceneFrame
__xgeTextureShadowSet
__xgeXuiLayoutWidget
```

内部函数也要带模块名，避免单头文件组装后符号含义不清。

内部全局变量使用 `g_xge` 或 `g_xgeModuleName`：

```c
g_xge
g_xgeTextureRenderer
g_xgeShapeRenderer
```

内部无锁版本如未来需要，使用 `_NoLock` 后缀。

## 文件命名

模块源码使用 `src/xge_<module>.c`：

```text
src/xge_core.c
src/xge_texture.c
src/xge_render_target.c
src/xge_xui_core.c
```

示例目录使用小写 snake_case：

```text
examples/render_target/
examples/shader_material/
examples/xui_bridge/
```

构建脚本使用目标后缀：

```text
examples\render_target\build.bat
examples/xui_bridge/build.sh
```

## 新 API 检查清单

新增公开 API 前检查：

- 是否以 `xge` 开头。
- 是否体现模块名。
- 是否优先使用 `Get` / `Set` 表达属性访问。
- 是否使用 `xgeXxxFree` 释放资源。
- 是否返回 `int` 错误码或明确的值类型。
- 是否需要同步添加到 `xge.h`、实现、测试和 spec。
- 是否避免引入 V1 旧命名。
- 是否符合单头文件组装后的符号清晰度。
