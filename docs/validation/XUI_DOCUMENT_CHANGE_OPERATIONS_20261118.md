# Immutable Document change operations

`xuiDocumentChangeSetGetOperations` returns the same contiguous operation
array as the change info record. It uses separate pointer and `size_t`
count outputs and performs no allocation or array copy. Holding an
independent reference to the change set keeps the array readable after the
document and transaction close. The operations remain immutable.

The CPU-only fixture checks invalid arguments, pointer and count identity,
and reading the array while only the independent reference remains.
It builds and passes with the generated hosted and static SDKs at both
O0 and O2 on Windows and Linux.
