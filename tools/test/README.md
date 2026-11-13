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
