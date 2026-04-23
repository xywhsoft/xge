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

- [ ] 通过
- [ ] 失败
- [ ] 跳过

现象：

```text

```

日志：

```text

```

## 2. XUI Incubation

命令：

```bat
build\xge_xui_incubation.exe
```

结果：

- [ ] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [ ] Button normal/hover/active 状态恢复正常。
- [ ] Popup 打开、关闭、点击外部关闭正常。
- [ ] IconButton 和 Tooltip 正常。
- [ ] Tabs 切换正常。
- [ ] Progress 文本和绿色进度条正常。
- [ ] CheckBox、Radio、Switch 正常。
- [ ] ComboBox 展开和选择正常。
- [ ] Splitter 拖动正常。
- [ ] ScrollBar/ScrollView 正常。

日志：

```text
xui incubation summary:
```

## 3. XUI Layout Validation

命令：

```bat
build\xge_xui_layout_validation.exe
```

结果：

- [ ] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [ ] 初始布局无错位、重叠、闪烁。
- [ ] 调整窗口大小时布局稳定。
- [ ] grow、gap、space-between 显示合理。
- [ ] content desired 与 resize tracking 文本随窗口变化。

日志：

```text
xui layout summary:
```

## 4. XUI Input Validation

命令：

```bat
build\xge_xui_input_validation.exe
```

结果：

- [ ] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [ ] 英文输入正常。
- [ ] 中文 IME 正常。
- [ ] 长文本横向滚动正常。
- [ ] Home/End、Ctrl+Left/Ctrl+Right 正常。
- [ ] Ctrl+A/C/X/V 正常。
- [ ] 双击选词正常。
- [ ] readonly 可选择但不可修改。
- [ ] disabled 不响应编辑。

日志：

```text
xui input summary:
```

## 5. XUI TextEdit Validation

命令：

```bat
build\xge_xui_text_edit_validation.exe
```

结果：

- [ ] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [ ] 基础输入、换行、删除能力正常。
- [ ] 中文 IME 正常。
- [ ] 光标移动对标记事本基础能力。
- [ ] PageUp/PageDown、Home/End、Ctrl+Home/Ctrl+End 正常。
- [ ] 鼠标点击定位和拖拽选择正常。
- [ ] Ctrl+A/C/X/V 正常。
- [ ] Ctrl+Z/Ctrl+Y 正常。
- [ ] Ctrl+W 自动换行切换正常。
- [ ] 大段文本滚动和选择无明显卡顿、错位。

日志：

```text
xui textedit summary:
```

## 关闭条件

- 全部可执行验证项通过，或失败项已修复。
- 如果某项跳过，必须写明原因。
- 通过后可勾选 SPEC 中对应人工验证项和 `Windows 人工验证通过`。
