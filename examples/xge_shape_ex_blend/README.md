# ShapeEx blend atlas

This example renders all 17 ShapeEx blend modes in a fixed 4 by 5 atlas.
Each cell contains the same translucent destination and source rectangles so
color and alpha composition can be inspected directly.

The cells are ordered left-to-right, top-to-bottom:

1. normal
2. multiply
3. screen
4. overlay
5. darken
6. lighten
7. color-dodge
8. color-burn
9. hard-light
10. soft-light
11. difference
12. exclusion
13. hue
14. saturation
15. color
16. luminosity
17. add

Build from the repository root with:

```bat
examples\xge_shape_ex_blend\build.bat
```

Run `build\xge_shape_ex_blend.exe` without arguments for the interactive preview.
Press Escape or close the window to exit. Use `--frames N` for a bounded run and
`--capture PATH` to save the atlas and exit. A PNG path as the first argument is
also accepted for existing capture scripts.
The checked-in `reference.png` is the current XGE output.
