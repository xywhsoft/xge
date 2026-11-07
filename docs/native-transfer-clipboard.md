# Owned data and external transfer

The safe Native factories are `xgeDataObjectCreateOwned` and
`xuiDataObjectCreateOwned`. Raw factories/providers retain their explicit C
caller-managed contract. An Owned object cannot accept a raw provider. A
provider owns one `read/user/owner` triple; owner Drop consumes captures and
userdata once on every checked outcome. There is no second cleanup closure.

The immutable generation model and real RC/Trace/Prepare/Finalize/Finish
protocol remain the ones documented in `native-dataobject-owned.md`.
Whole-clipboard copies add a nonwrapping 64-bit publication revision: +8 bytes
per data object, without another allocation. Successful slot publication
increments it. Plain Get adds no scan or reference bookkeeping for byte data.

`xgeDataObjectCopyToClipboardWithTransfer` and
`xuiClipboardCopyDataWithTransfer` pin every original generation before any
provider. Plain bytes are borrowed until the backend returns; they are not
copied through an intermediate DataObject or Buffer. Each lazy format is
queried, allocated once, then filled through the same pinned generation.
Changed result size, changed source revision, error, or allocation failure
prevents the backend call. The backend receives the complete set once.

For explicit output buffers, an individually short DataOutput Set/Write copies
no bytes and records required size. Earlier successful Writes remain visible
if a later Write is short. Probe copies nothing. Callback failure takes
precedence over capacity. A safe language bytes result does not publish a
partially filled buffer.

## Code capability at a real external boundary

`include/xnative_transfer.h` is the canonical portable ABI. Services are
borrowed synchronously. A successful typed Wrap consumes exactly one unique
Native COM implementation duty and retains an independent code capability.
Failure consumes neither. Explicit process-resident providers may declare that
fact; NULL never silently means that a dynamic module is protected.

Normal byte DataObjects do not allocate or retain code capabilities. Only
Run/Drag/clipboard external escape uses the services protocol. The synchronous
Run keeps its caller's borrowed services in a creator-thread stack scope. A
short lock protects publishing/copying that scope, never an allocator, OS call,
callback or code retain. A different thread cannot borrow it. COM objects use
the full resident IDataObject/IDropSource/IDropTarget capsules, including method
entry pins and checked creator-thread retirement, rather than a Release-only
proxy. Native descriptors expose exact owned child edges and checked consumed
status. Existing resident queues carry deferred cleanup; no Native queue is
introduced here.

Protocol v1 requires all returned child COM interfaces and medium cleanup code
to be OS-owned or explicitly process-resident. XGE returns HGLOBAL media with
`pUnkForRelease=NULL`, uses the OS format enumerator, and does not implement
advisory interfaces. Arbitrary third-party DLL child interfaces need their own
carrier and are outside this version.

## Windows publication and drag

Windows prepares every requested HGLOBAL representation before one
`OleSetClipboard` call. PNG retains its lossless format and, when decoding
succeeds, additionally supplies the existing alpha DIBV5 compatibility form.
GetData duplicates independent HGLOBAL memory. No provider executes after a
prepared clipboard publication.

Empty binary data uses a valid zero-size movable HGLOBAL. The standard OLE
duplicate helper rejects that handle, so this one case creates another exact
zero-size handle directly; the native reader returns length zero without
locking it. Nonempty data retains OleDuplicateData. GetData performs one
additional GlobalSize query to distinguish this case.

A cold CLI invocation initializes a temporary OLE apartment (S_OK and S_FALSE
both require Uninitialize), publishes, then Flushes before leaving it. Changed
apartment mode is rejected before publication. An existing Run apartment is
reused. Preparation/wrapping failure makes zero Set calls. Once Set succeeds,
a Flush/OS failure cannot promise rollback: the API reports the failure while
the complete new snapshot may already be the clipboard. OS-held references
still own their actual Native carrier and resident code capability.

Drag remains lazy. A private +8-byte pointer in the Native IDataObject borrows
an outcome on the DoDragDrop caller's C stack only during that invocation.
Creator thread is checked before reading or writing it; it is cleared before
that stack returns on every normal error/cancel/success path. Actual provider
failure is latched separately from ordinary FORMATETC discovery. This preserves
the first status and nullable diagnostic when the OS reports only cancellation.
Later COM requests use their resident checked boundary and never a saved stack
pointer. Native C callbacks must not unwind across the C API.

## X11 and other platforms

X11 publishes one immutable, copied format snapshot and responds with all
TARGETS, exact custom atom bytes, and the UTF8_STRING alias for UTF-8 text.
Binary NUL is retained. Requests with property None use the requested target.
The existing one-request/no-INCR limit is checked before publication.

The SelectionRequest responder runs within the synchronous Run event loop;
cleanup clears its snapshot before closing the Display and before Run returns.
The Run caller's code capability covers this entire interval. Data already
copied into an OS property contains no code pointer. Other text-only backends
reject a multi-format request before any write instead of silently discarding
the remaining formats.

Proxy ABI 18 appends one optional typed `clipboardSetItemsWithTransfer`
function pointer (+8 bytes/table). Earlier field offsets remain unchanged;
embedded Context/backend records are rebuilt with the new exact table size.
The later optional `fontLoadSystem` pointer is appended after that prefix.
Legacy raw Proxy tables ending at the earlier prefix remain accepted and the
missing callback reads as NULL; Owned Proxy objects still require the current
complete table because their capability record is immutable.
The XUI-to-XGE cold item projection uses a bounded typed stack array and no
temporary heap adapter. Owned Context drag sessions admit Owned data and trace
the actual session reference. The Native XGE-to-XUI provider bridge retains the
real source through the existing checked lifetime protocol.

The internal UI bridge uses the checked callback variant so a negative status
with no diagnostic remains distinct from a successful no-effect result. Its
complete native return tail is protected by a Context scope. The default OS
dispatch thread is recorded in one atomic global; input admission rejects a
different thread before installing the non-owning Context back-link. An Owned
Context can measure/shape and perform internal drag operations before a window
exists, but native input pumping requires its OS UI thread to be ready. Legacy
raw manual/offscreen registration retains its explicit same-thread C lifetime
contract and is not a thread-safe weak reference facility.

Transfer event payloads are typed with the inline `xuiEventDragData` accessor:
only ENTER/OVER/LEAVE/DROP carry `xui_drag_event_data_t`. Both sized-record
guards must pass. Geometric BEGIN/MOVE/END/CANCEL are excluded before payload
dereference. The resulting view lasts for the callback; a real owned data share
is required to retain its DataObject afterward.

## Acceptance boundaries

`check_data_transfer_cpu.py` compiles the production consumers with typed fake
OLE/Xlib boundaries. It does not start a window, renderer, browser, network, or
modify the actual clipboard. Snapshot, reentry, OOM, capacity, PNG/DIBV5,
publication/cancellation and RC checks are separate from matching SDK, resident
capsule DLL-tail, and language acceptance. Their reports must name the exact
source snapshot and runtime; component acceptance alone is not whole-package
completion.
