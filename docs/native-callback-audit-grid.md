# Native Grid Owned callbacks and owning editor configuration

This cohort adds real typed Owned Grid count/cell/set, validation, editor and
editor-configuration publishers. Each callback has an independent userdata and
Hold/Drop/Trace owner. A three-callback adapter replacement publishes atomically;
post-publication refresh failure does not resurrect the previous callback slots.
The inner TableView is an internal borrowed part under the native owner guard.

ConfigOutput is a genuine native owning output record. Enum labels, palette and
four text fields are copied by native setters, remain valid through actual editor
consumption, and are released by Dispose. Text is C-string UI text: embedded NUL
is checked and rejected. Init is only for fresh C storage, never a managed reset.
Owned Editor receives a const callback-period ConfigOutput view and const Cell;
raw pUser and raw cell pValue are not language values. Raw Config + Owned Editor
is rejected before Hold or publication in either registration direction. A pure
native preflight repeats that check after a reentrant Hold, without a second
publication window under the creator-thread UI contract.

Return contracts: Owned Config requires exactly zero for success; a positive
result is invalid and a negative result is checked failure. Owned Editor returns
positive for handled, zero for native fallback, negative for checked failure.
Validation rejects negative results as errors. Count/cell failures are retained
by the context checked call scope. Editor callbacks cover their physical tail;
formatter text is freed before its owner snapshot is dropped.

The Cell formatter lease is shared by Table and Grid and spans real formatting
and rendering. Cached Cell.Use queries perform no text allocation or copy; a
1,000-query native test asserts zero allocations, reallocations and frees.
Projection copies native scalars and borrowed display pointers only, with raw
value/callback fields zeroed. BOOL editing releases the read pin before writeback.

Owned SnapshotEnd, UpdateEnd and group cleanup preserve the first explicit
failure (including NULL diagnostic). They additionally peek once at existing TLS
diagnostic before arbitrary Drop, so a void callback's primary diagnostic cannot
be replaced by a secondary cleanup failure. This is one actual TLS read on a
healthy Owned endpoint; raw/no-owner methods retain their direct path. Void C
ABIs cannot report false-with-NULL by themselves: language owner/checked scope
or mailbox must carry that existing failed bit. No new TLS stack is introduced.

Acceptance is CPU/headless native API validation, not shipping-language or GPU
certification. Windows and Linux clients at O0 and O2 link one matched O2 engine
per platform. Twelve families, 128 build/run rows per platform (256 total), pass:
Grid + original Grid; internal parts + 15 original suites; remaining publishers;
Timeline groups; Tree + original; Table + original; Cell; Context; date port;
cursor readiness; global Frame; global Error/Quit/Drag. All pinned inputs remain
unchanged. The aggregate evidence and byte manifest are outside delivery runtime:
F:/xlang-validation/foreign-completion-20261103/native-callback-grid-cohort-r1.json
and native-callback-grid-inputs-r1/manifest.json. Later Proxy v17 / FinishV2 work
is deliberately absent from this frozen v16 cohort.
