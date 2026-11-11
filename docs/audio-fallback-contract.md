# Sound fallback lifetime

`xgeSoundFallbackSet` copies the path once into an immutable record. It is
thread-safe, accepts configuration before Init and rejects new publication with
Busy while initialization or shutdown owns admission. Allocation failure leaves
the previous configuration unchanged. The path is not part of the resettable
engine context, so a successful Init preserves it.

The existing engine admission spin lock linearizes publication, Clear and
snapshot reference acquisition/release. Path text never changes. Set allocates
and copies outside the lock, then rechecks admission before publishing. Old
records are freed outside the lock only after their last reader releases them.
Reads therefore remain valid while another thread replaces or clears the preset.
Clear is idempotent, thread-safe and allowed during cleanup, including Init/Unit.
It only removes a configuration reference and cannot invalidate a playing voice.

Cold Unit also clears pre-init configuration. Live voices still make AudioUnit
and global Unit Busy before teardown starts. Stop configuration publishers before
final cleanup: an operation overlapping Unit may legitimately linearize after
that Unit has returned; this is not a barrier against future setters.

`xgeSoundFallbackGet` retains its existing caller-storage contract: supply an
empty, zero-initialized Sound. `xgeSoundFallbackCreateOwned` instead returns an
independent stable owning record, clears its output on failure and uses the
ordinary checked Sound release. Both load from one retained path snapshot.
Neither adds a per-read path copy beyond the existing native file/path loader.

Automatic fallback applies only to Sound, including group loading. The created
voice keeps its group backend normally, even if the public Group closes first.
Clearing/replacing the preset does not affect that voice. A missing/unusable
fallback preserves the original ordinary file failure; allocation, admission or
argument failures in the fallback attempt are terminal and are not concealed by
the earlier file error.

The immutable path header adds one `size_t` reference count (8 requested bytes on
the tested x64 ABIs) to the existing single path allocation. Snapshot reads add
short lock/refcount operations; setters use admission checks around the original
allocation/copy. No allocation/free/callback runs under the admission lock, and
no second path is kept in the engine context. The existing owning Sound record
and native decoder allocations remain costs; this is not a zero-cost claim.

`tests/xge/xge_audio_fallback_test.c` exercises pre-init preservation, cold Unit,
error precedence, retained snapshots across replacement/Unit, concurrent native
publishers/readers, closing/initializing admission, overwrite/overflow rejection,
64 allocation prefixes and real PCM/group independence. The private CPU setup
waits for the actual default worker to process a readiness job before taking
heap baselines. This accounts for its transient 48-byte startup proxy without
weakening equality checks or changing production initialization.

Windows/Linux O0/O2 qualification has 412 passing rows, including existing audio,
SoundLoad, FontLoad and Image/Texture regressions. Three old failures reproduce
on both platforms and optimization levels (16 control rows including builds),
before any freed path is accessed. Exact source/full O2 SDK build/report hashes
are in `docs/validation/audio-fallback-20261111.json`. These are local CPU tests,
without a device, window or network. Language/module/final delivery qualification
is recorded separately; this does not pass automatic module-global retirement.
