# Workflow canvas shared-reference handoff

The approved shared model protocol now extends to Workflow canvas slots.
`xuiWorkflowWidgetGetWorkflowOwned` obtains an independent reference and promotes
a native owning unique slot once. It refuses to promote a borrowed unique slot.
`xuiWorkflowWidgetSetWorkflowShared` retains its own actual reference; sharing
the model between canvases does not consume the caller's reference. Closing
canvases and their Context leaves external Workflow/Graph views usable.

Each owning canvas slot traces exactly one Workflow edge. The internal Graph
canvas borrows that Workflow's Graph and does not trace a second owning edge.
An old reference that cleanup did not consume is retained in one extra pointer
slot, remains traced, and is retried before a further handoff or physical
destruction. Destruction preparation stops the internal Graph canvas borrowing
its model before checked release of the current Workflow slot. An unconsumed
current reference also prevents physical destruction and is retried safely.
After logical retirement clears public part slots, preparation locates the
still-live canvas in its native owner's part list before detaching the borrow.

The internal Graph setter reports actual publication separately from its
status. If a later invalidate fails or requests logical retirement, the outer
Workflow slot still takes ownership of the corresponding new model reference.
The earlier error wins over subsequent cleanup failures. A reported setter
error can therefore occur after a new shared model was installed. It never
means the caller's own reference was consumed.

Creator/quiescence checks precede slot changes. Model cleanup reentry into the
same canvas reports Busy. A consumed cleanup error is reported once and its
reference is not retried. If cleanup did not consume a reference, both the old
retired slot and the new current slot remain valid until retry succeeds.
The original unique/borrowed C APIs remain available.

`tools/test/check_workflow_shared_canvas.py` qualifies production source units
against the exact previously built native static SDK and the complete O2
compiler's one XRT4641 implementation. The proxy is an earlier frozen CPU
fixture; concurrent text/geometry changes are excluded. Windows and Linux each
passed 23 build/run/header commands, with C11 O0/O2 cases for:

- two-canvas references, independent Graph survival and exact owning edges;
- unique promotion OOM, unchanged outputs and 32 handoff allocation prefixes;
- borrowed unique refusal and independent references from borrowed shared slots;
- actual model Drop failures and widget retirement during those callbacks;
- linker-injected unconsumed retired/current releases, retained edges,
  subsequent handoff, checked Context close refusal with consumed=false, and
  successful retry after the internal Graph canvas has stopped borrowing;
- linker-injected post-publication invalidate failure, preservation of its
  primary error over a later real Drop failure, and widget retirement between
  Graph publication and Workflow publication;
- original Workflow widget/interaction and Graph widget/shared scenarios;
- C11/C++11/C++17 public header compilation.

The two explicitly injected statuses exercise native checked-result contracts;
they are not claimed as real scheduler/OS failures. Ordinary ownership,
consuming callbacks, allocation failures and legacy scenarios use actual
production model code. Warm 1000 get/release and idempotent-set iterations
observe zero XRT allocation/free/reallocation calls. Each canvas has one extra
pointer slot; native reference, quiescence and call-scope bookkeeping still
cost CPU. Raw-C CPU equivalence is not claimed.

Final reports are outside Git:

- `F:/xlang-validation/xge-full-20261110/workflow-shared-canvas-win-r7/results.json`
- `/var/tmp/xlang-validation/xge-full-20261110/workflow-shared-canvas-linux-r6/results.json`

The original Graph canvas gate independently passed eight commands per platform
using the same matched XRT/static SDK and replacement production units:
`workflow-canvas-graph-regression-{win,linux}-r3/results.json` under the same
platform validation bases. Total: 62 successful commands.

This is CPU native qualification, not a rebuilt SDK or installed runtime/IDE.
Workflow language declarations, owned schema callbacks, ordinary schema/default
reentry and associated-Graph mutation policy, persistent module cycles and final
matched delivery remain separate. No graphics-device, network or security test
ran. Earlier foundation reports retain their original scope and bytes.
