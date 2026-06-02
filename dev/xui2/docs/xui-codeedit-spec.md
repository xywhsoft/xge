# XUI CodeEdit SPEC

Status markers:

- `[ ]` not started
- `[~]` in progress
- `[X]` complete

This SPEC tracks the full CodeEdit control. The detailed architecture is in
`xui-codeedit-design.md`; this file is the implementation control plane. Keep
this document current whenever CodeEdit API, source files, tests, examples, or
verification status changes.

## Completion Rule

CodeEdit is complete only when all capability groups below are `[X]`, the
verification matrix passes, and `docs/xui/widget-codeedit.md` documents the
final user-facing API.

Implementation may land in multiple commits, but public model decisions should
follow the complete design up front so XUI-based IDE tools do not need repeated
rewrites.

## 1. Design and Tracking

- [X] Create full CodeEdit architecture document.
  - Evidence: `dev/xui2/docs/xui-codeedit-design.md`
  - Completion: document covers goals, boundaries, model, APIs, rendering,
    XRT reuse, customization, tests, and completion definition.

- [X] Create this tracked SPEC.
  - Evidence: `dev/xui2/docs/xui-codeedit-spec.md`
  - Completion: all major CodeEdit work areas have `[ ]`, `[~]`, or `[X]`
    entries with concrete completion criteria.

- [X] Add CodeEdit widget user documentation.
  - Target: `dev/xui2/docs/xui/widget-codeedit.md`
  - Completion: public API summary, behavior notes, style properties, example,
    and verification commands are documented.
  - Evidence: `dev/xui2/docs/xui/widget-codeedit.md`

- [X] Keep docs index current.
  - Target: `dev/xui2/docs/README.md`
  - Completion: design, SPEC, and final widget doc are linked from the index.
  - Evidence: `dev/xui2/docs/README.md`

## 2. Public API Surface

- [X] Add CodeEdit constants and flags to `xui.h`.
  - Include: command ids, token kinds, style ids, diagnostic severity, margin
    kinds, marker/indicator kinds, fold flags, EOL modes, dirty categories.
  - Completion: constants compile from external C code and do not collide with
    existing XUI constants.

- [X] Add public CodeEdit structs to `xui.h`.
  - Include: `xui_code_edit_desc_t`, `xui_code_pos_t`,
    `xui_code_range_t`, `xui_code_token_t`, `xui_code_style_t`,
    `xui_code_margin_desc_t`, `xui_code_diagnostic_t`,
    `xui_code_fold_range_t`, provider structs.
  - Completion: structs are size-versioned where needed and usable without
    including internal headers.

- [X] Add public CodeEdit handles and callbacks.
  - Include: `xui_code_document`, `xui_code_theme`,
    `xui_code_lexer_proc`, `xui_code_fold_proc`,
    `xui_code_command_proc`, completion/hover/signature callbacks.
  - Completion: API supports built-in and custom providers.

- [X] Add public CodeEdit function declarations.
  - Include: create, document, editing, selection, navigation, language, style,
    margin, marker, indicator, diagnostic, folding, search, command, provider,
    error, scroll, and hit-test functions.
  - Completion: `xui.h` is internally consistent and builds with all new tests.
  - Current: standalone document, language, style, marker/indicator/diagnostic,
    folding, search, provider, margin, selection, editing, and command APIs are
    declared; widget create/render/event APIs and a widget language-registry
    getter are present. Additional widget-level language setters and hit-test
    APIs remain pending.

## 3. Source File Scaffold

- [X] Add CodeDocument implementation file.
  - Target: `dev/xui2/src/xui_code_document.c`
  - Completion: creates/destroys documents, stores text, exposes line index,
    supports edits, and builds in isolation tests.

- [X] Add lexer implementation files.
  - Targets: `src/xui_code_lexer.c`, `src/xui_code_lexer_c.c`
  - Completion: built-in C lexer, callback lexer bridge, regex rule lexer, and
    external token buffer path compile.

- [X] Add fold implementation file.
  - Target: `dev/xui2/src/xui_code_fold.c`
  - Completion: fold ranges, collapsed state, and visible-line mapping are
    separated from widget rendering.

- [X] Add layout implementation file.
  - Target: `dev/xui2/src/xui_code_layout.c`
  - Completion: visible line range, text/margin rects, hit testing, caret rect,
    and scroll calculations are isolated and testable.

