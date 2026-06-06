# XUI Terminal Control SPEC

This SPEC tracks the XUI2-native `Terminal` control. Status markers:

- `[ ]` not started
- `[~]` in progress
- `[X]` completed

## Scope

V1 delivers one XUI-native `xuiTerminal` widget that can act as the frontend for:

- fake/test sessions
- local process sessions through a session adapter
- remote SSH sessions through a future adapter

The widget implements terminal frontend behavior. The widget core does not
directly spawn or manage real platform processes; concrete session adapters own
that transport work.

## Maintenance Rules

- Keep this SPEC updated whenever Terminal implementation state changes.
- Do not mark a task `[X]` until code, test, example, and documentation evidence exists where applicable.
- Use `xui` public names only; `xui2` is a folder/workstream name.
- Keep XSON deferred.
- Do not add proxy APIs silently.
- Preserve XUI cache-first rendering.
- Do not introduce a plugin/addon system.
- Do not introduce multiple renderer backends.
- Examples must support `--frames N` and `--seconds N`.

## Phase 0: Design And Planning

- [X] Create Terminal design document: `docs\terminal-control-design.md`.
- [X] Create tracked Terminal SPEC: `docs\terminal-control-spec.md`.
- [X] Add Terminal documents to `docs\README.md`.
- [X] Create widget user document: `docs\xui\widget-terminal.md`.

Completion criteria:

- Design records the XUI-native direction and excludes xterm.js-style addon/multi-renderer architecture.
- SPEC contains implementation phases and completion gates.
- Docs index links both planning documents.

## Phase 1: Public API And Widget Skeleton

- [X] Add public constants to `xui.h`.
- [X] Add terminal callback typedefs.
- [X] Add `xui_terminal_desc_t`.
- [X] Add `xui_terminal_cell_t` or an intentionally private equivalent.
- [X] Add `xuiTerminalGetType`.
- [X] Add `xuiTerminalCreate`.
- [X] Add basic typed widget registration in `src\xui_terminal.c`.
- [X] Add source file to `xui_sources.bat`.
- [X] Add minimal create/destroy/render unit test.

Completion criteria:

- `test_xui\build_terminal_test.bat` builds.
- Empty terminal can be created, sized, rendered, focused, and destroyed.
- No new proxy capability is required.

## Phase 2: Buffer Core

- [X] Add main screen buffer.
- [X] Add alternate screen buffer.
- [X] Add scrollback ring.
- [X] Add cursor state.
- [X] Add current text attributes.
- [X] Add tab stops.
- [X] Add dirty row map.
- [X] Add resize logic for cols/rows.
- [X] Add buffer tests for write, wrap, scroll, clear, and resize.

Completion criteria:

- Terminal buffer can store fixed-size cell rows.
- Scrollback preserves lines beyond visible rows.
- Alternate buffer switches without destroying main scrollback.

## Phase 3: UTF-8 And Parser

- [X] Add UTF-8 decoder.
- [X] Add printable codepoint handling.
- [X] Add C0 controls: BEL, BS, HT, LF, CR.
- [X] Add ESC parser.
- [X] Add CSI parser.
- [X] Add OSC parser skeleton for title/link future use.
- [X] Add parser dispatch table or equivalent handlers.
- [X] Add parser unit tests independent from widget rendering.

Completion criteria:

- `xuiTerminalWrite` can feed bytes through parser into buffer.
- Parser tests verify text, newlines, cursor movement, and erase behavior.

## Phase 4: VT/ANSI Sequence Coverage

- [X] Implement cursor movement: CUU, CUD, CUF, CUB, CUP.
- [X] Implement erase display: ED.
- [X] Implement erase line: EL.
- [X] Implement SGR reset and base attributes.
- [X] Implement 8/16 colors.
- [X] Implement 256 colors.
- [X] Implement true color.
- [X] Implement inverse style.
- [X] Implement underline style.
- [X] Implement bold/dim style.
- [X] Implement scroll region.
- [X] Implement cursor show/hide.
- [X] Implement save/restore cursor.
- [X] Implement alternate screen modes.
- [X] Implement bracketed paste mode flag.
- [X] Add sequence tests.

Completion criteria:

- Common shell output, progress-line rewrites, colored output, and full-screen terminal apps have the required V1 primitives.
- Unsupported sequences are ignored safely.

