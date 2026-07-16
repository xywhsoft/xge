#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_file(const char* path, void** data, int* size)
{
	FILE* file;
	long length;
	void* bytes;

	file = fopen(path, "rb");
	if ( file == NULL ) return 0;
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);
	if ( (length <= 0) || (length > INT32_MAX) ) {
		fclose(file);
		return 0;
	}
	bytes = malloc((size_t)length);
	if ( (bytes == NULL) || (fread(bytes, 1, (size_t)length, file) != (size_t)length) ) {
		free(bytes);
		fclose(file);
		return 0;
	}
	fclose(file);
	*data = bytes;
	*size = (int)length;
	return 1;
}

static const char* find_font(void)
{
	static const char* fonts[] = {
		"C:\\Windows\\Fonts\\consola.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf"
	};
	FILE* file;
	int i;

	for ( i = 0; i < (int)(sizeof(fonts) / sizeof(fonts[0])); i++ ) {
		file = fopen(fonts[i], "rb");
		if ( file != NULL ) {
			fclose(file);
			return fonts[i];
		}
	}
	return NULL;
}

int main(void)
{
	const char* font_path;
	const char* scan;
	const char text[] = "AV\nTest";
	void* font_data;
	void* xrf_data;
	int font_size;
	int xrf_size;
	uint32_t codepoint;
	uint32_t cluster;
	int trailing;
	xge_font_face face;
	xge_font_face resolved;
	xge_font_family family;
	xge_font_t font;
	xge_font_t cached_font;
	xge_font_t hybrid_font;
	xge_font_instance_desc_t font_desc;
	xge_font_face_info_t face_info;
	xge_font_metrics_t metrics;
	xge_font_cache_stats_t stats;
	xge_font_cache_stats_t hybrid_before;
	xge_font_cache_stats_t hybrid_after;
	xge_text_shape_desc_t shape_desc;
	xge_glyph_run_t run;
	xge_glyph_t glyph;
	xge_shape_ex outline;
	const uint8_t* outline_commands;
	const xge_vec2_t* outline_points;
	int outline_command_count;
	int outline_point_count;
	xge_xrf_header_t* xrf_header;
	int result;

	scan = "\xF0\x9F\x98\x80";
	if ( (xgeTextUTF8Next(&scan, &codepoint) != XGE_OK) || (codepoint != 0x1F600u) || (*scan != 0) ) {
		printf("text foundation failed: UTF-8 U+1F600\n");
		return 1;
	}
	font_path = find_font();
	if ( font_path == NULL ) {
		printf("text foundation skipped: no test font\n");
		return 0;
	}
	font_data = NULL;
	font_size = 0;
	if ( !load_file(font_path, &font_data, &font_size) ) {
		printf("text foundation failed: read font\n");
		return 1;
	}
	face = NULL;
	result = xgeFontFaceLoadMemory(&face, font_data, font_size, NULL);
	free(font_data);
	if ( (result != XGE_OK) || (face == NULL) ) {
		printf("text foundation failed: face load %d\n", result);
		return 1;
	}
	memset(&face_info, 0, sizeof(face_info));
	if ( (xgeFontFaceGetInfo(face, &face_info) != XGE_OK) || (face_info.iSourceHash == 0) || (face_info.iWeight <= 0) ) {
		printf("text foundation failed: face metadata\n");
		xgeFontFaceFree(face);
		return 1;
	}
	family = NULL;
	resolved = NULL;
	if ( (xgeFontFamilyCreate(&family) != XGE_OK) || (xgeFontFamilyAddFace(family, face) != XGE_OK) ||
	     (xgeFontFamilyResolve(family, XGE_FONT_WEIGHT_NORMAL, XGE_FONT_SLANT_NORMAL, &resolved) != XGE_OK) || (resolved != face) ) {
		printf("text foundation failed: family resolve\n");
		xgeFontFaceFree(resolved);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	xgeFontFaceFree(resolved);
	memset(&font, 0, sizeof(font));
	memset(&font_desc, 0, sizeof(font_desc));
	font_desc.iSize = sizeof(font_desc);
	font_desc.fPixelSize = 18.0f;
	if ( xgeFontCreate(&font, face, &font_desc) != XGE_OK ) {
		printf("text foundation failed: font create\n");
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	memset(&metrics, 0, sizeof(metrics));
	if ( (xgeFontGetMetrics(&font, &metrics) != XGE_OK) || (metrics.fLineHeight <= 0.0f) || (metrics.fUnderlineThickness <= 0.0f) ) {
		printf("text foundation failed: metrics\n");
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	memset(&shape_desc, 0, sizeof(shape_desc));
	memset(&run, 0, sizeof(run));
	shape_desc.iSize = sizeof(shape_desc);
	shape_desc.pFont = &font;
	shape_desc.sText = text;
	shape_desc.iFlags = XGE_TEXT_SHAPE_DEFAULT;
	if ( (xgeTextShape(&shape_desc, &run) != XGE_OK) || (run.iGlyphCount != 7) ||
	     (run.fWidth <= 0.0f) || (run.fHeight < metrics.fLineHeight * 2.0f) ) {
		printf("text foundation failed: shape count=%d width=%.2f height=%.2f\n", run.iGlyphCount, run.fWidth, run.fHeight);
		xgeGlyphRunFree(&run);
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	cluster = 0;
	trailing = 0;
	if ( (xgeGlyphRunHitTest(&run, 0.0f, metrics.fLineHeight + 1.0f, &cluster, &trailing) != XGE_OK) || (cluster < 3u) ) {
		printf("text foundation failed: hit test cluster=%u\n", (unsigned int)cluster);
		xgeGlyphRunFree(&run);
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	outline = NULL;
	outline_commands = NULL;
	outline_points = NULL;
	outline_command_count = 0;
	outline_point_count = 0;
	if ( (xgeShapeExCreate(&outline) != XGE_OK) ||
	     (xgeGlyphRunAppendShapeEx(&run, outline, 10.0f, 20.0f) != XGE_OK) ||
	     (xgeShapeExGetPath(outline, &outline_commands, &outline_command_count, &outline_points, &outline_point_count) != XGE_OK) ||
	     (outline_commands == NULL) || (outline_points == NULL) ||
	     (outline_command_count <= 0) || (outline_point_count <= 0) ) {
		printf("text foundation failed: vector outline commands=%d points=%d\n", outline_command_count, outline_point_count);
		xgeShapeExDestroy(outline);
		xgeGlyphRunFree(&run);
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	xgeShapeExDestroy(outline);
	xgeGlyphRunFree(&run);
	if ( xgeFontGlyphAtlasGet(&font, 'A', &glyph) != XGE_OK ) {
		printf("text foundation failed: atlas glyph\n");
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	memset(&stats, 0, sizeof(stats));
	if ( (xgeFontCacheGetStats(&font, &stats) != XGE_OK) || (stats.iGlyphCount == 0) || (stats.iAtlasPageCount == 0) || (stats.iBucketCount < 256) ) {
		printf("text foundation failed: cache stats glyphs=%u pages=%u buckets=%u\n",
		       (unsigned int)stats.iGlyphCount, (unsigned int)stats.iAtlasPageCount, (unsigned int)stats.iBucketCount);
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	xrf_data = NULL;
	xrf_size = 0;
	if ( xgeFontBuildXRFMemory(&font, 32, 96, &xrf_data, &xrf_size) != XGE_OK ) {
		printf("text foundation failed: XRF build\n");
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	xrf_header = (xge_xrf_header_t*)xrf_data;
	if ( (xrf_header->iVersion != XGE_XRF_VERSION) || (xrf_header->iHeaderSize != sizeof(xge_xrf_header_v2_t)) ) {
		printf("text foundation failed: XRF v2 header\n");
		xgeMemoryFree(xrf_data);
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	memset(&cached_font, 0, sizeof(cached_font));
	if ( xgeFontLoadXRFMemory(&cached_font, xrf_data, xrf_size) != XGE_OK ) {
		printf("text foundation failed: XRF load\n");
		xgeMemoryFree(xrf_data);
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	xgeMemoryFree(xrf_data);
	memset(&shape_desc, 0, sizeof(shape_desc));
	memset(&run, 0, sizeof(run));
	shape_desc.iSize = sizeof(shape_desc);
	shape_desc.pFont = &cached_font;
	shape_desc.sText = "ABC";
	shape_desc.iFlags = XGE_TEXT_SHAPE_DEFAULT;
	if ( (xgeTextShape(&shape_desc, &run) != XGE_OK) || (run.iGlyphCount != 3) || (run.fWidth <= 0.0f) ) {
		printf("text foundation failed: cached shape\n");
		xgeGlyphRunFree(&run);
		xgeFontFree(&cached_font);
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	xgeGlyphRunFree(&run);
	xgeFontFree(&cached_font);
	if ( xgeFontSaveXRF(&font, "build\\xge_text_foundation.xrf", 32, 96) != XGE_OK ) {
		printf("text foundation failed: hybrid cache save\n");
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	memset(&hybrid_font, 0, sizeof(hybrid_font));
	if ( xgeFontLoadCached(&hybrid_font, font_path, "build\\xge_text_foundation.xrf", 18.0f, 32, 96) != XGE_OK ) {
		printf("text foundation failed: hybrid cache load\n");
		remove("build\\xge_text_foundation.xrf");
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	memset(&hybrid_before, 0, sizeof(hybrid_before));
	memset(&hybrid_after, 0, sizeof(hybrid_after));
	(void)xgeFontCacheGetStats(&hybrid_font, &hybrid_before);
	if ( (xgeFontGlyphAtlasGet(&hybrid_font, 0x00E9u, &glyph) != XGE_OK) ||
	     (xgeFontCacheGetStats(&hybrid_font, &hybrid_after) != XGE_OK) ||
	     (hybrid_after.iAtlasPageCount <= hybrid_before.iAtlasPageCount) ) {
		printf("text foundation failed: hybrid miss pages=%u -> %u\n",
		       (unsigned int)hybrid_before.iAtlasPageCount, (unsigned int)hybrid_after.iAtlasPageCount);
		xgeFontFree(&hybrid_font);
		remove("build\\xge_text_foundation.xrf");
		xgeFontFree(&font);
		xgeFontFamilyFree(family);
		xgeFontFaceFree(face);
		return 1;
	}
	xgeFontFree(&hybrid_font);
	remove("build\\xge_text_foundation.xrf");
	xgeFontFree(&font);
	xgeFontFamilyFree(family);
	xgeFontFaceFree(face);
	printf("text foundation passed\n");
	return 0;
}
