static int __xgeOtReserve(void** array, int* capacity, int need, size_t unit)
{
	int next;
	void* data;
	if ( need < 0 || (size_t)need > SIZE_MAX / unit ) return XGE_ERROR_OUT_OF_MEMORY;
	if ( need <= *capacity && (!need || *array) ) return XGE_OK;
	next = *capacity > INT_MAX / 2 ? need : *capacity * 2;
	if ( next < need ) next = need;
	if ( (size_t)next > SIZE_MAX / unit ) return XGE_ERROR_OUT_OF_MEMORY;
	data = xrtRealloc(*array, (size_t)next * unit);
	if ( !data ) return XGE_ERROR_OUT_OF_MEMORY;
	*array = data; *capacity = next;
	return XGE_OK;
}

/* A ligature may share its cluster with attached marks. Use its GDEF only
 * when there is one carrier and every other glyph is a zero-advance GDEF mark;
 * multiple carriers do not expose their source-component mapping through HB.
 * Stops follow Unicode graphemes and include the carrier's physical pen and
 * shaped placement. Partial/malformed lists fall back as a complete unit. */
static int __xgeOtCarets(const xge_text_shape_desc_t* desc, xge_glyph_run_t* run,
	xge_font font, uint32_t start, uint32_t end, const hb_glyph_info_t* infos,
	const hb_glyph_position_t* shaped, unsigned glyph_count, float advance)
{
	xge_glyph_run_backend_t* backend = run->pBackend;
	hb_position_t* positions = NULL;
	unsigned available, wanted, at, count = 0;
	int exact = 0, result;
	int rtl = (desc->iFlags & XGE_TEXT_SHAPE_RTL) != 0;
	float previous = 0, pen = 0, offset = 0;
	unsigned carrier = glyph_count, j;
	int unambiguous = 1;
	void* carets;
	if ( end <= start + 1 ) return XGE_OK;
	if ( !backend->pGraphemeBreaks ) {
		backend->pGraphemeBreaks = xrtMalloc((size_t)run->iTextSize);
		if ( !backend->pGraphemeBreaks ) return XGE_ERROR_OUT_OF_MEMORY;
		__xgeOtGraphemesUtf8((const utf8_t*)desc->sText, (size_t)run->iTextSize, NULL, backend->pGraphemeBreaks);
	}
	for ( at = start + 1; at < end; at++ )
		if ( backend->pGraphemeBreaks[at - 1] == GRAPHEMEBREAK_BREAK ) count++;
	if ( !count ) return XGE_OK;
	for ( j = 0; j < glyph_count; j++ ) {
		if ( glyph_count > 1 && hb_ot_layout_get_glyph_class(hb_font_get_face(__xgeFontShapeFont(font)),
			infos[j].codepoint) == HB_OT_LAYOUT_GLYPH_CLASS_MARK ) {
			if ( shaped[j].x_advance != 0 ) unambiguous = 0;
		} else if ( carrier != glyph_count ) unambiguous = 0;
		else { carrier = j; offset = pen + (float)shaped[j].x_offset * font->fScale; }
		pen += (float)shaped[j].x_advance * font->fScale;
	}
	wanted = 0;
	available = unambiguous && carrier < glyph_count ? hb_ot_layout_get_ligature_carets(__xgeFontShapeFont(font),
		rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR, infos[carrier].codepoint, 0, &wanted, NULL) : 0;
	if ( available == count && __xgeOtCaretValuesValid(__xgeFontShapeFont(font), infos[carrier].codepoint, count) ) {
		positions = xrtMalloc((size_t)count * sizeof(*positions));
		if ( !positions ) return XGE_ERROR_OUT_OF_MEMORY;
		wanted = count;
		hb_ot_layout_get_ligature_carets(__xgeFontShapeFont(font), rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR,
			infos[carrier].codepoint, 0, &wanted, positions);
		exact = wanted == count;
		for ( at = 0; at < wanted && exact; at++ ) {
			float stop = offset + (float)positions[rtl ? wanted - 1 - at : at] * font->fScale;
			if ( rtl ) stop = advance - stop;
			if ( !isfinite(stop) || stop < previous || stop < 0 || stop > advance ) exact = 0;
			previous = stop;
		}
	}
	if ( run->iCaretCount < 0 || count > (unsigned)(INT_MAX - run->iCaretCount) ) result = XGE_ERROR_OUT_OF_MEMORY;
	else {
		/* A typed pointer object must not be accessed through void**. */
		carets = run->pCarets;
		result = __xgeOtReserve(&carets, &backend->iCaretCapacity,
			run->iCaretCount + (int)count, sizeof(*run->pCarets));
		if ( result == XGE_OK && !carets ) result = XGE_ERROR_OUT_OF_MEMORY;
		if ( result == XGE_OK ) run->pCarets = carets;
	}
	if ( result == XGE_OK ) {
		unsigned index = 0;
		for ( at = start + 1; at < end; at++ ) {
			if ( backend->pGraphemeBreaks[at - 1] == GRAPHEMEBREAK_BREAK ) {
				xge_glyph_caret_t* caret = &run->pCarets[run->iCaretCount++];
				caret->iTextOffset = at;
				caret->fAdvance = exact ? offset + (float)positions[rtl ? count - 1 - index : index] * font->fScale :
					advance * (float)(index + 1) / (float)(count + 1);
				if ( rtl && exact ) caret->fAdvance = advance - caret->fAdvance;
				index++;
			}
		}
	}
	xrtFree(positions);
	return result;
}
