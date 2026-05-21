#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSON_BUFFER_SIZE 65536
#define XSON_GROUPS 5
#define XSON_CHILDREN 18

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
	int bTextOK;
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
			printf("xui_treeview_xson font loaded: %s\n", arrFonts[i]);
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

static int AppendFormat(char* sBuffer, int* pOffset, int iCapacity, const char* sFormat, ...)
{
	va_list args;
	int iWritten;

	if ( (sBuffer == NULL) || (pOffset == NULL) || (sFormat == NULL) || (*pOffset >= iCapacity) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	va_start(args, sFormat);
	iWritten = vsnprintf(sBuffer + *pOffset, (size_t)(iCapacity - *pOffset), sFormat, args);
	va_end(args);
	if ( (iWritten < 0) || ((*pOffset + iWritten) >= iCapacity) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*pOffset += iWritten;
	return XGE_OK;
}

static int AppendNode(char* sBuffer, int* pOffset, int* pFirst, int iId, int iParent, const char* sText, int bExpanded, int bEnabled, int bIcon, int bCheck, int bChecked)
{
	if ( *pFirst == 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*pFirst = 0;
	if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "{\"id\":%d", iId) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( iParent >= 0 && AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"parent\":%d", iParent) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"text\":\"%s\"", sText) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( bExpanded && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"expanded\":true") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( bEnabled == 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"enabled\":false") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( bIcon == 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"icon\":false") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( bCheck && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"check\":true") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( bChecked && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"checked\":true") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	return AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, "}");
}

static int AppendTreeStart(char* sBuffer, int* pOffset, const char* sPanelId, const char* sTitle, const char* sTreeId, const char* sStyle, int iSelected, float fScrollY, const char* sMode)
{
	if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "{\"type\":\"column\",\"id\":\"%s\",\"style\":\"panel\",\"children\":[{\"type\":\"label\",\"style\":\"title\",\"text\":\"%s\"},{\"type\":\"treeView\",\"id\":\"%s\",\"style\":\"%s\",\"selected\":%d", sPanelId, sTitle, sTreeId, sStyle, iSelected) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( fScrollY > 0.0f && AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"scrollY\":%.1f", fScrollY) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( (sMode != NULL) && AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"scrollbarMode\":\"%s\"", sMode) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	return AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"nodes\":[");
}

static int AppendTreeEnd(char* sBuffer, int* pOffset)
{
	return AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, "]}]}");
}

