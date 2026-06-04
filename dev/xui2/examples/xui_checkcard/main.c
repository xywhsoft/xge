#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	800
#define DEMO_TARGET_H	430
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define CHECKCARD_COUNT	6
#define LEFT_COUNT	3
#define RIGHT_COUNT	3
#define LABEL_COUNT	14

typedef struct xui_checkcard_icon_t {
	xui_proxy pProxy;
	uint32_t iColor;
} xui_checkcard_icon_t;

typedef struct xui_checkcard_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pTitleFont;
	xui_font pBodyFont;
	xui_widget pRoot;
	xui_widget pRightGroup;
	xui_widget pHeading[LABEL_COUNT];
	xui_widget pCard[CHECKCARD_COUNT];
	xui_widget pIcon[CHECKCARD_COUNT];
	xui_widget pTitle[CHECKCARD_COUNT];
	xui_widget pDescription[CHECKCARD_COUNT];
	xui_checkcard_icon_t arrIcon[CHECKCARD_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iChangeCount;
	int iGroupIndex;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bMultiOK;
	int bSingleOK;
	int bContentOK;
	int bInputOK;
} xui_checkcard_demo_t;

static const char* g_arrTitle[CHECKCARD_COUNT] = {
	"Starter",
	"Workflow",
	"Insight",
	"Standard",
	"Advanced",
	"Enterprise"
};

static const char* g_arrDescription[CHECKCARD_COUNT] = {
	"Simple onboarding setup for a small team and daily reviews.",
	"Reusable process template with task stages and handoff notes.",
	"Metrics preset with reporting cards and weekly summary views.",
	"Balanced option for routine product and project management.",
	"Automation-ready plan with approvals, owners, and audit trail.",
	"Strict control profile for cross-team governance and delivery."
};

static uint32_t __xuiCheckCardColorWithAlpha(uint32_t iColor, uint32_t iAlpha)
{
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static void __xuiCheckCardUsage(void)
{
	printf("usage: xui_checkcard [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiCheckCardParseArgs(xui_checkcard_demo_t* pDemo, int argc, char** argv)
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
			__xuiCheckCardUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiCheckCardFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
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

static int __xuiCheckCardRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_checkcard_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_checkcard_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tRect, 8.0f, XUI_COLOR_RGBA(246, 249, 253, 255));
}

static int __xuiCheckCardIconRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_checkcard_icon_t* pIcon;
	xui_proxy pProxy;
	xui_rect_t tRect;
	float fX;
	float fY;
	int iRet;

	(void)iStateId;
	pIcon = (xui_checkcard_icon_t*)pUser;
	if ( pIcon == NULL ) return XUI_OK;
	pProxy = pIcon->pProxy;
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	tRect = xuiWidgetGetContentRect(pWidget);
	fX = tRect.fX + 7.0f;
	fY = tRect.fY + 7.0f;
	iRet = pProxy->drawRoundRectFill(pProxy, pDraw, (xui_rect_t){fX, fY, 13.0f, 31.0f}, 2.0f, pIcon->iColor);
	if ( iRet == XUI_OK ) iRet = pProxy->drawRectFill(pProxy, pDraw, (xui_rect_t){fX + 11.0f, fY + 22.0f, 15.0f, 9.0f}, pIcon->iColor);
	if ( iRet == XUI_OK ) iRet = pProxy->drawRoundRectFill(pProxy, pDraw, (xui_rect_t){fX + 23.0f, fY + 27.0f, 5.0f, 5.0f}, 2.5f, pIcon->iColor);
	if ( iRet == XUI_OK ) iRet = pProxy->drawRoundRectFill(pProxy, pDraw, (xui_rect_t){fX + 27.0f, fY + 36.0f, 4.0f, 4.0f}, 2.0f, __xuiCheckCardColorWithAlpha(pIcon->iColor, 150));
	return iRet;
}

static void __xuiCheckCardChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	xui_checkcard_demo_t* pDemo;

	(void)pWidget;
	(void)bChecked;
	pDemo = (xui_checkcard_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
	}
}

