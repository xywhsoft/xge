# XUI 声明式界面与样式设计

本文档定义 XUI 面向 APP 界面和游戏内嵌界面的下一阶段设计方向。目标是在不引入 HTML/CSS 级复杂度的前提下，让 XUI 支持通过 XSON 描述快速构建 retained UI，并利用 XValue 的父表继承能力实现轻量、共享、可统一管理的样式系统。

> 2026-05-07 口径更新：XSON 类型、children、overflow、z、scroll、IME、基础绘制等字段以 `XUI Widget V2基础设计.md` 与 `XUI Widget V2基础SPEC.md` 为准。本文保留第一版 loader / XSON 历史设计，但涉及 Widget 基础行为的部分必须按 Widget V2 同步。

> 2026-05-09 口径更新：Widget 阶段 E2 已完成，XSON 事件绑定必须建立在 Widget 基础语义事件之上。当前 `onClick`、Mouse、Key、TextInput、HotKey、Command 等已实现绑定维持现状；Drag 等字段作为声明式层后续任务推进，仍禁止脚本和热路径字符串查找。

## 设计目标

- 支持 APP 页面、工具界面、游戏 HUD、弹窗、菜单、设置页和列表类界面。
- 保持 retained widget tree，不每帧重建 UI。
- 使用 XRT/XValue/XSON 作为描述和数据基础，不新增独立解析器。
- 样式通过 XValue 字典和列表父表机制共享数据，避免复制完整样式字段。
- 布局功能完整但克制，补强 Flex-lite，而不是实现浏览器布局引擎。
- XSON 只描述结构、样式、资源引用和事件绑定名，不内置脚本语言。
- 复杂行为、业务逻辑和数据源仍由 C 代码提供。
- 加载后运行路径必须可缓存，避免每次 layout/paint 都做字符串查找。

## 非目标

- 不实现 CSS selector、cascade、media query。
- 不实现 DOM、JS、浏览器事件模型。
- 不实现完整 Flexbox 或 CSS Grid。
- 不把业务控件都固化为 XUI 核心控件。
- 不为大列表创建海量 widget。
- 不要求依赖 UI 设计器。

## 总体架构

```text
XSON resource
  -> XRT resource provider
  -> XValue tree
  -> resolve imports
  -> attach style parent tables
  -> build retained XUI widget tree
  -> cache resolved layout/visual fields
  -> update model/event bindings
```

XValue 负责：

- XSON 解析后的数据树。
- 字典和列表父表链。
- 样式字段共享。
- 后续主题和样式统一管理。

XUI 负责：

- 把 XValue 节点转换成 widget tree。
- 将常用样式字段解析到运行期 cache。
- 维护 id/name 索引。
- 管理事件绑定、数据绑定和资源生命周期。
- 根据 style/theme/model 版本使 cache 失效。

## XSON 文件结构

建议使用 `.xui.xson` 或 `.xui.json` 后缀。顶层结构：

```json
{
  "xui": 1,
  "name": "settings_page",
  "theme": "app_dark",
  "imports": [
    "res://ui/styles/common.xui.xson"
  ],
  "styles": {
    "page": {
      "layout": "column",
      "padding": 16,
      "gap": 12,
      "bg": "@panel"
    },
    "row": {
      "@parent": "page",
      "layout": "row",
      "padding": 0,
      "alignY": "center"
    }
  },
  "tree": {
    "type": "panel",
    "id": "root",
    "style": "page",
    "children": []
  }
}
```

字段说明：

- `xui`：格式版本。
- `name`：页面或组件名。
- `theme`：主题名，可由 C 侧注册或资源加载。
- `imports`：导入通用 style、token 或组件模板。
- `styles`：命名样式表，支持 XValue 父表继承。
- `tree`：根 widget 描述。

## Imports 路径规则

第一版 `imports` 使用资源 URI 语义，统一通过 XGE resource provider 加载：

- `res://...`、`file://...` 等带 scheme 的 URI 原样传给 `xgeResourceLoad`。
- 不带 scheme 的相对路径，以当前 XSON 文件所在目录为 base 解析。
- 解析后的路径需要规范化 `.` 和 `..`，避免同一资源被不同相对路径重复加载。
- imports 按数组顺序加载，后加载资源可以覆盖先加载资源中的同名 style/token/template。
- 被当前文件本地声明的 `styles`、tokens、templates 优先级最高。
- imports 只导入共享声明，不导入被引用文件的 `tree`。
- 必须检测 import 循环，并在错误信息中包含资源 URI 链。
- 同一个 page load 过程中，同一规范化 URI 只加载一次。

