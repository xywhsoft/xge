# Sized Texture operations and owned factories

`xgeTextureReadPixelsSized`, `xgeTextureUpdateRGBASized` and
`xgeTextureUpdateYUV420PSized` accept actual capacities. They use the production
byte-row helper before accessing pixels, including every YUV source plane.
Required bytes are `(height-1)*stride + rowBytes`; final-row padding is not
required. Nonpositive stride retains the native tight-row default. C callers
must provide valid ranges disjoint from private texture storage. The raw
legacy operations now share these implementations with unbounded capacity.

Failed size/stride validation leaves input/output pixels unchanged. Readback
uses the CPU shadow; live render-target pixels must use RenderTarget readback.
Driver errors can leave partially changed pixels; GL binding and unpack
alignment are restored, but pixel updates and sampler parameters are not
transactional GPU operations. YUV drawing keeps the existing full-range
conversion; this work does not change it to video-range 16..235.

Owned RGBA/image/path/memory factories now expose explicit creation/decode
flags. The old defaults remain lossless RGBA creation and premultiplied image
decode. FromImage copies native pixels before returning. FallbackGetOwned
creates an independent owned texture. Fallback replacement validates capacity,
prepares a candidate, then publishes it and relocates its upload-queue node;
allocation failure preserves the old fallback.

`xgeFontLoadSystemOwned` publishes a real owned font pointer, not caller
storage. It reserves an engine obligation before platform lookup/allocation,
rolls back on failure, and participates in checked Busy shutdown.

Both platforms passed 44 native checks at O0/O2, including platform system
fonts, actual OpenGL, odd-sized YUV planes, exact/padded capacities, unchanged
invalid outputs, fallback replacement and OOM rollback. Frozen builds and
reports are pinned in `validation/texture-sized-20261110.json`. Linux graphics
uses Xvfb. Image-decoder allocation fixes have their own focused report.

These methods do not add native per-draw locks or claim cross-thread safety.
Native C mutation/release remains subject to the engine creator-thread
contract; language-side rejection/retirement must be closed separately. No
release package, hardware performance result or full-ecosystem completion is
implied by these native checks.
