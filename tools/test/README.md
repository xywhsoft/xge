# Test runner

List the available test entry points:

```text
python tools/test/run.py --list
```

Run one or more entries by script stem or repository-relative path:

```text
python tools/test/run.py --suite xui build_code_edit_test
python tools/test/run.py tests/xge/build_feature_profiles.bat
```

Individual scripts remain available for focused debugging. The runner always
starts them from the repository root so their paths are consistent.

For the CPU Icon/Category finalizer regression, use an explicitly matched host
engine build and its XRT provider:

```text
python tools/test/check_icon_context_release.py --root . --engine build/host --runtime <matching-XRT-library> --out build/icon-context-release
```

The gate runs GCC O0/O2 standalone clients against the O2 engine: seven release
routes that close the Context from a finalizer, twelve ambient/callback error
cases, and the existing Icon and IconPicker regressions. It opens no window.
`docs/validation/icon-context-release-20261113.json` records the qualified
Windows/Linux cohorts. Object lifetime watches, data models, retained custom
callbacks and other same-object reentry cases remain separate work.

Physical Icon/Category lifetime watches and checked registry-owner removal use:

```text
python tools/test/check_icon_watches.py --root . --engine build/host --runtime <matching-XRT-library> --out build/icon-watches
```

This adds expiry-before-finalizer, peer unwatch, native retained references,
alias retention, Context destruction, accepted-with-callback-error and 1000
zero-heap watch/unwatch pairs to the preceding cases. The gate runs 20 commands
per platform with GCC O0/O2 clients and the explicit O2 engine. This is the
native model floor; it does not certify retained custom callbacks or every
same-object reentry path. See `docs/validation/icon-watches-20261113.json`.

`IconClear` additionally retains its Category until the whole clear body returns,
so finalizers may remove the category, clear the category registry, close the
Context or recursively clear icons without freeing the outer array reader:

```text
python tools/test/check_icon_clear_reentry.py --root . --engine build/host --runtime <matching-XRT-library> --out build/icon-clear-reentry
```

This runs 24 commands, adding the four routes with 16 ambient/callback-error
combinations and 1000 zero-heap empty clears. The regression exits inside the
callback if destruction happens prematurely, before returning to the old array
reader. See `docs/validation/icon-clear-reentry-20261113.json`. Loader/draw
callbacks and other mutation paths remain separate work.

Source freezes must classify `build/shared/obj/*.o`, `.d` and `.json` as mutable
compiler outputs, rather than source inputs. Keep platform build caches
separate when preparing isolated Windows/Linux source trees. The November 13
receipt preserves the original overbroad freeze failures and records an
independent unchanged-source and unchanged-staged-output verification.

Callback-bearing Icon size/prepare/draw and Category preload operations also
hold their native carrier reference and Context receipt for the entire native
body and cleanup tail:

```text
python tools/test/check_icon_operation_lifetime.py --root . --engine build/host --runtime <matching-XRT-library> --out build/icon-operations
```

The 28-command gate adds 56 lifetime/error combinations, including alias
retention, and 4000 zero-heap normal operations. Its raw Painter fixture keeps
an explicit caller-owned Context scope for the Painter session; it does not
claim an owned raw Painter API. Cache invalidation or mutation from callbacks
still needs separate qualification. Platform source snapshots for this cohort
exclude generated build caches and keep Windows/Linux caches separate.

Checked borrowed text reads are covered separately:

```text
python tools/test/check_text_read_views.py --root . --engine build/host --runtime <matching-XRT-library> --out build/text-reads
```

The 16-command gate builds/runs the sized document/editor read, generic edit,
full CodeEdit and TextEdit fixtures at GCC O0/O2. It checks embedded NUL and UTF-8 extents,
present-empty versus absent output, native snapshot OOM, and 1000 cached reads
with unchanged XRT allocation/free/reallocation counters. It does not measure
language string allocation or open a native window. The broad CodeEdit suite
uses the CPU rendering Proxy. Its prior find-scope table failure came from
asserting synchronously before the 80 ms native debounce update. The test now
advances the update loop by 40 ms and 50 ms, checks the pending empty table,
then the expected three columns/three rows. Original failures and baseline
comparisons remain historical evidence; no native find behavior changes.

The selected IconPicker formatter/draw reader and generic cache callback
failure receipt have a combined regression gate:

```text
python tools/test/check_icon_picker_format_lifetime.py --root . --engine build/host --runtime <matching-XRT-library> --out build/picker-format-lifetime
```

Its 36 commands retain the earlier Icon lifetime/diagnostic tests and add 40
selection/alias/unregister/close/error combinations plus 24 generic cache
failure cases. The selected Icon and Context remain retained until all native
format/draw output use is complete. A negative cache callback result is sealed
before draw cleanup/pending destruction; an absent first diagnostic stays
absent, and a later error-report hook cannot replace it. Raw best-effort render
behavior is retained; checked callers balance a frame `xuiCallScopeBegin/End`.

The CPU Proxy's prior and new 1000 warm-render workloads both allocate/free
1000 times and perform zero reallocations. The test enforces that prior native
allocation ceiling, rather than claiming the renderer allocates nothing. It
does not establish timing equivalence, real graphics behavior, owned custom
Icon/formatter closures or every reentrant cache mutation.
