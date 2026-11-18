# Owned 3D particle billboards

The batch stores numeric billboard geometry in one immutable allocation.
Submission reuses the persistent native queue and retains its texture
references. The legacy caller-buffer API continues to export the same data.

Managed renderers and textures reserve the graphics lifetime. An explicit
engine shutdown with live owners returns Busy. Sokol's window shutdown can
reclaim the device before language callbacks reach a cleanup safe point.
On the same creator thread, checked release then consumes the remaining
CPU references and leases without issuing GL calls to the ended context.

The Windows and Linux tests draw real textured billboards through both SDK
linkage modes at O0 and O2. They compare the old and new queue pixels, cover
snapshot immutability and allocation failure, check the queue after a failed
replacement and release retained handles after window teardown. The same
warmed queue passes repeated legacy and batch submissions with allocation
failure enabled.