## Phase 5: Write Queue And Flow Control

- [X] Implement `xuiTerminalWrite`.
- [X] Implement `xuiTerminalWriteText`.
- [X] Implement `xuiTerminalFlush`.
- [X] Add internal write queue.
- [X] Add parse budget configuration.
- [X] Process queued data in `xuiUpdate`.
- [X] Add tests for queued writes and flush.

Completion criteria:

- Large writes do not require immediate full parse in the input API.
- Flush can force pending data to be parsed for tests and deterministic examples.

## Phase 6: Input Encoding

- [X] Add input callback.
- [X] Encode text input to UTF-8 bytes.
- [X] Encode Enter.
- [X] Encode Backspace.
- [X] Encode Tab.
- [X] Encode arrow keys.
- [X] Encode Home/End.
- [X] Encode PageUp/PageDown.
- [X] Encode Ctrl combinations.
- [X] Encode Alt combinations if practical in V1.
- [X] Encode paste.
- [X] Support bracketed paste output when mode is enabled.
- [X] Add input encoding tests.

Completion criteria:

- The widget can drive a fake session and future process/SSH sessions through byte callbacks.
- Input APIs do not mutate the terminal screen directly except where local echo is explicitly provided by a fake session.

## Phase 7: Fit And Resize

- [X] Compute cell width and height from font metrics.
- [X] Compute cols/rows from content rect and padding.
- [X] Implement `xuiTerminalFit`.
- [X] Implement `xuiTerminalGetColumns`.
- [X] Implement `xuiTerminalGetRows`.
- [X] Add resize callback.
- [X] Preserve buffer content on resize.
- [X] Add tests for resize and callback firing.

Completion criteria:

- Resizing the widget updates terminal geometry.
- Session adapters can receive the new cols/rows.

## Phase 8: Rendering

- [X] Resolve terminal palette and style properties.
- [X] Render background.
- [X] Render cell background runs.
- [X] Render text runs.
- [X] Render selection overlay.
- [X] Render cursor.
- [X] Render hover link underline if link detection is present.
- [X] Render focus frame.
- [X] Render internal scrollbar or expose scroll model for ScrollBar composition.
- [X] Add render tests with test proxy counters.

Completion criteria:

- Dirty rows invalidate render cache.
- Text runs are merged enough to avoid one draw call per cell in common output.
- Rendering uses existing proxy APIs only.

## Phase 9: Scrollback And Selection

- [X] Integrate `xui_scroll_model_t`.
- [X] Implement wheel scrolling.
- [X] Implement PageUp/PageDown scroll behavior when selection is not active.
- [X] Implement drag selection.
- [X] Implement double-click word selection.
- [X] Implement triple-click row selection.
- [X] Implement Shift selection extension.
- [X] Implement `xuiTerminalGetSelectionText`.
- [X] Implement `xuiTerminalCopySelection`.
- [X] Implement `xuiTerminalClearSelection`.
- [X] Add selection tests.

Completion criteria:

- Selection text is extracted from screen and scrollback correctly.
- Right-click/menu operations do not hide or corrupt selection.

## Phase 10: Menu, Clipboard, And Built-In Commands

- [X] Add context menu.
- [X] Add Copy.
- [X] Add Paste.
- [X] Add Select All.
- [X] Add Clear Screen.
- [X] Add Clear Scrollback.
- [X] Add Find.
- [X] Use proxy clipboard get/set where available.
- [X] Add command/menu tests.

Completion criteria:

- Terminal supports common user operations without application-specific menu code.
- Clipboard behavior follows existing XUI input/menu conventions where practical.

## Phase 11: Built-In Search, Serialize, Links, Unicode

- [X] Implement plain text search in screen and scrollback.
- [X] Implement find next/previous.
- [X] Implement current search match highlight.
- [X] Implement plain text serialization.
- [X] Implement URL detection.
- [X] Add link callback.
- [X] Add first Unicode width table.
- [X] Support CJK wide-cell layout.
- [X] Add tests for search, serialize, links, and wide cells.

Completion criteria:

- Features that would be xterm.js addons are built into Terminal as normal XUI APIs.
- No plugin/addon API is introduced.

## Phase 12: Session Interface

- [X] Add `xui_terminal_session_t` abstraction.
- [X] Add attach/detach API.
- [X] Add fake session implementation.
- [X] Add session write callback wiring.
- [X] Add session resize callback wiring.
- [X] Add tests for fake session echo and scripted output.