第一版合并范围：

- `styles`：按名称合并。
- `tokens`：按名称合并。
- `templates`：按名称合并。
- `theme`：当前文件显式 `theme` 优先；import 文件只提供可引用的 theme/token 声明。

## Widget 描述

每个节点至少包含 `type`，`type` 同时决定 Widget role：

```json
{
  "type": "button",
  "id": "save",
  "text": "Save",
  "w": 120,
  "h": 36,
  "onClick": "save_settings"
}
```

Widget V2 后推荐支持的类型分类：

- Container：`panel`、`row`、`column`、`stack`、`grid`、`dock`。
- Viewport：`scrollView`、`treeView`、`tableView`、`propertyGrid`。
- Virtual viewport：`virtualized ListView`，只允许 `itemTemplate`，不允许普通 `children`。
- Control：`label`、`button`、`input`、`numericInput`、`colorPicker`、`datePicker`、`image`、`separator`、`checkbox`、`radio`、`Toggle`、`slider`、`progress`、`comboBox`、`tabs`、`toolbar`、`statusBar`、`accordion`。
- Overlay：`popup`、`menu`、`Window`、`MsgBox`、`toast`、`tooltip`、`window`，通过 `layer`、owner 或 portal 进入 overlay root；`tooltip` 同时保留任意 widget 通用属性口径。
- Workbench composite：`dockLayout`。它是新范式工作台复合控件，使用 `dockWindows` 声明 dockwindow 列表、region、side、state 和 client children；普通 `children` 只允许出现在单个 dockwindow 的 client 内，不直接挂在 dockLayout 根节点下。

children 规则：

- Container 默认允许 `children` 并参与子布局。
- Viewport 允许 `children`，但必须在 content rect 内滚动和裁剪。
- Virtual viewport 不允许普通 `children`，只允许 `itemTemplate` 描述 slot 内容。
- Control 默认不允许用户 `children`；需要图标、文本、前后缀等内容时用控件专有 slot 字段，不把它退化成通用容器。
- Overlay 的 children 规则由具体 overlay 控件决定，但它的绘制层级必须走 Widget V2 layer/zIndex。
- Workbench composite 的 children 规则由控件模型决定；`dockLayout` 使用 `dockWindows` 管理窗口集合，dockwindow client 内的声明式 children 才参与普通 widget tree 生命周期。

`row`、`column`、`stack`、`grid`、`dock` 可以作为 `type` 简写：

```json
{ "type": "row", "children": [] }
```

等价于：

```json
{ "type": "panel", "layout": "row", "children": [] }
```

## 尺寸与边距

尺寸支持多种写法：

```json
{ "w": 120, "h": 32 }
{ "w": "grow", "h": "content" }
{ "w": "grow:2", "h": "48dip" }
{ "w": "50%" }
{ "w": { "unit": "percent", "value": 50 } }
```

规则：

- 数字默认是 px。
- `"content"` 映射到 content size。
- `"grow"` 等价于 grow 1。
- `"grow:N"` 表示 grow 权重。
- `"*"` / `"N*"` 是 grow 1 / grow N 的兼容写法，推荐新文档和示例继续使用 `"grow"` / `"grow:N"`。
- `"N%"` 表示 percent。
- `"Ndip"` 表示 DIP。

边距和内边距：

```json
{ "padding": 12 }
{ "margin": [4, 8] }
{ "padding": [8, 12, 8, 12] }
```

数组约定：

- `[x, y]` 表示 left/right = x，top/bottom = y。
- `[left, top, right, bottom]` 表示四边。

Grid 字段：

```json
{ "type": "grid", "columns": 3, "rowHeight": 40, "columnGap": 8, "rowGap": 8 }
{ "type": "button", "columnSpan": 2 }
```

第一版 Grid 只提供固定列、固定或方格行高、列间距、行间距和 `columnSpan`。`columnSpan` 是子元素字段，只影响该子元素在父 grid 中横跨的列数；不支持 row span、命名区域或 CSS Grid 式占用矩阵。