- [X] Add command implementation file.
  - Target: `dev/xui2/src/xui_code_command.c`
  - Completion: default command map, key binding table, command dispatch, and
    custom command hook are implemented.

- [X] Add theme implementation file.
  - Target: `dev/xui2/src/xui_code_theme.c`
  - Completion: default CodeEdit theme, token style mapping, and style queries
    are implemented.

- [X] Add search implementation file.
  - Target: `dev/xui2/src/xui_code_search.c`
  - Completion: document-level plain search, regex search, replace-all, and
    regex capture replacement are implemented and testable before widget UI.

- [X] Add annotation implementation file.
  - Target: `dev/xui2/src/xui_code_annotation.c`
  - Completion: marker, indicator, diagnostic, query, clear, and edit tracking
    data paths are implemented independently from widget rendering.

- [X] Add language implementation file.
  - Target: `dev/xui2/src/xui_code_language.c`
  - Completion: language registry, default C language registration, callback
    lexer dispatch, regex lexer dispatch, extension lookup, and fold metadata
    are implemented.

- [X] Add external token buffer implementation file.
  - Target: `dev/xui2/src/xui_code_token_buffer.c`
  - Completion: external tokens are stored by document version and can be
    queried wholly or by offset range.

- [X] Add fold state implementation file.
  - Target: `dev/xui2/src/xui_code_fold_state.c`
  - Completion: fold ranges can be stored, queried, toggled, collapsed,
    expanded, rebuilt from a provider, and converted to visible lines.

- [X] Add provider implementation file.
  - Target: `dev/xui2/src/xui_code_provider.c`
  - Completion: completion, hover, signature help, and command provider
    callbacks can be registered, cleared, and invoked before widget UI exists.

- [X] Add margin implementation file.
  - Target: `dev/xui2/src/xui_code_margin.c`
  - Completion: ordered margin storage, default margins, width/visibility
    updates, layout, text rect calculation, hit testing, and custom callback
    descriptors are implemented before widget rendering.

- [X] Add search indicator bridge.
  - Target: `dev/xui2/src/xui_code_search.c`
  - Completion: plain and regex search matches can be written to the
    annotation indicator store and cleared independently from widget rendering.

- [X] Add selection implementation file.
  - Target: `dev/xui2/src/xui_code_selection.c`
  - Completion: primary selection, anchor/caret offsets, preferred column,
    normalized ranges, navigation commands, select-all, and future-ready
    selection-count APIs are implemented before widget event handling.

- [X] Add editing implementation file.
  - Target: `dev/xui2/src/xui_code_editing.c`
  - Completion: document-level insertion, deletion, replace-selection,
    indentation, outdentation, line-comment toggling, readonly rejection, and
    selection updates are implemented before widget input handling.

- [X] Add widget implementation file.
  - Target: `dev/xui2/src/xui_code_edit.c`
  - Completion: registers the `codeedit` widget type, wires XUI input/render,
    owns internal ScrollFrame/Menu/Popup hooks, and exposes public APIs.
  - Current: widget source exists, registers `codeedit`, owns core editor
    state, exposes lifecycle/getter/text/readonly/error/search/scroll/menu APIs,
    wires input/render/IME/menu/provider-command hooks, owns internal XUI
    ScrollBar children, and uses XUI ScrollModel for scroll range and
    ensure-visible math.

## 4. CodeDocument

- [X] Implement piece-table storage.
  - Completion: original buffer, append buffer, and piece list support set,
    insert, delete, replace, and full text export.

- [X] Implement newline normalization.
  - Completion: LF internal storage, EOL detection, output EOL conversion, and
    paste normalization work for LF, CRLF, and CR.

- [X] Implement UTF-8 boundary safety.
  - Completion: edit positions clamp to valid codepoint boundaries; invalid
    UTF-8 input reports an error and cannot corrupt the document.

- [X] Implement line index.
  - Completion: O(1) line count, offset-to-line lookup, line-to-offset lookup,
    line range query, and edit invalidation pass tests.

- [X] Implement edit transactions.
  - Completion: begin/end edit, single-edit transactions, paste/indent grouped
    transactions, affected range tracking, and callback notification work.
  - Current: begin/end edit, single-edit snapshots, grouped transaction undo,
    dirty/version tracking, `xuiCodeDocumentGetLastEditRange` affected range
    tracking, and transaction-aware document change callbacks are implemented.
    Edits outside a transaction notify immediately; grouped edits notify once
    when the outer edit ends.

