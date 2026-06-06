# XUI Terminal Control Design

This document defines the planned XUI2-native `Terminal` control. The control borrows product and architecture lessons from xterm.js, but it is not API-compatible with xterm.js and does not copy the web addon model.

## Direction

`Terminal` is an XUI-native terminal frontend:

- It lives only in XUI.
- It has only one rendering system: XUI cache/proxy rendering.
- It does not expose a plugin ecosystem.
- It does not expose multiple renderer backends.
- It should be practical for local process interaction and remote SSH interaction.
- It should be good enough to run real terminal applications rather than being a colored log viewer.

The relationship is similar to the CodeEdit direction:

```text
Scintilla -> CodeEdit: learn from mature editor behavior, implement XUI-native editor.
xterm.js  -> Terminal: learn from mature terminal behavior, implement XUI-native terminal.
```

## Reference Concepts From xterm.js

Keep these concepts:

- `write(data)`: backend output enters the terminal frontend.
- `onData(data)`: user input is encoded and emitted to the backend/session.
- parser and buffer are core terminal state, not just UI text.
- resize emits terminal columns and rows to the session.
- screen buffer, scrollback, alternate screen, cursor state, selection, search, serialization, and links are core product capabilities.

Do not keep these concepts:

- DOM/WebGL/Canvas renderer switching.
- addon/plugin API.
- browser import/open lifecycle.
- web frontend-specific abstractions.

## Ownership Boundary

Terminal does not make the widget itself responsible for operating-system
terminal transports. Transport lifetime belongs to session adapters.

Not owned by the `Terminal` widget core:

- spawning `cmd.exe`, PowerShell, bash, or any shell
- ConPTY lifecycle
- SSH socket lifecycle
- serial port lifecycle
- process environment and working directory
- platform event loops

Owned by `Terminal` widget:

- terminal parser
- screen buffer
- scrollback buffer
- alternate buffer
- cursor state
- text selection
- keyboard/input encoding
- paste mode
- terminal rendering
- internal search and serialization
- resize notifications

Owned by XUI terminal session adapters:

- local process session
- SSH session
- fake/test session
- future serial or debugger console session

Current implementation note:

- `xuiTerminalCreateProcessSession` provides Windows local process adapters.
- The default adapter is pipe-backed for simple local commands.
- `XUI_TERMINAL_PROCESS_CONPTY` opts into a ConPTY-backed process session.
- Process session adapters own process creation, stdin, stdout/stderr polling,
  termination, destruction, and transport resize.
- ConPTY lifecycle, `ResizePseudoConsole` propagation, output capture, and
  interactive `cmd.exe` input/output are implemented. ConPTY uses a reader
  thread to drain pseudo-console output into the session queue, then
  `xuiTerminalSessionPoll` feeds queued bytes through the normal parser.
  Full-screen terminal application parity remains tracked separately.
- `xuiTerminalCreateSshSession` provides an OpenSSH-backed remote session
  adapter. It builds a safe OpenSSH command line from
  `xui_terminal_ssh_desc_t` and reuses the existing process/ConPTY session
  path. Terminal still does not own SSH protocol parsing, credential storage,
  host-key policy, or remote network lifecycle.

The widget talks to sessions through bytes:

```text
session output -> xuiTerminalWrite -> parser -> buffer -> render
user input -> key/text/mouse -> terminal encoder -> onData/session input
resize -> onResize/session resize
```

## Internal Modules

Initial files:

```text
src\xui_terminal.c
src\xui_terminal_core.c
src\xui_terminal_vt.c
src\xui_terminal_buffer.c
src\xui_terminal_session.c

test_xui\xui_terminal_parser_test.c
test_xui\xui_terminal_buffer_test.c
test_xui\xui_terminal_test.c

examples\xui_terminal\main.c
docs\xui\widget-terminal.md
```

If the first implementation starts in fewer files, internal sections should keep these boundaries so later splitting is mechanical.

## Core Model

