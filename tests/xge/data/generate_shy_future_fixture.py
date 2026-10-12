from pathlib import Path
import sys
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

order=['.notdef','space','hyphen','a','b','c','d','a.term','c.term','a.retro']
advances=[600,250,150,400,300,800,300,500,500,1150]
b=FontBuilder(1000,isTTF=True);b.setupGlyphOrder(order)
b.setupCharacterMap({32:'space',45:'hyphen',0x3bb:'a',0x3bc:'b',0x3bd:'c',0x3be:'d'})
glyphs={}
for i,name in enumerate(order):
 p=TTGlyphPen(None)
 if name!='space':
  p.moveTo((20,0));p.lineTo((advances[i]-50,0));p.lineTo((advances[i]-50,500));p.lineTo((20,500));p.closePath()
 glyphs[name]=p.glyph()
b.setupGlyf(glyphs);b.setupHorizontalMetrics({n:(advances[i],0) for i,n in enumerate(order)})
b.setupHorizontalHeader(ascent=800,descent=-200)
b.setupNameTable({'familyName':'XGE SHY Future Probe','styleName':'Regular','fullName':'XGE SHY Future Probe','psName':'XGEShyFutureProbe'})
b.setupOS2(sTypoAscender=800,sTypoDescender=-200,usWinAscent=1000,usWinDescent=250)
b.setupPost();b.setupMaxp()
addOpenTypeFeaturesFromString(b.font,'''
languagesystem DFLT dflt;
languagesystem grek dflt;
lookup A { sub a by a.term; } A;
lookup C { sub c by c.term; } C;
lookup Retro { sub a.term by a.retro; } Retro;
lookup Terminals { sub a' lookup A hyphen; sub c' lookup C hyphen; } Terminals;
lookup Future { sub a.term' lookup Retro hyphen b space c.term hyphen; } Future;
feature calt { lookup Terminals; lookup Future; } calt;
''')
b.font.recalcTimestamp=False;b.font['head'].created=b.font['head'].modified=3400000000
b.save(Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).with_name('xge_shy_future_fixture.ttf'))
