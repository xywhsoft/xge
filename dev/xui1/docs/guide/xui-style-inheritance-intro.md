# XUI 样式继承入门

本教程说明 XSON UI 中 style、tokens 和 `@parent` 的继承规则。XUI 的样式继承基于 XValue 父表链，目标是共享数据、减少复制，并把运行时热路径控制在轻量 style cache 上。

[返回教程索引](README.md) | [XSON UI](xui-xson-intro.md) | [XUI 布局](xui-layout-intro.md) | [XUI API](../api/xui.md)

## 核心规则

样式解析发生在 page load、refresh 或 sync 阶段。layout/paint 阶段读取已经解析好的 `xge_xui_style_t`，不会反复查 XSON 字符串字段。

字段优先级：

1. widget/control 默认值。
2. 命名 style。
3. style 的 `@parent` 链。
4. widget inline 字段。

实际效果是：子 style 只写差异字段，缺失字段从父 style 查询；widget inline 字段始终可以覆盖 style。

## `@parent`

`@parent` 字段引用同一 page merged styles 中的另一个 style：

```json
{
  "styles": {
    "fill": {
      "width": "100%",
      "height": "100%"
    },
    "panel": {
      "@parent": "fill",
      "layout": "column",
      "padding": 12,
      "gap": 6,
      "background": "#101820"
    },
    "toolbar": {
      "@parent": "panel",
      "layout": "row",
      "height": 44,
      "justify": "space-between"
    }
  }
}
```

`toolbar` 不需要重复写 `width/height/padding/gap/background`。读取这些字段时，如果 `toolbar` 自己没有字段，会继续查 `panel`，再查 `fill`。

循环继承会导致 page load 失败：

```json
{
  "styles": {
    "a": { "@parent": "b" },
    "b": { "@parent": "a" }
  }
}
```

错误会包含 `style parent cycle`。

## Inline 覆盖

widget 可以在引用 style 后覆盖局部字段：

```json
{
  "tree": {
    "type": "row",
    "id": "toolbar",
    "style": "toolbar",
    "padding": [4, 5, 6, 7]
  }
}
```

这里 `toolbar` 仍继承 layout、height、justify、gap 等字段，但 `padding` 使用 widget inline 值。

这适合少量实例差异。大量重复差异应提取为新的命名 style，避免页面描述难以维护。

## Tokens

tokens 用来把颜色、间距、字体和纹理从 style 字段里抽出来：

```json
{
  "tokens": {
    "colors": {
      "panel": "#101820",
      "accent": "#2A7FFF",
      "text": "#F0F4FF"
    },
    "spacing": {
      "pad": 12,
      "gap": 6,
      "toolbarH": 44
    }
  },
  "styles": {
    "panel": {
      "padding": "@spacing.pad",
      "gap": "@spacing.gap",
      "background": "@colors.panel"
    }
  }
}
```

支持的命名空间包括：

- `tokens.colors`
- `tokens.spacing`
- `tokens.fonts`
- `tokens.textures`

字体和纹理 token 引用外部对象生命周期。XSON 不创建或释放字体对象；image 的 borrowed texture token 也不由 XSON 释放。

C 侧也可以注册 context token 作为 fallback：

```c
xgeXuiTokenSetColor(&ui, "accent", XGE_COLOR_RGBA(42, 127, 255, 255));
xgeXuiTokenSetSpacing(&ui, "pad", 12.0f);
xgeXuiTokenSetFont(&ui, "body", &font);
```

XSON/import token 优先，context token 作为 fallback。

## Imports 和合并顺序

imports 只合并 `styles/tokens/templates`，不导入 `tree`：

```json
{
  "imports": [ "theme/base.xson", "theme/dark.xson" ],
  "styles": {
    "localPanel": { "@parent": "panel", "padding": 16 }
  }
}
```

合并规则：

- imports 按数组顺序处理。
- 后导入覆盖先导入。
- 当前 page 本地声明覆盖 imports。
- 相对路径按当前 XSON 所在目录解析。
- 带 scheme 的 URI 原样交给 `xgeResourceLoad`。
- 循环导入会失败。

这意味着项目可以把基础 tokens、通用控件 style 和页面局部 style 分层组织。

## 生命周期约束

XValue 字典和列表的父表链只保存父表引用，不拥有父表生命周期。如果父表先释放，子表继续查询父表会崩溃。

第一版 XUI 的约束是：

- imported styles、merged styles、当前 page styles 必须由 `xge_xui_page_t` 持有。
- 不允许单独释放仍可能作为父表被查询的 XValue 表。
- `xgeXuiPageUnload` 统一释放 page 持有的 style/token/template 数据。

不要把临时 XValue 表设置为某个长期 page style 的父表。

## Style Cache 和刷新

加载页面时，XSON style 会解析到 widget 的 `xge_xui_style_t`。这份 style cache 包含布局和绘制高频字段，例如 layout、size、gap、padding、background、radius、grid、dock 等。

context token 改变后，已加载 page 不会在 paint/layout 热路径自动重新查 XSON。需要显式同步：

```c
xgeXuiTokenSetColor(&ui, "accent", XGE_COLOR_RGBA(80, 160, 255, 255));
xgeXuiPageSyncStyle(&page);
```

需要强制重算当前 page 样式时使用：

```c
xgeXuiPageRefreshStyle(&page);
```

可以用 style version 做外部缓存判断：

```c
uint32_t version = xgeXuiPageStyleVersion(&page);
```

## 当前限制

- `@parent` 第一版只继承命名 styles，不把 theme style 或 engine default style 作为 XValue 父表接入。
- 不支持 CSS selector、class cascade、伪类选择器或媒体查询。
- 不支持表达式。`@spacing.pad * 2` 不会求值。
- 不支持运行时热路径动态解析 XSON 字段。改 token 后要通过 page sync/refresh 更新 cache。
- 不支持自动 flatten 成完整 style 副本。父表链用于共享字段，生命周期必须统一管理。

## 示例

- `examples/xui_xson_style_lab`：tokens、style `@parent`、inline override 和 button 样式。
- `examples/xui_xson_app_layout_lab`：页面级 tokens/styles 组织。
- `test/test_main.c`：包含 style 继承、inline 覆盖、token fallback、style cache sync 和 parent cycle 回归测试。

## 下一步

- 学习 [XSON UI 入门](xui-xson-intro.md)。
- 学习 [XUI 布局入门](xui-layout-intro.md)。
