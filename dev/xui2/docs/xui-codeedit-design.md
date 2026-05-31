# XUI CodeEdit Control Design

This document defines the complete XUI2 CodeEdit control. CodeEdit is not a
Scintilla compatibility layer. It is an XUI-native code editing system that
recreates the important IDE-grade capabilities of Scintilla while fitting the
XUI2 widget, cache, proxy, input, style, and test architecture.

The design goal is to make XUI capable of building a complete IDE without
binding future tools to an unstable text editor core. Implementation can be
delivered in small commits, but the public model, extension points, file
layout, and behavioral contracts should be settled up front.

## 1. Goals

- provide a first-class code editing widget for XUI-based IDEs and tools
- support large editable UTF-8 documents through an editor-owned document model
- provide line numbers, margins, markers, indicators, diagnostics, folding,
  selection, caret, scrolling, search, replace, command binding, and menu hooks
- support a built-in C language mode as the first shipped language mode
- provide complete custom language support through callback lexers, regex
  lexers, style maps, fold providers, and externally supplied token buffers
- reuse XRT facilities where practical, especially memory, files, path helpers,
  UTF conversion, hashing, string helpers, and regex
- preserve the XUI2 boundary: XUI owns UI state and rendering; platform input,
  windowing, file watching, and final present stay outside XUI
- avoid relying on native child windows or Scintilla source code

## 2. Non-Goals

- do not expose Scintilla message ids or compatibility macros
- do not embed a platform editor window
- do not require LSP, debugger, or project-system code in XUI core
- do not make file I/O mandatory for the editor core
- do not make every rendered line a child widget
- do not require a full C parser for the first built-in language mode
- do not implement a browser-like document layout engine inside CodeEdit

## 3. Product Scope

CodeEdit is a standard XUI2 widget plus a small set of editor model objects.

```text
xuiCodeDocument
  text storage, line index, edit transactions, undo/redo, versioning

xuiCodeLexer
  tokenization providers, regex rule lexer, built-in C lexer

xuiCodeFold
  fold range provider, fold state, visible-line mapping

xuiCodeView
  caret, selections, scroll, hit testing, visible line layout

xuiCodeMargin
  line number, marker, fold, change, diagnostic, and custom margins

xuiCodeDecoration
  markers, indicators, diagnostics, search results, current-line highlight

xuiCodeCommandMap
  commands, key bindings, context menu actions

xuiCodeTheme
  token style, margin style, indicator style, editor metrics

xuiCodeEdit widget
  event routing, rendering, composition, public widget integration
```

The editor must be useful for:

- source code editing
- IDE document tabs
- script editors
- shader editors
- configuration editors
- debugger source views
- generated-code preview panes
- read-only log/source viewers

## 4. File Layout

Recommended initial files:

```text
dev/xui2/src/xui_code_document.c
dev/xui2/src/xui_code_lexer.c
dev/xui2/src/xui_code_lexer_c.c
dev/xui2/src/xui_code_fold.c
dev/xui2/src/xui_code_layout.c
dev/xui2/src/xui_code_command.c
dev/xui2/src/xui_code_theme.c
dev/xui2/src/xui_code_edit.c
dev/xui2/test_xui/xui_code_document_test.c
dev/xui2/test_xui/xui_code_lexer_test.c
dev/xui2/test_xui/xui_code_edit_test.c
dev/xui2/test_xui/build_code_document_test.bat
dev/xui2/test_xui/build_code_lexer_test.bat
dev/xui2/test_xui/build_code_edit_test.bat
dev/xui2/examples/xui_codeedit/main.c
dev/xui2/examples/xui_codeedit/build.bat
dev/xui2/docs/xui/widget-codeedit.md
```

`xui.h` owns the public API. Internal structs live in the `.c` files and
shared internal helpers go in `xui_internal.h` only when more than one widget
needs them.

## 5. Naming

Public control name:

```text
CodeEdit
```

Public C prefix:

```text
xuiCodeEdit*
xuiCodeDocument*
xuiCodeLexer*
xuiCodeTheme*
xuiCodeCommand*
```

Widget type name:

```text
codeedit
```

File names use `codeedit` for examples/docs and `code_edit` for source files
when following XUI2 C file naming:

```text
examples/xui_codeedit
src/xui_code_edit.c
```

## 6. XRT Usage

CodeEdit should reuse XRT where it is already a good fit:

| Area | XRT capability |
| --- | --- |
| memory | `xrtMalloc`, `xrtRealloc`, `xrtFree` |
| files | `xrtFileReadAll`, `xrtFileWriteAll`, async variants when needed |
| path helpers | `xrtPathGetExt`, `xrtPathGetName`, `xrtPathJoin` |
| UTF conversion | `xrtUTF8to16`, `xrtUTF8to32`, `xrtIsUTF8` |
| hashing | `xrtHash32`, `xrtHash64` |
| string helpers | `xrtStrComp`, `xrtStrLike`, `xrtStrSim`, `xrtStrView` |
| regex | `xrtRegex*`, `xrtRegexSet*`, `xregexspan` |

Regex support must be guarded by `#ifndef XRT_NO_REGEX`. If regex is disabled,
CodeEdit still builds, but regex lexer rules and regex search return an
unsupported result.

