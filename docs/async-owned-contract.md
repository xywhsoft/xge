# Owned image and texture loads

`xgeAsyncImageCreateOwned` / `xgeAsyncTextureCreateOwned` create empty shared
requests. Register the exact `int(void*)` callback, then call the matching Start.
The request owns its target, URI, callback node, stored error and an independent
code owner. No caller target pointer escapes. The explicit C request API remains
separate; Font/Sound owned loads are still pending their resource contracts.

Only the creator calls methods or releases the last native reference. Start and
Poll hold activity through provider callbacks, completion snapshots, cleanup and
the complete native return. Callback self-clear, result transfer and dropping the
last caller reference are valid. Reentrant Start/Reset return Busy. Owner hooks
run outside locks; failed callback replacement preserves the previous slot.

Start accepts a valid operation even if a synchronous provider immediately fails.
Info exposes Failed and the original result code; ThrowIfFailed delivers its
stored diagnostic. Worker diagnostics are captured before the XRT thread boundary
clears them. Callback failures are separately reported by Start/Poll and do not
rewrite the load's settled result. A notification is attempted at most once.

File image/texture decoding can use a worker. Providers still run synchronously
on the creator; resource failures starting a worker do not silently switch to a
blocking decode. Only Poll commits worker output to a texture. Cancellation
suppresses completion and result publication, without interrupting OS IO/decoding.
Reset explicitly discards an unclaimed result/error and keeps the callback; it is
nonblocking and returns Busy while a worker or enclosing callback is active.

TakeImage moves its pixel allocation into empty caller storage exactly once.
TakeTexture allocates/adopts an independent native owned record; allocation
failure leaves the request's result intact. Texture work reserves the existing
graphics owner obligation before loading and transfers/releases it with the
result, so checked Unit cannot end a backend underneath pending work/results.
There is no added pixel copy at the transfer boundary; the normal decoder and
texture construction still have their original native allocations/copies.

Finalization requests cancellation but never blocks joining an unfinished worker.
It returns Deferred without consuming the caller's release duty; retry on the
creator. Cleanup waits for the actual XRT thread completion, not a flag written
before the worker's physical return. Callback/error edges are cleared only after
that point; the independent code owner is dropped last by the finishing phase.
The existing Count/Trace, preparation, quiescence, finalization and completion
protocols expose the real references and deferred steps to language runtimes.

The 21-mode O0/O2 native gate covers shared refs, private result independence,
callback removal/last-owner release/failure/reentry, provider and worker errors,
both cancellation paths, wrong-thread refusal, synchronous and threaded OOM
prefixes, failed output transfer rollback, graph edges, code owner hold failure,
and deterministic blocked-worker release and claimed-graph finalization. A test-only typed thread gate
proves the deferred physical-return branch without relying on timing. A warmed
1,000-iteration Info/Poll loop survives a zero-allocation budget. This does not
claim no bookkeeping, no cold allocations or hardware performance.

See `docs/validation/async-owned-20261110.json` for exact source/build/report pins.
Native and language integration qualifications are separate; this batch is not
a published SDK/compiler/IDE delivery or completion of all async families.
