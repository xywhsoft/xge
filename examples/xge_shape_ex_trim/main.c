#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 512
#define DEMO_H 512

typedef struct trim_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int bCaptureDone;
} trim_demo_t;

typedef int (*trim_path_builder)(xge_shape_ex pShape);

static int path_cubic(xge_shape_ex pShape)
{
	int ret = xgeShapeExMoveTo(pShape, 28.0f, 64.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 72.0f, 8.0f, 172.0f, 120.0f, 236.0f, 64.0f);
	return ret;
}

static int path_quadratic(xge_shape_ex pShape)
{
	int ret = xgeShapeExMoveTo(pShape, 276.0f, 64.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExQuadTo(pShape, 376.0f, 6.0f, 484.0f, 64.0f);
	return ret;
}

static int path_closed_wrap(xge_shape_ex pShape)
{
	int ret = xgeShapeExMoveTo(pShape, 38.0f, 174.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 76.0f, 116.0f, 194.0f, 116.0f, 232.0f, 174.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 194.0f, 232.0f, 76.0f, 232.0f, 38.0f, 174.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExClose(pShape);
	return ret;
}

static int path_dashed(xge_shape_ex pShape)
{
	int ret = xgeShapeExMoveTo(pShape, 276.0f, 174.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 318.0f, 112.0f, 436.0f, 236.0f, 484.0f, 174.0f);
	return ret;
}

static int path_multi(xge_shape_ex pShape)
{
	int ret = xgeShapeExMoveTo(pShape, 28.0f, 270.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 68.0f, 234.0f, 132.0f, 306.0f, 192.0f, 270.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExMoveTo(pShape, 42.0f, 318.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 94.0f, 282.0f, 184.0f, 354.0f, 236.0f, 318.0f);
	return ret;
}

static int path_multi_right(xge_shape_ex pShape)
{
	int ret = xgeShapeExMoveTo(pShape, 276.0f, 270.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 316.0f, 234.0f, 380.0f, 306.0f, 440.0f, 270.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExMoveTo(pShape, 290.0f, 318.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 342.0f, 282.0f, 432.0f, 354.0f, 484.0f, 318.0f);
	return ret;
}

static int path_fill_trim(xge_shape_ex pShape)
{
	return xgeShapeExAppendRect(pShape, 34.0f, 382.0f, 204.0f, 92.0f, 18.0f, 18.0f, 1);
}

static int path_wrapped_open(xge_shape_ex pShape)
{
	int ret = xgeShapeExMoveTo(pShape, 280.0f, 430.0f);
	if ( ret == XGE_OK ) ret = xgeShapeExCubicTo(pShape, 324.0f, 366.0f, 438.0f, 494.0f, 484.0f, 430.0f);
	return ret;
}

static int draw_path(
	trim_path_builder pBuilder,
	uint32_t iFill,
	uint32_t iStroke,
	float fWidth,
	int bTrim,
	float fBegin,
	float fEnd,
	int bSimultaneous,
	const float* pDash,
	int iDashCount)
{
	xge_shape_ex shape = NULL;
	int ret = xgeShapeExCreate(&shape);

	if ( ret == XGE_OK ) ret = pBuilder(shape);
	if ( ret == XGE_OK ) ret = xgeShapeExFillColor(shape, iFill);
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeColor(shape, iStroke);
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeWidth(shape, fWidth);
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeCap(shape, XGE_SHAPE_EX_CAP_ROUND);
	if ( ret == XGE_OK ) ret = xgeShapeExStrokeJoin(shape, XGE_SHAPE_EX_JOIN_ROUND);
	if ( (ret == XGE_OK) && (pDash != NULL) && (iDashCount > 0) ) {
		ret = xgeShapeExStrokeDash(shape, pDash, iDashCount, 3.0f);
	}
	if ( (ret == XGE_OK) && bTrim ) {
		ret = xgeShapeExTrimPath(shape, fBegin, fEnd, bSimultaneous);
	}
	if ( ret == XGE_OK ) ret = xgeShapeExDrawPx(shape, 0.25f);
	xgeShapeExDestroy(shape);
	return ret;
}

static int draw_scene(void)
{
	static const uint32_t transparent = XGE_COLOR_RGBA(0, 0, 0, 0);
	static const uint32_t guide = XGE_COLOR_RGBA(62, 73, 84, 255);
	static const float dash[2] = {16.0f, 10.0f};
	int ret;

	ret = draw_path(path_cubic, transparent, guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_cubic, transparent, XGE_COLOR_RGBA(255, 126, 72, 255), 9.0f, 1, 0.18f, 0.72f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_quadratic, transparent, guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_quadratic, transparent, XGE_COLOR_RGBA(96, 210, 242, 255), 9.0f, 1, 0.25f, 0.78f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_closed_wrap, transparent, guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_closed_wrap, transparent, XGE_COLOR_RGBA(169, 140, 255, 255), 9.0f, 1, 0.76f, 0.20f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_dashed, transparent, guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_dashed, transparent, XGE_COLOR_RGBA(95, 208, 151, 255), 8.0f, 1, 0.14f, 0.86f, 0, dash, 2);
	if ( ret == XGE_OK ) ret = draw_path(path_multi, transparent, guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_multi, transparent, XGE_COLOR_RGBA(245, 188, 88, 255), 8.0f, 1, 0.18f, 0.70f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_multi_right, transparent, guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_multi_right, transparent, XGE_COLOR_RGBA(238, 91, 128, 255), 8.0f, 1, 0.18f, 0.70f, 1, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_fill_trim, XGE_COLOR_RGBA(45, 67, 82, 255), guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_fill_trim, XGE_COLOR_RGBA(45, 133, 168, 180), XGE_COLOR_RGBA(255, 205, 96, 255), 8.0f, 1, 0.08f, 0.64f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_wrapped_open, transparent, guide, 3.0f, 0, 0.0f, 1.0f, 0, NULL, 0);
	if ( ret == XGE_OK ) ret = draw_path(path_wrapped_open, transparent, XGE_COLOR_RGBA(133, 220, 192, 255), 9.0f, 1, 1.16f, 1.72f, 0, NULL, 0);
	return ret;
}

static int capture(trim_demo_t* pDemo)
{
	unsigned char* pPixels;
	int iStride = DEMO_W * 4;
	int ret;

	if ( pDemo->bCaptureDone ) return XGE_OK;
	pPixels = (unsigned char*)malloc((size_t)iStride * DEMO_H);
	if ( pPixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	ret = xgeRenderTargetReadPixels(&pDemo->tTarget, pPixels, iStride);
	if ( ret == XGE_OK ) ret = xgeImageSavePNG(pDemo->sCapturePath, DEMO_W, DEMO_H, pPixels, iStride);
	free(pPixels);
	if ( ret == XGE_OK ) {
		pDemo->bCaptureDone = 1;
		printf("xge_shape_ex_trim capture saved: %s\n", pDemo->sCapturePath);
	}
	return ret;
}

static int frame(void* pUser)
{
	trim_demo_t* pDemo = (trim_demo_t*)pUser;
	xge_pass_t pass;
	int ret = xgeBegin();

	if ( ret != XGE_OK ) return ret;
	xgePassInit(&pass, &pDemo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(15, 20, 26, 255));
	ret = xgePassBegin(&pass);
	if ( ret == XGE_OK ) ret = draw_scene();
	if ( ret == XGE_OK ) ret = xgePassEnd(&pass);
	if ( ret == XGE_OK ) ret = capture(pDemo);
	if ( ret == XGE_OK ) ret = xgeEnd();
	if ( pDemo->bCaptureDone ) xgeQuit();
	return ret;
}

int main(int argc, char** argv)
{
	trim_demo_t demo;
	xge_desc_t desc;
	int ret;

	memset(&demo, 0, sizeof(demo));
	snprintf(demo.sCapturePath, sizeof(demo.sCapturePath), "%s", (argc > 1) ? argv[1] : "artifacts/xge_shape_ex_trim.png");
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = DEMO_W;
	desc.iHeight = DEMO_H;
	desc.sTitle = "xge_shape_ex_trim";
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) return 1;
	ret = xgeRenderTargetCreate(&demo.tTarget, DEMO_W, DEMO_H);
	if ( ret == XGE_OK ) ret = xgeRun(frame, &demo);
	xgeRenderTargetFree(&demo.tTarget);
	xgeUnit();
	return (ret == XGE_OK) ? 0 : 1;
}
