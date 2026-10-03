"""Generate our small, deterministic, freely redistributable shaping fixture.

No OS font or externally licensed font bytes are copied. FontTools is needed
only to regenerate the committed TTF, not to build or run XGE.
"""
from pathlib import Path
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString

order = [".notdef", "space", "A", "V", "a", "f", "i", "x", "fi", "ffi", "acute", "lambda", "mu", "lambda_mu", "fx", "ix", "fa", "aleph", "bet", "gimel", "aleph_bet_gimel", "y", "f_y"]
advances = {g: 600 for g in order}
advances.update(space=250, f=400, i=200, fi=700, ffi=1000, acute=0, lambda_mu=850, fx=900, ix=800, fa=750, aleph_bet_gimel=1000)
builder = FontBuilder(1000, isTTF=True)
builder.setupGlyphOrder(order)
builder.setupCharacterMap({32:"space", 65:"A", 86:"V", 97:"a", 102:"f", 105:"i", 120:"x", 121:"y", 0x301:"acute", 0x3bb:"lambda", 0x3bc:"mu", 0x5d0:"aleph", 0x5d1:"bet", 0x5d2:"gimel", 0x5b0:"acute"})
glyphs = {}
for index, name in enumerate(order):
    pen = TTGlyphPen(None)
    if name != "space":
        width = 150 if name == "acute" else max(100, advances[name] - 80)
        height = 130 if name == "acute" else 550 + (index % 3) * 30
        pen.moveTo((30, 0)); pen.lineTo((width, 0)); pen.lineTo((width, height)); pen.lineTo((30, height)); pen.closePath()
    glyphs[name] = pen.glyph()
builder.setupGlyf(glyphs)
builder.setupHorizontalMetrics({g: (advances[g], 0) for g in order})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({"familyName":"XGE OpenType Fixture", "styleName":"Regular", "uniqueFontIdentifier":"XGE-OpenType-Fixture-1", "fullName":"XGE OpenType Fixture", "psName":"XGEOpenTypeFixture"})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=1000, usWinDescent=250)
builder.setupPost()
builder.setupMaxp()
addOpenTypeFeaturesFromString(builder.font, """
languagesystem DFLT dflt;
languagesystem latn dflt;
languagesystem grek dflt;
languagesystem hebr dflt;
feature liga {
  lookupflag IgnoreMarks;
  sub f f i by ffi; sub f i by fi; sub lambda mu by lambda_mu;
  sub f x by fx; sub i x by ix; sub f a by fa; sub aleph bet gimel by aleph_bet_gimel;
  sub f y by f_y;
  sub f_y by fi a;
} liga;
feature kern {
  lookupflag 0;
  pos A V -100;
  pos ffi <70 0 -100 0> acute <0 0 0 0>;
  pos aleph_bet_gimel <-30 0 -100 0> acute <0 0 0 0>;
} kern;
markClass acute <anchor 0 0> @ACUTE;
feature mark { pos base a <anchor 350 700> mark @ACUTE; pos base aleph <anchor 350 700> mark @ACUTE; } mark;
feature mark {
  pos ligature fi <anchor 100 700> mark @ACUTE ligComponent <anchor 500 700> mark @ACUTE;
  pos ligature ffi <anchor 100 700> mark @ACUTE ligComponent <anchor 400 700> mark @ACUTE ligComponent <anchor 850 700> mark @ACUTE;
  pos ligature aleph_bet_gimel <anchor 850 700> mark @ACUTE ligComponent <anchor 400 700> mark @ACUTE ligComponent <anchor 100 700> mark @ACUTE;
} mark;
table GDEF {
  GlyphClassDef [A V a f i x lambda mu aleph bet gimel], [fi ffi lambda_mu fx ix fa aleph_bet_gimel], [acute], [];
  LigatureCaretByPos fi 120;
  LigatureCaretByPos ffi 250 800;
  LigatureCaretByPos lambda_mu 300;
  LigatureCaretByPos fx 120 160;
  LigatureCaretByPos ix -10;
  LigatureCaretByPos aleph_bet_gimel 250 800;
} GDEF;
""")
builder.font["head"].created = builder.font["head"].modified = 3406620150
builder.font.recalcTimestamp = False
target = Path(__file__).parent / "data" / "xge_opentype_fixture.ttf"
target.parent.mkdir(exist_ok=True)
builder.save(target)
print(target)

# Keep the original fixture byte-for-byte stable. This second font exercises
# original (including off-curve) point indices, not rasterizer vertex indices.
from fontTools.ttLib.tables._g_l_y_f import Glyph, GlyphComponent, GlyphCoordinates
from fontTools.ttLib.tables import otTables

