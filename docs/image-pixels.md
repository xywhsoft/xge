# Read-only Image pixels

The legacy xgeImageGetPixels returns a borrowed address and promises read-only
access. The checked xgeImageGetPixelView adds its byte extent as an xbytesview;
xgeImageCopyPixels copies the same span into caller-sized writable storage.
RGBA8 and A8 use positive dimensions, a row no larger than INT_MAX and a stride
large enough for that row. Nonpositive stride means tight rows. Required bytes
are `(height-1)*normalizedStride+rowBytes`, not stride*height: no padding after
the final row is assumed. Inter-row padding is part of the contiguous span.
Planar/unknown formats return UNSUPPORTED rather than guessing plane extents.

For raw C images, the caller provides a valid initialized backing through the
last active row and exclusive access; the struct has no allocator-provenance or
allocation-length field. This query does not certify arbitrary foreign pointer
capacity. Native decoded Images supply valid tight RGBA storage. The borrowed
view ends at release/mutation. Neither operation changes metadata or alpha flags.
The copy validates the complete capacity before writing; failure preserves the
destination. Overlap of valid writable output and source has memmove semantics.
There is no color/alpha conversion, allocation or XRT call in these operations.

The language binding can copy a borrowed view within its native call scope into
an independent bytes snapshot. That snapshot allocates/copies once and survives
image mutation/close. A Buffer destination uses a direct span and one native copy,
without a temporary bytes/Buffer. A read-only borrow in C is not a zero-copy bytes
value in the language; those costs must not be conflated. The legacy API's mutable
void-pointer type does not add a safe mutable-image contract.

Independent zero-copy owning pixel views are not implemented here. The current
Image exclusively owns its allocation; sharing backing storage versus expiring
views on mutation/close requires a separate lifetime rule. Neither read-only
snapshot nor caller-buffer copy is claimed to be that missing owning view.

Pixel access is an independent static-archive member, separate from image decode,
PNG, alpha conversion, XRT implementation and graphics. Native O0/O2 C11/C++11/
C++17 clients exercise exact bytes, missing final padding, A8, stride defaults,
short/null output, unsupported format, invalid geometry, overlap and preservation.
Optional Linux sanitizer checks instrument this pure component/client, not an
entire engine or decoder. Matched engine/ABI and language consumption have their
own receipts; declaration presence is not runtime qualification.