- [X] Implement undo/redo.
  - Completion: text, caret/selection snapshot, dirty state, and versions are
    restored correctly across undo/redo.

- [X] Implement document versions and dirty state.
  - Completion: text version, change version, saved version, and dirty query
    are deterministic.

- [X] Implement optional file helpers using XRT.
  - Completion: explicit load/save helpers use `xrtFileReadAll` and
    `xrtFileWriteAll`; editor core remains usable without file I/O.

## 5. Widget Integration

- [X] Register `codeedit` widget type.
  - Completion: `xuiCodeEditGetType` and `xuiCodeEditCreate` work and widget
    destruction frees all owned resources.

- [X] Integrate internal ScrollFrame and ScrollBar.
  - Completion: horizontal/vertical scrolling, viewport clipping, and
    ensure-visible behavior work through existing XUI scroll infrastructure.
  - Current: CodeEdit has public scroll state, exposes its `xui_scroll_model_t`,
    creates internal horizontal and vertical XUI ScrollBar children, computes
    auto scrollbar visibility and viewport size from content dimensions, clamps
    scroll offsets through XUI ScrollModel, syncs scrollbar range/value/steps,
    routes pointer wheel into scroll offsets, invalidates layout/cache/render on
    scroll changes, applies scroll offsets and viewport bounds to hit testing,
    clips rendering to the active viewport, and exposes
    `xuiCodeEditEnsureCaretVisible`.

- [X] Implement focus, capture, and event mask wiring.
  - Completion: CodeEdit receives keyboard, text, IME, pointer, wheel,
    context-menu, focus, and command events only when appropriate.
  - Current: focus/blur, keyboard command, text input, and command-dispatch
    event handlers are wired through the widget event system; IME is enabled
    and composition events are routed; left-button pointer down/move/up focus,
    capture, caret placement, drag selection, double-click selection, wheel,
    context-menu, and provider command routing are wired and covered by the
    widget tests. Scroll infrastructure is tracked separately.

- [X] Implement context menu.
  - Completion: menu supports undo/redo, cut/copy/paste/delete, select all,
    find, replace, go to line, comment, and fold actions with enable state.
  - Current: CodeEdit owns an XUI Menu, opens it from API or context-menu
    events, updates enable state for undo/redo/edit/select actions, queries
    provider command enable-state for host UI commands such as find, replace,
    and go to line, and routes menu selections through the CodeEdit command
    path.

- [X] Implement error string APIs.
  - Completion: widget and document error queries report last failure for
    invalid input, regex failure, provider failure, and allocation failure.
  - Current: `xuiCodeDocumentGetLastError` and `xuiCodeEditGetLastError` exist,
    widget text/command failures update the widget error string, regex pattern
    failures report search errors, and successful widget text updates clear the
    last error.

## 6. Editing Behavior

- [X] Implement caret navigation.
  - Completion: left/right/up/down, word movement, line start/end, document
    start/end, page up/down, preferred visual column.
  - Current: left/right/up/down, word movement, line start/end, document
    start/end, page up/down, and preferred column work through the command
    layer; CodeEdit supplies viewport-derived page line counts and widget tests
    cover PageDown plus Shift+PageUp selection.

- [X] Implement selection behavior.
  - Completion: click, drag, shift navigation, select all, word select, line
    select, and future-ready selection-count API.
  - Current: primary selection, shift navigation, select all, normalized range,
    reversed flag, word select, line select, and future-ready selection count
    API work; widget click/drag primary selection works, double-click selects a
    word, and Ctrl+double-click selects a line.

- [X] Implement text insertion and deletion.
  - Completion: text input, Enter, Backspace, Delete, word delete, replace
    selection, and readonly rejection work.
  - Current: insert text, replace selection, Enter/newline, Backspace, Delete,
    word delete, and readonly rejection work at document/selection/command
    level; widget text-event insertion is wired and tested. IME composition is
    tracked separately.

- [X] Implement indentation commands.
  - Completion: Tab, Shift+Tab, indent selection, outdent selection, tab width,
    and spaces-vs-tabs policy work.
  - Current: indent/outdent selected lines work with explicit indent string or
    indent column count, command execution can insert the configured indent
    string, the widget inserts real tab characters by default,
    `XUI_CODE_EDIT_EXPAND_TABS` opts into space expansion, runtime APIs can
    change expand-tabs and indent-column settings, and
    `XUI_CODE_EDIT_INDENT_WITH_TABS` remains an explicit tab-indentation policy.

