# XUI Style System Design

本文记录 XUI 样式系统设计。XUI 样式目标是简单易用、够用、灵活、执行效率快，并能覆盖动态换肤需求。XUI 不做完整 CSS，而是做强类型、可缓存、可广播失效的 Style Manager。

## 1. Design Position

XUI 样式系统不是 CSS 子集，而是：

```text
Typed StyleSheet + Token Theme + Resolved Style Cache
```

核心目标：

- 写起来像样式表。
- 运行时像 C 结构体。
- 换肤时有版本和依赖追踪。
- 绘制热路径不查字符串、不跑 selector。
- 样式变化能精确触发布局、缓存和 damage 失效。

## 2. File Format Decision

XUI 第一版样式和主题文件继续使用 XSON/JSON 结构化格式，不引入新的样式语言和解析器。

原因：

- XUI 已经有 XSON、tokens、styles、imports 基础。
- 解析器、错误定位、导入机制和 UIDesign 工具链可以复用。
- 样式字段本身是结构化数据，用 XSON 表达自然。
- 新语法会带来 parser、formatter、语法高亮、转义、错误恢复等额外成本。
- 当前重点是样式模型、缓存和动态失效机制，不是语言设计。

推荐文件命名：

```text
*.xui.xson     UI page
*.theme.xson   theme file
*.style.xson   stylesheet file
```

通过顶层字段区分文件类型：

```json
{ "xui": 2 }
{ "xuiTheme": 2 }
{ "xuiStyle": 2 }
```

后续如果需要类 CSS 的简写语法，可以提供可选编译器，把 `.xss` 编译成 `.style.xson`。运行时只消费结构化 XSON。

## 3. Core Concepts

| Concept | Responsibility |
| --- | --- |
| `Token` | 主题变量：颜色、间距、字体、圆角、纹理、阴影 |
| `Theme` | 一组 token、字体、资源和全局默认，可继承 |
| `StyleRule` | 结构化匹配规则：type/class/id/state/part |
| `StyleClass` | widget 可挂多个 class，用于语义样式组合 |
| `NamedStyle` | 可继承的命名样式，适合设计器、XSON 和复用 |
| `ResolvedStyle` | 最终解析后的热路径缓存，widget 只读它 |
| `StyleManager` | 加载、解析、匹配、diff、广播失效的中心服务 |

控件不在绘制时解析 XSON、查 class、查 token。加载、换肤或 class/style 变化时统一解析到 `ResolvedStyle`。

## 4. File Layers

样式文件分三层：

### 4.1 Theme File

Theme file 放 token、字体、资源和全局默认。

Example:

```json
{
  "xuiTheme": 2,
  "name": "dark",
  "extends": "base.theme.xson",
  "tokens": {
    "colors": {
      "accent": "#2F7DFF",
      "text": "#E8EEF8",
      "panel": "#1F2630",
      "danger": "#D94848"
    },
    "spacing": {
      "xs": 2,
      "sm": 4,
      "md": 8,
      "lg": 12
    },
    "radius": {
      "sm": 3,
      "md": 5
    }
  },
  "fonts": {
    "ui": { "family": "system", "size": 13 },
    "mono": { "family": "monospace", "size": 13 }
  }
}
```

### 4.2 Stylesheet File

Stylesheet file 放 named styles、class rules、state styles 和 part styles。

Example:

```json
{
  "xuiStyle": 2,
  "imports": [
    "base.style.xson"
  ],
  "styles": {
    "button.base": {
      "padding": [10, 6],
      "radius": "@radius.sm",
      "font": "@fonts.ui",
      "textColor": "@colors.text"
    },
    "button.primary": {
      "@parent": "button.base",
      "background": "@colors.accent",
      "state": {
        "hover": { "background": "#3D8BFF" },
        "pressed": { "background": "#1F68D8" },
        "disabled": { "opacity": 0.45 }
      }
    }
  },
  "rules": [
    {
      "match": { "type": "button", "class": ["primary"] },
      "style": "button.primary"
    },
    {
      "match": { "type": "label", "class": ["muted"] },
      "set": { "textColor": "#99A4B3" }
    }
  ]
}
```

### 4.3 Page XSON

Page XSON 放 UI tree、id、class、局部 inline override 和页面级 stylesheet 引用。

Example:

```json
{
  "xui": 2,
  "theme": "themes/dark.theme.xson",
  "stylesheets": [
    "styles/app.style.xson",
    "styles/dashboard.style.xson"
  ],
  "tree": {
    "type": "row",
    "id": "actions",
    "class": ["toolbar"],
    "children": [
      {
        "type": "button",
        "id": "ok",
        "class": ["primary"],
        "text": "OK"
      },
      {
        "type": "button",
        "id": "cancel",
        "class": ["secondary"],
        "text": "Cancel"
      }
    ]
  }
}
```

## 5. Selector Boundary

第一版 selector 保持克制，只支持结构化匹配：

```text
type        button / label / panel
.class      primary / danger / toolbar-item
#id         okButton
:state      hover / pressed / disabled / focused / checked / selected / error
::part      thumb / track / header / cell / icon / text
```

不支持：

