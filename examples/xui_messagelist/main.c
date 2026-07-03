#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 540
#define DEMO_TARGET_H 420
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_messagelist_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pDialog;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int bCreateOK;
	int bLayoutOK;
	int bRenderOK;
	int iClickCount;
} xui_messagelist_demo_t;

static void __xuiMessageListUsage(void)
{
	printf("usage: xui_messagelist [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiMessageListParseArgs(xui_messagelist_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiMessageListUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiMessageListFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static uint32_t __xuiMessageListReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiMessageListSendButtonTransitions(xui_messagelist_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	iRet = XUI_OK;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
	}
	if ( iRet == XUI_OK && (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
	}
	if ( iRet == XUI_OK && (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
	}
	if ( iRet == XUI_OK && (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
	}
	return iRet;
}

static int __xuiMessageListHandleInput(xui_messagelist_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	float fUiX;
	float fUiY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	fUiX = fX - DEMO_OFFSET_X;
	fUiY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiMessageListReadButtons();
	if ( !pDemo->bHasMouse || pDemo->fLastMouseX != fX || pDemo->fLastMouseY != fY || pDemo->iLastButtons != iButtons ) {
		iRet = xuiInputPointerMove(pDemo->pContext, fUiX, fUiY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, fUiX, fUiY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiMessageListSendButtonTransitions(pDemo, fUiX, fUiY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiMessageListEvent(xui_widget pWidget, const xui_message_list_event_t* pEvent, void* pUser)
{
	xui_messagelist_demo_t* pDemo;
	(void)pWidget;
	pDemo = (xui_messagelist_demo_t*)pUser;
	if ( pDemo == NULL || pEvent == NULL ) return;
	if ( pEvent->iEvent == XUI_MESSAGE_EVENT_CLICK ) {
		pDemo->iClickCount++;
		if ( pEvent->pNode != NULL ) {
			printf("xui_messagelist click index=%d sender=%s text=%s\n", pEvent->iIndex, pEvent->pNode->sSender, pEvent->pNode->sText);
		}
	}
}

static int __xuiMessageListCreateUi(xui_messagelist_demo_t* pDemo)
{
	xui_message_node_t arrNodes[8];
	xui_message_list_desc_t tDesc;
	int iRet;

	memset(arrNodes, 0, sizeof(arrNodes));
	arrNodes[0] = (xui_message_node_t){sizeof(xui_message_node_t), "m1", "Leaf Elbold Dadunur", "10:53", "Mobile UI must keep finger-sized hit targets.", XUI_MESSAGE_NODE_SELF, 0, NULL};
	arrNodes[1] = (xui_message_node_t){sizeof(xui_message_node_t), "m2", "Anubis", "10:54", "It also needs to respect narrow screen limits.", XUI_MESSAGE_NODE_OTHER, 0, NULL};
	arrNodes[2] = (xui_message_node_t){sizeof(xui_message_node_t), "s1", "", "Yesterday 10:56", "Yesterday 10:56", XUI_MESSAGE_NODE_SYSTEM, 0, NULL};
	arrNodes[3] = (xui_message_node_t){sizeof(xui_message_node_t), "m3", "Leaf Elbold Dadunur", "10:57", "Cross-platform behavior cannot skip this detail.", XUI_MESSAGE_NODE_SELF, 0, NULL};
	arrNodes[4] = (xui_message_node_t){sizeof(xui_message_node_t), "m4", "Anubis", "10:58", "@serci+loser Maybe use the desktop editor first.", XUI_MESSAGE_NODE_OTHER, 0, NULL};
	arrNodes[5] = (xui_message_node_t){sizeof(xui_message_node_t), "s2", "", "Today 09:15", "Today 09:15", XUI_MESSAGE_NODE_SYSTEM, 0, NULL};
	arrNodes[6] = (xui_message_node_t){sizeof(xui_message_node_t), "m5", "Leaf Elbold Dadunur", "09:16", "The first version should cover history display, events, loading and saving.", XUI_MESSAGE_NODE_SELF, 0, NULL};
	arrNodes[7] = (xui_message_node_t){sizeof(xui_message_node_t), "m6", "Anubis", "09:17", "Then we can wire it to real account, avatar and message storage.", XUI_MESSAGE_NODE_OTHER, 0, NULL};

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrNodes = arrNodes;
	tDesc.iNodeCount = 8;
	tDesc.bAutoScroll = 0;
	iRet = xuiMessageListCreate(pDemo->pContext, &pDemo->pDialog, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pDialog, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	xuiMessageListSetEvent(pDemo->pDialog, __xuiMessageListEvent, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pDialog);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bCreateOK = 1;
	return XUI_OK;
}

static int __xuiMessageListCreateAssets(xui_messagelist_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiMessageListFindTtf();
	if ( sFontPath == NULL ) {
		printf("xui_messagelist: no usable system TTF font found\n");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiMessageListCreateUi(pDemo);
}

static void __xuiMessageListDestroyAssets(xui_messagelist_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) xuiDestroy(pDemo->pContext);
	if ( pDemo->pFont != NULL ) pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	if ( pDemo->pTarget != NULL ) pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
}

static int __xuiMessageListFrame(void* pUser)
{
	xui_messagelist_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_messagelist_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiMessageListHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bLayoutOK = xuiMessageListGetNodeCount(pDemo->pDialog) == 8 && xuiMessageListGetBubbleRect(pDemo->pDialog, 0).fW > 0.0f;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(242, 243, 245, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bRenderOK = 1;
	xgeClear(XUI_COLOR_RGBA(32, 36, 42, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		printf("xui_messagelist final-summary frames=%d create=%d layout=%d render=%d nodes=%d clicks=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bRenderOK,
			xuiMessageListGetNodeCount(pDemo->pDialog),
			pDemo->iClickCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_messagelist_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiMessageListParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiMessageListUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI MessageList";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_messagelist: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiMessageListCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_messagelist: create assets failed: %d\n", iRet);
		__xuiMessageListDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiMessageListFrame, &tDemo);
	__xuiMessageListDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bRenderOK) ? 0 : 1;
}
