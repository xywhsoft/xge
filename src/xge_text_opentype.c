/* Included by xge_text_run.c. The integration is C; HarfBuzz is built
 * separately and supplies the font's complete GSUB/GPOS shaping machinery. */
#define ub_bsearch __xgeOtBsearch
#define ub_get_next_char_utf8 __xgeOtNextUtf8
#define ub_get_next_char_utf16 __xgeOtNextUtf16
#define ub_get_next_char_utf32 __xgeOtNextUtf32
#define ub_is_extended_pictographic __xgeOtPictographic
#include "../lib/libunibreak/src/unibreakdef.c"
#include "../lib/libunibreak/src/emojidef.c"
#include "../lib/libunibreak/src/graphemebreak.h"
#include "xge_unicode_grapheme.h"
#include "xge_unicode_script.h"
static void __xgeOtGraphemesUtf8(const utf8_t* text, size_t bytes, const char* language, char* map)
{
	(void)language;
	__xgeGraphemeMap(text, bytes, map, __xgeOtNextUtf8);
}
#undef ub_bsearch
#undef ub_get_next_char_utf8
#undef ub_get_next_char_utf16
#undef ub_get_next_char_utf32
#undef ub_is_extended_pictographic

#include "xge_text_opentype_caret_validation.inl"
#include "xge_text_opentype_carets.inl"

static int __xgeOtStrongScript(hb_script_t script)
{
	return script != HB_SCRIPT_COMMON && script != HB_SCRIPT_INHERITED && script != HB_SCRIPT_UNKNOWN;
}
#include "xge_text_opentype_fallback.inl"

