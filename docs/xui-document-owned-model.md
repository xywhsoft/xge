# Default-allocator Document models

`xuiDocumentCreateOwned` is the safe model origin. It accepts only the native
default allocator: custom allocation/free functions and allocator userdata are
rejected before allocation. Owned snapshots, changes, preparations, fragments
and source indexes inherit verified provenance. Owned import/open factories
create that same origin directly, rather than promoting an existing raw model.

Legacy `Create`/import APIs remain explicit C interoperability. A default
allocator chosen through those legacy APIs does not silently become managed.
An unsafe model's `Ownership` returns an empty reference; its `AddRef`,
`ShareOwned` and checked owned-release APIs reject it. Legacy Retain/Release
given an actually owned model enter its real ownership protocol.

## Seven actual lifetimes

Document, Snapshot, ChangeSet, Prepare, Fragment and SourceIndex are shared
native RC objects. `ShareOwned(source, &output)` initializes output to NULL,
checks exact kind/provenance/retirement, and acquires exactly one new owner on
success. It uses the same private checked acquisition primitive as AddRef.
No public AddRef forwarding function or additional control allocation is used.

Transaction is unique. Commit/Abort close its writer but retain the transaction
handle and its owner; they are ordinary methods. Only checked release consumes
that unique owner. A collector snapshot pin preserves its physical header,
not its writer or payload: releasing the owner immediately performs Abort and
business retirement, including after Claim. A real active call instead defers
without consumption. Finished pinned headers still have an exact Count and
empty Trace, and Finalize/Finish remain idempotent. Transaction has no public
Share/AddRef API.

All seven use their actual counters. Their `xrtownershipref.Data` is the
embedded lifetime metadata, not an invented leaf or a fabricated count of one.
Trace lists actual owning edges:

* Document: pending Prepare, each owned observer capture, and a dynamic latched
  error. Static XRT errors have no counted ownership edge.
* Prepare: its visual ChangeSet and publication-plan ChangeSet.
* Fragment and SourceIndex: their one owned Snapshot each.
* Transaction: its owned Document while the payload is live.
* Snapshot and ChangeSet: no foreign-language children; their retained native
  state graphs are immutable C data, not hidden callbacks.

The corresponding UI owners list these model references rather than treating
them as leaves: View/Editor, renderer, MessageList active/retiring bindings,
and HTML interaction panels. Native UI subscription backlinks do not own the
UI object. They are explicitly tagged, cancelled before physical UI teardown,
and remain cancellable after Claim without acquiring a new model owner.

## Threads, phases and errors

Document mutation/notification and Transaction business operations belong to
their creator thread. Immutable models can share/read/release across threads.
Prepare retains its actual self-reference during Run and uses atomic state and
cancellation. Existing strong releases are allowed after Claim. A last semantic
release on the wrong creator returns negative `XUI_ERROR_BUSY`, unconsumed.
Only an incomplete operation with no checked failure returns positive
`XUI_STATUS_DEFERRED`.

All graph edge changes participate in XRT Mutation. Short per-model locks cover
RC/claim admission; no observer or allocator callback runs under those locks.
Semantic Finalize/Finish run outside Freeze. Their phase flags are published
under Mutation before payload changes, so frozen reads cannot enter a partial
semantic tail. Count/Trace metadata stays identifiable after retirement.

Claimed but intact payloads may be read; no new owner, subscription, snapshot or
transaction can be created from a claimed participant. Semantic Finalize/Clear
ends business-read admission. Raw C callers must not read retired payloads.
Cancellation/requestStop used by cleanup remains available; it does not
temporarily remove a claim or install new work.

`ReleaseOwnerChecked(handle, &consumed)` reports status separately from owner
consumption. Negative failure with `consumed=true` must never be retried as a
second release. V2 completion provides pure DropReady and checked Drop; the
independent FinalizationV1 contract preserves incomplete semantic cleanup.
Consumers must query that capability instead of reducing it to the legacy
boolean Finalize slot.

