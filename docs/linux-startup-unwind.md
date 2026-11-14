# Linux GLX and EGL startup unwind

XGE's Sokol logger records a fatal startup failure and returns it through the
checked Run boundary. Private GLX/X11/EGL initializers must therefore stop
after logging, rather than using a missing visual/window/context. Their bool
results now propagate to a stage-aware cleanup path. No application init,
frame or application-cleanup callback runs before successful startup.

The path balances startup buffers, temporary FBConfig storage, cursors,
colormap/window, graphics drawable/context, initialized EGL display and Sokol's
own `dlopen` reference. A failed EGL initialization is not terminated as though
it had succeeded. A context is unbound only when this startup made it current.
Native custom cursors are unbound before closing their X11 display, including
on normal completion. Subsequent runs can recover in the same process.

X11 window and GLX drawable requests can report asynchronous errors. Their
temporary handler is synchronized and the previous handler restored before
failure logging or return. Invalid resource IDs are cleared before teardown.
The contract follows the [Xlib error-handling API](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Using_the_Default_Error_Handlers).
This remains a process-global Xlib handler used on the engine thread, not a
new concurrency protocol for unrelated libraries issuing Xlib calls.
EGL configuration output is bounded by the supplied array, consistent with
the [EGL specification](https://registry.khronos.org/EGL/specs/eglspec.1.5.pdf).

The new GLX MakeCurrent log item is appended, preserving every existing Sokol
log-item value. Public XGE headers, ABI fingerprint, signatures and bindings
are unchanged. Default Sokol panic behavior without a logger remains intact.

The actual vendored Sokol is tested at O0/O2 for 18 GLX/X11 failure stages and
16 EGL stages in both desktop OpenGL and OpenGL ES 3 profiles. Every case
fails three times, balances acquired resources, restores the application's
error handler, then opens a real local software-graphics window. Recovery
also leaves a real custom cursor bound so platform teardown must release it.
Both C++11/C++17 implementations compile. The old header independently
reproduces visual-failure SIGSEGV in all three profiles and both optimizations;
those rejection runs are recorded separately from successful acceptance.

The actual O2 direct-XRT static engine is called by O0/O2 clients for the same
18 GLX stages. Failed startup permits direct retry. After a completed graphics
Run, the existing lifecycle is ENDED, requiring Unit/Init before another Run.
The gate honors that contract and opens three healthy windows in successive
engine lifetimes after each failure sequence. XRT live storage and invalid/
double-free counts remain balanced for failures. This does not replace full
ownership, all-backend or hardware-GPU acceptance.

XGE's separate resolver in `src/xge/core/xge_impl.c` currently retains one
process-wide OpenGL library reference after a healthy run. The gate explicitly
records this existing cache and proves that Sokol adds no accumulating owner
across healthy lifetimes. This is not a claim that the resolver has retired;
its backend-switch and module-retirement ownership still needs separate audit.

An early EGL matrix found an unexpected XOpenDisplay failure before its chosen
fault stage. A 50-run reset/no-reset comparison reproduced one connection
failure with Xvfb reset enabled and none with it disabled. Acceptance keeps
the local server alive with `-noreset`; it does not retry or suppress a failed
targeted stage. Historical failures and all individual process logs remain
archived. Xvfb TCP listening is disabled throughout; there are no remote or
network/security tests.

The O2 host/static SDK uses qualified object reuse, not a full rebuild. Host
relinks use the original full CodeDocument parent and compile the aggregate
and the already-qualified autosave Document TU (162 reused); static builds
compile the aggregate/generated contract (164 reused). Native code, source
freeze, toolchain flags and public ABI hashes are pinned in
`docs/validation/linux-startup-unwind-20261114.json`.
The same host comparison changes file size by zero on Windows/Linux, code
size by +0/+832 bytes and BSS by +0/+32 bytes. All new checks and releases are
startup/teardown work; no per-frame operation or allocation is added in the
modified source. There is no CPU-time equivalence claim.

Language JIT/independent dynamic/static acceptance and Windows regression use
their own receipts. Later display disconnects and non-GL platform failure
branches remain separate work. No final runtime/SDK/IDE installation is made
by this native acceptance.