static int __xgeGlyphRunOpenType(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run,
	const char* begin, const char* limit, const char** next, float* line_width, float* line_right, float* line_gap)
{
	xge_glyph_run_backend_t* backend = run->pBackend;
	xge_font font, candidate;
	hb_font_t* shape_font;
	hb_buffer_t* buffer;
	hb_glyph_info_t* infos;
	hb_glyph_position_t* positions;
	hb_script_t script = desc->iScript ? hb_script_from_iso15924_tag(desc->iScript) : HB_SCRIPT_UNKNOWN;
	hb_feature_t kern = {HB_TAG('k','e','r','n'), 0, 0, (unsigned)-1};
	const char *scan = begin, *end = begin;
	uint32_t cp;
	unsigned count, i;
	int result;
	result = __xgeOtEnsureScripts(desc, run);
	if (result != XGE_OK) return result;
	font = NULL;
	while ( scan < limit ) {
		const char* scalar = scan;
		result = __xgeTextUTF8DecodeBounded(&scan, limit, &cp);
		if ( result != XGE_OK ) return result;
		if ( cp == '\r' || cp == '\n' || cp == '\t' ||

#if XGE_ENABLE_EMOJI
			((desc->iFlags & XGE_TEXT_SHAPE_EMOJI) && __xgeEmojiMayStart(cp) && !__xgeOtIgnorable(cp))
#else
			0
#endif
 ) break;
		result = __xgeOtFallbackFont(desc, run, scalar, limit, &candidate);
		if (result != XGE_OK) return result;
		if ( !candidate || !__xgeFontFace(candidate) || (font && candidate != font) ) break;
		{
			hb_script_t actual = __xgeOtItemScript(desc, run, scalar, cp);
			if ( !desc->iScript && __xgeOtStrongScript(actual) ) {
				if (__xgeOtStrongScript(script) && actual != script && !backend->pGraphemeBreaks) {
					result = __xgeOtEnsureGraphemes(desc, run);
					if (result != XGE_OK) return result;
				}
				int joined = backend->pGraphemeBreaks && scalar > desc->sText &&
					backend->pGraphemeBreaks[scalar - desc->sText - 1] != GRAPHEMEBREAK_BREAK;
				if ( __xgeOtStrongScript(script) && actual != script && !joined ) break;
				if (!joined || !__xgeOtStrongScript(script)) script = actual;
			}
		}
		font = candidate; end = scan;
	}
	if ( end == begin ) return XGE_ERROR_NOT_FOUND;
	shape_font = __xgeFontShapeFont(font);
	if ( !shape_font ) return XGE_ERROR_OUT_OF_MEMORY;
	buffer = hb_buffer_create();
	if ( buffer == hb_buffer_get_empty() ) return XGE_ERROR_OUT_OF_MEMORY;
	/* The caller supplies a resolved item direction. Reorder glyph groups below
	 * to retain the shared XUI ascending logical-cluster contract. */
	result = __xgeOtBufferItem(buffer, desc, begin, end, script);
	if (result != XGE_OK) goto done;
	kern.value = (desc->iFlags & XGE_TEXT_SHAPE_KERNING) != 0;
	hb_shape(shape_font, buffer, &kern, 1);
	if ( !hb_buffer_allocation_successful(buffer) ) { result = XGE_ERROR_OUT_OF_MEMORY; goto done; }
	infos = hb_buffer_get_glyph_infos(buffer, &count);
	positions = hb_buffer_get_glyph_positions(buffer, NULL);
	result = __xgeOtRebaseClusters(desc, begin, end, infos, count);
	if (result != XGE_OK) goto done;
	if ( desc->iFlags & XGE_TEXT_SHAPE_RTL ) {
		unsigned left, right;
		for ( left = 0, right = count; left < right && left < --right; left++ ) {
			hb_glyph_info_t info = infos[left]; hb_glyph_position_t position = positions[left];
			infos[left] = infos[right]; infos[right] = info; positions[left] = positions[right]; positions[right] = position;
		}
		for ( left = 0; left < count; ) {
			unsigned end_group = left + 1, begin_group = left;
			while ( end_group < count && infos[end_group].cluster == infos[left].cluster ) end_group++;
			right = end_group;
			while ( begin_group < right && begin_group < --right ) {
				hb_glyph_info_t info = infos[begin_group]; hb_glyph_position_t position = positions[begin_group];
				infos[begin_group] = infos[right]; infos[right] = info; positions[begin_group] = positions[right]; positions[right] = position; begin_group++;
			}
			left = end_group;
		}
	}
	if ( count > (unsigned)(INT_MAX - run->iGlyphCount) ) { result = XGE_ERROR_OUT_OF_MEMORY; goto done; }
	{
		void* glyphs = run->pGlyphs;
		result = __xgeOtReserve(&glyphs, &backend->iGlyphCapacity,
			run->iGlyphCount + (int)count, sizeof(*run->pGlyphs));
		if ( result == XGE_OK ) run->pGlyphs = glyphs;
	}
	if ( result != XGE_OK ) goto done;
	result = __xgeGlyphRunKeepFont(backend, font);
	if ( result != XGE_OK ) goto done;
	if ( font->fAscent > run->fAscent ) run->fAscent = font->fAscent;
	if ( font->fDescent < run->fDescent ) run->fDescent = font->fDescent;
	if ( font->fLineGap > *line_gap ) *line_gap = font->fLineGap;
	run->fLineHeight = run->fAscent - run->fDescent + *line_gap;
	for ( i = 0; i < count; ) {
		unsigned finish = i + 1, j;
		uint32_t cluster_end;
		float advance = 0;
		while ( finish < count && infos[finish].cluster == infos[i].cluster ) finish++;
		cluster_end = finish < count ? infos[finish].cluster : (uint32_t)(end - desc->sText);
		if ( infos[i].cluster < (uint32_t)(begin - desc->sText) || infos[i].cluster >= cluster_end ||
			cluster_end > (uint32_t)(end - desc->sText) ) { result = XGE_ERROR_INVALID_STATE; goto done; }
		for ( j = i; j < finish; j++ ) advance += (float)positions[j].x_advance * font->fScale;
		result = __xgeOtCarets(desc, run, font, infos[i].cluster, cluster_end,
			infos + i, positions + i, finish - i, advance);
		if ( result != XGE_OK ) goto done;
		for ( j = i; j < finish; j++ ) {
			xge_glyph_position_t* item = &run->pGlyphs[run->iGlyphCount++];
			xge_glyph_metrics_t metrics;
			const char* logical = desc->sText + infos[j].cluster;
			memset(item, 0, sizeof(*item));
			result = __xgeTextUTF8DecodeBounded(&logical, limit, &item->iCodepoint);
			if ( result != XGE_OK ) goto done;
			item->iCluster = infos[j].cluster; item->iClusterEnd = cluster_end;
			item->iGlyph = (int)infos[j].codepoint; item->pFont = font; item->iItemKind = XGE_TEXT_ITEM_GLYPH;
			item->fAdvanceX = (float)positions[j].x_advance * font->fScale;
			item->fOffsetX = (float)positions[j].x_offset * font->fScale;
			item->fOffsetY = -(float)positions[j].y_offset * font->fScale;
			result = xgeFontGlyphGetByIndex(font, item->iGlyph, &metrics);
			if ( result != XGE_OK ) goto done;
			if ( *line_width + item->fOffsetX + metrics.fX1 > *line_right )
				*line_right = *line_width + item->fOffsetX + metrics.fX1;
			*line_width += item->fAdvanceX;
			if ( *line_width > *line_right ) *line_right = *line_width;
		}
		i = finish;
	}
	*next = end;
	result = XGE_OK;
done:
	hb_buffer_destroy(buffer);
	return result;
}
