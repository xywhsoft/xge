# Matched XRT804 SDK qualification

Align the vendored XRT core with committed upstream
`804cc769de3030118c2b46ef7af69543d1b82d9d` and the qualified xlang
strict-data compiler. Only four vendored files change; selected extension
dependencies and native XGE/XUI implementations remain byte-identical.
The graph copy flag is additive. Existing copy behavior and record layouts
remain unchanged. The new strict DATA mode refuses opaque fallback in one
traversal; it is not a change to legacy Clone semantics.

Windows and Linux each built fresh default-plus-3D hosted and direct-external
static SDKs at O2 (164 + 166 compilation units, no reused production objects).
The hosted SDK resolves the one compiler XRT; the static SDK has no embedded
XRT or host-import slots. No final runtime or IDE was installed.

Each OS passed 43 source-unit steps (including the original baseline
reproduction) and 48 actual hosted/static client steps at O0/O2. These cover
all 99 integer model Busy boundaries, Shared reference identity, original
Clone/history isolation, checked lifetime, allocation failures, schema/XSON
regressions and imports. Warm Shared operations retain the previously
qualified zero-XRT-heap result; admission and reference operations remain.
Separate verification rechecked all frozen build inputs, objects, dependency
records, gates, logs and client product hashes: 7,659 Windows and 7,451 Linux
files. Language model promotion and complete SDK/IDE delivery remain open.
No network, graphics-device or security tests were run.
