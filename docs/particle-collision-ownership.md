# Owned particle collision callbacks

`xgeParticleWorldSetCollisionOwned` registers a callback on the actual managed
ParticleWorld. The world traces its retained callback owner as well as its code
owner. Each invocation takes and drops an additional owner snapshot through
the complete callback/cleanup return. A NULL descriptor is an explicit trusted
resident-C contract; a language callback always supplies its traced owner.
Raw worlds keep their existing borrowed, boolean-valued collision callback.

Registration requires the creating thread and rejects a busy world. It
snapshots the incoming descriptor before arbitrary Hold code. Failed Hold
keeps the old registration; successful publication followed by failed old
Drop keeps the new registration. Drop consumes its reference even on failure.
Reentry cannot replace a live invocation's callback. An active native call
protects the world if a callback closes its last durable owner.

Managed callbacks return 0 for a miss, 1 for a valid hit, or a negative checked
failure. Other positive values and invalid hit geometry fail. Update/Prewarm
stop on the first failure and restore busy/event-suppression state. Earlier
failure, including failure without a diagnostic, wins over later Drop errors.
Already changed particles are not rolled back; discarded remaining whole
steps are counted as dropped time and are not replayed on the next call.

Finalization detaches the callback under the ownership mutation protocol,
releases simulation contents once, then consumes the detached registration.
Code ownership survives through the final finish phase. Failed callback Drop
is not retried. The existing full world ownership protocol is unchanged.

The raw world remains 496 bytes on the qualified x64 targets. The managed
record is now 600 bytes, 32 more than the previous ownership foundation, for
the callback descriptor. The same small configuration uses five allocations
and 1232/1336 requested live bytes. Registration itself allocates nothing in
the native world. Every actual collision adds one owner Hold/Drop pair plus
managed result validation; no extra metadata is added to each particle.
1000 prepared native callbacks perform zero XRT heap operations. These are
allocation observations, not CPU equivalence or zero-overhead claims.

`tools/test/check_particle_world_ownership.py --collision` executes independent
GCC O0/O2 raw/managed hosted/static clients against matched O2 SDKs and one
complete compiler XRT on Windows/Linux. It reuses all 10,237 kernel assertions,
then checks callback results, malformed hits, primary error preservation,
replacement, close during Hold/body, prewarm cleanup, trace/finalization,
allocation prefixes and hot-path ownership counts. All eight programs per
platform must pass. See `validation/particle-collision-ownership-20261114.json`.
Language declarations, GPU ownership and final runtime delivery are separate.
