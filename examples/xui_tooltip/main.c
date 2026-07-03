#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	780
#define DEMO_TARGET_H	540
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define TARGET_COUNT	8

typedef struct xui_tooltip_box_t {
	xui_proxy_t* pProxy;
	uint32_t iFill;
	uint32_t iStroke;
	float fStrokeWidth;
} xui_tooltip_box_t;

typedef struct xui_tooltip_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTargetSurface;
	xui_font pFont;
	xui_font pSmallFont;
	xui_widget pRoot;
	xui_widget pPanel;
	xui_widget pAnchorMarker;
	xui_widget pTarget[TARGET_COUNT];
	xui_widget pLabel[TARGET_COUNT];
	xui_tooltip_box_t tPanelBox;
	xui_tooltip_box_t tMarkerBox;
	xui_tooltip_box_t tTargetBox[TARGET_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int bCreateOK;
	int bTooltipOK;
	int bCustomOK;
} xui_tooltip_demo_t;

static void __xuiTooltipUsage(void)
{
	printf("usage: xui_tooltip [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiTooltipParseArgs(xui_tooltip_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiTooltipUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTooltipFindTtf(void)
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

static void __xuiTooltipThickness(float fLeft, float fTop, float fRight, float fBottom, xui_thickness_t* pOut)
{
	pOut->fLeft = fLeft;
	pOut->fTop = fTop;
	pOut->fRight = fRight;
	pOut->fBottom = fBottom;
}

static int __xuiTooltipSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	}
	return iRet;
}

static int __xuiTooltipBoxRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_tooltip_box_t* pBox;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	pBox = (xui_tooltip_box_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pBox == NULL) || (pBox->pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = pBox->pProxy->drawRectFill(pBox->pProxy, pDraw, tRect, pBox->iFill);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pBox->fStrokeWidth > 0.0f ) {
		iRet = pBox->pProxy->drawRectStroke(pBox->pProxy, pDraw, tRect, pBox->fStrokeWidth, pBox->iStroke);
	}
	return iRet;
}

static xui_vec2_t __xuiTooltipEquipmentMeasure(xui_context pContext, xui_widget pOwner, void* pUser)
{
	(void)pContext;
	(void)pOwner;
	(void)pUser;
	return (xui_vec2_t){310.0f, 226.0f};
}

static int __xuiTooltipDrawText(xui_tooltip_demo_t* pDemo, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( (pDemo == NULL) || (pDraw == NULL) || (pFont == NULL) ) {
		return XUI_OK;
	}
	return pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pFont, sText, tRect, iColor, iFlags | XUI_TEXT_CLIP);
}

