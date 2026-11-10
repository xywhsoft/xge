# Engine initialization lifetime

`xgeInit` copies `sTitle` before successful publication. The caller can change
or release the original UTF-8 buffer after return. The copy survives until
successful `xgeSetTitleChecked` or final `xgeUnitChecked` cleanup. Other pointer
fields in `xge_desc_t` retain their explicit C caller lifetime contract.

`xgeSetTitleChecked` works on the creator thread after Init and during Run. It
allocates/copies before replacing the old title; OOM leaves the old pointer and
text intact. Before Run, it changes the future window title. During Run, it also
updates the native window. `xgeGetTitle` returns a creator-thread borrowed
snapshot; another successful setter or Unit invalidates it. It returns NULL
when unavailable. This pointer must not escape as an owned string.

`xgeInitWithCode` additionally holds one real `xge_callback_owner_t`. This is an
independent engine obligation, even in manual mode with no callbacks. Failed
Hold acquires nothing; a successful Hold followed by failure is rolled back
exactly once. The initializer rejects nested Init/Unit while admission is in
progress, and does not hold a publisher lock while calling arbitrary owner code.

Successful initialization keeps that reference across Run, module Close and
Busy shutdown. Final cleanup clears the native slot and drops the owner last,
while reentry admission remains closed. Failed Drop still consumes its acquired
reference and is reported as a checked error; repeated Unit never retries it.
The owner must cover the complete native return tail, as the resident xlang code
lease hook does. Raw C `xgeInit` supplies no implicit code owner.

Windows and Linux each pass 20 build/run rows at O0/O2, including actual bounded
window titles, alias-safe replacement, title and input-queue OOM rollback,
owner Hold/Drop failures, reentry, foreign threads, Busy retention and balanced
heap cleanup. Existing owned graphics/device checks also pass 44 rows per OS.
These results use the explicitly frozen r24 cohort, not all concurrent XUI work.

In a warmed native process, a title of `engine cost` requires two live XRT
allocations totaling 61,452 requested bytes: the title and ordered input queue.
The process-resident C owner itself allocates nothing. These are cold path
observations; they are not a performance comparison with a JIT code lease.

The internal draw queue can be the last owner of a Texture and currently makes
strict Unit return Busy. The independently reproduced case and the pending
two-phase shutdown design are separate from this initialization fix.

Evidence: `docs/validation/engine-initialization-20261110.json`.
