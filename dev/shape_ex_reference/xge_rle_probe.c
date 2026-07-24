#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef xrtMalloc
#undef xrtRealloc
#undef xrtFree
#define xrtMalloc malloc
#define xrtRealloc realloc
#define xrtFree free

#include "../../src/xge_shape_ex_coverage.c"

#define PROBE_WIDTH 192
#define PROBE_HEIGHT 160
#define PROBE_KAPPA 0.552284f

static int write_pgm(const char* path, const unsigned char* pixels)
{
	FILE* file = fopen(path, "wb");
	size_t size = (size_t)PROBE_WIDTH * PROBE_HEIGHT;
	size_t written;

	if ( file == NULL ) return 0;
	fprintf(file, "P5\n%d %d\n255\n", PROBE_WIDTH, PROBE_HEIGHT);
	written = fwrite(pixels, 1, size, file);
	fclose(file);
	return written == size;
}

int main(int argc, char** argv)
{
	const char* output = argc > 1 ? argv[1] : "xge_rle_probe.pgm";
	const float x = 34.0f;
	const float y = 36.0f;
	const float w = 124.0f;
	const float h = 88.0f;
	const float rx = 18.0f;
	const float ry = 18.0f;
	const float kx = rx * PROBE_KAPPA;
	const float ky = ry * PROBE_KAPPA;
	const uint8_t commands[10] = {
		XGE_SHAPE_EX_CMD_MOVE_TO,
		XGE_SHAPE_EX_CMD_LINE_TO, XGE_SHAPE_EX_CMD_CUBIC_TO,
		XGE_SHAPE_EX_CMD_LINE_TO, XGE_SHAPE_EX_CMD_CUBIC_TO,
		XGE_SHAPE_EX_CMD_LINE_TO, XGE_SHAPE_EX_CMD_CUBIC_TO,
		XGE_SHAPE_EX_CMD_LINE_TO, XGE_SHAPE_EX_CMD_CUBIC_TO,
		XGE_SHAPE_EX_CMD_CLOSE
	};
	const xge_vec2_t points[17] = {
		{x + w, y + ry},
		{x + w, y + h - ry},
		{x + w, y + h - ry + ky}, {x + w - rx + kx, y + h}, {x + w - rx, y + h},
		{x + rx, y + h},
		{x + rx - kx, y + h}, {x, y + h - ry + ky}, {x, y + h - ry},
		{x, y + ry},
		{x, y + ry - ky}, {x + rx - kx, y}, {x + rx, y},
		{x + w - rx, y},
		{x + w - rx + kx, y}, {x + w, y + ry - ky}, {x + w, y + ry}
	};
	xge_shape_ex_coverage_raster_t raster;
	xge_shape_ex_matrix_t matrix = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
	unsigned char pixels[PROBE_WIDTH * PROBE_HEIGHT];
	int ret;
	int i;

	memset(&raster, 0, sizeof(raster));
	memset(pixels, 0, sizeof(pixels));
	ret = __xgeShapeExCoverageRasterizePath(
		&raster, commands, 10, points, 17, matrix,
		0, 0, PROBE_WIDTH, PROBE_HEIGHT, 0
	);
	if ( ret != XGE_OK ) return 2;
	for ( i = 0; i < raster.iSpanCount; ++i ) {
		const xge_shape_ex_coverage_span_t* span = &raster.pSpans[i];
		int j;

		for ( j = 0; j < span->iLength; ++j ) {
			int px = span->iX + j;
			if ( px >= 0 && px < PROBE_WIDTH && span->iY >= 0 && span->iY < PROBE_HEIGHT ) {
				pixels[span->iY * PROBE_WIDTH + px] = 255;
			}
		}
	}
	__xgeShapeExCoverageRasterFree(&raster);
	return write_pgm(output, pixels) ? 0 : 3;
}
