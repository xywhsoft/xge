"""Owned Greek/Hebrew SHY context, narrow/wide terminal glyphs and PUA aliases."""
from pathlib import Path
import sys
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

order = ['.notdef', 'space', 'hyphen']
advances = [600, 250, 150]
cmap = {32: 'space', 45: 'hyphen'}
features = ['languagesystem DFLT dflt;', 'languagesystem grek dflt;',
            'languagesystem hebr dflt;']
rules = []
pairs = [(0x3bb, 0x3bc, 'greekNarrow', 500),
         (0x3bd, 0x3be, 'greekWide', 1150),
         (0x5d0, 0x5d1, 'hebrewNarrow', 500),
         (0x5d2, 0x5d3, 'hebrewWide', 1150)]
for first, last, name, terminal in pairs:
    a, b = name + 'First', name + 'Last'
    order.extend([a, b, a + '.context', b + '.context', a + '.terminal'])
    advances.extend([400, 300, 800, 900, terminal])
    cmap[first], cmap[last] = a, b
    for label, glyph, target in [('First', a, a + '.context'),
                                  ('Last', b, b + '.context'),
                                  ('Terminal', a, a + '.terminal')]:
        lookup = name + label
        features.append(f'lookup {lookup} {{ sub {glyph} by {target}; }} {lookup};')
    rules.extend([f"sub {a}' lookup {name}First space {b};",
                  f"sub {a}.context space {b}' lookup {name}Last;",
                  f"sub {a}' lookup {name}First {b};",
                  f"sub {a}.context {b}' lookup {name}Last;",
                  f"sub {a}' lookup {name}Terminal hyphen;",
                  f"sub {a}' lookup {name}Terminal space hyphen;"])
    # HarfBuzz reverses an overridden Greek stream to the script's natural
    # order before GSUB. Explicit mirrored patterns exercise the same literal
    # aliases under RLO; natural Hebrew remains covered by the forward rules.
    if first < 0x500:
        rules.extend([f"sub {b}' lookup {name}Last space' {a}' lookup {name}First;",
                      f"sub {b}' lookup {name}Last {a}' lookup {name}First;",
                      f"sub hyphen {a}' lookup {name}Terminal;",
                      f"sub hyphen space {a}' lookup {name}Terminal;"])
features.append('feature calt { ' + ' '.join(rules) + ' } calt;')
cmap.update({0xe700 + i: name for i, name in enumerate(order) if i})
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order); builder.setupCharacterMap(cmap)
glyphs = {}
for i, name in enumerate(order):
    pen = TTGlyphPen(None)
    if name != 'space':
        right, top = advances[i] - 60, 200 + 20 * i
        pen.moveTo((20, 0)); pen.lineTo((right, 0))
        pen.lineTo((right, top)); pen.lineTo((20, top)); pen.closePath()
    glyphs[name] = pen.glyph()
builder.setupGlyf(glyphs)
builder.setupHorizontalMetrics({name: (advances[i], 0) for i, name in enumerate(order)})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({'familyName': 'XGE SHY Context Fixture', 'styleName': 'Regular',
                       'uniqueFontIdentifier': 'XGE-SHY-Context-Fixture-1',
                       'fullName': 'XGE SHY Context Fixture', 'psName': 'XGEShyContextFixture'})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
addOpenTypeFeaturesFromString(builder.font, '\n'.join(features))
builder.font.recalcTimestamp = False
builder.font['head'].created = builder.font['head'].modified = 3400000000
path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent / 'data' / 'xge_shy_context_fixture.ttf'
builder.save(path)
print(f'Generated {path.name}: Greek/Hebrew base/terminal context, literal aliases, fixed timestamps')