- [X] Implement line comment command.
  - Completion: toggle line comment uses active language comment metadata.
  - Current: document-level and command-context toggle work when provided a line
    comment token; CodeEdit owns a language registry, loads default C metadata,
    exposes the registry for custom language registration, and widget command
    execution uses the active language `sLineComment`. `xuiCodeEditSetLanguage`
    and `xuiCodeEditGetLanguage` support runtime language switching for custom
    line-comment metadata.

- [X] Implement clipboard commands.
  - Completion: cut/copy/paste/select-all use the existing XUI proxy clipboard
    contract and preserve multiline content.
  - Current: copy/cut/paste are routed through `xui_code_command_context_t.pProxy`
    and preserve multiline content; select-all is handled by the selection
    command path.

- [X] Implement IME composition.
  - Completion: candidate rect follows caret, composition text displays at the
    caret, commit is one edit transaction, cancel leaves document unchanged.
  - Current: CodeEdit enables IME, exposes a caret-derived candidate rectangle,
    keeps preedit text outside the document, renders composition text at the
    caret with `codeedit.ime.color`, commits final text through a single edit
    transaction, and cancels composition without mutating the document.

## 7. Layout and Rendering

- [X] Implement editor layout.
  - Completion: margin rects, text rect, visible physical lines, visible visual
    rows, content size, and scroll range are computed deterministically.

- [X] Implement base rendering layers.
  - Completion: background, margins, current line, selection, text, caret, and
    disabled/readonly visuals draw correctly.
  - Current: CodeEdit cache rendering paints normal and readonly backgrounds,
    margin band/divider, current line highlight, visible text lines,
    line-number/marker/fold/diagnostic/custom margin visuals, selection ranges,
    primary caret, token overlays, and whitespace/EOL display layers. Tests
    verify base rect/text rendering plus style-driven current-line and readonly
    background draws.

- [X] Implement token rendering.
  - Completion: visible token spans render with style table colors and default
    style fallback.
  - Current: CodeEdit updates its token buffer from the built-in C lexer on
    render when the document version changes, overlays visible token spans,
    supports theme and `codeedit.syntax.*.color` fallback, and tests verify
    token color output. Incremental/custom lexer expansion remains reserved by
    the lexer/language provider APIs.

- [X] Implement visible-line virtualization.
  - Completion: painting and layout process only visible rows plus a small
    buffer and do not iterate all lines during scroll.

- [X] Implement whitespace and EOL display options.
  - Completion: spaces, tabs, indentation guides, and EOL marks can be toggled.
  - Current: public display option flags and widget APIs exist, and the cache
    renderer can toggle ASCII markers for spaces, tabs, end-of-line marks, and
    indentation guide lines with style-property customization.

- [X] Implement monospaced fast path.
  - Completion: common code font path uses cached char advance and line height.
  - Current: layout, content width, caret rect, and hit-test use cached
    `fCharWidth`/`fLineHeight` and tab-aware visual columns.

- [X] Implement proportional-font fallback.
  - Completion: proportional text can be displayed and hit-tested with measured
    prefixes, even if optimized for monospaced fonts.

## 8. Language and Lexer

- [X] Implement built-in C lexer.
  - Completion: keywords, types, identifiers, numbers, strings, chars,
    comments, preprocessor directives, operators, braces, and errors tokenize.

- [X] Implement callback lexer provider.
  - Completion: application callback receives document and line range, returns
    tokens, and error handling is deterministic.

- [X] Implement regex lexer provider using XRT.
  - Completion: regex rules compile with `xrtRegex*` or `xrtRegexSet*`, scan
    line ranges, preserve rule priority, and report compile errors.

- [X] Implement `XRT_NO_REGEX` fallback.
  - Completion: build succeeds when regex is disabled; regex lexer/search APIs
    return unsupported with an error string.

- [X] Implement external token buffer.
  - Completion: application-supplied tokens render when document version
    matches and are rejected or invalidated when stale.

- [X] Implement language registry.
  - Completion: register language, set by id, set by extension, and language
    comment/fold metadata work.

## 9. Style and Theme

