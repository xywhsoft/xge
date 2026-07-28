# XUI CodeEdit

CodeEdit is the XUI2 code editor widget. It is not a Scintilla-compatible API
layer; it provides the same class of editor capabilities through XUI-native
documents, selections, rendering, providers, commands, menus, and theme objects.

The first built-in language target is C. Applications can still provide custom
tokens, fold ranges, commands, completion, hover, and signature data through the
provider and buffer APIs.

## Goals

- provide an IDE-capable source editor inside normal XUI layouts
- keep document, selection, command, lexer, fold, annotation, provider, and theme
  models reusable outside the widget
- reuse XRT-backed regex support for search and regex lexer rules
- support XUI input, focus, pointer capture, IME, clipboard, Popup/Menu, cache
  rendering, and widget invalidation
- keep the public surface extensible without matching Scintilla message ids

## Default Visual

CodeEdit renders into a widget cache. The current widget renderer draws:

- editor background and a divider
- margin band with line numbers, marker blocks, fold header glyphs, diagnostic
  blocks, and custom margin render callbacks
- visible text lines
- primary selection fill
- primary caret
- built-in C token spans using the CodeEdit theme table
- optional whitespace and EOL markers

The monospaced fast path uses an 8 px character advance and an 18 px line
height in the current implementation. Proportional-font fallback, active-line
painting, margin event routing, diagnostic hover display, inline indicator
visuals, and deeper style property integration are tracked in the CodeEdit SPEC.

## Document Model

`xui_code_document` stores UTF-8 text with LF-normalized internal newlines. It
supports:

- set/get text
- byte and offset-range reads without flattening the document
- line count and line ranges
- offset to line/column mapping
- edit ranges
- dirty/version tracking
- undo/redo
- transaction begin/end
- file helpers where the platform layer supports them

Output EOL conversion is controlled by the document and editor EOL mode APIs.

The editing store is a gap buffer. Local typing and deletion update the gap
directly, while line offsets following a same-line edit are shifted lazily.
Use `xuiCodeDocumentGetByte` or `xuiCodeDocumentCopyRange` in renderers,
lexers, search providers, and other hot paths. `xuiCodeDocumentGetText`
materializes a contiguous snapshot and should be reserved for explicit whole
document export or compatibility code.

## Widget Creation

```c
xui_code_edit_desc_t desc;
xui_widget codeEdit;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pFont = font;
desc.sText = "int main(void) {\n\treturn 0;\n}\n";
desc.sLanguage = "c";
desc.bShowLineNumbers = 1;
desc.bShowMarkerMargin = 1;
desc.bShowFoldMargin = 1;
desc.bShowDiagnosticMargin = 1;
desc.iTabColumns = 4;
desc.iIndentColumns = 4;

xuiCodeEditCreate(context, &codeEdit, &desc);
xuiWidgetAddChild(root, codeEdit);
```

Readonly mode keeps focus, selection, copy, scrolling, context menu, and
navigation available while blocking mutations.

## Public Widget API

```c
xuiCodeEditGetType
xuiCodeEditCreate
xuiCodeEditGetDocument
xuiCodeEditGetSelection
xuiCodeEditGetTheme
xuiCodeEditGetFoldState
xuiCodeEditGetAnnotations
xuiCodeEditGetTokenBuffer
xuiCodeEditGetProviders
xuiCodeEditGetMargins
xuiCodeEditGetCommandMap
xuiCodeEditGetLanguageRegistry
xuiCodeEditGetMenuWidget
xuiCodeEditSetText
xuiCodeEditGetText
xuiCodeEditSetReadonly
xuiCodeEditIsReadonly
xuiCodeEditSetScroll
xuiCodeEditGetScroll
xuiCodeEditSetDisplayOptions
xuiCodeEditGetDisplayOptions
xuiCodeEditSetMinimap
xuiCodeEditGetMinimap
xuiCodeEditGetMinimapRect
xuiCodeEditShowCompletion
xuiCodeEditCancelCompletion
xuiCodeEditIsCompletionOpen
xuiCodeEditApplyCompletionItems
xuiCodeEditSetCompletionOptions
xuiCodeEditSetTabColumns
xuiCodeEditGetTabColumns
xuiCodeEditSetIndentColumns
xuiCodeEditGetIndentColumns
xuiCodeEditSetExpandTabs
xuiCodeEditGetExpandTabs
xuiCodeEditOpenMenu
xuiCodeEditGetLastError
```

## Editing And Selection

CodeEdit routes text input, IME composition, key commands, menu commands, and
pointer selection through the shared document and selection APIs.

Supported widget behavior currently includes:

- focus and IME candidate rectangle wiring
- text insertion from text and IME events
- Backspace and Delete commands
- Tab insertion and indentation command dispatch
- select-all command dispatch
- pointer click caret placement
- pointer drag range selection
- mouse wheel scroll updates
- context menu open and menu command dispatch
- line-comment command dispatch using the active language metadata

By default, pressing Tab inserts a real `\t` character. Set
`XUI_CODE_EDIT_EXPAND_TABS` in `xui_code_edit_desc_t.iFlags`, or call
`xuiCodeEditSetExpandTabs(widget, 1)`, to expand Tab and indent commands to
spaces. The number of spaces comes from `iIndentColumns` or
`xuiCodeEditSetIndentColumns`. `XUI_CODE_EDIT_INDENT_WITH_TABS` remains valid as
an explicit tab-indentation policy.

