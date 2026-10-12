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
