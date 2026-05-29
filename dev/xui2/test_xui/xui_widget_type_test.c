#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_widget_type_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_widget_type_test_data_t {
	int iMagic;
	int iCreateValue;
	int iMeasureCount;
	int iArrangeCount;
	int iCacheCount;
} xui_widget_type_test_data_t;

typedef struct xui_widget_type_test_user_t {
	int iInitCount;
	int iDestroyCount;
	int iParentInitCount;
	int iParentDestroyCount;
} xui_widget_type_test_user_t;

static xui_layout_t __xuiTestLayoutDefault(void)
{
	xui_layout_t tLayout;

	memset(&tLayout, 0, sizeof(tLayout));
	tLayout.iLayoutType = XUI_LAYOUT_ROW;
	tLayout.iWidthMode = XUI_SIZE_CONTENT;
	tLayout.iHeightMode = XUI_SIZE_CONTENT;
	tLayout.iFlowMode = XUI_FLOW_BLOCK;
	tLayout.iAlignX = XUI_ALIGN_START;
	tLayout.iAlignY = XUI_ALIGN_START;
	tLayout.iTableRowSpan = 1;
	tLayout.iTableColumnSpan = 1;
	tLayout.fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	tLayout.fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	tLayout.fShrink = 1.0f;
	tLayout.fGap = 3.0f;
	return tLayout;
}

static int __xuiTestParentInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_widget_type_test_data_t* pData;
	xui_widget_type_test_user_t* pTypeUser;

	(void)pWidget;
	pData = (xui_widget_type_test_data_t*)pTypeData;
	pTypeUser = (xui_widget_type_test_user_t*)pUser;
	pData->iMagic = 0x12345678;
	pData->iCreateValue = (pCreateData != NULL) ? *(const int*)pCreateData : 0;
	pTypeUser->iParentInitCount++;
	return XUI_OK;
}

static void __xuiTestParentDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_widget_type_test_data_t* pData;
	xui_widget_type_test_user_t* pTypeUser;

	(void)pWidget;
	pData = (xui_widget_type_test_data_t*)pTypeData;
	pTypeUser = (xui_widget_type_test_user_t*)pUser;
	if ( pData != NULL ) {
		pTypeUser->iParentDestroyCount++;
	}
}

static int __xuiTestChildInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_widget_type_test_data_t* pData;
	xui_widget_type_test_user_t* pTypeUser;

	(void)pWidget;
	(void)pCreateData;
	pData = (xui_widget_type_test_data_t*)pTypeData;
	pTypeUser = (xui_widget_type_test_user_t*)pUser;
	pData->iMagic ^= 0x0000FFFF;
	pTypeUser->iInitCount++;
	return XUI_OK;
}

static void __xuiTestChildDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_widget_type_test_data_t* pData;
	xui_widget_type_test_user_t* pTypeUser;

	(void)pWidget;
	pData = (xui_widget_type_test_data_t*)pTypeData;
	pTypeUser = (xui_widget_type_test_user_t*)pUser;
	if ( pData->iMagic == (0x12345678 ^ 0x0000FFFF) ) {
		pTypeUser->iDestroyCount++;
	}
}

static int __xuiTestMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_widget_type_test_data_t* pData;

	(void)pWidget;
	(void)tConstraint;
	pData = (xui_widget_type_test_data_t*)pUser;
	pData->iMeasureCount++;
	pSize->fX = 23.0f;
	pSize->fY = 7.0f;
	return XUI_OK;
}

static int __xuiTestArrange(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_widget_type_test_data_t* pData;

	(void)pWidget;
	(void)tContentRect;
	pData = (xui_widget_type_test_data_t*)pUser;
	pData->iArrangeCount++;
	return XUI_OK;
}

static int __xuiTestCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget_type_test_data_t* pData;

	(void)pWidget;
	(void)pDraw;
	(void)iStateId;
	pData = (xui_widget_type_test_data_t*)pUser;
	pData->iCacheCount++;
	return XUI_OK;
}