static int __xuiTooltipEquipmentPaint(xui_context pContext, xui_widget pOwner, xui_draw_context pDraw, xui_rect_t tRect, void* pUser)
{
	xui_tooltip_demo_t* pDemo;
	xui_rect_t tInner;
	xui_rect_t tIcon;
	xui_rect_t tLine;
	xui_rect_t tText;
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	int iRet;

	(void)pContext;
	(void)pOwner;
	pDemo = (xui_tooltip_demo_t*)pUser;
	if ( (pDemo == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(18, 28, 42, 246));
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.drawRectStroke(&pDemo->tProxy, pDraw, tRect, 1.0f, XUI_COLOR_RGBA(94, 128, 168, 255));
	if ( iRet != XUI_OK ) return iRet;

	tInner = (xui_rect_t){tRect.fX + 12.0f, tRect.fY + 12.0f, tRect.fW - 24.0f, tRect.fH - 24.0f};
	tIcon = (xui_rect_t){tInner.fX, tInner.fY, 42.0f, 42.0f};
	iRet = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tIcon, XUI_COLOR_RGBA(52, 108, 190, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.drawRectStroke(&pDemo->tProxy, pDraw, tIcon, 1.0f, XUI_COLOR_RGBA(154, 204, 255, 255));
	if ( iRet != XUI_OK ) return iRet;
	tA = (xui_vec2_t){tIcon.fX + 10.0f, tIcon.fY + 30.0f};
	tB = (xui_vec2_t){tIcon.fX + 31.0f, tIcon.fY + 9.0f};
	tC = (xui_vec2_t){tIcon.fX + 28.0f, tIcon.fY + 30.0f};
	iRet = pDemo->tProxy.drawTriangleFill(&pDemo->tProxy, pDraw, tA, tB, tC, XUI_COLOR_RGBA(244, 220, 126, 255));
	if ( iRet != XUI_OK ) return iRet;

	tText = (xui_rect_t){tInner.fX + 54.0f, tInner.fY, tInner.fW - 54.0f, 23.0f};
	iRet = __xuiTooltipDrawText(pDemo, pDraw, pDemo->pFont, "Storm Rune Blade", tText, XUI_COLOR_RGBA(255, 218, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XUI_OK ) return iRet;
	tText.fY += 23.0f;
	tText.fH = 19.0f;
	iRet = __xuiTooltipDrawText(pDemo, pDraw, pDemo->pSmallFont, "Rare one-hand sword", tText, XUI_COLOR_RGBA(156, 184, 216, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XUI_OK ) return iRet;

	tLine = (xui_rect_t){tInner.fX, tInner.fY + 56.0f, tInner.fW, 1.0f};
	iRet = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tLine, XUI_COLOR_RGBA(70, 92, 118, 255));
	if ( iRet != XUI_OK ) return iRet;
	tText = (xui_rect_t){tInner.fX, tInner.fY + 66.0f, tInner.fW, 23.0f};
	iRet = __xuiTooltipDrawText(pDemo, pDraw, pDemo->pFont, "Attack +128        Guard +36", tText, XUI_COLOR_RGBA(226, 236, 248, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XUI_OK ) return iRet;
	tText.fY += 23.0f;
	iRet = __xuiTooltipDrawText(pDemo, pDraw, pDemo->pFont, "Crit +12%          Speed +8%", tText, XUI_COLOR_RGBA(226, 236, 248, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XUI_OK ) return iRet;

	tLine.fY = tInner.fY + 118.0f;
	iRet = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tLine, XUI_COLOR_RGBA(70, 92, 118, 255));
	if ( iRet != XUI_OK ) return iRet;
	tText.fY = tInner.fY + 128.0f;
	tText.fH = 22.0f;
	iRet = __xuiTooltipDrawText(pDemo, pDraw, pDemo->pSmallFont, "12% chance: chain lightning on hit", tText, XUI_COLOR_RGBA(146, 218, 255, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XUI_OK ) return iRet;
	tText.fY += 22.0f;
	iRet = __xuiTooltipDrawText(pDemo, pDraw, pDemo->pSmallFont, "8% chance: restore energy after block", tText, XUI_COLOR_RGBA(168, 234, 188, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XUI_OK ) return iRet;
	tText.fY += 22.0f;
	return __xuiTooltipDrawText(pDemo, pDraw, pDemo->pSmallFont, "Set bonus: cooldown recovery +5%", tText, XUI_COLOR_RGBA(226, 178, 255, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
}

static int __xuiTooltipDynamicResolve(xui_context pContext, xui_widget pWidget, xui_tooltip_desc_t* pDesc, void* pUser)
{
	xui_tooltip_demo_t* pDemo;
	xui_rect_t tRect;
	float fLocalX;

	(void)pContext;
	if ( (pWidget == NULL) || (pDesc == NULL) || (pUser == NULL) ) {
		return 0;
	}
	pDemo = (xui_tooltip_demo_t*)pUser;
	tRect = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pDemo->fUiMouseX - tRect.fX;
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->iType = XUI_TOOLTIP_TEXT;
	pDesc->iAnchor = XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fOffsetX = 10.0f;
	pDesc->fOffsetY = 12.0f;
	pDesc->fDelay = 0.0f;
	pDesc->bFollowCursor = 1;
	if ( fLocalX < tRect.fW * 0.33f ) {
		pDesc->sText = "Dynamic tooltip: left zone";
	} else if ( fLocalX < tRect.fW * 0.66f ) {
		pDesc->sText = "Dynamic tooltip: middle zone";
	} else {
		pDesc->sText = "Dynamic tooltip: right zone";
	}
	return 1;
}

static int __xuiTooltipAddTarget(xui_tooltip_demo_t* pDemo, int iIndex, xui_rect_t tRect, const char* sText, uint32_t iFill, const xui_tooltip_desc_t* pTooltip)
{
	xui_label_desc_t tLabel;
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pTarget[iIndex]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pDemo->tTargetBox[iIndex].pProxy = &pDemo->tProxy;
	pDemo->tTargetBox[iIndex].iFill = iFill;
	pDemo->tTargetBox[iIndex].iStroke = XUI_COLOR_RGBA(90, 142, 206, 255);
	pDemo->tTargetBox[iIndex].fStrokeWidth = 1.0f;
	(void)xuiWidgetSetLayoutType(pDemo->pTarget[iIndex], XUI_LAYOUT_OVERLAY);
	(void)xuiWidgetSetRect(pDemo->pTarget[iIndex], tRect);
	__xuiTooltipThickness(12.0f, 8.0f, 12.0f, 8.0f, &tPadding);
	(void)xuiWidgetSetPadding(pDemo->pTarget[iIndex], tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pTarget[iIndex], __xuiTooltipBoxRender, &pDemo->tTargetBox[iIndex]);

	memset(&tLabel, 0, sizeof(tLabel));
	tLabel.iSize = sizeof(tLabel);
	tLabel.sText = sText;
	tLabel.pFont = pDemo->pFont;
	tLabel.iTextColor = XUI_COLOR_RGBA(38, 50, 66, 255);
	tLabel.iDisabledTextColor = XUI_COLOR_RGBA(130, 138, 150, 255);
	tLabel.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tLabel.iWrapMode = XUI_TEXT_WRAP_NONE;
	iRet = xuiLabelCreate(pDemo->pContext, &pDemo->pLabel[iIndex], &tLabel);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)__xuiTooltipSetFillLayout(pDemo->pLabel[iIndex]);
	(void)xuiWidgetSetHitTestVisible(pDemo->pLabel[iIndex], 0);
	iRet = xuiWidgetAddChild(pDemo->pTarget[iIndex], pDemo->pLabel[iIndex]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pTooltip != NULL ) {
		iRet = xuiWidgetSetTooltip(pDemo->pTarget[iIndex], pTooltip);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return xuiWidgetAddChild(pDemo->pPanel, pDemo->pTarget[iIndex]);
}

static int __xuiTooltipCreateUi(xui_tooltip_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	xui_tooltip_desc_t tTip;
	xui_rect_t tCustomAnchor;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pPanel);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->tPanelBox.pProxy = &pDemo->tProxy;
	pDemo->tPanelBox.iFill = XUI_COLOR_RGBA(248, 250, 253, 255);
	pDemo->tPanelBox.iStroke = XUI_COLOR_RGBA(100, 176, 232, 255);
	pDemo->tPanelBox.fStrokeWidth = 1.0f;
	(void)xuiWidgetSetRect(pDemo->pPanel, (xui_rect_t){24.0f, 34.0f, 732.0f, 472.0f});
	(void)xuiWidgetSetLayoutType(pDemo->pPanel, XUI_LAYOUT_MANUAL);
	__xuiTooltipThickness(0.0f, 0.0f, 0.0f, 0.0f, &tPadding);
	(void)xuiWidgetSetPadding(pDemo->pPanel, tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pPanel, __xuiTooltipBoxRender, &pDemo->tPanelBox);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pPanel);
	if ( iRet != XUI_OK ) return iRet;

	tCustomAnchor = (xui_rect_t){352.0f, 224.0f, 28.0f, 28.0f};
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pAnchorMarker);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->tMarkerBox.pProxy = &pDemo->tProxy;
	pDemo->tMarkerBox.iFill = XUI_COLOR_RGBA(42, 120, 210, 255);
	pDemo->tMarkerBox.iStroke = XUI_COLOR_RGBA(255, 255, 255, 255);
	pDemo->tMarkerBox.fStrokeWidth = 2.0f;
	(void)xuiWidgetSetRect(pDemo->pAnchorMarker, tCustomAnchor);
	(void)xuiWidgetSetHitTestVisible(pDemo->pAnchorMarker, 0);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pAnchorMarker, __xuiTooltipBoxRender, &pDemo->tMarkerBox);
	iRet = xuiWidgetAddChild(pDemo->pPanel, pDemo->pAnchorMarker);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tTip, 0, sizeof(tTip));
	tTip.iSize = sizeof(tTip);
	tTip.iType = XUI_TOOLTIP_TEXT;
	tTip.sText = "anchor: widget bottom\noffsetY: +10";
	tTip.iAnchor = XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
	tTip.fOffsetX = 0.0f;
	tTip.fOffsetY = 10.0f;
	tTip.fDelay = 0.0f;
	iRet = __xuiTooltipAddTarget(pDemo, 0, (xui_rect_t){272.0f, 68.0f, 188.0f, 48.0f}, "Bottom + offset", XUI_COLOR_RGBA(232, 241, 252, 255), &tTip);
	if ( iRet != XUI_OK ) return iRet;

	tTip.sText = "anchor: widget top\noffsetY: +10";
	tTip.iAnchor = XUI_TOOLTIP_ANCHOR_WIDGET_TOP;
	tTip.fOffsetX = 0.0f;
	tTip.fOffsetY = 10.0f;
	tTip.fDelay = 0.0f;
	tTip.bFollowCursor = 0;
	tTip.bCustomAnchorRect = 0;
	iRet = __xuiTooltipAddTarget(pDemo, 1, (xui_rect_t){272.0f, 358.0f, 188.0f, 48.0f}, "Top + offset", XUI_COLOR_RGBA(255, 244, 220, 255), &tTip);
	if ( iRet != XUI_OK ) return iRet;

	tTip.sText = "anchor: widget right\noffsetX: +12";
	tTip.iAnchor = XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT;
	tTip.fOffsetX = 12.0f;
	tTip.fOffsetY = 0.0f;
	iRet = __xuiTooltipAddTarget(pDemo, 2, (xui_rect_t){62.0f, 213.0f, 188.0f, 48.0f}, "Right + offset", XUI_COLOR_RGBA(236, 246, 238, 255), &tTip);
	if ( iRet != XUI_OK ) return iRet;

	tTip.sText = "anchor: widget left\noffsetX: +12";
	tTip.iAnchor = XUI_TOOLTIP_ANCHOR_WIDGET_LEFT;
	tTip.fOffsetX = 12.0f;
	tTip.fOffsetY = 0.0f;
	iRet = __xuiTooltipAddTarget(pDemo, 3, (xui_rect_t){482.0f, 213.0f, 188.0f, 48.0f}, "Left + offset", XUI_COLOR_RGBA(238, 236, 250, 255), &tTip);
	if ( iRet != XUI_OK ) return iRet;

	tTip.sText = "anchor: cursor\nfollowCursor: true\noffset: 14,16";
	tTip.iAnchor = XUI_TOOLTIP_ANCHOR_CURSOR;
	tTip.fOffsetX = 14.0f;
	tTip.fOffsetY = 16.0f;
	tTip.fDelay = 0.0f;
	tTip.bFollowCursor = 1;
	iRet = __xuiTooltipAddTarget(pDemo, 4, (xui_rect_t){62.0f, 82.0f, 188.0f, 48.0f}, "Cursor follow", XUI_COLOR_RGBA(232, 246, 250, 255), &tTip);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiTooltipAddTarget(pDemo, 5, (xui_rect_t){482.0f, 82.0f, 188.0f, 48.0f}, "Dynamic cursor resolver", XUI_COLOR_RGBA(232, 246, 250, 255), NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetTooltipResolver(pDemo->pTarget[5], __xuiTooltipDynamicResolve, pDemo);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tTip, 0, sizeof(tTip));
	tTip.iSize = sizeof(tTip);
	tTip.iType = XUI_TOOLTIP_TEXT;
	tTip.sText = "anchor: custom rect\nthis blue marker owns the popup position";
	tTip.iAnchor = XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
	tTip.bCustomAnchorRect = 1;
	tTip.tAnchorRect = (xui_rect_t){24.0f + tCustomAnchor.fX, 34.0f + tCustomAnchor.fY, tCustomAnchor.fW, tCustomAnchor.fH};
	tTip.fOffsetY = 8.0f;
	tTip.fDelay = 0.0f;
	iRet = __xuiTooltipAddTarget(pDemo, 6, (xui_rect_t){62.0f, 354.0f, 188.0f, 48.0f}, "Custom anchor rect", XUI_COLOR_RGBA(245, 238, 250, 255), &tTip);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tTip, 0, sizeof(tTip));
	tTip.iSize = sizeof(tTip);
	tTip.iType = XUI_TOOLTIP_CUSTOM;
	tTip.iAnchor = XUI_TOOLTIP_ANCHOR_WIDGET_LEFT;
	tTip.fOffsetX = 12.0f;
	tTip.fDelay = 0.0f;
	tTip.onMeasure = __xuiTooltipEquipmentMeasure;
	tTip.onPaint = __xuiTooltipEquipmentPaint;
	tTip.pUser = pDemo;
	return __xuiTooltipAddTarget(pDemo, 7, (xui_rect_t){482.0f, 354.0f, 188.0f, 48.0f}, "Custom paint + left", XUI_COLOR_RGBA(235, 240, 248, 255), &tTip);
}

static uint32_t __xuiTooltipReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) {
		iButtons |= XUI_POINTER_BUTTON_LEFT;
	}
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) {
		iButtons |= XUI_POINTER_BUTTON_RIGHT;
	}
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) {
		iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	}
	return iButtons;
}

