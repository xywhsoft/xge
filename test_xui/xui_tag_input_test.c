#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_tag_input_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static void __xuiTagInputChanged(xui_widget pWidget, int iTagCount, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iTagCount;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTagInput;
	xui_widget pInput;
	xui_surface pTarget;
	xui_font pFont;
	xui_tag_input_desc_t tDesc;
	const char* arrTags[2];
	xui_rect_t tRect;
	xui_rect_t tClose;
	xui_rect_i_t tFullRect;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pTagInput = NULL;
	pInput = NULL;
	pTarget = NULL;
	pFont = NULL;
	iChanged = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 360.0f, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "input viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "font", 4, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 120.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	arrTags[0] = "Tag1";
	arrTags[1] = "Tag2";
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.ppTags = arrTags;
	tDesc.iTagCount = 2;
	tDesc.sPlaceholder = "Please input";
	tDesc.pFont = pFont;
	tDesc.iMaxTags = 8;
	iRet = xuiTagInputCreate(pContext, &pTagInput, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTagInput != NULL, "taginput create");
	XUI_TEST_CHECK(xuiTagInputGetTagCount(pTagInput) == 2, "initial count");
	XUI_TEST_CHECK(strcmp(xuiTagInputGetTag(pTagInput, 0), "Tag1") == 0, "initial tag");
	XUI_TEST_CHECK(strcmp(xuiTagInputGetPlaceholder(pTagInput), "Please input") == 0, "placeholder");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateCount(pTagInput) == 4, "cache states");
	iRet = xuiWidgetSetRect(pTagInput, (xui_rect_t){10.0f, 10.0f, 320.0f, 38.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "taginput rect");
	iRet = xuiTagInputSetChange(pTagInput, __xuiTagInputChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	iRet = xuiWidgetAddChild(pRoot, pTagInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "add taginput");
	pInput = xuiTagInputGetInputWidget(pTagInput);
	XUI_TEST_CHECK(pInput != NULL, "input child");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 360, 120, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tRect = xuiTagInputGetInputRect(pTagInput);
	XUI_TEST_CHECK(tRect.fW >= 72.0f && tRect.fH > 0.0f, "input rect");
	tClose = xuiTagInputGetCloseRect(pTagInput, 0);
	XUI_TEST_CHECK(tClose.fW > 0.0f && tClose.fH > 0.0f, "close rect");
	tFullRect = (xui_rect_i_t){0, 0, 360, 120};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pTagInput, 0) != NULL, "normal cache");

	iRet = xuiTagInputSetText(pTagInput, "Alpha");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTagInputGetText(pTagInput), "Alpha") == 0, "set text");
	iRet = xuiTagInputCommit(pTagInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "commit text");
	XUI_TEST_CHECK(xuiTagInputGetTagCount(pTagInput) == 3 && strcmp(xuiTagInputGetTag(pTagInput, 2), "Alpha") == 0, "commit adds tag");
	XUI_TEST_CHECK(strcmp(xuiTagInputGetText(pTagInput), "") == 0 && iChanged == 1, "commit clears text");

	iRet = xuiSetFocusWidget(pContext, pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus input");
	iRet = xuiInputText(pContext, 'N');
	XUI_TEST_CHECK(iRet == XUI_OK, "text N");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch N");
	iRet = xuiInputText(pContext, 'e');
	XUI_TEST_CHECK(iRet == XUI_OK, "text e");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch e");
	iRet = xuiInputText(pContext, 'w');
	XUI_TEST_CHECK(iRet == XUI_OK, "text w");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch w");
	iRet = xuiInputText(pContext, ',');
	XUI_TEST_CHECK(iRet == XUI_OK, "text comma");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch comma");
	XUI_TEST_CHECK(xuiTagInputGetTagCount(pTagInput) == 4 && strcmp(xuiTagInputGetTag(pTagInput, 3), "New") == 0, "comma commits");
	XUI_TEST_CHECK(strcmp(xuiTagInputGetText(pTagInput), "") == 0 && iChanged == 2, "comma clears");

	iRet = xuiInputKeyDown(pContext, 8, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "backspace");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch backspace");
	XUI_TEST_CHECK(xuiTagInputGetTagCount(pTagInput) == 3 && iChanged == 3, "backspace removes last");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout before close");
	tClose = xuiTagInputGetCloseRect(pTagInput, 0);
	iRet = xuiInputPointerMove(pContext, 10.0f + tClose.fX + 2.0f, 10.0f + tClose.fY + 2.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "move close");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch move close");
	iRet = xuiInputPointerDown(pContext, 10.0f + tClose.fX + 2.0f, 10.0f + tClose.fY + 2.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "down close");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch down close");
	iRet = xuiInputPointerUp(pContext, 10.0f + tClose.fX + 2.0f, 10.0f + tClose.fY + 2.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "up close");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch up close");
	XUI_TEST_CHECK(xuiTagInputGetTagCount(pTagInput) == 2 && strcmp(xuiTagInputGetTag(pTagInput, 0), "Tag2") == 0 && iChanged == 4, "close removes first");

	iRet = xuiWidgetSetEnabled(pTagInput, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled render prepare");
	XUI_TEST_CHECK((xuiTagInputGetState(pTagInput) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_tag_input_test passed\n");
	return 0;
}
