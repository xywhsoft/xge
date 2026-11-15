# FlowGraph configuration replacement

`xuiFlowGraphSetNodeConfig` deep-clones the supplied XRT value. A custom
handle's Clone can execute application code; retaining a node index across
that callback is unsafe. A reproduction removed the selected node from
Clone and caused a NULL node dereference after cloning returned.

The function now remembers the revision before cloning and rejects a changed
model with `XUI_ERROR_INVALID_STATE`. It releases its new copy once and leaves
the callback's own changes intact. Removing and recreating the same id also
counts as a model change; a new node cannot silently receive the old request.
The revision remains a finite native counter, not a long-lived identity token.

On replacement, install the new configuration and advance the revision before
releasing the old value. Its Drop can remove or relocate the node, so the
function must not access the node or graph after that release. A release
callback's side effects do not undo the replacement that already committed.

`tests/xui/xui_flow_graph_config_reentry_test.c` covers removal during Clone,
same-id recreation, removal from the old configuration's Drop, exactly-once
clone cleanup and balanced live allocations over repeated runs. The Windows
entry point is `tests/xui/build_flow_graph_config_reentry_test.bat`.

This is targeted invalidation protection, not a promise that arbitrary graph
reentry or destruction is supported. Other configuration consumers, command
history and a uniform graph callback admission rule require their own review.
Thread-affinity, caller lifetime and existing trace quiescence requirements
remain in effect. No native resource scope, shared-owner or language API is
introduced by this fix.
