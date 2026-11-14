# Owned hotkey and Context error callbacks

`xuiHotKeyRegisterOwned` and `xuiSetErrorCallbackOwned` retain one callback
owner before publication. Candidate Hold may reenter, replace another slot or
close Context; publication rechecks live state and leaves the previous slot
untouched on failed admission. A false/fresh-diagnostic Hold rolls back every
actually acquired reference. Raw and command replacement consume prior owned
registrations too. The legacy raw APIs keep borrowed callback semantics.

Each owned registration has a small independent record. Context/Widget slots
remain direct typed function/user fields. Hotkey dispatch pins all selected
records before the first callback; unregister/replace/clear cannot release an
in-flight callback. This uses native record references rather than calling the
user owner Hold again during dispatch. Both function and payload/code remain
alive. A NULL callback still selects the existing Widget HOTKEY event route;
use Unregister for removal. Bulk clear detaches its whole selected set before
arbitrary owner Drop can reenter. Registrations created by that Drop survive.
Widget destruction clears its subtree and Context destruction consumes all.

Context ownership tracing visits each actual registered owner. An active
Context operation refuses quiescent retirement; in-flight snapshot references
are released before its operation ends. Owned Contexts admit only their Proxy
creator thread. Callback records add no forwarder, opcode dispatcher, language
box, atomic thread counter or per-hot-callback owner Hold.

The error observer snapshots its typed callback/user/record, suppresses recursive
reporting, and retains callback/Context through self-replacement and logical
close. Its business diagnostic remains visible and independently owned by the
receipt; fresh observer/cleanup failure is checked. Primary status/diagnostic,
including failure without an allocated diagnostic, survives consuming cleanup.
Physical teardown disables observation before consuming arbitrary callback
owners. The separately committed pending-pump close check stops later commands.

Error info and event pointers are borrowed for the callback. Native C producers
must supply valid pointer fields for that duration. The language binding needs
callback epoch views and independent text snapshots; native completion is not
language/compiler/package delivery. These callbacks report framework-issued
notifications, not every possible checked API failure.

## Measured costs

On Windows x64, Context grows from 147704 to 147840 bytes (+136), hotkey entries
from 56 to 64 bytes, and each owned registration record is 48 bytes. No native
record is allocated for raw callbacks or command-only hotkeys. At most eight
selected hotkeys use the existing stack snapshot; larger selections use one
native temporary snapshot. Raw record slots have an eight-byte pointer cost.

Both platforms measure 1000 prepared replacements: owned hotkey/error each have
1000 allocations/frees, raw equivalents zero; no reallocations. This is native
ownership storage, not a language bridge measurement. 1000 prepared hotkey plus
error-report rounds have zero XRT heap operations and zero additional owner Hold
calls. Registration-time acquisition and ordinary Context/graph/error checks
still have costs; no CPU equivalence or whole-binding zero-overhead claim.

`check_owned_input_callbacks.py` links actual hosted/static SDKs at O0/O2 and
also runs unchanged input routing. Cases cover self-removal, replace/raw/command
handoff, selected-peer clear, owner-Drop reentry, active Widget/Context close,
recursive errors, ambient diagnostics, callback/drop failure, registration OOM,
17-way snapshot allocation failure, real-record reference-overflow injection,
allocation-free bulk clear, graph edges and exact cleanup. The private-header
case only changes the reference counter in a real native registration; it does
not fake Context or replace implementation. Static header helper warnings are
excluded from unused-function diagnostics. Existing text read/edit/CodeEdit/
TextEdit regression clients run separately. See the small validation index.