XRT file helpers are optional convenience APIs. The document core should not
read or write files by itself unless the caller invokes explicit load/save
helpers.

## 7. Core Data Model

### 7.1 Positions

Internal positions use UTF-8 byte offsets because the surrounding XUI2 text
editing path already works with UTF-8 text and byte-based insertion. Line and
column APIs are derived from the line index.

```c
typedef struct xui_code_pos_t {
    int iOffset;       /* UTF-8 byte offset */
    int iLine;         /* zero-based physical line */
    int iColumn;       /* visual column after tab expansion when requested */
} xui_code_pos_t;

typedef struct xui_code_range_t {
    int iStart;        /* inclusive UTF-8 byte offset */
    int iEnd;          /* exclusive UTF-8 byte offset */
} xui_code_range_t;
```

Rules:

- offsets must always land on UTF-8 codepoint boundaries
- invalid offsets are clamped to valid boundaries
- line numbers are zero-based in C APIs
- visual columns are computed by layout and depend on tab width
- document columns are byte/codepoint based and must not be confused with
  visual columns

### 7.2 Newlines

The document stores normalized newlines internally.

```text
XUI_CODE_EOL_LF
XUI_CODE_EOL_CRLF
XUI_CODE_EOL_CR
XUI_CODE_EOL_AUTO
```

Default load behavior:

- detect first dominant line ending
- normalize internal storage to LF
- remember preferred output EOL
- preserve final-newline state

Paste behavior:

- normalize incoming CRLF/CR to LF before insertion
- report the edit as one transaction

Save/export behavior:

- convert LF to configured output EOL
- optionally ensure final newline

### 7.3 Text Storage

CodeEdit should use a piece-table document model from the first implementation.

```text
original buffer: immutable text supplied by SetText/Load
append buffer:   inserted text
pieces:          ordered slices into original or append buffer
line index:      physical lines derived from pieces
```

Rationale:

- fast insertion and deletion for editor workloads
- efficient undo/redo because edits record piece operations
- original text does not need full copy-on-edit churn
- later migration to rope remains possible without changing public APIs

Required document operations:

```c
xuiCodeDocumentCreate
xuiCodeDocumentDestroy
xuiCodeDocumentSetText
xuiCodeDocumentGetText
xuiCodeDocumentGetLength
xuiCodeDocumentGetLineCount
xuiCodeDocumentGetLineRange
xuiCodeDocumentGetLineText
xuiCodeDocumentOffsetToLineColumn
xuiCodeDocumentLineColumnToOffset
xuiCodeDocumentInsert
xuiCodeDocumentDelete
xuiCodeDocumentReplace
xuiCodeDocumentBeginEdit
xuiCodeDocumentEndEdit
xuiCodeDocumentUndo
xuiCodeDocumentRedo
xuiCodeDocumentCanUndo
xuiCodeDocumentCanRedo
xuiCodeDocumentGetVersion
xuiCodeDocumentGetChangeVersion
xuiCodeDocumentGetDirty
xuiCodeDocumentSetDirty
```

### 7.4 Line Index

The line index is a first-class cache:

```c
typedef struct xui_code_line_info_t {
    int iStartOffset;
    int iEndOffset;
    int iTextEndOffset;    /* excludes newline bytes */
    int iFlags;
} xui_code_line_info_t;
```

Line index requirements:

- line count query is O(1)
- offset to line lookup is O(log n) or better
- edits invalidate only affected line index ranges when possible
- each line can carry flags for modified, saved, folded, diagnostic, and dirty
  lexer state
- line ids must be stable enough for markers to survive local edits

### 7.5 Edit Transactions

All mutations are transactions. A transaction may contain one or more primitive
piece edits.

```text
single character typed
paste block
indent selection
replace all
format range
external document reload
```

A transaction records:

- before/after selection
- before/after caret
- affected byte range
- affected line range
- inserted/deleted text or piece operation list
- timestamp or merge group for typing coalescing
- command id that caused the edit

Undo/redo must restore text, caret, selections, scroll anchor when appropriate,
dirty state, and lexer/fold invalidation ranges.

## 8. Selections and Carets

The first public implementation should fully support one primary selection and
design for future multi-selection.

```c
#define XUI_CODE_SELECTION_PRIMARY 0

typedef struct xui_code_selection_t {
    int iAnchorOffset;
    int iCaretOffset;
    int iPreferredColumn;
    int iFlags;
} xui_code_selection_t;
```

Selection flags:

```text
XUI_CODE_SELECTION_RECT
XUI_CODE_SELECTION_REVERSED
XUI_CODE_SELECTION_INACTIVE
```

The API must not assume there will only ever be one selection:

```c
xuiCodeEditSetSelection
xuiCodeEditGetSelection
xuiCodeEditGetSelectionCount
xuiCodeEditGetSelectionAt
xuiCodeEditClearSelections
xuiCodeEditAddSelection
```

V1 behavior can cap selection count to 1 and return unsupported for additional
selections, but struct and command design must be multi-selection ready.

## 9. Commands

Every editor action routes through a command id. Key handling should translate
input into commands, and commands mutate editor state.

