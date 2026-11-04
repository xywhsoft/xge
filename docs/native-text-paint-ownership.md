# Native retained paint ownership

TextShape adds one const PaintOwnershipV1 pointer (+8 bytes relative to the
checked-paint intermediate ABI). The immutable descriptor has exact size/tag,
pure DropReady, frozen TraceRaw and consuming DropChecked. Every status from a
valid DropChecked consumes exactly one admitted owner, including failure with no
diagnostic. No retry is permitted for that consumed reference. Descriptor/code
lifetime must be protected by the paint's real backend/capability owners.

A shape is unique owning storage. A C record copy does not retain its paint:
move the record and clear or stop destroying the source. Borrowed draw views,
Document source_run and shared_seed aliases do not create owning records and
must not be traced as duplicate Font references. The default glyph paint owns
one real Font ref; its descriptor traces exactly that edge. Its immutable
creator thread remains required even after Font semantic finalization, because
glyph-run cleanup is not merely release of a Font header.

FreeChecked validates metadata and DropReady before any field or array is
detached. BUSY preserves the complete original record. After admission it detaches
owned storage, invokes the consuming protocol, releases clusters/carets and
preserves the first diagnostic. Status carries failure independently of NULL
error text. Standalone raw C paintFreeChecked keeps its separate consumed=false
retry contract; this retry hook is not a substitute for consuming ownership in
managed caches. Native managed adoption and multi-paint cache preflight require
separate integration: validate every old paint before irreversible replacement.

DropReady and TraceRaw may run under ownership freeze and run no user code or
allocation. Actual Drop/Free/Finalize/Finish must run outside freeze. This is an
existing C precondition, not a claim that MutationBegin rejects a same-thread
nested freeze. The runtime Plan exits freeze before semantic duties. The native
Context/Widget/cache paths must obey the same rule without a private-field test,
new TLS stack, hidden queue or arbitrary callback under freeze.

This source batch qualifies standalone protocol/default backend, common resource
ABI, font slots and Cell affinity. It does not declare the remaining Layout,
DocumentRenderer or all cache/surface ownership paths completed.
