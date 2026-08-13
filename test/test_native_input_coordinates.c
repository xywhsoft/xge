#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../xge.c"

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("test_native_input_coordinates failed: %s\n", (message)); \
			return 0; \
		} \
	} while ( 0 )

typedef struct test_scene_input_t {
	int iType;
	float fX;
	float fY;
	float fDX;
	float fDY;
} test_scene_input_t;

static int __testNear(float fA, float fB)
{
	return fabsf(fA - fB) < 0.001f;
}

static int __testSceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	test_scene_input_t* pInput;

	pInput = (test_scene_input_t*)pScene->pUser;
	pInput->iType = pEvent->iType;
	pInput->fX = pEvent->fX;
	pInput->fY = pEvent->fY;
	pInput->fDX = pEvent->fDX;
	pInput->fDY = pEvent->fDY;
	return XGE_OK;
}

static int __testGetEvent(int iType, float fX, float fY, float fDX, float fDY)
{
	xge_input_event_t tEvent;

	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1, "missing ordered input event");
	TEST_CHECK(tEvent.iType == iType, "ordered input type");
	TEST_CHECK(__testNear(tEvent.fX, fX) && __testNear(tEvent.fY, fY), "ordered input position");
	TEST_CHECK(__testNear(tEvent.fDX, fDX) && __testNear(tEvent.fDY, fDY), "ordered input delta");
	return 1;
}

static int __testDpiCase(float fDpiScale, test_scene_input_t* pSceneInput)
{
	sapp_event tEvent;
	float fMouseX;
	float fMouseY;

	while ( xgeInputEventPendingCount() > 0 ) {
		xge_input_event_t tDiscard;
		(void)xgeInputEventGet(&tDiscard);
	}
	g_xge.fDpiScale = fDpiScale;
	g_xge.iWindowWidth = 400;
	g_xge.iWindowHeight = 300;
	g_xge.iFramebufferWidth = (int)(400.0f * fDpiScale);
	g_xge.iFramebufferHeight = (int)(300.0f * fDpiScale);
	g_xge.iWidth = g_xge.iFramebufferWidth;
	g_xge.iHeight = g_xge.iFramebufferHeight;
	g_xge.iTouchCount = 0;
	memset(g_xge.arrTouches, 0, sizeof(g_xge.arrTouches));

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_MOUSE_MOVE;
	tEvent.mouse_x = 125.0f;
	tEvent.mouse_y = 180.0f;
	tEvent.mouse_dx = 5.0f;
	tEvent.mouse_dy = -4.0f;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetEvent(XGE_EVENT_MOUSE_MOVE, 125.0f, 180.0f, 5.0f, -4.0f), "mouse move event");
	xgeMouseGet(&fMouseX, &fMouseY);
	TEST_CHECK(__testNear(fMouseX, 125.0f) && __testNear(fMouseY, 180.0f), "mouse polling position");
	TEST_CHECK(pSceneInput->iType == XGE_EVENT_MOUSE_MOVE &&
		__testNear(pSceneInput->fX, 125.0f) && __testNear(pSceneInput->fY, 180.0f) &&
		__testNear(pSceneInput->fDX, 5.0f) && __testNear(pSceneInput->fDY, -4.0f),
		"scene mouse coordinates");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_MOUSE_SCROLL;
	tEvent.mouse_x = 140.0f;
	tEvent.mouse_y = 195.0f;
	tEvent.scroll_x = -1.0f;
	tEvent.scroll_y = 2.0f;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetEvent(XGE_EVENT_MOUSE_WHEEL, 140.0f, 195.0f, -1.0f, 2.0f), "wheel event");
	xgeMouseGet(&fMouseX, &fMouseY);
	TEST_CHECK(__testNear(fMouseX, 140.0f) && __testNear(fMouseY, 195.0f), "wheel current position");
	TEST_CHECK(pSceneInput->iType == XGE_EVENT_MOUSE_WHEEL &&
		__testNear(pSceneInput->fX, 140.0f) && __testNear(pSceneInput->fY, 195.0f),
		"scene wheel coordinates");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_MOUSE_DOWN;
	tEvent.mouse_button = SAPP_MOUSEBUTTON_LEFT;
	tEvent.mouse_x = 160.0f;
	tEvent.mouse_y = 210.0f;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetEvent(XGE_EVENT_MOUSE_DOWN, 160.0f, 210.0f, 0.0f, 0.0f), "mouse down event");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_TOUCHES_BEGAN;
	tEvent.num_touches = 1;
	tEvent.touches[0].identifier = 7u;
	tEvent.touches[0].pos_x = 210.0f;
	tEvent.touches[0].pos_y = 240.0f;
	tEvent.touches[0].changed = true;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetEvent(XGE_EVENT_TOUCH_BEGIN, 210.0f, 240.0f, 210.0f, 240.0f), "touch begin event");
	TEST_CHECK(pSceneInput->iType == XGE_EVENT_TOUCH_BEGIN &&
		__testNear(pSceneInput->fX, 210.0f) && __testNear(pSceneInput->fY, 240.0f),
		"scene touch coordinates");

	TEST_CHECK(xgeInputEventPendingCount() == 0, "ordered input queue empty");
	return 1;
}

static int __testMiniProgramCoordinates(void)
{
	xge_input_event_t tEvent;

	TEST_CHECK(xgeMiniProgramInitSimple(400, 300, 2.0f) == XGE_OK,
		"miniprogram init");
	TEST_CHECK(xgeGetWidth() == 800 && xgeGetHeight() == 600,
		"miniprogram framebuffer dimensions");
	TEST_CHECK(xgeMiniProgramTouchOne(XGE_TOUCH_BEGIN, 9, 25.0f, 30.0f, 1.0f) == XGE_OK,
		"miniprogram touch");
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1 &&
		tEvent.iType == XGE_EVENT_TOUCH_BEGIN &&
		__testNear(tEvent.fX, 50.0f) && __testNear(tEvent.fY, 60.0f),
		"miniprogram touch framebuffer coordinates");
	xgeMiniProgramUnit();
	return 1;
}

int main(void)
{
	xge_desc_t tDesc;
	xge_scene_t tScene;
	test_scene_input_t tSceneInput;
	int iFailed;

	iFailed = 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		printf("test_native_input_coordinates failed: xgeInit\n");
		return 1;
	}
	memset(&tScene, 0, sizeof(tScene));
	memset(&tSceneInput, 0, sizeof(tSceneInput));
	tScene.pUser = &tSceneInput;
	tScene.onEvent = __testSceneEvent;
	if ( xgeSceneSet(&tScene) != XGE_OK ||
	     !__testDpiCase(1.0f, &tSceneInput) ||
	     !__testDpiCase(2.0f, &tSceneInput) ||
	     !__testMiniProgramCoordinates() ) {
		iFailed = 1;
	}
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_native_input_coordinates passed at DPI 1.0 and 2.0\n");
	return 0;
}