Command families:

```text
navigation
selection
editing
clipboard
indentation
commenting
search
replace
folding
marker
diagnostic
view
completion
debug integration
```

Core command ids:

```text
XUI_CODE_COMMAND_MOVE_LEFT
XUI_CODE_COMMAND_MOVE_RIGHT
XUI_CODE_COMMAND_MOVE_UP
XUI_CODE_COMMAND_MOVE_DOWN
XUI_CODE_COMMAND_MOVE_WORD_LEFT
XUI_CODE_COMMAND_MOVE_WORD_RIGHT
XUI_CODE_COMMAND_MOVE_LINE_START
XUI_CODE_COMMAND_MOVE_LINE_END
XUI_CODE_COMMAND_MOVE_DOCUMENT_START
XUI_CODE_COMMAND_MOVE_DOCUMENT_END
XUI_CODE_COMMAND_SELECT_LEFT
XUI_CODE_COMMAND_SELECT_RIGHT
XUI_CODE_COMMAND_SELECT_UP
XUI_CODE_COMMAND_SELECT_DOWN
XUI_CODE_COMMAND_SELECT_ALL
XUI_CODE_COMMAND_DELETE_BACK
XUI_CODE_COMMAND_DELETE_FORWARD
XUI_CODE_COMMAND_DELETE_WORD_BACK
XUI_CODE_COMMAND_DELETE_WORD_FORWARD
XUI_CODE_COMMAND_INSERT_NEWLINE
XUI_CODE_COMMAND_INSERT_TAB
XUI_CODE_COMMAND_INDENT
XUI_CODE_COMMAND_OUTDENT
XUI_CODE_COMMAND_TOGGLE_LINE_COMMENT
XUI_CODE_COMMAND_COPY
XUI_CODE_COMMAND_CUT
XUI_CODE_COMMAND_PASTE
XUI_CODE_COMMAND_UNDO
XUI_CODE_COMMAND_REDO
XUI_CODE_COMMAND_FIND_NEXT
XUI_CODE_COMMAND_FIND_PREVIOUS
XUI_CODE_COMMAND_REPLACE_NEXT
XUI_CODE_COMMAND_FOLD_TOGGLE
XUI_CODE_COMMAND_FOLD_ALL
XUI_CODE_COMMAND_UNFOLD_ALL
XUI_CODE_COMMAND_GOTO_LINE
XUI_CODE_COMMAND_SHOW_COMPLETION
XUI_CODE_COMMAND_SHOW_SIGNATURE_HELP
```

Custom command ids should be accepted above an application range:

```text
XUI_CODE_COMMAND_USER_BASE = 10000
```

Command callback:

```c
typedef int (*xui_code_command_proc)(
    xui_widget pCodeEdit,
    int iCommand,
    const void* pCommandData,
    void* pUser);
```

Return values:

```text
0 = not handled
1 = handled
2 = handled and request repaint
3 = handled and text changed
```

## 10. Key Bindings

CodeEdit owns a command map:

```c
typedef struct xui_code_key_binding_t {
    int iKey;
    uint32_t iModifiers;
    int iCommand;
    int iWhen;
} xui_code_key_binding_t;
```

Default key bindings should match common desktop editor behavior:

- arrows move caret
- Shift plus arrows extends selection
- Ctrl+A/C/X/V/Z/Y work as expected
- Ctrl+F opens find callback
- Ctrl+H opens replace callback
- Ctrl+/ toggles line comment if the language defines a line comment token
- F3/Shift+F3 find next/previous
- F9 toggles breakpoint marker through command hook
- Ctrl+G opens go-to-line callback

Applications can override or remove bindings:

```c
xuiCodeEditBindKey
xuiCodeEditUnbindKey
xuiCodeEditClearKeyBindings
xuiCodeEditLoadDefaultKeyBindings
```

## 11. Input, Clipboard, and IME

CodeEdit subscribes to:

```text
pointer move/down/up/wheel/double click/context menu
key down/up
text input
IME composition
focus in/out
command
```

Clipboard uses the XUI proxy clipboard service already used by Input and
TextEdit.

IME requirements:

- enable IME when CodeEdit has focus and is editable
- keep IME enabled for readonly mode only if composition is explicitly allowed
- candidate rect follows the active caret after scroll and layout
- composition text is drawn at the caret without committing to the document
- committing composition becomes one edit transaction

Pointer behavior:

- click places caret
- drag selects text
- double click selects word
- triple click selects line
- margin click triggers margin-specific action
- right click opens CodeEdit context menu without losing selection
- wheel scrolls vertically
- Shift+wheel or horizontal wheel scrolls horizontally

## 12. Layout and Scroll Model

CodeEdit must not create one widget per line. It renders rows directly inside
the viewport, following ListView/TreeView/TableView patterns.

Internal structure:

```text
CodeEdit root widget
  internal ScrollFrame
    viewport widget
      cache-rendered visible editor surface
    horizontal ScrollBar
    vertical ScrollBar
  optional overlay children from popup/menu providers
```

The control owns:

```text
content width
content height
line height
char advance cache
margin widths
viewport rect
text rect
visible physical line range
visible visual row range
scroll offset
```

