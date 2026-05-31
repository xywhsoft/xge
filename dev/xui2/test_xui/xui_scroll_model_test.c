#include "xui.h"

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

cleanup:
	if ( iFailed ) {
		return 1;
	}
	printf("xui_scroll_model_test passed\n");
	return 0;
}
