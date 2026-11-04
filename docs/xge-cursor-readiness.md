# Cursor state before native window readiness

Logical `xgeInit` (including manual mode) does not create a Sokol/native window.
Previously `xgeSetCursor` used only the logical running flag and could call
`XDefineCursor` with no X11 display through the real XUI queued-input pump.

`xgeSetCursor` now always records a valid requested cursor and changes the
platform cursor only while Sokol is valid. The native Sokol init callback applies
the recorded cursor when the window actually becomes ready. Invalid enum values
leave the stored state unchanged. No additional per-frame state or allocation
was added; readiness is checked only when a different cursor is requested.

`xge_cursor_readiness_test.c` changes every cursor repeatedly before logical
initialization, after manual-mode initialization (never Run/window creation),
and after checked shutdown. It checks state, repeated assignment, invalid input,
and memory-debug baseline. The original SplitLayout queued-input regression is
also retained. Actual OS cursor appearance/window rendering is outside these
headless tests; the ready-window application is source/compile verified.
