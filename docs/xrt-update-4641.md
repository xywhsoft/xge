# Shared XRT producer update

The vendored producer is committed XRT `4641cd2e4da09d8b1a66872cabe085c263d8b5ba`.
Generation excludes unrelated producer worktree changes. The canonical complete
header SHA-256 is `369f884c1c1fdc9621f654c81bb86850e85d992dc56b9cebf395e20ea03dfe7b`.

All 3,482 previous public signatures are unchanged; three functions are added:
`xrtNetAddrTryParse`, `xrtTlsClientResumeCapacity`, and `xrtTrySwapErrorTake`. The
shared opaque TLS resume declaration also moves into the common TLS header.
This update adds no time-ABI compatibility layer.

Matched isolated Windows/Linux full compiler, default and debug hosted/static
SDKs, plus a Linux EGL pair use the same core. Both platforms pass native host
session O0/O2, public package rendering/error/unload, and live debug checks.
The source receipt is `docs/validation/shared-xrt-4641-20261117.json`. These are
qualification candidates; the xlang canonical source/runtime/IDE convergence
and final installation remain separate. No network tests execute.

Generated HarfBuzz files under build/shared are mutable build-cache outputs,
not immutable source. A separate source-input record verifies every non-build
source byte and records the exact cache changes; native build reports retain
their generated dependency and product hashes. Historical freezes are retained.

The existing manual global init/unit contract remains intact. A language module
that keeps a global GraphicsContext while global init independently pins that
module cannot retire until the initialization ownership cycle is resolved. The
C-resident initializer control retires the same context/global graph. An owning
Engine controller decision and claimed language-callback cleanup are pending;
this update does not claim those patterns are qualified.
