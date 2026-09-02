# ShapeEx paint-order example

This pure-C example renders open and explicitly closed paths with both fill-first
and stroke-first paint order.

Build from the repository root:

```bat
examples\xge_shape_ex_paint_order\build.bat
```

Run the executable without arguments for the interactive preview. Press Escape
or close the window to exit. Use `--frames N` for a bounded run and `--capture
PATH` to save the atlas and exit. An output PNG path as the first argument is
also accepted for existing capture scripts.
The checked-in `reference.png` is the current XGE output.
