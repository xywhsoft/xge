#include "xui.h"

#include <math.h>
#include <stdio.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_scroll_model_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiScrollModelNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.01f;
}

int main(void)
{
	xui_scroll_model_t tModel;
	xui_rect_t tViewport;
	xui_vec2_t tPoint;
	float fX;
	float fY;
	float fOldX;
	float fOldY;
	int iFailed;
	int iRet;

	iFailed = 0;
	xuiScrollModelInit(&tModel);
	XUI_TEST_CHECK(tModel.iSize == sizeof(tModel), "init size");

	iRet = xuiScrollModelSetViewport(&tModel, (xui_rect_t){10.0f, 20.0f, 100.0f, 80.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set viewport");
	tViewport = xuiScrollModelGetViewport(&tModel);
	XUI_TEST_CHECK(__xuiScrollModelNear(tViewport.fX, 10.0f) && __xuiScrollModelNear(tViewport.fW, 100.0f), "get viewport");

	iRet = xuiScrollModelSetContentSize(&tModel, 260.0f, 180.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set content size");
	iRet = xuiScrollModelGetContentSize(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollModelNear(fX, 260.0f) && __xuiScrollModelNear(fY, 180.0f), "get content size");
	iRet = xuiScrollModelGetMaxOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollModelNear(fX, 160.0f) && __xuiScrollModelNear(fY, 100.0f), "max offset");

	iRet = xuiScrollModelSetOffset(&tModel, 300.0f, 200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set offset clamp");
	iRet = xuiScrollModelGetOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollModelNear(fX, 160.0f) && __xuiScrollModelNear(fY, 100.0f), "offset clamped");

	iRet = xuiScrollModelScrollBy(&tModel, -30.0f, -40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll by");
	iRet = xuiScrollModelGetOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollModelNear(fX, 130.0f) && __xuiScrollModelNear(fY, 60.0f), "scroll by offset");

	iRet = xuiScrollModelEnsureRectVisible(&tModel, (xui_rect_t){20.0f, 15.0f, 30.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure rect");
	iRet = xuiScrollModelGetOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollModelNear(fX, 20.0f) && __xuiScrollModelNear(fY, 15.0f), "ensure rect offset");

	tPoint = xuiScrollModelScreenToViewport(&tModel, 15.0f, 25.0f);
	XUI_TEST_CHECK(__xuiScrollModelNear(tPoint.fX, 5.0f) && __xuiScrollModelNear(tPoint.fY, 5.0f), "screen to viewport");
	tPoint = xuiScrollModelViewportToContent(&tModel, tPoint.fX, tPoint.fY);
	XUI_TEST_CHECK(__xuiScrollModelNear(tPoint.fX, 25.0f) && __xuiScrollModelNear(tPoint.fY, 20.0f), "viewport to content");
	tPoint = xuiScrollModelContentToScreen(&tModel, 25.0f, 20.0f);
	XUI_TEST_CHECK(__xuiScrollModelNear(tPoint.fX, 15.0f) && __xuiScrollModelNear(tPoint.fY, 25.0f), "content to screen");

	iRet = xuiScrollModelSetContentSize(&tModel, XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide content size");
	iRet = xuiScrollModelSetOffset(&tModel, XUI_LAYOUT_UNBOUNDED - 512.0f,
		XUI_LAYOUT_UNBOUNDED - 512.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide offset");
	iRet = xuiScrollModelScrollBy(&tModel, XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide positive scroll saturates");
	iRet = xuiScrollModelGetMaxOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK, "get wide max offset");
	iRet = xuiScrollModelGetOffset(&tModel, &fOldX, &fOldY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOldX == fX && fOldY == fY,
		"wide positive scroll reaches max offset");
	iRet = xuiScrollModelScrollBy(&tModel, -XUI_LAYOUT_UNBOUNDED, -XUI_LAYOUT_UNBOUNDED);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide negative scroll saturates");
	iRet = xuiScrollModelGetOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && fX == 0.0f && fY == 0.0f,
		"wide negative scroll reaches zero");

	iRet = xuiScrollModelEnsureRectVisible(&tModel,
		(xui_rect_t){(int)XUI_LAYOUT_UNBOUNDED, (int)XUI_LAYOUT_UNBOUNDED,
			(int)XUI_LAYOUT_UNBOUNDED, (int)XUI_LAYOUT_UNBOUNDED});
	XUI_TEST_CHECK(iRet == XUI_OK, "extreme rectangle remains valid");
	iRet = xuiScrollModelGetMaxOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK, "get max after extreme rectangle");
	iRet = xuiScrollModelGetOffset(&tModel, &fOldX, &fOldY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOldX == fX && fOldY == fY,
		"extreme rectangle saturates without integer overflow");

	fOldX = fX;
	fOldY = fY;
	iRet = xuiScrollModelScrollBy(&tModel, NAN, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "nan delta rejected");
	iRet = xuiScrollModelGetOffset(&tModel, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && fX == fOldX && fY == fOldY,
		"rejected delta preserves offset");

	tPoint = xuiScrollModelScreenToViewport(&tModel, INFINITY, 0.0f);
	XUI_TEST_CHECK(tPoint.fX == 0.0f && tPoint.fY == 0.0f,
		"invalid screen coordinate returns zero viewport point");
	tPoint = xuiScrollModelScreenToContent(&tModel, NAN, 0.0f);
	XUI_TEST_CHECK(tPoint.fX == 0.0f && tPoint.fY == 0.0f,
		"invalid screen coordinate returns zero content point");
	tPoint = xuiScrollModelContentToScreen(&tModel, 0.0f, -INFINITY);
	XUI_TEST_CHECK(tPoint.fX == 0.0f && tPoint.fY == 0.0f,
		"invalid content coordinate returns zero screen point");

cleanup:
	if ( iFailed ) {
		return 1;
	}
	printf("xui_scroll_model_test passed\n");
	return 0;
}
