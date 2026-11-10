# Graphics startup and retryable EGL cleanup

The private GL slot inventory lives in `src/xge/render/xge_gl_slots.inc`.
Its exact typed entries generate declarations, definitions, loads, validation,
publication and unloading. All ordinary render functions are mandatory. In 3D,
either depth-clear function is sufficient; timestamp queries remain optional.

`xge_gl_load_ex` resolves into local typed variables and publishes only after
all required functions have been validated. Failure preserves the entire
previous table. Loading/unloading is a cold, exclusive context-owner operation,
not a concurrent reconfiguration API. The legacy loader remains available.
It forwards through one extra indirect callback per slot; callers using the
user-aware loader avoid that adapter. The loader itself creates no heap object
and adds no check/call to a drawing operation. Resolver/driver initialization
can allocate independently; this is not a claim about their total cost.

Sokol invokes its first frame immediately after its initialization callback,
even if that callback requested quit. XGE therefore clears its running flag
before returning from a failed backend admission or GL load, remembers the
checked startup error, and requests quit. A failed GL transaction must not
dispatch a language frame callback or dereference a GL slot.

## Caller-owned raw EGL records

`xgeEGLUnitChecked` returns an error when unbinding, destroying the context,
destroying the surface or terminating the display fails. It stops at that stage,
clears only successfully released handles, and preserves the remaining native
obligation in the caller's record. `bInitialized` becomes false once cleanup
starts; `bCleanupPending` remains true until it finishes. Retry the checked
function with the same record. Retrying a fully cleared record succeeds without
releasing anything twice. The legacy void function also retains the record on
failure, but the checked entry is required to observe the result.

Startup records each acquired native handle immediately. Rollback uses the same
checked reverse-order cleanup. A failed startup can therefore leave
`bInitialized == 0` and `bCleanupPending != 0`: do not overwrite, reinitialize or
discard that record. If rollback completes, the original startup error/stage is
preserved; if rollback fails, the last stage identifies the cleanup failure and
the original startup failure remains in the diagnostic log. The returned status
still describes the original startup failure.

This raw helper owns EGL display initialization/termination. It is not a shared
display/context interoperability protocol. The caller remains responsible for
exclusive use and thread discipline. Do not infer managed ownership from this
record or from the fact that an EGL context is current.

The extra flag changes the public record layout; all consumers require a matched
SDK and runtime fingerprint. No second XRT implementation is introduced.
`xgeOffscreenUnit` also preserves an unfinished EGL cleanup obligation; the old
raw Offscreen API still does not attach the XGE renderer.

## Qualification boundary

Private transaction tests cover every missing mandatory slot, both missing
depth-clear functions, optional timestamp absence, prior-table preservation,
legacy loading and unloading. Actual EGL/GLES tests cover PBuffer/Surfaceless
FBO pixels through that loader. Both system-GL include orders compile as strict
C11, C++11 and C++17.

Matched shared/static SDK tests inject single failures in all four cleanup
stages and in startup rollback, then retry against actual Mesa EGL. A Linux
static-engine test wraps its real `dlsym` lookup to remove one required GL
function and verifies checked failure without first-frame dispatch. Normal
Sokol rendering and resource lifetime remain separate real-device checks.

These contracts support the next owning graphics-session implementation.
They do not claim an owning EGL renderer session, replaceable host event-loop
dispatch, qualified Windows EGL delivery or hardware performance acceptance.
Evidence is indexed in `docs/validation/graphics-context-safety-20261110.json`.
