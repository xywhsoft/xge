# Owned Document selection geometry

The existing selection layout pass also serves the owned rectangle result.
The result carries all rectangles in document coordinates and survives the
renderer, context and snapshot. Adjacent fragments still merge exactly as
in the caller-buffer API.

The native result grows its buffer as needed. Publication does not copy the
rectangle array. The renderer keeps the snapshot allocator alive through the
call tail and discards unpublished geometry when a measurement callback
requests closure or the tail otherwise fails.

The CPU fixture compares 20 rectangles and the legacy prefix, checks a
collapsed and a stale range, verifies retained geometry after the source
objects close and exercises four allocation failure prefixes. It also checks
that a close request from the measurement callback leaves no result allocation.
Windows and Linux SDK clients build at O0 and O2 with hosted and static linkage.
