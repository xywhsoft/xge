# Owned image, texture and font loads

`xgeAsyncImageCreateOwned` / `xgeAsyncTextureCreateOwned` create empty shared
requests; `xgeAsyncFontCreateOwned` adds the matching font request when text is
enabled. Register the exact `int(void*)` callback, then call the matching Start.
The request owns its target, URI, callback node, stored error and an independent
code owner. No caller target pointer escapes. The explicit C request API remains
separate; Sound owned loads are still pending the audio resource contracts.

Only the creator calls methods or releases the last native reference. Start and
Poll hold activity through provider callbacks, completion snapshots, cleanup and
the complete native return. Callback self-clear, result transfer and dropping the
last caller reference are valid. Reentrant Start/Reset return Busy. Owner hooks
run outside locks; failed callback replacement preserves the previous slot.

Start accepts a valid operation even if a synchronous provider immediately fails.
Info exposes Failed and the original result code; ThrowIfFailed delivers its
stored diagnostic. Worker diagnostics are captured before the XRT thread boundary
clears them. A source callback contract failure is immediately checked, kept
as the failed result and never followed by a completion callback. It remains
observable even when that callback also requested cancellation. A completion
callback failure is separately reported by Start/Poll and does not rewrite an
already settled result. A notification is attempted at most once.

File image/texture decoding can use a worker. Providers still run synchronously
on the creator; resource failures starting a worker do not silently switch to a
blocking decode. Only Poll commits worker output to a texture. Cancellation
suppresses completion and result publication, without interrupting OS IO/decoding.
Reset explicitly discards an unclaimed result/error and keeps the callback; it is
nonblocking and returns Busy while a worker or enclosing callback is active.

Font requests preserve the existing synchronous caller-thread FONT contract,
including local files with ThreadingSet enabled. They do not create a worker or
claim that font IO is nonblocking. Completion can run inside FontStartOwned;
register it before starting. An accepted decoder failure is stored in Info and
delivered by ThrowIfFailed, just like the other owning requests.

TakeImage moves its pixel allocation into empty caller storage exactly once.
TakeTexture allocates/adopts an independent native owned record; allocation
failure leaves the request's result intact. Texture work reserves the existing
graphics owner obligation before loading and transfers/releases it with the
result, so checked Unit cannot end a backend underneath pending work/results.
There is no added pixel copy at the transfer boundary; the normal decoder and
texture construction still have their original native allocations/copies.

FontStartOwned allocates its private zeroed Font record on demand. The stable
record is used from the first native load through FontTakeOwned. Take adopts and
detaches that same record without copying or allocating; it may be called from
completion, and an unsuccessful adoption leaves the result in the request.
Reset/last release destroys an unclaimed font and the private record. The request
keeps the graphics lifetime during loading and while its result is unclaimed;
taking transfers that obligation to the owned Font. A failed private-record/URI
allocation rolls back this obligation and leaves the request Pending for retry.
The common target union still occupies the old texture-sized slot: a Font pointer
does not embed the larger Font record in every Image/Texture request.

Finalization requests cancellation but never blocks joining an unfinished worker.
It returns Deferred without consuming the caller's release duty; retry on the
creator. Cleanup waits for the actual XRT thread completion, not a flag written
before the worker's physical return. Callback/error edges are cleared only after
that point; the independent code owner is dropped last by the finishing phase.
The existing Count/Trace, preparation, quiescence, finalization and completion
protocols expose the real references and deferred steps to language runtimes.

File workers retain only the final read diagnostic. A failed application-directory
probe is discarded before the working-directory fallback; terminal memory errors
and genuine final IO failures are retained. See `async-relative-path-contract.md`.

The 23-mode O0/O2 native gate covers shared refs, private result independence,
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

The 2026-11-11 Font addition passes 232 native build/run rows on Windows/Linux:
four host/static O0/O2 variants per OS exercise 16 Font modes, then the existing
23-mode Image/Texture gate runs at O0/O2 on both OSes. Font private-record/URI
allocation rollback is retryable, every one of 64 allocation budgets balances
owners/live heap, and Take succeeds with a zero-allocation budget. Each variant
has 6 checked failures and 58 successes. Empty Font/Image/Texture requests all
remain 1 retained allocation / 248 requested bytes on the tested x64 ABIs.
This is warmed XRT requested storage, not total heap capacity or hardware timing.
Exact frozen source/full rebuild/report pins are in
`docs/validation/async-font-owned-20261111.json`.
