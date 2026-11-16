# Additive XRT 09d snapshot qualification

The vendored core now pins XRT `09d543a3e29f3113d258b1f58718345de867e06e`.
Generation used that immutable commit, excluding the upstream working tree.
The 3481 existing public function declarations are unchanged; one optional
WebSocket TLS-ticket notification function is added. No network test ran.

Core SHA256: `4c1dd0e450c20d2ab887e7552cd038e5b394d8659fd1ae6f66211f76d4625c36`.
The Native-owned `xrt_config.h` and README remain intact. All 13 preexisting
text/geometry working-tree changes were preserved and excluded from this batch.

External evidence under `F:/xlang-validation/xge-full-20261110/`:

- `xrt-latest-committed-09d-r1/results.json`: isolated producer regeneration.
- `xrt-09d-additive-audit-r1.json`: existing declaration comparison.
- `shared-09d-native-sdk-{win,linux}-r2/results.json`: full host/static SDK builds.
- `shared-09d-host-{xl,xlc}-{win,linux}-r1/results.json`: exact XRT function
  identities, shared heap, Context cycles, embedded TCC calls and balanced cleanup.
- `shared-09d-model-native-{win,linux}-r1/results.json`: 33 source-unit
  O0/O2/header checks per platform, including original model/schema/XSON and
  shared model, Busy, OOM and DataObject cases.
- `shared-09d-source-promotion-native-r1.json`: unchanged unrelated inputs.

The first SDK preparation omitted the Native-owned config and failed before
compilation. The independent `r2` preparation restored exactly that config and
README; original failed evidence remains preserved. This qualification does not
install a shipping IDE/runtime, cover GPU/desktop performance, or complete the
remaining Workflow language integration.
