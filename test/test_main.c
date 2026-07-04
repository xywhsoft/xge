#include "xge.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char* message)
{
	if ( !condition ) {
		printf("xge smoke failed: %s\n", message);
		return 0;
	}
	return 1;
}

static int test_shape_ex(void)
{
	xge_shape_ex shape;
	xge_shape_ex_scene scene;
	xge_shape_ex_matrix_t matrix;
	xge_shape_ex_color_stop_t stops[2];
	xge_rect_t bounds;
	float dash[2] = {4.0f, 2.0f};
	int ret;

	shape = NULL;
	ret = xgeShapeExCreate(&shape);
	if ( !check((ret == XGE_OK) && (shape != NULL), "ShapeEx create") ) return 0;
	if ( !check(xgeShapeExMoveTo(shape, 0.0f, 0.0f) == XGE_OK, "ShapeEx moveTo") ) return 0;
	if ( !check(xgeShapeExLineTo(shape, 10.0f, 0.0f) == XGE_OK, "ShapeEx lineTo") ) return 0;
	if ( !check(xgeShapeExQuadTo(shape, 12.0f, 6.0f, 10.0f, 10.0f) == XGE_OK, "ShapeEx quadTo") ) return 0;
	if ( !check(xgeShapeExCubicTo(shape, 8.0f, 14.0f, 2.0f, 14.0f, 0.0f, 10.0f) == XGE_OK, "ShapeEx cubicTo") ) return 0;
	if ( !check(xgeShapeExClose(shape) == XGE_OK, "ShapeEx close") ) return 0;
	if ( !check(xgeShapeExAppendSvgPath(shape, "M20 20 A8 8 0 0 1 28 28 S40 36 48 28") == XGE_OK, "ShapeEx SVG path") ) return 0;
	if ( !check(xgeShapeExAppendRect(shape, 4.0f, 4.0f, 18.0f, 12.0f, 3.0f, 3.0f, 1) == XGE_OK, "ShapeEx append rect") ) return 0;
	if ( !check(xgeShapeExAppendRect(shape, 5.0f, 5.0f, 10.0f, 8.0f, 2.0f, 2.0f, 0) == XGE_OK, "ShapeEx append ccw rounded rect") ) return 0;
	if ( !check(xgeShapeExAppendCircle(shape, 12.0f, 12.0f, 5.0f, 5.0f, 1) == XGE_OK, "ShapeEx append circle") ) return 0;
	if ( !check(xgeShapeExAppendCircle(shape, 12.0f, 12.0f, 3.0f, 4.0f, 0) == XGE_OK, "ShapeEx append ccw ellipse") ) return 0;
	if ( !check(xgeShapeExFillColor(shape, XGE_COLOR_RGBA(1, 2, 3, 4)) == XGE_OK, "ShapeEx fill color") ) return 0;
	stops[0].fOffset = 0.0f;
	stops[0].iColor = XGE_COLOR_RGBA(255, 0, 0, 255);
	stops[1].fOffset = 1.0f;
	stops[1].iColor = XGE_COLOR_RGBA(0, 0, 255, 255);
	if ( !check(xgeShapeExFillLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx linear gradient") ) return 0;
	if ( !check(xgeShapeExFillGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT) == XGE_OK, "ShapeEx gradient spread") ) return 0;
	matrix.fA = 0.8f;
	matrix.fB = 0.0f;
	matrix.fC = 0.0f;
	matrix.fD = 1.0f;
	matrix.fE = 0.1f;
	matrix.fF = 0.0f;
	if ( !check(xgeShapeExFillGradientTransformSet(shape, &matrix) == XGE_OK, "ShapeEx gradient transform") ) return 0;
	if ( !check(xgeShapeExFillRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx radial gradient") ) return 0;
	if ( !check(xgeShapeExFillColor(shape, XGE_COLOR_RGBA(1, 2, 3, 4)) == XGE_OK, "ShapeEx fill color reset") ) return 0;
	if ( !check(xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(5, 6, 7, 8)) == XGE_OK, "ShapeEx stroke color") ) return 0;
	if ( !check(xgeShapeExStrokeLinearGradient(shape, 0.0f, 0.0f, 1.0f, 0.0f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke linear gradient") ) return 0;
	if ( !check(xgeShapeExStrokeGradientSpread(shape, XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT) == XGE_OK, "ShapeEx stroke gradient spread") ) return 0;
	if ( !check(xgeShapeExStrokeGradientTransformSet(shape, &matrix) == XGE_OK, "ShapeEx stroke gradient transform") ) return 0;
	if ( !check(xgeShapeExStrokeRadialGradient(shape, 0.5f, 0.5f, 0.5f, 0.45f, 0.45f, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, stops, 2) == XGE_OK, "ShapeEx stroke radial gradient") ) return 0;
	if ( !check(xgeShapeExStrokeColor(shape, XGE_COLOR_RGBA(5, 6, 7, 8)) == XGE_OK, "ShapeEx stroke color reset") ) return 0;
	if ( !check(xgeShapeExStrokeWidth(shape, 2.0f) == XGE_OK, "ShapeEx stroke width") ) return 0;
	if ( !check(xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND) == XGE_OK, "ShapeEx stroke cap") ) return 0;
	if ( !check(xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND) == XGE_OK, "ShapeEx stroke join") ) return 0;
	if ( !check(xgeShapeExStrokeDash(shape, dash, 2, 1.0f) == XGE_OK, "ShapeEx stroke dash") ) return 0;
	if ( !check(xgeShapeExFillRule(shape, XGE_SHAPE_EX_FILL_EVEN_ODD) == XGE_OK, "ShapeEx fill rule") ) return 0;
	if ( !check(xgeShapeExOpacity(shape, 0.75f) == XGE_OK, "ShapeEx opacity") ) return 0;
	if ( !check((xgeShapeExGetBounds(shape, 0.25f, &bounds) == XGE_OK) && (bounds.fW > 0.0f) && (bounds.fH > 0.0f), "ShapeEx bounds") ) return 0;
	if ( !check(xgeShapeExClipRectSet(shape, (xge_rect_t){1.0f, 2.0f, 8.0f, 9.0f}) == XGE_OK, "ShapeEx clip rect") ) return 0;
	if ( !check(xgeShapeExClipClear(shape) == XGE_OK, "ShapeEx clip clear") ) return 0;
	matrix.fA = 1.0f;
	matrix.fB = 0.0f;
	matrix.fC = 0.0f;
	matrix.fD = 1.0f;
	matrix.fE = 8.0f;
	matrix.fF = 9.0f;
	if ( !check(xgeShapeExTransformSet(shape, &matrix) == XGE_OK, "ShapeEx transform") ) return 0;
	if ( !check(xgeShapeExTransformIdentity(shape) == XGE_OK, "ShapeEx transform identity") ) return 0;
	{
		xge_shape_ex clone = NULL;
		ret = xgeShapeExClone(shape, &clone);
		if ( !check((ret == XGE_OK) && (clone != NULL), "ShapeEx clone") ) return 0;
		xgeShapeExDestroy(clone);
	}
	scene = NULL;
	ret = xgeShapeExSceneCreate(&scene);
	if ( !check((ret == XGE_OK) && (scene != NULL), "ShapeEx scene create") ) return 0;
	if ( !check(xgeShapeExSceneAdd(scene, shape) == XGE_OK, "ShapeEx scene add") ) return 0;
	if ( !check(xgeShapeExSceneOpacity(scene, 0.5f) == XGE_OK, "ShapeEx scene opacity") ) return 0;
	if ( !check(xgeShapeExSceneTransformSet(scene, &matrix) == XGE_OK, "ShapeEx scene transform") ) return 0;
	if ( !check(xgeShapeExSceneClear(scene) == XGE_OK, "ShapeEx scene clear") ) return 0;
	xgeShapeExSceneDestroy(scene);
	xgeShapeExDestroy(shape);
	{
		xge_shape_ex smooth = NULL;
		xge_rect_t smoothBounds;

		ret = xgeShapeExCreate(&smooth);
		if ( !check((ret == XGE_OK) && (smooth != NULL), "ShapeEx smooth create") ) return 0;
		ret = xgeShapeExAppendSvgPath(smooth, "M0 0 T10 10 20 0");
		if ( !check(ret == XGE_OK, "ShapeEx repeated T parse") ) {
			xgeShapeExDestroy(smooth);
			return 0;
		}
		ret = xgeShapeExGetBounds(smooth, 0.05f, &smoothBounds);
		if ( !check((ret == XGE_OK) && (smoothBounds.fH > 12.0f), "ShapeEx repeated T reflection") ) {
			xgeShapeExDestroy(smooth);
			return 0;
		}
		xgeShapeExDestroy(smooth);
	}
	return 1;
}

static int test_svg(void)
{
	static const char svg_text[] =
		"<svg viewBox=\"0 0 32 16\" preserveAspectRatio=\"xMinYMax meet\">"
		"<style>.smokeAccent{fill:#010203;stroke:none}#smokeId{fill:#040506;stroke:none}polygon{fill:#070809;stroke:none}</style>"
		"<style>.smokeCurrent{color:goldenrod;fill:currentColor;stroke:transparent}</style>"
		"<style>path.smokeCompound{fill:#0a0b0c;stroke:none}.smokePair.extra{fill:#0d0e0f;stroke:none}#smokeId2.extra{fill:#101112;stroke:none}</style>"
		"<style>stop.smokeStop{stop-color:#19c37d;stop-opacity:100%}</style>"
		"<style><![CDATA[/* xge smoke */@font-face{font-family:ignore;src:url(ignore)}path.smokeCData{fill:/*inline*/#131415;stroke:none}@media screen{.ignored{fill:red}}]]></style>"
		"<style>path.smokeSpecific{fill:#161718;stroke:none}.smokeSpecific{fill:#000000;stroke:none}.smokeAttr{fill:#191a1b;stroke:none}</style>"
		"<defs>"
		"<linearGradient id=\"grad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" spreadMethod=\"reflect\" gradientTransform=\"translate(0.1 0) scale(0.8 1)\">"
		"<stop offset=\"0%\" stop-color=\"#ff0000\"/>"
		"<stop offset=\"100%\" style=\"stop-color:#0000ff;stop-opacity:0.8\"/>"
		"</linearGradient>"
		"<radialGradient id=\"rad\" cx=\"50%\" cy=\"50%\" r=\"50%\" fx=\"35%\" fy=\"35%\" spreadMethod=\"repeat\" gradientTransform=\"scale(0.95 0.95) skewX(4)\">"
		"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
		"<stop offset=\"100%\" stop-color=\"#112233\"/>"
		"</radialGradient>"
		"<linearGradient id=\"gradAlias\" href=\"#lateStops\" x1=\"100%\" y1=\"0%\" x2=\"0%\" y2=\"100%\"/>"
		"<linearGradient id=\"gradChain\" xlink:href=\"#gradAlias\" spreadMethod=\"repeat\"/>"
		"<linearGradient id=\"lateStops\"><stop offset=\"0%\" stop-color=\"#102030\"/><stop offset=\"100%\" stop-color=\"#f0c020\"/></linearGradient>"
		"<linearGradient id=\"currentStopGrad\" color=\"#abcdef\"><stop offset=\"0\" stop-color=\"currentColor\"/><stop offset=\"1\" style=\"stop-color:currentColor\"/></linearGradient>"
		"<linearGradient id=\"cssStopGrad\"><stop class=\"smokeStop\" offset=\"0\"/><stop class=\"smokeStop\" offset=\"1\"/></linearGradient>"
		"<radialGradient id=\"radAlias\" href=\"#lateRadStops\" cx=\"50%\" cy=\"50%\" r=\"60%\"/>"
		"<radialGradient id=\"lateRadStops\"><stop offset=\"0%\" stop-color=\"#ffffff\"/><stop offset=\"100%\" stop-color=\"#203040\"/></radialGradient>"
		"<clipPath id=\"smokeClip\"><rect x=\"24\" y=\"12\" width=\"4\" height=\"3\"/></clipPath>"
		"<clipPath id=\"smokeBoxClip\" clipPathUnits=\"objectBoundingBox\"><g transform=\"translate(0.1 0)\"><rect x=\"0\" y=\"0\" width=\"0.5\" height=\"1\"/></g></clipPath>"
		"<mask id=\"smokeMask\" maskContentUnits=\"objectBoundingBox\"><rect x=\"0\" y=\"0\" width=\"0.5\" height=\"1\" fill=\"white\" opacity=\"0.75\"/></mask>"
		"<rect id=\"single\" x=\"0\" y=\"0\" width=\"4\" height=\"3\" fill=\"#223344\"/>"
		"<symbol id=\"smokeSymbol\" viewBox=\"-1 -1 2 2\" preserveAspectRatio=\"none\"><rect x=\"-1\" y=\"-1\" width=\"2\" height=\"2\" fill=\"#334455\" stroke=\"none\"/></symbol>"
		"<g id=\"badge\" fill=\"#224466\" stroke=\"#88aacc\" stroke-width=\"1.5\" stroke-dasharray=\"3 1\" paint-order=\"stroke fill\">"
		"<rect x=\"1\" y=\"1\" width=\"8\" height=\"6\" rx=\"1\"/>"
		"<polyline points=\"1,10 6,12 10,10\" fill=\"none\"/>"
		"</g>"
		"</defs>"
		"<g transform=\"translate(1,1)\" fill=\"#224466\" stroke=\"#88aacc\" stroke-width=\"1.5\" stroke-dasharray=\"3 1\" paint-order=\"stroke fill\">"
		"<rect x=\"1\" y=\"1\" width=\"8\" height=\"6\" rx=\"1\" fill=\"#336699\"/>"
		"<polyline points=\"1,10 6,12 10,10\" fill=\"none\"/>"
		"</g>"
		"<use href=\"#single\" x=\"12\" y=\"10\"/>"
		"<use xlink:href=\"#badge\" x=\"15\" y=\"0\" opacity=\"0.7\"/>"
		"<use href=\"#smokeSymbol\" x=\"26\" y=\"11\" width=\"4\" height=\"2\"/>"
		"<use href=\"#lateSmokeSymbol\" x=\"1\" y=\"8\" width=\"4\" height=\"2\"/>"
		"<rect x=\"22\" y=\"1\" width=\"8\" height=\"5\" fill=\"url(#grad)\"/>"
		"<circle cx=\"19\" cy=\"12\" r=\"3\" fill=\"url(#rad)\"/>"
		"<svg x=\"1\" y=\"4\" width=\"4\" height=\"2\" viewBox=\"0 0 40 20\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"40\" height=\"20\" fill=\"#3344aa\" stroke=\"none\"/></svg>"
		"<g visibility=\"hidden\"><path d=\"M10 8 L12 8 L12 9 Z\" fill=\"#5566dd\" stroke=\"none\" visibility=\"visible\"/></g>"
		"<g display=\"none\"><path d=\"M13 8 L15 8 L15 9 Z\" fill=\"#12feef\" stroke=\"none\" visibility=\"visible\"/></g>"
		"<rect x=\"22\" y=\"7\" width=\"8\" height=\"3\" fill=\"url(#gradChain)\"/>"
		"<rect x=\"1\" y=\"6\" width=\"2\" height=\"1\" fill=\"url(#currentStopGrad)\"/>"
		"<rect x=\"4\" y=\"6\" width=\"2\" height=\"1\" fill=\"url(#cssStopGrad)\"/>"
		"<circle cx=\"15\" cy=\"12\" r=\"2\" fill=\"url(#radAlias)\"/>"
		"<path d=\"M2 12 C8 8 12 15 18 11\" fill=\"none\" stroke=\"url(#gradChain)\" stroke-width=\"1.5\"/>"
		"<path d=\"M1 1 H7 V7 H1 Z M3 3 H5 V5 H3 Z\" fill=\"#446688\" fill-rule=\"evenodd\"/>"
		"<path d=\"M12 2 C18 2 18 12 24 12\" fill=\"none\" stroke=\"rgb(255, 64, 32)\" stroke-width=\"2\"/>"
		"<path d=\"M10 6 L12 6 L12 7 Z\" fill=\"rgb(100% 50% 0%)\" stroke=\"none\"/>"
		"<path d=\"M13 6 L15 6 L15 7 Z\" fill=\"rgba(18 52 86 / 100%)\" stroke=\"none\"/>"
		"<path d=\"M16 6 L18 6 L18 7 Z\" fill=\"#0f0f\" stroke=\"none\"/>"
		"<path d=\"M19 6 L21 6 L21 7 Z\" fill=\"#336699ff\" stroke=\"none\"/>"
		"<path d=\"M22 6 L24 6 L24 7 Z\" fill=\"hsl(210 100% 50%)\" stroke=\"none\"/>"
		"<path d=\"M25 6 L27 6 L27 7 Z\" fill=\"hsla(300, 100%, 50%, 1)\" stroke=\"none\"/>"
		"<path d=\"M28 6 L30 6 L30 7 Z\" fill=\"#fedcba\" opacity=\"100%\" stroke=\"none\"/>"
		"<path d=\"M30 6 L32 6 L32 7 Z\" fill=\"#55aa11\" fill-opacity=\"100%\" stroke=\"none\"/>"
		"<circle cx=\"28\" cy=\"8\" r=\"3\" fill=\"green\" opacity=\"0.5\"/>"
		"<rect x=\"23\" y=\"11\" width=\"8\" height=\"5\" fill=\"tomato\" clip-path=\"url(#smokeClip)\"/>"
		"<rect x=\"12\" y=\"1\" width=\"8\" height=\"5\" fill=\"royalblue\" clip-path=\"url(#smokeBoxClip)\"/>"
		"<rect x=\"20\" y=\"11\" width=\"8\" height=\"4\" fill=\"gold\" mask=\"url(#smokeMask)\"/>"
		"<path class=\"smokeAccent\" d=\"M2 14 L4 14 L4 15 Z\"/>"
		"<path id=\"smokeId\" d=\"M6 14 L8 14 L8 15 Z\"/>"
		"<polygon points=\"10,14 12,14 12,15\"/>"
		"<path class=\"smokeCurrent\" d=\"M14 14 L16 14 L16 15 Z\"/>"
		"<path color=\"LightSteelBlue\" fill=\"currentColor\" d=\"M18 14 L20 14 L20 15 Z\"/>"
		"<path class=\"smokeCompound\" d=\"M22 14 L24 14 L24 15 Z\"/>"
		"<path class=\"smokePair extra\" d=\"M25 14 L27 14 L27 15 Z\"/>"
		"<path id=\"smokeId2\" class=\"extra\" d=\"M29 14 L31 14 L31 15 Z\"/>"
		"<path class=\"smokeCData\" d=\"M1 12 L3 12 L3 13 Z\"/>"
		"<path class=\"smokeSpecific\" d=\"M4 12 L6 12 L6 13 Z\"/>"
		"<path class=\"smokeAttr\" fill=\"#000000\" d=\"M7 12 L9 12 L9 13 Z\"/>"
		"<defs><symbol id=\"lateSmokeSymbol\" viewBox=\"0 0 10 5\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"10\" height=\"5\" fill=\"#778899\" stroke=\"none\"/></symbol></defs>"
		"</svg>";
	static const char svg_units[] =
		"<svg width=\"25.4mm\" height=\"1in\">"
		"<rect x=\"10%\" y=\"25%\" width=\"50%\" height=\"25%\" fill=\"#123456\" stroke=\"#654321\" stroke-width=\"5%\"/>"
		"</svg>";
	xge_svg svg;
	xge_svg cached_a;
	xge_svg cached_b;
	xge_svg cached_c;
	xge_rect_t viewbox;
	xge_rect_t viewport;
	const char* cache_path = "build\\xge_shapeex_svg_smoke_tmp.svg";
	FILE* file;
	int ret;

	svg = NULL;
	cached_a = NULL;
	cached_b = NULL;
	cached_c = NULL;
	ret = xgeSvgCreate(&svg);
	if ( !check((ret == XGE_OK) && (svg != NULL), "SVG create") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_text, (int)strlen(svg_text));
	if ( !check(ret == XGE_OK, "SVG load memory") ) return 0;
	ret = xgeSvgGetViewBox(svg, &viewbox);
	if ( !check((ret == XGE_OK) && (viewbox.fX == 0.0f) && (viewbox.fY == 0.0f) && (viewbox.fW == 32.0f) && (viewbox.fH == 16.0f), "SVG viewBox") ) return 0;
	ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	if ( !check((ret == XGE_OK) && (viewport.fX == 0.0f) && (viewport.fY == 32.0f) && (viewport.fW == 64.0f) && (viewport.fH == 32.0f), "SVG meet viewport") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "none");
	if ( ret == XGE_OK ) {
		ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){1.0f, 2.0f, 64.0f, 64.0f}, &viewport);
	}
	if ( !check((ret == XGE_OK) && (viewport.fX == 1.0f) && (viewport.fY == 2.0f) && (viewport.fW == 64.0f) && (viewport.fH == 64.0f), "SVG none viewport") ) return 0;
	ret = xgeSvgSetPreserveAspectRatio(svg, "xMaxYMin slice");
	if ( ret == XGE_OK ) {
		ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 64.0f, 64.0f}, &viewport);
	}
	if ( !check((ret == XGE_OK) && (viewport.fX == -64.0f) && (viewport.fY == 0.0f) && (viewport.fW == 128.0f) && (viewport.fH == 64.0f), "SVG slice viewport") ) return 0;
	ret = xgeSvgLoadMemory(svg, svg_units, (int)strlen(svg_units));
	if ( !check(ret == XGE_OK, "SVG units load memory") ) return 0;
	ret = xgeSvgGetViewBox(svg, &viewbox);
	if ( !check((ret == XGE_OK) && (viewbox.fW > 95.9f) && (viewbox.fW < 96.1f) && (viewbox.fH > 95.9f) && (viewbox.fH < 96.1f), "SVG root length units") ) return 0;
	ret = xgeSvgGetDrawViewport(svg, (xge_rect_t){0.0f, 0.0f, 192.0f, 96.0f}, &viewport);
	if ( !check((ret == XGE_OK) && (viewport.fX > 47.9f) && (viewport.fX < 48.1f) && (viewport.fY == 0.0f) && (viewport.fW > 95.9f) && (viewport.fW < 96.1f) && (viewport.fH > 95.9f) && (viewport.fH < 96.1f), "SVG clear resets aspect") ) return 0;

	file = fopen(cache_path, "wb");
	if ( !check(file != NULL, "SVG cache file open") ) return 0;
	if ( fwrite(svg_text, 1, strlen(svg_text), file) != strlen(svg_text) ) {
		fclose(file);
		printf("xge smoke failed: SVG cache file write\n");
		return 0;
	}
	fclose(file);
	ret = xgeSvgLoadCached(cache_path, &cached_a);
	if ( !check((ret == XGE_OK) && (cached_a != NULL), "SVG cached load A") ) return 0;
	ret = xgeSvgLoadCached(cache_path, &cached_b);
	if ( !check((ret == XGE_OK) && (cached_b == cached_a), "SVG cached load B") ) return 0;
	ret = xgeSvgCacheInvalidate(cache_path);
	if ( !check(ret == XGE_OK, "SVG cache invalidate") ) return 0;
	ret = xgeSvgLoadCached(cache_path, &cached_c);
	if ( !check((ret == XGE_OK) && (cached_c != NULL), "SVG cached reload") ) return 0;
	xgeSvgDestroy(cached_a);
	xgeSvgDestroy(cached_b);
	xgeSvgDestroy(cached_c);
	xgeSvgDestroy(svg);
	xgeSvgCacheClear();
	remove(cache_path);
	return 1;
}

int main(void)
{
	uint32_t color;
	xge_color_t unpacked;
	xge_frame_stats_t stats;

	color = xgeColorRGBA(1, 2, 3, 4);
	if ( !check(color == 0x01020304u, "color pack") ) return 1;
	unpacked = xgeColorUnpack(color);
	if ( !check((unpacked.fR > 0.003f) && (unpacked.fR < 0.005f) &&
	            (unpacked.fG > 0.007f) && (unpacked.fG < 0.009f) &&
	            (unpacked.fB > 0.011f) && (unpacked.fB < 0.013f) &&
	            (unpacked.fA > 0.015f) && (unpacked.fA < 0.017f), "color unpack") ) return 1;
	stats = xgeFrameStatsGet();
	if ( !check(stats.iFrameCount == 0, "uninitialized frame count") ) return 1;
	if ( !test_shape_ex() ) return 1;
	if ( !test_svg() ) return 1;
	printf("xge smoke passed\n");
	return 0;
}
