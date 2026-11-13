# X11 display connection failure

The Sokol X11 path called its fatal logger when XOpenDisplay failed, then
continued to DefaultScreen with a NULL Display. XGE's logger returned and did
not preserve a failed Run result. Both standalone vendored Sokol and the actual
static XGE reproduced SIGSEGV in C O0/O2 tests; this is not a language-wrapper
failure.

The local Sokol patch returns from that exact failure branch after discarding
the state buffers. At this point the display, cursors, window, graphics context
and app init callback have not been created or called. Discard releases the
clipboard/drop allocations and clears Sokol state; custom cursor slots are
still unbound. It does not invoke app cleanup for an app which never initialized.
XGE's fatal logger records XGE_ERROR_BACKEND_FAILED in the existing Run outcome,
so the blocking caller receives the first failure through the ordinary checked
status contract. No public signature or ABI changes.

The fatal log path uses the existing Run spin lock and first-error reference;
the logger has one extra source-level level-zero test. Healthy frames acquire
no new lock or allocation. Failed connection attempts still allocate and free
the two Sokol startup buffers; they are not allocation-free operations. In the
matched O2 host artifacts the Windows text section grows 128 bytes, Linux text
192 bytes and Linux unwind data 72 bytes; file lengths remain unchanged due to
layout padding. This is not a CPU-time equivalence claim.

There is no extra preflight X11 connection, per-frame check or allocation on
the healthy path, and no longjmp across native resource scopes. Manual/CPU Run
does not require a display. Failed automatic runs can retry in the same process
after a local display becomes available, or clear callbacks and close normally.
Existing explicit callback ownership still applies: a registered frame callback
remains retained until cleared or engine cleanup.

The two separate C fixtures cover unset, empty and syntactically invalid display
names, 12 consecutive failures, no app/frame/cleanup dispatch, exact buffer
balance, same-process real Xvfb/software-GL recovery and CPU manual mode. A native
allocation result is limited to Sokol's allocator/XRT accounting; Xlib/driver
process caches are not claimed to have zero total process memory. The frozen
SDK gates and dated validation receipt identify actual build and run scope.

This patch handles XOpenDisplay failure before any X11 resources exist. It does
not make every Sokol fatal branch recoverable. Later GLX/EGL/window failures,
allocator panics and a display-server disconnect need their own stage-aware
unwind; returning blindly or longjmp from every panic would be unsafe. The
default Sokol behavior without a custom logger still aborts on panic.