int main(void)
{
	xui_context pContext;
	xui_widget_type pBaseType;
	xui_widget_type pParentType;
	xui_widget_type pChildType;
	xui_widget pWidget;
	xui_widget_type_desc_t tDesc;
	xui_cache_policy_t tPolicy;
	xui_widget_type_test_user_t tParentUser;
	xui_widget_type_test_user_t tChildUser;
	xui_widget_type_test_data_t* pData;
	xui_widget_content_measure_proc onContentMeasure;
	xui_widget_cache_render_proc onCacheRender;
	xui_vec2_t tSize;
	xui_layout_t tLayout;
	void* pCallbackUser;
	int iCreateValue;
	int iRet;
	int iFailed;

	pContext = NULL;
	pParentType = NULL;
	pChildType = NULL;
	pWidget = NULL;
	iCreateValue = 42;
	iFailed = 0;
	memset(&tParentUser, 0, sizeof(tParentUser));
	memset(&tChildUser, 0, sizeof(tChildUser));

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");

	pBaseType = xuiWidgetGetBaseType();
	XUI_TEST_CHECK((pBaseType != NULL) && (xuiWidgetTypeGetName(pBaseType) != NULL), "base type missing");
	XUI_TEST_CHECK(strcmp(xuiWidgetTypeGetName(pBaseType), "widget") == 0, "base type name mismatch");
	XUI_TEST_CHECK(xuiWidgetFindType(pContext, "widget") == pBaseType, "base type lookup failed");

	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_NONE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "test.parent";
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_widget_type_test_data_t);
	tDesc.pUser = &tParentUser;
	tDesc.onInit = __xuiTestParentInit;
	tDesc.onDestroy = __xuiTestParentDestroy;
	tDesc.onContentMeasure = __xuiTestMeasure;
	tDesc.onLayoutArrange = __xuiTestArrange;
	tDesc.onCacheRender = __xuiTestCacheRender;
	tDesc.tLayout = __xuiTestLayoutDefault();
	tDesc.tCachePolicy = tPolicy;
	iRet = xuiWidgetRegisterType(pContext, &pParentType, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pParentType != NULL), "parent type register failed");
	XUI_TEST_CHECK(xuiWidgetFindType(pContext, "test.parent") == pParentType, "parent type lookup failed");
	XUI_TEST_CHECK(xuiWidgetTypeGetParent(pParentType) == pBaseType, "parent base chain failed");
	XUI_TEST_CHECK(xuiWidgetTypeGetUserData(pParentType) == &tParentUser, "parent type user data failed");
	XUI_TEST_CHECK(xuiWidgetRegisterType(pContext, &pChildType, &tDesc) == XUI_ERROR_ALREADY_INITIALIZED, "duplicate type should fail");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "test.child";
	tDesc.pParent = pParentType;
	tDesc.iTypeDataSize = sizeof(xui_widget_type_test_data_t);
	tDesc.pUser = &tChildUser;
	tDesc.onInit = __xuiTestChildInit;
	tDesc.onDestroy = __xuiTestChildDestroy;
	iRet = xuiWidgetRegisterType(pContext, &pChildType, &tDesc);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pChildType != NULL), "child type register failed");
	XUI_TEST_CHECK(xuiWidgetTypeGetParent(pChildType) == pParentType, "child parent chain failed");

	iRet = xuiWidgetCreateTyped(pContext, pChildType, &pWidget, &iCreateValue);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pWidget != NULL), "typed widget create failed");
	XUI_TEST_CHECK(xuiWidgetGetType(pWidget) == pChildType, "widget type getter failed");
	XUI_TEST_CHECK(xuiWidgetIsType(pWidget, pChildType) && xuiWidgetIsType(pWidget, pParentType) && xuiWidgetIsType(pWidget, pBaseType), "widget is-type failed");
	pData = (xui_widget_type_test_data_t*)xuiWidgetGetTypeData(pWidget);
	XUI_TEST_CHECK((pData != NULL) && (pData->iCreateValue == 42), "type data init failed");
	XUI_TEST_CHECK((tParentUser.iParentInitCount == 1) && (tChildUser.iInitCount == 1), "init chain failed");

	tLayout = xuiWidgetGetLayout(pWidget);
	XUI_TEST_CHECK((tLayout.iLayoutType == XUI_LAYOUT_ROW) && (tLayout.fGap == 3.0f), "default layout inheritance failed");
	XUI_TEST_CHECK(xuiWidgetGetCachePolicy(pWidget).iPolicy == XUI_CACHE_POLICY_NONE, "default cache policy inheritance failed");

	iRet = xuiWidgetMeasureContent(pWidget, (xui_vec2_t){100.0f, 100.0f}, &tSize);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tSize.fX == 23.0f) && (tSize.fY == 7.0f) && (pData->iMeasureCount == 1), "inherited content measure failed");
	iRet = xuiWidgetGetContentMeasureCallback(pWidget, &onContentMeasure, &pCallbackUser);
	XUI_TEST_CHECK((iRet == XUI_OK) && (onContentMeasure == __xuiTestMeasure) && (pCallbackUser == pData), "content callback install failed");
	iRet = xuiWidgetGetCacheRenderCallback(pWidget, &onCacheRender, &pCallbackUser);
	XUI_TEST_CHECK((iRet == XUI_OK) && (onCacheRender == __xuiTestCacheRender) && (pCallbackUser == pData), "cache callback install failed");

	XUI_TEST_CHECK(xuiWidgetUnregisterType(pChildType) == XUI_ERROR_ALREADY_INITIALIZED, "live type unregister should fail");
	xuiWidgetDestroy(pWidget);
	pWidget = NULL;
	XUI_TEST_CHECK((tChildUser.iDestroyCount == 1) && (tParentUser.iParentDestroyCount == 1), "destroy chain failed");

	iRet = xuiWidgetUnregisterType(pChildType);
	XUI_TEST_CHECK(iRet == XUI_OK, "child unregister failed");
	pChildType = NULL;
	iRet = xuiWidgetUnregisterType(pParentType);
	XUI_TEST_CHECK(iRet == XUI_OK, "parent unregister failed");
	pParentType = NULL;

cleanup:
	if ( pWidget != NULL ) {
		xuiWidgetDestroy(pWidget);
	}
	if ( pChildType != NULL ) {
		(void)xuiWidgetUnregisterType(pChildType);
	}
	if ( pParentType != NULL ) {
		(void)xuiWidgetUnregisterType(pParentType);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_widget_type_test passed\n");
	return 0;
}
