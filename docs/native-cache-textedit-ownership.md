# Native TextEdit cache ownership

This change is native XUI behavior, not a language-side forwarding layer.
`xuiTextEdit*` still calls the same native implementations directly. The native
creator-thread and outside-Ownership-Freeze destruction contracts still apply.

## Actual owners

The existing bounded 64-entry paint cache owns every admitted entry, including
partially produced entries. A failed or deferred cleanup never decrements its
count or clears a carrier that still owns paint/chunks. An entry becomes a
lookup hit only after its source, geometry and advance directory are complete.

In an Owned Context each entry retains its actual resolved Font once. The
Font reference is validated against the Context backend capability, enumerated
as one real ownership edge and released only after the entry's paint/chunks
have been consumed. Paint descriptors enumerate their own independent Font
references as well. Metadata aliases are not additional owning edges.

One unique `pWorkLayout` is the persistent owner for whole-layout and affected
wrapped-paragraph candidates. Production writes directly to this slot. A
failed candidate or a successful but busy paint remains there until checked
cleanup can consume it. When source mapping requires the layout for line
queries, ownership moves into the ordinary layout slot; no temporary stack
variable takes ownership. Identity projection can reuse the work slot and
owns its resolved Font key. After IME temporarily reuses the work layout,
normal direct-line metadata is marked dirty before it can be reused.

The document and IME layout slots, work slot and every counted paint/chunk entry
are included in the type's Trace/Ready/Prepare. While an IME render temporarily
swaps payload fields, the native activity reservation makes graph enumeration
fail closed; it does not pretend that borrowed stack aliases are graph roots.
Physical type destruction runs only after real preparation has consumed all
these owners.

## Editing and failure

Text replacement, deletion, insertion, selection movement and history apply
validate/reserve their input candidates before checked cache preparation and
before changing text. Incremental edits retain the previous affected-window
algorithm: only the edited physical line's small shapes or neighboring chunk
window are consumed. Untouched chunks remain the same actual owners. Prepared
chunks remain reserved until the matching patch commits or is canceled.

A pure wait preserves the carrier and text. Ordinary native APIs return BUSY;
true destruction preparation uses DEFERRED and keeps the Widget/Context owner.
A consuming cleanup failure remains a negative result even if the diagnostic
is NULL. It uses the existing Context cleanup receipt; it is not inferred from
BUSY, TLS presence, or a parent operation's error. Failed candidate entries
stay counted until a later real cleanup consumes them.

The same receiver rejects reentrant mutation while a provider is consuming or
building its geometry. Mutations and geometry getters keep one native activity
scope until their physical return tail. Borrowed scrollbar/model getters return
NULL when a callback has requested destruction, rather than returning a pointer
into storage that the scope tail is about to release.

## Measured costs and acceptance boundary

On the matched x64 GCC build, `xui_text_edit_data_t` changes from 25,016 to
25,048 bytes (+32); each paint entry remains 216 bytes because its validity bit
uses prior padding. One reusable native Layout has a 664-byte owning object,
plus its real source, geometry and paint allocations; it is bounded to one
work slot. This can retain a candidate after failure or retain reusable geometry
between measurements. It replaces transient candidate construction rather
than creating an unbounded retirement directory.

New bookkeeping includes a real Font retain/trace/release per admitted paint
entry, a same-receiver activity reservation, and native call scopes on mutators
and geometry getters. Full-cache retirement still scans the existing bounded
64 entries. Incremental edit preparation scans those entry keys and clears
only the original affected chunk window. No extra language box, opcode dispatch,
provider forwarding function, or per-entry allocation is added by this change.
There is no claim that this bookkeeping equals unmanaged pointer costs.

The focused CPU fixtures use the complete matched production object graph,
without an initialized engine/window/GPU. Windows and Linux O0/O2 cover actual
TextEdit construction, direct edits, retained Font graph count versus real RC,
first NULL cleanup failure, successful busy output followed by OOM, 16 allocation
failure budgets, deferred Context cleanup, and five native geometry getter
physical tails when the provider destroys the Widget. A 48-KiB edit and undo
check that at most three chunks are rebuilt. The pre-existing offscreen TextEdit
regression also covers keyboard/editing/find/selection and IME rendering.

Recorded candidate: `native-textedit-cache-overlay-r10`; Windows results:
`native-textedit-cache-focused-win-r10`; Linux results:
`native-textedit-cache-focused-linux-r5`. These are incremental native CPU
qualifications over the pinned WidgetCache r2 shared layer, not a fresh final
shipping DLL, new ambient-error semantics, or all-XUI completion claim.
