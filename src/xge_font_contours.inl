/* Bounded, unhinted TrueType points. Preserve the font's original point
 * numbering, including off-curve points; rasterizer vertices insert implied
 * points and therefore cannot serve GDEF/GPOS contour-point indices. */
typedef struct xge_tt_source_t {
	const unsigned char *glyf, *loca;
	uint32_t glyf_bytes, loca_bytes, glyphs;
	unsigned loca_format;
} xge_tt_source_t;
typedef struct xge_tt_point_t { double x, y; unsigned char on, end; } xge_tt_point_t;
typedef int (*xge_tt_emit_proc)(const xge_tt_point_t*, void*);
typedef struct xge_tt_walk_t {
	const xge_tt_source_t* source;
	uint32_t glyph_path[64], prefix_path[64];
	unsigned depth, work;
} xge_tt_walk_t;
static unsigned __xgeTTU16(const unsigned char* p) { return ((unsigned)p[0] << 8) | p[1]; }
static int __xgeTTS16(const unsigned char* p)
{ unsigned v = __xgeTTU16(p); return v < 32768 ? (int)v : (int)v - 65536; }
static uint32_t __xgeTTU32(const unsigned char* p)
{ return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]; }
static int __xgeTTRange(size_t at, size_t bytes, size_t length)
{ return at <= length && bytes <= length - at; }
static int __xgeTTSourceInit(xge_tt_source_t* out,
	const unsigned char* head, uint32_t head_bytes,
	const unsigned char* maxp, uint32_t maxp_bytes,
	const unsigned char* glyf, uint32_t glyf_bytes,
	const unsigned char* loca, uint32_t loca_bytes)
{
	xge_tt_source_t source = {0};
	if (!head || head_bytes < 54 || !maxp || maxp_bytes < 6 || !glyf || !loca) return 0;
	source.loca_format = __xgeTTU16(head + 50); source.glyphs = __xgeTTU16(maxp + 4);
	if (source.loca_format > 1 || !source.glyphs ||
		(uint64_t)(source.glyphs + 1) * (source.loca_format ? 4 : 2) > loca_bytes) return 0;
	source.glyf = glyf; source.glyf_bytes = glyf_bytes; source.loca = loca; source.loca_bytes = loca_bytes;
	*out = source; return 1;
}
static int __xgeTTRecord(const xge_tt_source_t* source, uint32_t glyph,
	const unsigned char** data, size_t* bytes)
{
	uint32_t start, end;
	if (!source->glyf || !source->loca || glyph >= source->glyphs) return 0;
	if (source->loca_format == 0) {
		if (!__xgeTTRange((size_t)glyph * 2, 4, source->loca_bytes)) return 0;
		start = __xgeTTU16(source->loca + glyph * 2) * 2;
		end = __xgeTTU16(source->loca + glyph * 2 + 2) * 2;
	} else if (source->loca_format == 1) {
		if (!__xgeTTRange((size_t)glyph * 4, 8, source->loca_bytes)) return 0;
		start = __xgeTTU32(source->loca + (size_t)glyph * 4);
		end = __xgeTTU32(source->loca + (size_t)glyph * 4 + 4);
	} else return 0;
	if (end < start || end > source->glyf_bytes) return 0;
	*data = source->glyf + start; *bytes = end - start; return 1;
}
typedef struct xge_tt_flags_t {
	const unsigned char* data; size_t bytes, at; unsigned repeat, flag;
} xge_tt_flags_t;
static int __xgeTTFlag(xge_tt_flags_t* iterator, unsigned* flag)
{
	if (iterator->repeat) iterator->repeat--;
	else {
		if (iterator->at >= iterator->bytes) return 0;
		iterator->flag = iterator->data[iterator->at++];
		if (iterator->flag & 8) {
			if (iterator->at >= iterator->bytes) return 0;
			iterator->repeat = iterator->data[iterator->at++];
		}
	}
	*flag = iterator->flag; return 1;
}
static int __xgeTTSimple(xge_tt_walk_t* walk, const unsigned char* data,
	size_t bytes, unsigned contours, xge_tt_emit_proc emit, void* user, uint32_t* count)
{
	xge_tt_flags_t iterator = {0};
	size_t flags_at, x_at, y_at, x_bytes = 0, y_bytes = 0, at;
	unsigned points, i, contour = 0, previous = 0, flag, instructions;
	int64_t x = 0, y = 0;
	if (!contours) { *count = 0; return 1; }
	if (!__xgeTTRange(10, (size_t)contours * 2 + 2, bytes)) return 0;
	for (i = 0; i < contours; i++) {
		unsigned end = __xgeTTU16(data + 10 + i * 2);
		if (i && end <= previous) return 0;
		previous = end;
	}
	points = previous + 1;
	if (walk->work < points * 2u) return 0;
	walk->work -= points * 2u;
	at = 10 + (size_t)contours * 2; instructions = __xgeTTU16(data + at); at += 2;
	if (!__xgeTTRange(at, instructions, bytes)) return 0;
	flags_at = at + instructions;
	iterator.data = data; iterator.bytes = bytes; iterator.at = flags_at;
	for (i = 0; i < points; i++) {
		if (!__xgeTTFlag(&iterator, &flag) || iterator.repeat >= points - i) return 0;
		x_bytes += (flag & 2) ? 1 : (flag & 16) ? 0 : 2;
		y_bytes += (flag & 4) ? 1 : (flag & 32) ? 0 : 2;
	}
	x_at = iterator.at;
	if (!__xgeTTRange(x_at, x_bytes, bytes)) return 0;
	y_at = x_at + x_bytes;
	if (!__xgeTTRange(y_at, y_bytes, bytes)) return 0;
	iterator.at = flags_at; iterator.repeat = 0;
	for (i = 0; i < points; i++) {
		xge_tt_point_t point; int result;
		if (!__xgeTTFlag(&iterator, &flag)) return 0;
		if (flag & 2) { unsigned delta = data[x_at++]; x += (flag & 16) ? (int)delta : -(int)delta; }
		else if (!(flag & 16)) { x += __xgeTTS16(data + x_at); x_at += 2; }
		if (flag & 4) { unsigned delta = data[y_at++]; y += (flag & 32) ? (int)delta : -(int)delta; }
		else if (!(flag & 32)) { y += __xgeTTS16(data + y_at); y_at += 2; }
		if (x < INT16_MIN || x > INT16_MAX || y < INT16_MIN || y > INT16_MAX) return 0;
		point.x = (double)x; point.y = (double)y; point.on = !!(flag & 1);
		point.end = i == __xgeTTU16(data + 10 + contour * 2);
		if (point.end) contour++;
		result = emit ? emit(&point, user) : 1;
		if (result != 1) return result;
	}
	*count = points; return 1;
}
typedef struct xge_tt_find_t { uint32_t wanted, at; int found; xge_tt_point_t point; } xge_tt_find_t;
static int __xgeTTFind(const xge_tt_point_t* point, void* user)
{
	xge_tt_find_t* find = user;
	if (find->at == find->wanted) { find->point = *point; find->found = 1; }
	find->at++; return 1;
}
static int __xgeTTWalk(xge_tt_walk_t*, uint32_t, uint32_t, xge_tt_emit_proc, void*, uint32_t*);
static int __xgeTTFindInWalk(xge_tt_walk_t* walk, uint32_t glyph, uint32_t index,
	uint32_t prefix, xge_tt_point_t* out)
{
	xge_tt_find_t find = {0}; uint32_t count;
	find.wanted = index;
	if (__xgeTTWalk(walk, glyph, prefix, __xgeTTFind, &find, &count) != 1 || !find.found) return 0;
	*out = find.point; return 1;
}
typedef struct xge_tt_transform_t {
	double xx, xy, yx, yy, dx, dy; xge_tt_emit_proc emit; void* user;
} xge_tt_transform_t;
static int __xgeTTTransform(const xge_tt_point_t* point, void* user)
{
	xge_tt_transform_t* transform = user; xge_tt_point_t mapped = *point;
	mapped.x = transform->xx * point->x + transform->xy * point->y + transform->dx;
	mapped.y = transform->yx * point->x + transform->yy * point->y + transform->dy;
	if (!isfinite(mapped.x) || !isfinite(mapped.y)) return 0;
	return transform->emit ? transform->emit(&mapped, transform->user) : 1;
}
static int __xgeTTCompound(xge_tt_walk_t* walk, uint32_t glyph, const unsigned char* data,
	size_t bytes, uint32_t prefix, xge_tt_emit_proc emit, void* user, uint32_t* count)
{
	size_t at = 10; uint32_t points = 0, component = 0; unsigned flags = 0, has_instructions = 0;
	do {
		xge_tt_transform_t transform = {1,0,0,1,0,0,emit,user};
		unsigned child, words, xy, transform_bits; int arg1, arg2, result; uint32_t child_points;
		if (component == prefix) { *count = points; return 1; }
		if (!walk->work || !__xgeTTRange(at, 4, bytes)) return 0;
		walk->work--;
		flags = __xgeTTU16(data + at); child = __xgeTTU16(data + at + 2); at += 4;
		if (flags & 0xe010u || (flags & 0x1800u) == 0x1800u) return 0;
		has_instructions |= flags & 0x100u; words = flags & 1; xy = flags & 2;
		if (!__xgeTTRange(at, words ? 4 : 2, bytes)) return 0;
		if (words) {
			arg1 = xy ? __xgeTTS16(data + at) : (int)__xgeTTU16(data + at);
			arg2 = xy ? __xgeTTS16(data + at + 2) : (int)__xgeTTU16(data + at + 2); at += 4;
		} else {
			arg1 = xy && data[at] >= 128 ? (int)data[at] - 256 : data[at];
			arg2 = xy && data[at + 1] >= 128 ? (int)data[at + 1] - 256 : data[at + 1]; at += 2;
		}
		transform_bits = flags & 0xc8u;
		if (transform_bits == 8) {
			if (!__xgeTTRange(at, 2, bytes)) return 0;
			transform.xx = transform.yy = __xgeTTS16(data + at) / 16384.0; at += 2;
		} else if (transform_bits == 0x40) {
			if (!__xgeTTRange(at, 4, bytes)) return 0;
			transform.xx = __xgeTTS16(data + at) / 16384.0;
			transform.yy = __xgeTTS16(data + at + 2) / 16384.0; at += 4;
		} else if (transform_bits == 0x80) {
			if (!__xgeTTRange(at, 8, bytes)) return 0;
			transform.xx = __xgeTTS16(data + at) / 16384.0;
			transform.yx = __xgeTTS16(data + at + 2) / 16384.0;
			transform.xy = __xgeTTS16(data + at + 4) / 16384.0;
			transform.yy = __xgeTTS16(data + at + 6) / 16384.0; at += 8;
		} else if (transform_bits) return 0;
		if (xy) {
			transform.dx = arg1; transform.dy = arg2;
			if (flags & 0x800u) {
				transform.dx = transform.xx * arg1 + transform.xy * arg2;
				transform.dy = transform.yx * arg1 + transform.yy * arg2;
			}
		} else {
			xge_tt_point_t parent_point, child_point;
			if ((uint32_t)arg1 >= points ||
				!__xgeTTFindInWalk(walk, glyph, (uint32_t)arg1, component, &parent_point) ||
				!__xgeTTFindInWalk(walk, child, (uint32_t)arg2, UINT32_MAX, &child_point)) return 0;
			transform.dx = parent_point.x - transform.xx * child_point.x - transform.xy * child_point.y;
			transform.dy = parent_point.y - transform.yx * child_point.x - transform.yy * child_point.y;
		}
		/* The renderer is unhinted: ROUND_XY_TO_GRID and instructions do not
         * grid-fit these design coordinates. Unspecified offsets are unscaled. */
		result = __xgeTTWalk(walk, child, UINT32_MAX, __xgeTTTransform, &transform, &child_points);
		if (result != 1) return result;
		if (child_points > 65536u - points) return 0;
		points += child_points; component++;
	} while (flags & 0x20u);
	if (has_instructions) {
		unsigned instructions;
		if (!__xgeTTRange(at, 2, bytes)) return 0;
		instructions = __xgeTTU16(data + at); at += 2;
		if (!__xgeTTRange(at, instructions, bytes)) return 0;
	}
	*count = points; return 1;
}
/* 1 = complete, 0 = malformed/unsupported/budget, -1 = emitter OOM.
 * Prefix walks resolve point-to-point component attachment without storing
 * a second outline. Recursive self-prefixes must strictly decrease; true
 * component cycles, excessive depth/work and bad offsets are rejected. */
