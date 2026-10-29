#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

static void __xuiTagInputDestroyOnChange(xui_widget pWidget, int iTagCount, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)iTagCount;
	if ( pCount != NULL ) (*pCount)++;
	(void)xuiWidgetDestroy(pWidget);
}

typedef struct xui_tag_input_edit_t {
	xui_widget pLastWidget;
	int iCommitCount;
	int bLastValid;
} xui_tag_input_edit_t;

static int __xuiTagInputEditEvent(xui_widget pWidget, const xui_edit_event_t* pEvent, void* pUser)
{
	xui_tag_input_edit_t* pEdit = (xui_tag_input_edit_t*)pUser;

	if ( pEdit == NULL || pEvent == NULL ) return XUI_OK;
	pEdit->pLastWidget = pWidget;
	pEdit->bLastValid = pEvent->bValid;
	if ( pEvent->iType == XUI_EDIT_EVENT_COMMIT ) pEdit->iCommitCount++;
	return XUI_OK;
}

static int __xuiTagInputSetTagsOomPreserves(xui_widget pWidget)
{
	xallocationbudget* pBudget = NULL;
	xallocationscope tScope;
	const char* arrReplacement[2] = {"Replacement", "Other"};
	int iRet;

	if ( pWidget == NULL ) return 0;
	memset(&tScope, 0, sizeof(tScope));
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) ) return 0;
	if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
		xrtAllocationBudgetFree(pBudget);
		return 0;
	}
	iRet = xuiTagInputSetTags(pWidget, arrReplacement, 2);
	(void)xrtAllocationBudgetLeave(&tScope);
	xrtAllocationBudgetFree(pBudget);
	return iRet == XUI_ERROR_OUT_OF_MEMORY && xuiTagInputGetTagCount(pWidget) == 1 &&
		strcmp(xuiTagInputGetTag(pWidget, 0), "Original") == 0;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_tag_input_edit_t tEdit;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTagInput;
	xui_widget pInput;
	xui_widget pProbe;
	xui_widget pProbeInput;
	xui_surface pTarget;
	xui_font pFont;
	xui_tag_input_desc_t tDesc;
	xui_accessible_node_t tNode;
	const char* arrTags[2];
	const char* arrLongTags[1];
	const char* arrMixedTags[4];
	xui_rect_t tRect;
	xui_rect_t tClose;
	xui_rect_i_t tFullRect;
	float fBorderWidth;
	float fTagHeight;
	float fReadBorderWidth;
	float fReadTagHeight;
	int iChanged;
	int iDestroyCount;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pTagInput = NULL;
	pInput = NULL;
	pProbe = NULL;
	pProbeInput = NULL;
	pTarget = NULL;
	pFont = NULL;
	iChanged = 0;
	iDestroyCount = 0;
	iFailed = 0;
	memset(&tEdit, 0, sizeof(tEdit));
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

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fTagHeight = NAN;
	pProbe = pRoot;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
		"invalid metrics clear create output");
	pProbe = pRoot;
	iRet = xuiTagInputCreate(NULL, &pProbe, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
		"invalid context clears create output");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iTagCount = 1;
	pProbe = pRoot;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
		"tag array required for descriptor count");
	tDesc.ppTags = arrTags;
	tDesc.iTagCount = 2;
	tDesc.iMaxTags = 1;
	pProbe = pRoot;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
		"descriptor count respects max tags");
	tDesc.iTagCount = 0;
	tDesc.iMaxTags = XUI_TAG_INPUT_TAG_CAPACITY + 1;
	pProbe = pRoot;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
		"descriptor rejects unsupported max tags");
	arrLongTags[0] = "abcdef";
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.ppTags = arrLongTags;
	tDesc.iTagCount = 1;
	tDesc.iMaxLength = 3;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL &&
		strcmp(xuiTagInputGetTag(pProbe, 0), "abc") == 0,
		"descriptor tags respect max length");
	arrLongTags[0] = "\x41\xE4\xB8\xAD\x42";
	iRet = xuiTagInputSetTags(pProbe, arrLongTags, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTagInputGetTag(pProbe, 0), "A") == 0,
		"tag truncation preserves utf8 boundary");
	xuiWidgetDestroy(pProbe);
	pProbe = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	arrLongTags[0] = "Original";
	tDesc.ppTags = arrLongTags;
	tDesc.iTagCount = 1;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "set tags oom create");
	XUI_TEST_CHECK(__xuiTagInputSetTagsOomPreserves(pProbe),
		"set tags oom preserves existing tags");
	xuiWidgetDestroy(pProbe);
	pProbe = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMaxTags = 1;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "limited paste create");
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "limited paste attach");
	pProbeInput = xuiTagInputGetInputWidget(pProbe);
	XUI_TEST_CHECK(pProbeInput != NULL, "limited paste input");
	iRet = xuiTestProxySetClipboardText(&tState, "One,Two,");
	if ( iRet == XUI_OK ) iRet = xuiInputPaste(pProbeInput);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTagInputGetTagCount(pProbe) == 1 &&
		strcmp(xuiTagInputGetTag(pProbe, 0), "One") == 0 &&
		strcmp(xuiTagInputGetText(pProbe), "Two") == 0,
		"limited paste removes committed prefix and preserves pending tag");
	iRet = xuiTagInputRemoveTag(pProbe, 0);
	if ( iRet == XUI_OK ) iRet = xuiTagInputCommit(pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTagInputGetTagCount(pProbe) == 1 &&
		strcmp(xuiTagInputGetTag(pProbe, 0), "Two") == 0 &&
		strcmp(xuiTagInputGetText(pProbe), "") == 0,
		"pending pasted tag remains committable after capacity frees");
	xuiWidgetDestroy(pProbe);
	pProbe = NULL;

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
	iRet = xuiTagInputGetVisualMetrics(pTagInput, &fBorderWidth, &fTagHeight);
	XUI_TEST_CHECK(iRet == XUI_OK, "read visual metrics");
	iRet = xuiTagInputSetVisualMetrics(pTagInput, NAN, fTagHeight);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan border width");
	iRet = xuiTagInputSetVisualMetrics(pTagInput, fBorderWidth, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite tag height");
	iRet = xuiTagInputGetVisualMetrics(pTagInput, &fReadBorderWidth, &fReadTagHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && fReadBorderWidth == fBorderWidth && fReadTagHeight == fTagHeight,
		"invalid metrics preserve state");
	iRet = xuiTagInputSetVisualMetrics(pTagInput, fBorderWidth, 23.5f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tRect = xuiTagInputGetTagRect(pTagInput, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tRect.fH == 24,
		"fractional tag height preserves rounded pixel bounds");
	iRet = xuiTagInputSetVisualMetrics(pTagInput, fBorderWidth, fTagHeight);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore visual metrics");
	iRet = xuiTagInputSetTags(pTagInput, NULL, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTagInputGetTagCount(pTagInput) == 2,
		"invalid tag array preserves state");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 360, 120, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tRect = xuiTagInputGetInputRect(pTagInput);
	XUI_TEST_CHECK(tRect.fW >= 72.0f && tRect.fH > 0.0f, "input rect");
	tClose = xuiTagInputGetCloseRect(pTagInput, 0);
	XUI_TEST_CHECK(tClose.fW > 0.0f && tClose.fH > 0.0f, "close rect");
	tFullRect = (xui_rect_i_t){0, 0, 360, 120};
	xuiTestProxyResetTextCounters(&tState);
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == 0,
		"render reuses tag geometry measured during layout");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pTagInput, 0) != NULL, "normal cache");
	XUI_TEST_CHECK(tState.iLastTextDrawSize >= 0, "tag text draws use bounded byte lengths");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pTagInput) == 3, "tag accessibility node count");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiWidgetGetAccessibleNode(pTagInput, 1, &tNode);
	XUI_TEST_CHECK(iRet == XUI_OK && tNode.iRole == XUI_ACCESSIBLE_ROLE_BUTTON &&
		strcmp(tNode.sName, "Tag1") == 0 &&
		(tNode.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_CLOSE)) != 0,
		"tag accessibility exposes close action");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable ancestor");
	XUI_TEST_CHECK((xuiTagInputGetState(pTagInput) & XUI_WIDGET_STATE_DISABLED) != 0,
		"ancestor disabled state is effective");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable ancestor");

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

	iRet = xuiInputKeyDown(pContext, XUI_KEY_BACKSPACE, 0);
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

	arrMixedTags[0] = "  Alpha  ";
	arrMixedTags[1] = "   ";
	arrMixedTags[2] = "Beta";
	arrMixedTags[3] = ";";
	iRet = xuiTagInputSetTags(pTagInput, arrMixedTags, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTagInputGetTagCount(pTagInput) == 2 &&
		strcmp(xuiTagInputGetTag(pTagInput, 0), "Alpha") == 0 &&
		strcmp(xuiTagInputGetTag(pTagInput, 1), "Beta") == 0 && iChanged == 5,
		"set tags trims and skips empty tags");
	arrTags[0] = "Alpha";
	arrTags[1] = "Beta";
	iRet = xuiTagInputSetTags(pTagInput, arrTags, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && iChanged == 5, "setting identical tags is silent");
	iRet = xuiTagInputSetMaxLength(pTagInput, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTagInputGetTag(pTagInput, 0), "Alp") == 0 &&
		strcmp(xuiTagInputGetTag(pTagInput, 1), "Bet") == 0 && iChanged == 6,
		"lowering max length constrains existing tags once");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout before capture release");
	tClose = xuiTagInputGetCloseRect(pTagInput, 0);
	iRet = xuiInputPointerDown(pContext, 10.0f + tClose.fX + 2.0f,
		10.0f + tClose.fY + 2.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "capture release pointer down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pTagInput,
		"tag close captures pointer");

	iRet = xuiWidgetSetEnabled(pTagInput, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"disable releases tag close capture");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled render prepare");
	XUI_TEST_CHECK((xuiTagInputGetState(pTagInput) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	arrLongTags[0] = "Touch";
	tDesc.ppTags = arrLongTags;
	tDesc.iTagCount = 1;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "multi-touch close create");
	iRet = xuiWidgetSetRect(pProbe, (xui_rect_t){10.0f, 64.0f, 220.0f, 38.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pProbe);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi-touch close layout");
	tRect = xuiWidgetGetWorldRect(pProbe);
	tClose = xuiTagInputGetCloseRect(pProbe, 0);
	iRet = xuiInputPointerDownEx(pContext, 401u, XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tClose.fX + 2.0f),
		(int)(tRect.fY + tClose.fY + 2.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 401u, XUI_POINTER_TYPE_TOUCH) == pProbe,
		"first touch owns tag close");
	iRet = xuiInputPointerDownEx(pContext, 402u, XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tClose.fX + 2.0f),
		(int)(tRect.fY + tClose.fY + 2.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 402u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tClose.fX + 2.0f),
		(int)(tRect.fY + tClose.fY + 2.0f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTagInputGetTagCount(pProbe) == 1 &&
		xuiGetPointerCaptureEx(pContext, 401u, XUI_POINTER_TYPE_TOUCH) == pProbe &&
		xuiGetPointerCaptureEx(pContext, 402u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot remove tag or release close capture");
	iRet = xuiInputPointerUpEx(pContext, 401u, XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tClose.fX + 2.0f),
		(int)(tRect.fY + tClose.fY + 2.0f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTagInputGetTagCount(pProbe) == 0 &&
		xuiGetPointerCaptureEx(pContext, 401u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch removes tag and releases close capture");
	xuiWidgetDestroy(pProbe);
	pProbe = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "destroy-on-enter create");
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-enter attach");
	pProbeInput = xuiTagInputGetInputWidget(pProbe);
	XUI_TEST_CHECK(pProbeInput != NULL, "destroy-on-enter input");
	iDestroyCount = 0;
	iRet = xuiTagInputSetChange(pProbe, __xuiTagInputDestroyOnChange, &iDestroyCount);
	if ( iRet == XUI_OK ) iRet = xuiTagInputSetText(pProbe, "EnterDestroy");
	if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pProbeInput);
	if ( iRet == XUI_OK ) iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyCount == 1 &&
		xuiTagInputGetInputWidget(pProbe) == NULL,
		"enter change callback may destroy tag input");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "edit commit create");
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "edit commit attach");
	pProbeInput = xuiTagInputGetInputWidget(pProbe);
	XUI_TEST_CHECK(pProbeInput != NULL, "edit commit input");
	memset(&tEdit, 0, sizeof(tEdit));
	iRet = xuiEditSetEvent(pProbe, __xuiTagInputEditEvent, &tEdit);
	if ( iRet == XUI_OK ) iRet = xuiTagInputSetText(pProbe, "EnterTag");
	if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pProbeInput);
	if ( iRet == XUI_OK ) iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTagInputGetTagCount(pProbe) == 1 &&
		strcmp(xuiTagInputGetTag(pProbe, 0), "EnterTag") == 0 &&
		strcmp(xuiTagInputGetText(pProbe), "") == 0 &&
		tEdit.iCommitCount == 1 && tEdit.pLastWidget == pProbe && tEdit.bLastValid,
		"enter commits through unified edit event");
	xuiWidgetDestroy(pProbe);
	pProbe = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	arrLongTags[0] = "Accessible";
	tDesc.ppTags = arrLongTags;
	tDesc.iTagCount = 1;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "accessible close create");
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "accessible close attach");
	iRet = xuiWidgetPerformAccessibleAction(pProbe, 2, XUI_ACCESSIBLE_ACTION_CLOSE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTagInputGetTagCount(pProbe) == 0 &&
		xuiWidgetGetAccessibleNodeCount(pProbe) == 1, "accessible close removes tag");
	xuiWidgetDestroy(pProbe);
	pProbe = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "destroy-on-add create");
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-add attach");
	iDestroyCount = 0;
	iRet = xuiTagInputSetChange(pProbe, __xuiTagInputDestroyOnChange, &iDestroyCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-add callback");
	iRet = xuiTagInputAddTag(pProbe, "destroy");
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyCount == 1 &&
		xuiTagInputGetInputWidget(pProbe) == NULL, "change callback may destroy tag input");

	pProbe = NULL;
	iRet = xuiTagInputCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "destroy-on-delimited create");
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-delimited attach");
	pProbeInput = xuiTagInputGetInputWidget(pProbe);
	XUI_TEST_CHECK(pProbeInput != NULL, "destroy-on-delimited input");
	iDestroyCount = 0;
	iRet = xuiTagInputSetChange(pProbe, __xuiTagInputDestroyOnChange, &iDestroyCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-delimited callback");
	iRet = xuiInputSetText(pProbeInput, "One");
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyCount == 0, "programmatic input text is silent");
	iRet = xuiSetFocusWidget(pContext, pProbeInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus destroy-on-delimited input");
	iRet = xuiInputText(pContext, ',');
	XUI_TEST_CHECK(iRet == XUI_OK, "queue destroy-on-delimited separator");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyCount == 1 &&
		xuiTagInputGetInputWidget(pProbe) == NULL, "delimited callback may destroy tag input");

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
