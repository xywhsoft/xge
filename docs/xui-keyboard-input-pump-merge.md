# XUI keyboard input pump merge note

This note is subordinate to:

- `docs/windows-ime-contract-v1.md`
- `docs/xui-input-routing-contract-merge.md`

## Scope

`xuiProxyXgePumpInput(xui_context)` and `xuiProxyXgePumpInputRect` are the
ordered bridges for XGE/XUI input. `xuiProxyXgePumpKeyboard` remains only as a
source-compatible name and delegates to `xuiProxyXgePumpInput`; it cannot skip
pointer records without violating the native FIFO order.

The helper:

1. drains pointer, touch, keyboard, text, and IME records from one FIFO;
2. maps unowned native keys to XUI keys;
3. dispatches focused-target, global-hotkey, ancestor-bubble, and built-in
   default stages;
4. forwards text and marked-text records to the focused control;
5. synchronizes the Full-mode candidate popup;
6. refreshes the focused TSF text snapshot and candidate rectangle.

Applications must not add a second key whitelist or separately drain
`xgeTextGet` / `xgeImeEventGet` when using this helper.

## Why this replaces the old suppression fix

The old implementation used composition flags and selected Win32 messages to
suppress duplicate input. The current backend uses TSF
`ITfKeystrokeMgr` as the native ownership authority. A TIP-owned key never
enters the XGE ordered queue; every unowned key follows the normal XUI route.
Native presentation mode delegates to Windows and receives the committed
result through Sokol's ordinary `WM_CHAR` text route.

This removes both stale-composition suppression failures and application-level
special-key forwarding.

## Merge files

- `xui.h`
- `src/xge_input.c`
- `src/xge_ime_win32_tsf.c`
- `src/xui_proxy_xge.c`
- `src/xui_input.c`
- `src/xui_terminal.c`
- `test/test_input_event_queue.c`
- `test/test_ime_mode_contract.c`
- `test_xui/xui_input_test.c`
