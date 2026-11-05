# Native CodeEdit cache ownership

The existing line directory and incremental chunk shaping algorithm remain in
place. The IME prefix measurement now uses a real counted line-layout entry,
instead of producing an owning shape in a stack temporary. Repeated offsets
of the same displayed composition reuse that entry. A successful shape whose
paint cannot yet be cleaned, or partial output followed by allocation failure,
keeps its actual carrier in the directory until checked preparation succeeds.

Each counted entry owns its resolved Font in an Owned Context. That reference
also keeps a font used as a cache key from being freed and reused at the same
address. Trace visits this reference once in addition to the paint's distinct
Font reference. Preparation consumes shapes/chunks before releasing the entry
Font; if an owner was not consumed, its complete record stays registered.

Pure waiting returns `XUI_STATUS_DEFERRED` inside destruction preparation and
is distinct from a consuming cleanup failure. A normal edit reports BUSY before
changing the Document if its affected cache window cannot yet be retired.
Negative cleanup results, including a NULL diagnostic, use the Context cleanup
receipt. Ordinary operation failures stay local to their checked call.

The geometry producer reserves the same receiver while native shaping is in
progress. A nested offset query or Document edit cannot observe a half-built
entry or recursively produce another one. `xuiCodeEditGetOffsetRect` retains
the native Context activity scope through its physical return tail, including
callbacks that request destruction of the CodeEdit. The borrowed internal
Document is protected by its actual owner identity; it cannot be independently
destroyed through the public Document API. Its active edit pin outlives any
pre-edit callback that destroys its owning Widget.

The extra producer flag uses existing pointer-alignment padding. On the tested
x64 ABI, the native CodeEdit data remains 1,608 bytes and each line entry remains
120 bytes. There is no new per-line directory, cache allocation or language
wrapper. There is real bookkeeping: an entry retains/releases its Font, traces
its true owning references, and a geometry call enters a native activity scope.
This is not described as zero bookkeeping.

CPU tests exercise the actual production CodeEdit translation unit and replace
its old linked object with an empty object to avoid duplicate definitions. They
cover successful busy paint, partial OOM, exact Font edge counts, 48 KiB edits
that rebuild at most three chunks, borrowed-Document owner protection, provider
reentry, and destruction at the edit/geometry return tail. The complete existing
offscreen CodeEdit test also runs, including editing, selection, IME, find,
completion, syntax, minimap and touch paths. No engine window, GPU, browser or
network is started. Windows and Linux O0/O2 passed over the fresh Ambient r9
shared native objects; this is native CPU acceptance, not a final shipping DLL
or public package declaration claim.
