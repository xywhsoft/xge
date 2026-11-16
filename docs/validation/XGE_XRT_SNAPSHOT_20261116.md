# XRT snapshot dependency and identity checks

`tools/package/sync_xrt.py` now follows each selected extension's
`dependency_manifests` recursively, in dependency order. The requested roots
remain xllm, xllm-session and xwork. The current closure also contains xhttp,
xhttp-policy and xhttp-client. Immutable exports read manifests from the selected
commit; only explicit overlays may supply working-tree manifests. Missing,
cyclic or malformed dependencies stop generation before installing the snapshot.

The source fingerprint uses a case-sensitive repository-relative path ordering.
Immutable Git archives explicitly disable autocrlf and select LF. Git archive
otherwise applies the host checkout conversion, so identical commits could
produce different source fingerprints despite identical generated public headers.
The check still compares the complete fingerprint, copied headers/sources and
provenance; it does not bypass mismatches.

Validation on Windows and Linux:

- Seven unit cases per OS cover real Git archives with autocrlf true/false/input,
  mixed-case path ordering, dependency diamonds, the current transitive family,
  cycles, missing dependencies and malformed declarations.
- Actual generation/check of commit a8e51e8697ac3bdb69c5e598d0d41ae9da533ccd
  passes on Windows; Linux independently regenerates and verifies the Windows
  snapshot, including the source fingerprint and all six extension trees.
- The resulting core header SHA256 is
  `1567b04ec597aa0183ad198c413610dda09075704209edccf515a8c91add5726`.
- Independently linked FlowGraph reentry/model and Workflow schema/XSON CPU
  cases pass at O0/O2 on both platforms using this sole XRT implementation.
  The reentry case checks all 63 Graph and 30 Workflow integer entries, history,
  destruction, independent models, OOM and heap balance.

External evidence lives under `F:/xlang-validation/xge-full-20261110/`:
`xrt-sync-canonical-{win,linux}-r4/results.json`,
`xrt-latest-models-win-r3/results.json` and
`xrt-latest-models-linux-r2/results.json`. Earlier failed attempts remain there;
they are not successful acceptance evidence. In particular the first Linux
generation used the Windows-mounted temporary volume and timed out; the
successful regeneration uses Linux-local temporary storage.

The subsequent vendored-source update pins commit
6305aa546828d40a0b081d8d68d21718cc317d51. Its consumed roots are unchanged from
a8e51e86; the canonical source fingerprint is
`70e0bad91666f3d127f055a68050e75af074b2dba7aeb9faccea25d7491b4650`.
Every copied source/header is identical to the independently rebuilt Windows
and Linux default+3D host/static SDK candidates in `latest-native-sdk-*-r2`.
Their actual complete O2 xlang runtimes pass the host binding, shared allocation
and cleanup, and embedded TCC native-call gate: 213 host symbols on Windows and
211 on Linux, without missing symbols. Both xl and xlc runtime images are tested
in independent processes. See `latest-native-vendor-verified-r1.json` and
`latest-xrt-host-{xl,xlc}-{win-r3,linux-r2}/results.json` in the evidence root.

This source update does not install the candidate SDKs or update an IDE, and
does not complete the full xlang integration. The unrelated text/geometry
working-tree changes were excluded from the qualified source candidates.
