# Overlay 与消息服务颜色样式

本页覆盖 `Popup`、`Menu`、`MsgTip`、`Toast`、`MsgBox` 与 `FileDialog`。
主入口：[XUI 全局颜色样式](XUI_COLOR_STYLES.md)。

## 共同约定

- 以下公开颜色键均注册为 `XUI_STYLE_VALUE_COLOR`，只标记 `CACHE | RENDER`，不标记布局或树结构脏。
- 使用现有样式解析顺序，支持全局 default、type、named、class、state-class、inline 及 token 引用。`0x00000000` 是有效覆盖，不作为“未设置”。
- 未指定键时保留 API/描述符基础色；清除规则后恢复基础色。服务 `GetColors` 返回基础色，不返回样式覆盖值。
- 绘制期间解析到局部颜色，不把 stylesheet 写回服务调色板。需要同步缓存子控件时使用 `onPreparePaint`；不手工调用 child cache draw、`UpdateBegin/End` 或 `ClearDirty`。
- `Popup`、`MsgBox`、`FileDialog` 的相关键允许继承。复合控件 root 的本地 inline/class 变化由 prepare hook 同帧同步到依赖缓存，无需先调用 `xuiUpdate`。
- 下列类型和属性会在对应类型/服务首次创建时注册。应用可先创建控件，再通过 `xuiWidgetFindType` 设置类型规则。

## Popup

样式目标：`xuiPopupGetType(context)` 对应的 `popup` 根 widget。

| 键 | 绘制部位 |
| --- | --- |
| `popup.panel.color` | 面板背景 |
| `popup.border.color` | 面板边框 |
| `popup.shadow.color` | 面板阴影 |
| `popup.backdrop.color` | 全 viewport 背景遮色 |
| `popup.scrollbar.track.color` | 滚动条轨道 |
| `popup.scrollbar.thumb.color` | 常态滑块 |
| `popup.scrollbar.hover.color` | 悬停滑块/按钮派生色 |
| `popup.scrollbar.active.color` | 按下滑块/按钮派生色 |
| `popup.scrollbar.focus.color` | 滚动条焦点框 |
| `popup.scrollbar.disabled.color` | 禁用滚动条 |
| `popup.scrollbar.button.color` | full 模式箭头按钮背景 |
| `popup.scrollbar.button.icon_color` | 箭头图形 |
| `popup.scrollbar.corner.color` | 双滚动条角区 |
| `popup.scrollbar.grip.color` | grip 模式角区条纹 |

滚动条委托给 `ScrollView -> ScrollFrame -> ScrollBar`；直接设置到对应子控件的 `scrollbar.*` 等原生样式继续有效。prepare 只在颜色改变时同步，记录并恢复调用方对子容器调色板的修改。

背景遮色使用不参与命中的独立 child，颜色变化不改变 outside/owner 策略或 shell 尺寸。有效 backdrop alpha 为零时采用 `NONE`，不分配全屏纹理；非零时采用 `SELF`；恢复零色时由 core 在 cache prepare 中释放旧 surface。空 shell 默认为 `NONE`，其 children 仍正常绘制和命中。

公开 `xuiPopupSetColors` 保留原有模态焦点修复与必要的 shield 几何更新；Menu 的 prepare 使用 `xuiInternalPopupSetPanelColors` 纯配色桥接，不触发布局、焦点或数据回调。

## Menu

样式目标：`xuiMenuGetType(context)` 对应的 `menu` widget，不是它的 popup 根。

| 键 | 绘制部位 |
| --- | --- |
| `menu.panel.color` | 菜单背景及 popup 面板基础色 |
| `menu.border.color` | popup 边框基础色 |
| `menu.shadow.color` | popup 阴影基础色 |
| `menu.item.hover_color` | 悬停行 |
| `menu.text.color` | 普通文本 |
| `menu.text.hover_color` | 悬停文本 |
| `menu.text.disabled_color` | 禁用文本 |
| `menu.text.danger_color` | danger 项文本 |
| `menu.shortcut.color` | 快捷键提示 |
| `menu.mark.color` | 勾选、单选、子菜单标记 |
| `menu.separator.color` | 分隔线 |
| `menu.focus.color` | 焦点框 |

不注册会覆盖 API 调色板的固定浅色默认值，因此 picker 经 `xuiMenuSetColors` 传来的已解析配色保持有效。显式 `menu.*` 规则覆盖相应 API 字段；独立的 `popup.*` 规则仍可覆盖 popup 自身 chrome。打开菜单的面板与文本在同一次 render 中刷新，清除样式恢复调用方调色板。

