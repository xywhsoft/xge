# Immutable backend references

Platform and graphics descriptors now have separate, typed native references.
`CreateOwned` copies the descriptor and its optional C-string name into one
allocation, retains one supplied payload/code owner, and publishes only after
that Hold succeeds. NULL and empty names remain distinct. Function pointers
retain their exact C types; they are neither erased nor invoked by creation.

`ShareOwned` acquires an independent reference to the same immutable entity.
Closing one reference leaves the others valid. `Describe` borrows the const
descriptor while a real reference remains held and clears its output on
failure. Ordinary access/share and final destruction follow the creation
thread. Atomic nonfinal reference release follows the existing native kernel
contract. No engine or graphics context is retained or started by a snapshot.

The supplied owner must cover the callback payload, code and complete native
return tail. NULL/empty owners are for externally guaranteed static C payloads;
they are not a safe shortcut for a language closure. A failed Hold owns nothing.
A successful Hold followed by an error is rolled back. Drop consumes its one
reference even when it reports failure, and is never repeated for that reference.
A creation failure may leave a non-NULL cleanup obligation if rollback cannot
consume it; callers must observe the output and use checked release.

Both families instantiate the existing native ownership kernel and expose exact
Ops/adapter/preparation/quiescence/finalization/completion descriptors. Their
trace visits the payload/code owner's real reference. Claiming or retiring a
snapshot disables reading, and finalization removes its owner edge before
calling user Drop outside mutation locks. Cleanup failures remain checked.

The source-unit gate uses frozen 09d-XRT Native baselines, overlays only these
files, and links the exact qualified xlang XRT library. Both Windows and Linux
pass O0/O2 lifecycle tests, C11/C++11/C++17 public headers and a complete engine
translation-unit syntax check: eight commands per OS. Tests cover independent
aliases, name copies, exact callback/userdata preservation, trace edges and
protocol identities, claim/restore/finalize/clear/finish/drop, wrong-thread
rejection, failed Hold, failure after Hold, consumed failed Drop, malformed
owners, 16 allocation prefixes and balanced warmed live allocations.

The final tool copies only include/src/extlibs/modules/single compiler inputs.
Earlier broad snapshot copying included unrelated apps/artifacts; those old
reports remain separate. Linux's selected baseline is retained as an indexed
ZIP and unpacked under `/var/tmp/xlang-validation/` for local filesystem I/O.
Final exact reports and hashes are indexed from the xlang provenance receipt
`xge-backend-owned-20261116.json` under its development release materials.

This is a native prerequisite, not a rebuilt SDK or delivered xlang binding.
The public header changed, so the final SDK must regenerate its ABI fingerprint.
The old profile flags are used only for the source integration syntax gate.
The snapshots do not install descriptors, replace Sokol, drive callbacks, or
claim that the legacy descriptor-only setters implement a host loop. The host
session/Run scheduling decision and generic userdata receiver projection remain
separate work. Existing engine/context Busy rules and drawing paths are unchanged.

Costs are explicit: one native allocation plus a name copy when present, one
payload/code Hold/Drop per entity, reference/graph/thread bookkeeping, and no
additional descriptor/name copy on sharing. No language/C CPU equivalence or
complete graph-collector, provider-unload or backend-rendering claim is made.
