# Shared 3D Scene ownership

Additive CPU Scene ownership reuses the existing native DataObject lifetime
kernel. `SceneCreateOwned` and `SceneShareOwned` each transfer one real native
reference. Legacy `SceneCreate` remains caller-serialized; its physical Scene
pointer and all existing business operations are unchanged.

`SceneAccessBegin/End` pins one reference and one activity until the complete
native call and borrowed-output consumption finish. Begin, End and Share require
the creating thread. A nonfinal release may consume one reference on another
thread; final destruction requires the creator. A refused final release reports
`consumed=false`, allowing a checked retry. End can physically free the Scene
after all application owners are dropped. Do not read its pointer afterwards.
All owned direct C business calls must be enclosed by this access protocol.

The ownership Ops/Adapter/Preparation/Quiescence/CompletionV2/FinalizationV1
interfaces expose actual counts, independent activity and retryable cleanup.
The Scene's native nodes, names, meshes, materials and models contain no owning
language/callback slots. Raw `user_data` is borrowed; it is not traced or freed.
No language `var` userdata or callback-accepting loader is introduced here.

Scene IDs now use an overflow-checked atomic reservation, allowing independent
Scenes to be created on different threads without racing the process counter.
Mutating one Scene still requires serialized creator-thread admission. Internal
node walks and transform calculations gain no per-node ownership locks.

## Qualification

XRT: `4641cd2e4da09d8b1a66872cabe085c263d8b5ba`. Native source parent:
`b44cdfa716e455fda2de8c02fa6b9216fee7f78f`; the three build-source overlays are
`include/xge.h`, `src/xge/3d/xge3d_internal.h`, `src/xge/3d/xge3d_scene.c`.

Windows and Linux each built fresh O2 hosted and static SDKs (no production
replacement object). The actual SDK gate compiled/ran the test at O0 and O2 in
both modes, plus full/core-only 3D source syntax and the disabled public header:
11 commands per OS. The disabled test does not compile excluded private 3D
implementation units and is not a full disabled-profile runtime qualification.

Tests cover sharing, consumed versus refused releases, nested active pins with
the last owner removed, wrong-thread admission/final release, claimed/readiness
and complete graph finalization, legacy cleanup, hierarchy/stale IDs, world and
global transforms, checked allocation failures, name rollback and 128 Scenes
created on four threads. All native body allocations return to baseline; invalid
and double free counters remain unchanged. There is no graphics or network test.

External immutable source/build/gate evidence:

- Windows: `F:/xlang-validation/xge-full-20261110/scene3d-owned-native-win-r1`,
  `scene3d-owned-sdk-win-r1`, `scene3d-owned-native-gate-win-r4`.
- Linux: `/var/tmp/xlang-validation/xge-full-20261110/scene3d-owned-native-linux-r1`,
  `scene3d-owned-sdk-linux-r1`, `scene3d-owned-native-gate-linux-r4`.
- Exact before/after layout probes: `scene3d-owned-layout-{win,linux}-r1` in the
  corresponding validation root. The small qualification index pins reports.

## Cost and limits

On both tested x64 builds, the legacy Scene body grows from 672 to 680 bytes;
an owned Scene appends the existing 32-byte lifetime root. Construction remains
one XRT allocation (712 requested bytes for an owned Scene). Sharing allocates
no native body. A 1000-call native query loop using Begin/End has zero XRT heap
allocations. Each protected call still performs reference/activity and mutation
domain bookkeeping; this is not a zero-CPU-overhead claim.

GPU resource ownership, rendering, model/animation ownership, managed userdata,
callbacks and language delivery are separate work. No release files are installed
by this native batch. The original C precondition to free GPU resources before
destroying their context remains in force until those owned families are added.
