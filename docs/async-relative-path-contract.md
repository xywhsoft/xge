# Relative file probes and worker diagnostics

A relative file read tries the application directory, then the working
directory. A failed first probe must not leave a diagnostic on an otherwise
successful fallback. The owned async worker captures its real final diagnostic
before returning; propagating a speculative file error made a successfully
decoded image appear Failed.

The common file helper now clears the non-memory probe error immediately before
the final read. Its existing memory-error exits remain terminal. The final read
keeps its own diagnostic, so genuinely missing files are still checked failures.
No decoder, thread, C ABI, result storage or language wrapper changes are needed.

The regression executable lives outside its working directory. A small TGA exists
only in that working directory: the application-directory read must fail, the
fallback must return a Ready owned image with exact RGBA bytes and no stored
error. A second process checks that a real missing file remains Failed with its
original diagnostic. Both check exact heap balance after physical worker return.

The old r21 engine fails the success case at O0/O2 on both Windows and Linux;
the missing-file controls pass. The fixed r22 engine passes all six build/run
checks per platform. The existing raw and owned async suites also pass on both
platforms: 136 native checks including the new relative-path gate. Tests use local
files only and do not open a graphics device or network connection.

See `docs/validation/async-owned-20261110.json` for matching source/build/report
hashes, failed baseline reports and the separate language integration status.
This fix does not complete Font/Sound loading, backend admission or SDK delivery.
