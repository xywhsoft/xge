"""Own fonts whose incomplete primary has visibly different f/x outlines.

FontTools is only required to regenerate fixtures, not to build XGE.
"""
from pathlib import Path
import sys
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen

order = ['.notdef', 'space', 'f', 'i', 'x', 'comma', 'open', 'close',
         'alpha', 'acute', 'e', 'eacute']
mapping = {32: 'space', 102: 'f', 105: 'i', 120: 'x', 44: 'comma',
           40: 'open', 41: 'close', 0x3b1: 'alpha', 0x301: 'acute',
           101: 'e', 0xe9: 'eacute'}
target = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).parent / 'data'
target.mkdir(parents=True, exist_ok=True)
for partial in (False, True):
    kind = 'primary' if partial else 'full'
    advances = {name: 600 for name in order}
    advances.update(space=250, f=200 if partial else 400, i=300,
                    x=200 if partial else 500, comma=200, open=200, close=200,
                    acute=0, e=400, eacute=500)
    cmap = dict(mapping)
    if partial:
        for cp in (105, 0x3b1, 0x301, 101):
            del cmap[cp]
    cmap.update({0xe600 + i: name for i, name in enumerate(order) if i})
    builder = FontBuilder(1000, isTTF=True)
    builder.setupGlyphOrder(order); builder.setupCharacterMap(cmap)
    glyphs = {}
    for i, name in enumerate(order):
        pen = TTGlyphPen(None)
        if name != 'space':
            width = 100 if name == 'acute' else advances[name] - 50
            height = 110 if name == 'acute' else (300 if partial else 500) + i * 10
            pen.moveTo((20, 0)); pen.lineTo((width, 0))
            pen.lineTo((width, height)); pen.lineTo((20, height)); pen.closePath()
        glyphs[name] = pen.glyph()
    builder.setupGlyf(glyphs)
    builder.setupHorizontalMetrics({name: (advances[name], 0) for name in order})
    builder.setupHorizontalHeader(ascent=800, descent=-200)
    family = 'XGE Word Context ' + kind.capitalize()
    builder.setupNameTable({'familyName': family, 'styleName': 'Regular',
                           'uniqueFontIdentifier': family + '-1',
                           'fullName': family, 'psName': 'XGEWordContext' + kind.capitalize()})
    builder.setupOS2(sTypoAscender=800, sTypoDescender=-200,
                    usWinAscent=1000, usWinDescent=250)
    builder.setupPost(); builder.setupMaxp()
    builder.font.recalcTimestamp = False
    builder.font['head'].created = builder.font['head'].modified = 3400000000
    path = target / ('xge_word_context_' + kind + '.ttf')
    builder.save(path); print(path)