static int __xuiTooltipSendButtonTransitions(xui_tooltip_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiTooltipHandleInput(xui_tooltip_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiTooltipReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiTooltipSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	if ( (fWheelX != 0.0f) || (fWheelY != 0.0f) ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiTooltipRunChecks(xui_tooltip_demo_t* pDemo, int bExerciseTooltip)
{
	xui_rect_t tOwner;
	xui_rect_t tMarker;
	xui_rect_t tTip;
	int bAllOK;

	pDemo->bCreateOK =
		(pDemo->pPanel != NULL) &&
		(pDemo->pTarget[0] != NULL) &&
		(pDemo->pTarget[7] != NULL) &&
		(pDemo->pAnchorMarker != NULL) &&
		(xuiWidgetGetTooltip(pDemo->pTarget[0])->iType == XUI_TOOLTIP_TEXT) &&
		(xuiWidgetGetTooltip(pDemo->pTarget[6])->bCustomAnchorRect != 0) &&
		(xuiWidgetGetTooltip(pDemo->pTarget[7])->iType == XUI_TOOLTIP_CUSTOM);
	if ( !pDemo->bCreateOK ) {
		return;
	}
	if ( !bExerciseTooltip ) {
		pDemo->bTooltipOK = 1;
		pDemo->bCustomOK = 1;
		return;
	}

	bAllOK = 1;
	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[0]);
	pDemo->fUiMouseX = tOwner.fX + tOwner.fW * 0.5f;
	pDemo->fUiMouseY = tOwner.fY + tOwner.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	tTip = xuiWidgetTooltipGetRect(pDemo->pContext);
	bAllOK = bAllOK && (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[0]) && (tTip.fY >= tOwner.fY + tOwner.fH);

	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[1]);
	pDemo->fUiMouseX = tOwner.fX + tOwner.fW * 0.5f;
	pDemo->fUiMouseY = tOwner.fY + tOwner.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	tTip = xuiWidgetTooltipGetRect(pDemo->pContext);
	bAllOK = bAllOK && (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[1]) && (tTip.fY + tTip.fH <= tOwner.fY);

	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[2]);
	pDemo->fUiMouseX = tOwner.fX + tOwner.fW * 0.5f;
	pDemo->fUiMouseY = tOwner.fY + tOwner.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	tTip = xuiWidgetTooltipGetRect(pDemo->pContext);
	bAllOK = bAllOK && (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[2]) && (tTip.fX >= tOwner.fX + tOwner.fW);

	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[3]);
	pDemo->fUiMouseX = tOwner.fX + tOwner.fW * 0.5f;
	pDemo->fUiMouseY = tOwner.fY + tOwner.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	tTip = xuiWidgetTooltipGetRect(pDemo->pContext);
	bAllOK = bAllOK && (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[3]) && (tTip.fX + tTip.fW <= tOwner.fX);

	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[4]);
	pDemo->fUiMouseX = tOwner.fX + 24.0f;
	pDemo->fUiMouseY = tOwner.fY + 20.0f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	tTip = xuiWidgetTooltipGetRect(pDemo->pContext);
	bAllOK = bAllOK && (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[4]) && (tTip.fX >= pDemo->fUiMouseX) && (tTip.fY >= pDemo->fUiMouseY);

	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[5]);
	pDemo->fUiMouseX = tOwner.fX + tOwner.fW - 18.0f;
	pDemo->fUiMouseY = tOwner.fY + 20.0f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	bAllOK = bAllOK && (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[5]) && xuiWidgetTooltipIsOpen(pDemo->pContext);

	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[6]);
	tMarker = xuiWidgetGetWorldRect(pDemo->pAnchorMarker);
	pDemo->fUiMouseX = tOwner.fX + tOwner.fW * 0.5f;
	pDemo->fUiMouseY = tOwner.fY + tOwner.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	tTip = xuiWidgetTooltipGetRect(pDemo->pContext);
	bAllOK = bAllOK && (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[6]) && (tTip.fY >= tMarker.fY + tMarker.fH);

	tOwner = xuiWidgetGetWorldRect(pDemo->pTarget[7]);
	pDemo->fUiMouseX = tOwner.fX + tOwner.fW * 0.5f;
	pDemo->fUiMouseY = tOwner.fY + tOwner.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, 0);
	(void)xuiUpdate(pDemo->pContext, 0.0f);
	tTip = xuiWidgetTooltipGetRect(pDemo->pContext);
	pDemo->bTooltipOK = bAllOK && (xuiWidgetTooltipIsOpen(pDemo->pContext) != 0);
	pDemo->bCustomOK = (xuiWidgetTooltipGetOwner(pDemo->pContext) == pDemo->pTarget[7]) && (tTip.fW >= 300.0f) && (tTip.fH >= 220.0f) && (tTip.fX + tTip.fW <= tOwner.fX);
}

