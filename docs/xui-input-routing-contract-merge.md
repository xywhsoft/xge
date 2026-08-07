# XGE / XUI input routing contract

## Goal

Application code must not maintain a whitelist of text-editing keys or forward
selected platform messages into widgets. Keyboard, text, IME, pointer, and
touch input use one normalized route with an explicit consume result.

## Native ownership

The operating-system text service runs before XUI. On Windows TSF modes,
`ITfKeystrokeMgr::TestKeyDown/KeyDown` and `TestKeyUp/KeyUp` decide whether the
TIP owns a key. An owned key stops at that boundary and is not visible to XUI,
global hotkeys, or application fallback. This covers Backspace during IME
composition without a heuristic key table.

Native presentation mode delegates the complete IME lifecycle to Windows;
Sokol converts the committed `WM_CHAR` into an ordinary ordered text event.
Composition/Full modes use the TSF text store; IMM is retained only as a
platform fallback when TSF cannot be initialized.

## XUI keyboard route

For an unowned keyboard-down event, the order is:

1. focused widget target handler;
2. registered XUI global hotkey table;
3. focused widget ancestor bubble handlers;
4. XUI built-in Tab, Enter, Escape, and context-menu defaults;
5. optional XGE application fallback.

At any XUI stage, `XUI_EVENT_DISPATCH_STOP` consumes the event and ends the
remaining stages. Returning `XUI_OK` means success, not consumption. The Ex
input APIs report `XUI_INPUT_RESULT_CONSUMED` separately from errors.

Text and IME events do not use the hotkey table. They target the focused widget
and then bubble to ancestors. Pointer events use capture, target, and bubble
phases with per-pointer capture maintained by XUI.

## XGE to XUI bridge

Platform callbacks update compatibility polling state and append normalized
records to the ordered event FIFO. `xuiProxyXgePumpInput` drains pointer,
touch, keyboard, text, IME, and candidate records through XUI APIs in arrival
order. It reports consumed keys to XGE compatibility state, refreshes the
focused text snapshot, and does not manually address individual editor
controls. `xuiProxyXgePumpKeyboard` is an alias for this complete pump.

Applications must not combine this pump with a second manual text/IME bridge.
The legacy polling getters are compatibility views of the same input, not an
independent delivery stream.

## Legacy scene callback boundary

The existing `xge_scene.onEvent` return value remains an error/quit signal and
cannot be reinterpreted as a consume boolean. Integrations that need an XGE
fallback after XUI should use the ordered-input consume result and an explicit
fallback callback whose result distinguishes PASS, CONSUMED, and ERROR.

## Required invariants

- No application key-forwarding table.
- No IME editing-key blacklist.
- No focused control bypass for a global hotkey.
- No ancestor bubble before the hotkey table.
- No duplicate text delivery from ordered and polling APIs.
- No nonzero event-handler result treated as both consume and fatal error.

## Merge files

- `xge.h`
- `xui.h`
- `src/xge_input.c`
- `src/xge_ime_win32_tsf.c`
- `src/xui_proxy_xge.c`
- `src/xui_input.c`
- `src/xui_terminal.c`
- `test/test_input_event_queue.c`
- `test/test_ime_mode_contract.c`
- `test_xui/xui_input_test.c`
