/* Convert the same unhinted points used by GDEF into stb rasterizer vertices.
 * This also fixes compound scale/offset and point-attachment handling without
 * modifying the vendored rasterizer. CFF keeps its existing stb outline path. */
typedef struct xge_tt_collect_t {
	xge_tt_point_t* points; size_t count, capacity;
} xge_tt_collect_t;
static int __xgeTTCollect(const xge_tt_point_t* point, void* user)
{
	xge_tt_collect_t* collect = user;
	if (collect->count >= 65536u) return 0;
	if (collect->count == collect->capacity) {
		size_t capacity = collect->capacity ? collect->capacity * 2 : 64;
		xge_tt_point_t* points = xrtRealloc(collect->points, capacity * sizeof(*points));
		if (!points) return -1;
		collect->points = points; collect->capacity = capacity;
	}
	collect->points[collect->count] = *point;
	/* stb vertices store design coordinates as int16. Round transformed
	 * original points once; implied quadratic midpoints keep stb's floor. */
	collect->points[collect->count].x = floor(point->x + .5);
	collect->points[collect->count++].y = floor(point->y + .5); return 1;
}
static int __xgeTTVertex(stbtt_vertex* vertex, int type, double x, double y, double cx, double cy)
{
	x = floor(x + .5); y = floor(y + .5); cx = floor(cx + .5); cy = floor(cy + .5);
	if (!isfinite(x) || !isfinite(y) || !isfinite(cx) || !isfinite(cy) ||
		x < INT16_MIN || x > INT16_MAX || y < INT16_MIN || y > INT16_MAX ||
		cx < INT16_MIN || cx > INT16_MAX || cy < INT16_MIN || cy > INT16_MAX) return 0;
	memset(vertex, 0, sizeof(*vertex)); vertex->type = (unsigned char)type;
	vertex->x = (stbtt_vertex_type)x; vertex->y = (stbtt_vertex_type)y;
	vertex->cx = (stbtt_vertex_type)cx; vertex->cy = (stbtt_vertex_type)cy; return 1;
}
static int __xgeTTOutline(const xge_tt_source_t* source, uint32_t glyph,
	stbtt_vertex** vertices, int* vertex_count)
{
	xge_tt_walk_t walk = {0}; xge_tt_collect_t collect = {0}; stbtt_vertex* result_vertices = NULL;
	uint32_t count; size_t first, out = 0; int result;
	*vertices = NULL; *vertex_count = 0; walk.source = source; walk.work = 1048576u;
	result = __xgeTTWalk(&walk, glyph, UINT32_MAX, __xgeTTCollect, &collect, &count);
	if (result != 1) goto done;
	if (!count) goto done;
	result_vertices = xrtMalloc(((size_t)count * 3 + 1) * sizeof(*result_vertices));
	if (!result_vertices) { result = -1; goto done; }
	for (first = 0; first < collect.count; ) {
		size_t last = first, at, stop; xge_tt_point_t start, control = {0}; int pending = 0;
		while (last < collect.count && !collect.points[last].end) last++;
		if (last == collect.count) { result = 0; goto done; }
		if (collect.points[first].on) { start = collect.points[first]; at = first + 1; stop = last + 1; }
		else if (collect.points[last].on) { start = collect.points[last]; at = first; stop = last; }
		else {
			start = collect.points[first]; start.x = floor((start.x + collect.points[last].x) * .5);
			start.y = floor((start.y + collect.points[last].y) * .5); at = first; stop = last + 1;
		}
		if (!__xgeTTVertex(&result_vertices[out++], STBTT_vmove, start.x, start.y, 0, 0)) { result = 0; goto done; }
		for (; at < stop; at++) {
			const xge_tt_point_t* point = &collect.points[at];
			if (point->on) {
				if (!__xgeTTVertex(&result_vertices[out++], pending ? STBTT_vcurve : STBTT_vline,
					point->x, point->y, control.x, control.y)) { result = 0; goto done; }
				pending = 0;
			} else {
				if (pending && !__xgeTTVertex(&result_vertices[out++], STBTT_vcurve,
					floor((control.x + point->x) * .5), floor((control.y + point->y) * .5), control.x, control.y)) { result = 0; goto done; }
				control = *point; pending = 1;
			}
		}
		if (!__xgeTTVertex(&result_vertices[out++], pending ? STBTT_vcurve : STBTT_vline,
			start.x, start.y, control.x, control.y)) { result = 0; goto done; }
		first = last + 1;
	}
	*vertices = result_vertices; *vertex_count = (int)out; result_vertices = NULL;
done:
	xrtFree(result_vertices); xrtFree(collect.points); return result;
}