static int BuildXson(app_state_t* pApp)
{
	int iOffset;
	int iFirst;
	int i;
	int j;
	int iGroupId;
	int iNodeId;
	char sText[80];

	iOffset = 0;
	pApp->sXson[0] = 0;
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"xui\":1,"
			"\"styles\":{"
			"\"root\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":18,\"background\":\"#ECF0F6FF\",\"columnGap\":12,\"rowGap\":12},"
			"\"panel\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[12,10,12,12],\"gap\":8,\"background\":\"#F8FAFDFF\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1,\"radius\":4},"
			"\"title\":{\"height\":28,\"textColor\":\"#2A3A4EFF\"},"
			"\"tree\":{\"width\":\"100%\",\"height\":\"grow\",\"itemHeight\":24,\"indent\":18,\"backgroundColor\":\"#F8FAFDFF\",\"rowColor\":\"#F6F9FCFF\",\"selectedColor\":\"#D1E6FAFF\",\"textColor\":\"#2A3442FF\",\"disabledTextColor\":\"#87909DB8\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#4A8ED2FF\"},"
			"\"largeTree\":{\"width\":\"100%\",\"height\":\"grow\",\"itemHeight\":22,\"indent\":18,\"backgroundColor\":\"#F8FAFDFF\",\"rowColor\":\"#F7FAFCFF\",\"selectedColor\":\"#D9EBFAFF\",\"textColor\":\"#2A3442FF\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#4A8ED2FF\"},"
			"\"accentTree\":{\"width\":\"100%\",\"height\":\"grow\",\"itemHeight\":26,\"indent\":20,\"backgroundColor\":\"#F8FAFDFF\",\"rowColor\":\"#F2F7F7FF\",\"selectedColor\":\"#CDEDE5FF\",\"textColor\":\"#243442FF\",\"disabledTextColor\":\"#8A94A0B8\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#43A77CFF\",\"expanderColor\":\"#2870A8FF\",\"iconColor\":\"#3E8F70FF\",\"checkColor\":\"#32845FFF\"}"
			"},\"tree\":{\"type\":\"grid\",\"id\":\"root\",\"style\":\"root\",\"children\":[") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	if ( AppendTreeStart(pApp->sXson, &iOffset, "panel_basic", "Basic tree through XSON", "basic", "tree", 30, 0.0f, NULL) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iFirst = 1;
	if ( AppendNode(pApp->sXson, &iOffset, &iFirst, 10, -1, "Project", 1, 1, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 20, 10, "src", 1, 1, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 30, 20, "xge_xui_tree_view.c", 0, 1, 1, 1, 1) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 40, 20, "disabled source file", 0, 0, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 50, 10, "examples", 0, 1, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 60, 50, "xui_treeview_xson", 0, 1, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 70, 10, "docs no icon", 0, 1, 0, 0, 0) != XGE_OK ||
	     AppendTreeEnd(pApp->sXson, &iOffset) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	if ( AppendTreeStart(pApp->sXson, &iOffset, "panel_large", "Large tree: full scrollbar and scrollY", "large", "largeTree", 2034, 540.0f, "full") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iFirst = 1;
	if ( AppendNode(pApp->sXson, &iOffset, &iFirst, 1000, -1, "Large data root", 1, 1, 1, 0, 0) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < XSON_GROUPS; i++ ) {
		iGroupId = 1100 + i;
		snprintf(sText, sizeof(sText), "Group %02d", i + 1);
		if ( AppendNode(pApp->sXson, &iOffset, &iFirst, iGroupId, 1000, sText, 1, 1, 1, 0, 0) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		for ( j = 0; j < XSON_CHILDREN; j++ ) {
			iNodeId = 2000 + i * XSON_CHILDREN + j;
			snprintf(sText, sizeof(sText), "Declared tree item %03d", i * XSON_CHILDREN + j + 1);
			if ( AppendNode(pApp->sXson, &iOffset, &iFirst, iNodeId, iGroupId, sText, 0, 1, 1, (j % 8) == 0, (j % 8) == 0) != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
	}
	if ( AppendTreeEnd(pApp->sXson, &iOffset) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	if ( AppendTreeStart(pApp->sXson, &iOffset, "panel_checks", "Checkbox, disabled, and no-icon rows", "checks", "accentTree", 3010, 0.0f, NULL) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iFirst = 1;
	if ( AppendNode(pApp->sXson, &iOffset, &iFirst, 3000, -1, "Settings", 1, 1, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 3010, 3000, "Checked option", 0, 1, 1, 1, 1) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 3020, 3000, "Unchecked option", 0, 1, 1, 1, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 3030, 3000, "Disabled option", 0, 0, 1, 1, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 3040, 3000, "No icon row", 0, 1, 0, 0, 0) != XGE_OK ||
	     AppendTreeEnd(pApp->sXson, &iOffset) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	if ( AppendTreeStart(pApp->sXson, &iOffset, "panel_styled", "Styled tree: accent colors", "styled", "accentTree", 4020, 0.0f, NULL) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iFirst = 1;
	if ( AppendNode(pApp->sXson, &iOffset, &iFirst, 4000, -1, "Styled root", 1, 1, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 4010, 4000, "Branch A", 1, 1, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 4020, 4010, "Selected accent child", 0, 1, 1, 1, 1) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 4030, 4010, "Disabled accent child", 0, 0, 1, 0, 0) != XGE_OK ||
	     AppendNode(pApp->sXson, &iOffset, &iFirst, 4040, 4000, "Branch B", 0, 1, 1, 0, 0) != XGE_OK ||
	     AppendTreeEnd(pApp->sXson, &iOffset) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}}") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	return XGE_OK;
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
		printf("xui_treeview_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	float fPanelH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	if ( pRoot == NULL ) {
		return;
	}
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 960.0f ) {
		fRootW = 960.0f;
	}
	if ( fRootH < 680.0f ) {
		fRootH = 680.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	fPanelH = (fRootH - 36.0f - 12.0f) * 0.5f;
	xgeXuiWidgetSetGrid(pRoot, 2, fPanelH, 12.0f, 12.0f);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pBasic;
	xge_xui_widget pLarge;
	xge_xui_widget pChecks;
	xge_xui_widget pStyled;

	pBasic = xgeXuiPageFind(&pApp->tPage, "basic");
	pLarge = xgeXuiPageFind(&pApp->tPage, "large");
	pChecks = xgeXuiPageFind(&pApp->tPage, "checks");
	pStyled = xgeXuiPageFind(&pApp->tPage, "styled");
	pApp->bCreateOK =
		(pBasic != NULL) && (pBasic->procEvent == xgeXuiTreeViewEventProc) &&
		(pLarge != NULL) && (pLarge->procEvent == xgeXuiTreeViewEventProc) &&
		(pChecks != NULL) && (pChecks->procEvent == xgeXuiTreeViewEventProc) &&
		(pStyled != NULL) && (pStyled->procEvent == xgeXuiTreeViewEventProc) &&
		(pApp->tPage.iTreeViewCount == 4) &&
		(pApp->tPage.iLabelCount == 4);
	pApp->bLayoutOK = (pBasic != NULL) && (pBasic->tRect.fW > 300.0f) && (pStyled != NULL) && (pStyled->tRect.fH > 180.0f);
	pApp->bStateOK =
		(pApp->tPage.iTreeViewCount == 4) &&
		(xgeXuiTreeViewGetSelected(&pApp->tPage.arrTreeView[0]) == 30) &&
		(xgeXuiTreeViewGetVisibleCount(&pApp->tPage.arrTreeView[0]) == 6) &&
		(xgeXuiTreeViewGetNodeChecked(&pApp->tPage.arrTreeView[0], 30) == 1) &&
		(xgeXuiTreeViewGetNodeEnabled(&pApp->tPage.arrTreeView[0], 40) == 0) &&
		(xgeXuiTreeViewGetScrollbarMode(&pApp->tPage.arrTreeView[1]) == XGE_XUI_SCROLLBAR_MODE_FULL) &&
		(xgeXuiTreeViewGetScroll(&pApp->tPage.arrTreeView[1]) > 0.0f) &&
		(xgeXuiTreeViewGetFirstVisible(&pApp->tPage.arrTreeView[1]) > 0) &&
		(xgeXuiTreeViewGetNodeChecked(&pApp->tPage.arrTreeView[2], 3010) == 1) &&
		(xgeXuiTreeViewGetNodeEnabled(&pApp->tPage.arrTreeView[2], 3030) == 0) &&
		(xgeXuiTreeViewGetSelected(&pApp->tPage.arrTreeView[3]) == 4020);
	pApp->bTextOK =
		pApp->bStateOK &&
		(pApp->tPage.arrTreeView[0].pFont != NULL) &&
		(pApp->tPage.arrTreeView[1].pFont != NULL) &&
		(pApp->tPage.arrTreeView[2].pFont != NULL) &&
		(pApp->tPage.arrTreeView[3].pFont != NULL) &&
		(pApp->tPage.arrLabel[0].pFont != NULL) &&
		(pApp->tPage.arrTreeView[0].iNodeCount > 0) &&
		(pApp->tPage.arrTreeView[0].arrNodes[0].sText != NULL) &&
		(XGE_COLOR_GET_A(pApp->tPage.arrTreeView[0].iTextColor) != 0);
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
		printf("xui_treeview_xson final-summary frames=%d create=%d layout=%d state=%d text=%d treeCount=%d firstLarge=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->bTextOK,
			pApp->tPage.iTreeViewCount,
			xgeXuiTreeViewGetFirstVisible(&pApp->tPage.arrTreeView[1]));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TREEVIEW_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 980;
	tDesc.iHeight = 720;
	tDesc.sTitle = "XUI TreeView XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK && tApp.bTextOK) ? 0 : 3;
}
