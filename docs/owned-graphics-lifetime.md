# Native owned graphics and checked engine shutdown

Native owned factories reserve a cold graphics lifetime obligation before
allocation and transfer it to the successful record. Failed creation clears
the output and releases the reservation. Final native release frees the outer
record; caller-storage initializers keep their existing explicit C contract.
Do not copy an owned record or pass it to a caller-storage initializer.

`xgeUnitChecked` returns `XGE_ERROR_BUSY` while these obligations exist,
before changing engine state or retiring registrations. Release resources and
retained user slots, then retry. `xgeGraphicsOwnedResourceCount` includes
creation in progress, not just GPU allocations. Admission/counting is outside
the context record so `xgeInit` cannot erase an outstanding obligation. A
spinlock covers only admission/count transitions, never allocation, GL or user
callbacks; draw/update methods have no added epoch lookup or global lock.

Initialization has its own cold admission reservation: duplicate Init and Unit
cannot enter a partially initialized context, and failure releases that
reservation for retry. Owned creation binds to the engine thread (or the
pre-Init owner thread). Foreign-thread factories are rejected before native
allocation/work. Preloaded texture queues and accounting survive Init, including
failed Init; a required upload-queue allocation failure rolls creation back
rather than returning a texture that will silently miss its first upload.

Physical backend cleanup may be forced by the window system. It closes
creation admission and leaves the context ended. Old owners can be released
without calling dead GL functions; they must be released before checked Unit
and a fresh Init/Run. Reusing the ended context is refused. Internal raw
renderer records are not counted as external owners, avoiding a shutdown
dependency on the very cleanup that Unit must perform.

The native owning edges are explicit:

- Checked Material shader slots retain a shader; legacy void shader setters
  remain borrowed. Variant-set shaders own their enclosing storage, allowing
  a retained shader to outlive its set.
- Retaining a RenderTarget texture retains the parent target. Resize refuses
  active passes or external texture references. Transactional resize preserves
  old storage on failure and relocates the prepared upload-queue node, rather
  than publishing the address of its temporary candidate.
- Owned Pass retains its target. Checked release reports status separately
  from owner consumption. An active non-current Pass is Busy and unchanged;
  current Pass cleanup restores the native LIFO state before releasing storage.
- The default owned XUI Proxy reserves one engine obligation. Font/Surface and
  retained paints keep its physical storage alive; the obligation is dropped
  after physical Proxy release, not during semantic graph finalization.

Texture/Font native owned queries distinguish outer allocations from raw C
records and embedded target textures. The xlang rollback path uses them to
avoid freeing native-owned storage twice.

Validation: `tools/test/check_graphics_lifetime.py` covers the production gate
at O0/O2, 8 workers x 10,000 acquire/drop attempts and 5,000 competing closes;
Linux additionally uses ASan/UBSan. `tools/test/check_owned_graphics.py` covers
CPU/OOM/retained slots, existing adopted storage, Proxy/paint and callback
regressions. Its explicit `--graphics` case tests nested Pass restoration and
pixel readback, shaders/variants surviving their set, ended-context Busy,
release and restart through two bounded real OpenGL contexts.

Windows and Linux each passed 20 build/run checks; Windows uses a desktop
context, Linux an Xvfb display. This establishes OpenGL behavior, not hardware
performance or full engine sanitizer coverage. The independent production
gate passed 4 steps on each platform. A stale Proxy-test Busy assertion was
updated to the existing `4d5257f6` deferred-raw-paint result; unchanged storage
and no attempted allocation remain required.

Exact source/build/test inputs and logs are outside Git under
`F:/xlang-validation/xge-full-20261110/`: `native-freeze-r7.json`,
`native-host-win-r7/`, `native-static-win-r7/`, `native-host-linux-r7/`,
`native-graphics-win-r8/`, `native-graphics-linux-r8/`, and
`domain-{win,linux}-r5/`. The host builds use an explicit, separately built
runtime matching the xlang XRT header. The runner separately pins immutable SDK
headers and optional current test sources (`--test-root`); neither the upstream
XRT header nor a source checkout can silently replace the built SDK. No release
installation is implied.

The ended-Pass case also verifies that checked End failure and successful owner
consumption are independent: it restores the stack, drops the target, reports
NOT_INITIALIZED, and permits the following Unit. It must neither leak an owner
nor retry cleanup on consumed storage.
