# Workflow configuration diagnostic reports

`xuiWorkflowCollectConfigDiagnostics` visits the same schema fields as
`xuiWorkflowValidateConfigEx` in a single pass. It returns an owned array of
`path`, `code`, and `message` records. The texts remain valid after model
release. Missing configuration can be passed as either C NULL or XRT null.

The legacy count and fixed-capacity array both use the same validation rules.
They retain their existing borrowed output and do not allocate report values.
The new report returns no rows if allocation fails at any point.

The CPU-only fixture covers 20 diagnostics, the legacy count/prefix behavior,
invalid type lookup and result survival after model and schema release. It
passes GCC C11 O0/O2 on Windows and Linux with the matching xlang XRT.
The complete tested allocation-failure prefix is 184.