static int __xuiTooltipCreateAssets(xui_tooltip_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_tooltip: xuiCreate failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) {
		printf("xui_tooltip: xuiSetProxy failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) {
		return iRet;
	}

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTargetSurface, &tSurfaceDesc);
	if ( iRet != XUI_OK ) {
		printf("xui_tooltip: surfaceCreate failed: %d\n", iRet);
		return iRet;
	}
	sFontPath = __xuiTooltipFindTtf();
	if ( sFontPath == NULL ) {
		printf("xui_tooltip: no usable system TTF font found\n");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pSmallFont, sFontPath, 12.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTooltipCreateUi(pDemo);
}

static void __xuiTooltipDestroyAssets(xui_tooltip_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
	}
	if ( pDemo->pSmallFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pSmallFont);
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	}
	if ( pDemo->pTargetSurface != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTargetSurface);
	}
}

static int __xuiTooltipFrame(void* pUser)
{
	xui_tooltip_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_tooltip_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiTooltipHandleInput(pDemo);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiTooltipRunChecks(pDemo, (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0));
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTargetSurface, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTargetSurface, &tFullRect, 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}

	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTargetSurface, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		printf("xui_tooltip final-summary frames=%d create=%d tooltip=%d custom=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bTooltipOK, pDemo->bCustomOK);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_tooltip_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiTooltipParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiTooltipUsage();
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 800;
	tDesc.iHeight = 580;
	tDesc.sTitle = "xui_tooltip";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_tooltip: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTooltipCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		__xuiTooltipDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiTooltipFrame, &tDemo);
	__xuiTooltipDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bTooltipOK && tDemo.bCustomOK) ? 0 : 1;
}