```text
Terminal widget
  terminal core
    parser
    UTF-8 decoder
    terminal modes
    main buffer
    alternate buffer
    scrollback ring
    cursor state
    tab stops
    dirty row map
  XUI widget state
    content rect
    cell metrics
    scroll model
    selection state
    hover link
    context menu
    callbacks
  renderer
    background runs
    text runs
    selection overlay
    cursor overlay
    scrollbar
```

Terminal is not a `TextEdit`. Its buffer stores cells, not editable paragraphs.

## Cell And Buffer Model

Proposed cell:

```c
typedef struct xui_terminal_cell_t {
    uint32_t iCodepoint;
    uint32_t iFgColor;
    uint32_t iBgColor;
    uint32_t iFlags;
    uint16_t iStyle;
    uint8_t iWidth;
} xui_terminal_cell_t;
```

Potential style flags:

```text
XUI_TERMINAL_CELL_BOLD
XUI_TERMINAL_CELL_ITALIC
XUI_TERMINAL_CELL_UNDERLINE
XUI_TERMINAL_CELL_INVERSE
XUI_TERMINAL_CELL_DIM
XUI_TERMINAL_CELL_BLINK
XUI_TERMINAL_CELL_WIDE
XUI_TERMINAL_CELL_WIDE_CONT
```

Buffer state:

```text
cols
rows
main screen rows
alternate screen rows
scrollback ring
scrollback row link id arrays
cursor x/y
saved cursor
current attributes
current OSC 8 link id and visible-cell link ids
scroll region
tab stops
wrap mode
origin mode
insert mode
application cursor/keypad modes
dirty rows
```

The public API should expose stable queries only. Internal cell storage can change later.

## Parser Scope

V1 should support a useful ANSI/VT/xterm subset:

- printable UTF-8
- C0 controls: BEL, BS, HT, LF, CR
- ESC: save/restore cursor and basic mode controls
- CSI cursor movement: CUU, CUD, CUF, CUB, CUP
- CSI erase: ED, EL
- CSI SGR: reset, bold, underline, inverse, 8/16 color, 256 color, true color
- scrolling and scroll region
- show/hide cursor
- main buffer and alternate buffer
- bracketed paste mode
- title change event if OSC title is received
- OSC 8 hyperlinks using BEL and ST terminators, with scrollback hit metadata
- practical xterm mouse tracking modes for press, release, drag, any-motion,
  wheel, focus reports, and DEC highlight protocol reports
- iTerm2-style inline image requests through `OSC 1337;File=options:payload`,
  reported to the application through a synchronous image callback
- DCS Sixel payloads terminated by ST, reported through the same image callback

Deferred:

- full DEC private mode matrix beyond the implemented practical set
- terminal-native image raster decoding and placement beyond the callback bridge
- complex grapheme clustering beyond the current first-pass fixed-cell support

The parser should be tested independently from the widget.

## Write Queue And Flow

`xuiTerminalWrite` should append data to an input queue. Processing may happen immediately for small input, but the design must allow budgeted parsing in `xuiUpdate` to avoid UI stalls when command output is large.

Public direction:

```c
xuiTerminalWrite(pWidget, pData, iSize);
xuiTerminalWriteText(pWidget, sText);
xuiTerminalFlush(pWidget);
xuiTerminalSetParseBudget(pWidget, iMaxBytesPerFrame, fMaxSecondsPerFrame);
```

The exact V1 implementation may process synchronously first, but API and internal architecture should not prevent budgeted processing later.

## Input Encoding

User input is encoded into terminal bytes and emitted through a callback. The widget should not directly write to a process.

Input cases:

- normal text -> UTF-8
- Enter -> CR or CRLF by configuration
- Backspace -> DEL or BS by configuration
- Tab -> HT
- arrows/home/end/page keys -> escape sequences
- Ctrl/Alt combinations -> terminal sequences/control bytes
- paste -> plain paste or bracketed paste
- pointer input -> terminal mouse protocol bytes when DEC/xterm mouse tracking
  modes are enabled

