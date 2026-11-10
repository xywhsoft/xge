# Owning EGL renderer context

`xgeGraphicsContextCreateEglOwned` attaches a real EGL/GLES 3 renderer to an
initialized manual engine. The creator thread owns the single graphics epoch.
Choose PBuffer (`surfaceless=false`) or Surfaceless (`true`). Raw EGL helpers
remain explicit C interoperability and do not themselves attach the renderer.
Default SDKs without `XGE_ENABLE_EGL=1` return Unsupported before allocation or
code-owner admission. Windows EGL delivery is still unqualified.

The constructor refuses a foreign current EGL context. It reserves the engine
before acquiring any code/OS resource; reentrant engine shutdown, frames, new
contexts and GPU roots are refused during attachment. It validates GLES 3,
viewport limits, actual PBuffer dimensions and the complete typed GL table
before publishing renderer readiness. Failure rolls back in reverse order.
A non-NULL output on failure is an outstanding cleanup obligation, not success:
retain it and retry `xgeGraphicsContextReleaseOwnerChecked`. Never discard it.

The optional callback owner is held once and traced as a real owning edge. The
caller must supply a correct native storage/code lifetime; it is not safe to
pretend that generated code is process resident. `ShareOwned` adds one native
reference without copying the context. All methods and physical destruction
require the creator thread. `InfoGet` requires the exact record `size`.

Last release logically retires the renderer and cancels private queued draws
and uploads. Existing external GPU children retain a usable physical context;
new graphics roots are refused. Release returns Deferred without consuming the
owner until children, active passes, frame dispatch and other work are finished.
EGL cleanup and restoration of the previous thread client API are checked and
retryable. The code owner and engine reservation survive every incomplete step.
The completed graphics epoch requires engine Unit/Init before a new context.
Global `xgeUnitChecked` continues to return Busy when any graphics owner lives.

The context is exclusive. A raw caller that displaces it must call checked
`MakeCurrent` before using the renderer again. A failed recovery clears renderer
readiness before any synchronous callback reentry; physical current state is
checked afterward. Unavailable GPU texture updates/sampler changes are refused
without silently changing only their CPU shadows. Render-target readback never
returns the private CPU shadow as a screenshot; without a renderer it reports
InvalidState. Ordinary CPU-preloaded textures retain their CPU read/write APIs.

## Costs and qualification

There is one native allocation for context storage and the common owning-graph
record; EGL/driver allocations are outside XRT accounting. Admission, info,
sharing, recovery and retirement perform the required thread/RC/graph work.
Drawing uses existing XGE entry points and the same GL table. No parent lookup,
per-draw context retention, bridge function, boxing or second XRT is introduced.
Readiness gates in Pass now use the renderer state rather than window state.
This is not an end-to-end zero-overhead or hardware performance claim.

Tests run against actual shared/static engines at O0/O2 with a real Mesa driver:
XGE shapes, FBO pixels, GLES shader compilation, child retention, queue-only
ownership, creator affinity, graph finalization, owner-hook errors and XRT OOM.
Typed ELF interposition injects startup, GL lookup, surface query, cleanup,
client-API restoration and recovery failures. It explicitly injects synchronous
user reentry at a failed driver-call boundary; EGL does not itself dispatch the
XGE error callback. CPU memory balances and existing Sokol GPU paths are checked.
See `docs/validation/owning-graphics-context-20261110.json`.

The language integration separately discovered a module-global/code-residency
cycle. Native context ownership tests passing does not qualify that boundary.
xlang must resolve it before declaring the managed context fully delivered.
Replaceable host dispatch, owned WGL and broader graphics backends also remain
separate work; the old backend descriptor tables alone do not implement them.
