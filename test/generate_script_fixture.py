"""Own deterministic SFNT with visibly different per-script locl glyphs."""
from pathlib import Path
from copy import deepcopy
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

root = Path(__file__).resolve().parent
order = [".notdef", "space", "a", "lambda", "hira", "kana", "long", "alaph", "beh", "tatweel",
         "open", "close", "open_latn", "close_latn", "open_grek", "close_grek", "long_kana", "tatweel_syrc"]
advances = {name: 600 for name in order}
advances.update(space=250, open=200, close=200, open_latn=300, close_latn=300,
                open_grek=450, close_grek=450, long=400, long_kana=900, tatweel=500, tatweel_syrc=800)
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap({32: "space", 97: "a", 0x3bb: "lambda", 0x3042: "hira", 0x30ab: "kana", 0x30fc: "long",
                          0x710: "alaph", 0x628: "beh", 0x640: "tatweel", 40: "open", 41: "close", 0x3008: "open", 0x3009: "close",
                          **{0xe000 + i: name for i, name in enumerate(order) if i}})
glyphs = {}
for index, name in enumerate(order):
    pen = TTGlyphPen(None)
    if name != "space":
        pen.moveTo((20, 0)); pen.lineTo((advances[name] - 60, 0))
        pen.lineTo((advances[name] - 60, 400 + index * 10)); pen.lineTo((20, 400 + index * 10)); pen.closePath()
    glyphs[name] = pen.glyph()
builder.setupGlyf(glyphs)
builder.setupHorizontalMetrics({name: (advances[name], 0) for name in order})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({"familyName": "XGE Script Fixture", "styleName": "Regular", "uniqueFontIdentifier": "XGE-Script-Fixture-1",
                       "fullName": "XGE Script Fixture", "psName": "XGEScriptFixture"})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
features = """
languagesystem DFLT dflt;
languagesystem latn dflt;
languagesystem grek dflt;
languagesystem kana dflt;
languagesystem syrc dflt;
languagesystem arab dflt;
feature locl {
 script latn; language dflt; sub open by open_latn; sub close by close_latn;
 script grek; language dflt; sub open by open_grek; sub close by close_grek;
 script kana; language dflt; sub long by long_kana;
 script syrc; language dflt; sub tatweel by tatweel_syrc;
} locl;
"""
addOpenTypeFeaturesFromString(builder.font, features)
builder.font.recalcTimestamp = False
builder.font["head"].created = builder.font["head"].modified = 3400000000
builder.save(root / "data/xge_script_fixture.ttf")
probe = deepcopy(builder.font)
for table in builder.font["cmap"].tables:
    if table.isUnicode():
        table.cmap.pop(0x710, None)
builder.save(root / "data/xge_script_no_alaph.ttf")
for table in probe["cmap"].tables:
    if table.isUnicode():
        table.cmap.pop(0x640, None)
addOpenTypeFeaturesFromString(probe, features.replace("sub tatweel by tatweel_syrc;", "sub tatweel by tatweel_syrc; sub .notdef by tatweel_syrc;"))
probe.save(root / "data/xge_script_probe_tatweel.ttf")
print("Generated own Unicode script fixtures; no existing fixture modified")
