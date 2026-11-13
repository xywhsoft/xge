# Owned custom Icon callbacks

`xuiIconAddCustomOwned` registers one immutable group with four independent
callback, user-data and owner slots. Draw is required; measure, prepare and
destroy are optional. An absent callback cannot carry an owner. Raw C user
data with no owner retains the caller's ordinary lifetime obligations.

The factory copies and acquires every candidate owner before publishing the
Icon. A failed Hold, diagnostic, allocation or reentrant Context/category
retirement rolls back the acquired prefix. Pre-publication failure never
invokes the business destroy callback. Post-publication retirement invokes it
once before consuming the four owners. Failed consuming Drops are not retried.
Status and nullable diagnostic are preserved independently. A native checked
cleanup failure can accompany a non-NULL partial result, which the caller must
remove; an Icon physically retired before return is reported as NULL with
INVALID_STATE. The language factory performs partial-result rollback.

Existing Icon operation pins protect the complete immutable group through
measure, prepare and draw return tails, including unregistration and Context
close inside the callback. Destroy uses cleanup admission which permits a
closing Context. Callback arguments remain temporary borrowed views.

The Context ownership graph visits all actual owner slots, including repeated
references to the same owner and legacy Contexts without an owned Proxy.
Registry aliases do not duplicate those group slots.

On the qualified Windows/Linux x64 ABIs, Icon remains 312 bytes and Category
272 bytes, equal to the previous implementation. The private descriptor reuses
its normalized size/user fields to distinguish the owned group; ordinary Icons
do not grow. A custom group requests 160 bytes (four 40-byte slots) once at
registration. Callbacks call the original function pointers directly after a
private tag branch. They do not take four fresh owner references on each call.

Painter Surface/quad/text/nine-patch entry points now reject a Font or Surface
from another owned backend before interpreting its payload, including no-op
drawing cases. Legacy raw Context behavior is preserved. Owned resource calls
add canonical Proxy/capability lookup and readiness checks; there is no added
resource copy or retain. This is a native safety check shared by C and xlang.

`tools/test/check_icon_custom_owned.py` runs O0/O2 clients against the explicit
O2 host engine using one XRT runtime. It includes 108 operation/reentry/error/
alias combinations, independent owner Hold/Drop failures, registration OOM,
post-publication queued retirement, graph edges and allocation-free native hot
calls. The Painter regression uses real native owned resources with a CPU
backend. Controlled baseline exits occur before foreign payload use. See the
small validation receipt for pinned external reports and source hashes.

This batch does not implement active cache-reader snapshots, manual Painter
session ownership, generic callback/module-cycle collection or real GPU
backend acceptance. It does not claim equal language/C CPU time.
