#include "xge.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static const char* find_font(void)
{
	static const char* paths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf"
	};
	FILE* file;
	int i;

	for ( i = 0; i < (int)(sizeof(paths) / sizeof(paths[0])); i++ ) {
		file = fopen(paths[i], "rb");
		if ( file != NULL ) {
			fclose(file);
			return paths[i];
		}
	}
	return NULL;
}

static int fail(const char* message)
{
	printf("emoji test failed: %s\n", message);
	return 1;
}

int main(void)
{
	static const char text_mixed[] = "A" "\xF0\x9F\x98\x80" "B";
	static const char text_family[] =
		"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D"
		"\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6";
	static const char text_heart[] = "\xE2\x9D\xA4";
	static const char text_heart_color[] = "\xE2\x9D\xA4\xEF\xB8\x8F";
	static const char text_custom[] = "\xF0\x9F\xA7\xAA";
	static const char custom_svg[] =
		"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 36 36\">"
		"<path fill=\"#7dd3fc\" d=\"M12 3h12v3l-2 2v7l8 14c1 2-1 4-3 4H9c-2 0-4-2-3-4l8-14V8l-2-2z\"/>"
		"<path fill=\"#2563eb\" d=\"M10 26h16l3 5H7z\"/></svg>";
	const char* font_path;
	xge_emoji_pack pack;
	xge_emoji_pack custom;
	xge_emoji_match_t match;
	xge_emoji_metrics_t emoji_metrics;
	xge_font_t font;
	xge_font_metrics_t font_metrics;
	xge_text_shape_desc_t desc;
	xge_glyph_run_t stable_run;
	xge_glyph_run_t expand_run;
	xge_vec2_t measured;
	uint32_t cluster;
	uint32_t custom_id;
	int trailing;
	int i;
	int found_emoji;

	pack = NULL;
	if ( xgeEmojiPackLoadBuiltin(&pack) != XGE_OK || pack == NULL ) return fail("load built-in pack");
	memset(&match, 0, sizeof(match));
	match.iSize = sizeof(match);
	if ( xgeEmojiPackMatch(pack, text_family, -1, &match) != XGE_OK ||
	     match.iTextSize != (int)strlen(text_family) ) {
		xgeEmojiPackFree(pack);
		return fail("longest ZWJ sequence match");
	}
	memset(&match, 0, sizeof(match));
	match.iSize = sizeof(match);
	if ( xgeEmojiPackMatch(pack, text_heart_color, -1, &match) != XGE_OK ||
	     match.iTextSize != (int)strlen(text_heart_color) ) {
		xgeEmojiPackFree(pack);
		return fail("variation selector match");
	}

	font_path = find_font();
	if ( font_path == NULL ) {
		xgeEmojiPackFree(pack);
		printf("emoji test skipped: no test font\n");
		return 0;
	}
	memset(&font, 0, sizeof(font));
	if ( xgeFontLoad(&font, font_path, 24.0f) != XGE_OK ) {
		xgeEmojiPackFree(pack);
		return fail("font load");
	}
	memset(&font_metrics, 0, sizeof(font_metrics));
	if ( xgeFontGetMetrics(&font, &font_metrics) != XGE_OK ) {
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("font metrics");
	}

	memset(&desc, 0, sizeof(desc));
	memset(&stable_run, 0, sizeof(stable_run));
	desc.iSize = sizeof(desc);
	desc.pFont = &font;
	desc.sText = text_mixed;
	desc.iTextSize = -1;
	desc.iFlags = XGE_TEXT_SHAPE_DEFAULT;
	desc.pEmojiPack = pack;
	desc.iEmojiLinePolicy = XGE_EMOJI_LINE_STABLE;
	if ( xgeTextShape(&desc, &stable_run) != XGE_OK || stable_run.iGlyphCount != 3 ||
	     stable_run.pGlyphs[1].iItemKind != XGE_TEXT_ITEM_EMOJI ||
	     stable_run.pGlyphs[1].iCluster != 1u || stable_run.pGlyphs[1].iClusterEnd != 5u ) {
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("mixed run shape and cluster boundaries");
	}
	if ( fabsf(stable_run.fLineHeight - font_metrics.fLineHeight) > 0.01f ) {
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("stable line height");
	}
	measured = xgeTextMeasure(&font, text_mixed);
	if ( fabsf(measured.fX - stable_run.fWidth) > 0.01f ||
	     fabsf(measured.fY - stable_run.fHeight) > 0.01f ) {
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("measure and shaped run consistency");
	}
	cluster = 0;
	trailing = 0;
	if ( xgeGlyphRunHitTest(
			&stable_run,
			stable_run.pGlyphs[0].fAdvanceX + stable_run.pGlyphs[1].fAdvanceX * 0.75f,
			1.0f, &cluster, &trailing
		) != XGE_OK || cluster != 1u || trailing != 1 ) {
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("emoji hit test");
	}

	memset(&expand_run, 0, sizeof(expand_run));
	desc.iEmojiLinePolicy = XGE_EMOJI_LINE_EXPAND;
	desc.fEmojiScale = 1.8f;
	if ( xgeTextShape(&desc, &expand_run) != XGE_OK ||
	     expand_run.fLineHeight <= stable_run.fLineHeight ) {
		xgeGlyphRunFree(&expand_run);
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("expanded emoji line height");
	}
	xgeGlyphRunFree(&expand_run);

	memset(&expand_run, 0, sizeof(expand_run));
	desc.sText = text_family;
	desc.iEmojiLinePolicy = XGE_EMOJI_LINE_STABLE;
	desc.fEmojiScale = 1.0f;
	if ( xgeTextShape(&desc, &expand_run) != XGE_OK || expand_run.iGlyphCount != 1 ||
	     expand_run.pGlyphs[0].iClusterEnd != (uint32_t)strlen(text_family) ) {
		xgeGlyphRunFree(&expand_run);
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("ZWJ sequence is one text item");
	}
	xgeGlyphRunFree(&expand_run);

	memset(&expand_run, 0, sizeof(expand_run));
	desc.sText = text_heart;
	if ( xgeTextShape(&desc, &expand_run) != XGE_OK ) {
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("text-default heart shape");
	}
	found_emoji = 0;
	for ( i = 0; i < expand_run.iGlyphCount; i++ ) {
		if ( expand_run.pGlyphs[i].iItemKind == XGE_TEXT_ITEM_EMOJI ) found_emoji = 1;
	}
	xgeGlyphRunFree(&expand_run);
	if ( found_emoji ) {
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("AUTO respects text-default presentation");
	}
	memset(&expand_run, 0, sizeof(expand_run));
	desc.sText = text_heart_color;
	if ( xgeTextShape(&desc, &expand_run) != XGE_OK || expand_run.iGlyphCount != 1 ||
	     expand_run.pGlyphs[0].iItemKind != XGE_TEXT_ITEM_EMOJI ) {
		xgeGlyphRunFree(&expand_run);
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("VS16 selects color presentation");
	}
	xgeGlyphRunFree(&expand_run);

	custom = NULL;
	custom_id = 0;
	memset(&emoji_metrics, 0, sizeof(emoji_metrics));
	emoji_metrics.iSize = sizeof(emoji_metrics);
	emoji_metrics.fAdvanceEm = 1.1f;
	emoji_metrics.fWidthEm = 1.0f;
	emoji_metrics.fHeightEm = 1.0f;
	emoji_metrics.fBaselineRatio = 0.82f;
	if ( xgeEmojiPackCreate(&custom) != XGE_OK ||
	     xgeEmojiPackAddSvgMemory(custom, text_custom, custom_svg, (int)strlen(custom_svg), &emoji_metrics, &custom_id) != XGE_OK ||
	     custom_id == 0 ) {
		xgeEmojiPackFree(custom);
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("custom pack");
	}
	memset(&expand_run, 0, sizeof(expand_run));
	desc.sText = text_custom;
	desc.pEmojiPack = custom;
	desc.iEmojiPresentation = XGE_EMOJI_PRESENTATION_COLOR;
	if ( xgeTextShape(&desc, &expand_run) != XGE_OK || expand_run.iGlyphCount != 1 ||
	     expand_run.pGlyphs[0].iEmojiId != custom_id ) {
		xgeGlyphRunFree(&expand_run);
		xgeEmojiPackFree(custom);
		xgeGlyphRunFree(&stable_run);
		xgeFontFree(&font);
		xgeEmojiPackFree(pack);
		return fail("custom pack shaping");
	}

	xgeGlyphRunFree(&expand_run);
	xgeEmojiPackFree(custom);
	xgeGlyphRunFree(&stable_run);
	xgeFontFree(&font);
	xgeEmojiPackFree(pack);
	printf("emoji test passed\n");
	return 0;
}
