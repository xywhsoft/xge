# XUI Text Menu Detail Lab

`xui_text_menu_detail_lab` 专门拆开 `Input` 和 `TextEdit` 的默认右键菜单细节。它不是再验证一次编辑功能本身，而是聚焦“什么时候弹菜单、哪些项会被禁用、触发各项后文本和剪贴板怎么变化”。

## 覆盖 API

- `xgeXuiInputInit`
- `xgeXuiInputUnit`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiInputSetReadonly`
- `xgeXuiInputSetPassword`
- `xgeXuiInputEvent`
- `xgeXuiTextEditInit`
- `xgeXuiTextEditUnit`
- `xgeXuiTextEditSetText`
- `xgeXuiTextEditGetText`
- `xgeXuiTextEditSetReadonly`
- `xgeXuiTextEditSetWordWrap`
- `xgeXuiTextEditEvent`
- `xgeXuiMenuIsOpen`
- `xgeXuiMenuClose`
- `xgeClipboardSetText`
- `xgeClipboardGetText`

说明：默认菜单项的具体开关状态通过公开结构体字段 `arrDefaultMenuEnabled` 观察；菜单动作则通过默认菜单绑定的 `procSelect` 路径触发。

## 运行方式

```bat
examples\xui_text_menu_detail_lab\build.bat
build\xge_xui_text_menu_detail_lab.exe --frames 5
```

## 自动检查

- `Input` 在“有选区 + 剪贴板非空”时，`Select All / Cut / Copy / Paste / Delete` 五项都应可用。
- `Input` 菜单动作会依次验证 `Copy -> Cut -> Paste -> Delete -> Select All`。
- `Input` 进入 `readonly` 后，`Cut / Paste / Delete` 应禁用；进入 `password` 后，默认菜单不应打开。
- `TextEdit` 在普通状态下也会验证五项 enable 状态和 `Copy -> Cut -> Paste -> Delete -> Select All`。
- `TextEdit` 进入 `readonly` 后，`Cut / Paste / Delete` 应禁用。

如果当前环境的系统剪贴板不可用，示例会自动退化为不依赖 paste 成功的校验路径，但仍会验证菜单 enable 规则以及 `Cut / Delete / Select All`。

程序退出前会打印 `xui-text-menu-detail-lab final-summary`。

## 人工观察

- 顶部状态条里的 `input_flags`、`input_actions`、`input_guard`、`edit_flags`、`edit_actions`、`edit_guard` 都应为 `1`。
- 首个编辑框最终应停在 ` beta`。
- 第二个编辑框最终应停在 `ab\ncd` 或它的最后一步展示文本；顶部 `menus=x/y` 应是非零。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
- 若当前环境没有可用系统剪贴板，`Paste` 动作结果不会作为失败条件，但菜单 enable 规则仍会被检查。
