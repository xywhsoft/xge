#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_icon_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_icon_custom_test_t {
	int iDrawCount;
	int iPrepareCount;
	int iDestroyCount;
} xui_icon_custom_test_t;

static int __xuiIconRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) &&
	       (tRect.fW == fW) && (tRect.fH == fH);
}

static int __xuiIconCustomMeasure(xui_icon pIcon, xui_vec2_t* pSize, void* pUser)
{
	(void)pIcon;
	(void)pUser;
	pSize->fX = 20.0f;
	pSize->fY = 10.0f;
	return XUI_OK;
}

static int __xuiIconCustomPrepare(xui_icon pIcon, int iPixelWidth, int iPixelHeight, void* pUser)
{
	xui_icon_custom_test_t* pTest;

	(void)pIcon;
	if ( (iPixelWidth <= 0) || (iPixelHeight <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	pTest = (xui_icon_custom_test_t*)pUser;
	pTest->iPrepareCount++;
	return XUI_OK;
}

static int __xuiIconCustomDraw(xui_icon pIcon, xui_painter pPainter, xui_rect_t tRect, const xui_icon_draw_desc_t* pDesc, void* pUser)
{
	xui_icon_custom_test_t* pTest;
	uint32_t iColor;

	(void)pIcon;
	pTest = (xui_icon_custom_test_t*)pUser;
	pTest->iDrawCount++;
	iColor = (pDesc != NULL) ? pDesc->iColor : XUI_COLOR_WHITE;
	return xuiPainterFillRect(pPainter, tRect, iColor);
}

static void __xuiIconCustomDestroy(xui_icon pIcon, void* pUser)
{
	xui_icon_custom_test_t* pTest;

	(void)pIcon;
	pTest = (xui_icon_custom_test_t*)pUser;
	pTest->iDestroyCount++;
}

static void __xuiIconResourceDestroy(xui_context pContext, void* pHandle, void* pUser)
{
	xui_test_proxy_state_t* pState;

	(void)pContext;
	pState = (xui_test_proxy_state_t*)pUser;
	if ( (pState != NULL) && (pHandle != NULL) ) {
		pState->tProxy.surfaceDestroy(&pState->tProxy, (xui_surface)pHandle);
	}
}

static int __xuiIconTestRegistry(void)
{
	static const unsigned char arrRaster[] = {1, 2, 3, 4};
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_icon_category pCategory;
	xui_icon_category pRetainedCategory;
	xui_icon_category_desc_t tCategoryDesc;
	xui_icon_desc_t tIconDesc;
	xui_icon pFirst;
	xui_icon pSecond;
	xui_icon pThird;
	xui_icon pAlias;
	xui_icon_id iFirstId;
	xui_icon_id iSecondId;
	xui_icon_id iThirdId;
	uint32_t iGeneration;
	int iFailed;
	int iRet;

	pContext = NULL;
	pCategory = NULL;
	pRetainedCategory = NULL;
	pFirst = NULL;
	pSecond = NULL;
	pThird = NULL;
	pAlias = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	xuiIconCategoryDescDefault(&tCategoryDesc);
	xuiIconDescDefault(&tIconDesc);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "registry context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "registry proxy set");
	iRet = xuiIconCategoryCreate(pContext, "ui", &tCategoryDesc, &pCategory);
	XUI_TEST_CHECK(iRet == XUI_OK && pCategory != NULL, "category create");
	XUI_TEST_CHECK(xuiIconCategoryCreate(pContext, "ui", NULL, NULL) == XUI_ERROR_ALREADY_INITIALIZED, "category duplicate");
	XUI_TEST_CHECK(xuiIconCategoryGetCount(pContext) == 1, "category count");
	XUI_TEST_CHECK(xuiIconCategoryFind(pContext, "ui") == pCategory, "category find");
	XUI_TEST_CHECK(xuiIconCategoryGetAt(pContext, 0) == pCategory, "category ordered lookup");
	XUI_TEST_CHECK(strcmp(xuiIconCategoryGetName(pCategory), "ui") == 0, "category name");

	tIconDesc.sDisplayName = "First icon";
	tIconDesc.sTags = "action,primary";
	iRet = xuiIconAddRasterMemory(pCategory, "first", arrRaster, (int)sizeof(arrRaster), &tIconDesc, &pFirst);
	XUI_TEST_CHECK(iRet == XUI_OK && pFirst != NULL, "first icon add");
	iFirstId = xuiIconGetId(pFirst);
	XUI_TEST_CHECK(iFirstId == 1u, "first id");
	XUI_TEST_CHECK(strcmp(xuiIconGetDisplayName(pFirst), "First icon") == 0, "display name");
	XUI_TEST_CHECK(strcmp(xuiIconGetTags(pFirst), "action,primary") == 0, "tags");

	xuiIconDescDefault(&tIconDesc);
	iRet = xuiIconAddSvgPath(
		pCategory,
		"second",
		"M2 2 L22 22",
		(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f},
		NULL,
		&tIconDesc,
		&pSecond);
	XUI_TEST_CHECK(iRet == XUI_OK && pSecond != NULL, "second icon add");
	iSecondId = xuiIconGetId(pSecond);
	XUI_TEST_CHECK(iSecondId == 2u, "second id");
	XUI_TEST_CHECK(xuiIconCategoryGetIconCount(pCategory) == 2, "icon count");
	XUI_TEST_CHECK(xuiIconFind(pCategory, "second") == pSecond, "icon name lookup");
	XUI_TEST_CHECK(xuiIconFindById(pCategory, iSecondId) == pSecond, "icon id lookup");

	XUI_TEST_CHECK(xuiIconRemoveById(pCategory, iFirstId) == XUI_OK, "remove first");
	XUI_TEST_CHECK(xuiIconFindById(pCategory, iFirstId) == NULL, "removed id tombstone");
	iRet = xuiIconAddRasterMemory(pCategory, "third", arrRaster, (int)sizeof(arrRaster), NULL, &pThird);
	XUI_TEST_CHECK(iRet == XUI_OK && pThird != NULL, "third icon add");
	iThirdId = xuiIconGetId(pThird);
	XUI_TEST_CHECK(iThirdId == 3u, "removed id not reused");
	XUI_TEST_CHECK(xuiIconCategoryGetIconSlotCount(pCategory) == 3u, "slot count includes tombstone");
	XUI_TEST_CHECK(xuiIconCategoryGetIconAt(pCategory, 0) == pSecond, "ordered active first");
	XUI_TEST_CHECK(xuiIconCategoryGetIconAt(pCategory, 1) == pThird, "ordered active second");

	iRet = xuiIconAddAlias(pCategory, "third_alias", pThird, NULL, &pAlias);
	XUI_TEST_CHECK(iRet == XUI_OK && pAlias != NULL, "alias add");
	XUI_TEST_CHECK(xuiIconGetRefCount(pThird) == 2, "alias retains target");
	XUI_TEST_CHECK(xuiIconRemove(pCategory, "third_alias") == XUI_OK, "alias remove");
	pAlias = NULL;
	XUI_TEST_CHECK(xuiIconGetRefCount(pThird) == 1, "alias releases target");

	iGeneration = xuiIconCategoryGetGeneration(pCategory);
	XUI_TEST_CHECK(xuiIconCategoryBeginUpdate(pCategory) == XUI_OK, "begin batch");
	XUI_TEST_CHECK(xuiIconAddRasterMemory(pCategory, "batch_a", arrRaster, (int)sizeof(arrRaster), NULL, NULL) == XUI_OK, "batch add a");
	XUI_TEST_CHECK(xuiIconAddRasterMemory(pCategory, "batch_b", arrRaster, (int)sizeof(arrRaster), NULL, NULL) == XUI_OK, "batch add b");
	XUI_TEST_CHECK(xuiIconCategoryGetGeneration(pCategory) == iGeneration, "batch defers generation");
	XUI_TEST_CHECK(xuiIconCategoryEndUpdate(pCategory) == XUI_OK, "end batch");
	XUI_TEST_CHECK(xuiIconCategoryGetGeneration(pCategory) == iGeneration + 1u, "batch advances once");

	xuiIconClear(pCategory);
	XUI_TEST_CHECK(xuiIconCategoryGetIconCount(pCategory) == 0, "clear active icons");
	XUI_TEST_CHECK(xuiIconCategoryGetIconSlotCount(pCategory) == 6u, "clear keeps tombstones");
	iRet = xuiIconAddRasterMemory(pCategory, "after_clear", arrRaster, (int)sizeof(arrRaster), NULL, &pThird);
	XUI_TEST_CHECK(iRet == XUI_OK, "add after clear");
	XUI_TEST_CHECK(xuiIconGetId(pThird) == 7u, "clear does not reuse ids");

	XUI_TEST_CHECK(xuiIconCategoryAddRef(pCategory) == XUI_OK, "category add ref");
	pRetainedCategory = pCategory;
	XUI_TEST_CHECK(xuiIconCategoryRemove(pContext, "ui") == XUI_OK, "category remove");
	pCategory = NULL;
	XUI_TEST_CHECK(xuiIconCategoryFind(pContext, "ui") == NULL, "removed category absent");
	XUI_TEST_CHECK(xuiIconCategoryGetRefCount(pRetainedCategory) == 1, "retained detached category");
	XUI_TEST_CHECK(xuiIconCategoryRelease(pRetainedCategory) == XUI_OK, "release detached category");
	pRetainedCategory = NULL;

cleanup:
	if ( pRetainedCategory != NULL ) {
		(void)xuiIconCategoryRelease(pRetainedCategory);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	return iFailed;
}

static int __xuiIconTestRenderAndOwnership(void)
{
	static const char sSvg[] = "<svg viewBox=\"0 0 24 24\"><path d=\"M2 2h20v20H2z\"/></svg>";
	static const unsigned char arrRaster[] = {9, 8, 7, 6};
	xui_test_proxy_state_t tState;
	xui_icon_custom_test_t tCustomState;
	xui_icon_category_desc_t tCategoryDesc;
	xui_icon_draw_desc_t tDrawDesc;
	xui_icon_custom_desc_t tCustomDesc;
	xui_resource_desc_t tResourceDesc;
	xui_context pContext;
	xui_icon_category pFixed;
	xui_icon_category pFlexible;
	xui_icon pPath;
	xui_icon pSvg;
	xui_icon pSvgFile;
	xui_icon pRaster;
	xui_icon pRasterFile;
	xui_icon pOwned;
	xui_icon pCustom;
	xui_icon pResourceIcon;
	xui_resource pResource;
	xui_surface pResourceSurface;
	xui_surface pOwnedSurface;
	xui_surface pTarget;
	xui_painter pPainter;
	xui_rect_t tLastRect;
	int iDestroyBefore;
	int iFailed;
	int iRet;

	memset(&tCustomState, 0, sizeof(tCustomState));
	pContext = NULL;
	pFixed = NULL;
	pFlexible = NULL;
	pPath = NULL;
	pSvg = NULL;
	pSvgFile = NULL;
	pRaster = NULL;
	pRasterFile = NULL;
	pOwned = NULL;
	pCustom = NULL;
	pResourceIcon = NULL;
	pResource = NULL;
	pResourceSurface = NULL;
	pOwnedSurface = NULL;
	pTarget = NULL;
	pPainter = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "render proxy set");
	XUI_TEST_CHECK(xuiTestSurfaceCreate(&tState, &pTarget, 256, 256, XUI_SURFACE_USAGE_TARGET) == XUI_OK, "target create");

	xuiIconCategoryDescDefault(&tCategoryDesc);
	tCategoryDesc.fWidth = 16.0f;
	tCategoryDesc.fHeight = 16.0f;
	XUI_TEST_CHECK(xuiIconCategoryCreate(pContext, "fixed", &tCategoryDesc, &pFixed) == XUI_OK, "fixed category");
	XUI_TEST_CHECK(xuiIconAddSvgPath(
		pFixed,
		"path",
		"M0 0 H24 V24 H0 Z",
		(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f},
		NULL,
		NULL,
		&pPath) == XUI_OK, "path add");

	xuiIconCategoryDescDefault(&tCategoryDesc);
	tCategoryDesc.iSizeMode = XUI_ICON_SIZE_UNRESTRICTED;
	tCategoryDesc.fWidth = 0.0f;
	tCategoryDesc.fHeight = 0.0f;
	tCategoryDesc.iCacheCapacity = 2;
	XUI_TEST_CHECK(xuiIconCategoryCreate(pContext, "flexible", &tCategoryDesc, &pFlexible) == XUI_OK, "flexible category");
	XUI_TEST_CHECK(xuiIconAddSvgMemory(pFlexible, "svg", sSvg, (int)sizeof(sSvg), NULL, &pSvg) == XUI_OK, "svg memory add");
	XUI_TEST_CHECK(xuiIconAddRasterMemory(pFlexible, "raster", arrRaster, (int)sizeof(arrRaster), NULL, &pRaster) == XUI_OK, "raster memory add");
	XUI_TEST_CHECK(xuiIconAddSvgFile(pFlexible, "svg_file", "virtual.svg", NULL, &pSvgFile) == XUI_OK, "svg file add");
	XUI_TEST_CHECK(xuiIconAddRasterFile(pFlexible, "raster_file", "virtual.png", NULL, &pRasterFile) == XUI_OK, "raster file add");
	XUI_TEST_CHECK(xuiTestSurfaceCreate(&tState, &pResourceSurface, 18, 12, 0) == XUI_OK, "resource surface create");
	memset(&tResourceDesc, 0, sizeof(tResourceDesc));
	tResourceDesc.iSize = sizeof(tResourceDesc);
	tResourceDesc.sName = "managed_surface";
	tResourceDesc.iKind = XUI_RESOURCE_SURFACE;
	tResourceDesc.pHandle = pResourceSurface;
	tResourceDesc.pUser = &tState;
	tResourceDesc.onDestroy = __xuiIconResourceDestroy;
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pResource, &tResourceDesc) == XUI_OK, "surface resource set");
	pResourceSurface = NULL;
	XUI_TEST_CHECK(xuiIconAddResource(
		pFlexible,
		"resource",
		pResource,
		(xui_rect_t){0.0f, 0.0f, 18.0f, 12.0f},
		NULL,
		&pResourceIcon) == XUI_OK, "resource icon add");

	memset(&tCustomDesc, 0, sizeof(tCustomDesc));
	tCustomDesc.iSize = sizeof(tCustomDesc);
	tCustomDesc.onMeasure = __xuiIconCustomMeasure;
	tCustomDesc.onPrepare = __xuiIconCustomPrepare;
	tCustomDesc.onDraw = __xuiIconCustomDraw;
	tCustomDesc.onDestroy = __xuiIconCustomDestroy;
	tCustomDesc.pUser = &tCustomState;
	XUI_TEST_CHECK(xuiIconAddCustom(pFlexible, "custom", &tCustomDesc, NULL, &pCustom) == XUI_OK, "custom add");

	XUI_TEST_CHECK(xuiPainterBegin(pContext, pTarget, &pPainter) == XUI_OK, "painter begin");
	xuiIconDrawDescDefault(&tDrawDesc);
	tDrawDesc.iColor = XUI_COLOR_RGBA(20, 40, 60, 255);
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pPath, (xui_rect_t){0.0f, 0.0f, 100.0f, 100.0f}, &tDrawDesc) == XUI_OK, "fixed path draw");
	tLastRect = xuiTestSurfaceGetLastDst(pTarget);
	XUI_TEST_CHECK(__xuiIconRectEq(tLastRect, 42.0f, 42.0f, 16.0f, 16.0f), "fixed category draw size");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastColor(pTarget) == tDrawDesc.iColor, "path tint");

	XUI_TEST_CHECK(xuiIconDraw(pPainter, pSvg, (xui_rect_t){0.0f, 0.0f, 32.0f, 20.0f}, NULL) == XUI_OK, "svg first draw");
	XUI_TEST_CHECK(xuiTestProxyGetSvgSurfaceLoadCount(&tState) == 1, "svg first cache load");
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pSvg, (xui_rect_t){0.0f, 0.0f, 32.0f, 20.0f}, NULL) == XUI_OK, "svg cached draw");
	XUI_TEST_CHECK(xuiTestProxyGetSvgSurfaceLoadCount(&tState) == 1, "svg cache hit");
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pSvg, (xui_rect_t){0.0f, 0.0f, 64.0f, 40.0f}, NULL) == XUI_OK, "svg second size");
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pSvg, (xui_rect_t){0.0f, 0.0f, 96.0f, 60.0f}, NULL) == XUI_OK, "svg third size");
	XUI_TEST_CHECK(xuiTestProxyGetSvgSurfaceLoadCount(&tState) == 3, "svg size variants");
	XUI_TEST_CHECK(xuiTestProxyGetSurfaceDestroyCount(&tState) >= 1, "svg lru eviction");

	XUI_TEST_CHECK(xuiIconDraw(pPainter, pRaster, (xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f}, NULL) == XUI_OK, "raster first draw");
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pRaster, (xui_rect_t){0.0f, 0.0f, 48.0f, 48.0f}, NULL) == XUI_OK, "raster second draw");
	XUI_TEST_CHECK(xuiTestProxyGetSurfaceLoadCount(&tState) == 1, "raster loads once");
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pResourceIcon, (xui_rect_t){0.0f, 0.0f, 36.0f, 24.0f}, NULL) == XUI_OK, "resource icon draw");
	XUI_TEST_CHECK(xuiResourceRemove(pResource) == XUI_OK, "resource remove");
	pResource = NULL;
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pResourceIcon, (xui_rect_t){0.0f, 0.0f, 36.0f, 24.0f}, NULL) == XUI_ERROR_RESOURCE_FAILED, "removed resource detected");
	XUI_TEST_CHECK(xuiTestSurfaceCreate(&tState, &pResourceSurface, 24, 16, 0) == XUI_OK, "replacement resource surface create");
	tResourceDesc.pHandle = pResourceSurface;
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pResource, &tResourceDesc) == XUI_OK, "replacement resource set");
	pResourceSurface = NULL;
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pResourceIcon, (xui_rect_t){0.0f, 0.0f, 48.0f, 32.0f}, NULL) == XUI_OK, "replacement resource resolved");

	XUI_TEST_CHECK(xuiIconPrepare(pCustom, 30.0f, 15.0f) == XUI_OK, "custom prepare");
	XUI_TEST_CHECK(tCustomState.iPrepareCount == 1, "custom prepare callback");
	XUI_TEST_CHECK(xuiIconDraw(pPainter, pCustom, (xui_rect_t){0.0f, 0.0f, 40.0f, 40.0f}, &tDrawDesc) == XUI_OK, "custom draw");
	XUI_TEST_CHECK(tCustomState.iDrawCount == 1, "custom draw callback");

	XUI_TEST_CHECK(xuiPainterEnd(pPainter) == XUI_OK, "painter end");
	pPainter = NULL;

	iDestroyBefore = xuiTestProxyGetSurfaceDestroyCount(&tState);
	XUI_TEST_CHECK(xuiIconTouch(pSvg) == XUI_OK, "svg touch");
	XUI_TEST_CHECK(xuiTestProxyGetSurfaceDestroyCount(&tState) == iDestroyBefore + 2, "touch clears svg cache");
	iDestroyBefore = xuiTestProxyGetSvgSurfaceLoadCount(&tState);
	XUI_TEST_CHECK(xuiIconPrepare(pSvgFile, 22.0f, 18.0f) == XUI_OK, "svg file prepare");
	XUI_TEST_CHECK(xuiTestProxyGetSvgSurfaceLoadCount(&tState) == iDestroyBefore + 1, "svg file loader used");
	iDestroyBefore = xuiTestProxyGetSurfaceLoadCount(&tState);
	XUI_TEST_CHECK(xuiIconPrepare(pRasterFile, 22.0f, 18.0f) == XUI_OK, "raster file prepare");
	XUI_TEST_CHECK(xuiTestProxyGetSurfaceLoadCount(&tState) == iDestroyBefore + 1, "raster file loader used");

	XUI_TEST_CHECK(xuiTestSurfaceCreate(&tState, &pOwnedSurface, 12, 12, 0) == XUI_OK, "owned surface create");
	XUI_TEST_CHECK(xuiIconAddSurface(
		pFlexible,
		"owned",
		pOwnedSurface,
		(xui_rect_t){0.0f, 0.0f, 12.0f, 12.0f},
		XUI_ICON_SURFACE_TAKE_OWNERSHIP,
		NULL,
		&pOwned) == XUI_OK, "owned surface add");
	pOwnedSurface = NULL;
	iDestroyBefore = xuiTestProxyGetSurfaceDestroyCount(&tState);
	XUI_TEST_CHECK(xuiIconRemove(pFlexible, "owned") == XUI_OK, "owned surface remove");
	pOwned = NULL;
	XUI_TEST_CHECK(xuiTestProxyGetSurfaceDestroyCount(&tState) == iDestroyBefore + 1, "owned surface destroyed");

	XUI_TEST_CHECK(xuiIconRemove(pFlexible, "custom") == XUI_OK, "custom remove");
	pCustom = NULL;
	XUI_TEST_CHECK(tCustomState.iDestroyCount == 1, "custom destroy callback");

cleanup:
	if ( pPainter != NULL ) {
		(void)xuiPainterEnd(pPainter);
	}
	if ( pOwnedSurface != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pOwnedSurface);
	}
	if ( pResourceSurface != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pResourceSurface);
	}
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	return iFailed;
}

int main(void)
{
	if ( __xuiIconTestRegistry() != 0 ) return 1;
	if ( __xuiIconTestRenderAndOwnership() != 0 ) return 1;
	printf("xui_icon_test passed\n");
	return 0;
}
