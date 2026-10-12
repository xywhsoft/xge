"""Project-owned deterministic GSUB backtrack/lookahead and ligature font."""
from pathlib import Path
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

order = ['.notdef', 'space', 'a', 'b', 'a.context', 'b.context', 'f', 'i', 'fi']
advances = [600, 250, 400, 300, 800, 900, 600, 250, 750]
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap({32: 'space', 97: 'a', 98: 'b', 102: 'f', 105: 'i',
                          **{0xe400 + i: name for i, name in enumerate(order) if i}})
glyphs = {}
for index, name in enumerate(order):
    pen = TTGlyphPen(None)
    if name != 'space':
        right = advances[index] - 60
        pen.moveTo((20, 0)); pen.lineTo((right, 0)); pen.lineTo((right, 300 + 30 * index))
        pen.lineTo((20, 300 + 30 * index)); pen.closePath()
    glyphs[name] = pen.glyph()
builder.setupGlyf(glyphs)
builder.setupHorizontalMetrics({name: (advances[i], 0) for i, name in enumerate(order)})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({'familyName': 'XGE ASCII Context Fixture', 'styleName': 'Regular',
                       'uniqueFontIdentifier': 'XGE-ASCII-Context-Fixture-1',
                       'fullName': 'XGE ASCII Context Fixture', 'psName': 'XGEASCIIContextFixture'})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
addOpenTypeFeaturesFromString(builder.font, '''
languagesystem DFLT dflt;
languagesystem latn dflt;
lookup AContext { sub a by a.context; } AContext;
lookup BContext { sub b by b.context; } BContext;
feature calt {
    sub a' lookup AContext space b;
    sub a.context space b' lookup BContext;
} calt;
feature liga { sub f i by fi; } liga;
''')
builder.font.recalcTimestamp = False
builder.font['head'].created = builder.font['head'].modified = 3400000000
path = Path(__file__).resolve().parent / 'data' / 'xge_ascii_context_fixture.ttf'
builder.save(path)
print(f'Generated {path.name}: project-owned outlines and GSUB, fixed timestamps')
