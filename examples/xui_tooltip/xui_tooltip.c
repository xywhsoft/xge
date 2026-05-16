#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TARGET_COUNT 6

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_font_t tFontSmall;
	xge_xui_widget pPanel;
	xge_xui_widget pTarget[TARGET_COUNT];
	xge_xui_label_t tLabel[TARGET_COUNT];
	int bFontReady;
	int bSmallFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bTooltipOK;
	int bCustomOK;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFontOne(xge_font pFont, float fSize)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], fSize) == XGE_OK ) {
			printf("xui_tooltip font loaded: %s size=%.1f\n", arrFonts[i], fSize);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static xge_font AppSmallFont(app_state_t* pApp)
{
	return pApp->bSmallFontReady ? &pApp->tFontSmall : AppFont(pApp);
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + pWidget->tRect.fW * 0.5f;
	tCenter.fY = pWidget->tRect.fY + pWidget->tRect.fH * 0.5f;
	return tCenter;
}

static void MakeMouseMove(xge_event_t* pEvent, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_MOVE;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void Text(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( pFont != NULL ) {
		xgeTextDrawRect(pFont, sText, tRect, iColor, iFlags | XGE_TEXT_CLIP);
	}
}

static xge_vec2_t EquipmentTooltipMeasure(xge_xui_context pContext, xge_xui_widget pOwner, void* pUser)
{
	(void)pContext;
	(void)pOwner;
	(void)pUser;
	return (xge_vec2_t){ 310.0f, 242.0f };
}

static void EquipmentTooltipPaint(xge_xui_context pContext, xge_xui_widget pOwner, xge_rect_t tRect, void* pUser)
{
	app_state_t* pApp;
	xge_font pFont;
	xge_font pSmall;
	xge_rect_t tInner;
	xge_rect_t tIcon;
	xge_rect_t tLine;
	xge_rect_t tText;

	(void)pContext;
	(void)pOwner;
	pApp = (app_state_t*)pUser;
	pFont = (pApp != NULL) ? AppFont(pApp) : NULL;
	pSmall = (pApp != NULL) ? AppSmallFont(pApp) : pFont;
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(18, 28, 42, 244));
	xgeShapeRectStrokePx((xge_rect_t){ tRect.fX + 0.5f, tRect.fY + 0.5f, tRect.fW - 1.0f, tRect.fH - 1.0f }, 1.0f, XGE_COLOR_RGBA(94, 128, 168, 255));
	tInner = (xge_rect_t){ tRect.fX + 12.0f, tRect.fY + 12.0f, tRect.fW - 24.0f, tRect.fH - 24.0f };
	tIcon = (xge_rect_t){ tInner.fX, tInner.fY, 42.0f, 42.0f };
	xgeShapeRectFillPx(tIcon, XGE_COLOR_RGBA(52, 108, 190, 255));
	xgeShapeRectStrokePx((xge_rect_t){ tIcon.fX + 0.5f, tIcon.fY + 0.5f, tIcon.fW - 1.0f, tIcon.fH - 1.0f }, 1.0f, XGE_COLOR_RGBA(154, 204, 255, 255));
	xgeShapeLinePx(tIcon.fX + 10.0f, tIcon.fY + 30.0f, tIcon.fX + 30.0f, tIcon.fY + 10.0f, 3.0f, XGE_COLOR_RGBA(244, 220, 126, 255));
	xgeShapeLinePx(tIcon.fX + 22.0f, tIcon.fY + 31.0f, tIcon.fX + 32.0f, tIcon.fY + 21.0f, 2.0f, XGE_COLOR_RGBA(244, 220, 126, 255));
	Text(pFont, "风暴符文剑", (xge_rect_t){ tInner.fX + 54.0f, tInner.fY, tInner.fW - 54.0f, 22.0f }, XGE_COLOR_RGBA(255, 218, 96, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	Text(pSmall, "稀有 单手剑", (xge_rect_t){ tInner.fX + 54.0f, tInner.fY + 22.0f, tInner.fW - 54.0f, 18.0f }, XGE_COLOR_RGBA(156, 184, 216, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	tLine = (xge_rect_t){ tInner.fX, tInner.fY + 56.0f, tInner.fW, 1.0f };
	xgeShapeRectFillPx(tLine, XGE_COLOR_RGBA(70, 92, 118, 255));
	tText = (xge_rect_t){ tInner.fX, tInner.fY + 66.0f, tInner.fW, 24.0f };
	Text(pFont, "攻击力  +128        防御力  +36", tText, XGE_COLOR_RGBA(226, 236, 248, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	tText.fY += 24.0f;
	Text(pFont, "暴击率  +12%        攻速    +8%", tText, XGE_COLOR_RGBA(226, 236, 248, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	tLine.fY = tInner.fY + 120.0f;
	xgeShapeRectFillPx(tLine, XGE_COLOR_RGBA(70, 92, 118, 255));
	tText.fY = tInner.fY + 130.0f;
	tText.fH = 24.0f;
	Text(pSmall, "12% 概率：命中时触发连锁闪电", tText, XGE_COLOR_RGBA(146, 218, 255, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	tText.fY += 24.0f;
	Text(pSmall, "8% 概率：格挡后恢复 3 点怒气", tText, XGE_COLOR_RGBA(168, 234, 188, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	tText.fY += 24.0f;
	Text(pSmall, "套装词条：技能冷却恢复 +5%", tText, XGE_COLOR_RGBA(226, 178, 255, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
}

static int DynamicTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser)
{
	float fLocalX;

	(void)pUser;
	if ( (pContext == NULL) || (pWidget == NULL) || (pDesc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iType = XGE_XUI_TOOLTIP_TEXT;
	pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fOffsetX = 10.0f;
	pDesc->fOffsetY = 12.0f;
	pDesc->fDelay = 0.0f;
	pDesc->bFollowCursor = 1;
	fLocalX = pContext->fTooltipMouseX - pWidget->tRect.fX;
	if ( fLocalX < pWidget->tRect.fW * 0.33f ) {
		pDesc->sText = "Dynamic tooltip: left zone";
	} else if ( fLocalX < pWidget->tRect.fW * 0.66f ) {
		pDesc->sText = "Dynamic tooltip: middle zone";
	} else {
		pDesc->sText = "Dynamic tooltip: right zone";
	}
	return 1;
}

static int AddTarget(app_state_t* pApp, int iIndex, const char* sText, uint32_t iBackground, const xge_xui_tooltip_desc_t* pTooltip)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(42.0f));
	xgeXuiWidgetSetPaddingPx(pWidget, 12.0f, 8.0f, 12.0f, 8.0f);
	xgeXuiWidgetSetBackground(pWidget, iBackground);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(90, 142, 206, 255));
	xgeXuiWidgetSetRadius(pWidget, 4.0f);
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, AppFont(pApp), sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(38, 50, 66, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
	if ( pTooltip != NULL ) {
		xgeXuiWidgetSetTooltip(pWidget, pTooltip);
	}
	if ( xgeXuiWidgetAdd(pApp->pPanel, pWidget) != XGE_OK ) {
		xgeXuiLabelUnit(&pApp->tLabel[iIndex]);
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	pApp->pTarget[iIndex] = pWidget;
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_tooltip_desc_t tTip;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, AppFont(pApp));
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);

	pApp->pPanel = xgeXuiWidgetCreate();
	if ( pApp->pPanel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pApp->pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiWidgetSetGap(pApp->pPanel, 12.0f);
	xgeXuiWidgetSetBackground(pApp->pPanel, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetBorder(pApp->pPanel, 1.0f, XGE_COLOR_RGBA(100, 176, 232, 255));
	xgeXuiWidgetSetRadius(pApp->pPanel, 4.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	memset(&tTip, 0, sizeof(tTip));
	tTip.iType = XGE_XUI_TOOLTIP_TEXT;
	tTip.sText = "Static text tooltip\nUse this for short guidance.";
	tTip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
	tTip.fOffsetY = 6.0f;
	tTip.fDelay = 0.0f;
	if ( AddTarget(pApp, 0, "Static text", XGE_COLOR_RGBA(232, 241, 252, 255), &tTip) != XGE_OK ) {
		return XGE_ERROR;
	}
	tTip.sText = "Right anchored tooltip";
	tTip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT;
	tTip.fOffsetX = 8.0f;
	tTip.fOffsetY = 0.0f;
	if ( AddTarget(pApp, 1, "Widget right anchor", XGE_COLOR_RGBA(236, 246, 238, 255), &tTip) != XGE_OK ) {
		return XGE_ERROR;
	}
	tTip.sText = "Delayed tooltip, useful when hints should not flash immediately.";
	tTip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_TOP;
	tTip.fOffsetX = 0.0f;
	tTip.fOffsetY = -6.0f;
	tTip.fDelay = 0.45f;
	if ( AddTarget(pApp, 2, "Delayed opening", XGE_COLOR_RGBA(255, 244, 220, 255), &tTip) != XGE_OK ) {
		return XGE_ERROR;
	}
	tTip.sText = "This tooltip follows the cursor.";
	tTip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_CURSOR;
	tTip.fOffsetX = 10.0f;
	tTip.fOffsetY = 12.0f;
	tTip.fDelay = 0.0f;
	tTip.bFollowCursor = 1;
	if ( AddTarget(pApp, 3, "Follow cursor", XGE_COLOR_RGBA(238, 236, 250, 255), &tTip) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( AddTarget(pApp, 4, "Dynamic resolver", XGE_COLOR_RGBA(232, 246, 250, 255), NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetTooltipResolver(pApp->pTarget[4], DynamicTooltipResolve, pApp);

	memset(&tTip, 0, sizeof(tTip));
	tTip.iType = XGE_XUI_TOOLTIP_CUSTOM;
	tTip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT;
	tTip.fOffsetX = 10.0f;
	tTip.fDelay = 0.0f;
	tTip.procMeasure = EquipmentTooltipMeasure;
	tTip.procPaint = EquipmentTooltipPaint;
	tTip.pUser = pApp;
	if ( AddTarget(pApp, 5, "Custom equipment tooltip", XGE_COLOR_RGBA(235, 240, 248, 255), &tTip) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int iWidth;
	int iHeight;
	float fW;
	float fH;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (pApp->iLastWidth == iWidth) && (pApp->iLastHeight == iHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return;
	}
	fW = (float)iWidth;
	fH = (float)iHeight;
	if ( fW < 760.0f ) {
		fW = 760.0f;
	}
	if ( fH < 560.0f ) {
		fH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 48.0f, 360.0f, 388.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp, int bExerciseTooltip)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	xge_rect_t tRect;

	pApp->bCreateOK =
		(pApp->pPanel != NULL) &&
		(pApp->pTarget[0] != NULL) &&
		(pApp->pTarget[5] != NULL) &&
		(xgeXuiWidgetGetTooltip(pApp->pTarget[0])->iType == XGE_XUI_TOOLTIP_TEXT) &&
		(xgeXuiWidgetGetTooltip(pApp->pTarget[5])->iType == XGE_XUI_TOOLTIP_CUSTOM);
	if ( !pApp->bCreateOK ) {
		return;
	}
	if ( !bExerciseTooltip ) {
		pApp->bTooltipOK = 1;
		pApp->bCustomOK = 1;
		return;
	}
	tCenter = WidgetCenter(pApp->pTarget[5]);
	MakeMouseMove(&tEvent, tCenter.fX, tCenter.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	tRect = xgeXuiWidgetTooltipGetRect(&pApp->tXui);
	pApp->bTooltipOK =
		(xgeXuiWidgetTooltipIsOpen(&pApp->tXui) != 0) &&
		(xgeXuiWidgetTooltipGetOwner(&pApp->tXui) == pApp->pTarget[5]);
	pApp->bCustomOK = (tRect.fW >= 300.0f) && (tRect.fH >= 230.0f);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFontOne(&pApp->tFont, XGE_XUI_DEMO_FONT_SIZE) == XGE_OK);
	pApp->bSmallFontReady = (LoadFontOne(&pApp->tFontSmall, 10.0f) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp, pApp->iFrameLimit > 0);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < TARGET_COUNT; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bSmallFontReady ) {
		xgeFontFree(&pApp->tFontSmall);
	}
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	return xgeXuiDispatchEvent(&pApp->tXui, pEvent);
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp, pApp->iFrameLimit > 0);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_tooltip final-summary frames=%d create=%d tooltip=%d custom=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bTooltipOK, pApp->bCustomOK);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TOOLTIP_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Tooltip";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bTooltipOK && tApp.bCustomOK) ? 0 : 3;
}