Dock 字段：

```json
{
  "type": "dock",
  "children": [
    { "type": "panel", "dock": "top", "height": 48 },
    { "type": "panel", "dock": "left", "width": 240 },
    { "type": "panel", "dock": "fill", "width": "grow", "height": "grow" }
  ]
}
```

第一版 DockLayout 用于 APP 壳、工具栏、侧栏、状态栏和内容区这类固定边界布局。子元素 `dock` 支持 `top`、`bottom`、`left`、`right`、`fill`、`center`，按声明顺序扣减剩余 rect，`fill/center` 使用当前剩余区域。不支持约束求解、命名区域或多轮回流。

SafeArea 字段：

```json
{
  "xui": 1,
  "safeArea": [12, 24, 12, 0],
  "tree": { "type": "dock", "children": [] }
}
```

第一版 SafeArea 不新增容器 widget。它沿用已有 root padding 语义：page 顶层 `safeArea` 在加载时应用到当前 XUI context root，使页面 root 落在 root content rect 内；overlay root 仍保持全窗口，便于弹窗、菜单和调试层覆盖。`safeArea` 支持数字、二元数组、四元数组和 spacing token。Page 卸载时恢复加载前的 root padding，避免声明式页面污染后续手写 UI。

ScrollView 字段：

```json
{
  "type": "scrollView",
  "width": 320,
  "height": 240,
  "contentSize": [320, 800],
  "offset": { "x": 0, "y": 120 },
  "wheelAxis": "vertical",
  "dragMode": "none",
  "scrollbarDrag": true,
  "nestedScroll": "passEdge",
  "children": []
}
```

ScrollView 已迁移到 ScrollViewBase，代码类型上 `xge_xui_scroll_view_t` 直接等同于 `xge_xui_scroll_view_base_t`。容器默认开启 content clip；`contentSize`/`contentWidth`/`contentHeight` 描述虚拟内容尺寸，`offset`/`scrollOffset`/`contentOffset` 或 `scrollX`/`scrollY` 描述初始滚动位置，颜色字段支持 `backgroundColor`、`barColor`、`thumbColor`。布局完成后，ScrollView 只对子 widget 的最终 rect 做滚动偏移，保持 layout 本身单次遍历；hit test 和滚轮只在 content rect 内生效，padding 区域不会触发滚动。

ScrollViewBase 字段：

- `wheelAxis`：`vertical`/`y`、`horizontal`/`x`、`both`，默认 `vertical`。
- `dragMode`：`none` 默认关闭；`content`、`pan`、`drag` 会开启内容拖拽。左键拖拽必须显式启用，避免干扰地图编辑器、画布编辑器等控件。
- `scrollbarDrag`：是否允许拖动滚动条 thumb，默认 `true`。
- `nestedScroll`：`consume` 或 `passEdge`；`passEdge` 表示当前滚动视图无法继续滚动时事件上浮给父级。
- `overflow`：普通控件可用 `visible`、`hidden`、`clip`、`scroll`，但声明式 `scrollView` 是显式 Viewport，不靠普通 overflow 隐式创建复杂滚动容器。

virtualized ListView 要求固定 `itemHeight`，通过 `xgeXuiListViewSetAdapter(count, create, bind, user)` 获取数据数量、创建可复用 slot widget，并在 slot 对应 index 变化时调用 bind。列表根据 content rect 和 `scrollY` 计算可见范围，只创建可见范围需要的 slot；滚动时复用已有 slot 并更新 rect，不为不可见 item 建立完整 widget 子树。选择回调使用独立 user 指针，不覆盖 adapter user。

XSON `virtualized ListView` 第一版支持 `itemCount`、`itemHeight`、`scrollY`、`backgroundColor`、`barColor`、`thumbColor` 和 `itemTemplate`。`itemTemplate` 可以是内联对象，也可以是顶层 `templates` 中的名称。slot 创建时复用 page widget builder 构建模板子树，因此模板内可以使用已有控件、样式和 token；数据字段替换留给后续 model/binding 阶段处理。示例：

```json
{
  "templates": {
    "rowItem": {
      "type": "row",
      "height": 44,
      "children": [
        { "type": "label", "name": "title", "text": "Item" }
      ]
    }
  },
  "tree": {
    "type": "virtualized ListView",
    "itemCount": 1000,
    "itemHeight": 44,
    "itemTemplate": "rowItem"
  }
}
```

