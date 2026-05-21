#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_AREA_COUNT 4
#define LABEL_PER_AREA 9
#define LABEL_COUNT (LABEL_AREA_COUNT * LABEL_PER_AREA)
#define XSON_BUFFER_SIZE 32768

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	xge_font_t tFontSmall;
	xge_font_t tFontLarge;
	int bFontReady;
	int bSmallFontReady;
	int bLargeFontReady;
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

static int LoadFontOne(xge_font pFont, float fSize)
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
		if ( xgeFontLoad(pFont, arrFonts[i], fSize) == XGE_OK ) {
			printf("xui_label_xson font loaded: %s size=%.1f\n", arrFonts[i], fSize);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LoadFonts(app_state_t* pApp)
{
	pApp->bFontReady = (LoadFontOne(&pApp->tFont, XGE_XUI_DEMO_FONT_SIZE) == XGE_OK);
	pApp->bSmallFontReady = (LoadFontOne(&pApp->tFontSmall, 10.0f) == XGE_OK);
	pApp->bLargeFontReady = (LoadFontOne(&pApp->tFontLarge, 18.0f) == XGE_OK);
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static xge_font AppSmallFont(app_state_t* pApp)
{
	return pApp->bSmallFontReady ? &pApp->tFontSmall : AppFont(pApp);
}

static xge_font AppLargeFont(app_state_t* pApp)
{
	return pApp->bLargeFontReady ? &pApp->tFontLarge : AppFont(pApp);
}

static const char* AlignX(int iIndex)
{
	static const char* arrText[LABEL_PER_AREA] = {
		"left", "center", "right",
		"left", "center", "right",
		"left", "center", "right"
	};

	return arrText[iIndex % LABEL_PER_AREA];
}

static const char* AlignY(int iIndex)
{
	static const char* arrText[LABEL_PER_AREA] = {
		"top", "top", "top",
		"middle", "middle", "middle",
		"bottom", "bottom", "bottom"
	};

	return arrText[iIndex % LABEL_PER_AREA];
}

static const char* AlignText(int iIndex)
{
	static const char* arrText[LABEL_PER_AREA] = {
		"Left Top",
		"Center Top",
		"Right Top",
		"Left Middle",
		"Center Middle",
		"Right Middle",
		"Left Bottom",
		"Center Bottom",
		"Right Bottom"
	};

	return arrText[iIndex % LABEL_PER_AREA];
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

static const char* AreaStyle(int iArea)
{
	return (iArea == 2) ? "areaAlt" : "area";
}

static const char* LabelStyle(int iArea)
{
	if ( iArea == 1 ) {
		return "labelBorder";
	}
	if ( iArea == 2 ) {
		return "label";
	}
	return "label";
}

static const char* Area2Background(int iSlot)
{
	static const char* arrBg[LABEL_PER_AREA] = {
		"#EAF2FFFF", "#E9F8EFFF", "#FFF3D8FF",
		"#F3EAFBFF", "#FFECECFF", "#E8F6FAFF",
		"#F4F6FAFF", "#EFF2E7FF", "#F8EEF7FF"
	};

	return arrBg[iSlot % LABEL_PER_AREA];
}

static int AppendLabelNode(char* sBuffer, int* pOffset, int iArea, int iSlot, const char* sText, const char* sFont, const char* sTextColor)
{
	int iIndex;

	iIndex = (iArea * LABEL_PER_AREA) + iSlot;
	(void)iIndex;
	if ( iSlot > 0 ) {
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( AppendFormat(
			sBuffer,
			pOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"label\",\"id\":\"label_%d_%d\",\"style\":\"%s\",\"text\":\"%s\",\"font\":\"%s\",\"textColor\":\"%s\",\"disabledColor\":\"#828A96FF\",\"textAlign\":\"%s\",\"textVAlign\":\"%s\"",
			iArea,
			iSlot,
			LabelStyle(iArea),
			sText,
			sFont,
			sTextColor,
			AlignX(iSlot),
			AlignY(iSlot)) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( iArea == 2 ) {
		if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"background\":\"%s\"", Area2Background(iSlot)) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( (iArea == 3) && (iSlot == 2) ) {
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"enabled\":false") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( (iArea == 3) && (iSlot == 3) ) {
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"underline\":true") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( (iArea == 3) && (iSlot == 4) ) {
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"background\":\"#E5F0FFFF\",\"borderColor\":\"#2A78D2FF\",\"borderWidth\":1") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( (iArea == 3) && (iSlot == 7) ) {
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"background\":\"#F8F4ECFF\",\"padding\":[12,10,12,10]") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, "}");
}

static int BuildXson(app_state_t* pApp)
{
	static const char* arrSpecialText[LABEL_PER_AREA] = {
		"Small font",
		"Large font",
		"Disabled",
		"Underline",
		"Border + fill",
		"Direct draw",
		"Two lines\\ncentered",
		"Padded text",
		"Accent text"
	};
	static const char* arrSpecialFont[LABEL_PER_AREA] = {
		"@fonts.small", "@fonts.large", "@fonts.body",
		"@fonts.body", "@fonts.body", "@fonts.body",
		"@fonts.body", "@fonts.body", "@fonts.body"
	};
	static const char* arrSpecialColor[LABEL_PER_AREA] = {
		"#222A38FF", "#222A38FF", "#222A38FF",
		"#1A60B0FF", "#263A52FF", "#603812FF",
		"#242A34FF", "#242A34FF", "#1470D2FF"
	};
	int iOffset;
	int i;

	iOffset = 0;
	pApp->sXson[0] = 0;
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"xui\":1,"
			"\"styles\":{"
			"\"root\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":18,\"background\":\"#E5EBF4FF\",\"columnGap\":12,\"rowGap\":12},"
			"\"area\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":10,\"background\":\"#F8FAFDFF\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1,\"radius\":4,\"columnGap\":8,\"rowGap\":8},"
			"\"areaAlt\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":10,\"background\":\"#E7EEF7FF\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1,\"radius\":4,\"columnGap\":8,\"rowGap\":8},"
			"\"label\":{\"width\":\"100%\",\"height\":\"100%\",\"margin\":4,\"padding\":[7,6,7,6]},"
			"\"labelBorder\":{\"width\":\"100%\",\"height\":\"100%\",\"margin\":4,\"padding\":[7,6,7,6],\"borderColor\":\"#4A7EB8FF\",\"borderWidth\":1}"
			"},"
			"\"tree\":{\"type\":\"grid\",\"id\":\"root\",\"style\":\"root\",\"children\":[") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		if ( i > 0 ) {
			if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
		if ( AppendFormat(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "{\"type\":\"grid\",\"id\":\"area%d\",\"style\":\"%s\",\"children\":[", i, AreaStyle(i)) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( i == 0 ) {
			for ( int j = 0; j < LABEL_PER_AREA; j++ ) {
				if ( AppendLabelNode(pApp->sXson, &iOffset, 0, j, AlignText(j), "@fonts.body", "#242A34FF") != XGE_OK ) {
					return XGE_ERROR_OUT_OF_MEMORY;
				}
			}
		} else if ( i == 1 ) {
			for ( int j = 0; j < LABEL_PER_AREA; j++ ) {
				if ( AppendLabelNode(pApp->sXson, &iOffset, 1, j, AlignText(j), "@fonts.body", "#203E60FF") != XGE_OK ) {
					return XGE_ERROR_OUT_OF_MEMORY;
				}
			}
		} else if ( i == 2 ) {
			for ( int j = 0; j < LABEL_PER_AREA; j++ ) {
				if ( AppendLabelNode(pApp->sXson, &iOffset, 2, j, AlignText(j), "@fonts.body", "#222E3EFF") != XGE_OK ) {
					return XGE_ERROR_OUT_OF_MEMORY;
				}
			}
		} else {
			for ( int j = 0; j < LABEL_PER_AREA; j++ ) {
				if ( AppendLabelNode(pApp->sXson, &iOffset, 3, j, arrSpecialText[j], arrSpecialFont[j], arrSpecialColor[j]) != XGE_OK ) {
					return XGE_ERROR_OUT_OF_MEMORY;
				}
			}
		}
		if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}}");
}

static int CreateUI(app_state_t* pApp)
{
	xge_font pFont;

	pFont = AppFont(pApp);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pFont) != XGE_OK ||
		xgeXuiTokenSetFont(&pApp->tXui, "small", AppSmallFont(pApp)) != XGE_OK ||
		xgeXuiTokenSetFont(&pApp->tXui, "large", AppLargeFont(pApp)) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( BuildXson(pApp) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, pApp->sXson, (int)strlen(pApp->sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_label_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pArea;
	float fRootW;
	float fRootH;
	float fAreaH;
	float fCellH;
	int iWidth;
	int iHeight;
	int i;

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
	if ( fRootW < 760.0f ) {
		fRootW = 760.0f;
	}
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	fAreaH = (fRootH - 36.0f - 12.0f) * 0.5f;
	if ( fAreaH < 240.0f ) {
		fAreaH = 240.0f;
	}
	xgeXuiWidgetSetGrid(pRoot, 2, fAreaH, 12.0f, 12.0f);
	fCellH = (fAreaH - 20.0f - 16.0f) / 3.0f;
	if ( fCellH < 54.0f ) {
		fCellH = 54.0f;
	}
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		char sId[16];
		snprintf(sId, sizeof(sId), "area%d", i);
		pArea = xgeXuiPageFind(&pApp->tPage, sId);
		if ( pArea != NULL ) {
			xgeXuiWidgetSetGrid(pArea, 3, fCellH, 8.0f, 8.0f);
		}
	}
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;
	int i;

	pApp->bCreateOK = (pApp->tPage.iLabelCount == LABEL_COUNT);
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		char sId[16];
		snprintf(sId, sizeof(sId), "area%d", i);
		if ( xgeXuiPageFind(&pApp->tPage, sId) == NULL ) {
			pApp->bCreateOK = 0;
		}
	}
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		char sId[24];
		snprintf(sId, sizeof(sId), "label_%d_%d", i / LABEL_PER_AREA, i % LABEL_PER_AREA);
		pWidget = xgeXuiPageFind(&pApp->tPage, sId);
		if ( (pWidget == NULL) || (pWidget->pUser == NULL) || (pWidget->procPaint != xgeXuiLabelPaintProc) ) {
			pApp->bCreateOK = 0;
		}
	}
	pWidget = xgeXuiPageFind(&pApp->tPage, "area0");
	pApp->bLayoutOK =
		(pWidget != NULL) &&
		(pWidget->tRect.fW > 200.0f) &&
		(xgeXuiPageFind(&pApp->tPage, "area3") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "area3")->tRect.fH > 200.0f);
	pWidget = xgeXuiPageFind(&pApp->tPage, "label_3_3");
	pLabel = (pWidget != NULL) ? (xge_xui_label)pWidget->pUser : NULL;
	pApp->bStateOK = (pLabel != NULL) && (pLabel->bUnderline != 0);
	pWidget = xgeXuiPageFind(&pApp->tPage, "label_3_2");
	pApp->bStateOK = pApp->bStateOK && (pWidget != NULL) && (xgeXuiWidgetIsEnabled(pWidget) == 0);
	pWidget = xgeXuiPageFind(&pApp->tPage, "label_1_0");
	pApp->bStateOK = pApp->bStateOK && (pWidget != NULL) && (pWidget->tStyle.fBorderWidth > 0.0f);
	pWidget = xgeXuiPageFind(&pApp->tPage, "label_2_0");
	pApp->bStateOK = pApp->bStateOK && (pWidget != NULL) && (XGE_COLOR_GET_A(pWidget->tStyle.iBackgroundColor) != 0);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFonts(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
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
	if ( pApp->bLargeFontReady ) {
		xgeFontFree(&pApp->tFontLarge);
	}
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
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui_label_xson final-summary frames=%d create=%d layout=%d state=%d labels=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->tPage.iLabelCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_LABEL_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 680;
	tDesc.sTitle = "XUI Label XSON";
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
