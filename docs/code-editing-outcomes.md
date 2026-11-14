# Composite editing outcomes

`xuiCodeEditingIndentSelection`, `xuiCodeEditingOutdentSelection` and
`xuiCodeEditingToggleLineComment` used to discard the result of
`xuiCodeDocumentEndEdit`. EndEdit can notify an owned callback and fail without
an XRT diagnostic, or finish a callback that closes the document. Continuing
with `xuiCodeSelectionSetRange` then hid the failure and could read a document
whose physical lifetime had ended.

The shared private completion helper now returns EndEdit failure before any
later selection/document access. If the edit body failed first, it saves that
diagnostic, including NULL, completes the undo group, discards a later cleanup
diagnostic and returns the original status. The successful path still updates
the selection as before. There is no public ABI, owner or threading change.

An error does **not** undo edits already committed to the document. The native
undo group is closed on body failure. A failed notification leaves the original
selection state; callers must inspect the current document rather than assume
atomic rollback. This matches the existing single-edit command behavior and
does not add a public partial-commit carrier.

`tools/test/check_code_editing_outcome.py` links the actual hosted/static engine
and one complete XRT to O0/O2 clients on Windows/Linux. For each compound command
it checks no-diagnostic owner Drop failure, retained first diagnostic, callback
close with physical lifetime invalidation, and normal grouped text/undo state.
Forty allocation budgets per command also force a partially committed first
line followed by body OOM and a later EndEdit failure; the earlier failure wins.
The old static archive precisely reproduces status 0 instead of callback failure.
Old-close execution is not used as a safe UAF reproduction or a sanitizer claim.

There are 48 positive outcome build/run steps and 16 existing CodeDocument
callback/history/ownership regression steps across the two platforms. Four old
archive builds and four expected status-defect rejections are recorded
separately. The matched SDKs stay O2 and reuse qualified objects; they are not
whole-engine source rebuilds. Exact reports, source hashes and size deltas are
indexed in `docs/validation/code-editing-outcomes-20261114.json`.

The private helper adds no allocation. It does real first-error ownership work
on a failed body and checks EndEdit status on success. The same-O2 host file
length and BSS are unchanged on both platforms; text grows by 128 bytes each.
This is not a CPU-equivalence claim. UI interaction, unrelated user XUI changes,
other callback-bearing models and final ecosystem delivery remain separate.