Visible range calculation must account for:

- physical lines
- folded lines
- soft-wrapped visual rows
- variable line height reserved for future inline widgets or diagnostics
- horizontal scroll
- margin widths

Scroll APIs:

```c
xuiCodeEditSetScroll
xuiCodeEditGetScroll
xuiCodeEditScrollBy
xuiCodeEditEnsureOffsetVisible
xuiCodeEditEnsureLineVisible
xuiCodeEditGetVisibleLineRange
xuiCodeEditGetLineRect
xuiCodeEditGetTextRect
xuiCodeEditHitTest
```

## 13. Rendering Architecture

CodeEdit should use layered rendering inside its widget cache:

```text
background layer
margin background layer
current line layer
selection layer
indicator background layer
text token layer
indicator foreground layer
caret/IME layer
margin content layer
overlay adornment layer
```

The first implementation can draw into one viewport cache, but code should be
organized by layers so later partial caches can be added without changing
public behavior.

Recommended cache granularity:

```text
root cache: border/background and static frame
viewport cache: visible rows and margins
optional future line tile cache: groups of visible rows
```

Dirty categories:

```text
XUI_CODE_DIRTY_TEXT
XUI_CODE_DIRTY_LINE_INDEX
XUI_CODE_DIRTY_LAYOUT
XUI_CODE_DIRTY_LEXER
XUI_CODE_DIRTY_FOLD
XUI_CODE_DIRTY_MARKER
XUI_CODE_DIRTY_INDICATOR
XUI_CODE_DIRTY_SELECTION
XUI_CODE_DIRTY_CARET
XUI_CODE_DIRTY_SCROLL
XUI_CODE_DIRTY_STYLE
XUI_CODE_DIRTY_MARGIN
```

Rendering must only ask lexer/providers for visible or dirty ranges.

## 14. Text Metrics

CodeEdit is optimized for monospaced code fonts but should not hard fail for
proportional fonts.

Metrics:

```text
font ascent/descent/line height
space advance
tab width in columns
line gap
caret width
margin padding
fold icon size
marker icon size
```

For monospaced fonts:

- cache average char advance from space or `M`
- compute x positions cheaply

For proportional fonts:

- measure line prefixes or token spans through proxy text measurement
- cache per visible line layout

The default should be monospaced metrics because CodeEdit is an IDE control.

## 15. Syntax and Lexer System

### 15.1 Token Model

```c
typedef struct xui_code_token_t {
    int iStartOffset;
    int iEndOffset;
    int iStyle;
    int iKind;
    int iFlags;
} xui_code_token_t;
```

Token kinds:

```text
XUI_CODE_TOKEN_TEXT
XUI_CODE_TOKEN_KEYWORD
XUI_CODE_TOKEN_TYPE
XUI_CODE_TOKEN_IDENTIFIER
XUI_CODE_TOKEN_NUMBER
XUI_CODE_TOKEN_STRING
XUI_CODE_TOKEN_CHAR
XUI_CODE_TOKEN_COMMENT
XUI_CODE_TOKEN_PREPROCESSOR
XUI_CODE_TOKEN_OPERATOR
XUI_CODE_TOKEN_BRACE
XUI_CODE_TOKEN_ERROR
XUI_CODE_TOKEN_CUSTOM_BASE
```

Token style id is separate from token kind. This lets one lexer map several
token kinds to the same visual style or applications remap styles.

### 15.2 Lexer Providers

CodeEdit supports three language input paths:

1. callback lexer
2. regex rule lexer
3. external token buffer

Callback lexer:

```c
typedef int (*xui_code_lexer_proc)(
    xui_code_document pDocument,
    int iStartLine,
    int iEndLine,
    xui_code_token_t* pTokens,
    int iTokenCapacity,
    int* pTokenCount,
    void* pUser);
```

Regex lexer rule:

```c
typedef struct xui_code_regex_rule_t {
    const char* sName;
    const char* sPattern;
    int iTokenKind;
    int iStyle;
    int iFlags;
} xui_code_regex_rule_t;
```

Regex lexer behavior:

- compile rules using `xrtRegexSet*` where possible
- use `xrtRegexFindAt` or `xrtRegexSetMatchesAt` for scanning
- support rule priority by rule order
- support multiline rules for comments and strings where configured
- fail gracefully with an error string when a pattern cannot compile

External token buffer:

```c
xuiCodeEditSetTokens(pWidget, pTokens, iTokenCount, iTextVersion)
```

External tokens are useful when a parser or language server owns syntax data.

### 15.3 Built-in C Lexer

The built-in C lexer should cover:

- keywords
- common primitive types
- identifiers
- integer and floating literals
- string literals
- char literals
- line comments
- block comments
- preprocessor directives
- operators
- braces and punctuation
- invalid unterminated strings/comments

The C lexer does not need semantic parsing. It should be deterministic,
incremental by line range, and good enough for an IDE source view.

### 15.4 Language Definition

```c
typedef struct xui_code_language_t {
    int iSize;
    const char* sId;
    const char* sName;
    const char* const* arrExtensions;
    int iExtensionCount;
    const char* sLineComment;
    const char* sBlockCommentStart;
    const char* sBlockCommentEnd;
    xui_code_lexer_proc onLex;
    xui_code_fold_proc onFold;
    void* pUser;
} xui_code_language_t;
```

