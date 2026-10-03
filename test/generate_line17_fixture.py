"""Own Unicode 17 line-layout outlines with direct PUA glyph aliases.

This fixture has no GSUB/GPOS. All non-space glyphs have distinct outlines
and advance 500/1000 em. No installed fonts or browser are required.
"""
from pathlib import Path
import sys
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen

points = [32, 45, 0x2010, 97, 98, 0x4e2d]
order = ['.notdef'] + ['u%04x' % cp for cp in points]
cmap = {cp: order[i + 1] for i, cp in enumerate(points)}
cmap.update({0xe900 + i: name for i, name in enumerate(order) if i})
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap(cmap)
glyphs = {}
for i, name in enumerate(order):
    pen = TTGlyphPen(None)
    if i != 1:
        pen.moveTo((30, 0)); pen.lineTo((400, 0))
        pen.lineTo((400, 250 + i * 35)); pen.lineTo((30, 250 + i * 35)); pen.closePath()
    glyphs[name] = pen.glyph()
builder.setupGlyf(glyphs)
builder.setupHorizontalMetrics({name: (500, 0) for name in order})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({'familyName': 'XGE Line17', 'styleName': 'Regular',
                       'uniqueFontIdentifier': 'XGE-Line17-1', 'fullName': 'XGE Line17',
                       'psName': 'XGELine17'})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
builder.font.recalcTimestamp = False
builder.font['head'].created = builder.font['head'].modified = 3400000000
target = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).parent / 'data/xge_line17_fixture.ttf'
target.parent.mkdir(parents=True, exist_ok=True)
builder.save(target)
print(target)
