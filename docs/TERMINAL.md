# XUI Terminal 使用指南

`Terminal` 是面向本地 shell、SSH 和全屏 TUI 的终端控件。它维护独立的主屏、备用屏和逻辑滚动历史，解析 VT/xterm 输出，并把键盘、IME、焦点和鼠标输入编码后交给会话。

## 创建与连接

```c
xui_terminal_desc_t desc;
xui_widget terminal;
xui_terminal_process_desc_t process;
xui_terminal_session_t* session;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pFont = font;
desc.iColumns = 120;
desc.iRows = 36;
desc.iScrollbackLimit = 10000;
desc.iParseBudgetBytes = 65536;
desc.fCellWidth = 8.0f;
desc.fCellHeight = 16.0f;
xuiTerminalCreate(context, &terminal, &desc);

memset(&process, 0, sizeof(process));
process.iSize = sizeof(process);
process.sCommandLine = "cmd.exe";
process.iFlags = XUI_TERMINAL_PROCESS_CONPTY;
process.iColumns = desc.iColumns;
process.iRows = desc.iRows;
session = xuiTerminalCreateProcessSession(&process);
xuiTerminalAttachSession(terminal, session);
```

控件销毁前应先调用 `xuiTerminalDetachSession`，再调用 `xuiTerminalSessionDestroy`。Windows 上需要交互式 shell 或 TUI 时必须使用 `XUI_TERMINAL_PROCESS_CONPTY`；普通管道只适合非交互输出，不能提供正确的屏幕尺寸和终端输入语义。

## 缓冲区与 resize

- 主屏和备用屏分别保存单元格、样式、宽字符、组合字符和软换行状态。
- 备用屏不进入滚动历史，退出 `1049` 模式后恢复主屏和主屏光标。
- 滚动历史按逻辑行保存，不依赖创建时的列数。窗口宽度变化时重新投影显示行，不会因为旧屏幕行数不足而丢弃被挤出的文本。
- 缩小高度时，主屏顶部内容进入历史；扩大高度时可从历史恢复。`cls`、`ED 3` 或 `xuiTerminalClearScrollback` 才会主动清除对应内容。
- `xuiTerminalFit` 根据控件内容区和像素对齐后的 cell 指标计算列数、行数，并把新尺寸同步给已连接的 ConPTY。

`iScrollbackLimit` 是逻辑历史行上限。大型日志场景应设置明确上限，避免无限增长；全屏 TUI 通常主要使用备用屏，不需要很大的历史值。

## 输出和性能

`xuiTerminalWrite` 只把字节追加到环形队列，不移动已有数据。每次 update 最多解析 `iParseBudgetBytes`；`xuiTerminalFlush` 会立即处理全部待解析数据。会话轮询已经在有输出时执行 flush。

高吞吐场景建议：

1. 每次以 KiB 级块调用 `xuiTerminalWrite`，不要逐字节调用。
2. 保持 32-128 KiB 的解析预算，使连续输出不会长时间占用单帧。
3. 只有确实需要同步读取单元格或序列化文本时才调用 `xuiTerminalFlush`。
4. 使用 `xuiTerminalGetStats` 观察队列峰值、解析进度、逻辑历史行、显示行和缓冲区内存。

`test_xui/build_terminal_large_perf_test.bat` 提供批量 ANSI 输出、重排和查找基准。

## VT 与 TUI 输入

当前解析器覆盖常用的 cursor movement、erase、insert/delete character/line、scroll region、SGR 16/256/true-color、粗体/淡色/斜体/下划线/闪烁/反色/删除线、主/备用屏、保存/恢复光标、tab stop、OSC 标题与超链接、DEC line drawing、DSR/DA、应用光标、bracketed paste、焦点报告和 xterm 鼠标模式。

鼠标支持 `1000`、`1002`、`1003` 跟踪以及 X10、UTF-8 (`1005`)、SGR (`1006`) 和 URXVT (`1015`) 编码。应用启用鼠标模式后，普通点击和滚轮发送给 TUI；按住 Shift 可绕过应用鼠标并进行本地文本选择。

键盘会根据应用光标模式和修饰键生成 SS3/CSI 序列。`Ctrl+F` 原样发送给 TUI，内部查找使用 `Ctrl+Shift+F`，后续匹配使用 `F3`，反向匹配使用 `Shift+F3`。

## 选择、查找和链接

- 单击拖动选择，双击选择单词，三击选择显示行。
- `Ctrl+Shift+C` 复制，`Ctrl+Shift+V` 粘贴；应用启用 `2004` 时自动使用 bracketed paste。
- 内置查找窗口支持大小写、完整单词和正则表达式。历史结果绑定逻辑行 ID，历史重新换行后仍定位到相同单元格；活动屏幕被 TUI 改写或 resize 后会使旧匹配失效，避免显示错误高亮。
- OSC 8 超链接保留在单元格中；普通 `http`、`https`、`mailto` 和 `file` 文本也可命中。回调只自动接受安全的 `http`、`https` 和 `mailto` scheme，其他 scheme 由应用显式处理。

## 样式

可使用 `terminal.background.color`、`terminal.foreground.color`、`terminal.cursor.color`、`terminal.selection.color`、`terminal.selection.text_color`、`terminal.search.highlight_color`、`terminal.focus.color`、`terminal.link.hover_color`、`terminal.cell.width`、`terminal.cell.height`、`terminal.padding`、`terminal.palette.0` 至 `terminal.palette.255` 和继承属性 `font.name`。

cell 宽高和 padding 最终对齐到整数像素。终端不会使用亚像素位置绘制网格、光标或装饰线。
