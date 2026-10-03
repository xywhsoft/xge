/* Ordered SFNT fallback keeps combining clusters and contextual words in one
 * font whenever possible. Probe nominal misses through HB as NFC/GSUB can
 * render a sequence even when its individual cmap entries are absent. */
#include "xge_text_ignorable_data.inc"
#include "xge_text_opentype_context.inl"
static int __xgeOtIgnorable(uint32_t cp)
{
	size_t low = 0, high = sizeof(__xgeOtIgnorableRanges) / sizeof(*__xgeOtIgnorableRanges);
	while (low < high) {
		size_t mid = low + (high - low) / 2;
		if (cp < __xgeOtIgnorableRanges[mid].first) high = mid;
		else if (cp > __xgeOtIgnorableRanges[mid].last) low = mid + 1;
		else return 1;
	}
	return 0;
}
static int __xgeOtEnsureScripts(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run)
{
	xge_glyph_run_backend_t* backend = run->pBackend;
	size_t i;
	if (backend->bScriptsChecked || desc->iScript) return XGE_OK;
    /* A complete ASCII item can infer Latin while collecting its glyphs.
     * A punctuation-only subitem cannot: resolve it in the supplied context. */
	for (i = 0; i < (size_t)desc->iContextSize; i++)
		if ((unsigned char)desc->sContext[i] >= 128) break;
    if (i < (size_t)desc->iContextSize || desc->iContextOffset ||
        desc->iTextSize != desc->iContextSize) {
		backend->pScriptMap = xrtMalloc((size_t)desc->iContextSize);
		if (!backend->pScriptMap) return XGE_ERROR_OUT_OF_MEMORY;
		backend->iScriptMapBytes=desc->iContextSize;
		if (!__xgeScriptMap((const unsigned char*)desc->sContext, (size_t)desc->iContextSize,
			backend->pScriptMap, __xgeOtNextUtf8)) {
			xrtFree(backend->pScriptMap); backend->pScriptMap = NULL;
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	backend->bScriptsChecked = 1; return XGE_OK;
}
static hb_script_t __xgeOtItemScript(const xge_text_shape_desc_t* desc,
	xge_glyph_run_t* run, const char* scalar, uint32_t cp)
{
	xge_glyph_run_backend_t* backend = run->pBackend;
	if (desc->iScript) return hb_script_from_iso15924_tag(desc->iScript);
	if (backend->pScriptMap) return (hb_script_t)__xgeScriptTag(
		backend->pScriptMap[desc->iContextOffset + (scalar - desc->sText)]);
	return hb_unicode_script(hb_unicode_funcs_get_default(), cp);
}
/* One source of buffer properties for both coverage probes and final shaping.
 * HB sees the full paragraph, but all public offsets stay item-relative. */
static int __xgeOtBufferItem(hb_buffer_t* buffer, const xge_text_shape_desc_t* desc,
	const char* begin, const char* end, hb_script_t script)
{
	unsigned offset = (unsigned)(desc->iContextOffset + (begin - desc->sText));
	int bytes = (int)(end - begin);
	hb_buffer_set_direction(buffer, desc->iFlags & XGE_TEXT_SHAPE_RTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
	hb_buffer_set_script(buffer, script);
	hb_buffer_set_language(buffer, hb_language_from_string(desc->sLanguage ? desc->sLanguage : "und", -1));
	hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
	hb_buffer_set_flags(buffer, (hb_buffer_flags_t)((offset == 0 ? HB_BUFFER_FLAG_BOT : 0) |
		(offset + (unsigned)bytes == (unsigned)desc->iContextSize ? HB_BUFFER_FLAG_EOT : 0)));
	{
		int result = __xgeOtBufferAddContext(buffer, desc, begin, end);
		if (result != XGE_OK) return result;
	}
	hb_buffer_guess_segment_properties(buffer);
	return XGE_OK;
}
static int __xgeOtRebaseClusters(const xge_text_shape_desc_t* desc,
	const char* begin, const char* end, hb_glyph_info_t* infos, unsigned count)
{
	unsigned i, first = (unsigned)(desc->iContextOffset + (begin - desc->sText));
	unsigned last = (unsigned)(desc->iContextOffset + (end - desc->sText));
	for (i = 0; i < count; i++) {
		if (infos[i].cluster < first || infos[i].cluster >= last) return XGE_ERROR_INVALID_STATE;
		infos[i].cluster -= (unsigned)desc->iContextOffset;
	}
	return XGE_OK;
}
static int __xgeOtEnsureGraphemes(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run)
{
	xge_glyph_run_backend_t* backend = run->pBackend;
	if (!backend->pGraphemeBreaks) {
		backend->pGraphemeBreaks = xrtMalloc((size_t)run->iTextSize);
		if (!backend->pGraphemeBreaks) return XGE_ERROR_OUT_OF_MEMORY;
		__xgeOtGraphemesUtf8((const utf8_t*)desc->sText, (size_t)run->iTextSize, NULL, backend->pGraphemeBreaks);
	}
	return XGE_OK;
}
static int __xgeOtWordScalar(uint32_t cp)
{
	hb_unicode_general_category_t category;
	if (__xgeOtIgnorable(cp)) return cp != 0x00adu; /* SHY may become visible at a line cut. */
	category = hb_unicode_general_category(hb_unicode_funcs_get_default(), cp);
	return (category >= HB_UNICODE_GENERAL_CATEGORY_LOWERCASE_LETTER &&
		category <= HB_UNICODE_GENERAL_CATEGORY_OTHER_NUMBER);
}
static const char* __xgeOtGraphemeEnd(const xge_text_shape_desc_t* desc,
	xge_glyph_run_t* run, const char* begin, const char* limit)
{
	xge_glyph_run_backend_t* backend = run->pBackend;
	const char* end = begin + 1;
	while (end < limit && backend->pGraphemeBreaks[end - desc->sText - 1] != GRAPHEMEBREAK_BREAK) end++;
	return end;
}
static int __xgeOtWordEnd(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run,
	const char* begin, const char* limit, const char** out)
{
	const char *at = begin, *end = begin; hb_script_t script = HB_SCRIPT_UNKNOWN;
	while (at < limit) {
		const char *scan = at, *grapheme_end = __xgeOtGraphemeEnd(desc, run, at, limit);
		hb_script_t next_script = script; int word = 1;
		while (scan < grapheme_end) {
			uint32_t cp; hb_script_t actual; int result = __xgeTextUTF8DecodeBounded(&scan, grapheme_end, &cp);
			if (result != XGE_OK) return result;
			if (!__xgeOtWordScalar(cp) || cp == '\r' || cp == '\n' || cp == '\t' ||
				
#if XGE_ENABLE_EMOJI
				((desc->iFlags & XGE_TEXT_SHAPE_EMOJI) && __xgeEmojiMayStart(cp) && !__xgeOtIgnorable(cp))
#else
				0
#endif
			) word = 0;
			actual = __xgeOtItemScript(desc, run, at, cp);
			if (__xgeOtStrongScript(actual) && !__xgeOtStrongScript(next_script)) next_script = actual;
		}
		/* Script changes are allowed inside one grapheme, never between words.
		 * The head script determines that grapheme's shaping context. */
		if (at != begin) {
			const char* head = at; uint32_t cp; hb_script_t actual;
			int result = __xgeTextUTF8DecodeBounded(&head, grapheme_end, &cp);
			if (result != XGE_OK) return result;
			actual = __xgeOtItemScript(desc, run, at, cp);
			if (__xgeOtStrongScript(script) && __xgeOtStrongScript(actual) && script != actual) word = 0;
		}
		if (!word) { if (at == begin) end = grapheme_end; break; }
		script = next_script; end = grapheme_end; at = end;
	}
	*out = end; return XGE_OK;
}
static int __xgeOtFontCovers(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run,
	xge_font candidate, const char* begin, const char* end, int* covered)
{
	hb_font_t* font; const char* scan = begin; int nominal = 1, result = XGE_OK;
	hb_script_t script = desc->iScript ? hb_script_from_iso15924_tag(desc->iScript) : HB_SCRIPT_UNKNOWN;
	hb_buffer_t* buffer;
	*covered = 0;
	if (!__xgeFontFace(candidate)) return XGE_OK;
	font = __xgeFontShapeFont(candidate); if (!font) return XGE_ERROR_OUT_OF_MEMORY;
	while (scan < end) {
		uint32_t cp; hb_codepoint_t glyph; hb_script_t actual; const char* scalar = scan;
		result = __xgeTextUTF8DecodeBounded(&scan, end, &cp); if (result != XGE_OK) return result;
		actual = __xgeOtItemScript(desc, run, scalar, cp);
		if (!desc->iScript && !__xgeOtStrongScript(script) && __xgeOtStrongScript(actual)) script = actual;
		if (!__xgeOtIgnorable(cp) && !hb_font_get_nominal_glyph(font, cp, &glyph)) nominal = 0;
	}
	if (nominal) { *covered = 1; return XGE_OK; }
	buffer = hb_buffer_create(); if (buffer == hb_buffer_get_empty()) return XGE_ERROR_OUT_OF_MEMORY;
	result = __xgeOtBufferItem(buffer, desc, begin, end, script);
	if (result != XGE_OK) goto done;
	{
		hb_feature_t kern = {HB_TAG('k','e','r','n'), !!(desc->iFlags & XGE_TEXT_SHAPE_KERNING), 0, (unsigned)-1};
		unsigned count, i; hb_glyph_info_t* infos;
		hb_shape(font, buffer, &kern, 1);
		if (!hb_buffer_allocation_successful(buffer)) { result = XGE_ERROR_OUT_OF_MEMORY; goto done; }
		infos = hb_buffer_get_glyph_infos(buffer, &count);
		result = __xgeOtRebaseClusters(desc, begin, end, infos, count);
		if (result != XGE_OK) goto done;
		*covered = 1;
		for (i = 0; i < count; i++) if (!infos[i].codepoint) {
			const char *at, *cluster_end; uint32_t cp; unsigned j;
			if (infos[i].cluster < (unsigned)(begin - desc->sText) || infos[i].cluster >= (unsigned)(end - desc->sText)) {
				result = XGE_ERROR_INVALID_STATE; goto done;
			}
			at = desc->sText + infos[i].cluster;
			cluster_end = end;
			for (j = 0; j < count; j++)
				if (infos[j].cluster > infos[i].cluster && desc->sText + infos[j].cluster < cluster_end)
					cluster_end = desc->sText + infos[j].cluster;
			/* A merged cluster's head can be ignorable while a later base/mark
			 * is not. Never excuse its .notdef by inspecting only that head. */
			while (at < cluster_end) {
				result = __xgeTextUTF8DecodeBounded(&at, cluster_end, &cp); if (result != XGE_OK) goto done;
				if (!__xgeOtIgnorable(cp)) { *covered = 0; break; }
			}
			if (!*covered) break;
		}
	}
done:
	hb_buffer_destroy(buffer); return result;
}
static int __xgeOtPickFont(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run,
	const char* begin, const char* end, xge_font* out)
{
	xge_font candidate = desc->pFont; unsigned depth;
	*out = NULL;
	for (depth = 0; candidate && depth < 32; depth++, candidate = candidate->pFallback) {
		int covered, result = __xgeOtFontCovers(desc, run, candidate, begin, end, &covered);
		if (result != XGE_OK) return result;
		if (covered) { *out = candidate; return XGE_OK; }
	}
	return XGE_OK;
}
#include "xge_text_opentype_word_context.inl"
static int __xgeOtFallbackFont(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run,
	const char* begin, const char* limit, xge_font* out)
{
	xge_glyph_run_backend_t* backend = run->pBackend; const char *word_end, *grapheme_end;
	int result, glyph; uint32_t cp; const char* scan = begin;
	result = __xgeOtEnsureScripts(desc, run); if (result != XGE_OK) return result;
	if (!desc->pFont->pFallback || !__xgeFontFace(desc->pFont)) {
		result = __xgeTextUTF8DecodeBounded(&scan, limit, &cp); if (result != XGE_OK) return result;
		*out = __xgeFontResolveCodepoint(desc->pFont, cp, &glyph); return XGE_OK;
	}
	if (backend->pFallbackUnitEnd && begin < backend->pFallbackUnitEnd) {
		*out = backend->pFallbackUnitFont; return XGE_OK;
	}
	result = __xgeOtEnsureGraphemes(desc, run); if (result != XGE_OK) return result;
	grapheme_end = __xgeOtGraphemeEnd(desc, run, begin, limit);
	if (!backend->pFallbackWordEnd || begin >= backend->pFallbackWordEnd) {
		if(desc->iContextOffset || desc->iContextSize!=run->iTextSize){
			const char *word_begin,*context_end;
			result=__xgeOtContextWord(desc,run,begin,&word_begin,&context_end);if(result!=XGE_OK)return result;
			ptrdiff_t local=context_end-desc->sContext-desc->iContextOffset;
			word_end=desc->sText+(local<run->iTextSize?local:run->iTextSize);
			result=__xgeOtPickContextFont(desc,run,begin,word_begin,context_end,word_end,out);
		}else{
			result=__xgeOtWordEnd(desc,run,begin,limit,&word_end);
			if(result==XGE_OK)result=__xgeOtPickFont(desc,run,begin,word_end,out);
		}
		if(result!=XGE_OK)return result;
		backend->pFallbackWordEnd = word_end;
		if (*out) {
			backend->pFallbackUnitFont = *out; backend->pFallbackUnitEnd = word_end; return XGE_OK;
		}
	}
	result = __xgeOtPickFont(desc, run, begin, grapheme_end, out); if (result != XGE_OK) return result;
	if (!*out) {
		result = __xgeTextUTF8DecodeBounded(&scan, grapheme_end, &cp); if (result != XGE_OK) return result;
		*out = __xgeFontResolveCodepoint(desc->pFont, cp, &glyph);
	}
	backend->pFallbackUnitFont = *out; backend->pFallbackUnitEnd = grapheme_end; return XGE_OK;
}
