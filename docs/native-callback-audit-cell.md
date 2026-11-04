# Native owned Cell design and acceptance boundary

This design is approved for the remaining callback work. It is not a completed
binding or a completed TableView migration. The existing raw C ABI remains an
explicit caller-owned interface; an opaque pointer is never a language value.

## Owning record

`xui_table_view_cell_output_t` keeps scalar/style fields directly accessible.
Private state owns UTF-8 text, tooltip, one genuine `xvalue`, and two exactly typed
callbacks with independent userdata and ownership descriptors. None of the owned
storage fields is a writable language field. Text setters clone/replace and
reject embedded NUL while native display consumers still require C text.

Use `Init` for fresh automatic storage and `Create` for a stable heap record.
Public
retention cannot turn an automatic callback output into a persistent object.
Native reference count, lifetime watch, quiescence, checked release and complete
Trace account for real value/callback/source references. Call scopes pin physical
storage across reentrant hooks and preserve errors through final cleanup.

## Reusable cache

`output.Use(cachedCell)` retains a read pin on one canonical heap source and
copies scalar/style values. Existing source chains are flattened, so there is no
chain of adapters or delayed lookup. Text, value and callback data stay in the
source until consumption ends. SetText, SetTooltip, SetValue, callback replacement,
Reset and Use on a read-pinned source return `XUI_ERROR_BUSY`. An external
reference may be released during a pin; physical destruction waits for real pins.
Direct scalar/style projections are snapshots; private payloads cannot be changed
through those projections.

The native renderer creates its output record on the C stack and disposes it at
the actual last consumer. There is no automatic heap Cell object per query. An
explicitly created independent Cell can be reused by GetCellOwned callers.

## Callbacks and text

A dynamic formatter returns an owned `xstrview` using the existing owned_text
contract. The result is XRT-compatible allocated C text with a terminator at Size;
the actual consumer validates and uses it, frees it, then drops the callback
snapshot. Widget/DrawContext/output parameters remain callback-period views.
Cached formatters and renderers are installed once and keep separate owners.
Hold/Drop never run under an ownership mutation or publisher lock.

All native use of widgets, outputs and draw data stays inside a Context operation
until output disposal and snapshot release finish. Reentrant close cannot free
those physical records early. A callback must return normally across the C ABI.

The language does not gain a string/bytes retained-span facility in this change.
SetText clone is the safe baseline; Use is the no-clone cache path. No temporary
xvalue box stands in for a string ownership descriptor.

## Consumer migration

The owned provider receives a callback-period CellOutput view. GetCellOwned fills
an independent caller-owned record transactionally. Internal merge, accessibility,
selection, formatting and drawing retain a real output until each last use; no
GetCell helper may dispose it before a later formatter/render call.

Legacy pValue remains opaque C data. Owned values use xvalue retain/clone semantics
and expose that actual edge to the ownership graph. Legacy per-cell raw callbacks
are not silently promoted to managed callbacks. Raw and owned adapter modes must
preserve the existing raw registration failure/rollback contract, including
reentrant replacement, without restoring fn/user fields after their owners drop.

## Required checks

- Scalar, text, tooltip and actual value round trips; alias replacement and NUL.
- Independent callback owners, exact signatures, graph edges and cycle cleanup.
- Every allocation failure around registration, copying and publication.
- Owner Hold reentry, self replacement, close, false Drop without a diagnostic.
- Use pins: setter BUSY, release last external reference, reuse after unpin,
  flattening, no automatic-storage escape, and no ownership cycles from Use.
- Real formatter/render consumers free text before Drop and keep Context busy.
- Cached Use performs zero text copies and zero per-cell heap allocations.
- Existing raw TableView/TableGrid and provider lifetime tests remain meaningful.

Current implementation status: Native Cell and TableView consumers pass Windows
GCC O0/O2 clients against the O2 host engine. The existing TableView suite and
996-case provider-lifetime suite also pass. Evidence: native-callback-table-win-r7
and native-callback-table-cell-win-r1 under the task validation directory.
Linux Native Cell/TableView and the existing regression suites also pass under
native-callback-table-linux-r1 and native-callback-table-cell-linux-r1.
Managed language/domain integration and TableGrid remain pending. Raw
column/global/merge renderer callbacks still require their separate Owned slots;
the old raw C provider ABI does not become a managed callback automatically.

Native value data and displayed text remain independent, as in the raw C cell.
A formatter can inspect the real value via GetValue; SetValue does not stringify
or box its input. Long-lived managed Cells must use Create/Release and the exact
native domain. Init/Dispose are for fresh automatic C records and must not be
projected as methods that can overwrite an already initialized managed Cell.