static void __xuiCheckCardGroupChanged(xui_widget pGroup, xui_widget pSelected, int iSelectedIndex, void* pUser)
{
	xui_checkcard_demo_t* pDemo;

	(void)pGroup;
	(void)pSelected;
	pDemo = (xui_checkcard_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iGroupIndex = iSelectedIndex;
		pDemo->iChangeCount++;
	}
}

static int __xuiCheckCardAddLabel(xui_checkcard_demo_t* pDemo, xui_widget pParent, xui_widget* ppLabel, const char* sText, xui_font pFont, uint32_t iColor, uint32_t iFlags, int iWrap)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pFont;
	tDesc.iTextColor = iColor;
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(142, 154, 169, 255);
	tDesc.iTextFlags = iFlags | XUI_TEXT_CLIP;
	tDesc.iWrapMode = iWrap;
	tDesc.fLineGap = 1.0f;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetHitTestVisible(pLabel, 0);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	*ppLabel = pLabel;
	return XUI_OK;
}

static int __xuiCheckCardAddIcon(xui_checkcard_demo_t* pDemo, int iIndex, xui_widget pCard, uint32_t iColor)
{
	xui_widget pIcon;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pIcon);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->arrIcon[iIndex].iColor = iColor;
	pDemo->arrIcon[iIndex].pProxy = &pDemo->tProxy;
	(void)xuiWidgetSetLayoutType(pIcon, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pIcon, __xuiCheckCardIconRender, &pDemo->arrIcon[iIndex]);
	(void)xuiWidgetSetHitTestVisible(pIcon, 0);
	iRet = xuiWidgetAddChild(pCard, pIcon);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pIcon);
		return iRet;
	}
	pDemo->pIcon[iIndex] = pIcon;
	return XUI_OK;
}