## MsgTip 与 Toast

MsgTip 的目标是 `xuiMsgTipGetWidget(tip)`，类型名 `msgtip`：

| 键 | 绘制部位 |
| --- | --- |
| `msgtip.background.color` | 提示背景 |
| `msgtip.border.color` | 边框 |
| `msgtip.text.color` | 文本 |
| `msgtip.icon.color` | 内置图标 tint 或备用图标背景 |
| `msgtip.icon.text.color` | 备用图标字形 |
| `msgtip.shadow.color` | 阴影 |

Toast 的目标是 `xuiToastGetItemWidget(toast, slot)`，类型名 **`toast-item`**。每条可独立设置 class/inline，不从服务对象或其他条目读样式：

| 键 | 绘制部位 |
| --- | --- |
| `toast.background.color` | 通知背景 |
| `toast.border.color` | 边框 |
| `toast.text.color` | 标题 |
| `toast.text.muted_color` | 消息正文 |
| `toast.shadow.color` | 阴影 |
| `toast.info.color` | info 类型强调色 |
| `toast.success.color` | success 类型强调色 |
| `toast.warning.color` | warning 类型强调色 |
| `toast.error.color` | error 类型强调色 |
| `toast.close.color` | 关闭图形 |
| `toast.close.hover_color` | 悬停关闭图形 |
| `toast.icon.background.color` | 图标底色，默认从类型色派生 alpha |
| `toast.progress.color` | 时长条，默认从类型色派生 alpha |

Toast 派生 alpha 与来源 alpha 相乘，来源透明零不会重新变成可见色。MsgTip 的自定义 icon surface 按白色 tint 绘制，保留图像内容，不受 `msgtip.icon.color` 重染。

## MsgBox

样式目标：`xuiMsgBoxGetWindowWidget(box)`，类型名 **`msgbox`**，仍继承并兼容 `Window`。

| 键 | 绘制部位 |
| --- | --- |
| `msgbox.backdrop.color` | 模态背景遮色 |
| `msgbox.client.color` | 消息框客户区 |
| `msgbox.text.color` | 消息正文 |
| `msgbox.text.muted_color` | 禁用按钮文本 |
| `msgbox.icon.color` | 内置图标 tint/备用图标背景 |
| `msgbox.icon.text.color` | 备用图标字形 |
| `msgbox.button.color` | 常态及焦点态按钮背景 |
| `msgbox.button.hover_color` | 悬停按钮背景 |
| `msgbox.button.active_color` | 按下/checked 按钮背景 |
| `msgbox.button.focus_color` | 焦点按钮边框 |
| `msgbox.button.disabled_color` | 禁用按钮背景 |
| `msgbox.button.border_color` | 按钮边框 |

客户区优先级为 API 基础色、`window.client.color`、`msgbox.client.color`。标题栏、边框、关闭图形等继续使用 **`window.*`**；按钮文本、图标及直接设置到按钮上的样式继续使用 **`button.*`**。

按钮状态视觉在 prepare 中比较/同步；保存原 fill/border/disabled-text，清除服务样式后恢复，包括样式激活期间调用方的新配色。服务的调色板不被修改。自定义 icon surface 不重染；内置图标接受 `msgbox.icon.color`。显式提供服务颜色时，切换消息类型不覆盖调用方的图标颜色。

消息框与 backdrop 是两个 overlay sibling，backdrop 的 prepare 也参与依赖同步，避免先绘制的背景滞后一帧。消息框的 prepare 显式链父 Window hook，并检查父 hook 返回后的存活状态。

## FileDialog

类型与目标：

| 类型 | 取得 widget |
| --- | --- |
| `file-dialog`，继承 Window | `xuiFileDialogGetWindowWidget(dialog)` |
| `file-dialog-list`，继承 ListView | `xuiFileDialogGetFileListWidget(dialog)` |
| `file-dialog-path-bar` | `xuiWidgetGetParent(xuiFileDialogGetPathBreadcrumbWidget(dialog))` |

