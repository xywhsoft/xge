#include "xui.h"

#include <stdio.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_layout_callback_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_layout_callback_state_t {
	xui_vec2_t tContentSize;
	xui_vec2_t tLayoutSize;
	int iContentMeasureCount;
	int iLayoutMeasureCount;
	int iLayoutArrangeCount;
} xui_layout_callback_state_t;

static int __xuiTestRectEquals(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static xui_thickness_t __xuiTestThickness(float fLeft, float fTop, float fRight, float fBottom)
{
	xui_thickness_t tValue;

	tValue.fLeft = fLeft;
	tValue.fTop = fTop;
	tValue.fRight = fRight;
	tValue.fBottom = fBottom;
	return tValue;
}

static int __xuiTestContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_layout_callback_state_t* pState;

	(void)pWidget;
	(void)tConstraint;
	pState = (xui_layout_callback_state_t*)pUser;
	pState->iContentMeasureCount++;
	*pSize = pState->tContentSize;
	return XUI_OK;
}

static int __xuiTestLayoutMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_layout_callback_state_t* pState;

	(void)pWidget;
	(void)tConstraint;
	pState = (xui_layout_callback_state_t*)pUser;
	pState->iLayoutMeasureCount++;
	*pSize = pState->tLayoutSize;
	return XUI_OK;
}

static int __xuiTestLayoutArrange(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_layout_callback_state_t* pState;
	xui_widget pChild;
	xui_rect_t tChildRect;

	pState = (xui_layout_callback_state_t*)pUser;
	pState->iLayoutArrangeCount++;
	pChild = xuiWidgetGetFirstChild(pWidget);
	if ( pChild == NULL ) {
		return XUI_OK;
	}
	tChildRect.fX = tContentRect.fX + 4.0f;
	tChildRect.fY = tContentRect.fY + 5.0f;
	tChildRect.fW = tContentRect.fW - 8.0f;
	tChildRect.fH = tContentRect.fH - 10.0f;
	return xuiWidgetArrange(pChild, tChildRect);
}

int main(void)
{
	xui_layout_callback_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pChild;
	xui_vec2_t tMeasured;
	xui_vec2_t tContent;
	xui_rect_t tRect;
	xui_rect_t tContentRect;
	xui_widget_content_measure_proc onContentMeasure;
	xui_widget_layout_measure_proc onLayoutMeasure;
	xui_widget_layout_arrange_proc onLayoutArrange;
	void* pCallbackUser;
	int iRet;
	int iFailed;

	pContext = NULL;
	pRoot = NULL;
	pChild = NULL;
	iFailed = 0;

	tState.tContentSize.fX = 80.0f;
	tState.tContentSize.fY = 30.0f;
	tState.tLayoutSize.fX = 160.0f;
	tState.tLayoutSize.fY = 50.0f;
	tState.iContentMeasureCount = 0;
	tState.iLayoutMeasureCount = 0;
	tState.iLayoutArrangeCount = 0;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiSetViewportSize(pContext, 300.0f, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport failed");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pRoot != NULL), "root create failed");
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pChild != NULL), "child create failed");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root failed");
	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_ROW);
	XUI_TEST_CHECK(iRet == XUI_OK, "set row failed");
	iRet = xuiWidgetSetContentMeasureCallback(pChild, __xuiTestContentMeasure, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "set content measure failed");
	iRet = xuiWidgetSetSizeMode(pChild, XUI_SIZE_CONTENT, XUI_SIZE_CONTENT);
	XUI_TEST_CHECK(iRet == XUI_OK, "set child content size mode failed");
	iRet = xuiWidgetAddChild(pRoot, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "add child failed");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "row layout failed");
	tRect = xuiWidgetGetRect(pChild);
	XUI_TEST_CHECK(__xuiTestRectEquals(tRect, 0.0f, 0.0f, 80.0f, 30.0f), "content callback should drive content size");
	XUI_TEST_CHECK(tState.iContentMeasureCount > 0, "content measure was not called");

	iRet = xuiWidgetMeasureContent(pChild, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tContent);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tContent.fX == 80.0f) && (tContent.fY == 30.0f), "measure content failed");
	iRet = xuiWidgetSetPadding(pChild, __xuiTestThickness(5.0f, 6.0f, 7.0f, 8.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "child padding failed");
	iRet = xuiWidgetMeasure(pChild, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tMeasured.fX == 92.0f) && (tMeasured.fY == 44.0f), "content size should include padding");

	iRet = xuiWidgetGetContentMeasureCallback(pChild, &onContentMeasure, &pCallbackUser);
	XUI_TEST_CHECK((iRet == XUI_OK) && (onContentMeasure == __xuiTestContentMeasure) && (pCallbackUser == &tState), "get content measure failed");

	iRet = xuiWidgetSetPadding(pRoot, __xuiTestThickness(10.0f, 11.0f, 12.0f, 13.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "root padding failed");
	iRet = xuiWidgetSetLayoutCallbacks(pRoot, __xuiTestLayoutMeasure, __xuiTestLayoutArrange, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "set layout callbacks failed");
	iRet = xuiWidgetSetSizeMode(pRoot, XUI_SIZE_CONTENT, XUI_SIZE_CONTENT);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root content size mode failed");
	iRet = xuiWidgetGetLayoutCallbacks(pRoot, &onLayoutMeasure, &onLayoutArrange, &pCallbackUser);
	XUI_TEST_CHECK((iRet == XUI_OK) && (onLayoutMeasure == __xuiTestLayoutMeasure) && (onLayoutArrange == __xuiTestLayoutArrange) && (pCallbackUser == &tState), "get layout callbacks failed");

	iRet = xuiWidgetMeasure(pRoot, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tMeasured.fX == 182.0f) && (tMeasured.fY == 74.0f), "custom layout measure failed");
	XUI_TEST_CHECK(tState.iLayoutMeasureCount > 0, "layout measure was not called");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom arrange layout failed");
	XUI_TEST_CHECK(tState.iLayoutArrangeCount > 0, "layout arrange was not called");
	tContentRect = xuiWidgetGetContentRect(pRoot);
	XUI_TEST_CHECK(__xuiTestRectEquals(tContentRect, 10.0f, 11.0f, 278.0f, 96.0f), "root content rect failed");
	tRect = xuiWidgetGetRect(pChild);
	XUI_TEST_CHECK(__xuiTestRectEquals(tRect, 14.0f, 16.0f, 270.0f, 86.0f), "custom arrange child rect failed");

	tState.tContentSize.fX = 120.0f;
	tState.tContentSize.fY = 20.0f;
	iRet = xuiWidgetSetContentMeasureCallback(pChild, __xuiTestContentMeasure, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "same callback should be accepted");
	iRet = xuiWidgetInvalidate(pChild, XUI_WIDGET_DIRTY_LAYOUT);
	XUI_TEST_CHECK(iRet == XUI_OK, "manual layout invalidate failed");
	iRet = xuiWidgetMeasureContent(pChild, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tContent);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tContent.fX == 120.0f) && (tContent.fY == 20.0f), "content measure after state change failed");

	iRet = xuiWidgetSetLayoutCallbacks(pRoot, NULL, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear layout callbacks failed");
	iRet = xuiWidgetGetLayoutCallbacks(pRoot, &onLayoutMeasure, &onLayoutArrange, &pCallbackUser);
	XUI_TEST_CHECK((iRet == XUI_OK) && (onLayoutMeasure == NULL) && (onLayoutArrange == NULL) && (pCallbackUser == NULL), "cleared layout callbacks failed");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_layout_callback_test passed\n");
	return 0;
}
