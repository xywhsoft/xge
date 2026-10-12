# Footnote newline Editor regression

Run `python tests/xui/build_document_footnote_newline_editor_test.py --source-root . --build-dir <matching-shared-library-directory> --out-dir <new-directory>` on Windows or Linux.

The 168 proxy cases use four footnote contexts, Unicode/76-byte labels and all three line endings with BOM. Two public paragraph commands are observed in SOURCE/LIVE/VISUAL (144 cases); VISUAL input adds 24 Enter cases and continued input. For ordered/task ancestors, VISUAL collapsed single-newline input splits a list item; independently declared goldens retain left order/check state, advance the ordered marker and create an unchecked task. The public commands' paragraph goldens remain unchanged.

Every case verifies exact independent source, semantic/full reparse equivalence, proven Footnote scope, valid selection, immutable old snapshot, common Undo/Redo and three-mode round trips. Continued programmatic input is a separate history unit. This entry requires an actual matching DLL/so and generated build configuration. It does not stand in for native windows, physical IME, readers or scale performance.
