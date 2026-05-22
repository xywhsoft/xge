# XUI APP 界面开发范例

本教程用一个 APP shell 范例说明如何组织 XUI/XSON 页面。目标是用轻量 retained UI 支撑常见 APP 界面：顶部栏、侧边栏、可滚动内容、状态栏和局部数据刷新。

[返回教程索引](README.md) | [XSON UI](xui-xson-intro.md) | [XUI 布局](xui-layout-intro.md) | [XUI 样式继承](xui-style-inheritance-intro.md)

> 当前 guide 描述 APP/XSON 组织方式。ScrollView、ListView、TreeView、TableView 等必须复用 ScrollModel / ScrollFrame 新 viewport 基础设施；具体控件只保留自己的内容、选择和命中模型。overlay、clip、Z 序和 focus restore 由 Widget 基础层统一处理。

## 推荐结构

常见 APP 页面可以拆成：

- root：窗口尺寸和 safe area。
- shell：`dock` 容器，负责 header、rail、content、status 的整体分区。
- header：`row`，放标题、搜索框、操作按钮。
- rail：`column`，放导航按钮。
- content：`scrollView` 或 `listView`。
- status：`row`，放状态文本、进度、同步提示。
- overlay：弹窗、菜单、tooltip 或调试层，通常不要放进 shell 的 content flow。

第一版不要用复杂 CSS 式响应式规则。优先使用 Dock、Row、Column、Grid、ScrollView 和 ListView 组合。

## XSON 页面骨架

```json
{
  "xui": 1,
  "safeArea": [0, 0, 0, 0],
  "tokens": {
    "colors": {
      "shell": "#15191F",
      "panel": "#202631",
      "rail": "#10141A",
      "rule": "#3A4250",
      "accent": "#3277FF",
      "text": "#EEF4FF"
    },
    "spacing": {
      "pad": 10,
      "gap": 8,
      "headerH": 48,
      "railW": 72,
      "statusH": 28,
      "contentW": 480,
      "contentH": 420
    }
  },
  "styles": {
    "shell": { "layout": "dock", "width": "100%", "height": "100%", "padding": "@spacing.pad", "gap": "@spacing.gap", "background": "@colors.shell" },
    "header": { "layout": "row", "dock": "top", "height": "@spacing.headerH", "alignY": "center", "justify": "space-between", "padding": [8, 6], "background": "@colors.panel" },
    "rail": { "layout": "column", "dock": "left", "width": "@spacing.railW", "gap": 6, "padding": 6, "background": "@colors.rail" },
    "content": { "dock": "fill", "width": "grow", "height": "grow", "padding": 6, "contentSize": ["@spacing.contentW", "@spacing.contentH"], "backgroundColor": "@colors.panel", "barColor": "@colors.rule", "thumbColor": "@colors.accent" },
    "status": { "layout": "row", "dock": "bottom", "height": "@spacing.statusH", "alignY": "center", "padding": [8, 4], "background": "@colors.panel" },
    "title": { "textColor": "@colors.text" },
    "action": { "width": 92, "height": 28, "color": "@colors.accent", "textColor": "@colors.text" }
  },
  "tree": {
    "type": "dock",
    "id": "app",
    "style": "shell",
    "children": [
      {
        "type": "row",
        "id": "header",
        "style": "header",
        "children": [
          { "type": "label", "id": "title", "style": "title", "text": "${app.title}" },
          { "type": "button", "id": "refresh", "style": "action", "text": "Refresh", "onClick": "refresh" }
        ]
      },
      {
        "type": "column",
        "id": "rail",
        "style": "rail",
        "children": [
          { "type": "button", "id": "nav-a", "style": "action", "text": "A", "onClick": "navA" },
          { "type": "button", "id": "nav-b", "style": "action", "text": "B", "onClick": "navB" }
        ]
      },
      {
        "type": "scrollView",
        "id": "content",
        "style": "content",
        "children": [
          {
            "type": "grid",
            "id": "cards",
            "gridColumns": 2,
            "rowHeight": 64,
            "gap": 8,
            "children": [
              { "type": "panel", "id": "card-a", "background": "@colors.rail" },
              { "type": "panel", "id": "card-b", "background": "@colors.rail" }
            ]
          }
        ]
      },
      {
        "type": "row",
        "id": "status",
        "style": "status",
        "children": [
          { "type": "label", "id": "status-text", "style": "title", "text": "${app.status}" }
        ]
      }
    ]
  }
}
```

`examples/xui_xson_app_layout_lab` 是这个结构的可构建验证版本。