| 键 | 绘制部位 |
| --- | --- |
| `file_dialog.path.background.color` | 路径栏背景 |
| `file_dialog.path.border.color` | 路径栏边框 |
| `file_dialog.row.color` | 文件行基础背景 |
| `file_dialog.row.hover_color` | 悬停文件行 |
| `file_dialog.row.selected_color` | 选中文件行 |
| `file_dialog.focus.color` | 文件行焦点框 |
| `file_dialog.text.color` | 普通文件名 |
| `file_dialog.text.disabled_color` | 禁用文件名，优先于选中态 |
| `file_dialog.text.selected_color` | 选中文件名 |
| `file_dialog.icon.color` | 内置文件类型图像 tint，默认白色保留图像原色 |

以上键也可设在 dialog window 上，经继承和 prepaint 同帧应用到路径栏及文件行。

路径栏无自有颜色 API，默认使用 `theme.panel`/`theme.border`。文件行未改变的工厂调色板使用 `theme.text`、`theme.state.disabled`、`theme.state.hover`、`theme.selection`、`theme.state.focus`；调用方经 `xuiListViewSetColors` 改过的字段保留为基础色。文件行优先采用相应 **`listview.*`** 规则，再由 `file_dialog.*` 特定键覆盖；默认选中前景为白色，但调用方显式改变的列表文本基础色会被保留。

主题 token 通过私有祖先类型的 `file_dialog.theme.*` 属性建立缓存依赖。它们不是公共调色入口，也不修改基础调色板；替换或清除公共类型样式不会丢失主题依赖。注册属性的 default 不接受 TOKEN，因此此处使用现有 type-rule token 模式。

Window chrome、根目录列表、输入框、路径 Breadcrumb、按钮、过滤 ComboBox、滚动条分别委托给原生 **`window.*`、`listview.*`、`input.*`、`breadcrumb.*`、`button.*`、`combobox.*`、`scrollbar.*`**。不重新实现它们的状态和数据回调。调用方替换文件列表的 item renderer 后，stylesheet 不替换或重染用户自绘内容。

FileDialog 的 window/list prepare 均显式寻找并调用父类型 hook；Window 的客户区、按钮颜色依赖不会因服务子类型而被屏蔽。

## 验证

环境：Windows PowerShell，GCC 16.1.0；在 detached worktree 中运行：

```powershell
.\test_xui\build_style_overlays_test.bat -Regression
```

也可在任意目录通过 PowerShell wrapper 执行，或用 `-Tests style_overlays` 只运行颜色专项。wrapper 链接本地源码而非旧 DLL，包含必需的 `src/xui_accessibility.c`；按源码、头文件、编译器、flags 哈希复用 object，产物在 `build/style_overlays/`。

9 个测试程序全部通过：`style_overlays`、`prepare_paint`、`popup`、`popup_focus`、`menu`、`msgtip`、`toast`、`msgbox`、`file_dialog`。其中 Popup focus 为 42/42，core prepare 为 0 failures。

颜色专项使用实际控件、真实 cache prepare 和记录型绘制 proxy，断言 cache surface 内的 fill/text/icon 颜色，不只检查注册或 getter。覆盖 runtime token/default/type/named/class/inline、透明零、清除恢复、打开状态同帧切换、API 基础色保留、Toast 派生 alpha、Window 父 hook、FileDialog 全部 10 个公开键及用户 item renderer。Popup 另断言默认透明无纹理、非零分配、转零释放；复合依赖测试检查暖帧 `iUpdatedCaches == 0`。专项刻意不先调用 `xuiUpdate`。

核心全状态缓存和销毁保护由 `prepare_paint` 专项覆盖，交互/生命周期由原有各控件测试覆盖。这里没有替代主线完整回归，也不是 GPU 像素截图测试。

## 集成序列

业务提交按以下顺序摘取（早期 MsgBox 手工刷新实现由后续增量移除，不应只摘早期部分）：

```text
e750822 cd8278c 4177667 d41c520 f5be5c9
77b38cc 1e39c7b 48614e1
ad45cbb 9381e9f efd7a51
```

本页文档提交接在上述序列之后。主线已有的依赖副本请跳过：

| 原主线提交 | 本 worktree 的副本 |
| --- | --- |
| `4ca291e`，core prepaint | `1318811` |
| `8de114a`，含滚动条透明零修复 | `f05fcda` |
| `9140f86` | `061c3b3` |
| `4943e92`，Window chrome 依赖 | `90a0c4f` |
| `faed550` | `4bd02ec` |
| `b817c2b` | `2bab1a9` |
| `4f5eac8`，含 Window prepaint | `957c84f` |

共享源文件仅通过上述已授权依赖 cherry-pick 导入，没有手工编辑。业务写集限于六个所属源文件、本专项测试/wrapper 和本文档；未 push。
