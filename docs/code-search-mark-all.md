# Mark-all regex work and limit cleanup

`xuiCodeSearchMarkAllRegex` now prepares one counted document snapshot and one
compiled regex/matcher for the complete scan. It reuses the existing forward
match, case, whole-word and multiline rules. Empty matches are skipped and
advance by the original grapheme rule. Backward/wrap flags remain ignored for
mark-all. Selected old indicators are cleared before scanning; allocation or
match failure can leave partial new indicators. This is not atomic rollback.

The old loop called `xuiCodeSearchFindRegex` for every match plus the final
not-found result. Each call compiled a new regex/matcher and copied the entire
subject. Actual O0/O2 hosted/static clients mark 64 matches: compile/matcher/
release counts fall from 65/65/65 to 1/1/1. Static-only link instrumentation
counts copies 65 to 1 and subject bytes 16640 to 256. XRT heap counts fall from
3825 alloc/3825 free/1949 realloc to 68/68/37 at the same workload and revision
inputs. Hosted counters are installed through the existing XRT resolver;
static counters use test-only linker wrapping. No instrumentation is added to
product code. These are work/heap measurements, not a CPU-equivalence claim.

The whole operation keeps a real document call pin. The snapshot remains
independent of later state and retains embedded NUL. Regex and text storage are
released while the first status/diagnostic is preserved, then the pin is ended.
Regex-disabled source variants now explicitly return unsupported, zero count
and a bounded diagnostic instead of silently reporting successful zero matches.
The disabled-source tests are not qualification of a complete optional SDK.

The preceding counted-replacement repair introduced a missing regex release
when its checked document length equals INT_MAX. That branch now destroys the
already created regex before ending the scope. A test-only static GetLength
adapter reaches this boundary using an ordinary three-byte document; the
actual unchanged SDK reproduced the leaked live allocations and missing release.
The repaired SDK releases exactly once and returns to its prior live count.
No 2 GiB allocation or forged native object layout is used. The real oversized
input itself is not claimed to have been executed.

Windows/Linux O0/O2 hosted/static gates cover NUL, case/word/flag behavior,
Unicode/zero-width progress, invalid patterns, no matches, 512 allocation
budgets, lifetime balance and the bounded limit branch. Existing counted
replacement, composite editing and document ownership suites are rerun against
the same new SDK. There are 164 actual SDK build/run regression steps plus eight
disabled-source branch steps; 36 old control steps are separate. Initial fixture
attempts (unused test flag, uncleared caller diagnostic) are retained; final
controls and all product gates pass. Native C callers clear an error they have
handled before entering a later checked operation.

The O2 host relinks compile three units/reuse 161 because the original qualified
full parent predates the earlier document kernel changes. Static builds compile
two/reuse 164. This is dependency-qualified reuse, not a full source rebuild.
Versus the previous counted-search SDK, host text grows Windows 896/Linux 704
bytes, BSS is unchanged, and file deltas are +512/-32 bytes (including linker/
export layout). Exact source, cohorts, counters and logs are pinned in
`docs/validation/code-search-mark-all-20261114.json`. No public C signature or
layout changes, network test, final runtime/IDE install or full ecosystem
completion is claimed.
