# Native cleanup receipts

Context and Cell call scopes keep a status latch separate from their nullable
first diagnostic. A failure with no error object remains a failure, and no
later destructor/Drop may replace that first NULL with its own diagnostic.
Each linked scope retains its own error reference only on first failure.
Reference retention/release does not allocate. Ending clears the token and
restores the exact first error, including NULL; an empty token is a no-op.
The address-stable token remains marked ending across consuming cleanup, so
reentry cannot end the same token twice. Tokens must not be copied or moved.

`xuiDestroyOwnerChecked(context, &consumed)` accepts a logical Context close
exactly once. Acceptance sets consumed even if eventual native cleanup fails.
A still-active enclosing native operation can defer physical cleanup; its
checked scope reports that tail's status. `xuiDestroyChecked` discards only the
consumed output. The legacy void `xuiDestroy` preserves idempotent/no-op behavior
for NULL/already-closing contexts. An internal C-stack receipt also spans
physical teardown that follows an ordinary operation with no external scope.
No TLS stack, allocation, extra language object or retirement queue is added.
All ordinary Context and Cell operations retain the creator-thread contract;
Cell scope Begin/End refuse wrong-thread cleanup without consuming the token.

Costs: each Context/Cell scope grows from 40 to 48 bytes on x64. Cell's ending
flag uses existing padding. Context scope End performs one TLS diagnostic peek;
Cell Begin/End also check creator thread identity. Failure paths retain one
error reference per enclosing scope and release those references on End.
Existing calls remain direct C APIs. Physical Context teardown adds one 48-byte
stack receipt, with no heap allocation and no new per-Context storage.

CPU acceptance uses real custom Font and Proxy owners through full native
Context cleanup. It covers exact NULL/non-NULL primary preservation against a
secondary Proxy failure, nested scope/ref balance, allocation failure injection,
reentrant close/End, deferred native operations, valid legacy void cleanup,
Cell reset/final-pin cleanup and successful/invalid token paths. The public
scope layout changes require a fresh matched Native build and SDK; a patched
object linked to clients compiled with the old 40-byte layout is not evidence.

Ordinary negative operation results belong only to that operation's own scope;
a native callback can handle such a result and continue. They are not sticky
failures in its parent cleanup scopes. Actual consuming cleanup failures are
recorded in all enclosing receipts. Snapshot/formatter EndOutcome accepts and
consumes the detached primary error reference explicitly (including NULL with
a negative status). A subsequent Drop reports its own cleanup status to those
receipts, using the operation's earlier diagnostic as the exact prefix. The
operation still returns its original primary status. No parent/TLS inference
is used to guess whether a NULL primary existed.

Snapshot admission delivery is explicit at every caller: BeginReturned lets a
checked direct consumer propagate the negative status; BeginReported reports
infrastructure failures for void/value/veto and event consumers without an
independent status route. These internal names have no unclassified alias.
A direct API's handled failure does not poison its parent cleanup receipt.
The Grid regression asserts that distinction, including a true rollback Drop
failure; the Context merge getter checks its direct negative result separately
from event dispatch's existing checked receipt contract.
On successful owned SnapshotEnd the common explicit tail takes TLS once,
followed by the consuming Drop's diagnostic check and outcome collection.
This adds one diagnostic read compared with the previous owned tail, with no
heap allocation; the non-owned fast path keeps its existing early return.
