# Explicit owning host sessions

`xgeHostSessionCreateOwned` attaches a manual XGE engine to a platform/graphics
backend pair and a native OpenGL 3.3 or GLES 3 context. The existing Sokol `Run`
and owning EGL APIs keep their separate entry points. Backend snapshots alone
still do not initialize a renderer or run an event loop.

Create the engine with `XGE_RUN_MANUAL`, create immutable owned backend snapshots,
then supply framebuffer dimensions, DPI scale, framebuffer ID and context hooks.
The session shares both snapshots and holds the hooks owner independently.
Callers can release their own snapshot references after successful creation.
Both snapshots and the hooks must retain/trace callback payload and code through
the complete callback return tail. Empty owners require independently guaranteed
static C code/payload; they are not a substitute for language callback ownership.

`makeCurrent` and `isCurrent` are required. `isCurrent` returns 0 or 1, or a
negative error. The optional resolver receives a name borrowed for that call;
without it, XGE owns a system GL/GLES library reference and loads exact typed GL
slots. Version, backend kind, viewport limits and framebuffer completeness are
validated against the real context. The session reserves one graphics epoch;
another renderer cannot attach concurrently.

The creator thread drives frames explicitly:

```c
int events = xgeHostSessionPoll(host); /* nonnegative backend result retained */
if (events < 0) { /* checked error handling */ }
int status = xgeHostSessionBegin(host);
if (status == XGE_OK) {
    xgeClear(XGE_COLOR_RGBA(20, 25, 30, 255));
    xgeShapeRectFill((xge_rect_t){10, 10, 40, 20}, XGE_COLOR_RGBA(255, 90, 20, 255));
    status = xgeHostSessionEnd(host);
    if (status == XGE_OK) status = xgeHostSessionPresent(host);
}
```

An accepted begin is paired with exactly one end. Begin rolls back accepted work
if a callback or subsequent upload reports an error. End consumes its callback
invocation even on failure; present is then refused. A successful end leaves a
pending presentation, consumed by `present` or explicitly `discardFrame`.
Present consumes its invocation even if its void callback reports an error.
Destruction never presents an unfinished frame. Registered scene/frame callbacks
are not implicitly executed by this explicit loop; applications dispatch their
chosen frame logic themselves. `xgeFrame()` returns Busy while a host owns the
renderer, preventing its legacy frame reset from corrupting the explicit state.

`resize` queues CPU metadata and is permitted inside `poll`; the next begin
validates and applies it. Invalid resize leaves the previous dimensions and GL
framebuffer binding intact. Other same-session mutating callback reentry returns
Busy. `info` reads cached state without invoking a callback; `isCurrent` performs
the actual hook query. A wrong thread is refused before allocation or dispatch.

Last release defers without consumption for an open frame/pass or active
callback. Otherwise it retires the renderer and cancels private pending uploads.
GPU children retain the current context and delay physical teardown; new GPU
owners are refused. Release children and retry the retained session obligation.
Audio children are independent of this renderer. After a completed graphics
epoch, engine Unit/Init is required before attaching another context.

Cleanup stops the renderer, calls graphics unit, retires GL slots/libraries,
releases the native context, calls platform unit, releases snapshots and finally
drops the hooks owner. The context release hook reports `{status, consumed}`:
false preserves the exact native duty for retry, while true consumes it even if
status is an error. It must handle partial platform/graphics initialization too.
Successful individual steps are never repeated. A failed factory can return a
non-NULL output containing this same checked cleanup obligation. Keep it until
release reports consumption. The first diagnostic survives later cleanup errors.

There is one cold session allocation plus the separately owned snapshots and
callback environments. Frame boundaries perform existing scope/ownership checks
and direct typed backend callbacks. No dispatcher, forwarding draw wrapper or
per-draw owner lookup is introduced. The local 1,000-frame test observes zero
XRT allocation/free/realloc; this excludes driver allocation and does not claim
CPU equivalence with handwritten C.

`tools/test/check_host_session.py` tests actual local WGL/EGL drawing with shared
and static builds, O0/O2 callers, creator affinity, callback reentry, explicit
states, native graph protocol steps, OOM prefixes and consuming/retryable cleanup.
Evidence is indexed in `docs/validation/host-session-20261117.json`. These native
tests do not prove collectible language cycles, physical module unload, cold OS
TLS allocation faults, all backend SDKs or a matching xlang/IDE installation.
