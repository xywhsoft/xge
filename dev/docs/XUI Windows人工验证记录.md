# XUI Windows 人工验证记录

本文用于记录 XUI 在 Windows 上的人工验证结果。每完成一项验证，应同步更新 `dev/docs/XUI轻量控件与布局SPEC.md`。

## 环境

- Windows：
- GPU：
- GCC：
- XGE commit/分支：

## 1. XUI Bridge

命令：

```bat
build\xge_xui_bridge.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

现象：

```text
初始版本只显示右侧空进度条，左侧 XUI 内容未正常绘制。修复 xui_bridge 示例后复测通过：左侧面板、标题、状态行、按钮和输入框显示正常；按钮可点击，点击后右侧进度条更新，进度条满后重置；输入框可正常输入、选择，Ctrl+A/C/X、删除等编辑快捷键正常。
```

日志：

```text
xui bridge font loaded; xui clicks/game-events 正常输出。复测截图确认通过。
```

## 2. XUI Incubation

命令：

```bat
build\xge_xui_incubation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] Button normal/hover/active 状态恢复正常。
- [x] Popup 打开、关闭、点击外部关闭正常。
- [x] IconButton 和 Tooltip 正常。
- [x] Tabs 切换正常。
- [x] Progress 文本和绿色进度条正常。
- [x] CheckBox、Radio、Switch 正常。
- [x] ComboBox 展开和选择正常。
- [x] Splitter 拖动正常。
- [x] ScrollBar/ScrollView 正常。

日志：

```text
xui incubation summary:
初测发现 popup/ComboBox 下拉默认可见、布局过紧且 ComboBox 右侧命中区与 splitter 重叠；修复 popup 默认可见、扩大示例布局后复测。随后发现 popup overlay 绘制顺序不正确，root 层文字/图标穿到 popup 之上；修复 root 与 overlay 之间的 flush 后复测通过。checkbox、radio、combobox、switch、tabs、popup 打开关闭均人工确认可正常交互。
```

## 3. XUI Layout Validation

命令：

```bat
build\xge_xui_layout_validation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] 初始布局无错位、重叠、闪烁。
- [x] 调整窗口大小时布局稳定。
- [x] grow、gap、space-between 显示合理。
- [x] content desired 与 resize tracking 文本随窗口变化。

日志：

```text
xui layout summary:
初始状态和多轮窗口缩放回放均通过。resize tracking 文本跟随窗口尺寸更新，布局块、grow、space-between 和 resize panel 显示稳定，无明显错位、重叠或闪烁。
```

## 4. XUI Input Validation

命令：

```bat
build\xge_xui_input_validation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] 英文输入正常。
- [x] 中文 IME 正常。
- [x] 长文本横向滚动正常。
- [x] Home/End、Ctrl+Left/Ctrl+Right 正常。
- [x] Ctrl+A/C/X/V 正常。
- [x] 双击选词正常。
- [x] readonly 可选择但不可修改。
- [x] disabled 不响应编辑。

日志：

```text
xui input summary:
初测窗口空白、UI 偶尔闪现；修复示例每帧稳定重绘后复测。随后发现长文本未按输入框 content rect 裁剪，修复 clipped widget 批处理 flush 与 Input 文本绘制裁剪后复测通过。英文/中文输入、选择、Ctrl+A/C/X/V、删除、Home/End、Ctrl+Left/Right、readonly 可选择不可修改、disabled 不响应编辑均人工确认通过。
```

## 5. XUI TextEdit Validation

命令：

```bat
build\xge_xui_text_edit_validation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] 基础输入、换行、删除能力正常。
- [x] 中文 IME 正常。
- [x] 光标移动对标记事本基础能力。
- [x] PageUp/PageDown、Home/End、Ctrl+Home/Ctrl+End 正常。
- [x] 鼠标点击定位、双击选词和拖拽选择正常。
- [x] Ctrl+A/C/X/V 正常。
- [x] Ctrl+Z/Ctrl+Y 正常。
- [x] Ctrl+W 自动换行切换正常。
- [x] 大段文本滚动和选择无明显卡顿、错位。
- [x] 右键/长按默认上下文菜单正常。
- [x] 菜单项与选区、readonly、剪贴板状态联动置灰正常。

日志：

```text
xui textedit summary:
初测发现光标和行高错位，拖选会出现无关选区高亮；修复行高计算和选区绘制后，拖拽选择、PageUp/PageDown、基础编辑操作复测通过。随后为 TextEdit 接入默认右键菜单，菜单包含 Select All/Cut/Copy/Paste/Delete，剪切、粘贴、删除进入 undo 栈，右键/长按弹出菜单不清空选区；修复菜单接入后双击选词缺失问题。最终人工确认多行输入框基础输入、换行、删除、中文 IME、点击定位、拖拽选择、双击选词、Ctrl+A/C/X/V、Ctrl+Z/Y、Ctrl+W、PageUp/PageDown、Home/End、Ctrl+Home/End、大段文本滚动和默认右键菜单均正常。
```

## 关闭条件

- 全部可执行验证项通过，或失败项已修复。
- 如果某项跳过，必须写明原因。
- 通过后可勾选 SPEC 中对应人工验证项和 `Windows 人工验证通过`。