Potential callback:

```c
typedef void (*xui_terminal_data_proc)(xui_widget_t* pWidget, const uint8_t* pData, int iSize, void* pUser);
```

The session adapter connects this callback to local process input, SSH channel input, fake session input, or another byte sink.

## Session Adapter

`TerminalSession` is not a plugin system. It is XUI's narrow internal byte-stream interface for terminal transport.

Planned session families:

```text
fake session: test/example echo and scripted output
local process session: pipe-backed command adapter and opt-in ConPTY shell adapter
ssh session: OpenSSH-backed remote terminal adapter
```

Proposed public direction:

```c
typedef struct xui_terminal_session_t xui_terminal_session_t;

xuiTerminalAttachSession(pWidget, pSession);
xuiTerminalDetachSession(pWidget);
xuiTerminalSessionWrite(pSession, pData, iSize);
xuiTerminalCreateFakeSession(...);
xuiTerminalCreateProcessSession(...);
xuiTerminalBuildSshCommand(...);
xuiTerminalSessionPoll(...);
xuiTerminalSessionIsRunning(...);
xuiTerminalSessionTerminate(...);
xuiTerminalSessionResize(...);
xuiTerminalCreateSshSession(...);
```

The core widget should remain useful without any concrete session by allowing `xuiTerminalWrite` and `onData` callbacks.

## Rendering

Rendering should be cache-first with dirty rows.

Rendering order:

```text
terminal background
visible row background runs
visible row text runs
selection overlay
cursor
hover link underline
scrollbar
focus frame
```

Text drawing should merge consecutive cells with compatible style into runs:

```text
same foreground
same background
same style flags
same line
adjacent cell columns
```

The current implementation also keeps a persistent cache surface and dirty-row
map so ordinary output can redraw affected rows instead of repainting the full
terminal every time.

Text-run rendering is also the V1 ligature path. When ligatures are enabled,
compatible adjacent cells are submitted to the proxy text renderer as one string
so the font renderer can apply font-level ligatures. `xuiTerminalSetLigaturesEnabled`
can disable this behavior and force occupied cells to be drawn one cell at a
time for applications that require strict per-cell glyph output.

The first renderer uses existing proxy functions:

- rect fill
- text draw
- line draw for underline/link/focus if needed
- ScrollBar for scrollback

No new renderer backend or proxy capability is planned for V1.

## Font And Cell Metrics

Terminal should prefer monospaced fonts. V1 can accept any font, but cell metrics should be computed as fixed-width terminal cells.

Metrics:

```text
cell width
cell height
baseline
cols = floor(viewport width / cell width)
rows = floor(viewport height / cell height)
```

`xuiTerminalFit` is built in, not an addon:

```c
xuiTerminalFit(pWidget);
xuiTerminalGetColumns(pWidget);
xuiTerminalGetRows(pWidget);
```

When cols/rows change, the widget emits a resize callback so session adapters can resize the underlying PTY or SSH terminal.

## Selection, Clipboard, And Menu

Selection behavior:

- drag selects rectangular text range in terminal coordinates
- double-click selects word
- triple-click selects row
- Shift extends selection
- selection survives right-click menu
- selection is independent from terminal cursor

Clipboard:

- Copy exports selected text.
- Paste emits input bytes.
- Bracketed paste wraps pasted content when enabled by terminal mode.

Menu:

```text
Copy
Paste
Select All
Clear Screen
Clear Scrollback
Find
```

Menu titles should follow the shared input menu style where practical, but terminal may need terminal-specific commands.

## Built-In Features

These are internal capabilities, not addons:

```text
Fit: derive cols/rows from widget size
Search: find text in screen and scrollback
Serialize: export screen/scrollback text
Links: detect plain URLs and OSC 8 links
Unicode: width table, CJK wide-cell support, and first-pass combining marks
Ligatures: font-level ligature opportunity through compatible text runs, with an opt-out API
Mouse tracking: DECSET-controlled terminal mouse reports, including highlight mode
Inline images: iTerm2 OSC 1337 metadata and payload callback
Sixel: DCS metadata and payload callback
```

