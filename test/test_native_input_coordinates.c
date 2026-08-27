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
	uint32_t iModifiers;
	float fX;
	float fY;
	float fDX;
	float fDY;
} test_scene_input_t;

typedef struct test_resource_bridge_t {
	int iLoadCount;
	int iFreeCount;
} test_resource_bridge_t;

static int __testResourceBridgeLoad(const char* sURI, void** ppData, int* pSize, void* pUser)
{
	test_resource_bridge_t* pBridge = (test_resource_bridge_t*)pUser;
	(void)sURI;
	*ppData = xrtMalloc(1u);
	if ( *ppData == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	*(unsigned char*)*ppData = 0x5a;
	*pSize = 1;
	pBridge->iLoadCount++;
	return XGE_OK;
}

static void __testResourceBridgeFree(void* pData, void* pUser)
{
	test_resource_bridge_t* pBridge = (test_resource_bridge_t*)pUser;
	pBridge->iFreeCount++;
	xrtFree(pData);
}

static int __testNear(float fA, float fB)
{
	return fabsf(fA - fB) < 0.001f;
}

static int __testSceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	test_scene_input_t* pInput;

	pInput = (test_scene_input_t*)pScene->pUser;
	pInput->iType = pEvent->iType;
	pInput->iModifiers = (uint32_t)pEvent->iParam2;
	pInput->fX = pEvent->fX;
	pInput->fY = pEvent->fY;
	pInput->fDX = pEvent->fDX;
	pInput->fDY = pEvent->fDY;
	return XGE_OK;
}

