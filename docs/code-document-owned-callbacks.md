# CodeDocument native owned callbacks

Standalone CodeDocument remains uniquely owned; a CodeEdit's internal document
is owned by that editor. No reference-count or shared-attach layer is added.
`xuiCodeDocumentCallBegin/End` pin the actual Document and its observed Context
across native reads, borrowed-text copies and reentrant owner destruction.
Their address-stable zero-initialized scope does not set the writer flag.
Consuming the owner rejects new calls but delays physical free until all pins
leave. The existing lifetime watch reports actual physical destruction.

`xuiCodeDocumentSetChangeCallbackOwned` holds an explicit callback owner before
publication. A failed hold preserves the old typed function/user/owner slot;
an acquired reference is dropped exactly once even if Hold reports a fresh
error. Publication uses the existing XRT mutation gate. Dispatch snapshots and
holds the selected owner so replacement or clearing during the callback cannot
unload it mid-call. Userdata and ownership data remain separate pointers.
Legacy `SetChangeCallback` uses the same update path with a borrowed owner.

The unique destructor detaches and consumes the installed callback reference,
even if its Drop reports an error. The first status and exact error (including
a null diagnostic) survive later cleanup. Void callback Hold/Drop failures
reach the enclosing checked edit result. The document can already have changed
when a post-publication callback fails; this is not an edit rollback.

`TraceOwnership` enumerates the installed owner on its creator thread while
quiescent; CodeEdit's graph trace includes that edge exactly once. Internal
edit observers remain weak native relationships. `GetTextView` rejects closed
and foreign-thread reads before looking at text storage. File save pins its
complete lifetime without setting the writer flag, preserving autosave from
inside an edit/change callback. Its ordinary path uses the local XRT file API;
it does not install or invoke a virtual filesystem provider.

No callback registry or per-business-method forwarding layer is introduced.
Scope counters/context pins and XRT mutation gates are real bookkeeping costs.
An owned dispatch adds one Hold/Drop pair; replacement adds candidate Hold and
previous Drop. Native storage gains one owner record per Document. Cold text
snapshots and actual edits retain their original algorithmic allocation costs.
The language adapter still copies borrowed text; it cannot promise zero-copy
independent strings.

The focused CPU gate exercises genuine callbacks, trace/replacement/clear,
owner failure and close reentry, counted NUL snapshots, deferred physical free,
warm allocation budgets and the existing observer/history lifetime kernel.
Language bindings, full callback cycles, batch ownership and matched delivery
require separate acceptance.

Windows and Linux acceptance uses matching O2 host/static engines, with native
clients at O0 and O2. The callback gate passes 16 build/run commands across
both link modes/platforms; the static clients additionally execute the original
private observer/history/owner-close kernel. The existing counted-text,
CodeEdit and TextEdit gates pass another 32 commands. The final callback test
is an explicit test-only overlay on the frozen r2 engine source, adding a
foreign-thread rejection test without changing the accepted native binary.

The full r1 SDK compiled 164 host and 166 static units per platform. The r2
autosave correction compiled one host unit (163 reused) and two static units
(164 reused). It is not a second full rebuild. An intermediate writer-scope
implementation's autosave failure remains archived as a development
counterexample; the accepted read scope permits callback autosave and preserves
UTF-8/NUL contents. Exact paths, hashes and reuse are recorded in
`docs/validation/code-document-owned-20261114.json`. These CPU checks do not
claim full ecosystem completion, CPU equivalence or shipping installation.
