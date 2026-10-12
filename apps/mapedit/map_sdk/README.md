# XGE map format

`xge_map.h` is the shared C ownership and XSON I/O boundary for the editor and
runtime. Version 2 adds `format`, `version`, `layerCount`, `tileWidth`, and
`tileHeight` while retaining the original field names for V1 compatibility.

The reader accepts V1 files without a version. It infers a 1-8 layer count when
the serialized tile count is an exact multiple of the validated cell count.
The writer always emits V2 and uses XRT's atomic file replacement.

Dimensions are limited to 2048 per axis, 1,048,576 cells, and eight layers.
All multiplications are checked before allocation. Unsupported future versions
and inconsistent tile arrays are rejected without replacing the live document.
V2 requires integer schema fields and a declared `layerCount`; a shorter tile
array is a supported sparse encoding whose omitted trailing cells are zero.
Passage data must be an array and cell custom data must be an object.
