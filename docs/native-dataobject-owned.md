# Owned DataObject native graph and bounded output

Both families provide direct Native CreateOwned/ShareOwned factories. They use
the default XRT allocation path and actual atomic reference counts. Raw origins
are never promoted. Existing AddRef/Release on an Owned handle enter the same
Mutation/claim lifecycle; raw SetProvider is rejected on an Owned object.

An object owns each stable generation once. Owned provider generations own a
typed read/user/owner triple, using the existing owner Hold/Drop protocol. They
have no second language cleanup callback: final capture/userdata cleanup is
the owner's checked Drop. Raw C read/free hooks remain caller-managed interop.
An invocation pins its actual object and generation and independently Holds
the callback owner through the complete typed return. All owner hooks run
outside publisher locks and outside ownership Freeze.

Hold's bool is the acquisition fact. A fresh diagnostic after true rejects
publication and Drops the acquired candidate once. False acquires nothing and
is never followed by Drop. Replacement is published before old cleanup, so a
cleanup failure does not roll it back or overwrite reentrant peer publication.
Operations preserve the first failure independently of its nullable diagnostic.

The native ownership domain exposes the object and each generation as distinct
real nodes. Count includes real user, invocation and collector references.
Trace shows actual slot and capture owners, not a flattened or fake-count
shared node. Frozen metadata can be inspected without executing user hooks;
business operations and semantic cleanup are creator-affine. Claim rejects
new owners/publication but permits existing-owner retirement. Finalize, Clear,
Finish and final physical Drop have separate responsibilities. The published
48-byte FinishV2 DropReady gate covers creator affinity; finished empty headers
can be physically released on a worker. V1/typed-finalization descriptors refer
to the identical exact adapter, and unknown Ops are rejected.

DataOutput is a 32-byte callback-period view of the actual consumer destination.
Its format parameter is a const xstrview over the stable generation's cached
format length. No string scan or temporary Buffer is required per callback.
Set/Write validate null/range/overflow. A command that does not fit records the
required size and copies no bytes. A successful callback is followed by Get's
capacity check, producing BUFFER_TOO_SMALL. Probe(NULL) records required size
without copying. A callback's nonzero result, including +1, is a negative
CALLBACK_FAILED and has priority over capacity; +1 is never lifecycle DEFERRED.

Earlier successful Write commands remain delivered if a later command makes
the whole Get too small. A single nonfitting Set copies nothing; it does not
undo previous writes. Explicit caller buffers retain that delivered prefix.
Any return-bytes convenience must discard an unsuccessful output. Sparse
successful writes zero-fill their gaps, so successful output never exposes
uninitialized bytes. memmove supports overlapping source/destination. These
copies and gap initialization are genuine Native work, not bridge wrappers.

Private lifetime metadata uses the same allocation's tail. On x64 the initial
owned layer has a 32-byte raw object plus a 32-byte owned lifetime; a 56-byte
raw generation adds a 32-byte owned lifetime and, only for an Owned provider,
a 32-byte capture owner. A slot is one pointer. Format and optional data follow
in that same allocation. Ordinary bytes objects do not allocate or store an
external code capability. Later clipboard revision metadata is a separate
layer and will report its own size change.

The implementation is split into real production C components that depend on
XRT/public headers rather than Widget/graphics implementation. Existing drag
translation units include them, preserving the normal builder's TU organization.
The cold shared lifetime inl uses exact typed public callbacks; it is not an
operation-code or incompatible function-pointer dispatcher.

Native-only acceptance: Windows and Linux O0/O2, actual four-node cyclic graph
retirement, callback replacement/self-close, false/true Hold crossed with fresh
diagnostics, no erroneous Drop after false, ambient diagnostics, first nullable
cleanup failure under allocation denial, checked consumption, sparse/partial
output, per-command short capacity, overflow, sticky raw rejection, claim,
wrong-thread refusal and finished worker Drop. The original raw-generation
fixture also passes against the same two production components; public headers
pass C11/C++11/C++17. The indexed 22-step receipt is a standalone Native subset,
not a full matching engine/SDK, private-language-policy, resident-COM, clipboard,
or final xlang binding acceptance. Those remain separate integration work.