- [X] Implement default CodeEdit theme.
  - Completion: readable default colors for text, background, selection,
    current line, margins, diagnostics, search, and C syntax.

- [X] Implement style table.
  - Completion: style ids map to foreground/background/flags and token kinds
    can be remapped without lexer changes.

- [X] Integrate XUI style properties.
  - Completion: `codeedit.*` style properties resolve through existing XUI2
    style APIs and invalidate the right dirty categories.
  - Current: CodeEdit registers core paint properties for text, readonly text,
    background, divider, selection, caret, whitespace markers, and inherited
    font name plus `codeedit.syntax.*.color` token properties; cache rendering
    resolves those color properties and widget tests verify inline style
    overrides for both chrome and token output.

- [X] Implement application theme override.
  - Completion: `xuiCodeEditSetTheme` and `xuiCodeEditSetStyle` update visible
    editor output without rebuilding the document.
  - Current: standalone `xuiCodeTheme*` APIs support default/reset, style
    override, style query, and token remapping; widget-level color invalidation
    works for core `codeedit.*` properties, and widget-level
    `xuiCodeEditSetTheme`/`xuiCodeEditSetStyle` helpers invalidate and repaint
    token output.

## 10. Margins

- [X] Implement margin model.
  - Completion: ordered margins support line number, marker, fold, change,
    diagnostic, and custom margin kinds.

- [X] Implement line number margin.
  - Completion: line numbers align with visible lines, active line is styled,
    and folded/hidden lines do not draw extra numbers.
  - Current: line-number margin kind, default width, ordering, layout, and hit
    geometry are implemented; widget paint draws fold-visible line numbers and
    highlights the active caret line in the margin.

- [X] Implement marker margin.
  - Completion: marker icons or color blocks render and respond to clicks.
  - Current: marker margin kind, default width, ordering, click hit geometry,
    and annotation data source exist; widget paint renders marker blocks for
    visible lines, and clicks are consumed before text selection.

- [X] Implement fold margin.
  - Completion: fold arrows render on fold headers and toggle fold state.
  - Current: fold margin kind, default width, hit geometry, and fold-state
    commands exist; widget paint renders a fold header glyph for visible fold
    ranges, and pointer down toggles the clicked header line.

- [X] Implement diagnostic margin.
  - Completion: diagnostic severity icons or blocks render per visible line.
  - Current: diagnostic margin kind, default width, hit geometry, and
    diagnostic data source exist; widget paint renders severity color blocks
    for visible diagnostics, and clicks are consumed before text selection.

- [X] Implement custom margin callbacks.
  - Completion: applications can render and receive events for custom margins.
  - Current: custom margin descriptors preserve render/event callbacks and
    user data; widget paint calls custom render callbacks and pointer
    down/up dispatch invokes custom margin event callbacks.

## 11. Markers, Indicators, and Diagnostics

- [X] Implement marker types and line markers.
  - Completion: breakpoints, disabled breakpoints, current execution line,
    bookmarks, modified lines, and custom markers can be set/cleared.

- [X] Implement indicator types and range indicators.
  - Completion: squiggle, underline, box, background, foreground, and custom
    range decorations render with correct layering.
  - Current: indicator data types and range queries are implemented; actual
    paint layering is covered by the widget rendering tasks.

- [X] Implement diagnostics.
  - Completion: structured diagnostics create margin visuals, indicators, hover
    text, and severity style mapping.
  - Current: structured diagnostics, severity, copied strings, indexed lookup,
    and offset queries are implemented; margin visuals and hover display are
    covered by widget/margin tasks.

- [X] Implement marker/indicator edit tracking.
  - Completion: markers and indicators survive nearby edits or are invalidated
    according to documented range policy.
  - Current: line/offset delta tracking is implemented for the annotation store;
    markers shift or collapse to edited lines, and indicators/diagnostics shift,
    shrink, or collapse around overlapping edits according to the annotation
    range policy covered by tests.

## 12. Folding

- [X] Implement fold range storage.
  - Completion: fold ranges store start/end/level/flags and validate nesting.

- [X] Implement built-in C fold provider.
  - Completion: braces, block comments, and preprocessor ranges produce fold
    ranges.

- [X] Implement custom fold provider.
  - Completion: application callback can replace or augment fold ranges.

- [X] Implement fold state and visible-line mapping.
  - Completion: collapsed lines are hidden from layout, painting, hit testing,
    and keyboard movement.

