# Document HTML interaction ownership

The actual Native panel is a Context-owned composite manager. Create registers
its existing allocation in the Context directory before Document allocation or
Widget callouts. It retains the Document once, creates an ordinary primary
Window, and constructs its WebView with the primary Window native-owner
identity already installed. The internal WebView cannot be independently
publicly destroyed, detached or reparented. Normal public Window destruction
closes the panel through its physical lifetime watch. Pure getters do not
open scopes or drain deferred destruction at their return tail.

ReleaseOwnerChecked / ReleaseChecked request logical close without blocking.
A successful request consumes the caller's panel-use responsibility; the
Context directory retains the physical allocation until all Widget destruction
tails finish. In-flight/queued WebView delegates retain the real WebView data;
its Prepare revokes registrations and waits for those references. Releasing a
delegate only changes its RC. A later checked host boundary performs actual
Context retirement, retaining the module code lease until then. The CPU panel
fixture proves Native carrier retention; actual dynamic-DLL unload is a
separate joint gate, not inferred from the static fixture.

## Link callbacks

The descriptor and SetOpenLink retain the existing raw C char* callback
contract. The host remains responsible for raw userdata/code lifetime.
SetOpenLinkOwned installs a typed callback with signature
`void(const xstrview* url, void* user)` and a real callback owner. Publication
uses the common pre-Hold/reentry check/mutation/publish/old-Drop transaction.
Trace reports the actual callback owning reference, not a synthetic owner.
Dispatch snapshots the exact function, user and owner before Hold can reenter.
The JSON root and optional raw C string copy remain alive through the actual
callback-owner snapshot Drop, then are released before the manager scope ends.
This also permits clearing the registration or closing the panel in callback.

The immutable xstrview pointer and bytes are callback-period inputs. Language
bindings use the existing checked epoch view; Size reads directly, and an
explicit text conversion copies the Data/Size pair. Saving the view does not
extend the source lifetime. Owned dispatch adds no URL malloc/copy/free: the
view points into the retained parsed JSON. Legacy raw dispatch still creates
its required NUL-terminated C copy. No managed-string callback input ABI is
claimed. Generation reads dispatch by the actual JSON integer tag; a failed
UInt probe followed by a successful Int read must not leave a type error that
silently suppresses a legitimate callback.

## Document boundary

Native Document Retain/Release remain genuine Document RC operations. This
panel change does not implement a language owning domain for arbitrary custom
Document allocators or raw Subscribe callbacks. Those remain explicit
caller-managed C interoperability. The safe default Model surface must be
restricted to a verified default allocator and no language raw subscription
entry, with actual RC count/hold/release; no constant-one reference count or
opaque callback child flattening is permitted. The full Document callback /
custom allocator domain is a separately recorded gap. This batch must not be
reported as making all Document APIs safe language objects.

## Cost and validation

On the measured Windows x64 ABI the old panel is 88 bytes and the new panel is
280 bytes (+192): an embedded 72-byte manager directory node, 32-byte callback
owner, two 32-byte physical watches, Context pointer, typed callback and local
reservation state/alignment. It remains one allocation for the panel; no new
RC node, retirement queue or per-operation heap allocation is introduced by
this ownership metadata. Public mutators and callback consumers use the
existing Context/manager activity scopes; pure getters add creator-thread and
logical-close checks. This is real Native safety bookkeeping, not a claim of
zero lifecycle cost. Language declarations call the same Native entry points
directly without a product forwarding wrapper.

Focused acceptance is immutable: Windows
`F:/xlang-validation/foreign-completion-20261103/native-html-interaction-focused-win-r10/results.json`
and Linux
`/home/ubuntu/xlang-validation/native-html-interaction-focused-linux-r1/results.json`.
Each has O0/O2 strict builds and runs over the exact Ambient r9 production
object graph with actual HTML/WebView TUs compiled into the fixture. Matching
historical headers/helpers and the unchanged previously frozen WebView TU are
listed in the overlay manifest, not staged as new production changes. The
current DocUI authority retains all WebProvider and DocImage declarations.
The final unified full build must derive fresh exports/ABI metadata normally.

Cases cover typed view across snapshot Drop, self-clear/self-close, internal
part destroy rejection with continued usability, parent Window destruction,
extra delegate reference delaying Context consumption, Hold reentrant close,
false/NULL Drop failure, ordinary Update error remaining local, 128 XRT OOM
budgets, explicit panel/read/page CRT allocation failures, and close during
LoadHtml with no dangling page buffer. Heap counts and bytes balance. C11,
C++11 and C++17 DocUI declaration gates pass. Browser I/O is test-only CPU
stubs; browser/engine/window initialization, GPU, network and security tests
are not run or claimed.
