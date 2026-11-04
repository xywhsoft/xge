# Typed Context and merge callback acceptance

The remaining thirteen widget ContextMenu publishers now have independent typed
Owned APIs. They cover TagInput, Tabs, Window, DockPanel, ListView, TreeView,
TableView, TableGrid, PropertyGrid, Toolbar, StatusBar, FlowGraphWidget and
WorkflowWidget. TableView MergeProvider is the fourteenth publisher. Fifteen
actual dispatch locations use snapshots of the matching function, userdata and
owner. Scalar arguments and caller-owned hit records precede reentrant Hold.

Raw setters clear old owners even when fn/user addresses are unchanged. Each
sparse owner slot participates in Context ownership tracing; no callback is cast
to a generic signature. Publication retains and allocates before committing.
Native call scope covers old-owner cleanup and callback physical return.

Rollback, multi-slot cleanup and snapshot return preserve the first status and
its exact diagnostic, including NULL. Later cleanup cannot replace an earlier
failure simply because it could allocate a message. A failed snapshot Drop
conservatively rejects an int-result veto. Owned Tree count and Table merge
negative returns propagate rather than publishing an empty tree or accepting a
span update.

Windows GCC O0/O2 clients against the matched O2 engine pass:

- All fourteen cold publishers, OOM, ownership trace and failed Hold.
- All fourteen actual consumers, self-clear and close from callback/Hold.
- Reentrant Hold replacement with independent matching userdata.
- First NULL failure followed by an allocated secondary cleanup error.
- A veto whose final Drop fails, leaving the current frame unchanged.
- One thousand ordinary context dispatches with zero allocations/reallocations.
- The prior 51 publishers, six groups, Tree, Cell and Table suites, including
  the 996-case raw provider lifetime regression.

Evidence is under native-callback-context-*-win-r1 and context-context-win-r5 in
the task validation directory. Linux verification is pending at this freeze.
These are native API results; managed declarations/integration have separate
acceptance. Complex editor configuration, providers, raw per-item/column render
and TableGrid owning adapter migration are still tracked as remaining work.
