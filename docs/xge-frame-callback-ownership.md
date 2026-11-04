# Retained frame callbacks

`xgeSetFrameCallbackOwned` registers a real owner without starting the engine.
The slot owns one registration reference until replacement, explicit clear,
or `xgeUnitChecked`. Replacement commits the new slot before retiring the old
owner; old cleanup failure is reported while the new registration remains.
`xgeRunOwned` uses the same slot. Manual/mobile return does not end ownership.
Raw `xgeRun` retains its caller-managed userdata contract through that slot.

`xgeFrameCallbackDispatchChecked` runs the registered callback without drawing,
opening a window or starting an audio device. Status zero writes the callback
payload: zero continues, positive values request normal loop termination.
Negative callback or cleanup status leaves output unchanged. Recursive and
concurrent dispatch are refused; an active blocking Run restricts callback
dispatch and engine shutdown to its thread. Quit is allowed during a callback.
Unit refuses an active owned snapshot and may be retried after it returns.

Both native publisher paths, manual update and Sokol frames, use this dispatch
mechanism. A blocking Run records the first escaping negative callback or
teardown status independently from its diagnostic pointer. The diagnostic is
retained before later loop cleanup can consume TLS. A NULL first diagnostic
remains first. Positive stop requests alone remain successful completion.
Errors handled inside a callback do not poison the enclosing Run.

Metadata capture/read-pin happens under the publisher spin lock. Hold, Drop,
callbacks and error reference operations run after unlocking. A reader pin
keeps registration metadata valid while Hold reenters and clears/replaces the
slot. A separate real owner snapshot covers the callback's physical return.
Callback failure and later snapshot cleanup failure keep the original status,
including an original failure without a diagnostic. The XRT used by the
engine and owner must already be bound consistently.

Owned registration allocates one small native node. Successful frame dispatch
allocates nothing but performs publisher synchronization, a reader pin and an
owner Hold/Drop snapshot. Raw dispatch also uses the slot lock. These are
native API costs, present for a handwritten C client of the same API; this
does not claim zero total overhead for language callback/lifetime guards.

Global registrations are explicit external roots. Clearing a slot does not
by itself prove that another native thread has returned from dispatch. Hosts
must stop/join publishers before unloading their libraries. A language host's
checked retirement channel also preserves generated/native physical tails.

The CPU gate exercises real dispatch, registration OOM, unchanged output on
failure, self-clear, Hold reentry, snapshot cleanup and busy Unit. It never
initializes an engine/device. The exact shared first-outcome helpers are
tested separately with an original NULL and an original allocated error.
Actual windowed Sokol execution remains outside this headless gate.
