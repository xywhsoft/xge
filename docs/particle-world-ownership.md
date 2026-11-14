# Managed CPU particle-world lifetime

`xgeParticleWorldCreateOwned` and `CreateOwnedWithCode` allocate the same native
world and preserve its public handle type. They add creator-thread admission,
actual shared references, active-call pins and ownership-protocol capabilities.
There is no second simulation, operation dispatcher or forwarding resource.

The original `xgeParticleWorldCreate` remains the caller-managed C API. Its
borrowed collision callback/user and externally synchronized thread-transfer
contract remain available. Managed creation rejects either borrowed collision
field: persistent owned collision registration is the next unfinished step,
not an untraced raw-pointer shortcut through this factory.

## Native contract

| Entry point | Contract |
| --- | --- |
| `CreateOwned(out,desc)` | NULL desc uses native defaults; clears out on failure |
| `CreateOwnedWithCode(out,desc,owner)` | Holds one independent code owner; snapshots descriptor and owner before arbitrary Hold code |
| `ShareOwned(world,out)` | Creator-thread checked native retain; no allocation or world copy |
| `ReleaseOwnerChecked(world,&consumed)` | Consumed is authoritative even when cleanup reports an error |
| `FreeChecked(world)` | Checked release for raw or managed worlds |
| `Ownership(world)` | Actual managed native owner reference; raw worlds return an empty reference |
| `OwnershipAdapterV1` / `PreparationV1` / `QuiescenceV1` | Exact claim/restore, preparation and activity interfaces |
| `OwnershipFinalizationV1` / `CompletionV2` / `ReadReady` | Resumable finalization, post-Clear finish/drop and read admission |

Every native simulation/state-query entry admits a managed active reference **before**
reading world state, including the scalar State, EventPoll and Stats queries.
Nonnegative State/EventPoll values retain their native meaning. Negative errors
and fresh diagnostics remain independent checked failures. A wrong-thread
status-returning call reports Busy; Stats returns its zero value and a checked
diagnostic because its ABI has no separate status output.

An active Visit may release the last durable owner. Its actual native call pin
keeps the body alive until the visitor and operation finish. Other mutations
retain the original busy/reentry restrictions. Whole-graph claim is refused
while any managed operation is active. A non-last reference may be dropped
without finalizing on another thread; final destruction still requires the
creator. A refused last release retains that reference for checked retry.

Native cleanup clears the simulation's pool, arrays, slot map and retained
immutable definitions once. The code owner remains traced until post-Clear
finish, and its Drop consumes its reference on success **and failure**. Failed
Drop may have no diagnostic. Earlier failure, including its absence of a
diagnostic, wins over later cleanup errors. Finish retries do not invoke an
already-consumed Drop again. Legacy void Free publishes a checked diagnostic
for managed cleanup failure instead of silently losing a false/NULL outcome.

The code owner's hooks must be resident or protect their complete native
return tails, matching the existing native callback-owner contract. These APIs
do not solve the separately pending language module/global/CodeLease cycle.
They do not make a callback-scoped snapshot pointer independently owned.

## Allocation and execution costs

The old raw record is 496 bytes on the qualified x64 targets; it remains 496.
Four boolean flags use the existing private boolean-state slot. Only a managed
creation allocates the 568-byte native record containing its reference count,
code owner and lifetime metadata. Both variants use five XRT allocations for
the same small configuration; measured live requested storage is 1232 versus
1304 bytes. Share adds a reference without another allocation.

Raw calls now execute a mode-admission check. Managed calls additionally check
creator/state/claim, maintain reference/activity counters, participate in XRT
ownership transitions and preserve checked outcomes. This runs at operation
boundaries, not inside the particle integration loop. Internal public calls,
such as finished-effect auto-release, also retain their own admission. Prepared
1000 Update(0)/Stats pairs have zero heap operations in both variants; this is
not a CPU throughput or zero-cost bookkeeping claim. WithCode also snapshots
the small descriptor and owner before invoking Hold, and runs the actual user
Hold/Drop hooks.

## Evidence and remaining integration

`tools/test/check_particle_world_ownership.py` uses exact O2 hosted/static SDKs,
the sole full compiler XRT, and independent GCC O0/O2 clients on Windows/Linux.
Raw and managed profiles reuse all 10,237 existing kernel assertions. The
managed profile keeps caller-borrowed collision tests explicitly raw; it does
not certify owned collisions. Additional cases cover shared counts, creator
admission, active last-owner release, claim refusal/restore, finalization/finish,
false/NULL cleanup, diagnostic preservation, Hold rollback, constructor input
replacement and allocation-prefix cleanup with zero live-allocation growth.

The legacy constructor control checks the precise missed-input-snapshot
assertion in an isolated process, without dereferencing expired storage. New
matched SDKs also run the existing xlang particle-definition lesson and JIT /
dynamic / static acceptance. Source-freeze manifests exclude generated build
cache records; original failed preparation reports remain preserved separately.

Persistent owned collision callbacks, renderer/GPU ownership, language World
declarations, snapshot/conditional-result projection and final IDE/runtime
delivery remain unfinished. See the small validation receipt for exact paths,
hashes, image-size deltas and execution counts. No GPU or network tests run here.
