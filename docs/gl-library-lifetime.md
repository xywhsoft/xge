# GL resolver library ownership

The private resolver owns the library reference supporting the last committed
typed GL table. It no longer loses that reference on a backend change. Loading
a different backend acquires a candidate and asks `xge_gl_load_ex` to validate
the complete table before publication. Failure keeps the old table and its
library. Success transfers the candidate and retires the previous reference.

A failed `FreeLibrary`/`dlclose` retains the exact handle. One outstanding
rollback/retirement record is enough: another load must settle that record
before acquiring anything else. After a successful table replacement, a failed
old-library close is a checked failure **after commitment**; the new table is
still supported by its active owner. This is private startup/worker machinery,
not a new public partial-result API.

`xgeUnitChecked` still first refuses live graphics/audio children and active
work. Only after those admissions and the render-worker join does it unload
the slots and release the resolver libraries. A failed close keeps both its
handle and the engine code owner, blocks initialization, and allows another
creator-thread Unit to retry. It does not claim successful resource retirement.

Logical Unit during an active Sokol loop continues to stop the engine. Physical
resolver/code retirement is deferred until Sokol cleanup has finished XGE's
renderer resources. Sokol retains its **own** OS-library reference through its
later platform teardown. Final code-owner Drop runs with callback and graphics
admission closed. A deferred cleanup failure is remembered by the active Run;
its retained duty remains available for checked Unit retry. Normal Run still
ends the graphics epoch; Unit/Init is required for another healthy Run.

Sokol cleanup clears the typed slots once its context can no longer serve XGE
draws. A normal initialized engine retains its resolver reference until Unit.
Owned EGL contexts continue to use their direct EGL resolver and existing
resource reservation; they are not made into owners of this separate cache.
This change adds no concurrent draw/table-replacement guarantee and no API for
mutating a live backend with retained GPU objects.

There are no new draw-time operations, allocations, dynamic boxes or forwarding
wrappers. The owner records are static, and the candidate is a stack record.
Cold loading, backend changes and retirement have real library open/resolve/
close work; checked failure bookkeeping is not a CPU-equivalence claim.

`tools/test/check_gl_libraries.py` checks the actual typed loader with addresses
that are inspected and **never invoked**, including every required omission,
failed rollback, failed postcommit retirement, bounded ownership and repeated
backend changes. C11/C++11/C++17 at O0/O2 pass on Windows and Linux (36 process
steps per OS).

`tools/test/check_gl_library_engine.py` links the actual O2 static engine to
O0/O2 clients and one complete XRT. PE import-pointer/ELF entry wrapping counts
real GL library references and injects OS-close failure; it also tests code
retention, GPU Busy, reentrant logical Unit and repeated actual windows. Each
OS passes ten current build/run steps. Two old-library builds and two precise
old-cache rejections per OS are recorded separately. The old library leaves
one resolver reference after Unit; the new library leaves zero. No fake GL
function is executed in this device gate.

Windows's matched hosted engine additionally passes the existing native owned
graphics and bounded GPU/readback cases. Linux's full GLX startup-unwind engine
gate now requires zero references after Unit. A full compiler consumes each
matched candidate SDK; detailed reports, size deltas, reuse counts and scope
limits are pinned in `docs/validation/gl-library-lifetime-20261114.json`.

Windows r1's deferred-Unit fixture incorrectly expected only one explicit
library owner during startup; Sokol owns one as well. The final fixture counts
both. Linux r1 completed its cases but failed the final input-stability check
because that shared fixture was corrected during the run. Both original reports
remain; r2 qualifies the unchanged final fixture. No product edit or swallowed
failure was used to get those final passes.

macOS, Android and WebAssembly device behavior is not certified by these two
desktop gates. Other backend/disconnect paths, unrelated mutable XUI changes,
and complete ecosystem/IDE delivery remain separate acceptance work. Native
SDKs are isolated candidates, not an installed release.
