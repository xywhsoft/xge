# Image decoder allocation failures

The existing vendored stb_image v2.19 dereferenced failed JPEG test/load/info
allocations and the GIF info allocation. The real XGE Texture OOM test
reproduced the JPEG-test crash while loading a PNG, before PNG detection.
Four allocation checks now return the decoder's existing failure result.

GIF first-frame setup also checked only its output allocation, then wrote to
unallocated background/history buffers. Setup now checks and releases all three
buffers. First-frame decode failure releases the output pixels as well; successful
conversion retains its original ownership behavior. These are local changes to
the existing dependency, not an upstream version upgrade.

`tools/test/check_stb_image_oom.py` compiles the actual dependency header and tests
its public memory load/info interfaces with PNG, truncated JPEG, GIF and truncated
GIF, 20 deterministic allocation budgets, O0/O2, and live allocation accounting.
Windows and Linux pass; Linux additionally uses ASan/UBSan. Normal PNG pixels are
checked exactly. Evidence is in `validation/stb-image-oom-20261110.json`.

The complete decoder, animated multi-frame loading and every image format have
not been audited by this focused fix. No networking or device test is run.
