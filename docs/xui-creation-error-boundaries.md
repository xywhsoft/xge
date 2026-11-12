# Widget construction and diagnostic ownership

Widget factories isolate an inherited XRT diagnostic from native/type initialization callbacks. On success they restore that exact diagnostic; a new callback diagnostic refuses construction, preserves the first failure and retires the initialized prefix. A raw successful init is a completed stage even when it leaves a fresh diagnostic. Its destructor participates in rollback.

Construction owns a call scope through deferred cleanup. Weak child and owner watches prevent returning an invalidated child or writing a native-part output slot after its owner storage is retired. The native-part hook remains private; public declarations and ABI are unchanged.

`xuiInternalCallScopeEndRestored` is for entry points with no remaining cleanup. It returns the owned diagnostic receipt to TLS. Callers with further cleanup continue using `xuiInternalCallScopeEndOutcome` and explicitly restore their final receipt. TextEdit, CodeEdit offset geometry and MessageList previously omitted this last restoration in 36 paths.

## Validation

Build an embedded engine, then run:

```
python tools/test/check_xui_creation_errors.py --engine <build-directory> --out <new-output-directory>
python tools/test/check_xui_lifetime.py --root . --out <new-output-directory> --cache <object-cache> --tests lifetime drag_drop_reentry owned_callback owned_payload
```

The creation driver uses the actual build report, producer objects and transitive dependency stamps. It runs the consumer at O0 and O2, covering inherited pointer identity, real controls, fresh and integer-only failures, initialized-prefix rollback, nested scopes, cleanup diagnostics, self/context closure and native-part owner closure. `--run-dir` can place bounded test executables in a chosen runtime directory and removes only its owned aliases.

The Windows IDE qualification used a pinned producer base with explicit compatibility patches, not all current XGE source. Its creation fixture passed 191 checks at each optimization. Four implementation mutations were rejected. The selected lifecycle, drag/drop, callback and payload tests passed O0/O2; the separate lifetime-observer suite fails its pending-scope notification assertion equally with the preceding widget implementation. That pre-existing failure is unresolved and is not counted as a pass. The full lifetime driver default suite remains unchanged; `--tests` records an explicitly selected subset.

These checks do not claim sanitizer, Linux, complete heap balance or whole-application performance qualification. Full build identities and IDE integration evidence are recorded by the xlang IDE delivery stage.
