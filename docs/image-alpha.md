# Image alpha processing

`xgeImagePremultiply` is the legacy raw RGBA8 operation. Every invocation
multiplies RGB by alpha again using `(channel * alpha + 127) / 255`; alpha is
unchanged. It clears `XGE_IMAGE_STRAIGHT_ALPHA`, sets `XGE_IMAGE_PREMULTIPLIED`
and preserves other flag bits. Image decoding still invokes it exactly once
when premultiplied output is requested. The state flag is not an idempotence
guard for this legacy function.

The previous implementation flattened `width * height` into a signed integer
and ignored stride. An ordinary 2x2 RGBA image with a 12-byte row stride therefore
modified four padding bytes and left its final pixel unconverted. The actual
previous Windows/Linux host SDKs reproduce this mismatch on valid local storage.

The implementation now uses the shared checked row-layout calculation before
writing any pixel. A nonpositive stride retains the tight-row default; a
positive stride must fit a complete RGBA row. It leaves row padding unchanged
and needs no padding after the final row. NULL, another pixel format, invalid
dimensions, an unrepresentable row width or a short positive stride is a no-op.
The C caller supplies the writable storage described by the geometry and must
exclude concurrent mutation. This is not arbitrary C allocation validation.

The pixel loop is a pure component in `src/xge/text/xge_image_alpha.c`. Tight
rows keep one flat loop; padded rows are processed separately. It allocates
nothing and has no XRT, engine, graphics or thread-state calls. Distribution
compiler stack-protector hooks may still appear in its object file.

Ordinary single-file builds include the component in `xge_impl.c`. The static
SDK defines the private `XGE_IMAGE_ALPHA_SEPARATE_TU=1` switch and gives it a
separate archive member. This lets a pure alpha client select that member
without pulling unrelated undefined references from the engine aggregate.
The static SDK's remaining system/runtime dependencies and ordinary XRT
ownership contract still apply when those components are used.

`tools/test/check_image_alpha.py` checks padded/final rows, tight defaults,
rounding, invalid geometry and repeated raw conversion at O0/O2, including
C11/C++11/C++17 clients and optional Linux UBSan. Its standalone object and
program import checks identify the actual dependencies. The matched host/static
clients are exercised separately by `check_image_alpha_sdk.py`; existing PNG
and language image gates cover decoding/encoding integration.

`xgeImagePremultiplyChecked` supplies state-aware conversion. It validates
storage, RGBA8 geometry/stride and contradictory alpha flags before writing;
straight or unmarked input is converted once, and valid premultiplied input
succeeds unchanged. Other flag bits survive. The status function uses the same
pixel implementation, with no extra allocation or XRT call. Its error statuses
are `INVALID_ARGUMENT` for invalid storage/layout/alpha state and `UNSUPPORTED`
for another format. The existing void operation retains repeat semantics.

The new xlang `Image.premultiply()` declaration calls the checked operation
directly, with the existing owner/thread/error scope. Idempotence is the chosen
default for this new, unpublished interface; the optional user preference can
still refine it. These CPU tests do not qualify a graphical backend or promise
that common language lifecycle/error checks have zero cost.

The qualified Windows/Linux CPU batch records 18/20 standalone commands,
18 public host/static client commands on each platform, and 20/22 PNG
regression commands. The old host SDKs both reproduce the padded-row mismatch
with exit 83 on an ordinary in-bounds buffer. The small index is
`docs/validation/image-alpha-20261114.json`; raw logs, binaries and SDKs remain
external. A metadata-only static-builder successor reuses 164 objects and
rebuilds its generated contract record; its public clients pass separately.