Completion criteria:

- Terminal can be tested interactively without a real process.
- The same interface can later be used by local process and SSH adapters.

## Phase 13: Example And Documentation

- [X] Add `examples\xui_terminal\main.c`.
- [X] Add `examples\xui_terminal\build.bat`.
- [X] Example supports `--frames N`.
- [X] Example supports `--seconds N`.
- [X] Example demonstrates ANSI colors.
- [X] Example demonstrates cursor movement and line rewrite.
- [X] Example demonstrates scrollback.
- [X] Example demonstrates fake session input echo.
- [X] Example demonstrates resize event output.
- [X] Add `docs\xui\widget-terminal.md`.
- [X] Update `docs\work.md` with implementation summary and verification commands.

Completion criteria:

- `examples\xui_terminal\build.bat` succeeds.
- `build\xui_terminal.exe --frames 5` exits cleanly.
- Running without duration stays alive.

## Phase 14: Verification Gate

- [X] Run parser tests.
- [X] Run buffer tests.
- [X] Run widget tests.
- [X] Run `examples\xui_terminal\build.bat`.
- [X] Run `build\xui_terminal.exe --frames 5`.
- [X] Run `build_dll.bat`.
- [X] Run `git diff --check`.
- [X] Record final command output summary in `docs\work.md`.

Completion criteria:

- All V1 commands pass.
- No whitespace errors are reported.
- The SPEC accurately reflects completed and deferred work.

## Phase 15: Interactive Command Lab Example

- [X] Add `examples\xui_terminal_cmd_lab\main.c`.
- [X] Add `examples\xui_terminal_cmd_lab\build.bat`.
- [X] Example supports `--frames N`.
- [X] Example supports `--seconds N`.
- [X] Example supports built-in shell commands.
- [X] Example forwards XGE mouse, text, and keyboard input into XUI.
- [X] Example can attach a ConPTY-backed external process with `run <command line>`, `cmd`, or `powershell`.
- [X] Example provides `--process-smoke` for scripted local process validation.

Completion criteria:

- `examples\xui_terminal_cmd_lab\build.bat` succeeds.
- `build\xui_terminal_cmd_lab.exe --frames 40` exits cleanly.
- `build\xui_terminal_cmd_lab.exe --frames 240 --process-smoke` starts a real `cmd.exe /c` process and exits cleanly.
- Running without duration stays alive for manual terminal interaction.

## Deferred After V1

- [X] Local process/ConPTY adapter.
  - [X] Add public `xui_terminal_process_desc_t`.
  - [X] Add `xuiTerminalCreateProcessSession`.
  - [X] Add process session stdin write, non-blocking poll, running query, and terminate APIs.
  - [X] Poll attached process sessions from Terminal update.
  - [X] Add Windows pipe-backed local command test.
  - [X] Document the pipe-backed local process session.
  - [X] Add `XUI_TERMINAL_PROCESS_CONPTY` mode.
  - [X] Add ConPTY pseudo console lifecycle.
  - [X] Add `xuiTerminalSessionResize`.
  - [X] Propagate Terminal resize to ConPTY.
  - [X] Add ConPTY lifecycle and resize test.
  - [X] Validate interactive shell behavior with real terminal applications.
    - [X] 2026-06-05 status: ConPTY output capture now uses a reader thread and `STARTF_USESTDHANDLES` isolation so `cmd.exe /d /q` accepts input and returns command output through `xuiTerminalSessionPoll`.
- [X] SSH adapter.
  - [X] Add public `xui_terminal_ssh_desc_t`.
  - [X] Add `xuiTerminalBuildSshCommand`.
  - [X] Add `xuiTerminalCreateSshSession`.
  - [X] Use an OpenSSH-backed process session instead of embedding an SSH protocol stack in the Terminal widget core.
  - [X] Support host, user, port, identity file, extra OpenSSH options, optional remote command, working directory, resize callback, process flags, and initial terminal size.
  - [X] Add unit tests for command construction, truncation safety, invalid descriptors, and invalid session creation.
