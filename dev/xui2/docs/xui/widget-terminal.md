# XUI Terminal

Terminal is an XUI-native terminal frontend widget. It owns terminal display
state, parser state, scrollback, selection, input encoding, rendering, and the
session boundary. The widget stays transport-neutral, while the session API now
includes Windows local process sessions. The default process session is
pipe-backed; `XUI_TERMINAL_PROCESS_CONPTY` opts into ConPTY lifecycle and
resize handling. SSH remains tracked as a future adapter.

## Behavior

- fixed cell grid with main screen and alternate screen buffers
- scrollback ring backed by `xui_scroll_model_t`
- queued write path with configurable parse budget and deterministic flush
- UTF-8 decoder and V1 ANSI/VT parser for common shell output
- foreground/background palette, 8/16 colors, 256 colors, true color, bold,
  dim, underline, inverse, scroll regions, tab stops, cursor visibility,
  save/restore cursor, alternate screen, and bracketed paste mode
- first-pass Unicode width handling with CJK wide-cell placement and
  continuation cells; complex grapheme clusters remain deferred
- input callback encodes text, Enter, Backspace, Tab, arrows, Home/End,
  PageUp/PageDown, Ctrl letters, Alt printable/control combinations,
  Alt-modified navigation keys, Ctrl+Shift Copy/Paste, and paste text
- fake session can be attached for tests and examples
- Windows process session can spawn a local command, write input to stdin, and
  poll stdout/stderr into the terminal buffer
- ConPTY process sessions can create a pseudo console and receive terminal
  resize updates through `ResizePseudoConsole`
- drag selection, double-click word selection, triple-click row selection,
  Shift selection extension, select-all, and copy selection are available
- built-in context menu provides Copy, Paste, Select All, Clear Screen,
  Clear Scrollback, and Find commands
- plain text serialization, plain text search, find next/previous, and current
  match highlighting are available as first V1 APIs
- terminal style properties can override foreground/background colors, the
  first 16 ANSI palette colors, focus/search/link colors, padding, cell
  metrics, and inherited `font.name`
- output parsing marks dirty screen rows, and Terminal cache rendering keeps a
  persistent cache surface so normal output can redraw only affected rows

The current implementation is still a terminal frontend. The built-in process
session supports a pipe-backed adapter for simple local commands and an opt-in
ConPTY lifecycle path. Full interactive terminal application parity, SSH,
credentials, and transport error policies remain outside the widget core.

## Public API

```c
xuiTerminalGetType
xuiTerminalCreate
xuiTerminalWrite
xuiTerminalWriteText
xuiTerminalFlush
xuiTerminalClear
xuiTerminalClearScrollback
xuiTerminalSetParseBudget
xuiTerminalGetParseBudget
xuiTerminalFit
xuiTerminalResize
xuiTerminalGetColumns
xuiTerminalGetRows
xuiTerminalGetCursor
xuiTerminalSetInputCallback
xuiTerminalSetResizeCallback
xuiTerminalSetTitleCallback
xuiTerminalSetLinkCallback
xuiTerminalSetPalette
xuiTerminalGetPalette
xuiTerminalGetScrollModel
xuiTerminalGetCell
xuiTerminalSetBracketedPaste
xuiTerminalGetBracketedPaste
xuiTerminalInputText
xuiTerminalPasteText
xuiTerminalSelectAll
xuiTerminalClearSelection
xuiTerminalGetSelectionText
xuiTerminalCopySelection
xuiTerminalSerializeText
xuiTerminalFindText
xuiTerminalFindNext
xuiTerminalFindPrev
xuiTerminalClearFind
xuiTerminalGetFindMatch
xuiTerminalGetLinkAt
xuiTerminalOpenMenu
xuiTerminalGetMenuWidget
xuiTerminalAttachSession
xuiTerminalDetachSession
xuiTerminalCreateFakeSession
xuiTerminalCreateProcessSession
xuiTerminalSessionDestroy
xuiTerminalSessionWrite
xuiTerminalSessionPoll
xuiTerminalSessionIsRunning
xuiTerminalSessionTerminate
xuiTerminalSessionResize
xuiTerminalSessionSetResizeCallback
xuiTerminalGetChangeCount
```

## Descriptor