font = builder.font
original_order = font.getGlyphOrder()[:]
for name, carets in (("fi", (120,)), ("ffi", (250, 800))):
    old = glyphs[name]
    width, height = old.xMax if hasattr(old, "xMax") else advances[name] - 80, 600
    pen = TTGlyphPen(None)
    pen.moveTo((0 if name == "fi" else 30, 0))
    pen.qCurveTo(*[(x, 0) for x in carets], (width, 0))
    pen.lineTo((width, height)); pen.lineTo((30, height)); pen.closePath()
    font["glyf"][name] = pen.glyph()

def component(name, offset=(0, 0), matrix=None, flags=0):
    value = GlyphComponent(); value.glyphName = name
    value.x, value.y = offset; value.flags = flags
    if matrix is not None:
        value.transform = matrix
    return value

def compound(*components):
    value = Glyph(); value.numberOfContours = -1; value.components = list(components)
    return value

font["glyf"]["lambda_mu"] = compound(component("fi", (180, 0)))
font["glyf"]["aleph_bet_gimel"] = compound(component("ffi"))
extra = {
    "tt_scale": compound(component("ffi", (80, -20), [[.5, 0], [0, .5]])),
    "tt_scaled_offset": compound(component("ffi", (80, -20), [[.5, 0], [0, .5]], 0x800)),
    "tt_unscaled_offset": compound(component("ffi", (80, -20), [[.5, 0], [0, .5]], 0x1000)),
    "tt_xy_scale": compound(component("ffi", (100, 0), [[.5, 0], [0, .75]])),
    "tt_shear": compound(component("ffi", (100, 20), [[1, .5], [.25, 1]])),
    "tt_nested": compound(component("tt_scale", (100, 50))),
    "tt_attach": compound(component("fi")),
}
attached = GlyphComponent(); attached.glyphName = "fi"; attached.flags = 0
attached.firstPt, attached.secondPt = 3, 0
extra["tt_attach"].components.append(attached)
# A contour starting off-curve, and one with no explicit on-curve points.
for name, points, flags in (
    ("tt_off_start", [(0, 0), (100, 0), (100, 100), (0, 100)], [0, 1, 1, 1]),
    ("tt_off_all", [(0, 0), (100, 0), (100, 100), (0, 100)], [0, 0, 0, 0]),
    ("tt_many", [(i * 2, 100 + i % 2 * 20) for i in range(150)], [1] * 150),
):
    value = Glyph(); value.numberOfContours = 1
    value.coordinates = GlyphCoordinates(points); value.flags = bytearray(flags)
    value.endPtsOfContours = [len(points) - 1]
    from fontTools.ttLib.tables.ttProgram import Program
    value.program = Program(); value.program.fromBytecode([])
    extra[name] = value

for name, value in extra.items():
    font["glyf"][name] = value
    font["hmtx"].metrics[name] = (1500, 0)
font.setGlyphOrder(original_order + list(extra))
for name in tuple(extra):
    coords, ends, flags = font["glyf"][name].getCoordinates(font["glyf"])
    flat = Glyph(); flat.numberOfContours = len(ends)
    flat.coordinates = GlyphCoordinates(coords); flat.endPtsOfContours = list(ends)
    flat.flags = bytearray(flags); flat.program = Program(); flat.program.fromBytecode([])
    font["glyf"][name + "_flat"] = flat
    font["hmtx"].metrics[name + "_flat"] = (1500, 0)
font.setGlyphOrder(original_order + list(extra) + [name + "_flat" for name in extra])
for name, value in zip(font["GDEF"].table.LigCaretList.Coverage.glyphs,
                       font["GDEF"].table.LigCaretList.LigGlyph):
    if name in ("fi", "ffi", "lambda_mu", "aleph_bet_gimel"):
        indices = (1, 2) if name in ("ffi", "aleph_bet_gimel") else (1,)
        value.CaretValue = []
        for index in indices:
            caret = otTables.CaretValue(); caret.Format = 2; caret.CaretValuePoint = index
            value.CaretValue.append(caret)
        value.CaretCount = len(indices)
target_points = target.with_name("xge_opentype_points_fixture.ttf")
font.save(target_points)
# Independent FontTools coordinate oracle, read back after SFNT serialization.
from fontTools.ttLib import TTFont
with TTFont(target_points) as verified:
    rows = []
    for glyph_id, name in enumerate(verified.getGlyphOrder()):
        coords, ends, flags = verified["glyf"][name].getCoordinates(verified["glyf"])
        for index, ((x, y), flag) in enumerate(zip(coords, flags)):
            rows.append(f"{glyph_id} {index} {x:.10g} {y:.10g} {int(bool(flag & 1))} {int(index in ends)}\n")
    target_points.with_suffix(".points").write_text("".join(rows), encoding="ascii")
    pairs = [f"{verified.getGlyphID(name)} {verified.getGlyphID(name + '_flat')}\n" for name in extra]
    target_points.with_suffix(".pairs").write_text("".join(pairs), encoding="ascii")
print(target_points)
