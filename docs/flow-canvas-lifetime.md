# Flow canvas model ownership during callbacks

FlowGraphWidget owns its model when constructed with native defaults or an
owning descriptor. WorkflowWidget owns its workflow similarly; its internal
canvas borrows that workflow's graph. The composite retires its canvas before
destroying the workflow, so the graph's values have exactly one owning trace.

Configuration Clone/Drop callbacks can reenter UI code. A same-model Busy
rejection must preserve the actual widget owner, not just decline to free the
graph. Previously the widget could discard its model pointer after a void
destroy refused Busy. Both widget deletion and whole-Context destruction could
then orphan the native model. A real SDK reproduction covers these paths.

Both canvas types now participate in XUI's existing pure destroy-readiness
protocol. Widget logical close still invalidates its views immediately, while
physical cleanup retains the model slot until its callback has returned. A
checked Context destroy reports deferred with `consumed=false` in that interval.
The caller retains its Context owner and can retry after the native call ends.
Existing native Context call scopes provide the normal deferred physical tail;
a raw C caller operating directly on a model must respect that owner/retry rule.

Model replacement uses the existing Context call scope and a per-canvas
mutation flag. Reentrant replacement is Busy, including cancellation and old
configuration Drop callbacks. Legacy pointer getters return NULL and report
Busy without replacing an earlier error. Hover/hit queries preserve their
output on Busy. The hit-test guard uses the existing type-data lookup.
Busy factory rejection preserves the caller's output and consumes no model.

A workflow's internal canvas cannot be independently rebound through the public
FlowGraphWidget setter. That operation returns `XUI_ERROR_INVALID_STATE`; use
the owning WorkflowWidget setter instead. Its private entry checks the actual
native composite authority, following XUI's existing owned-part convention.

The flags occupy previous padding: the exact compiled private layouts remain
872 bytes for FlowGraphWidget and 40 bytes for WorkflowWidget on the measured
Windows/Linux x64 configurations. Readiness checks and successful setter scopes
still have CPU/bookkeeping cost; this is not a zero-overhead claim. Busy checks
in the regression allocate, free and reallocate zero XRT heap objects.

`tests/xui/xui_flow_canvas_lifetime_test.c` covers both types, current/input
model Busy, output canaries, constructor rejection, public-part authority,
independent peers, copied-value lifetime, replacement, close/retry, closure
during retired-value Drop, 64 allocation-failure prefixes and heap balance.
It must run against matched real hosted/static SDKs at O0 and O2.

This does not define shared/independent model references or their long-lived
language aliases. Legacy setters may report expiry after committing a borrowed
input; that behavior is explicitly exercised with caller-owned input retained.
Safe language model transfer still needs a separate ownership/result contract.
General Workflow schema, variable and validation callback reentry is also
outside this graph-configuration prerequisite.
