# Workflow native shared ownership foundation

This change implements the approved real shared-reference protocol for a
Workflow model and the independently shared Graph it owns. It does not promote
the pending xlang model candidate or settle schema/default callback policies.

`xuiWorkflowCreateOwned` creates a creator-thread model.
`xuiWorkflowShareOwned` obtains one actual reference without copying tables,
schemas, defaults or graph contents. `xuiWorkflowGetGraphOwned` obtains an
independent Graph reference. Closing the Workflow releases its Graph reference;
an external Graph reference remains usable. Loading replaces graph contents
while retaining both native identities. Legacy unique C construction remains.

Every existing int operation has an activity scope, with nested internal calls
sharing the outer scope. Another thread is rejected before reading mutable
model state. Count results retain zero, and find results retain -1. The model
exposes the existing native ownership graph, preparation, quiescence,
finalization and checked completion protocols. Its graph is one actual owning
edge, while schemas and variable defaults each contribute their own edges.

Checked release separates consumption from errors. A consumed reference must
not be retried even if a user Drop failed. Finalization rejects model reads,
retains the first cleanup error, and keeps the metadata address until the last
view/collector pin is released. A failed child release leaves the tables intact
for retry. It does not decide ordinary schema/default Clone/Drop reentry during
live model operations; that separate policy remains pending.

The source-unit gate `tools/test/check_workflow_shared.py` uses the complete
qualified compiler's one XRT, with SHA-256
`369f884c1c1fdc9621f654c81bb86850e85d992dc56b9cebf395e20ea03dfe7b`
(XRT `4641cd2e`). The baseline native sources are frozen, and unrelated working
tree geometry changes are excluded. Windows and Linux run C11 O0/O2 cases for:

- real aliases, independent close, Graph survival and stable identities on load;
- all 31 existing-Workflow int entries rejecting another thread while the
  creator mutates its model;
- creator-affine collector preparation/finalization, outstanding view pins and
  physical tail release from another thread after completion;
- first-error/consumed cleanup, four constructor allocation failure prefixes,
  allocation-free reference acquisition, balanced live allocations;
- the original Graph shared/Busy tests and original Workflow schema/XSON
  scenarios, including the latter scenarios on actual shared origins;
- Workflow widget source compatibility and C11/C++11/C++17 public headers.

The old schema fixture read an absent optional bool through a typed value
getter, leaving an XRT invalid-argument error that unique C execution ignored.
It now checks whether that optional field exists before reading it. Checked
model error propagation has not been weakened.

There are 33 successful build/run/header commands per platform. Exact final
reports and frozen source witnesses live outside Git:

- `F:/xlang-validation/xge-full-20261110/workflow-shared-native-win-r7/results.json`
- `/var/tmp/xlang-validation/xge-full-20261110/workflow-shared-native-linux-r4/results.json`

Four XRT allocations create an owned Workflow: the original Workflow and Graph
allocations plus one lifetime metadata allocation per model. Share does not
allocate or copy payloads. A warm 1000-count-call loop observes no XRT alloc,
free or realloc calls. Reference/activity, ownership-domain and creator checks
still cost CPU; this is not a claim of raw-C CPU equivalence.

This is a native CPU foundation gate. Workflow canvas reference handoff,
schema/default and associated-Graph callback reentry, owned schema callback
groups, safe language declarations, cross-module cycles, a matched rebuilt SDK,
and final IDE/runtime delivery remain separate work. No device, network or
security test ran, and no normal release tree was installed.
