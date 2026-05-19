# XSON UI 入门

本教程介绍如何用 XSON 描述 XUI 页面。XSON 页面会被加载成 retained widget tree，适合不依赖 UI 设计器、直接用布局描述快速开发 APP 界面和游戏内嵌界面。

[返回教程索引](README.md) | [XUI 布局](xui-layout-intro.md) | [XUI 控件](xui-controls-intro.md) | [XUI API](../api/xui.md)

> 当前 XSON 中，`type` 明确映射到 Control、Container、Viewport、Overlay；Control 默认不允许普通 `children`。`scroll` / `scrollView` 已按 ScrollModel + ScrollFrame 新口径恢复，`children` 会挂到内部 content widget；`popup`、`listView` 和 `treeView` 已恢复。
> `virtualList`、`tableView`、`propertyGrid`、`menu`、`comboBox`、`colorPicker`、`textEdit` 仍处于重构隔离状态，声明会暂时报不可用；通用层级字段使用 `layer` 与 `zIndex`/`z`，排序规则为 `layer > z > treeOrder`；通用命中字段支持 `hitTestVisible` 和 `inputTransparent`；通用焦点字段支持 `tabStop`、`tabIndex` 和 `imeMode`；通用基础绘制字段支持 `borderColor`、`borderWidth`、`focusRingColor`、`focusRingWidth`、`disabledOverlay`、`debugOutlineColor` 和 `debugOutlineWidth`。

## 适用范围

XSON UI 第一版覆盖结构化页面，而不是完整脚本 UI：

- Container：`panel/absolute/row/column/stack/grid/dock`，允许普通 `children`。
- Viewport：`scroll/scrollView` 已恢复普通 `children`，`listView` 已恢复固定行高列表能力，`treeView` 已恢复层级节点能力，其他 viewport 类型仍在新 VirtualView 口径下重构。
- Control：`label/button/image/input/numericInput/colorPicker/datePicker/checkbox/radio/switch/slider/progress/tabs/toolbar/statusBar/comboBox/breadcrumb/accordion/separator`，默认不允许普通 `children`。
- Overlay：`popup/tooltip/menu/dialog/messageBox/toast`。
- 样式：`styles`、`@parent`、tokens、inline override。
- 事件：当前支持 `onClick` 绑定到 C 侧注册名。
- 数据：当前支持 label/input/image 的 `${key}` 简单 model binding。
- 调试：详细 page trace 只在 `xgedbg` 中可用。

XSON 不执行脚本，不做表达式求值，也不在 layout/paint 热路径反复解析字符串字段。

## 最小加载流程

```c
xge_xui_context_t ui;
xge_xui_page_t page;
xge_xui_binder_t binder;

xgeXuiInit(&ui);
xgeXuiBinderInit(&binder);

if ( xgeXuiPageLoad(&ui, "ui/main.xson", &binder, &page) != XGE_OK ) {
	printf("page load failed: %s\n", xgeXuiPageGetError(&page));
}

xgeXuiUpdate(&ui, delta);
xgeXuiPaint(&ui);

xgeXuiPageUnload(&page);
xgeXuiUnit(&ui);
```

也可以用 `xgeXuiPageLoadMemory` 从内存字符串加载，示例见 `examples/xui_xson_page_lab`。

## 页面结构

一个页面通常包含 `tokens`、`styles`、`tree` 三部分：

```json
{
  "xui": 1,
  "tokens": {
    "colors": { "panel": "#202631", "text": "#EEF4FF", "accent": "#3277FF" },
    "spacing": { "pad": 10, "gap": 8, "headerH": 48 }
  },
  "styles": {
    "screen": { "layout": "dock", "width": "100%", "height": "100%", "padding": "@spacing.pad", "gap": "@spacing.gap" },
    "header": { "dock": "top", "height": "@spacing.headerH", "background": "@colors.panel" },
    "title": { "textColor": "@colors.text" },
    "action": { "width": 92, "height": 28, "color": "@colors.accent", "textColor": "@colors.text" }
  },
  "tree": {
    "type": "dock",
    "id": "app",
    "style": "screen",
    "children": [
      {
        "type": "row",
        "id": "header",
        "style": "header",
        "children": [
          { "type": "label", "id": "title", "style": "title", "text": "Dashboard" },
          { "type": "button", "id": "refresh", "style": "action", "text": "Refresh", "onClick": "refresh" }
        ]
      },
      { "type": "column", "id": "content", "dock": "fill", "width": "grow", "height": "grow" }
    ]
  }
}
```

`id` 为字符串时会写入 widget name，并建立 page 索引：

```c
xge_xui_widget refresh = xgeXuiPageFind(&page, "refresh");
```

