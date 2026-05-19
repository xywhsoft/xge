#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[20,20,20,20],\"columns\":3,\"rowHeight\":460,\"columnGap\":12,\"rowGap\":12,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":24,\"textColor\":\"#303A48FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"button\":{\"type\":\"button\",\"font\":\"@fonts.body\",\"height\":32,\"color\":\"#387ECCFF\",\"hoverColor\":\"#4692DEFF\",\"activeColor\":\"#2460AAFF\",\"textColor\":\"#F8FCFFFF\"},"
"\"accordion\":{\"type\":\"accordion\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":\"100%\",\"headerHeight\":28,\"spacing\":4,\"contentPadding\":8,\"background\":\"#F6FBFFFF\",\"headerColor\":\"#E8EEF7FF\",\"hoverColor\":\"#DAE6F6FF\",\"expandedColor\":\"#2E7CD6FF\",\"contentColor\":\"#F8FCFFFF\",\"borderColor\":\"#7FC4E5FF\",\"textColor\":\"#242A34FF\"},"
"\"accentAccordion\":{\"@parent\":\"accordion\",\"headerHeight\":32,\"spacing\":6,\"contentPadding\":10,\"headerColor\":\"#D6E8F4FF\",\"hoverColor\":\"#E2F2FCFF\",\"expandedColor\":\"#2A70A8FF\",\"contentColor\":\"#ECF8FCFF\",\"borderColor\":\"#569AC8FF\"}"
"},"
"\"tree\":{\"type\":\"grid\",\"style\":\"root\",\"children\":["
"{\"type\":\"accordion\",\"style\":\"accordion\",\"mode\":\"multiple\",\"sections\":["
"{\"title\":\"Multiple section A\",\"expanded\":true,\"children\":[{\"type\":\"label\",\"style\":\"label\",\"text\":\"Expanded client uses normal column layout.\"}]},"
"{\"title\":\"Multiple section B\",\"expanded\":false,\"children\":[{\"type\":\"label\",\"style\":\"label\",\"text\":\"Collapsed content should not occupy layout space.\"}]},"
"{\"title\":\"Disabled header\",\"expanded\":true,\"enabled\":false,\"children\":[{\"type\":\"label\",\"style\":\"label\",\"text\":\"The header is disabled but client remains visible.\"}]}"
"]},"
"{\"type\":\"accordion\",\"style\":\"accordion\",\"mode\":\"single\",\"sections\":["
"{\"title\":\"Single page one\",\"expanded\":true,\"children\":[{\"type\":\"label\",\"style\":\"label\",\"text\":\"Single mode keeps only one expanded section.\"}]},"
"{\"title\":\"Single page two\",\"expanded\":true,\"children\":[{\"type\":\"label\",\"style\":\"label\",\"text\":\"Adding an expanded section collapses the earlier one.\"}]},"
"{\"title\":\"Client action row\",\"expanded\":false,\"clientLayout\":\"row\",\"clientGap\":8,\"children\":[{\"type\":\"button\",\"style\":\"button\",\"text\":\"Apply\"},{\"type\":\"button\",\"style\":\"button\",\"text\":\"Reset\"}]}"
"]},"
"{\"type\":\"accordion\",\"style\":\"accentAccordion\",\"sections\":["
"{\"title\":\"Custom colors\",\"expanded\":true,\"children\":[{\"type\":\"label\",\"style\":\"label\",\"text\":\"Header, hover, expanded, client and border colors are configurable.\"}]},"
"{\"title\":\"Custom padding\",\"expanded\":true,\"children\":[{\"type\":\"label\",\"style\":\"label\",\"text\":\"Content padding and gap are applied to client widgets.\"}]}"
"]}"
"]}}";

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
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_accordion_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_accordion_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth == pApp->iLastWidth && iHeight == pApp->iLastHeight ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 500.0f ) {
		fRootH = 500.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_accordion pMulti;
	xge_xui_accordion pSingle;
	float fExpandedHeight;
	float fCollapsedHeight;

	pMulti = &pApp->tPage.arrAccordion[0];
	pSingle = &pApp->tPage.arrAccordion[1];
	fExpandedHeight = xgeXuiAccordionGetClientWidget(pMulti, 0)->tDesiredSize.fY;
	fCollapsedHeight = xgeXuiAccordionGetClientWidget(pMulti, 1)->tDesiredSize.fY;
	pApp->bCreateOK = (pApp->tPage.iAccordionCount == 3) && (pApp->tPage.iLabelCount == 7) && (pApp->tPage.iButtonCount == 2);
	pApp->bLayoutOK = (xgeXuiRoot(&pApp->tXui)->tRect.fW >= 920.0f) && (fExpandedHeight > 0.0f) && (fCollapsedHeight == 0.0f);
	pApp->bStateOK = xgeXuiAccordionIsExpanded(pMulti, 0) &&
		!xgeXuiAccordionIsExpanded(pMulti, 1) &&
		((xgeXuiAccordionGetHeaderWidget(pMulti, 2)->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) &&
		!xgeXuiAccordionIsExpanded(pSingle, 0) &&
		xgeXuiAccordionIsExpanded(pSingle, 1);
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

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_accordion_xson final-summary frames=%d create=%d layout=%d state=%d accordions=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->tPage.iAccordionCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_ACCORDION_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 520;
	tDesc.sTitle = "XUI Accordion XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