APIs:

```c
xuiCodeEditRegisterLanguage
xuiCodeEditSetLanguage
xuiCodeEditSetLanguageByExtension
xuiCodeEditSetLexer
xuiCodeEditSetRegexLexerRules
```

## 16. Styles and Theme

CodeEdit has an editor-specific theme layered on top of the XUI style system.

```c
typedef struct xui_code_style_t {
    uint32_t iForeground;
    uint32_t iBackground;
    uint32_t iFlags;
} xui_code_style_t;
```

Style flags:

```text
XUI_CODE_STYLE_BOLD
XUI_CODE_STYLE_ITALIC
XUI_CODE_STYLE_UNDERLINE
XUI_CODE_STYLE_STRIKE
```

Built-in style ids:

```text
XUI_CODE_STYLE_DEFAULT
XUI_CODE_STYLE_KEYWORD
XUI_CODE_STYLE_TYPE
XUI_CODE_STYLE_IDENTIFIER
XUI_CODE_STYLE_NUMBER
XUI_CODE_STYLE_STRING
XUI_CODE_STYLE_CHAR
XUI_CODE_STYLE_COMMENT
XUI_CODE_STYLE_PREPROCESSOR
XUI_CODE_STYLE_OPERATOR
XUI_CODE_STYLE_BRACE
XUI_CODE_STYLE_ERROR
XUI_CODE_STYLE_SELECTION
XUI_CODE_STYLE_CURRENT_LINE
XUI_CODE_STYLE_SEARCH_RESULT
XUI_CODE_STYLE_MATCHED_BRACE
XUI_CODE_STYLE_MARGIN
XUI_CODE_STYLE_LINE_NUMBER
XUI_CODE_STYLE_FOLD_ICON
XUI_CODE_STYLE_MARKER
XUI_CODE_STYLE_DIAGNOSTIC_ERROR
XUI_CODE_STYLE_DIAGNOSTIC_WARNING
XUI_CODE_STYLE_DIAGNOSTIC_INFO
XUI_CODE_STYLE_CUSTOM_BASE
```

Style properties should include:

```text
codeedit.background.color
codeedit.text.color
codeedit.text.disabled_color
codeedit.caret.color
codeedit.selection.color
codeedit.current_line.color
codeedit.margin.background.color
codeedit.margin.border.color
codeedit.line_number.color
codeedit.line_number.active_color
codeedit.fold.color
codeedit.marker.color
codeedit.indent_guide.color
codeedit.whitespace.color
codeedit.search_result.color
codeedit.brace_match.color
codeedit.diagnostic.error_color
codeedit.diagnostic.warning_color
codeedit.diagnostic.info_color
codeedit.font.name
codeedit.font.size
codeedit.line_gap
codeedit.tab_columns
```

## 17. Margins

Margins are ordered columns beside the text viewport. They are not hardcoded
into one line-number gutter.

```c
typedef struct xui_code_margin_desc_t {
    int iSize;
    int iId;
    int iKind;
    float fWidth;
    int iFlags;
    xui_code_margin_render_proc onRender;
    xui_code_margin_event_proc onEvent;
    void* pUser;
} xui_code_margin_desc_t;
```

Built-in margin kinds:

```text
XUI_CODE_MARGIN_LINE_NUMBER
XUI_CODE_MARGIN_MARKER
XUI_CODE_MARGIN_FOLD
XUI_CODE_MARGIN_CHANGE
XUI_CODE_MARGIN_DIAGNOSTIC
XUI_CODE_MARGIN_CUSTOM
```

Margin APIs:

```c
xuiCodeEditAddMargin
xuiCodeEditRemoveMargin
xuiCodeEditSetMarginWidth
xuiCodeEditGetMarginRect
xuiCodeEditHitTestMargin
```

Default margin order:

```text
change margin
marker margin
fold margin
line number margin
diagnostic margin
text viewport
```

Applications can disable or reorder margins.

## 18. Markers

Markers are line-attached symbols or backgrounds. They survive edits by
tracking line identity where possible.

Common marker types:

```text
breakpoint
disabled breakpoint
current execution line
bookmark
modified line
saved modified line
error line
warning line
info line
custom
```

Marker APIs:

```c
xuiCodeEditAddMarkerType
xuiCodeEditSetMarker
xuiCodeEditClearMarker
xuiCodeEditClearMarkers
xuiCodeEditGetMarkers
xuiCodeEditSetMarkerTooltip
xuiCodeEditSetMarkerUserData
```

Marker rendering can be:

- margin icon
- margin color block
- line background
- text underline
- custom renderer

## 19. Indicators

Indicators are range-attached decorations. They correspond to diagnostics,
search results, brace matching, references, current symbol, and custom IDE
overlays.

Indicator style types:

```text
XUI_CODE_INDICATOR_SQUIGGLE
XUI_CODE_INDICATOR_UNDERLINE
XUI_CODE_INDICATOR_BOX
XUI_CODE_INDICATOR_ROUNDED_BOX
XUI_CODE_INDICATOR_BACKGROUND
XUI_CODE_INDICATOR_TEXT_FOREGROUND
XUI_CODE_INDICATOR_CUSTOM
```

