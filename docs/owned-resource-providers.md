# Owned resource providers

Resource registry entries own their copied scheme/root and immutable callback
metadata. Registry publication, lookup pinning and detachment use one short
spin lock. No allocation or arbitrary owner/provider callback runs under it.
An in-flight load and a raw payload each retain real entry references; clearing,
replacing a mini-program bridge, or clearing inside a callback cannot erase
its selected free function, root or user pointer. This registry is outside the
engine context, survives Init, and is cleared by checked Unit even before Init.
Individual resource records and mutable callback state remain caller-synchronized.

The legacy C provider still borrows its user data / pack object. Copying their
pointer does not acquire an owner. Those callers must keep that state alive
through their returned payload's Free. C output resource records must be empty
before Load or LoadMemory; a new load does not silently release a prior result.
Free clears a record before invoking its deallocator, making reentrant Free inert.
Empty raw payloads release registry metadata without calling free(NULL).

The new AddOwned / XPackProviderAddOwned APIs retain the declared callback owner
as an external root until Clear/Unit. Each call keeps an independent owner
snapshot through its entire return tail, uses existing checked callback failure
handling, and may clear itself. Callbacks run on the loading caller. XGE async
image/texture loading detects registered schemes and continues to execute them
on its caller rather than silently moving language callbacks to a worker.

Read callbacks receive a callback-scoped xstrview and resource output. OutputSet
copies the exact byte count once, accepts NUL/high bytes, and checks the int-sized
native limit. Replacement allocates before discarding old bytes. A failure
poisons that invocation even if the callback ignores the returned status. Failed
calls publish no partial payload. Empty output is valid. A successful owned
payload is independent of the provider and retains no language callback, avoiding
a hidden resource -> closure edge or provider/resource reference cycle.

Owned XPack callbacks capture the pack object directly. Native joins the copied
root to the requested pack path, strips leading separators and rejects '..'
segments, preserving the existing XPack behavior. It does not decode URI escapes
or canonicalize arbitrary filesystem paths. Callbacks must use the intended pack
API rather than interpreting the result as an unrestricted OS filename.

Costs are explicit: registry entry/string storage at registration, callback owner
Hold/Drop and real snapshot references, one lock/pin per lookup, one byte copy in
OutputSet, and a pack-path allocation when applicable. Native raw resources now
retain an entry instead of allocating a second free-snapshot per load. Existing
ResourceLoadMemory already copied data; its header's old 'no copy' comment was
incorrect. Language bytes getters additionally make their ordinary value copy.
No per-method language forwarding implementation was added.

CPU tests cover NUL, empty bytes, raw/owned/XPack/mini-program self-removal, expiry
at the language boundary, failed Hold/Drop, first-error handling, ambient-error
preservation, OOM prefixes, overall live bytes and invalid/double frees. Four
concurrent readers race immutable raw providers against publication/clear; user
mutable callback state is not thereby made thread-safe. Test input and binary
cohorts are pinned separately; this is not a full XGE ecosystem completion claim.

Validation: final native cohort is native-source-r10 with native-host-win-r10
and native-host-linux-r10 under F:/xlang-validation/xge-full-20261110/.
native-resource-win-r2/results.json and native-resource-linux-r2/results.json
each passed 24 build/run checks, including real bounded OpenGL regressions.
Linux uses Xvfb, not a physical-device performance qualification.
