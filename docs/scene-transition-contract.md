# Scene transition contract

The caller-owned `xge_scene_t` API uses one transition state machine. Records
must remain valid while stacked or executing; `onFree` runs after removal and
may free the removed record. This API does not itself own C scene storage or
language callback closures. Managed scene ownership is a separate integration.

## Transitions

- One record occupies at most one stack slot. Setting the sole current scene
  and replacing the current scene with itself are no-ops.
- A failed Leave keeps the original top published and entered. It can be
  retried; no Free runs for that entry.
- Successful Leave unpublishes before Free. A Free error cannot resurrect the
  record. Pop still attempts to resume the previous scene.
- A failed Enter removes the candidate; Push/Replace resume the underlying
  scene where applicable. Enter rollback does not call Free for an activation
  that never completed.
- Clear progresses from the top. It continues past a Free error but stops at a
  failed Leave. The first failing status and diagnostic win, including a first
  failure with no diagnostic. An unrelated ambient diagnostic survives success.
- Checked engine shutdown clears scenes before other publishers or initialized
  state. Failed scene cleanup leaves the engine initialized for inspection and
  retry; already removed scenes are not reconstructed. Existing GPU-owner Busy
  admission still applies before shutdown begins.

Stack transitions cannot nest inside Enter/Leave/Pause/Resume/Free. Lifecycle
callbacks may configure update scheduling and dispatch events. Update and event
callbacks may switch scenes. Callback activity prevents engine destruction
until the physical callback return. Recursive manual frame stepping is Busy.

## Frame and thread boundaries

Monotonic stack and scheduling revisions invalidate the current frame before
any further old-record access or fixed-step accumulator update. Push followed
by Pop back to the same pointer still invalidates it. A replacement starts its
update/draw work on the next frame. The original frame delta is restored on
every exit.

Scene access belongs to the initialized engine's creator thread. Wrong-thread
queries report State without reading stack payload; output parameters remain
untouched. Scheduling requires a finite positive step and positive update cap.

## Evidence

`tools/test/check_scene_transitions.py` exercises 23 transition, failure,
reentrancy, thread and positive lifecycle modes at O0/O2 against the real host
engine on Windows and Linux. The old implementation fails the ten original
regression modes at both optimization levels. The intermediate candidate's
over-restrictive lifecycle scheduling/event behavior is preserved as a separate
red test; the final candidate passes these positive modes too.

The matched existing resource, callback and real OpenGL gates also pass.
Pins and external report paths are in
`docs/validation/scene-transitions-20261110.json`. These results certify this
native state machine, not managed Scene graphs, a language binding, or a release.
