"""Project-owned Hebrew GSUB context and ligature fixture with literal PUA aliases."""
from pathlib import Path
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

order = ['.notdef', 'space', 'aleph', 'bet', 'aleph.context', 'bet.context', 'gimel', 'dalet', 'gimeldalet', 'i']
advances = [600, 250, 400, 300, 800, 900, 600, 250, 750, 600]
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap({32: 'space', 105: 'i', 0x5d0: 'aleph', 0x5d1: 'bet', 0x5d2: 'gimel', 0x5d3: 'dalet',
                          **{0xe600 + i: name for i, name in enumerate(order) if i}})
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
builder.setupNameTable({'familyName': 'XGE RTL Context Fixture', 'styleName': 'Regular',
                       'uniqueFontIdentifier': 'XGE-RTL-Context-Fixture-1',
                       'fullName': 'XGE RTL Context Fixture', 'psName': 'XGERTLContextFixture'})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
addOpenTypeFeaturesFromString(builder.font, '''
languagesystem DFLT dflt;
languagesystem hebr dflt;
lookup AlephContext { sub aleph by aleph.context; } AlephContext;
lookup BetContext { sub bet by bet.context; } BetContext;
feature calt {
    sub aleph' lookup AlephContext space bet;
    sub aleph.context space bet' lookup BetContext;
} calt;
feature liga { sub gimel dalet by gimeldalet; } liga;
''')
builder.font.recalcTimestamp = False
builder.font['head'].created = builder.font['head'].modified = 3400000000
path = Path(__file__).resolve().parent / 'data' / 'xge_rtl_context_fixture.ttf'
builder.save(path)
print(f'Generated {path.name}: own Hebrew context/ligature outlines and PUA aliases, fixed timestamps')
