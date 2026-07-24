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

Pass a PNG path as the first argument to capture the atlas.
The checked-in `reference.png` is the current XGE output.