```c
typedef struct xui_terminal_desc_t {
    uint32_t iSize;
    xui_font pFont;
    int iColumns;
    int iRows;
    int iScrollbackLimit;
    int iParseBudgetBytes;
    float fCellWidth;
    float fCellHeight;
    float fPadding;
    uint32_t iBackgroundColor;
    uint32_t iForegroundColor;
    uint32_t iCursorColor;
    uint32_t iSelectionColor;
    uint32_t iSelectionTextColor;
    uint32_t iSearchHighlightColor;
    uint32_t iFocusColor;
    uint32_t iLinkHoverColor;
} xui_terminal_desc_t;
```

Defaults provide an 80x24 grid, a 1000-line scrollback limit, a 4096-byte parse
budget, 8x16 cell metrics, and a dark terminal palette. `xuiTerminalFit`
recomputes rows and columns from the widget content rect, padding, and cell
metrics.

## Style Properties

Terminal registers these style properties:

```text
terminal.background.color
terminal.foreground.color
terminal.cursor.color
terminal.selection.color
terminal.selection.text_color
terminal.search.highlight_color
terminal.focus.color
terminal.link.hover_color
terminal.palette.0 ... terminal.palette.15
terminal.cell.width
terminal.cell.height
terminal.padding
font.name
```

Color style values are resolved before queued output is parsed, so ANSI 8/16
color output uses the current styled palette when cells are written. Calling
`xuiTerminalSetPalette` updates the widget base palette; inline/theme style
values override only the first 16 ANSI palette entries in V1. The 256-color
cube and grayscale palette remain generated internally unless changed through
`xuiTerminalSetPalette`.

## Writing Output

```c
xui_terminal_desc_t desc;
xui_widget terminal;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pFont = font;
desc.iColumns = 80;
desc.iRows = 24;
desc.iScrollbackLimit = 1000;

xuiTerminalCreate(context, &terminal, &desc);
xuiTerminalWriteText(terminal, "\x1b[1;36mXUI Terminal\x1b[0m\r\n");
xuiTerminalFlush(terminal);
```

`xuiTerminalWrite` queues bytes. `xuiTerminalFlush` parses all pending data,
which is useful for tests and deterministic examples. Normal update processing
uses the configured parse budget.

## Session Boundary

```c
static void on_terminal_data(xui_widget widget, const uint8_t* data, int size, void* user)
{
    /* Forward bytes to a PTY, SSH channel, or test session. */
}

xuiTerminalSetInputCallback(terminal, on_terminal_data, app);
```

For tests and examples, use the fake session:

```c
static void on_session_resize(xui_terminal_session_t* session, int cols, int rows, void* user)
{
    /* Resize the attached PTY, SSH channel, or scripted test endpoint. */
}

xui_terminal_session_desc_t session_desc;
xui_terminal_session_t* session;

memset(&session_desc, 0, sizeof(session_desc));
session_desc.iSize = sizeof(session_desc);
session_desc.bEcho = 1;
session_desc.sPrompt = "$ ";
session_desc.onResize = on_session_resize;
session_desc.pResizeUser = app;

session = xuiTerminalCreateFakeSession(&session_desc);
xuiTerminalAttachSession(terminal, session);
```

The fake session writes scripted output back into the widget and can echo input.
It is not a real process implementation.
When a session is attached, the current terminal columns and rows are delivered
through the session resize callback immediately. Later `xuiTerminalFit` and
`xuiTerminalResize` calls notify both the widget resize callback and the
attached session callback so future process or SSH adapters can keep their PTY
size in sync. `xuiTerminalSessionResize` is the session-level resize entry and
is called by `xuiTerminalResize` and `xuiTerminalFit` for attached sessions.
`xuiTerminalSessionSetResizeCallback` also delivers the current size
immediately when the session is already attached.

For simple Windows local commands, use a process session:

```c
xui_terminal_process_desc_t process_desc;
xui_terminal_session_t* process_session;

memset(&process_desc, 0, sizeof(process_desc));
process_desc.iSize = sizeof(process_desc);
process_desc.sCommandLine = "cmd.exe /d /c echo hello";
process_desc.sWorkingDirectory = NULL;
process_desc.onResize = on_session_resize;
process_desc.pResizeUser = app;

process_session = xuiTerminalCreateProcessSession(&process_desc);
xuiTerminalAttachSession(terminal, process_session);
```

