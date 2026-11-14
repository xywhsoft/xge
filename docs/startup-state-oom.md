# Pre-window state allocation failures

Sokol initializes its clipboard and optional dropped-file storage before opening
a display or window. XGE's logger records fatal errors and returns; the old
initializer then asserted or wrote through NULL after a failed allocation.

The private initializer now returns success/failure. Every platform caller
returns before further setup when it fails. Partial discard frees only buffers
that exist, using the original allocator, and clears state. The application init,
frame and cleanup callbacks do not run. A later `sapp_run` can retry. The dropped
file buffer's signed-int extent is checked before multiplication.

XGE records `SAPP_LOGITEM_MALLOC_FAILED` as `XGE_ERROR_OUT_OF_MEMORY` (-5).
Other fatal platform failures retain `XGE_ERROR_BACKEND_FAILED` (-8). Existing
Run ownership/error delivery remains responsible for crossing a language boundary.
No public ABI, declaration wrapper or per-frame allocation is added. The added
branches execute during startup/allocation and on fatal logging.

`tools/test/check_state_oom.py` compiles the real vendored Sokol and links the
matched direct-XRT static engine at O0 and O2. Its pre-window tests inject each
buffer failure repeatedly, verify zero callbacks and balanced storage, and check
overflow rejection. `--graphics` also runs a real local window successfully in
the same process after the failures. Linux can use Xvfb with TCP disabled.
The pure Sokol test enables both buffers on Windows; XGE itself disables native
Sokol drag-and-drop there, so its Windows gate injects one native buffer stage.

This is a bounded startup fix. It does not claim safe unwinding from every later
window/GLX/EGL/default-icon allocation or display disconnect. Returning NULL from
the shared clear helper does not make its other callers automatically checked.
Apple, Android and web callers are source-reviewed only unless separately tested.
Default Sokol logging without a supplied logger retains its panic/abort policy.

The 2026-11-14 acceptance receipt is `validation/startup-state-oom-20261114.json`.
Windows/Linux each pass 12 native O0/O2 build/run steps with real same-process
window recovery. Linux additionally passes the 12-step X11 startup regression,
six-step GL-load-failure regression and two-step standalone Sokol ASan/UBSan/leak
gate. These 44 positive process rows exclude the 16 historical expected-abort
rows and eight SDK build/staging steps. Production code remains O2.

Qualified object reuse rebuilds only the changed aggregate in each host (163
other objects reused), plus the generated static contract for each archive
(164 other objects reused). It is not a full source rebuild. The two host file
lengths are unchanged; Windows text grows 128 bytes, Linux text shrinks 192
bytes in this matched comparison. No CPU-time equivalence is asserted.
