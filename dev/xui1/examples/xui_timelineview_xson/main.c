#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSON_BUFFER_SIZE 65536

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	char sXson[XSON_BUFFER_SIZE];
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_timelineview_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int AppendText(char* sBuffer, int* pOffset, int iCapacity, const char* sText)
{
	int iLen;

	if ( (sBuffer == NULL) || (pOffset == NULL) || (sText == NULL) || (*pOffset >= iCapacity) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iLen = (int)strlen(sText);
	if ( (*pOffset + iLen) >= iCapacity ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sBuffer + *pOffset, sText, (size_t)iLen);
	*pOffset += iLen;
	sBuffer[*pOffset] = 0;
	return XGE_OK;
}

static int BuildXson(app_state_t* pApp)
{
	int iOffset;

	iOffset = 0;
	pApp->sXson[0] = 0;
	return AppendText(
		pApp->sXson,
		&iOffset,
		XSON_BUFFER_SIZE,
		"{\"xui\":1,"
		"\"styles\":{"
		"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":18,\"gap\":10,\"background\":\"#ECF0F6FF\"},"
		"\"title\":{\"height\":32,\"font\":\"@fonts.default\",\"textColor\":\"#2A3A4EFF\"},"
		"\"timeline\":{\"width\":\"100%\",\"height\":\"grow\",\"font\":\"@fonts.default\",\"backgroundColor\":\"#F8FBFFFF\",\"headerColor\":\"#E4F0FBFF\",\"layerColor\":\"#F7FAFDFF\",\"selectedColor\":\"#BEDBF2FF\",\"gridColor\":\"#BCD7ECFF\",\"textColor\":\"#2A3442FF\",\"layerHeaderWidth\":172,\"frameWidth\":12,\"rowHeight\":24,\"rulerHeight\":26,\"scrollbarMode\":\"full\"}"
		"},"
		"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
		"{\"type\":\"label\",\"id\":\"title\",\"style\":\"title\",\"text\":\"XSON TimelineView: layers, frames, spans, selection and scroll\"},"
		"{\"type\":\"timelineView\",\"id\":\"timeline\",\"style\":\"timeline\",\"frameCount\":160,\"frameRate\":24,\"currentFrame\":24,\"scrollX\":180,\"scrollY\":12,"
		"\"layers\":["
		"{\"id\":101,\"name\":\"camera\",\"color\":\"#4A8ED2FF\",\"frames\":[{\"frame\":0,\"type\":\"key\"},{\"frame\":24,\"type\":\"key\"},{\"frame\":48,\"type\":\"normal\"}],\"spans\":[{\"id\":201,\"start\":0,\"end\":24,\"type\":\"motion\",\"label\":\"camera pan\",\"color\":\"#CFE3FAFF\"}]},"
		"{\"id\":102,\"name\":\"character\",\"height\":28,\"frames\":[{\"frame\":4,\"type\":\"blankKey\"},{\"frame\":12,\"type\":\"key\"},{\"frame\":56,\"type\":\"key\"}],\"spans\":[{\"id\":202,\"start\":12,\"end\":56,\"type\":\"motion\",\"label\":\"walk cycle\",\"color\":\"#D6EEDBFF\"}]},"
		"{\"id\":103,\"name\":\"shadow\",\"visible\":false,\"frames\":[{\"frame\":16,\"type\":\"normal\"},{\"frame\":58,\"type\":\"normal\"}],\"spans\":[{\"id\":203,\"start\":16,\"end\":58,\"type\":\"hold\",\"label\":\"shadow hold\",\"color\":\"#ECE7F6FF\"}]},"
		"{\"id\":104,\"name\":\"effects\",\"frames\":[{\"frame\":30,\"type\":\"key\"},{\"frame\":72,\"type\":\"blankKey\"}],\"spans\":[{\"id\":204,\"start\":30,\"end\":72,\"type\":\"event\",\"label\":\"spark\",\"customType\":\"fx\",\"color\":\"#F9E8C9FF\"}]},"
		"{\"id\":105,\"name\":\"locked guide\",\"locked\":true,\"frames\":[{\"frame\":8,\"type\":\"key\"}]}"
		"],"
		"\"selection\":[[1,12],{\"layer\":1,\"frame\":13},{\"layer\":3,\"frame\":30}]"
		"}]}}");
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( pApp->bFontReady ) {
		xgeXuiTokenSetFont(&pApp->tXui, "default", &pApp->tFont);
	}
	if ( BuildXson(pApp) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, pApp->sXson, (int)strlen(pApp->sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_timelineview_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int iW;
	int iH;

	iW = xgeGetWidth();
	iH = xgeGetHeight();
	if ( (iW == pApp->iLastWidth) && (iH == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	if ( pRoot == NULL ) {
		return;
	}
	if ( iW < 1040 ) {
		iW = 1040;
	}
	if ( iH < 620 ) {
		iH = 620;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, (float)iW, (float)iH });
	pApp->iLastWidth = xgeGetWidth();
	pApp->iLastHeight = xgeGetHeight();
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pTimelineWidget;
	xge_xui_timeline_view pTimeline;
	int iOverlapResult;
	int iLockedResult;
	int iRenameLockedResult;

	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	pTimelineWidget = xgeXuiPageFind(&pApp->tPage, "timeline");
	pTimeline = (pApp->tPage.iTimelineViewCount > 0) ? &pApp->tPage.arrTimelineView[0] : NULL;
	pApp->bCreateOK = (pRoot != NULL) && (pTimelineWidget != NULL) && (pTimeline != NULL);
	if ( pTimeline == NULL ) {
		return;
	}
	pApp->bLayoutOK =
		(pTimelineWidget->tRect.fW > 600.0f) &&
		(pTimelineWidget->tRect.fH > 360.0f) &&
		(xgeXuiScrollFrameGetViewportRect(pTimeline->pFrame).fW > 360.0f);
	iOverlapResult = xgeXuiTimelineViewAddSpan(pTimeline, 1, 301, 20, 28, XGE_XUI_TIMELINE_SPAN_MOTION, "overlap rejected");
	iLockedResult = xgeXuiTimelineViewSetFrameType(pTimeline, 4, 12, XGE_XUI_TIMELINE_FRAME_KEY);
	iRenameLockedResult = xgeXuiTimelineViewSetLayerName(pTimeline, 4, "locked guide renamed by xson check");
	pApp->bStateOK =
		(xgeXuiTimelineViewGetFrameCount(pTimeline) == 160) &&
		(xgeXuiTimelineViewGetFrameRate(pTimeline) == 24) &&
		(xgeXuiTimelineViewGetCurrentFrame(pTimeline) == 24) &&
		(xgeXuiTimelineViewGetLayerCount(pTimeline) == 5) &&
		(xgeXuiTimelineViewGetFrameType(pTimeline, 0, 24) == XGE_XUI_TIMELINE_FRAME_KEY) &&
		(xgeXuiTimelineViewGetFrameType(pTimeline, 1, 4) == XGE_XUI_TIMELINE_FRAME_BLANK_KEY) &&
		(xgeXuiTimelineViewGetLayer(pTimeline, 2)->bVisible == 0) &&
		(xgeXuiTimelineViewGetLayer(pTimeline, 4)->bLocked != 0) &&
		(xgeXuiTimelineViewGetSelectionCount(pTimeline) == 3) &&
		(xgeXuiTimelineViewGetScrollbarMode(pTimeline) == XGE_XUI_SCROLLBAR_MODE_FULL) &&
		(iOverlapResult < 0) &&
		(iLockedResult != XGE_OK) &&
		(iRenameLockedResult == XGE_OK);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
	xgeXuiUnit(&pApp->tXui);
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
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;
	xge_xui_timeline_view pTimeline;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		pTimeline = (pApp->tPage.iTimelineViewCount > 0) ? &pApp->tPage.arrTimelineView[0] : NULL;
		printf("xui_timelineview_xson final-summary frames=%d create=%d layout=%d state=%d layers=%d selection=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			(pTimeline != NULL) ? xgeXuiTimelineViewGetLayerCount(pTimeline) : 0,
			(pTimeline != NULL) ? xgeXuiTimelineViewGetSelectionCount(pTimeline) : 0);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TIMELINEVIEW_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 1040;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI TimelineView XSON";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		printf("xui_timelineview_xson xgeInit failed\n");
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