static int __testGetPointerEvent(int iType, uint64_t iPointerId, int iButton,
	uint32_t iButtons, uint32_t iModifiers,
	float fX, float fY, float fDX, float fDY)
{
	xge_input_event_t tEvent;

	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1, "missing ordered input event");
	TEST_CHECK(tEvent.iType == iType, "ordered input type");
	TEST_CHECK(tEvent.iPointerId == iPointerId, "ordered input pointer id");
	TEST_CHECK(tEvent.iButton == iButton && tEvent.iButtons == iButtons,
		"ordered input button state");
	TEST_CHECK(tEvent.iModifiers == iModifiers, "ordered input modifiers");
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
	g_xge.iMouseButtons = 0u;
	g_xge.iTouchCount = 0;
	memset(g_xge.arrTouches, 0, sizeof(g_xge.arrTouches));

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_MOUSE_MOVE;
	tEvent.mouse_x = 125.0f;
	tEvent.mouse_y = 180.0f;
	tEvent.mouse_dx = 5.0f;
	tEvent.mouse_dy = -4.0f;
	tEvent.modifiers = SAPP_MODIFIER_SHIFT | SAPP_MODIFIER_ALT | SAPP_MODIFIER_LMB;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_MOUSE_MOVE, 0u, 0, 0u,
		XGE_KEY_MOD_SHIFT | XGE_KEY_MOD_ALT,
		125.0f, 180.0f, 5.0f, -4.0f), "mouse move event");
	xgeMouseGet(&fMouseX, &fMouseY);
	TEST_CHECK(__testNear(fMouseX, 125.0f) && __testNear(fMouseY, 180.0f), "mouse polling position");
	TEST_CHECK(pSceneInput->iType == XGE_EVENT_MOUSE_MOVE &&
		pSceneInput->iModifiers == (XGE_KEY_MOD_SHIFT | XGE_KEY_MOD_ALT) &&
		__testNear(pSceneInput->fX, 125.0f) && __testNear(pSceneInput->fY, 180.0f) &&
		__testNear(pSceneInput->fDX, 5.0f) && __testNear(pSceneInput->fDY, -4.0f),
		"scene mouse coordinates");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_MOUSE_SCROLL;
	tEvent.mouse_x = 140.0f;
	tEvent.mouse_y = 195.0f;
	tEvent.scroll_x = -1.0f;
	tEvent.scroll_y = 2.0f;
	tEvent.modifiers = SAPP_MODIFIER_CTRL | SAPP_MODIFIER_SUPER | SAPP_MODIFIER_RMB;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_MOUSE_WHEEL, 0u, 0, 0u,
		XGE_KEY_MOD_CTRL | XGE_KEY_MOD_SUPER,
		140.0f, 195.0f, -1.0f, 2.0f), "wheel event");
	xgeMouseGet(&fMouseX, &fMouseY);
	TEST_CHECK(__testNear(fMouseX, 140.0f) && __testNear(fMouseY, 195.0f), "wheel current position");
	TEST_CHECK(pSceneInput->iType == XGE_EVENT_MOUSE_WHEEL &&
		__testNear(pSceneInput->fX, 140.0f) && __testNear(pSceneInput->fY, 195.0f),
		"scene wheel coordinates");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_MOUSE_DOWN;
	tEvent.mouse_button = SAPP_MOUSEBUTTON_LEFT;
	tEvent.modifiers = SAPP_MODIFIER_SHIFT | SAPP_MODIFIER_CTRL | SAPP_MODIFIER_LMB;
	tEvent.mouse_x = 160.0f;
	tEvent.mouse_y = 210.0f;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_MOUSE_DOWN, 0u, XGE_MOUSE_LEFT,
		XGE_MOUSE_LEFT, XGE_KEY_MOD_SHIFT | XGE_KEY_MOD_CTRL,
		160.0f, 210.0f, 0.0f, 0.0f), "mouse down event");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_MOUSE_UP;
	tEvent.mouse_button = SAPP_MOUSEBUTTON_LEFT;
	tEvent.modifiers = SAPP_MODIFIER_ALT | SAPP_MODIFIER_SUPER;
	tEvent.mouse_x = 165.0f;
	tEvent.mouse_y = 215.0f;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_MOUSE_UP, 0u, XGE_MOUSE_LEFT,
		0u, XGE_KEY_MOD_ALT | XGE_KEY_MOD_SUPER,
		165.0f, 215.0f, 0.0f, 0.0f), "mouse up event");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_TOUCHES_BEGAN;
	tEvent.num_touches = 1;
	tEvent.touches[0].identifier = 7u;
	tEvent.touches[0].pos_x = 210.0f;
	tEvent.touches[0].pos_y = 240.0f;
	tEvent.touches[0].changed = true;
	tEvent.modifiers = SAPP_MODIFIER_CTRL;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_TOUCH_BEGIN, 7u, XGE_MOUSE_LEFT,
		XGE_MOUSE_LEFT, XGE_KEY_MOD_CTRL,
		210.0f, 240.0f, 0.0f, 0.0f), "touch begin event");
	TEST_CHECK(pSceneInput->iType == XGE_EVENT_TOUCH_BEGIN &&
		pSceneInput->iModifiers == XGE_KEY_MOD_CTRL &&
		__testNear(pSceneInput->fX, 210.0f) && __testNear(pSceneInput->fY, 240.0f),
		"scene touch coordinates");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_TOUCHES_MOVED;
	tEvent.num_touches = 1;
	tEvent.touches[0].identifier = 7u;
	tEvent.touches[0].pos_x = 220.0f;
	tEvent.touches[0].pos_y = 250.0f;
	tEvent.touches[0].changed = true;
	tEvent.modifiers = SAPP_MODIFIER_SHIFT | SAPP_MODIFIER_ALT | SAPP_MODIFIER_MMB;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_TOUCH_MOVE, 7u, XGE_MOUSE_LEFT,
		XGE_MOUSE_LEFT, XGE_KEY_MOD_SHIFT | XGE_KEY_MOD_ALT,
		220.0f, 250.0f, 10.0f, 10.0f), "touch move event");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_TOUCHES_ENDED;
	tEvent.num_touches = 1;
	tEvent.touches[0].identifier = 7u;
	tEvent.touches[0].pos_x = 225.0f;
	tEvent.touches[0].pos_y = 255.0f;
	tEvent.touches[0].changed = true;
	tEvent.modifiers = SAPP_MODIFIER_SUPER;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_TOUCH_END, 7u, XGE_MOUSE_LEFT,
		0u, XGE_KEY_MOD_SUPER,
		225.0f, 255.0f, 5.0f, 5.0f), "touch end event");
	g_xge.iTouchCount = 0;
	memset(g_xge.arrTouches, 0, sizeof(g_xge.arrTouches));

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_TOUCHES_BEGAN;
	tEvent.num_touches = 1;
	tEvent.touches[0].identifier = 8u;
	tEvent.touches[0].pos_x = 260.0f;
	tEvent.touches[0].pos_y = 270.0f;
	tEvent.touches[0].changed = true;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_TOUCH_BEGIN, 8u, XGE_MOUSE_LEFT,
		XGE_MOUSE_LEFT, 0u, 260.0f, 270.0f, 0.0f, 0.0f),
		"second touch begin event");

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.type = SAPP_EVENTTYPE_TOUCHES_CANCELLED;
	tEvent.num_touches = 1;
	tEvent.touches[0].identifier = 8u;
	tEvent.touches[0].pos_x = 262.0f;
	tEvent.touches[0].pos_y = 274.0f;
	tEvent.touches[0].changed = true;
	tEvent.modifiers = SAPP_MODIFIER_CTRL | SAPP_MODIFIER_ALT;
	__xgeSokolEvent(&tEvent);
	TEST_CHECK(__testGetPointerEvent(XGE_EVENT_TOUCH_CANCEL, 8u, XGE_MOUSE_LEFT,
		0u, XGE_KEY_MOD_CTRL | XGE_KEY_MOD_ALT,
		262.0f, 274.0f, 2.0f, 4.0f), "touch cancel event");

	TEST_CHECK(xgeInputEventPendingCount() == 0, "ordered input queue empty");
	return 1;
}

