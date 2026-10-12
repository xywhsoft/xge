"""Own deterministic ASCII font with distinct DFLT and latn punctuation."""
from pathlib import Path
import sys
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

order = ['.notdef', 'space', 'a', 'open', 'close', 'open.common',
         'close.common', 'open.latn', 'close.latn']
advances = [600, 250, 600, 200, 200, 200, 200, 300, 300]
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap({32: 'space', 97: 'a', 40: 'open', 41: 'close',
                          **{0xe500 + i: name for i, name in enumerate(order) if i}})
glyphs = {}
for index, name in enumerate(order):
    pen = TTGlyphPen(None)
    if name != 'space':
        right = advances[index] - 60
        pen.moveTo((20, 0)); pen.lineTo((right, 0))
        pen.lineTo((right, 300 + 30 * index)); pen.lineTo((20, 300 + 30 * index))
        pen.closePath()
    glyphs[name] = pen.glyph()
builder.setupGlyf(glyphs)
builder.setupHorizontalMetrics({name: (advances[i], 0) for i, name in enumerate(order)})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({'familyName': 'XGE ASCII Script Fixture', 'styleName': 'Regular',
                       'uniqueFontIdentifier': 'XGE-ASCII-Script-Fixture-1',
                       'fullName': 'XGE ASCII Script Fixture', 'psName': 'XGEASCIIScriptFixture'})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
addOpenTypeFeaturesFromString(builder.font, '''
languagesystem DFLT dflt;
languagesystem latn dflt;
feature locl {
    script DFLT; language dflt;
    sub open by open.common; sub close by close.common;
    script latn; language dflt;
    sub open by open.latn; sub close by close.latn;
} locl;
''')
builder.font.recalcTimestamp = False
builder.font['head'].created = builder.font['head'].modified = 3400000000
path = (Path(sys.argv[1]) if len(sys.argv) > 1 else
        Path(__file__).resolve().parent / 'data' / 'xge_ascii_script_fixture.ttf')
builder.save(path)
print(f'Generated {path.name}: own DFLT/latn outlines, cmap and GSUB')