```text
ancestor selector
descendant selector
nth-child
arbitrary sibling selector
complex CSS specificity
media query
```

原因：

- 祖先/后代 selector 会显著增加样式失效范围和动态 class 切换成本。
- XUI 目标是 APP UI 和轻量文档，不需要完整 CSS cascade。
- StyleManager 应能快速计算受影响 widget，不应在热路径全树匹配复杂 selector。

## 6. Priority Order

固定样式优先级：

```text
builtin control default
< theme default
< app stylesheet
< page stylesheet
< named style extends chain
< widget class list, applied in class order
< id rule
< inline style
< inline state override
```

说明：

- 不做完整 CSS specificity。
- class 顺序是显式语义，后面的 class 可以覆盖前面的 class。
- state override 在各自层级内生效，不应让 theme hover 覆盖 app inline style。
- inline 始终是 widget-local 最强覆盖，除非明确进入 debug/forced theme 模式。

## 7. Inheritance Model

XUI 支持三种有限继承。

### 7.1 NamedStyle Inheritance

Named style 可通过 `@parent` 继承另一个 named style。

```json
{
  "styles": {
    "text.base": {
      "font": "@fonts.ui",
      "textColor": "@colors.text"
    },
    "text.muted": {
      "@parent": "text.base",
      "textColor": "#99A4B3"
    }
  }
}
```

要求：

- 加载期检测 parent cycle。
- 加载期展开或编译继承链。
- 热路径不查 parent chain。

### 7.2 Theme/Token Inheritance

Theme 可以继承 base theme，只覆盖部分 token。

```json
{
  "xuiTheme": 2,
  "extends": "base.theme.xson",
  "tokens": {
    "colors": { "accent": "#48A6FF" }
  }
}
```

### 7.3 Widget Tree Inheritance

Widget 树继承只允许少量字段自动继承：

```text
font
fontSize
lineHeight
textColor
textAlign
locale
textDirection
```

不自动继承：

```text
margin
padding
background
border
layout
width
height
position
dock
grid/table participation
cache policy
```

理由：APP UI 中 layout 和 box visual 自动继承会非常危险，容易造成全局样式难以预测。

## 8. Property Categories

每个 style property 必须标记 dirty 类型。

| Category | Examples | Dirty Effect |
| --- | --- | --- |
| layout | width, height, margin, padding, gap, display, dock, row/column/span | measure/arrange dirty |
| text-measure | font, fontSize, lineHeight, wrap, textTransform | measure dirty + cache dirty |
| visual | background, textColor, borderColor, opacity | local/state cache dirty |
| paint-bounds | shadow, outline, focusRing, glow | cache bounds dirty + damage |
| cache-policy | cacheMode, stateCachePolicy, tileSize | cache rebuild |
| interaction | cursor, hitTest, tabStop | event/hit/focus dirty |

动态换肤时，颜色变化不应全树重新布局；字体、spacing、padding 等才触发 layout dirty。

## 9. State Styles

状态样式是基础能力，不是控件局部补丁。

Common states:

```text
normal
hover
pressed
active
disabled
focused
checked
selected
readonly
error
open
editing
dirty
```

状态样式应参与 cache key：

```text
state id + style hash + size + dpi + theme version
```

Button、Tab、CheckBox、Toolbar item 等控件可以通过 state cache 实现状态切换只换 cache handle，不重新绘制内容。

## 10. Part Styles

复合控件需要 part style，但 part style 不应破坏控件封装。

Examples:

```text
button::icon
button::text
scrollbar::track
scrollbar::thumb
table::header
table::cell
tabs::tab
tabs::close
```

Part style 规则：

- part 名称由控件文档化。
- part style 编译期解析为 property id。
- 绘制热路径不能查字符串。
- 控件内部可为 part 建立 `ResolvedPartStyle` cache。

## 11. Control Private Properties

控件私有样式字段不应散落在各控件的临时结构中。

XUI 使用 style property registry：

```text
common.background
common.borderColor
common.padding
button.iconGap
button.iconPlacement
scrollbar.thumbColor
table.headerBackground
```

Registry 负责：

- property name -> property id。
- value type validation。
- default value。
- dirty category。
- applicable widget type or part。
- parse from XSON。
- write into resolved style or typed property bag。

通用字段进入固定 `ResolvedStyle` 结构；控件私有字段进入 typed property bag。加载期编译 property id，热路径不查字符串。

## 12. ResolvedStyle Cache

每个 widget 维护最终样式缓存：

```text
ResolvedStyle
ResolvedStateStyle[state]
ResolvedPartStyle[part][state]
styleHash
dependencyMask / dependencyList
styleGeneration
```

ResolvedStyle 的输入：

- widget type。
- widget id。
- class list。
- named style。
- inline style。
- current theme/token generation。
- inherited fields from parent widget。
- state/part overrides。

绘制、布局和 cache 系统只读取 ResolvedStyle，不读取 stylesheet/XSON。

## 13. Dynamic Skinning Flow

动态换肤流程：

```text
StyleManagerBeginUpdate
  update tokens / theme / stylesheet
StyleManagerEndUpdate
  bump token/theme/rule generation
  find affected widgets
  recompute resolved style
  diff old/new style
  emit layout/cache/damage dirty
```

