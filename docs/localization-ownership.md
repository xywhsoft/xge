# Context-owned localization

Language packs remain owned by their Context registry. Creating or reading a
language view does not give that view a separate native owning reference;
releasing the view must not remove the pack. Ordinary Context destruction
releases all packs. The existing C pointer and mutable-array APIs remain.

`xuiLanguageLifetimeWatch` attaches caller-owned, address-stable observation
storage. Destruction invalidates observers before freeing slot strings or the
pack itself. An owned Context's Proxy creator thread also governs observation.
Observers follow the existing detach-before-notify and normal-return contract;
the language binding's invalidators perform lifecycle bookkeeping only.

`xuiCreateLanguageOwned` supplies checked status/output creation. Code/name are
copied, and an invalid fallback ID follows the legacy English fallback rule.
The factory holds a native call scope and a temporary creation observer over
its checked tail. A non-NULL output is a new registry entry even when that tail
reports failure; a caller that cannot publish it uses `xuiLanguageRollbackCreate`.
Removal is reported independently from checked tail status through `bool*`.
Built-in, active and fallback-referenced packs are rejected without removal.
This is a cold unpublished-factory cleanup protocol, not ordinary pack release.

Failed publication rolls back its registry entry and native allocations; it
does not rewind consumed IDs, revision counters, registry capacity or prior
invalidation. Replacing a text slot instead copies before swapping: allocation
failure preserves the old text and revision. Exhausted custom IDs reject
creation before signed increment; the previous INT_MAX increment was undefined.

`xuiLanguageGetTextCount` and `xuiLanguageGetText` inspect a pack's own slots,
without exposing xarray mutation or applying fallback. IDs are one-based native
text IDs. Unset or invalid IDs return NULL; a stored empty string is present.
`xuiTranslate` remains the fallback-resolving query. Raw text pointers remain
borrowed until mutation or Context release. `xuiLanguageGetContext` returns the
existing registry owner, not a new native reference.

The extra lifetime-list pointer costs one pointer per pack (48 bytes across six
built-in packs on the tested 64-bit targets). Observer attachment allocates
nothing. Owned creation adds native scope/creation-watch bookkeeping and retains
the ordinary string/slot/registry allocation costs. Scalar/slot queries allocate
nothing; caller-owned language string snapshots have separate scan/copy costs.
No universal zero-cost or CPU throughput equivalence is claimed.

The CPU host-SDK clients execute at GCC O0/O2 on Windows/Linux, covering native
fallback, NULL/empty, explicit watched rollback, dependent/active/built-in
rejection, 48 creation allocation budgets, failed text replacement and bounded
ID exhaustion. Public C++11/17 prototypes and the existing 16-step editor/text
gate also pass on both targets. Evidence is indexed in
`docs/validation/localization-20261114.json`. A test-only translation-ID spelling
correction is outside the frozen SDK inputs and is pinned separately. Static
language programs, actual graphical backends and final matched delivery are
downstream integration work, rather than claims of this native prerequisite.