## 样式继承

XUI 样式继承基于 XValue 字典和列表的父表机制。底层 AVLTree 可以设置父表；查找字段时，如果当前表不存在该 key，则继续查父表，直到根父表。

当前 XValue 父表机制没有所有权安全保护。如果父表先于子表销毁，子表后续查询父表字段时可能触发悬空指针访问。第一版将其记录为已知缺陷，并通过 page/style arena 统一生命周期规避：同一页面加载出的 styles、theme tables、widget inline tables 必须由 page 统一持有，卸载时按整体释放，禁止单独销毁父表。

当前实现约束：

- 命名 style 的 `@parent` 循环会导致 page load 失败，错误信息包含 `style parent cycle`。
- `xvoTableSetParent` 只保存父表指针，不增加引用计数；参与继承的 imported styles、merged styles 和当前 page styles 必须由 `xge_xui_page_t` 持有。
- 第一版 widget inline 覆盖采用显式 overlay lookup，不把 widget node 自身设置为 named style 的 XValue 父表，避免 `children` 等结构字段被样式继承。
- Theme style 和 engine default style 暂不接入 XValue 父表链；当前继续使用 widget 默认值与 C 侧 `xgeXuiStyleFromTheme`，后续在 Theme Token 与 Style Cache 阶段统一处理。
- Page load 会为 XSON 创建出的 widget 建立固定容量 id/name 索引；索引溢出时 `xgeXuiPageFind` 回退递归查找。加载失败时释放已创建 document/import/widget/resource，并保留错误字符串。

推荐继承链：

```text
widget inline table
  -> named style table
    -> parent style table
      -> theme style table
        -> engine default table
```

示例：

```json
{
  "styles": {
    "panel": {
      "layout": "column",
      "padding": 12,
      "gap": 8,
      "bg": "@panel"
    },
    "Window": {
      "@parent": "panel",
      "padding": 16,
      "radius": 8
    },
    "toolbar": {
      "@parent": "panel",
      "layout": "row",
      "h": 40,
      "alignY": "center"
    }
  }
}
```

设计要求：

- 样式字段不做完整复制，优先通过父表链共享。
- Widget inline 字段优先级最高。
- 命名 style 可继承另一个命名 style。
- 命名 style 的父样式字段固定为 `@parent`。
- Theme style 可作为命名 style 的上级默认表。
- Engine default 是最终兜底。
- 必须检测父表循环。
- 必须保证同一继承链上的 XValue 表生命周期一致，由 page/style arena 统一释放。
- `children` 不参与样式继承，避免 UI 树结构语义混乱。
- 布局/绘制高频字段必须解析到 cache，不在热路径重复字符串查找。

## 样式 Cache 与版本

为了兼顾共享和性能，widget 运行期需要轻量 cache：

```text
styleValue pointer
styleVersion
themeVersion
resolvedLayoutStyle
resolvedVisualStyle
```

失效规则：

- 修改 widget inline 字段，标记该 widget style cache dirty。
- 修改命名 style 父表，引用该 style 的页面或 widget 标记 style cache dirty。
- 修改 theme token，依赖该 token 的页面标记 style cache dirty。
- layout 相关字段变化时标记 layout dirty。
- paint-only 字段变化时只标记 paint dirty。

第一版可以先使用 page 级版本号，后续再细化到 style/token 级依赖。

当前轻量实现：

- `xge_xui_style_t` 是第一版 layout/visual style cache，XSON loader 在 page load 阶段把高频字段解析进去，layout/paint 热路径不再访问 XValue 字符串表。
- `xge_xui_page_t.iStyleVersion` 表示 page 当前 style version，第一版 page load 成功后为 `1`。
- `xge_xui_context_t.iThemeVersion` 在 context 初始化时为 `1`，每次 `xgeXuiSetTheme` 成功设置后递增。
- `XGE_XUI_WIDGET_DIRTY_STYLE` 表示 widget style cache 需要重算；`xgeXuiWidgetMarkStyle` 会设置该标记、递增 widget style version，并同时触发布局和绘制 dirty。

## Theme Token

样式值可以引用 token：

```json
{
  "bg": "@panel",
  "color": "@text",
  "gap": "@space_m",
  "font": "@body"
}
```