第一版可以保守全树 style dirty，但架构上必须支持依赖追踪：

```text
widget depends on token colors.accent
widget depends on class primary
widget resolved against theme generation N
```

后续可按依赖局部失效。

## 14. Cache System Integration

样式系统必须直接服务 XUI cache system。

Rules:

- `ResolvedStyle` 变化生成新的 `styleHash`。
- 控件 cache key 包含 `styleHash`。
- state cache key 包含 `stateId + styleHash + size + dpi + themeVersion`。
- 颜色换肤只重建受影响 visual cache。
- 字体/lineHeight 变化触发 measure dirty 和 text cache rebuild。
- shadow/focusRing 变化触发 cache bounds 和 screen damage。

## 15. Layout System Integration

样式属性影响布局时，必须显式触发对应 dirty：

- `display`: parent arrange dirty。
- `layout`: self arrange dirty + subtree arrange dirty。
- `margin`: parent arrange dirty。
- `padding`: self measure/arrange dirty。
- `gap`: self arrange dirty。
- `font/lineHeight`: self measure dirty。
- `table/flow participation`: parent arrange dirty。

StyleManager 不直接调用 layout；它只发 dirty 标记，由 layout system 在 update 阶段处理。

## 16. Hot Reload and Tooling

XSON 样式文件适合热重载。

Hot reload 流程：

```text
file changed
parse XSON
validate registry properties
compile styles/rules
swap StyleSheet generation
StyleManagerEndUpdate
```

UIDesign 工具可以：

- 显示 widget 当前匹配规则。
- 显示 token dependency。
- 显示 resolved style diff。
- 标记无效属性，例如 flow-only 属性放到 row child 上。
- 提供 theme preview。

## 17. Error Handling

加载期应检测：

- invalid token reference。
- invalid property name。
- invalid value type。
- invalid style parent。
- style parent cycle。
- selector references unknown part。
- property not applicable to widget type or part。

策略：

- 开发/设计器模式：详细错误。
- Release：可选择 fail-fast 或 fallback default。
- Hot reload：新样式失败时保留旧样式。

## 18. Example Full Set

Theme:

```json
{
  "xuiTheme": 2,
  "tokens": {
    "colors": {
      "accent": "#2F7DFF",
      "accentHover": "#3D8BFF",
      "accentPressed": "#1F68D8",
      "text": "#E8EEF8",
      "muted": "#99A4B3"
    },
    "spacing": {
      "buttonX": 10,
      "buttonY": 6
    },
    "radius": {
      "button": 4
    }
  },
  "fonts": {
    "ui": { "family": "system", "size": 13 }
  }
}
```

Style:

```json
{
  "xuiStyle": 2,
  "styles": {
    "button.base": {
      "padding": ["@spacing.buttonX", "@spacing.buttonY"],
      "radius": "@radius.button",
      "font": "@fonts.ui",
      "textColor": "@colors.text"
    },
    "button.primary": {
      "@parent": "button.base",
      "background": "@colors.accent",
      "state": {
        "hover": { "background": "@colors.accentHover" },
        "pressed": { "background": "@colors.accentPressed" },
        "disabled": { "opacity": 0.45 }
      }
    }
  },
  "rules": [
    {
      "match": { "type": "button", "class": ["primary"] },
      "style": "button.primary"
    },
    {
      "match": { "type": "label", "class": ["muted"] },
      "set": { "textColor": "@colors.muted" }
    }
  ]
}
```

Page:

```json
{
  "xui": 2,
  "theme": "themes/dark.theme.xson",
  "stylesheets": ["styles/app.style.xson"],
  "tree": {
    "type": "row",
    "class": ["dialog-actions"],
    "children": [
      { "type": "button", "class": ["primary"], "text": "OK" },
      { "type": "button", "text": "Cancel" }
    ]
  }
}
```

## 19. Design Decisions So Far

- XUI 样式和主题继续使用 XSON/JSON，不引入新解析器。
- 文件类型通过 `xui` / `xuiTheme` / `xuiStyle` 顶层字段区分。
- 样式系统核心是 StyleManager、token theme 和 resolved style cache。
- 第一版 selector 克制，只支持 type/class/id/state/part。
- 不做完整 CSS specificity。
- 继承只支持 named style、theme/token 和少量 widget tree inherited fields。
- 所有 property 必须标记 dirty category。
- 动态换肤通过 generation、dependency 和 style diff 驱动 layout/cache/damage dirty。
- 绘制热路径不查字符串、不访问 XSON。

## 20. Open Questions

- class list 合并时是否允许显式 `!important`。当前建议不支持。
- id rule 是否必须比 class 强，还是仅作为规则顺序的一种。当前建议 id 强于 class。
- part style 的 property bag 是否与 widget style 共享结构。
- 第一版是否实现 token dependency 精确追踪，还是先全树 style dirty。
- 是否允许 stylesheet 在 page 内声明局部 `tokens`。当前建议允许但只作用于 page scope。
- `inline state override` 的 XSON 结构是否与 stylesheet state 完全一致。

