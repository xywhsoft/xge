# Stable DataObject generations

This layer fixes native reentrancy in both XGE and XUI DataObject families.
Their existing C APIs and caller-serialized mutable-container contract remain.
It does not certify raw callbacks as safe language closures, add managed graph
provenance, solve external COM code residency, or complete clipboard atomicity.

Each slot owns a stable immutable generation. A provider Get holds a real
generation reference and a real object/activity reference through the complete
provider return and final generation cleanup. Replacement publishes its fully
prepared generation first, then retires the detached old generation outside
publisher state. Final object destruction detaches the whole slot array before
running arbitrary raw cleanup hooks. Such hooks cannot publish into or revive
the destroyed object. Reentrant publication can replace the newly installed
generation and is never overwritten by the outer cleanup continuation.

Metadata, the stable format name and optional bytes occupy one allocation.
Failed publication consumes no raw provider userdata duty. New bytes are
copied before publication; allocation failure leaves old formats and values
unchanged. Capacity growth and allocation arithmetic are checked. Binary NUL
bytes are copied with explicit lengths. A failing provider's first nullable
diagnostic survives cleanup, including a secondary cleanup diagnostic.

Pure byte Get has no user callout, so it retains direct length/copy handling
under the existing serialized C contract. Provider Get adds object acquire /
release, activity increment / decrement and generation acquire / release: six
uncontended atomic reference/activity operations. This protects actual native
state; it is not a claim that arbitrary concurrent mutation became supported.

Measured C11 x64 sizes agree on Windows and Linux:

| Storage | Before | This layer |
| --- | ---: | ---: |
| DataObject | 24 bytes | 32 bytes |
| Per-format metadata | 48-byte slot | 8-byte slot + 56-byte generation |

The earlier preliminary design estimated the old object as 32 bytes. Actual
compiler sizeof measurement corrects that estimate to 24 bytes. The final
cost is +8 bytes per object and +16 bytes per format before allocator headers.
The generation, format and bytes use one allocation; initial slots need one
separate array allocation. Replacement of an existing bytes value remains one
allocation, as confirmed by injected allocation-failure budgets. There is no
second Buffer or output copy in Get.

Seven alternating paired O2 samples of one million 16-byte Get calls measured
the following medians on the validation machine. These are observations, not
a portable throughput guarantee or a zero-cost language binding claim.

| Platform / family | Bytes, old → new | Raw provider, old → new |
| --- | --- | --- |
| Windows XGE | 11.34 → 8.91 ns | 7.60 → 13.05 ns |
| Windows XUI | 9.33 → 8.46 ns | 8.08 → 14.16 ns |
| Linux XGE | 8.79 → 7.85 ns | 7.62 → 15.58 ns |
| Linux XUI | 8.73 → 8.43 ns | 7.69 → 15.00 ns |

The old raw provider implementation had no equivalent reentry protection, so
its timing is an unsafe legacy baseline, not a semantically equivalent safe
implementation. Native bookkeeping is reported rather than hidden.

The focused gate covers self replacement plus slot-array growth, last-owner
release inside Get, cleanup reentry, final-destruction reentry rejection,
negative/NULL first error, binary values, size probes, insufficient output,
and allocation failures. The original XGE data/drag and XUI drag tests also
pass O0/O2 against the same matched production objects. No engine or desktop
initialization, real clipboard change, browser, GPU, or network test is run.

Acceptance and immutable source/object evidence are indexed by
`docs/validation/native-dataobject-generations-r1.json`. Managed DataObject
Count/Trace/retirement, typed Owned providers/DataOutput, external code-tail
capabilities and atomic multi-format clipboard remain separate B work.
