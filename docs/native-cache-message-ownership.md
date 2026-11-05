# Native MessageList owning caches and callbacks

The native MessageList owns its node directory, each node's unique layouts and
Document binding, and the actual resources of their active/draft renderers.
No new language wrapper, operation-code dispatcher, or hidden retry queue is
introduced. The GUI creator-thread and outside-Ownership-Freeze cleanup
contracts still apply.

## Line caret geometry

The existing display-line shaping and linear display/source mapping algorithm
is preserved. Reusing whole-paragraph clusters is not equivalent: a line break
or inserted SHY hyphen can change contextual shaping. The cold caret path still
shapes precisely the displayed line and keeps the same ligature/grapheme and
zero-width-source rules.

The one line-shape scratch record now belongs to the List, rather than a C
stack variable. It is included in Trace/Ready/Prepare. A successful shape that
is currently busy, or a partial shape followed by allocation failure, keeps
its real carrier there. A later producer first retires that same owner. The
existing per-node caret array is published only after its complete mapping is
valid; no additional per-node shape allocation is added.

## Document candidates and retirement

Each Node has at most one active and one work Binding. Each Binding has at most
one active and one work Renderer. A candidate is put into its actual slot
before native creation, snapshot assignment or layout can invoke a backend.
Failure never discards a non-NULL partially owned output.

Font refresh builds the candidate while the old renderer is still owned. If
retiring the old renderer waits, the ready candidate stays in the work slot
and owns its own cloned Font/callback references. A later call resumes that
transition. An unready failed candidate must be retired before another one is
built. Raw descriptor copies are not used to resurrect a terminal renderer
whose Font/callback owners may already have been consumed.

Bulk clear/import/replacement prepares the old directory before structurally
removing it. It checks and consumes actual Layout/Renderer owners while that
directory still exists. A wait preserves the Binding and Document model in a
retiring state; renderer-dependent operations return checked BUSY. Retrying
clear, replacing the binding, or closing the Context can finish retirement.
After an irreversible cleanup failure, an old render cache may be unavailable;
there is no promise that already-consumed geometry can be rolled back. The
model reference and native cleanup carrier remain owned until completion.

Only after all such resources are consumed does structural destruction
unsubscribe models and free node/binding storage. Consuming negative cleanup
results, including a NULL diagnostic, are recorded through the Context's real
cleanup receipt. Ordinary negative operation results stay local to the call.

## Reentry and callback lifetime

Mutators and geometry producers keep a native Context activity scope to their
physical return tail and reserve the same receiver during provider calls.
Other Widgets remain independent. Geometry publication checks the original
change counter; a model edit during shaping cannot overwrite a later dirty
state with old geometry. The model notification keeps its own physical tail,
preserves pure selection-position mapping, and defers renderer changes if a
consumer/mutator is already active. Synchronization compares the actual held
snapshot revision with the current model revision before clearing pending work.

`xuiMessageListSetNodeRendererOwned` uses the common typed owned-callback
publisher. The callback's Node and Draw inputs are invocation-period views.
Result 0 uses the default renderer, a positive result means handled, and a
negative result is checked failure. A snapshot holds userdata/code ownership
through the complete thunk return and Drop tail, including self-clear or
replacement. A positive handled result is normalized to success for cleanup;
it is not passed off as an error status. The old native setter delegates to
the owned publisher when replacing an owned registration.

## Costs and evidence

Measured on the matched x64 GCC build:

| Native storage | Old | New | Change |
| --- | ---: | ---: | ---: |
| List TypeData | 960 B | 1,072 B | +112 B |
| Node | 304 B | 312 B | +8 B |
| Document Binding | 424 B | 440 B | +16 B |

The List increase contains the actual 96-byte scratch shape and scope state;
there is no extra allocation for that scratch directory. The Node/Binding
pointers register the same native candidate allocations that were previously
stack-local. One work candidate is retained when cleanup cannot yet consume
it. Native activity scopes, same-receiver guards, publication checks, graph
visits, and callback Hold/Drop are real bookkeeping costs, not zero-cost raw
pointer operations. Ordinary hot drawing does not enumerate the owning graph.

The CPU-only tests compile the real production MessageList TU at O0/O2 and
link the full pinned production object graph; the old MessageList object is
omitted to avoid two implementations. They exercise scratch BUSY/OOM, owned
callback self-clear, ordinary versus cleanup NULL failures, full active/draft
Document transitions, last-owner retirement, font-refresh continuation and
provider-triggered model edits. Original offscreen MessageList interactions
also pass.

The preserved Display audit keeps all original literal Unicode, display/source,
ligature, SHY, clipboard, failure and work-count assertions. Its cold work for
786,432 source bytes is still 1,114,112 counted operations; 32 warm hits do zero
shape/measure/display calls and at most 592 binary comparisons. Deep hits in
32,769-line input decode three scalars and shape two display bytes; painting
visits 24 visible lines. This is an algorithm/work-count check, not a language
versus handwritten-C latency benchmark.

Candidate `native-message-cache-overlay-r16` and Windows
`native-message-cache-focused-win-r16` / Linux
`native-message-cache-focused-linux-r4` record 12 build/run rows per platform,
all at O0/O2. The isolated audit copy only corrects its old relative source
include paths and renames its harness main; its assertions/business bodies
are unchanged. These are incremental native CPU qualifications over the old
pinned WidgetCache r2 receipts. The new shared ambient-error receipt layer,
final engine DLL/ABI, safe language declarations and remaining families require
their own matched fresh integration; this document does not claim those passed.
