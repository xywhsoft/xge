/* HB's Format 2 query does not preserve a failed contour callback's status:
 * it returns zero. Validate the entire relevant raw list first, so a missing
 * point or unknown format never masquerades as a valid zero caret. */
static unsigned __xgeGdefU16(const unsigned char* p) { return ((unsigned)p[0] << 8) | p[1]; }
static int __xgeGdefSpan(size_t at, size_t bytes, size_t length)
{ return at <= length && bytes <= length - at; }
static int __xgeOtCaretValuesValid(hb_font_t* font, hb_codepoint_t glyph, unsigned count)
{
	hb_blob_t* blob = hb_face_reference_table(hb_font_get_face(font), HB_TAG('G','D','E','F'));
	unsigned length, index = UINT_MAX, i; const unsigned char* data = (const unsigned char*)hb_blob_get_data(blob, &length);
	size_t list, coverage, ligature; int valid = 0;
	if (!data || length < 12 || __xgeGdefU16(data) != 1) goto done;
	list = __xgeGdefU16(data + 8);
	if (!list || !__xgeGdefSpan(list, 4, length)) goto done;
	coverage = __xgeGdefU16(data + list);
	if (!coverage || !__xgeGdefSpan(list + coverage, 4, length)) goto done;
	coverage += list;
	if (__xgeGdefU16(data + coverage) == 1) {
		unsigned n = __xgeGdefU16(data + coverage + 2), low = 0, high = n;
		if (!__xgeGdefSpan(coverage + 4, (size_t)n * 2, length)) goto done;
		while (low < high) { unsigned mid = low + (high - low) / 2;
			if (__xgeGdefU16(data + coverage + 4 + mid * 2) < glyph) low = mid + 1; else high = mid; }
		if (low < n && __xgeGdefU16(data + coverage + 4 + low * 2) == glyph) index = low;
	} else if (__xgeGdefU16(data + coverage) == 2) {
		unsigned n = __xgeGdefU16(data + coverage + 2), low = 0, high = n;
		if (!__xgeGdefSpan(coverage + 4, (size_t)n * 6, length)) goto done;
		while (low < high) { unsigned mid = low + (high - low) / 2;
			if (__xgeGdefU16(data + coverage + 4 + mid * 6 + 2) < glyph) low = mid + 1; else high = mid; }
		if (low < n) { const unsigned char* range = data + coverage + 4 + low * 6;
			unsigned first = __xgeGdefU16(range), last = __xgeGdefU16(range + 2);
			if (first <= glyph && glyph <= last) index = __xgeGdefU16(range + 4) + glyph - first; }
	} else goto done;
	if (index >= __xgeGdefU16(data + list + 2) ||
		!__xgeGdefSpan(list + 4, (size_t)(index + 1) * 2, length)) goto done;
	ligature = __xgeGdefU16(data + list + 4 + index * 2);
	if (!ligature || !__xgeGdefSpan(list + ligature, 2, length)) goto done;
	ligature += list;
	if (__xgeGdefU16(data + ligature) != count || !__xgeGdefSpan(ligature + 2, (size_t)count * 2, length)) goto done;
	for (i = 0; i < count; i++) {
		size_t caret = __xgeGdefU16(data + ligature + 2 + i * 2); unsigned format;
		if (!caret || !__xgeGdefSpan(ligature + caret, 4, length)) goto done;
		caret += ligature; format = __xgeGdefU16(data + caret);
		if (format == 2) {
			hb_position_t x, y;
			if (!hb_font_get_glyph_contour_point(font, glyph, __xgeGdefU16(data + caret + 2), &x, &y)) goto done;
		} else if (format == 3) {
			size_t device; unsigned delta_format, first, last;
			if (!__xgeGdefSpan(caret, 6, length)) goto done;
			device = __xgeGdefU16(data + caret + 4);
			if (!device) continue;
			if (!__xgeGdefSpan(caret + device, 6, length)) goto done;
			device += caret; delta_format = __xgeGdefU16(data + device + 4);
			if (delta_format == 0x8000u) continue;
			first = __xgeGdefU16(data + device); last = __xgeGdefU16(data + device + 2);
			if (first > last || delta_format < 1 || delta_format > 3 ||
				!__xgeGdefSpan(device + 6, (((size_t)(last - first + 1) * (1u << delta_format) + 15) / 16) * 2, length)) goto done;
		} else if (format != 1) goto done;
	}
	valid = 1;
done:
	hb_blob_destroy(blob); return valid;
}