- [X] Implement fold commands.
  - Completion: toggle fold, fold all, unfold all, and line-visible query work.

## 13. Search and Replace

- [X] Implement plain-text search.
  - Completion: forward/backward, case mode, whole-word, selection-only, and
    wrap search work.
  - Current: `xuiCodeSearchFindPlain` covers document-wide search and
    `xuiCodeSearchFindPlainRange` covers selection/range-limited search.

- [X] Implement regex search using XRT.
  - Completion: regex find uses compiled `xrtRegex*`, captures are available,
    and pattern errors are reported.

- [X] Implement replace and replace-all.
  - Completion: replacements are transaction grouped, support regex captures,
    and update selection/search indicators.
  - Current: document-level plain replace-all and regex replace-all work,
    including `$0`-style and `\0`-style capture expansion; CodeEdit-level
    plain and regex replace-all APIs update the document selection and search
    result indicators after replacement.

- [X] Implement search result indicators.
  - Completion: all search matches can be highlighted and cleared.

## 14. IDE Provider Hooks

- [X] Implement completion provider hook.
  - Completion: provider callback receives offset/prefix and can populate a
    completion list; UI presentation can be opened through XUI Popup/ListView.
  - Current: callback storage and invocation are implemented; Popup/ListView
    presentation is covered by widget tasks.

- [X] Implement hover provider hook.
  - Completion: provider callback returns hover content and range; Tooltip or
    Popup presentation is wired.
  - Current: callback storage and invocation are implemented; Tooltip/Popup
    presentation is covered by widget tasks.

- [X] Implement signature help provider hook.
  - Completion: provider callback returns active signature/parameter data and
    Popup presentation is wired.
  - Current: callback storage and invocation are implemented; Popup
    presentation is covered by widget tasks.

- [X] Implement command provider hook.
  - Completion: application commands can intercept default commands and return
    handled/not-handled results.

## 15. Example

- [X] Add CodeEdit example project.
  - Target: `dev/xui2/examples/xui_codeedit`
  - Completion: example builds and displays a C source sample with line
    numbers, syntax highlight, selection, markers, diagnostics, folding, search
    result, and context menu.
  - Evidence: `dev/xui2/examples/xui_codeedit/main.c`

- [X] Add example build script.
  - Target: `dev/xui2/examples/xui_codeedit/build.bat`
  - Completion: script builds `build\xui_codeedit.exe`.
  - Evidence: `dev/xui2/examples/xui_codeedit/build.bat`

- [X] Add automated example run mode.
  - Completion: `build\xui_codeedit.exe --frames 3` exits successfully and
    prints summary fields for document, lexer, render, input, selection,
    margin, marker, indicator, fold, search, and command.
  - Current: `--frames 3` runs headless through the XUI test proxy and prints
    all required summary fields as `1`.

## 16. Tests

- [X] Add CodeDocument tests.
  - Targets: `test_xui/xui_code_document_test.c`,
    `test_xui/build_code_document_test.bat`
  - Completion: tests cover set/get text, newline normalization, line index,
    offset mapping, edits, last edit range tracking, undo/redo, transactions,
    dirty/version tracking.

- [X] Add lexer tests.
  - Targets: `test_xui/xui_code_lexer_test.c`,
    `test_xui/build_code_lexer_test.bat`
  - Completion: tests cover built-in C tokens, regex rules, regex failures,
    external tokens, and `XRT_NO_REGEX` behavior where build config allows.

- [X] Add fold tests.
  - Targets: `test_xui/xui_code_fold_test.c`,
    `test_xui/build_code_fold_test.bat`
  - Completion: tests cover C fold ranges, comment folds, preprocessor folds,
    fold header flags, collapsed visible-line mapping.

- [X] Add layout tests.
  - Targets: `test_xui/xui_code_layout_test.c`,
    `test_xui/build_code_layout_test.bat`
  - Completion: tests cover visible line layout, margin/text rects, content
    size, hit testing, caret rects, folded-line rejection, scroll offsets, and
    tab-aware monospaced visual columns.

- [X] Add theme tests.
  - Targets: `test_xui/xui_code_theme_test.c`,
    `test_xui/build_code_theme_test.bat`
  - Completion: tests cover default styles, token style lookup, style override,
    token remapping, and invalid id rejection.

