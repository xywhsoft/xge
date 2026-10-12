"""Deterministic project-owned SFNT: Arabic joining and language-specific locl."""
from pathlib import Path
from copy import deepcopy
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

directory = Path(__file__).resolve().parent / "data"
order = [".notdef", "space", "i", "i_und", "i_en", "i_tr", "beh", "beh_init", "beh_medi", "beh_fina",
         "tatweel", "tatweel_syrc", "alaph", "fatha"]
advance = {name: 600 for name in order}
advance.update(space=250, i=200, i_und=300, i_en=400, i_tr=500, beh_init=700, beh_medi=800, beh_fina=900,
               tatweel=400, tatweel_syrc=950, fatha=0)
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap({32: "space", 105: "i", 0x628: "beh", 0x640: "tatweel", 0x710: "alaph", 0x64e: "fatha",
                          **{0xe100 + i: name for i, name in enumerate(order) if i}})
glyphs = {}
for index, name in enumerate(order):
    pen = TTGlyphPen(None)
    if name != "space":
        width = 120 if name == "fatha" else advance[name] - 80
        pen.moveTo((20, 0)); pen.lineTo((width, 0)); pen.lineTo((width, 350 + index * 10))
        pen.lineTo((20, 350 + index * 10)); pen.closePath()
    glyphs[name] = pen.glyph()
builder.setupGlyf(glyphs)
builder.setupHorizontalMetrics({name: (advance[name], 0) for name in order})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({"familyName": "XGE Context Fixture", "styleName": "Regular", "uniqueFontIdentifier": "XGE-Context-Fixture-1",
                       "fullName": "XGE Context Fixture", "psName": "XGEContextFixture"})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost(); builder.setupMaxp()
features = """
languagesystem DFLT dflt;
languagesystem latn dflt;
languagesystem latn ENG;
languagesystem latn TRK;
languagesystem arab dflt;
languagesystem syrc dflt;
feature locl {
 script latn;
 language dflt; sub i by i_und;
 language ENG exclude_dflt; sub i by i_en;
 language TRK exclude_dflt; sub i by i_tr;
 script syrc; language dflt; sub tatweel by tatweel_syrc;
} locl;
feature init { script arab; language dflt; sub beh by beh_init; } init;
feature medi { script arab; language dflt; sub beh by beh_medi; } medi;
feature fina { script arab; language dflt; sub beh by beh_fina; } fina;
markClass fatha <anchor 100 50> @TOP;
feature mark { script arab; language dflt; pos base [beh beh_init beh_medi beh_fina] <anchor 300 600> mark @TOP; } mark;
"""
addOpenTypeFeaturesFromString(builder.font, features)
builder.font.recalcTimestamp = False
builder.font["head"].created = builder.font["head"].modified = 3400000000
builder.save(directory / "xge_context_fixture.ttf")
probe = deepcopy(builder.font)
for table in probe["cmap"].tables:
    if table.isUnicode():
        table.cmap.pop(105, None)
        table.cmap.pop(0x628, None)
probe_features = features.replace("sub i by i_tr;", "sub i by i_tr; sub .notdef by i_tr;")
for form in ("init", "medi", "fina"):
    probe_features = probe_features.replace(f"sub beh by beh_{form};", f"sub beh by beh_{form}; sub .notdef by beh_{form};")
addOpenTypeFeaturesFromString(probe, probe_features)
probe.save(directory / "xge_context_probe.ttf")
medi_probe = deepcopy(builder.font)
for table in medi_probe["cmap"].tables:
    if table.isUnicode():
        table.cmap.pop(0x628, None)
# Unlike the general probe, this font covers a missing Beh only when the
# actual joining context selects medi. A truncated context must fall back.
addOpenTypeFeaturesFromString(medi_probe, features.replace(
    "sub beh by beh_medi;", "sub beh by beh_medi; sub .notdef by beh_medi;"))
medi_probe.save(directory / "xge_context_medi_probe.ttf")
print("Generated project-owned context fixtures, with fixed timestamps")
