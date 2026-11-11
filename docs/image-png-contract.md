# PNG bounds, I/O and allocation failures

PNG file and memory encoders share checked dimensions, stride and flag
validation before reading pixels or opening an output file. The filtered input
is bounded to 256 MiB, matching the existing memory encoder. Width also keeps
stb's signed filter-score sum in range. Nonpositive stride means tightly packed
RGBA8. A strided source needs no padding after the final row.

Premultiplied input is converted into tight temporary rows; padding is neither
copied nor read. Straight input is passed directly to the encoder. Encoded
memory is still allocated by the sole XRT instance and released by xrtFree.
The native pixel/stride APIs remain caller-capacity contracts; language region
declarations additionally validate the backing storage before native entry.

File output checks both callback short writes and fclose, including buffered
flush failures. Failure is a negative native status. This is ordinary file
writing, not an atomic replacement: an I/O or encoding failure can leave a
partial file, and callers must decide how to handle it.

stb's supported `STBIW_ZLIB_COMPRESS` seam uses the shared XRT Deflate codec.
No second runtime or compression implementation is introduced. The built-in
stretchy-buffer compressor is omitted because its allocation failure can
assert or dereference failed growth. Its quality input bounds hash candidates,
rather than naming a zlib level. Default quality 8 maps to XRT level 4's 16
probes with lazy matching; larger budgets select the corresponding higher
levels. Mapping 8 directly to level 8's 512 probes caused a measured slowdown
on noise and was rejected. PNG compressed bytes can change while decoded RGBA
remains lossless. The XGE XRT module profile explicitly includes Deflate.

Two small vendor changes are deliberate and recorded in the source diff:

- Free the compressed zlib buffer when PNG output allocation fails.
- Reject buffered non-JPEG signatures before allocating a JPEG info decoder.
  The precheck consumes no bytes or callbacks; a short initial buffer keeps the
  original parser path. Actual JPEG info keeps its existing decoder and cost.

PNG info can therefore run without an XRT allocation. This is not a promise
that metadata queries for every supported format allocate nothing. Parsing
dimensions also does not certify the complete image payload.

`tools/test/check_image_png.py` tests actual matched host/static products with
strict C11/C++11/C++17 clients. A protected-page source checks final-row reads;
pixel decode checks straight/premultiplied output. Allocation prefixes require
exact live heap and invalid/double-free counter equality. An aggregate fixture
injects short fwrite and failed fclose; Linux additionally uses /dev/full and
optional aggregate UBSan. No graphics context or device is needed. Separate
native samples compare codec bytes, CPU clock and allocation counts; they do
not establish language or isolated hardware performance.

The qualified source/build/evidence index belongs in
`docs/validation/image-png-20261112.json`. Raw builds, failure controls, logs and
samples stay outside the repository. This batch is an XGE prerequisite for
language image APIs; it does not mark full XGE integration or release delivery
complete.