## Owned change notifications

`SubscribeOwned` accepts an int-returning typed callback, independent userdata
and a valid owning callback record. Zero succeeds; every other integer is a
failure. Native View/Message observers also return the actual scope-tail
status. The public legacy raw subscription keeps its void signature and is
refused on managed Documents.

Notifications run after content is committed. Commit does not claim rollback
when notification fails. The Document latches the first nullable diagnostic,
raw callback status, failure count and last committed failure revision.
`GetNotificationFailureStatus` returns the original integer, including +1.
`CheckNotifications(clear)` and unobserved failure during checked retirement
return negative `XUI_ERROR_CALLBACK_FAILED`; raw +1 can never mean DEFERRED.
Read the raw status before clearing if it is needed independently of a
diagnostic. Clearing retains its own diagnostic before detaching that exact
batch; reentrant new failures are not erased by the old batch.

Notification holds the actual Document, ChangeSet and callback snapshot through
the final owner Drop. Self-unsubscribe, replacement and parent close cannot
invalidate callback inputs prematurely. The first failure survives secondary
cleanup, including a first failure with no allocated diagnostic.

The observer stores a typed raw-void/checked-int union. Native/raw observers
remain the original 40 bytes on x64. Only an Owned observer has a 32-byte owner
tail in the same allocation, for 72 bytes total. There is no callback erasure,
opcode dispatcher, second allocation, or unsafe ownerless language callback.

## Editor worker retirement

UI input/retired lists and the worker's transferred garbage/retired-active
records trace exactly their actual Prepare owners. Queued and ordinary active
pointers borrow those same records and are not counted a second time. Transfer
uses Mutation then the short mutex; CondWait and worker completion waits hold
neither a graph scope nor that mutex.

Checked Context destruction cancels and requests stop, returns DEFERRED while
the actual worker/XRT thread context is still running, and retains the full
Context/Widget/module carrier. A later checked boundary retries after real
proc completion. The worker does not free its parent or unload code from its
last frame. C void Context/Widget destruction retains its synchronous worker
completion contract. An explicit private Context-local destroy-mode stack
selects those paths; nested checked entry points override and restore that
mode. No flags are guessed, no xlang TLS is consulted, and no new queue exists.

## Measured cost and scope

Windows/Linux x64 compiler layout measurements agree:

| Native allocation | Before | Owned-model implementation |
| --- | ---: | ---: |
| Document | 168 | 232 |
| Snapshot | 48 | 80 |
| ChangeSet | 120 | 152 |
| Transaction | 136 | 176 |
| Prepare, including embedded builder | 384 | 464 |
| Fragment | 24 | 56 |
| SourceIndex | 152 | 184 |
| Default allocator control | 2296 | 2304 |
| Raw/native observer | 40 | 40 |
| Owned observer | — | 72 |
| Context | 143424 | 143432 |
| Widget / Type | 2040 / 408 | unchanged |

The shared lifetime metadata is 32 bytes. There is no separate model control
allocation or extra content copy. Existing doc_alloc controls retain the
allocator until their final physical pin is released; Transaction stack
builders also carry the larger native transaction layout. RC, graph admission,
phase checks and notification snapshot bookkeeping are real costs, including
for handwritten C using these same owned APIs. No claim of zero bookkeeping
or unchanged throughput is made.

Document output buffers are allocated in the native libc/default-allocator
domain and are released with `xuiDocumentFreeBuffer`. They must not be adopted
as xrtMalloc memory or freed with xrtFree. Borrowed slices require their model
owner; language text/array projection must preserve lengths and ownership.

This phase covers the verified default model origin, typed OwnedSubscribe and
its actual native/UI lifetime edges. Arbitrary allocator code/owners, public
raw subscription callbacks, raw activation descriptors and DataObject do not
become safe by a cast or an OOP name. Their separate owned protocols are not
claimed complete here. Public/private language binding and actual language
collector/module-lease acceptance are separate from these native CPU results.