static int __xgeTTWalk(xge_tt_walk_t* walk, uint32_t glyph, uint32_t prefix,
	xge_tt_emit_proc emit, void* user, uint32_t* count)
{
	const unsigned char* data; size_t bytes; unsigned i; int contours, result;
	*count = 0;
	if (walk->depth >= 64 || !walk->work || !__xgeTTRecord(walk->source, glyph, &data, &bytes)) return 0;
	if (!bytes) return 1;
	if (bytes < 10) return 0;
	for (i = 0; i < walk->depth; i++)
		if (walk->glyph_path[i] == glyph && prefix >= walk->prefix_path[i]) return 0;
	walk->glyph_path[walk->depth] = glyph; walk->prefix_path[walk->depth++] = prefix;
	contours = __xgeTTS16(data);
	if (contours >= 0) result = __xgeTTSimple(walk, data, bytes, (unsigned)contours, emit, user, count);
	else if (contours == -1) result = __xgeTTCompound(walk, glyph, data, bytes, prefix, emit, user, count);
	else result = 0;
	walk->depth--; return result;
}
static int __xgeTTPoint(const xge_tt_source_t* source, uint32_t glyph, uint32_t index,
	xge_tt_point_t* out)
{
	xge_tt_walk_t walk = {0}; walk.source = source; walk.work = 1048576u;
	return __xgeTTFindInWalk(&walk, glyph, index, UINT32_MAX, out);
}