## 样式和覆盖顺序

字段来源按以下顺序生效：

1. 控件默认值。
2. 命名 style。
3. style 的 `@parent` 链。
4. widget inline 字段。

示例：

```json
{
  "styles": {
    "fill": { "width": "100%", "height": "100%" },
    "panel": { "@parent": "fill", "layout": "column", "padding": 12, "gap": 6 },
    "toolbar": { "@parent": "panel", "layout": "row", "height": 44, "justify": "space-between" }
  },
  "tree": {
    "type": "row",
    "id": "toolbar",
    "style": "toolbar",
    "padding": [4, 5, 6, 7]
  }
}
```

这里 `toolbar` 继承 `panel` 和 `fill`，但自身 inline `padding` 覆盖 style 中的 padding。

## imports

`imports` 用于复用 tokens、styles 和 templates：

```json
{
  "imports": [ "common.xson", "theme/dark.xson" ],
  "styles": {
    "localPanel": { "@parent": "panel", "padding": 16 }
  },
  "tree": { "type": "column", "id": "root", "style": "localPanel" }
}
```

路径规则：

- 带 scheme 的 URI 原样交给 `xgeResourceLoad`。
- 相对路径按当前 XSON 文件所在目录解析。
- imports 只合并 `styles/tokens/templates`，不导入 `tree`。
- 后导入覆盖先导入；当前页面本地声明优先于 imports。
- 循环导入会使 page load 失败并给出错误。

## 事件绑定

XSON 事件只引用 C 侧注册名。未注册的事件名会使 page load 失败，避免静默丢事件。

```c
static void OnRefresh(xge_xui_widget widget, void* user)
{
	(void)widget;
	(void)user;
}

xge_xui_binder_t binder;
xgeXuiBinderInit(&binder);
xgeXuiBinderSetClick(&binder, "refresh", OnRefresh, app);
xgeXuiPageLoad(&ui, "ui/main.xson", &binder, &page);
```

```json
{ "type": "button", "id": "refresh", "text": "Refresh", "onClick": "refresh" }
```

`script` 和 `onClickScript` 会被拒绝。

## 数据绑定

第一版 model binding 只识别完整字段值 `${key}`，不支持插值或表达式：

```json
{
  "tree": {
    "type": "column",
    "children": [
      { "type": "label", "id": "nameLabel", "text": "${player.name}" },
      { "type": "input", "id": "nameInput", "value": "${player.name}" },
      { "type": "image", "id": "avatar", "src": "${player.avatar}" }
    ]
  }
}
```

```c
xge_xui_model_t model;
xgeXuiModelInit(&model);
xgeXuiModelSetText(&model, "player.name", "Ada");
xgeXuiModelSetText(&model, "player.avatar", "assets/avatar.png");
xgeXuiPageApplyModel(&page, &model);
```

`Hello ${name}`、`${a+b}` 这类写法不会被识别为 binding。

## VirtualList 模板

`virtualList` 可以引用顶层 `templates` 中的 item 模板。运行时只创建可见 slot，适合长列表。

```json
{
  "templates": {
    "row": {
      "type": "row",
      "height": 24,
      "children": [
        { "type": "label", "id": "row-label", "text": "Item" }
      ]
    }
  },
  "tree": {
    "type": "virtualList",
    "id": "inventory",
    "itemCount": 1000,
    "itemHeight": 24,
    "itemTemplate": "row"
  }
}
```

## 错误和调试

加载失败后读取错误字符串：

```c
if ( xgeXuiPageLoad(&ui, "ui/bad.xson", NULL, &page) != XGE_OK ) {
	printf("%s\n", xgeXuiPageGetError(&page));
}
```

错误信息包含资源 URI 和字段路径，例如 `ui/bad.xson: tree.children: expected array`。

详细 XSON page trace 属于 `xgedbg`，release `xge` 不声明、不导出：

```c
#if XGE_DEBUGMODE
xgedbgXuiPageTrace(&page, buffer, sizeof(buffer));
#endif
```

## 示例

- `examples/xui_xson_page_lab`：最小页面加载。
- `examples/xui_xson_style_lab`：tokens、`@parent`、inline override。
- `examples/xui_xson_app_layout_lab`：Dock shell、ScrollView、Grid。
- `examples/xui_xson_virtual_list_lab`：VirtualList 和 itemTemplate。
- `examples/xui_xson_layout_gallery_lab`：手写 layout gallery 的 XSON 迁移。

## 下一步

- 学习 [XUI 布局入门](xui-layout-intro.md)。
- 学习后续样式继承文档了解 `@parent` 与 XValue 父表生命周期。
