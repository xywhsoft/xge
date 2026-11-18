# Owned offscreen targets and mixed billboards

The graphics reservation lasts through the final target reference, including
color views and the queue entries that retain them. Resize replaces storage
after successful allocation. A failed resize preserves previous dimensions
and contents. The renderer rejects feedback before touching the target.

The caller-buffer readback supports packed and padded top-down RGBA8 rows.
Occupied host GL pixel pack state is temporarily cleared and restored.
The fixture checks actual pixel equality between legacy and mixed submission,
row padding and state restoration with a real pack buffer, failed allocations,
texture view retention after resize and checked release after window teardown.
Explicit engine shutdown is Busy while the managed leases still exist.

Both platforms passed hosted and static SDK tests at O0 and O2 and the
previous particle regression at O2. C11 and C++ public-header probes passed.
Dynamic-disabled source checks also built both affected units and confirmed
the checked Unsupported entry remains available.
