# Window state and graphics context readiness

The Sokol window and the graphics context have separate readiness flags. GPU
creation, upload, drawing, readback, deletion and capabilities use graphics
readiness. Window handles, input/IME, clipboard, drag/drop and Sokol event-loop
wakeups keep their window check. No additional per-method forwarding layer is
introduced; the old conditional reads the appropriate private state field.

Sokol marks graphics ready only after its GL loader succeeds. Cleanup resets
builtin renderers while the context is still valid, then clears graphics
readiness and window state. The optional 3D context becomes active at that same
successful boundary. The engine still rejects checked shutdown while owned GPU
resources remain. This change does not activate arbitrary host contexts or make
EGL rendering work merely by changing the reported backend name.

EGL backend selection is transactional too. Capability or native initialization
failure leaves both existing backend tables intact, including callback addresses,
user data and name pointers. Only successful creation commits EGL/GLES selection.
The old code changed those tables before checking capabilities or creating any
context. Context/surface/display cleanup remains in reverse initialization order.

The default matched r23 builds pass 96 native checks per platform, including
actual Sokol/GL resource and texture-state cases, CPU/OOM/adoption cases and async
request regressions. The matched language candidates pass 30 Windows and 24
Linux JIT/static AOT checks, including real bounded render/readback cases. These
are default-backend qualifications; optional 3D/debug configurations remain in
their full integration phases.

An independent EGL TU check uses actual Mesa EGL 1.5 under Xvfb software
rendering. It verifies successful pbuffer creation/cleanup and seven typed native
failure stages at O0/O2; the old implementation fails the rollback controls.
Its two aggregate-private logging hooks forward to public xgeLogWrite. This is
explicitly a unit qualification, not an enabled matched EGL SDK or XGE renderer
qualification. Disabled-EGL TU controls and matched default aggregate controls
are recorded separately, so unsupported capability checks cannot stand in for
the enabled path.

The Linux private g_xge BSS object remains 6,984 bytes at this toolchain/profile;
the new state fits existing padding. That observation does not establish an
all-platform size or performance guarantee. No compiler/Cgen change is required.

## Remaining host integration

The old platform/graphics tables are currently descriptions: their function
pointers do not drive the normal engine loop. A safe language backend must have
an actual host protocol, owner/code lifetime, creator-thread admission, checked
startup/cleanup and GPU-resource quiescence. Copying these borrowed tables into
a language record would not supply those contracts.

Host activation, owned offscreen/EGL sessions, build configuration and language
entry points are not finished by this foundation batch. They must retain the
existing single-context/checked-Busy rule, keep native host capabilities explicit,
and use the native rendering core rather than forwarding UI/rendering methods
through a new language wrapper. Exact pins are in
`docs/validation/backend-context-20261110.json`.