Public API direction:

```c
xuiTerminalFindText
xuiTerminalFindNext
xuiTerminalFindPrev
xuiTerminalSerializeText
xuiTerminalGetSelectionText
xuiTerminalSetLinkCallback
xuiTerminalSetLigaturesEnabled
xuiTerminalGetLigaturesEnabled
xuiTerminalSetUnicodeMode
```

## Unicode And Grapheme Handling

The terminal buffer is still fixed-cell. Base codepoints occupy one or two
cells according to the width table, while combining marks and variation
selectors attach to the previous visible cell and do not advance the cursor.
The cell stores a bounded suffix list so rendering, serialization, search, and
scrollback can preserve common composed text such as `e + U+0301`. The suffix
list is intentionally sized for common script marks while still avoiding
per-cell heap allocation. If a sequence exceeds the fixed
suffix capacity, the cell keeps the stored prefix and sets
`XUI_TERMINAL_CELL_COMBINING_OVERFLOW` so callers can detect truncation.

Common script mark ranges are treated as suffixes for fixed-cell storage and
search. The first-pass table covers Hebrew, Arabic, Cyrillic marks, Syriac,
NKo, Devanagari, Bengali-family Indic scripts, Thai/Lao, Tibetan, Myanmar,
Khmer, and related ranges. This preserves the byte stream and terminal column
behavior for common composed script text, but it is not a full shaping engine
and does not perform Indic glyph reordering or bidirectional layout. Zero-width
format controls such as ZWSP, ZWNJ, WORD JOINER, and BOM are also stored as
suffixes so they preserve serialization and search text without claiming a
terminal cell.

Hangul Jamo clusters are handled in the same fixed-cell model. Leading Jamo
occupy a double-width terminal cell; medial and final Jamo attach as suffixes,
so common decomposed Hangul syllables serialize and search as a single
double-width terminal cluster.

When a combining mark or joiner arrives without a previous visible cell,
Terminal writes U+25CC dotted circle plus the suffix codepoint as a visible
fallback. This keeps the byte stream inspectable and avoids silently dropping
text.

Full grapheme cluster segmentation remains future work. Full Unicode grapheme
boundary rules, Indic and Thai glyph reordering/shaping, bidirectional text,
and typography shaping beyond the proxy/font text renderer still require a
shaping layer beyond the fixed-cell V1 renderer.

## Style Properties

```text
terminal.background.color
terminal.foreground.color
terminal.cursor.color
terminal.selection.color
terminal.selection_text.color
terminal.link.color
terminal.scrollbar.size
terminal.cell.width
terminal.cell.height
terminal.padding
terminal.focus.color
font.name
```

Color table customization:

```c
xuiTerminalSetPalette(pWidget, arrColors, iCount);
xuiTerminalGetPalette(pWidget, arrColors, iCapacity);
```

## Example Plan

V1 example should not require a real shell. It should use a fake session that demonstrates:

- colored ANSI output
- cursor movement
- clear screen
- progress-line rewrite
- scrollback
- selection/copy
- keyboard input echo
- resize event reporting

Later examples:

```text
examples\xui_terminal_local
examples\xui_terminal_ssh
```

Local and SSH examples should be adapter examples, not required to validate the core widget.

## Test Plan

Tests should cover:

- parser control sequences
- SGR color/style parsing
- buffer wrapping and scrolling
- alternate buffer switching
- resize behavior
- input encoding
- xterm mouse reports
- selection extraction
- search and serialization
- widget render smoke through test proxy
- callback dispatch for input and resize

## Deferred

- Terminal-native image raster decoding and placement beyond the protocol callback bridge.
- Complex grapheme clusters.
- Dedicated glyph atlas renderer beyond the current dirty-row/text-run path.
- XSON description.
