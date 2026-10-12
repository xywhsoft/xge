# Build configuration

- `profiles.json` defines supported public build profiles.
- `modules.json` is the authoritative source-selection manifest.
- `includes.rsp` is the shared GCC-compatible include-path response file used
  by examples and tests.

Do not duplicate source lists in wrapper scripts; query `tools/build/build.py`
instead.