static int __xuiCheckCardCreateCard(xui_checkcard_demo_t* pDemo, int iIndex, int bChecked, xui_widget pRadioGroup)
{
	xui_check_card_desc_t tDesc;
	xui_widget pCard;
	xui_widget pParent;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.bChecked = bChecked;
	tDesc.fMinWidth = 320.0f;
	tDesc.fMinHeight = 88.0f;
	tDesc.fRadius = 0.0f;
	tDesc.fBorderWidth = 1.0f;
	tDesc.fCheckedBorderWidth = 1.0f;
	tDesc.fCornerSize = 18.0f;
	tDesc.fFocusWidth = 1.0f;
	iRet = xuiCheckCardCreate(pDemo->pContext, &pCard, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pCard, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetPadding(pCard, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	(void)xuiCheckCardSetChange(pCard, __xuiCheckCardChanged, pDemo);
	pParent = (pRadioGroup != NULL) ? pRadioGroup : pDemo->pRoot;
	if ( pRadioGroup != NULL ) {
		iRet = xuiRadioGroupAddCheckCard(pRadioGroup, pCard);
	} else {
		iRet = xuiWidgetAddChild(pParent, pCard);
	}
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pCard);
		return iRet;
	}
	pDemo->pCard[iIndex] = pCard;
	iRet = __xuiCheckCardAddIcon(pDemo, iIndex, pCard, (iIndex < LEFT_COUNT) ? XUI_COLOR_RGBA(29, 188, 174, 255) : XUI_COLOR_RGBA(60, 130, 214, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCheckCardAddLabel(pDemo, pCard, &pDemo->pTitle[iIndex], g_arrTitle[iIndex], pDemo->pTitleFont, XUI_COLOR_RGBA(36, 48, 62, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP, XUI_TEXT_WRAP_NONE);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiCheckCardAddLabel(pDemo, pCard, &pDemo->pDescription[iIndex], g_arrDescription[iIndex], pDemo->pBodyFont, XUI_COLOR_RGBA(96, 110, 126, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP, XUI_TEXT_WRAP_WORD);
}

static int __xuiCheckCardCreateUi(xui_checkcard_demo_t* pDemo)
{
	xui_radio_group_desc_t tGroupDesc;
	int i;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCheckCardRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiCheckCardAddLabel(pDemo, pDemo->pRoot, &pDemo->pHeading[0], "Multiple selection", pDemo->pTitleFont, XUI_COLOR_RGBA(32, 49, 67, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP, XUI_TEXT_WRAP_NONE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCheckCardAddLabel(pDemo, pDemo->pRoot, &pDemo->pHeading[1], "Single selection", pDemo->pTitleFont, XUI_COLOR_RGBA(32, 49, 67, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP, XUI_TEXT_WRAP_NONE);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tGroupDesc, 0, sizeof(tGroupDesc));
	tGroupDesc.iSize = sizeof(tGroupDesc);
	tGroupDesc.iOrientation = XUI_RADIO_GROUP_VERTICAL;
	tGroupDesc.fGap = 10.0f;
	iRet = xuiRadioGroupCreate(pDemo->pContext, &pDemo->pRightGroup, &tGroupDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRightGroup, XUI_LAYOUT_MANUAL);
	(void)xuiRadioGroupSetChange(pDemo->pRightGroup, __xuiCheckCardGroupChanged, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pRightGroup);
	if ( iRet != XUI_OK ) return iRet;

	for ( i = 0; i < LEFT_COUNT; i++ ) {
		iRet = __xuiCheckCardCreateCard(pDemo, i, i == 0, NULL);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < RIGHT_COUNT; i++ ) {
		iRet = __xuiCheckCardCreateCard(pDemo, LEFT_COUNT + i, i == 1, pDemo->pRightGroup);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static void __xuiCheckCardLayoutCard(xui_checkcard_demo_t* pDemo, int iIndex, float fX, float fY)
{
	(void)xuiWidgetSetRect(pDemo->pCard[iIndex], (xui_rect_t){fX, fY, 332.0f, 88.0f});
	(void)xuiWidgetSetRect(pDemo->pIcon[iIndex], (xui_rect_t){20.0f, 20.0f, 42.0f, 48.0f});
	(void)xuiWidgetSetRect(pDemo->pTitle[iIndex], (xui_rect_t){76.0f, 16.0f, 238.0f, 22.0f});
	(void)xuiWidgetSetRect(pDemo->pDescription[iIndex], (xui_rect_t){76.0f, 40.0f, 240.0f, 42.0f});
}

static void __xuiCheckCardLayout(xui_checkcard_demo_t* pDemo)
{
	int i;

	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetRect(pDemo->pHeading[0], (xui_rect_t){42.0f, 30.0f, 300.0f, 24.0f});
	(void)xuiWidgetSetRect(pDemo->pHeading[1], (xui_rect_t){426.0f, 30.0f, 300.0f, 24.0f});
	(void)xuiWidgetSetRect(pDemo->pRightGroup, (xui_rect_t){410.0f, 62.0f, 352.0f, 300.0f});
	for ( i = 0; i < LEFT_COUNT; i++ ) {
		__xuiCheckCardLayoutCard(pDemo, i, 36.0f, 62.0f + (float)i * 104.0f);
	}
	for ( i = 0; i < RIGHT_COUNT; i++ ) {
		__xuiCheckCardLayoutCard(pDemo, LEFT_COUNT + i, 0.0f, (float)i * 104.0f);
	}
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiCheckCardReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiCheckCardSendButtonTransitions(xui_checkcard_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCheckCardHandleInput(xui_checkcard_demo_t* pDemo)
{
	float fX;
	float fY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiCheckCardReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiCheckCardSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiCheckCardClick(xui_checkcard_demo_t* pDemo, xui_widget pWidget)
{
	xui_rect_t tWorld;
	float fX;
	float fY;

	tWorld = xuiWidgetGetWorldRect(pWidget);
	fX = tWorld.fX + tWorld.fW * 0.5f;
	fY = tWorld.fY + tWorld.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiCheckCardRunChecks(xui_checkcard_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tCard;
	xui_rect_t tIcon;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pRightGroup != NULL);
	for ( i = 0; i < CHECKCARD_COUNT; i++ ) {
		if ( pDemo->pCard[i] == NULL || pDemo->pIcon[i] == NULL || pDemo->pTitle[i] == NULL || pDemo->pDescription[i] == NULL ) {
			pDemo->bCreateOK = 0;
		}
	}
	tCard = xuiWidgetGetWorldRect(pDemo->pCard[0]);
	tIcon = xuiWidgetGetWorldRect(pDemo->pIcon[0]);
	pDemo->bLayoutOK = (tCard.fW > 300.0f) && (tIcon.fX > tCard.fX) && (tIcon.fX < tCard.fX + 70.0f);
	pDemo->bContentOK = (xuiWidgetGetParent(pDemo->pIcon[0]) == pDemo->pCard[0]) &&
		(xuiWidgetGetParent(pDemo->pTitle[0]) == pDemo->pCard[0]) &&
		(xuiWidgetGetParent(pDemo->pDescription[0]) == pDemo->pCard[0]);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		__xuiCheckCardClick(pDemo, pDemo->pCard[1]);
		__xuiCheckCardClick(pDemo, pDemo->pCard[3]);
		pDemo->bExerciseDone = 1;
	}
	pDemo->bMultiOK = xuiCheckCardGetChecked(pDemo->pCard[0]) &&
		(!bExerciseInput || xuiCheckCardGetChecked(pDemo->pCard[1]));
	if ( bExerciseInput ) {
		pDemo->bSingleOK = xuiCheckCardGetChecked(pDemo->pCard[3]) &&
			!xuiCheckCardGetChecked(pDemo->pCard[4]) &&
			!xuiCheckCardGetChecked(pDemo->pCard[5]) &&
			(xuiRadioGroupGetSelectedIndex(pDemo->pRightGroup) == 0);
	} else {
		pDemo->bSingleOK = !xuiCheckCardGetChecked(pDemo->pCard[3]) &&
			xuiCheckCardGetChecked(pDemo->pCard[4]) &&
			!xuiCheckCardGetChecked(pDemo->pCard[5]) &&
			(xuiRadioGroupGetSelectedIndex(pDemo->pRightGroup) == 1);
	}
	pDemo->bInputOK = !bExerciseInput || (pDemo->iChangeCount > 0);
}

static int __xuiCheckCardCreateAssets(xui_checkcard_demo_t* pDemo)
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
	sFontPath = __xuiCheckCardFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pTitleFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pBodyFont, sFontPath, 13.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pBodyFont);
	return __xuiCheckCardCreateUi(pDemo);
}

static void __xuiCheckCardDestroyAssets(xui_checkcard_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pTitleFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pTitleFont);
		pDemo->pTitleFont = NULL;
	}
	if ( pDemo->pBodyFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pBodyFont);
		pDemo->pBodyFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiCheckCardFrame(void* pUser)
{
	xui_checkcard_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int bAutoRun;
	int iRet;

	pDemo = (xui_checkcard_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiCheckCardHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCheckCardLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiCheckCardRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_checkcard final-summary frames=%d create=%d layout=%d multi=%d single=%d content=%d input=%d changes=%d group=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bMultiOK, pDemo->bSingleOK, pDemo->bContentOK, pDemo->bInputOK,
			pDemo->iChangeCount, pDemo->iGroupIndex, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_checkcard_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iGroupIndex = -1;
	iRet = __xuiCheckCardParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCheckCardUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI CheckCard";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_checkcard: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiCheckCardCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_checkcard: create assets failed: %d\n", iRet);
		__xuiCheckCardDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiCheckCardFrame, &tDemo);
	__xuiCheckCardDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bMultiOK && tDemo.bSingleOK && tDemo.bContentOK && tDemo.bInputOK) ? 0 : 1;
}