Indicator APIs:

```c
xuiCodeEditAddIndicatorType
xuiCodeEditSetIndicator
xuiCodeEditClearIndicator
xuiCodeEditClearIndicators
xuiCodeEditGetIndicatorsAt
```

Indicators must be layered:

```text
background indicators
selection
text
foreground indicators
caret
```

## 20. Diagnostics

Diagnostics are structured indicators with severity, message, code, and
optional source.

```c
typedef struct xui_code_diagnostic_t {
    int iSize;
    xui_code_range_t tRange;
    int iSeverity;
    const char* sCode;
    const char* sMessage;
    const char* sSource;
} xui_code_diagnostic_t;
```

Severity:

```text
XUI_CODE_DIAGNOSTIC_ERROR
XUI_CODE_DIAGNOSTIC_WARNING
XUI_CODE_DIAGNOSTIC_INFO
XUI_CODE_DIAGNOSTIC_HINT
```

Diagnostics render as:

- margin icon
- squiggle or underline
- optional line background
- hover tooltip
- status callback for selected diagnostic

Diagnostics are supplied by applications or providers. XUI does not run
compilers or LSP by itself.

## 21. Folding

Fold state belongs to CodeEdit. Fold ranges can come from:

- built-in C fold provider
- callback fold provider
- externally supplied fold ranges

```c
typedef struct xui_code_fold_range_t {
    int iStartLine;
    int iEndLine;
    int iLevel;
    int iFlags;
} xui_code_fold_range_t;
```

Fold flags:

```text
XUI_CODE_FOLD_COLLAPSED
XUI_CODE_FOLD_HEADER
XUI_CODE_FOLD_COMMENT
XUI_CODE_FOLD_REGION
XUI_CODE_FOLD_CUSTOM
```

Built-in C folding:

- `{ ... }` blocks
- block comments
- preprocessor `#if/#ifdef/#ifndef/#else/#endif`
- optional `#pragma region` style comments if configured

Folding affects visible-line mapping, scroll content height, hit testing, and
selection movement. Folded hidden text remains in the document.

## 22. Search and Replace

Search supports:

```text
plain text
case sensitive/insensitive
whole word
regex
forward/backward
selection-only
wrap search
```

Search APIs:

```c
xuiCodeEditFind
xuiCodeEditFindNext
xuiCodeEditFindPrevious
xuiCodeEditReplace
xuiCodeEditReplaceAll
xuiCodeEditClearSearchResults
```

Regex search uses XRT regex:

- compile with `xrtRegexBuilderCreate`
- use `xrtRegexFindAt`
- use captures for replacement expansion
- return detailed pattern error from `xrtRegexGetErrorMsg`

Search result rendering uses indicators.

## 23. Completion, Hover, and Signature Help

These are IDE hooks, not mandatory built-in engines.

Completion provider:

```c
typedef int (*xui_code_completion_proc)(
    xui_widget pCodeEdit,
    int iOffset,
    const char* sPrefix,
    xui_code_completion_item_t* pItems,
    int iItemCapacity,
    int* pItemCount,
    void* pUser);
```

Hover provider:

```c
typedef int (*xui_code_hover_proc)(
    xui_widget pCodeEdit,
    int iOffset,
    xui_code_hover_t* pHover,
    void* pUser);
```

Signature help provider:

```c
typedef int (*xui_code_signature_proc)(
    xui_widget pCodeEdit,
    int iOffset,
    xui_code_signature_help_t* pHelp,
    void* pUser);
```

UI presentation should reuse XUI Popup, Menu/ListView, Tooltip, Panel, and
TextEdit where appropriate.

## 24. Context Menu

CodeEdit owns an internal Menu widget like Input/TextEdit but with code editor
commands.

Default menu groups:

```text
Undo
Redo
---
Cut
Copy
Paste
Delete
---
Select All
---
Find
Replace
Go To Line
---
Toggle Comment
Fold
Unfold
```

Applications can:

- rename menu items
- hide menu items
- add custom command items
- provide context-sensitive enable state

## 25. Public API Sketch

### 25.1 Handles

```c
typedef struct xui_code_document_t* xui_code_document;
typedef struct xui_code_theme_t* xui_code_theme;
typedef struct xui_code_language_t xui_code_language_t;
```

### 25.2 Create

```c
typedef struct xui_code_edit_desc_t {
    int iSize;
    xui_font pFont;
    const char* sText;
    const char* sLanguage;
    int bReadonly;
    int bShowLineNumbers;
    int bShowFoldMargin;
    int bShowMarkerMargin;
    int bShowDiagnosticMargin;
    int bWordWrap;
    int iTabColumns;
    int iIndentColumns;
    int iEolMode;
    int iFlags;
} xui_code_edit_desc_t;

XUI_API xui_widget_type xuiCodeEditGetType(xui_context pContext);
XUI_API int xuiCodeEditCreate(
    xui_context pContext,
    xui_widget* ppWidget,
    const xui_code_edit_desc_t* pDesc);
```

