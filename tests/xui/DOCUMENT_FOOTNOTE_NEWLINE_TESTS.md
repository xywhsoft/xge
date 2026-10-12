# Footnote paragraph newline regression

The first line of a footnote body is a new parser input. Its printed label length must not determine logical Tab stops. Paragraph newline commands use parser-owned quote/list/task prefixes and retain untouched source bytes and hidden definitions.

Run `python tests/xui/build_document_footnote_newline_test.py --source-root . --out-dir <new-directory> --variant normal` and repeat with `--variant collision`. Windows uses GCC; Linux enables ASan/UBSan by default. Each invocation compiles all 29 kernel units in a new directory and never reuses an archive or object.

The two permanent drivers also run in the default `build_document_cst_test.py` normal/collision matrices; on Windows they run against the actual default DLL. The source oracle declares all before/after bytes independently: 600 scenarios covering the two public paragraph newline commands, LF/CRLF/CR, BOM/EOF, root/plain/quote/ordered/task contexts, Unicode labels and the pinned parser's 75/76-byte boundaries. Every footnote before and after must have proven footnote scope. The invalid 320-byte diagnostic is retained in the implementation evidence and is not counted as a footnote scenario.

The fault driver scans every allocation/cancellation checkpoint for four footnote contexts, two commands and optional continued input in the same transaction. Static kernels execute 32 scans; DLL consumers expose 16 allocation scans without internal cancellation hooks. Checks include source and semantic reparse equivalence, poisoned commit, same-document retry, old snapshots, stable untouched tail positions, saved state, a single Undo/Redo and allocator leak freedom. Kernel-level continued input is not a replacement for Editor, native window or physical IME validation.