static int __testMiniProgramCoordinates(void)
{
	xge_input_event_t tEvent;
	xge_miniprogram_bridge_t tBridgeA;
	xge_miniprogram_bridge_t tBridgeB;
	xge_resource_t tResource;
	test_resource_bridge_t tStateA;
	test_resource_bridge_t tStateB;
	float fNan = nanf("");

	memset(&tBridgeA, 0, sizeof(tBridgeA));
	memset(&tBridgeB, 0, sizeof(tBridgeB));
	memset(&tStateA, 0, sizeof(tStateA));
	memset(&tStateB, 0, sizeof(tStateB));
	tBridgeA.load_resource = __testResourceBridgeLoad;
	tBridgeA.free_resource = __testResourceBridgeFree;
	tBridgeA.pUser = &tStateA;
	tBridgeB.load_resource = __testResourceBridgeLoad;
	tBridgeB.free_resource = __testResourceBridgeFree;
	tBridgeB.pUser = &tStateB;
	g_xge.iTouchCount = 0;
	memset(g_xge.arrTouches, 0, sizeof(g_xge.arrTouches));
	TEST_CHECK(xgeMiniProgramSetBridge(&tBridgeA) == XGE_OK, "set first miniprogram bridge");
	TEST_CHECK(xgeMiniProgramInitSimple(400, 300, 2.0f) == XGE_OK,
		"miniprogram init");
	TEST_CHECK(xgeGetWidth() == 800 && xgeGetHeight() == 600,
		"miniprogram framebuffer dimensions");
	TEST_CHECK(xgeMiniProgramTouchOne(XGE_TOUCH_BEGIN, 9, 25.0f, 30.0f, 1.0f) == XGE_OK,
		"miniprogram touch");
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1 &&
		tEvent.iType == XGE_EVENT_TOUCH_BEGIN &&
		tEvent.iModifiers == 0u &&
		__testNear(tEvent.fX, 50.0f) && __testNear(tEvent.fY, 60.0f),
		"miniprogram touch framebuffer coordinates");
	memset(&tResource, 0, sizeof(tResource));
	TEST_CHECK(xgeResourceLoad("res://bridge-a", &tResource) == XGE_OK, "load with first bridge");
	TEST_CHECK(xgeMiniProgramSetBridge(&tBridgeB) == XGE_OK, "replace miniprogram bridge");
	xgeResourceFree(&tResource);
	TEST_CHECK(tStateA.iFreeCount == 1 && tStateB.iFreeCount == 0,
		"resource free uses bridge active at load time");
	memset(&tResource, 0, sizeof(tResource));
	TEST_CHECK(xgeResourceLoad("res://bridge-b", &tResource) == XGE_OK, "load with replacement bridge");
	xgeResourceFree(&tResource);
	TEST_CHECK(tStateB.iLoadCount == 1 && tStateB.iFreeCount == 1,
		"replacement bridge owns new resources");
	TEST_CHECK(xgeMiniProgramResize(400, 300, fNan) == XGE_ERROR_INVALID_ARGUMENT,
		"reject non-finite device pixel ratio");
	TEST_CHECK(xgeMiniProgramFrame(10.0) == 1 && xgeMiniProgramFrame(10.5) == 1 &&
		__testNear(xgeGetDelta(), 0.5f), "miniprogram frame timestamp drives delta");
	xgeMiniProgramUnit();
	return 1;
}

static int __testNonFiniteRectHandling(void)
{
	xge_rect_i_t tPixels;
	xge_rect_t tRect = {nanf(""), 0.0f, 10.0f, 10.0f};

	tPixels = xgeRectToPixelsNearest(tRect);
	TEST_CHECK(tPixels.iX == 0 && tPixels.iY == 0 && tPixels.iW == 0 && tPixels.iH == 0,
		"non-finite rectangle conversion");
	xgeViewportSet(tRect);
	tRect = xgeViewportGet();
	TEST_CHECK(tRect.fX == 0.0f && tRect.fY == 0.0f && tRect.fW == 0.0f && tRect.fH == 0.0f,
		"non-finite viewport rejected");
	xgeClipSet((xge_rect_t){0.0f, nanf(""), 10.0f, 10.0f});
	tRect = xgeClipGet();
	TEST_CHECK(tRect.fX == 0.0f && tRect.fY == 0.0f && tRect.fW == 0.0f && tRect.fH == 0.0f,
		"non-finite clip rejected");
	xgeViewportClear();
	xgeClipClear();
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
	     !__testMiniProgramCoordinates() ||
	     !__testNonFiniteRectHandling() ) {
		iFailed = 1;
	}
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_native_input_coordinates passed at DPI 1.0 and 2.0\n");
	return 0;
}
