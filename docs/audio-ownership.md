# Audio ownership and checked shutdown

Playback and group operations use the global engine's creator thread. Immutable
PCM clips retain their separate CPU/reference contract. Public Sound, Music,
Stream and Group record layouts have not grown.

## Parent and record lifetime

A successful sound or group initialization reserves one global-engine owner.
Both audio-only shutdown and global `xgeUnitChecked` refuse live backends with
`XGE_ERROR_BUSY`, before tearing down the engine. The legacy void AudioUnit
preserves it and reports a checked error. Release resources and retry shutdown.

These audio-engine owners are separate from graphics-context owners. They cover
in-flight constructors as well as finished backends, so reentrant audio-only
shutdown cannot destroy the constructor's parent. Audio prevents global engine
shutdown, but does not make an unrelated renderer wait for audio. The shared
admission lock/creator identity is reused; there is no additional lock allocation,
redundant production group counter or graphics dependency for a mixer node.

Every voice owns a reference to its private group backend. GroupFree clears the
public handle and releases its reference. If voices remain, the actual mixer node
and its global-engine reservation remain until the last voice releases them.
Sounds created from a clip also retain their immutable PCM independently.

`xgeSoundCreateOwned`, `xgeSoundCreateFromClipOwned`, `xgeMusicCreateOwned` and
`xgeStreamCreateOwned` return stable allocated public records. Failure resets
`*out` to NULL. Their ordinary matching Free/Close releases the backend and final
outer record. SoundAddRef retains that same stable record. Do not additionally
free the public pointer or copy a public record as an owning reference.

Existing in-place initializers require a zero-initialized/freed destination and
reject replacing a live sound/group. They retain caller-owned public storage;
ordinary release zeroes that storage. Existing repository callers were checked
for zero initialization. Failed constructors release all acquired parent links.
Invalid arguments, live-record replacement and explicit OUT_OF_MEMORY statuses
are not eligible for fallback. Shared path resolution reports a typed result,
stops on OOM, and preserves a caller diagnostic when a non-memory directory probe
recovers. Font saving uses the same private path contract.

The miniaudio allocation callbacks obey C allocation semantics: NULL reports a
failure to miniaudio, which may recover. A fresh allocation OOM is isolated there,
instead of leaking out of a successfully recovered decode/playback operation.
Existing caller errors are retained; fresh non-memory diagnostics remain visible.
Required miniaudio OOM results map to an explicit XGE allocation status. This is
shared allocator integration, not a per-method success-error suppression patch.

Checked SoundFree and GroupFree reject wrong-thread destruction without changing
the record. The legacy void release reports that error and preserves the resource
for a correct-thread retry. This does not silently transfer playback controls to
another thread or destroy a live engine from a worker.

For example, after normal XGE/audio initialization:

```c
xge_audio_group_t group = {0};
xge_sound sound = NULL;
int result = xgeAudioGroupInit(&group);
if (result == XGE_OK) {
    result = xgeSoundCreateOwned(&sound, "effect.wav", &group);
}
xgeAudioGroupFree(&group);
/* A created sound still owns its group backend here. */
if (sound != NULL) {
    /* AudioUnitChecked reports BUSY until sound's final release. */
    xgeSoundFree(sound);
}
result = xgeAudioUnitChecked();
```

## Verification and costs

`tools/test/check_audio_lifetimes.py` includes the production aggregate and uses
private test setup to select miniaudio's real no-device engine. It mixes actual
nonzero PCM after closing a group handle, exercises independent owned file
Sound/Music/Stream records, checks creator-thread rejection and refuses live
record replacement. It covers 40 clip-voice and 48 file-voice allocation budgets
for each of Sound, Music and Stream, then verifies owner/heap/invalid-free counters
return to baseline. Every successful file budget must have a clean diagnostic
and produce actual PCM. Streaming reads yield to real page jobs with a bounded
wait. Direct allocator/path tests preserve ambient and severe errors.

The separate lifetime-kernel test proves that an audio/global-engine owner blocks
global close while allowing graphics-context admission and completion, including
phase, overflow, wrong-thread and concurrent graphics close checks. That is a
state-machine test, not an assertion that a real EGL device was used by audio.

Old-source controls fail at the expected engine-preservation/group-retention
assertions before accessing a freed node. They are preserved evidence, not
passing release qualification. The matched native dynamic/static SDKs also run
existing reference, PCM, FontLoad and async loading regressions.

On the qualified x64 layouts, public Sound is 24 bytes and Group 16 bytes.
Private sound/group backends each grow by 8 bytes, without another backend
allocation or a PCM copy. An owned factory adds one 24-byte outer-record
allocation. The shared lifetime state grows from 32 to 40 bytes and audio adds
one transition byte. miniaudio initializes its sound
nodes; file/group wrappers avoid redundant full-object zeroing. Cold parent
admission/release and hot creator-thread checks use the existing spin lock and
thread identity. Each successful allocation callback adds one error-context lookup;
an existing ambient error additionally incurs retain/release, while failure does
the diagnostic filtering. These costs are native safety behavior, not zero-overhead
claims. Shared Music/Stream controls use scalar projections with compile-time
type/offset proofs, avoiding incompatible struct lvalues without copying records.

Raw evidence and matched build/source hashes are indexed in
`docs/validation/audio-ownership-20261111.json`. The no-device tests do not prove
physical device playback, complete xlang audio bindings, language module-global
code residency or a final SDK/IDE distribution.

## Resident-engine allocation failures

The follow-up test keeps one audio engine alive across every file-construction
failure, rather than relying only on a final engine shutdown. It exposed a
miniaudio v0.11.25 synchronous connector failure that retained a 112-byte cache
node until engine destruction. The local vendor fix returns the acquired node
and uninitializes the base before returning the failure; successful playback
and public ABI are unchanged. See `extlibs/miniaudio/XGE_PATCHES.md` for pinned
header hashes and the exact patch boundary.

Modes 13-15 check 64 caller-thread allocation prefixes each for Sound, Music and
Stream, with exact live-heap/parent balance after every iteration. Both platforms
pass at O0/O2. The unpatched Sound controls fail before engine shutdown on both
platforms at both optimization levels. Together with existing lifetime/reference/
PCM regressions, this follow-up has 148 passing rows and 8 expected control rows,
indexed by `docs/validation/audio-resident-20261111.json`. Worker-thread failure
injection and physical audio-device behavior are outside this check.
