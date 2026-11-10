# CPU clip optional-allocation errors

`xgeAudioClipDecodeMemory` already has a complete PCM result before its optional
buffer shrink. An allocation failure in that shrink used to return `XGE_OK` and
the valid clip while leaving `XERR_MEMORY` in the shared diagnostic channel.
Checked callers therefore observed a failure after an otherwise successful
decode. A generated valid stereo WAV plus allocation budget 2 reproduces this
on both Windows and Linux, without initializing an audio device or window.

The decoder now skips a shrink when capacity already equals the frame count.
Otherwise it isolates the optional allocation's diagnostic and discards its
memory error while keeping the valid PCM buffer. A pre-existing diagnostic is
restored exactly. Unexpected non-memory diagnostics remain failures and release
the unpublished result. Required allocation failures keep their checked OOM
status and null output. This changes no public signature, format or PCM bytes.

Windows and Linux matched host/static libraries, each at O0/O2 for the test
program, pass all 16 build/run rows. Each execution verifies eight allocation
budgets (2 required failures, 6 successes, including one optional failure), exact
PCM samples, ambient-error identity, balanced live allocations/bytes, and no new
invalid or double frees. Old host libraries fail the same success/error check
on both systems. The tests never initialize audio or graphics and perform no
network operation.

The cold shrink path adds diagnostic preservation calls; there is no extra PCM
copy, allocation or forwarding layer beyond the existing optional realloc.
This is a CPU decoder error-boundary qualification. It does not qualify device
playback, engine/group/voice ownership or the pending complete xlang audio API.
Exact frozen inputs and evidence are indexed in
`docs/validation/audio-clip-errors-20261111.json`.

The additive `xgeAudioClipGetSamples(clip,&samples,&count)` API supplies the
immutable interleaved PCM pointer and exact scalar-sample count together. It
resets both outputs on invalid input and borrows the caller's retained clip.
The legacy pointer-only getter remains available. The new function performs no
allocation, copy, device access or global engine operation; its count is not a
frame or byte count. This provides a typed producer for owning read-only
language sequences without a language-side forwarding function.

After a full native rebuild for the public header change, Windows and Linux
host/static O0/O2 programs pass 32 build/run rows across both the OOM fixture and
the span fixture. Span runs verify exact PCM/count, independent native retention
after the original reference is freed, invalid-output reset and 1,000 repeated
reads with allocation prohibited. Their source/SDK/runtime hashes are indexed in
`docs/validation/audio-clip-span-20261111.json`. This native producer does not
mean that the xlang AudioClip API, its thread contract, cancellation or playback
resource integration is complete.