The underlying selection and editing APIs also cover word movement, vertical
movement, line/document movement, select word, select line, indent, outdent, line
comment toggling, and readonly rejection.

## Display Options

Display options are bit flags:

```c
XUI_CODE_EDIT_SHOW_WHITESPACE
XUI_CODE_EDIT_SHOW_EOL
XUI_CODE_EDIT_SHOW_INDENT_GUIDES
XUI_CODE_EDIT_SHOW_MINIMAP
```

Use:

```c
xuiCodeEditSetDisplayOptions(
    codeEdit,
    XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL);
```

The renderer draws markers for spaces, tabs, EOL, and indentation guides. The
minimap reserves a configurable strip to the right of the text viewport,
samples document lines in O(viewport height), paints the visible range, and
supports click/drag navigation. When enabled, it replaces the regular vertical
scrollbar. It spans the full editor height and joins the horizontal scrollbar
at the lower-right edge.

## Context Menu

Each CodeEdit owns an internal XUI Menu. The menu can be opened by right-click,
context-menu events, or `xuiCodeEditOpenMenu`.

Default command groups:

- Undo, Redo
- Cut, Copy, Paste, Delete
- Select All
- Find, Replace, Go To
- Toggle Comment, Toggle Fold

Menu item enabled state follows readonly and selection state, then committed menu
items execute through the CodeEdit command path.

## Syntax And Tokens

The built-in C lexer recognizes:

- keywords and types
- identifiers
- numbers
- strings and chars
- comments
- preprocessor directives
- operators and braces
- lexer errors

Applications can provide tokens through `xui_code_token_buffer` or register
custom language providers. Theme token mapping is independent from lexer token
kinds, so applications can remap or restyle tokens without changing lexers.

Each CodeEdit owns a language registry. Defaults include C metadata and apps can
register custom language definitions through `xuiCodeEditGetLanguageRegistry`.
The widget command path reads `sLineComment` from the active language id in the
create descriptor.

## Search, Replace, And Annotations

CodeEdit foundation APIs include:

- plain-text search
- XRT regex search
- plain replace-all
- regex replace-all with capture expansion
- search result indicators
- markers
- range indicators
- diagnostics

Widget-level visual painting is partially implemented: margins render marker and
diagnostic blocks, while inline indicator layers and diagnostic hover/click UI
remain tracked in the SPEC.

## Providers

`xui_code_provider_set` stores application hooks for:

- completion
- hover
- signature help
- command interception

Completion is presented by an editor-owned Popup/ListView and does not steal
focus from CodeEdit. `Ctrl+Space` opens it manually; automatic opening is
debounced and configurable. Candidates support separate label, insert, filter,
sort, detail, documentation, and commit-character fields. CodeEdit filters and
sorts candidates, supports Up/Down, PageUp/PageDown, Home/End, Enter/Tab and
Escape, and exposes the selected candidate as inline virtual text. Multi-line
inline text occupies virtual rows and typed matching prefixes are consumed
without discarding the remaining suggestion.

Expensive providers can compute candidates off the UI thread and post the
result back to the UI thread with `xuiCodeEditApplyCompletionItems`. The
document version and request offset are validated before opening the popup, so
results produced for older text are rejected with `XUI_ERROR_UNSUPPORTED`.

## Style And Theme

Use `xuiCodeThemeSetDefault`, `xuiCodeThemeSetStyle`,
`xuiCodeThemeGetStyle`, `xuiCodeThemeMapTokenKind`, and
`xuiCodeThemeGetTokenStyle` to configure token colors and editor styles.

Style ids include default text, C token categories, selection, current line,
search result, matched brace, margin, line number, marker, and diagnostic
severities.

Current XUI style properties:

```text
codeedit.text.color
codeedit.text.readonly_color
codeedit.background.color
codeedit.divider.color
codeedit.selection.color
codeedit.caret.color
codeedit.whitespace.color
font.name
```

These properties resolve through the normal XUI style system and invalidate the
widget cache/render path. Token style table properties, layout metrics, and full
`codeedit.*` coverage are still tracked in the SPEC.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_code_document_test.bat
test_xui\build_code_lexer_test.bat
test_xui\build_code_fold_test.bat
test_xui\build_code_layout_test.bat
test_xui\build_code_theme_test.bat
test_xui\build_code_command_test.bat
test_xui\build_code_search_test.bat
test_xui\build_code_annotation_test.bat
test_xui\build_code_language_test.bat
test_xui\build_code_token_buffer_test.bat
test_xui\build_code_fold_state_test.bat
test_xui\build_code_provider_test.bat
test_xui\build_code_margin_test.bat
test_xui\build_code_search_indicator_test.bat
test_xui\build_code_selection_test.bat
test_xui\build_code_editing_test.bat
test_xui\build_code_edit_test.bat
build_dll.bat
examples\xui_codeedit\build.bat
build\xui_codeedit.exe --frames 3
```

## Current Scope

The widget is functional but not complete. It now has a gap-buffer document,
range-based hot paths, XUI ScrollBar composition, and an integrated minimap.
Remaining tracked areas include proportional-font refinement and deferred
IDE-grade capabilities such as diff view, block selection, semantic token
providers, file-backed paging, and asynchronous language-server request
adapters.
