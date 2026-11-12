# Document export allocation ownership

`XUI_DOCUMENT_EXPORT_XRT_ALLOCATOR == 1` authenticates the allocation contract
for the seven public `char** + uint64_t*` export producers: serialization,
HTML, plain text, ranges, table matrices, fragment serialization and fragment
HTML. The caller owns an independent allocation and releases it with
`xuiDocumentFreeBuffer`. It is allocated by the resident XRT, has the exact
reported payload length and an additional NUL terminator. Interior NUL bytes
are part of the payload. Custom model allocators and borrowed slices have
their own contracts and are not covered by this marker.

JSON serialization transfers the actual JSON writer allocation instead of
copying it to another heap. HTML and matrix growth use XRT reallocations.
The paired free function, range/fragment outputs, file-export scratch,
renderer range copies, edit/accessibility cached text and MessageList cached
values use the same allocation family. No allocator is inferred from the CRT
or compiler name. Existing callers should continue using the paired free API.

The immutable receipt `validation/native-document-exports-20261112.json`
records Windows/Linux O0/O2 core regression, seven-producer allocation-failure
sweeps, real JSON pointer identity, C/C++ header checks and Linux ASan/UBSan.
Three full hosted-engine UI consumers also finish with zero live XRT delta,
invalid frees and double frees. Hosted/static SDKs rebuild against the
qualified canonical xlang XRT snapshot. This is native allocator/export
qualification, not final xlang compiler, SDK or IDE installation.
