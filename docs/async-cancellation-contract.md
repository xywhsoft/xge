# Async cancellation and completion

`xgeAsyncRequestCancel` suppresses the completion callback. The shared finish
path now observes this for both file-backed worker decoding and a resource
provider that cancels reentrantly during synchronous loading. Previously both
paths called completion after cancellation, contrary to the public header.

Cancellation does not interrupt an OS read or decoder. Request storage, URI
and private worker output must remain alive until the worker has physically
returned; raw RequestFree joins it. Poll commits a successful worker result
on the caller thread and discards cancelled output. The synchronous raw C
loaders still use caller-supplied target storage; cancellation is not a promise
to roll back writes already performed by that synchronous loader.

Normal completion invokes its callback once. The callback may call RequestFree;
the finishing function and Poll must capture their return values before that
call and never inspect the freed/reset request afterward. A later Poll sees
the reset pending state rather than invoking the callback again.

`tools/test/check_async_cancellation.py` runs six modes at O0/O2: cancelled
file worker, cancelled synchronous provider, both successful paths, and both
successful paths with callback self-release. It uses a local 1x1 TGA file or
an in-memory provider, without a window, graphics/audio device or network.
The pre-fix Windows control fails both cancellation modes at both optimization
levels and passes both normal-completion controls.

The exact candidate, reports and preserved pre-fix source are indexed in
`docs/validation/async-cancellation-20261110.json`. This narrow C fix does not
provide the pending managed async request, result, callback and code owning
graph for xlang, and it does not replace a published SDK or runtime.
