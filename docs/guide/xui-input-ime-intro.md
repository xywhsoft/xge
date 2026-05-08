# XUI 文本输入与 IME 入门

本教程说明 XUI 输入框如何处理 UTF-8 文本、光标、选择和系统 IME。

[返回教程索引](README.md) | [XUI API](../api/xui.md) | [输入教程](input-intro.md)

> 当前 guide 描述第一版 Input/IME。Widget V2 后，IME 由 FocusManager 和 `imeMode` 统一管理；普通非文本控件默认禁用 IME，Input/TextEdit 显式申请 IME，Password 默认禁用 IME。

## 边界

XUI 第一版不实现完整输入法。平台后端负责接入系统 IME，并把提交文本、组合文本和候选框位置能力转给 XUI。

XUI 负责：

- 保存 UTF-8 文本。
- 处理插入、删除、光标和选择。
- 接收 composition 文本。
- 提供候选框位置。

## 文本核心

```c
xge_xui_text_t text;

xgeXuiTextInit(&text);
xgeXuiTextSet(&text, "hello");
xgeXuiTextInsert(&text, " world");
xgeXuiTextDeleteBack(&text);
```

单个 codepoint 可用：

```c
xgeXuiTextInsertCodepoint(&text, 0x4E2D);
```

## 光标和选择

```c
xgeXuiTextSetCursor(&text, 3);
xgeXuiTextSetSelection(&text, 1, 4);
```

索引基于 XUI 文本缓冲的内部约定。业务层不要把字节下标、codepoint 下标和 glyph 下标混用。

## Input 控件

```c
xge_xui_input_t input;

xgeXuiInputInit(&input, &ui, widget, &font);
xgeXuiInputSetText(&input, "player");
xgeXuiInputSetSelection(&input, 0, 6);
```

平台事件进入后：

```c
xgeXuiInputEvent(&input, &event);
```

通常不需要手动调用，`xgeXuiDispatchEvent` 会把事件送到焦点控件。

## IME 候选框

宿主需要知道候选框显示位置：

```c
xge_rect_t candidate;

candidate = xgeXuiGetImeCandidateRect(&xui);
```

桌面、移动端和小程序后端应把当前 context 的矩形传给系统 IME 或宿主输入组件。`Input` / `TextEdit` 会自动注册光标位置解析器；自定义文本控件需要通过 `xgeXuiWidgetSetImeCandidateRect` 注册。

## Composition 文本

```c
xgeXuiTextSetComposition(&text, "pin");
xgeXuiTextClearComposition(&text);
```

Composition 只是临时预编辑文本，提交后才进入正式文本内容。

## 常见错误

不要用 keydown 拼中文输入。中文、日文等输入必须依赖系统 IME 文本事件。

不要把候选框位置写死在输入框左上角，应使用当前光标位置附近的 candidate rect。

不要混用 UTF-8 字节长度和用户可见字符长度。后续复杂文本需要更明确的文本索引层。

## 下一步

- 控件整体使用读 [XUI 控件入门](xui-controls-intro.md)。
- APP 模式刷新读 [XUI 渲染入门](xui-render-intro.md)。
