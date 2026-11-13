# Standalone shared 3D decode quota

The existing quota implementation delegates to one XRT allocation budget. It
counts charged live XRT requests, including parser/scratch/retained blocks
inside a bounded load. It is not RSS or VRAM accounting. Pool backing,
allocator/OS/driver overhead and external allocator callbacks remain excluded.
Reallocation reserves old and new blocks until the old block is released.

xge3dDecodeBudgetShareOwned(budget,&out) acquires one independent native
reference. Failure clears out; success transfers the reference to the caller.
Release it with xge3dDecodeBudgetFree. Native scopes, charged blocks and other
owners retain their own references; freeing one public owner does not cancel
them or reset the shared counters. The underlying lock/refcount/accounting
algorithm is unchanged.

Create/share/free/info/setLimit now live in xge3d_decode_budget.c outside the
glTF/model translation unit, and their public types/declarations are available
with 3D enabled even when MODEL is disabled. The model/clip load APIs retain
their existing feature guards. config/modules.json selects the new small
unit with XGE_ENABLE_3D. No parser, model or GPU dependency is introduced by
the quota unit.

tools/test/check_3d_decode_budget.py runs 25 checks per qualified platform.
O0/O2 clients use the O2 host for the new quota case and existing CPU hierarchy/
spatial tests. A second O0/O2 program links only the quota unit and supplied
XRT with MODEL/ANIMATION/ASYNC/CACHE off. Its object has no model/parser/GL
references, and its program imports no XGE engine. C11/C++11/C++17 headers
also compile in that configuration.

The native test checks independent ownership, real charged bytes, live
allocations, rejection counters, limit refusal and continued accounting after
both public owners close while a real XRT scope still retains the budget.
1000 set/info/share/free rounds perform no XRT debug-counted heap operations
under a zero-allocation budget. This does not include creation of the quota's
backing object and does not equate CPU time.

No window, GPU, model file, downloaded asset or network action runs in this
gate. Language model-loader integration, callback-bearing 3D resources,
shared-library initialization and final runtime delivery are separate work.