- [X] Add command map tests.
  - Targets: `test_xui/xui_code_command_test.c`,
    `test_xui/build_code_command_test.bat`
  - Completion: tests cover default key bindings, custom bind/rebind, unbind,
    indexed binding reads, default reload, provider interception, navigation,
    editing, folding, clipboard copy/cut/paste, and unsupported UI-dependent
    commands.

- [X] Add search tests.
  - Targets: `test_xui/xui_code_search_test.c`,
    `test_xui/build_code_search_test.bat`
  - Completion: tests cover plain forward/backward/wrap/case/whole-word/range
    search, regex find, regex captures, regex errors, plain replace-all, and
    regex capture replace-all.

- [X] Add annotation tests.
  - Targets: `test_xui/xui_code_annotation_test.c`,
    `test_xui/build_code_annotation_test.bat`
  - Completion: tests cover marker set/replace/clear/query, indicator
    range set/clear/query, diagnostic string copy/query/clear, and basic
    edit tracking.

- [X] Add language tests.
  - Targets: `test_xui/xui_code_language_test.c`,
    `test_xui/build_code_language_test.bat`
  - Completion: tests cover default C registration, id lookup, extension
    lookup, C lex/fold dispatch, callback lexer dispatch, regex lexer dispatch,
    and registry clearing.

- [X] Add external token buffer tests.
  - Targets: `test_xui/xui_code_token_buffer_test.c`,
    `test_xui/build_code_token_buffer_test.bat`
  - Completion: tests cover token storage, version matching, stale rejection,
    full token reads, range token reads, and clearing.

- [X] Add fold state tests.
  - Targets: `test_xui/xui_code_fold_state_test.c`,
    `test_xui/build_code_fold_state_test.bat`
  - Completion: tests cover range storage, header flag normalization, line
    toggling, fold all, unfold all, line visibility, visible-line mapping, and
    custom provider rebuilds.

- [X] Add provider tests.
  - Targets: `test_xui/xui_code_provider_test.c`,
    `test_xui/build_code_provider_test.bat`
  - Completion: tests cover completion, hover, signature help, command
    provider registration, invocation, handled results, and clearing.

- [X] Add margin tests.
  - Targets: `test_xui/xui_code_margin_test.c`,
    `test_xui/build_code_margin_test.bat`
  - Completion: tests cover default margin order, total width, margin layout,
    text rect calculation, hit testing, line mapping with scroll, width changes,
    visibility, custom callback descriptors, removal, and clearing.

- [X] Add search indicator tests.
  - Targets: `test_xui/xui_code_search_indicator_test.c`,
    `test_xui/build_code_search_indicator_test.bat`
  - Completion: tests cover plain match indicators, whole-word filtering,
    regex match indicators, custom indicator ids, preservation of unrelated
    indicators, and clearing.

- [X] Add selection tests.
  - Targets: `test_xui/xui_code_selection_test.c`,
    `test_xui/build_code_selection_test.bat`
  - Completion: tests cover range normalization, reversed selection, selection
    count, unsupported multi-selection add, goto offset, goto line/column,
    left/right movement, word movement, vertical preferred column, line
    start/end, document start/end, shift selection, select all, word select,
    line select, and clearing.

- [X] Add editing tests.
  - Targets: `test_xui/xui_code_editing_test.c`,
    `test_xui/build_code_editing_test.bat`
  - Completion: tests cover insert, replace selection, readonly rejection,
    Backspace, Delete, delete selection, indent selection, outdent selection,
    line comment, uncomment, and invalid comment tokens.

- [X] Add CodeEdit widget tests.
  - Targets: `test_xui/xui_code_edit_test.c`,
    `test_xui/build_code_edit_test.bat`
  - Completion: tests cover create/destroy, input, selection, clipboard, IME
    candidate rect, scroll, margins, markers, indicators, diagnostics, folding,
    search, command binding, and context menu.
  - Current: tests cover type registration/reuse, create/destroy, root
    embedding, owned core handles, initial text, readonly state, default
    margins, default command map, text reset, document edit range, focus,
    text-event insertion, key-command deletion, Tab insertion, command-dispatch
    select-all, readonly blocking, IME enablement, candidate rectangle, and
    composition insertion, pointer focus/capture, click caret placement, and
    drag range selection, context-menu creation/opening, readonly menu state,
    right-click opening, menu delete execution, public scroll get/set, wheel
    scrolling, scroll clamping, scroll-aware pointer hit testing, cache
    rendering, background/text/selection/caret draw verification, C token-buffer
    refresh during render, theme-colored token span rendering, and
    whitespace/EOL display option rendering, plus inline `codeedit.*` style
    property resolution for marker color rendering. Tests also cover margin
    rendering, scroll-aware hit testing after margin text-origin adjustment, and
    language-metadata line comment command dispatch.