## C 侧加载

```c
typedef struct app_ui_t {
	xge_xui_context_t ui;
	xge_xui_page_t page;
	xge_xui_binder_t binder;
	xge_xui_model_t model;
} app_ui_t;

static void OnRefresh(xge_xui_widget widget, void* user)
{
	app_ui_t* app = (app_ui_t*)user;
	(void)widget;
	xgeXuiModelSetText(&app->model, "app.status", "refreshing");
	xgeXuiPageApplyModel(&app->page, &app->model);
}

static int LoadAppUi(app_ui_t* app)
{
	xgeXuiInit(&app->ui);
	xgeXuiBinderInit(&app->binder);
	xgeXuiModelInit(&app->model);
	xgeXuiBinderSetClick(&app->binder, "refresh", OnRefresh, app);
	xgeXuiModelSetText(&app->model, "app.title", "Dashboard");
	xgeXuiModelSetText(&app->model, "app.status", "ready");
	if ( xgeXuiPageLoad(&app->ui, "ui/app.xson", &app->binder, &app->page) != XGE_OK ) {
		return XGE_ERROR;
	}
	return xgeXuiPageApplyModel(&app->page, &app->model);
}
```

事件只绑定 C 侧注册名。未注册名称会使 page load 失败，适合在启动期发现配置错误。

## Safe Area

APP 或全面屏界面需要考虑刘海、系统手势区域和平台状态栏。可以在 XSON 顶层声明：

```json
{ "safeArea": [0, 24, 0, 0] }
```

也可以由平台代码动态设置：

```c
xgeXuiSetSafeAreaPx(&ui, left, top, right, bottom);
```

XUI 把 safe area 表达为 root padding。overlay root 保持全窗口，便于弹窗、菜单和调试层覆盖整个窗口。

## 内容区选择

使用 ScrollView：

- 内容数量中等。
- 子节点需要完整 retained tree。
- 内容区有表单、卡片、嵌套布局。

使用 ListView：

- 列表很长。
- item 高度固定。
- 只需要可见 slot 的 widget tree。

不要把几千行列表直接放进普通 Column。那会增加内存和 layout 成本。

## 数据刷新

第一版 model binding 是显式 apply：

```c
xgeXuiModelSetText(&model, "app.status", "loaded");
xgeXuiPageApplyModel(&page, &model);
```

它只支持完整字段 `${key}`，不做字符串插值和表达式。推荐把组合文本在 C 侧格式化后写入 model。

样式 token 改变后，显式同步 page style cache：

```c
xgeXuiTokenSetColor(&ui, "accent", XGE_COLOR_RGBA(64, 160, 255, 255));
xgeXuiPageSyncStyle(&page);
```

## 运行循环

APP 模式下仍然按 XUI retained 流程更新和绘制：

```c
static int AppFrame(void* user)
{
	app_ui_t* app = (app_ui_t*)user;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiUpdate(&app->ui, xgeGetDelta());
	xgeXuiPaint(&app->ui);
	xgeEnd();
	xgePresent();
	return 0;
}
```

如果没有连续动画，可以结合 APP 模式的刷新策略，只有输入、数据或 dirty 状态变化时刷新。

## 调试边界

release `xge` 不包含 XUI 调试 API。APP 开发期需要 snapshot、overlay 或 page trace 时，用 `xgedbg` 并用宏隔离：

```c
#if XGE_DEBUGMODE
xgedbgXuiPageTrace(&page, buffer, sizeof(buffer));
#endif
```

不要在 release 代码里保留调试占位函数。

## 常见问题

- shell 根容器优先用 `dock`，不要手写窗口尺寸变更时的 header/rail/content rect。
- 内容区 overflow 不会自动变成滚动条，必须显式选择 ScrollView 或 ListView。
- 全局颜色和间距用 tokens，不要在每个 widget 上散落硬编码。
- 页面局部差异用 inline override；重复差异提取为命名 style。
- 大列表用 ListView，不要创建所有行。
- 调试 API 全部放在 `#if XGE_DEBUGMODE` 中。

## 示例

- `examples/xui_xson_app_layout_lab`：APP shell 声明式布局验证。
- `examples/xui_xson_layout_gallery_lab`：更多 layout 几何组合。
- `examples/xui_listview_xson`：长列表模板和 slot 复用。
- `examples/app_mode`：APP run mode 基础。

## 下一步

- 学习 [XSON UI 入门](xui-xson-intro.md)。
- 学习 [XUI 渲染与刷新入门](xui-render-intro.md)。