### 25.3 Document

```c
XUI_API int xuiCodeEditSetText(xui_widget pWidget, const char* sText);
XUI_API const char* xuiCodeEditGetText(xui_widget pWidget);
XUI_API int xuiCodeEditSetDocument(xui_widget pWidget, xui_code_document pDocument);
XUI_API xui_code_document xuiCodeEditGetDocument(xui_widget pWidget);
XUI_API int xuiCodeEditLoadTextFile(xui_widget pWidget, const char* sPath, int iCharset);
XUI_API int xuiCodeEditSaveTextFile(xui_widget pWidget, const char* sPath, int iCharset);
XUI_API int xuiCodeEditGetLineCount(xui_widget pWidget);
XUI_API int xuiCodeEditGetLength(xui_widget pWidget);
XUI_API int xuiCodeEditGetDirty(xui_widget pWidget);
XUI_API int xuiCodeEditSetDirty(xui_widget pWidget, int bDirty);
```

### 25.4 Editing

```c
XUI_API int xuiCodeEditInsertText(xui_widget pWidget, const char* sText);
XUI_API int xuiCodeEditDeleteRange(xui_widget pWidget, int iStart, int iEnd);
XUI_API int xuiCodeEditReplaceRange(xui_widget pWidget, int iStart, int iEnd, const char* sText);
XUI_API int xuiCodeEditUndo(xui_widget pWidget);
XUI_API int xuiCodeEditRedo(xui_widget pWidget);
XUI_API int xuiCodeEditCanUndo(xui_widget pWidget);
XUI_API int xuiCodeEditCanRedo(xui_widget pWidget);
XUI_API int xuiCodeEditBeginEdit(xui_widget pWidget);
XUI_API int xuiCodeEditEndEdit(xui_widget pWidget);
```

### 25.5 Selection and Navigation

```c
XUI_API int xuiCodeEditSetSelection(xui_widget pWidget, int iStart, int iEnd);
XUI_API int xuiCodeEditGetSelection(xui_widget pWidget, int* pStart, int* pEnd);
XUI_API int xuiCodeEditSelectAll(xui_widget pWidget);
XUI_API int xuiCodeEditGotoOffset(xui_widget pWidget, int iOffset);
XUI_API int xuiCodeEditGotoLine(xui_widget pWidget, int iLine);
XUI_API int xuiCodeEditOffsetToLineColumn(xui_widget pWidget, int iOffset, int* pLine, int* pColumn);
XUI_API int xuiCodeEditLineColumnToOffset(xui_widget pWidget, int iLine, int iColumn, int* pOffset);
```

### 25.6 Language and Style

```c
XUI_API int xuiCodeEditSetLanguage(xui_widget pWidget, const char* sLanguage);
XUI_API int xuiCodeEditSetLexer(xui_widget pWidget, xui_code_lexer_proc onLex, void* pUser);
XUI_API int xuiCodeEditSetRegexLexerRules(xui_widget pWidget, const xui_code_regex_rule_t* pRules, int iCount);
XUI_API int xuiCodeEditSetTokens(xui_widget pWidget, const xui_code_token_t* pTokens, int iCount, int iTextVersion);
XUI_API int xuiCodeEditSetTheme(xui_widget pWidget, xui_code_theme pTheme);
XUI_API int xuiCodeEditSetStyle(xui_widget pWidget, int iStyle, const xui_code_style_t* pStyle);
```

### 25.7 Margins, Markers, Indicators, Diagnostics

```c
XUI_API int xuiCodeEditAddMargin(xui_widget pWidget, const xui_code_margin_desc_t* pDesc);
XUI_API int xuiCodeEditRemoveMargin(xui_widget pWidget, int iMarginId);
XUI_API int xuiCodeEditSetMarker(xui_widget pWidget, int iLine, int iMarker, int bSet);
XUI_API int xuiCodeEditClearMarker(xui_widget pWidget, int iLine, int iMarker);
XUI_API int xuiCodeEditSetIndicator(xui_widget pWidget, int iIndicator, int iStart, int iEnd);
XUI_API int xuiCodeEditClearIndicator(xui_widget pWidget, int iIndicator, int iStart, int iEnd);
XUI_API int xuiCodeEditSetDiagnostics(xui_widget pWidget, const xui_code_diagnostic_t* pDiagnostics, int iCount);
XUI_API int xuiCodeEditClearDiagnostics(xui_widget pWidget);
```

### 25.8 Folding

```c
XUI_API int xuiCodeEditSetFoldProvider(xui_widget pWidget, xui_code_fold_proc onFold, void* pUser);
XUI_API int xuiCodeEditSetFoldRanges(xui_widget pWidget, const xui_code_fold_range_t* pRanges, int iCount);
XUI_API int xuiCodeEditToggleFold(xui_widget pWidget, int iLine);
XUI_API int xuiCodeEditFoldAll(xui_widget pWidget);
XUI_API int xuiCodeEditUnfoldAll(xui_widget pWidget);
XUI_API int xuiCodeEditIsLineVisible(xui_widget pWidget, int iLine);
```

### 25.9 Providers

