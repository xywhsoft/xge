#include "xui.h"

#include <stdio.h>

static int g_iMarginFailAllocation = -1;

static void* xuiTestMarginRealloc(void* pMemory, size_t iSize)
{
	if ( g_iMarginFailAllocation >= 0 && g_iMarginFailAllocation-- == 0 ) return NULL;
	return xrtRealloc(pMemory, iSize);
}

#define xrtRealloc xuiTestMarginRealloc
#include "../../src/xui/code/xui_code_margin.c"
#undef xrtRealloc

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_margin_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int xuiTestMarginRender(xui_widget_t* pWidget, int iMarginId, int iLine, xui_draw_context_t* pDraw, xui_rect_t tRect, void* pUser)
{
	(void)pWidget;
	(void)iMarginId;
	(void)iLine;
	(void)pDraw;
	(void)tRect;
	(void)pUser;
	return XUI_OK;
}

int main(void)
{
	xui_code_margin_model pModel;
	xui_code_margin_desc_t tDesc;
	xui_code_margin_info_t arrInfos[8];
	xui_code_margin_info_t tInfo;
	xui_code_margin_hit_t tHit;
	xui_rect_t tViewport;
	xui_rect_t tTextRect;
	xui_rect_t tRect;
	float fWidth;
	int iCount;
	int iFailed;
	int iRet;

	pModel = NULL;
	iFailed = 0;
	tViewport = (xui_rect_t){10.0f, 20.0f, 300.0f, 120.0f};

	iRet = xuiCodeMarginModelCreate(&pModel);
	XUI_TEST_CHECK(iRet == XUI_OK && pModel != NULL, "model create");
	iRet = xuiCodeMarginModelLoadDefaults(pModel, 1, 1, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeMarginModelGetCount(pModel) == 5, "load defaults");
	iRet = xuiCodeMarginModelGetTotalWidth(pModel, &fWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && (int)fWidth == 92, "default total width");
	iRet = xuiCodeMarginModelLayout(pModel, tViewport, arrInfos, 8, &iCount, &tTextRect);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 5, "layout count");
	XUI_TEST_CHECK(arrInfos[0].iKind == XUI_CODE_MARGIN_CHANGE && arrInfos[0].tRect.fX == 10.0f && arrInfos[0].tRect.fW == 4.0f, "change margin rect");
	XUI_TEST_CHECK(arrInfos[1].iKind == XUI_CODE_MARGIN_MARKER && arrInfos[1].tRect.fX == 14.0f, "marker margin rect");
	XUI_TEST_CHECK(tTextRect.fX == 102.0f && tTextRect.fW == 208.0f, "text rect");

	iRet = xuiCodeMarginModelHitTest(pModel, tViewport, 17.0f, 53.0f, 16.0f, 32.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iKind == XUI_CODE_MARGIN_MARKER, "hit marker");
	XUI_TEST_CHECK(tHit.iLine == 4, "hit line with scroll");
	iRet = xuiCodeMarginModelGetRect(pModel, 4, tViewport, &tRect);
	XUI_TEST_CHECK(iRet == XUI_OK && tRect.fX == 44.0f && tRect.fW == 44.0f, "line number rect");

	iRet = xuiCodeMarginModelSetWidth(pModel, 4, 60.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set width");
	iRet = xuiCodeMarginModelGetTotalWidth(pModel, &fWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && (int)fWidth == 108, "updated total width");
	iRet = xuiCodeMarginModelSetVisible(pModel, 2, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide marker");
	iRet = xuiCodeMarginModelHitTest(pModel, tViewport, 17.0f, 53.0f, 16.0f, 32.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iKind == XUI_CODE_MARGIN_FOLD, "hidden marker shifts fold");

	tDesc.iSize = sizeof(tDesc);
	tDesc.iId = 20;
	tDesc.iKind = XUI_CODE_MARGIN_CUSTOM;
	tDesc.fWidth = 11.0f;
	tDesc.iFlags = XUI_CODE_MARGIN_VISIBLE | XUI_CODE_MARGIN_CLICKABLE;
	tDesc.onRender = xuiTestMarginRender;
	tDesc.onEvent = NULL;
	tDesc.pUser = &iFailed;
	iRet = xuiCodeMarginModelAdd(pModel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK, "add custom");
	iRet = xuiCodeMarginModelGet(pModel, xuiCodeMarginModelGetCount(pModel) - 1, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iId == 20 && tInfo.onRender == xuiTestMarginRender && tInfo.pUser == &iFailed, "custom info");
	iRet = xuiCodeMarginModelRemove(pModel, 20);
	XUI_TEST_CHECK(iRet == XUI_OK, "remove custom");
	iRet = xuiCodeMarginModelRemove(pModel, 20);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "remove missing");
	tDesc.iSize = sizeof(tDesc) - 1u;
	iRet = xuiCodeMarginModelAdd(pModel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "undersized descriptor rejected");
	tDesc.iSize = sizeof(tDesc);
	tDesc.fWidth = NAN;
	iRet = xuiCodeMarginModelAdd(pModel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "NaN width rejected");
	tDesc.fWidth = 11.0f;

	xuiCodeMarginModelClear(pModel);
	tDesc.iId = 20;
	tDesc.fWidth = 1.4f;
	tDesc.iFlags = XUI_CODE_MARGIN_VISIBLE;
	iRet = xuiCodeMarginModelAdd(pModel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK, "fractional first margin add");
	tDesc.iId = 21;
	iRet = xuiCodeMarginModelAdd(pModel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK, "fractional second margin add");
	iRet = xuiCodeMarginModelLayout(pModel, tViewport, arrInfos, 1,
		&iCount, &tTextRect);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2,
		"layout reports total count beyond capacity");
	XUI_TEST_CHECK(arrInfos[0].tRect.fX == 10 && arrInfos[0].tRect.fW == 1 &&
		tTextRect.fX == 13 && tTextRect.fW == 297,
		"fractional margin edges snap cumulatively");
	iRet = xuiCodeMarginModelGetRect(pModel, 21, tViewport, &tRect);
	XUI_TEST_CHECK(iRet == XUI_OK && tRect.fX == 11 && tRect.fW == 2,
		"fractional margin rect shares snapped edge");
	iRet = xuiCodeMarginModelHitTest(pModel, tViewport, 11.0f, 53.0f,
		16.0f, 32.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iId == 21 &&
		tHit.tRect.fX == 11 && tHit.tRect.fW == 2,
		"fractional margin hit uses layout geometry");
	iRet = xuiCodeMarginModelHitTest(pModel, tViewport, 310.0f, 53.0f,
		16.0f, 32.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED,
		"margin hit rejects viewport right edge");
	iRet = xuiCodeMarginModelLayout(pModel,
		(xui_rect_t){INT_MAX, 0, 1, 1}, arrInfos, 8, &iCount, &tTextRect);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iCount == 0,
		"overflowing integer viewport rejected");
	iRet = xuiCodeMarginModelLoadDefaults(pModel, 1, 1, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeMarginModelGetCount(pModel) == 5,
		"defaults restored after fractional geometry checks");
	tDesc.iId = 20;
	tDesc.fWidth = 11.0f;
	tDesc.iFlags = XUI_CODE_MARGIN_VISIBLE | XUI_CODE_MARGIN_CLICKABLE;
	iRet = xuiCodeMarginModelLayout(pModel, tViewport, NULL, 1,
		&iCount, &tTextRect);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"missing layout output rejected");
	iRet = xuiCodeMarginModelHitTest(pModel, tViewport, 17.0f, 53.0f,
		16.0f, -1.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"negative margin scroll rejected");
	iRet = xuiCodeMarginModelHitTest(pModel, tViewport, 17.0f, 53.0f,
		1.0f, FLT_MAX, &tHit);
	XUI_TEST_CHECK(iRet == XUI_ERROR_LIMIT_EXCEEDED,
		"overflowing margin line rejected");

	iRet = xuiCodeMarginModelAdd(pModel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeMarginModelGetCount(pModel) == 6,
		"restore custom margin");
	g_iMarginFailAllocation = 0;
	iRet = xuiCodeMarginModelLoadDefaults(pModel, 0, 0, 0, 0);
	g_iMarginFailAllocation = -1;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY &&
		xuiCodeMarginModelGetCount(pModel) == 6,
		"default load failure preserves model");
	iRet = xuiCodeMarginModelGet(pModel, 5, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iId == 20,
		"default load failure preserves custom margin");
	iRet = xuiCodeMarginModelLoadDefaults(pModel, 0, 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeMarginModelGetCount(pModel) == 1,
		"default load succeeds transactionally");
	xuiCodeMarginModelClear(pModel);
	XUI_TEST_CHECK(xuiCodeMarginModelGetCount(pModel) == 0, "clear model");

cleanup:
	xuiCodeMarginModelDestroy(pModel);
	if ( iFailed ) return 1;
	printf("xui_code_margin_test passed\n");
	return 0;
}
