"""Deterministic cmap subsets of our own font for real shaping fallback tests."""
from pathlib import Path
from fontTools.ttLib import TTFont

directory = Path(__file__).parent / "data"
original = directory / "xge_opentype_fixture.ttf"
subsets = {
    "no_mark": {0x301, 0x5b0},
    "no_base": {ord("a"), 0x5d0},
    "no_i": {ord("i"), 0x3bb, 0x5d1},
    "no_x_mark": {ord("x"), 0x301, 0x5b0},
    "mark_only": {32,65,86,97,102,105,120,121,0x3bb,0x3bc,0x5d0,0x5d1,0x5d2},
    "composed": {0x301,0x5b0},
}
for name, removed in subsets.items():
    with TTFont(original, recalcTimestamp=False) as font:
        for cmap in font["cmap"].tables:
            if cmap.isUnicode():
                for cp in removed:
                    cmap.cmap.pop(cp, None)
                if name == "composed":
                    cmap.cmap[0xe1] = "a"
        target = directory / f"xge_fallback_{name}.ttf"
        font.save(target)
        print(target)
