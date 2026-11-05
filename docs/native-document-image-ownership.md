# Native Document image resources and requests

Owned Context image functions use the Context's canonical Proxy. A valid
owning Surface output belongs to that capability and its creator thread, and
semantic release runs outside Ownership Freeze. Kind3 Surface release consumes
that valid owner even if other references or pins keep storage alive. Its
checked cleanup result is independent from consumption. This does not promise
recovery of an arbitrary invalid C backend's output.

The registry retains its own Surface owner. The image producer consumes its
original owner once on success or rollback, including a non-NULL failed output.
Invalid private-register inputs retain their original owner. Private register
requires an existing protecting native Context activity scope. Actual producer
rollback uses an explicit first-error prefix: a primary status with NULL
diagnostic cannot be replaced by a subsequent finalizer error. A sole cleanup
failure still reports its own status and nullable diagnostic.

PNG export holds a real Surface reference before calling any backend function.
A backend callback may remove the registry entry while reading pixels; the
temporary owner protects the actual payload until the native call returns.
All output buffers are discarded on failure. File loading holds its Context
through file/backend callouts, and physical watches prevent returning a
borrowed resource after a callback's legacy Context destruction at the tail.
The explicit raw C proxy path keeps its caller-managed transfer convention.

## Async request lifetime

Each request embeds one real Context-owned manager node in its existing
allocation. Context ownership was already a prerequisite of the old API,
which required release before Context destruction. Request methods run on the
Context creator. Worker input is copied/immutable; only cancellation is atomic
and completion data is read after the native thread's synchronized return.

Release now requests logical close and cancellation without joining a running
decoder on the UI thread. The Context retains the actual request and its code
carrier. True Prepare first cancels, then uses thread state and a zero-timeout
wait to confirm that the native procedure returned. Only then can it consume
worker/image/input storage. The worker never destroys its own request or
unloads its module from its return frame. A later checked UI/host boundary
finishes retirement. Lifetime watches invalidate at accepted logical close.

The global request list is only a non-owning scheduling/admission index. A short
C11 lock protects lookup/link/unlink across independent creator Contexts; no
allocation, thread API, Surface operation or other callout runs under it.
Selected jobs have creator Context and actual manager scopes through thread
publication/finishing. Poll snapshots at most four worker candidates and
reselects from the index after callouts; it never follows a captured raw next
pointer across callbacks that can close a peer. Same-Context recursive Poll is
BUSY, while independent Contexts may progress concurrently. The existing four
workers, 64 outstanding requests and 64 MiB copied-input limits remain.

Partial upload output belongs to an actual request Surface slot until consumed.
Its edge participates in tracing/preparation. Ordinary decode/upload errors,
including their owned diagnostic, are stored in the affected ticket. Reaping
a failed peer does not fail an unrelated successful Poll. Real consuming
cleanup failure still reaches the Context receipt and preserves its primary
NULL/error prefix. Stats/context getters do not dispatch or drain a Context.

## Costs and validation

On the measured x64 ABI a request grows from 152 to 232 bytes, integrating its
manager node, retained error and partial-output carrier into the same one
allocation. Poll/launch add real native scope and short-lock bookkeeping;
workers and copied input are not newly introduced. PNG export adds a temporary
Surface reference and native scope. No language wrapper, second owning queue
or per-Poll heap object is added, and this is not called zero bookkeeping.

Windows/Linux O0/O2 CPU fixtures compile the actual two production translation
units and omit their old objects. They cover real Surface graph counts,
registry deletion during read, partial output, NULL first error versus sole
cleanup, invalid-input nonconsumption, reentrant Context close, worker cancel,
blocked-decoder Context retirement, queued cancellation, peer close/Poll
reentry, peer failures with and without diagnostics, 24 construction OOM
budgets, and two independent creator Contexts performing 20 rounds each.
Debug allocation counts/bytes and the global index return to their baselines.

The regression fixture invokes only the existing renderer test's synchronous
and asynchronous image groups; their assertions remain unchanged. Its test-only
release adapter waits at checked UI boundaries to match those old assertions'
blocking-release timing. The separate owning fixture proves the new nonblocking
contract. The original renderer GPU main is never invoked. Tests use local
repository images and memory/CPU codecs, with no engine, device, browser,
network or security probe. These are native CPU results, not final public
package or dynamically unloaded DLL acceptance.