`xuiTerminalSessionWrite` forwards input bytes to the child stdin.
`xuiTerminalSessionPoll` performs a non-blocking stdout/stderr read and writes
available bytes into the attached terminal. Normal widget updates call poll
automatically, but tests and deterministic tools can call it directly. The
default process session is pipe-backed. To create a ConPTY-backed Windows
session, set:

```c
process_desc.iFlags = XUI_TERMINAL_PROCESS_CONPTY;
process_desc.iColumns = 80;
process_desc.iRows = 24;
```

ConPTY creation and resize propagation are implemented. Interactive shell and
full-screen terminal application parity still need dedicated validation.

## Unicode Width

Terminal uses a first-pass fixed-cell Unicode width table. CJK ranges write a
leading cell with `XUI_TERMINAL_CELL_WIDE` and a following continuation cell
with `XUI_TERMINAL_CELL_WIDE_CONT`. Cursor movement, line serialization,
search match length, selection rendering, and text-run rendering all work in
display columns so a CJK character occupies two terminal cells.

Complex grapheme clusters, emoji ZWJ sequences, and advanced shaping remain
future work tracked in the Terminal spec.

## Input And Paste

Use `xuiTerminalInputText` to emit user text through the input/session callback.
Use `xuiTerminalPasteText` for paste operations. When bracketed paste mode is
enabled, paste output is wrapped in the standard bracketed paste markers.

The widget also handles common keyboard events when it has focus. `Ctrl+C`
still emits the terminal control code; copy and paste use `Ctrl+Shift+C` and
`Ctrl+Shift+V` so they do not steal normal terminal input.

Alt printable keys emit an ESC-prefixed byte. Alt-modified arrows, Home/End,
and PageUp/PageDown use xterm-style modified CSI sequences such as
`ESC [ 1 ; 3 D` for Alt+Left.

## Context Menu

`xuiTerminalOpenMenu` opens the owned context menu at a screen position and
refreshes command enabled state before showing it. `xuiTerminalGetMenuWidget`
returns the owned menu for testing or integration. Right-click and XUI's
context-menu key/long-press path open the same menu.

The built-in menu includes Copy, Paste, Select All, Clear Screen, Clear
Scrollback, and Find. Find uses a single-line selection as the search query; if
there is no single-line selection but a previous query exists, it runs Find
Next with the stored query.

## Selection And Search

`xuiTerminalSelectAll`, `xuiTerminalGetSelectionText`,
`xuiTerminalCopySelection`, and `xuiTerminalClearSelection` expose selection
commands. Pointer interaction supports drag selection, double-click word
selection, triple-click row selection, and Shift extension across visible
screen and scrollback lines.

`xuiTerminalSerializeText` serializes scrollback and the visible screen as
plain text. `xuiTerminalFindText` starts a plain text search over terminal
logical lines. `xuiTerminalFindNext` and `xuiTerminalFindPrev` continue from
the current match and wrap across screen and scrollback lines. The active match
is highlighted with `iSearchHighlightColor`; `xuiTerminalGetFindMatch` returns
the current line, column, and display-column length, and `xuiTerminalClearFind`
clears the match state.

## Links

Terminal detects common URL prefixes directly in logical lines:

```c
http://
https://
file://
mailto:
```

`xuiTerminalGetLinkAt` checks a logical line and display column, copies the URL
into the caller buffer, and returns the URL byte length. It also reports the
link start column and display-column length when requested. Trailing punctuation
such as commas, periods, and closing brackets is excluded from the detected URL.

`xuiTerminalSetLinkCallback` registers the callback used when the user clicks a
detected URL. Hovering a detected URL draws an underline using
`iLinkHoverColor`.

OSC 8 hyperlinks are parsed in both BEL and ST terminated forms. Visible screen
cells store a link id, so `xuiTerminalGetLinkAt`, hover underline, and click
callbacks use the same API path as plain URL detection. When a linked row enters
scrollback, Terminal stores a compact column-level link id array beside the
scrollback text, so the same hit-test and click path continues to work for
scrollback rows.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_terminal_parser_test.bat
test_xui\build_terminal_test.bat
examples\xui_terminal\build.bat
build\xui_terminal.exe --frames 5
build_dll.bat
```

The example synthetic run should report `create=1`, `layout=1`, and
`dynamic=1`.
