# Native composite part ownership

## Contract

Internal component getters return borrowed widgets. `xuiWidgetDestroyChecked`
returns `XUI_ERROR_INVALID_STATE` for these widgets before changing their state,
parent/sibling chain, lifetime watches or current call scope. The same guard
applies to public detach, insert/reparent, overlay attach/detach and SetRoot.
The legacy void destroy reports the checked rejection through XRT error state.
A rejected operation leaves the part usable for normal queries/configuration.

Native constructors install the exact composite owner before the type init
chain. Private create/move/delete operations carry that owner explicitly; no
process, thread or reentrancy bypass flag exists. Public methods cannot inherit
an authority merely because they run inside a private operation.

The protected creation inventory is 97 part creations in 56 native helpers:
Panel, Accordion, Carousel, SplitLayout, Tabs, Window, DockPanel,
ScrollFrame/ScrollView, Popup/Menu, Canvas, ListView/TreeView/TableView,
TableGrid/PropertyGrid, Input/NumericInput/TagInput/ComboBox,
Cascader/ColorPicker/DatePicker/IconPicker, InventoryGrid, MessageList,
TimelineView, Terminal, TextEdit/CodeEdit and Workflow canvas. This follows the
existing public header's internal/borrowed part getter contracts, including the
explicit non-destroy/non-reparent notes on Panel, DockPanel and IconPicker.

Ordinary `Widget.GetChild`, user-added content, externally provided DockPanel
clients, external MenuBar menus and RadioGroup membership retain their existing
ownership/transfer rules. Independent MsgBox/FileDialog/Toast/controller roots
need their own real native owner protocol; this change does not invent a Widget
owner for them or claim that they are covered.

## Fixed slot lifetime

Each native Widget stores an owner identity and an optional fixed owner-data
slot. Automatic slot binding is limited to the exact owner's fixed TypeData
storage with bounds/alignment checks. Local creation outputs bind only when
published to a fixed owner field. Stack outputs and movable record/array
locations are never retained. Tabs/Accordion records that shift are deliberately
not stored as fixed slots.

Entering destroy-pending clears the widget's exact owner slot, including each
physical descendant. Requesting retirement of an off-tree popup disarms its slot
before the owner's TypeData can be freed. Physical destruction repeats the
idempotent clear. A replaced slot is cleared only if it still contains the old
widget, so delayed physical destruction cannot erase a replacement.
Private part deletion consequently uses constant-time exact owner validation;
it does not search the complete context, add a global lookup table, allocate a
weak node or perform per-frame/reference-count work.

The measured Win64 native Widget size is 1992 -> 2008 bytes (+16, +0.80%).
There are no new allocations, copies or reference-count operations per event or
paint. Creation has explicit private owner-aware factory/operation calls; those
cold-path calls are not described as free. Native metadata/data-layout changes
require matched engine/private headers, as with the rest of the private ABI.

## Acceptance

The new CPU-only real-engine case covers 34 protected getters, rejection during
Image type init and parent-change reentry, owner destruction during init,
queries/configuration after rejection, ordinary user child and RadioGroup
operations, 20 delayed off-tree retirements, fixed-slot replacement while the
old part is pending, and 50 allocation-failure budgets. Memory-debug live bytes,
live objects, invalid frees and double frees return to the baseline.

The guard cohort also runs 15 original Widget/composite suites and the existing
remaining/group/Tree/Table/Cell/Context callback suites at GCC O0/O2 against the
same O2 engine. The independent Table fix prevents raw adapter userdata from
being resurrected by raw -> Owned -> clear; its poison-user regression is part
of Table acceptance.

Reports are immutable external development evidence. Matching Windows/Linux
cohort paths and report checksums are recorded in the delivery provenance.
No window creation, renderer, GPU, audio device, network or security test is
required for this contract. The original SplitLayout input test uses logical
manual initialization and the actual queued native input pump, without Run.

## Date parse migration boundary

The original Grid date editor also reproduced a failure on the pre-guard native
baseline: its VB-style format was passed directly to XRT's current percent-token
parser; a successful fallback left the expected parse-mismatch diagnostic in
TLS and poisoned a later checked Table call. The port now translates the exact
DatePicker formatter token set, escapes literal percent, and isolates only that
speculative parse's diagnostic while restoring the caller's previous diagnostic
verbatim. Parse success is a boolean, so a legitimate epoch/zero timestamp is not
mistaken for failure. The actual legacy Grid regression and independent real-XRT
port tests cover this boundary. This is separate from rendering or field layout.