Token 由 C 侧注册或 XSON theme 文件提供。字体资源建议由 C 侧管理生命周期，XSON 中只引用字体 token。

当前轻量实现：

- `tokens.colors`：颜色 token，第一版用于 `background`、`borderColor`、`focusRingColor`、`disabledOverlay`、`debugOutlineColor` 等颜色字段。
- `tokens.spacing`：间距 token，第一版用于 `width/height/min/max/gap/margin/padding/radius/borderWidth/focusRingWidth/debugOutlineWidth/anchor/grid gap` 等数值或尺寸字段。
- `tokens.fonts`：字体 token 命名空间，只引用 C 侧持有的字体。
- `tokens.textures`：纹理 token 命名空间，只引用 C 侧持有的 texture；XSON `image.src` 加载的 texture 由 page 持有。
- 样式值可写 `@colors.name`、`@spacing.name`、`@fonts.name`、`@textures.name`，也可写未限定的 `@name`，未限定查找顺序为顶层 token、colors、spacing、fonts、textures。
- token 缺失是加载错误，不静默 fallback，错误信息包含字段路径，便于定位资源问题。
- C 侧可通过 `xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont`、`xgeXuiTokenSetTexture` 注册 context 级 token。XSON/import token 优先，context token 作为 fallback。
- C 侧 token 注册成功后递增 `themeVersion`，并标记 root style/layout/paint dirty。已加载 page 通过 `xgeXuiPageSyncStyle` 显式同步；同步时只在 page 记录的 theme version 落后时调用 `xgeXuiPageRefreshStyle`，重新把 XSON layout/visual 字段解析到 `xge_xui_style_t` cache。
- XSON 字段解析只发生在 page load 或 page style refresh/sync 阶段，不进入 layout/paint 热路径。布局和绘制阶段只读取已解析的 `xge_xui_style_t` 与控件状态字段。

## 基础控件 XSON 化

当前第一版支持结构型控件：

- `panel` / `absolute`：普通容器，映射到 absolute layout。
- `row`：横向布局容器。
- `column`：纵向布局容器。
- `stack`：层叠布局容器。
- `grid`：网格布局容器。
- `dock`：停靠布局容器，子项通过 `dock` 字段按声明顺序占用 top/bottom/left/right/fill 区域。

当前第一版支持轻量状态控件：

- `button`：绑定 `xge_xui_button_t`，支持 `text`、`font`、`textColor`、`textAlign`、`textVAlign`、`color`/`background`、`hoverColor`、`activeColor`、`focusColor`、`disabledColor` 和 `onClick`。`onClick` 通过 `xgeXuiButtonSetClick` 接入按钮自身事件过程，不覆盖控件 `pUser`。
- `image`：绑定 `xge_xui_image_t`，支持 `texture`、`src`、`source`/`srcRect`、`color`/`tint`、`mode`。`texture` 引用 C 侧注册的 texture token；`src` 通过 `xgeTextureLoad` 同步加载 page 自有 texture，并在 page unload 时释放。
- `input`：绑定 `xge_xui_input_t`，支持 `text`/`value`、`placeholder`、`font`、`textColor`、`background`/`backgroundColor`、`focusColor`、`cursorColor`、`placeholderColor`、`selectionColor`、`disabledTextColor`、`disabledBackgroundColor`、`password`、`readonly`、`disabled`、`selection`。文本缓冲、默认菜单和 IME 状态由 input 控件持有，page unload 时调用 `xgeXuiInputUnit`。
- 搜索输入不再提供独立 core 类型；基础搜索框使用 `input` 加 leading search decoration 和 trailing clear decoration 表达，带 suggestion popup 的搜索框归 ext/组合控件。
- `colorPicker`：绑定 `xge_xui_color_picker_t`，支持 `value`/`color`/`hex`、`r`/`g`/`b`/`a` 与 `red`/`green`/`blue`/`alpha`、`palette`、`font`、`backgroundColor`/`background`、`panelColor`、`borderColor`、`textColor`、`accentColor`、`fieldColor`、`hoverColor`。`color` 表示当前颜色值，文字颜色必须使用 `textColor`，避免与值字段冲突。`onChange` 当前严格拒绝，后续随统一 value/model 绑定落地。
- `datePicker`：绑定 `xge_xui_date_picker_t`，支持 `value`/`date`、`year`/`month`/`day`、`min`/`max`、`viewYear`/`viewMonth`、`font`、`backgroundColor`、`panelColor`、`headerColor`、`gridColor`、`textColor`、`selectedColor`、`mutedTextColor`、`disabledTextColor`、`hoverColor`、`focusColor`。`onChange` 当前严格拒绝，后续随统一 value/model 绑定落地。
- `label`：绑定 `xge_xui_label_t`，支持 `text`、`font`、`textColor`/`color`、`textAlign`、`textVAlign`。`font` 引用 C 侧注册的 font token，例如 `"@fonts.body"`。
- `separator`：绑定 `xge_xui_separator_t`，支持 `orientation`、`thickness`、`color`/`background`。