- [X] Practical xterm mouse tracking.
  - [X] Parse DECSET/DECRST mouse modes `?9`, `?1000`, `?1002`, and `?1003`.
  - [X] Parse focus tracking `?1004`, UTF-8 mouse `?1005`, SGR mouse `?1006`, alternate scroll `?1007`, and urxvt mouse `?1015`.
  - [X] Emit mouse press, release, drag, any-motion, wheel, and focus reports through the terminal input/session callback.
  - [X] Suppress local selection/link/menu pointer handling while terminal mouse tracking is active.
  - [X] Add widget tests for SGR press/release, drag, any-motion, wheel, and focus reports.
  - [X] Add DEC highlight mouse mode `?1001` protocol state, `CSI ... T` range handling, and release reports.
- [X] OSC 8 hyperlink parsing.
  - [X] Parse OSC 8 BEL terminated sequences.
  - [X] Parse OSC 8 ST terminated sequences.
  - [X] Store active link ids on visible main/alternate screen cells.
  - [X] Preserve OSC 8 metadata after linked rows enter scrollback.
  - [X] Route OSC 8 hit testing through `xuiTerminalGetLinkAt`.
  - [X] Route OSC 8 hover and click through the existing link callback path.
  - [X] Add OSC 8 widget tests.
- [X] Image protocols.
  - [X] Add public `xui_terminal_image_t`.
  - [X] Add `xui_terminal_image_proc`.
  - [X] Add `xuiTerminalSetImageCallback`.
  - [X] Parse iTerm2-style `OSC 1337;File=options:payload` inline image requests.
  - [X] Report protocol, cursor column/row, options, payload pointer, and payload size through a synchronous callback.
  - [X] Add OSC overflow protection so truncated image payloads do not dispatch.
  - [X] Add widget tests for image callback metadata and overflow suppression.
- [X] Sixel.
  - [X] Add `XUI_TERMINAL_IMAGE_SIXEL`.
  - [X] Parse DCS Sixel payloads terminated by ST.
  - [X] Report Sixel options, payload, cursor column/row, and payload size through the existing image callback.
  - [X] Add DCS overflow protection so truncated Sixel payloads do not dispatch.
  - [X] Add widget tests for Sixel callback metadata and overflow suppression.
- [~] Complex grapheme clusters.
  - [X] Add first-pass combining mark and variation selector storage on visible cells.
  - [X] Keep combining marks zero-width for cursor advance, serialization, search columns, and match length.
  - [X] Render and serialize base cell text together with attached combining marks.
  - [X] Add widget tests for `e + U+0301`, search column width, and leading combining fallback.
  - [X] Add widget tests for bounded suffix overflow flagging on overlong combining sequences.
  - [X] Add first-pass common script mark ranges for Hebrew, Arabic, Devanagari, and Thai.
  - [X] Add widget tests for Devanagari and Thai mark storage, serialization, search, and display columns.
  - [X] Expand first-pass script mark coverage across Cyrillic, Syriac, NKo, Bengali-family Indic scripts, Tibetan, Myanmar, Khmer, and related ranges.
  - [X] Add widget tests for Bengali, Tibetan, and Myanmar mark storage, serialization, search, and display columns.
  - [X] Add first-pass Hangul Jamo cluster handling for leading Jamo plus zero-width medial/final suffixes.
  - [X] Add widget tests for Hangul Jamo storage, serialization, search, and display width.
  - [X] Add first-pass zero-width format controls for ZWSP, ZWNJ, WORD JOINER, and BOM.
  - [X] Add widget tests for zero-width format storage, serialization, search, and display columns.
  - [ ] Full Unicode grapheme boundary rules, Indic/Thai glyph reordering/shaping, and bidirectional text remain deferred.
- [X] Ligature shaping.
  - [X] Preserve compatible same-style terminal cells as proxy text runs so the font renderer can apply font-level ligatures.
  - [X] Add `xuiTerminalSetLigaturesEnabled`.
  - [X] Add `xuiTerminalGetLigaturesEnabled`.
  - [X] Split rendering to one draw per occupied cell when ligatures are disabled.
  - [X] Add widget render tests for default one-run rendering and disabled per-cell rendering.
- [X] Glyph atlas or faster terminal renderer.
  - [X] Keep a persistent cache surface for Terminal rendering.
  - [X] Track dirty visible rows.
  - [X] Redraw only dirty rows for ordinary output.
  - [X] Merge compatible text cells into text runs instead of drawing one cell at a time.
  - [X] Add widget render tests for full render and dirty-row partial render text draw counts.
- [ ] XSON description.
