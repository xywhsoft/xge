#include "xui_internal.h"

#include <stdio.h>
#include <string.h>

static int xuiTestEditWidgetValid(xui_widget pWidget)
{
	return pWidget != NULL;
}

static void xuiTestEditAccessibilityQueue(xui_widget pWidget, int iEventType)
{
	(void)pWidget;
	(void)iEventType;
}

#define xuiInternalWidgetIsValid xuiTestEditWidgetValid
#define xuiInternalAccessibilityQueue xuiTestEditAccessibilityQueue
#include "../../src/xui/core/xui_edit.c"
#undef xuiInternalAccessibilityQueue
#undef xuiInternalWidgetIsValid

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_edit_test failed: %s\n", (msg)); \
			return 1; \
		} \
	} while ( 0 )

typedef struct xui_edit_test_event_t {
	int iCount;
	xui_edit_event_t tEvent;
} xui_edit_test_event_t;

static int xuiTestEditEvent(xui_widget pWidget,
	const xui_edit_event_t* pEvent, void* pUser)
{
	xui_edit_test_event_t* pState = (xui_edit_test_event_t*)pUser;

	(void)pWidget;
	if ( pState == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pState->iCount++;
	pState->tEvent = *pEvent;
	return XUI_OK;
}

static void xuiTestEditTargetInit(xui_widget_t* pWidget,
	const xui_internal_edit_adapter_t* pAdapter, int iTabBehavior)
{
	memset(pWidget, 0, sizeof(*pWidget));
	pWidget->pEditAdapter = pAdapter;
	pWidget->tEditBehavior.iSize = sizeof(pWidget->tEditBehavior);
	pWidget->tEditBehavior.iTabBehavior = iTabBehavior;
}

int main(void)
{
	xui_internal_edit_adapter_t tAdapter;
	xui_widget_t tHostA;
	xui_widget_t tHostB;
	xui_widget_t tTargetA;
	xui_widget_t tTargetB;
	xui_widget_t tTargetC;
	xui_edit_test_event_t tEvent;
	int iRet;

	memset(&tAdapter, 0, sizeof(tAdapter));
	tAdapter.iCapabilities = XUI_EDIT_CAP_TEXT;
	memset(&tHostA, 0, sizeof(tHostA));
	memset(&tHostB, 0, sizeof(tHostB));
	xuiTestEditTargetInit(&tTargetA, &tAdapter, XUI_EDIT_TAB_INSERT);
	xuiTestEditTargetInit(&tTargetB, &tAdapter, XUI_EDIT_TAB_FOCUS);
	xuiTestEditTargetInit(&tTargetC, &tAdapter, XUI_EDIT_TAB_INDENT);

	iRet = xuiInternalEditDelegate(&tHostA, &tTargetA);
	XUI_TEST_CHECK(iRet == XUI_OK && tHostA.pEditDelegate == &tTargetA &&
		tTargetA.pEditOwner == &tHostA,
		"first delegate establishes both links");
	iRet = xuiInternalEditDelegate(&tHostB, &tTargetB);
	XUI_TEST_CHECK(iRet == XUI_OK && tHostB.pEditDelegate == &tTargetB &&
		tTargetB.pEditOwner == &tHostB,
		"second delegate establishes both links");

	iRet = xuiInternalEditDelegate(&tHostA, &tTargetB);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED,
		"occupied delegate rejected");
	XUI_TEST_CHECK(tHostA.pEditDelegate == &tTargetA &&
		tTargetA.pEditOwner == &tHostA &&
		tHostB.pEditDelegate == &tTargetB && tTargetB.pEditOwner == &tHostB,
		"failed rebind preserves both existing relationships");
	XUI_TEST_CHECK(xuiEditGetCapabilities(&tHostA) == XUI_EDIT_CAP_TEXT &&
		xuiInternalEditHost(&tTargetA) == &tHostA,
		"preserved delegate remains operational");

	memset(&tEvent, 0, sizeof(tEvent));
	tHostA.onEditEvent = xuiTestEditEvent;
	tHostA.pEditEventUser = &tEvent;
	iRet = xuiInternalEditEmitSized(&tTargetA, XUI_EDIT_EVENT_TEXT_CHANGED,
		NULL, 7, 0, 0, 0, 0, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvent.iCount == 1 &&
		tEvent.tEvent.sText == NULL && tEvent.tEvent.iTextSize == 0,
		"null edit text cannot expose a positive byte count");
	iRet = xuiInternalEditEmitSized(&tTargetA, XUI_EDIT_EVENT_TEXT_CHANGED,
		"abc", 2, 0, 0, 0, 0, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvent.iCount == 2 &&
		tEvent.tEvent.iTextSize == 2,
		"sized edit text preserves its explicit byte count");

	iRet = xuiInternalEditDelegate(&tHostA, &tTargetC);
	XUI_TEST_CHECK(iRet == XUI_OK && tTargetA.pEditOwner == NULL &&
		tHostA.pEditDelegate == &tTargetC && tTargetC.pEditOwner == &tHostA &&
		tHostA.tEditBehavior.iTabBehavior == XUI_EDIT_TAB_INDENT,
		"successful rebind updates links and behavior");
	xuiInternalEditUndelegate(&tHostA);
	XUI_TEST_CHECK(tHostA.pEditDelegate == NULL && tTargetC.pEditOwner == NULL,
		"undelegate clears both links");

	printf("xui_edit_test passed\n");
	return 0;
}
