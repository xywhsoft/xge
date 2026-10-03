"""Project-owned Greek GSUB context and ligature fixture with literal PUA aliases."""
from pathlib import Path
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

order = ['.notdef', 'space', 'lambda', 'mu', 'lambda.context', 'mu.context', 'phi', 'alpha', 'phialpha']
advances = [600, 250, 400, 300, 800, 900, 600, 250, 750]
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap({32: 'space', 0x3bb: 'lambda', 0x3bc: 'mu', 0x3c6: 'phi', 0x3b1: 'alpha',
                          **{0xe500 + i: name for i, name in enumerate(order) if i}})
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
builder.setupNameTable({'familyName': 'XGE Unicode Context Fixture', 'styleName': 'Regular',
                       'uniqueFontIdentifier': 'XGE-Unicode-Context-Fixture-1',
                       'fullName': 'XGE Unicode Context Fixture', 'psName': 'XGEUnicodeContextFixture'})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
addOpenTypeFeaturesFromString(builder.font, '''
languagesystem DFLT dflt;
languagesystem grek dflt;
lookup LambdaContext { sub lambda by lambda.context; } LambdaContext;
lookup MuContext { sub mu by mu.context; } MuContext;
feature calt {
    sub lambda' lookup LambdaContext space mu;
    sub lambda.context space mu' lookup MuContext;
} calt;
feature liga { sub phi alpha by phialpha; } liga;
''')
builder.font.recalcTimestamp = False
builder.font['head'].created = builder.font['head'].modified = 3400000000
path = Path(__file__).resolve().parent / 'data' / 'xge_unicode_context_fixture.ttf'
builder.save(path)
print(f'Generated {path.name}: own Greek context/ligature outlines and PUA aliases, fixed timestamps')