未知 `type` 或非字符串 `type` 是加载错误，不静默降级为 `panel`。`input.onChange/onSubmit` 当前严格拒绝，不提供空占位，后续随阶段 K 的 model/value 绑定统一落地。

状态控件由 `xge_xui_page_t` 内固定容量 control arena 持有，不为每个 XSON 控件额外分配独立 holder。`xgeXuiPageUnload` 会先对 widget tree 中已绑定的状态控件调用对应 `Unit`，再释放 widget tree；page load 失败回滚同样会清理已创建状态控件，避免控件持有已释放 widget。

## 事件绑定

XSON 内只记录事件名：

```json
{
  "type": "button",
  "id": "ok",
  "text": "OK",
  "onClick": "dialog_ok"
}
```

C 侧注册事件：

```c
xgeXuiBinderSetClick(&tBinder, "dialog_ok", OnDialogOk, pUser);
xgeXuiBinderSetEvent(&tBinder, "preview_move", OnPreviewEvent, pUser);
```

禁止在 XSON 内嵌脚本。复杂逻辑必须回到 C。

当前轻量实现：

- `onClick` 字段引用 C 侧 `xgeXuiBinderSetClick` 注册的事件名。
- `onMouseEnter`、`onMouseLeave`、`onMouseMove`、`onMouseDown`、`onMouseUp`、`onMouseWheel`、`onDoubleClick`、`onContextMenu` 字段引用 C 侧 `xgeXuiBinderSetEvent` 注册的事件名，page load 时固化到 Widget 分类型事件槽。
- `onKeyDown`、`onKeyUp`、`onTextInput` 字段同样通过 `xgeXuiBinderSetEvent` 绑定，保持键盘输入与文本输入事件分层。
- `hotkey` 字段注册事件热键，格式为对象或对象数组：`{ "key": "B", "modifiers": ["ctrl", "shift"], "event": "preview_move" }`。`key` 支持单字符、`enter/tab/escape/space/delete/left/right/up/down/home/end/pageUp/pageDown/f10/menu` 等命名键；`modifiers` 支持字符串或数组。
- `command` 字段注册命令热键，格式为对象或对象数组：`{ "key": "S", "modifiers": "ctrl", "id": 7001, "name": "test.save" }`。命令通过 Widget Command 分发，目标控件可用 `onCommand` 接收。
- 未注册事件名是加载错误，错误信息包含字段路径。
- `script`、`onClickScript` 等脚本字段被拒绝，XSON 内不执行脚本。
- 结构型 widget 的 `onClick` 绑定到 page 创建的通用 widget 事件过程，触发后调用 `xge_xui_click_proc(widget, user)`；`user` 来自 binder 注册。
- `button` 的 `onClick` 绑定到按钮控件自身 `xgeXuiButtonSetClick`，由按钮状态机在点击成立时触发。
- `input.onChange/onSubmit` 依赖 value/model 双向状态模型，当前解析时作为加载错误处理，避免为了兼容留下空的占位函数或无效绑定。

XSON 事件绑定后续统一扩展：

- 已解析事件字段必须在 page load / refresh 阶段固化到 binder 或 widget event handle，layout/paint/input 热路径不得访问 XSON 字符串。

## 数据绑定

第一版只支持简单 key 绑定，不支持表达式：

```json
{
  "type": "label",
  "id": "coin_label",
  "text": "${player.coins}"
}
```

C 侧提供 model：

```c
xgeXuiModelSetInt(pModel, "player.coins", iCoins);
xgeXuiPageApplyModel(&tPage, pModel);
```