```c
XUI_API int xuiCodeEditSetCompletionProvider(xui_widget pWidget, xui_code_completion_proc onComplete, void* pUser);
XUI_API int xuiCodeEditSetHoverProvider(xui_widget pWidget, xui_code_hover_proc onHover, void* pUser);
XUI_API int xuiCodeEditSetSignatureProvider(xui_widget pWidget, xui_code_signature_proc onSignature, void* pUser);
XUI_API int xuiCodeEditSetCommandHandler(xui_widget pWidget, xui_code_command_proc onCommand, void* pUser);
```

## 26. Customization Model

CodeEdit must be customizable without modifying XUI source.

Applications can customize:

- languages
- regex lexer rules
- fold ranges
- token style map
- theme
- margins
- markers
- indicators
- diagnostics
- hover/completion/signature providers
- command handlers
- key bindings
- context menu items

The built-in C language mode should be implemented through the same provider
concepts where practical. It should not get privileged rendering paths.

## 27. Integration With Other XUI Controls

CodeEdit should reuse:

- ScrollFrame and ScrollBar for visible scrollbars
- Popup for completion/signature/search panels
- Menu for context menu
- Tooltip or MsgTip-style surface for hover
- Panel/ListView for completion lists
- Input/TextEdit for search/replace fields outside the editor
- DockPanel and Tabs at the application shell level

CodeEdit must not require DockPanel or Tabs, but it must behave correctly when
embedded inside them.

## 28. XSON Direction

XSON support should be deferred until the C API is stable, but the design
should reserve simple properties:

```xson
{
  "type": "codeedit",
  "language": "c",
  "readonly": false,
  "lineNumbers": true,
  "foldMargin": true,
  "markerMargin": true,
  "diagnosticMargin": true,
  "wordWrap": false,
  "tabColumns": 4,
  "text": "int main(void) { return 0; }"
}
```

Complex providers, callbacks, and external documents should remain C API
responsibility.

## 29. Performance Requirements

Baseline targets for the first complete design:

- 100,000 physical lines loadable in memory
- visible-range rendering independent of total line count
- line count query O(1)
- offset to line mapping O(log n) or better
- simple typing should not re-lex the full document
- scrolling should only relayout visible lines plus cache margin
- search can scan full document but must expose cancel/progress hooks later
- regex search must reuse compiled regex objects for repeated find next

The first implementation may not hit all targets, but no public API should
prevent reaching them.

## 30. Error Handling

CodeEdit should expose diagnostics for internal editor errors:

```text
invalid UTF-8 input
regex compile failure
lexer provider failure
fold provider failure
unsupported regex when XRT_NO_REGEX is enabled
out-of-memory while editing
invalid style id
invalid margin id
```

APIs return `0` for failure and should set an editor-local error string:

```c
XUI_API const char* xuiCodeEditGetLastError(xui_widget pWidget);
```

Document-level APIs should have an equivalent document error query.

## 31. Verification Plan

Required unit tests:

```text
CodeDocument:
  set/get text
  CRLF/LF normalization
  line index
  offset/line/column mapping
  insert/delete/replace
  undo/redo
  transaction merge boundaries
  dirty/version tracking

Lexer:
  built-in C tokens
  comments/strings/preprocessor
  invalid unterminated ranges
  regex rule compile errors
  regex rule tokenization
  external token buffer

Folding:
  brace ranges
  preprocessor ranges
  collapsed visible-line mapping
  fold toggles after edits

CodeEdit widget:
  create/destroy
  keyboard navigation
  selection
  clipboard commands
  IME candidate rect
  scroll and ensure-visible
  line number margin
  markers
  indicators
  diagnostics
  context menu
  command binding
```

Required example checks:

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_edit_test.bat
examples\xui_codeedit\build.bat
build\xui_codeedit.exe --frames 3
```

The example should print a summary including:

```text
document=1
lexer=1
render=1
input=1
selection=1
margin=1
marker=1
indicator=1
fold=1
search=1
command=1
```

## 32. Completion Definition

CodeEdit is complete when all of the following are true:

- public API compiles from `xui.h`
- CodeDocument supports piece-table edits, line index, undo/redo, versions,
  and dirty state
- CodeEdit widget renders a C file with line numbers, syntax highlight,
  selection, caret, current line, margins, markers, indicators, and diagnostics
- keyboard and pointer editing work through command dispatch
- clipboard and IME use the existing XUI proxy contract
- built-in C lexer works
- custom callback lexer works
- custom regex lexer works when XRT regex is enabled
- fold ranges and collapsed lines work
- search and replace work for plain text and regex
- applications can add custom margins, markers, indicators, diagnostics, and
  commands
- tests and example build scripts exist and pass
- `docs/xui/widget-codeedit.md` documents user-facing API and examples

## 33. Open Design Decisions

These decisions should be resolved before implementation starts:

- exact maximum default document size before returning an error
- whether document APIs duplicate returned text or expose temporary buffers
- whether proportional-font support is best-effort or formally supported
- how much multi-selection behavior is implemented immediately
- whether regex lexer rules support named capture style mapping in the first
  version
- whether search/replace UI is part of CodeEdit or only a provider hook

None of these should change the core architecture described above.