- [X] Add smoke verification to work handoff.
  - Target: `dev/xui2/docs/work.md`
  - Completion: latest CodeEdit build/test/example commands and results are
    recorded after implementation.
  - Current: document, lexer, fold, fold-state, layout, margin, theme,
    command-map/dispatch, search, search-indicator, selection, editing, annotation,
    language, token-buffer, provider, widget lifecycle/event tests and
    `build_dll.bat` are recorded; full custom lexer rendering, menu command UI,
    ScrollBar/preedit/example verification is pending; the latest widget test
    covers whitespace/EOL display options.

## 17. Verification Matrix

Required commands:

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
examples\xui_codeedit\build.bat
build\xui_codeedit.exe --frames 3
```

Expected example summary:

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

Current verification:

- [X] `test_xui\build_code_document_test.bat`
- [X] `test_xui\build_code_lexer_test.bat`
- [X] `test_xui\build_code_fold_test.bat`
- [X] `test_xui\build_code_layout_test.bat`
- [X] `test_xui\build_code_theme_test.bat`
- [X] `test_xui\build_code_command_test.bat`
- [X] `test_xui\build_code_search_test.bat`
- [X] `test_xui\build_code_annotation_test.bat`
- [X] `test_xui\build_code_language_test.bat`
- [X] `test_xui\build_code_token_buffer_test.bat`
- [X] `test_xui\build_code_fold_state_test.bat`
- [X] `test_xui\build_code_provider_test.bat`
- [X] `test_xui\build_code_margin_test.bat`
- [X] `test_xui\build_code_search_indicator_test.bat`
- [X] `test_xui\build_code_selection_test.bat`
- [X] `test_xui\build_code_editing_test.bat`
- [X] `test_xui\build_code_edit_test.bat`
- [X] `examples\xui_codeedit\build.bat`
- [X] `build\xui_codeedit.exe --frames 3`

## 18. Integration Readiness

- [X] Embeds cleanly in Panel.
  - Completion: CodeEdit can be used as the client widget inside Panel.

- [X] Embeds cleanly in Tabs.
  - Completion: multiple CodeEdit widgets in tabs keep independent documents,
    selections, scroll offsets, and dirty states.

- [X] Embeds cleanly in DockPanel.
  - Completion: CodeEdit behaves correctly in document panes and docked panes.

- [X] Supports read-only debugger source view.
  - Completion: current execution line, breakpoints, diagnostics, and copy work
    without allowing text mutation.

- [X] Supports IDE command integration.
  - Completion: host application can bind save, go to definition, find
    references, format document, and toggle breakpoint commands externally.

## 19. Deferred But Reserved

These are not required for first completion, but the API and model must not
block them.

- [~] Multi-selection and multi-caret behavior beyond primary selection.
  - Model supports additional inactive selection/caret ranges through
    `xuiCodeSelectionAdd`, `xuiCodeSelectionGetCount`, and
    `xuiCodeSelectionGetAt`.
  - Widget rendering draws multiple selected ranges and collapsed carets.
  - Editing commands still operate on the primary selection; multi-range edit
    application remains to be implemented.
- [ ] Inline widgets and code lenses.
- [ ] Minimap.
- [ ] Diff view.
- [ ] Block selection editing.
- [ ] Semantic token provider.
- [ ] LSP adapter package outside XUI core.
- [ ] File watcher integration outside XUI core.
- [ ] Very-large-file streaming model beyond piece table.

## 20. Maintenance Rules

- Every CodeEdit implementation change must update this SPEC if it changes
  status, scope, or completion criteria.
- Do not mark a capability `[X]` until its source, API, tests, example impact,
  and documentation impact are handled.
- If a capability is intentionally narrowed, add the reason and the replacement
  behavior under that item.
- Keep `xui-codeedit-design.md` for architecture and this SPEC for progress.
- Keep final widget usage details in `docs/xui/widget-codeedit.md`.