绑定更新只修改受影响控件的属性，并触发对应 dirty。

## 列表与模板

大列表必须虚拟化。XSON 可以描述 item 模板，数据由 C adapter 提供：

```json
{
  "type": "virtualized ListView",
  "id": "inventory",
  "itemHeight": 44,
  "itemTemplate": {
    "type": "row",
    "padding": [8, 6],
    "gap": 8,
    "children": [
      { "type": "image", "id": "icon", "w": 32, "h": 32, "src": "${icon}" },
      { "type": "label", "id": "name", "w": "grow", "text": "${name}" },
      { "type": "label", "id": "count", "w": 48, "text": "${count}" }
    ]
  }
}
```

第一版可以先只支持固定 item height。每个可见 item 复用有限数量 widget 或走轻量绘制路径。

## 资源引用

图片、XSON 和其他外部资源统一走 `xgeResourceLoad`：

```json
{
  "type": "image",
  "src": "res://ui/icons/save.png"
}
```

资源所有权由 page/loader 管理。字体建议不由 XSON 直接加载，优先通过 theme token 引用已加载字体。

## 建议 API

```c
typedef struct xge_xui_page_t xge_xui_page_t;
typedef struct xge_xui_binder_t xge_xui_binder_t;
typedef struct xge_xui_model_t xge_xui_model_t;

int xgeXuiPageLoad(
	xge_xui_context pContext,
	const char* sURI,
	const xge_xui_binder_t* pBinder,
	xge_xui_page_t* pPage);

int xgeXuiPageLoadMemory(
	xge_xui_context pContext,
	const void* pData,
	int iSize,
	const xge_xui_binder_t* pBinder,
	xge_xui_page_t* pPage);

void xgeXuiPageUnload(xge_xui_page_t* pPage);
xge_xui_widget xgeXuiPageRoot(xge_xui_page_t* pPage);
xge_xui_widget xgeXuiPageFind(xge_xui_page_t* pPage, const char* sId);
int xgeXuiPageApplyModel(xge_xui_page_t* pPage, xge_xui_model_t* pModel);
const char* xgeXuiPageGetError(xge_xui_page_t* pPage);

void xgeXuiBinderInit(xge_xui_binder_t* pBinder);
void xgeXuiBinderSetClick(xge_xui_binder_t* pBinder, const char* sName, xge_xui_click_proc procClick, void* pUser);
```

具体结构命名后续可按 XUI 剥离路线调整。

## 调试能力

声明式 UI 必须配套调试能力：

- XSON 加载错误带路径，例如 `tree.children[2].w invalid size unit`。
- Debug overlay 显示 widget id/type。
- 显示 rect、content rect、margin、padding。
- 显示 layout dirty、paint dirty、style cache dirty。
- 显示 hit test、focus、capture。
- 布局快照测试：给定窗口尺寸，输出 widget rect tree，用文本 diff 回归。

这些能力属于 `xgedbg`，必须受 `XGE_DEBUGMODE` 保护；正式 `xge` 只保留运行所需的最小错误码、必要错误字符串和正常 XUI runtime。XSON 详细 trace、layout snapshot、debug overlay、widget inspector、热重载诊断面板不进入 `xge`。

## 分阶段落地

阶段 1：核心语义修正与加载器骨架。

- 修正现有 layout 语义。
- 引入 page/load/binder 基础结构。
- XSON 加载基础 widget tree。
- 支持 style 父表继承。

阶段 2：常用控件 XSON 化。

- 支持 panel/row/column/stack/grid/label/button/input/image/separator。
- 支持 id/name 索引和事件名绑定。
- 支持 theme token。

阶段 3：APP 能力。

- 支持 scroll/dock/safeArea。
- 支持 virtualized ListView 和 itemTemplate。
- 支持 model 数据绑定。

阶段 4：开发体验。

- 布局快照测试。
- Debug overlay。
- 热重载或手动 reload。
- 示例迁移到 XSON。

## 关键原则

- XSON 负责描述。
- XValue 负责共享和继承。
- XUI 负责高效 retained runtime。
- C 负责业务逻辑。
- Cache 负责热路径性能。
- APP UI 和游戏 UI 的运行能力进入 `xge`。
- 调试与诊断能力进入 `xgedbg`。
