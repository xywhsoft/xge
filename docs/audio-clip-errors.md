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
