#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_AREA_COUNT 4
#define IMAGE_PER_AREA 9
#define IMAGE_COUNT (IMAGE_AREA_COUNT * IMAGE_PER_AREA)
#define XSON_BUFFER_SIZE 32768

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_texture_t tTexture;
	int bTextureReady;
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

static void MakeImagePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int x;
	int y;

	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			int i = (y * iWidth + x) * 4;
			int bLine = ((x % 8) == 0) || ((y % 8) == 0) || x == 0 || y == 0 || x == iWidth - 1 || y == iHeight - 1;
			pPixels[i + 0] = (unsigned char)(bLine ? 32 : 48 + x * 150 / iWidth);
			pPixels[i + 1] = (unsigned char)(bLine ? 92 : 112 + y * 90 / iHeight);
			pPixels[i + 2] = (unsigned char)(bLine ? 168 : 210);
			pPixels[i + 3] = 255;
		}
	}
}

static int CreateTexture(app_state_t* pApp)
{
	unsigned char arrPixels[96 * 56 * 4];

	MakeImagePixels(arrPixels, 96, 56);
	pApp->bTextureReady = (xgeTextureCreateRGBA(&pApp->tTexture, 96, 56, arrPixels) == XGE_OK);
	return pApp->bTextureReady ? XGE_OK : XGE_ERROR_RESOURCE_FAILED;
}

static const char* AlignX(int iSlot)
{
	static const char* arrText[IMAGE_PER_AREA] = {
		"left", "center", "right",
		"left", "center", "right",
		"left", "center", "right"
	};

	return arrText[iSlot % IMAGE_PER_AREA];
}

static const char* AlignY(int iSlot)
{
	static const char* arrText[IMAGE_PER_AREA] = {
		"top", "top", "top",
		"middle", "middle", "middle",
		"bottom", "bottom", "bottom"
	};

	return arrText[iSlot % IMAGE_PER_AREA];
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

static const char* ImageStyle(int iArea)
{
	if ( iArea == 1 ) {
		return "imageBorder";
	}
	return "image";
}

static int AppendImageNode(char* sBuffer, int* pOffset, int iArea, int iSlot)
{
	if ( iSlot > 0 ) {
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( AppendFormat(
			sBuffer,
			pOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"image\",\"id\":\"image_%d_%d\",\"style\":\"%s\",\"texture\":\"@textures.demo\",\"source\":[8,8,72,48],\"alignX\":\"%s\",\"alignY\":\"%s\"",
			iArea,
			iSlot,
			ImageStyle(iArea),
			AlignX(iSlot),
			AlignY(iSlot)) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( iArea == 2 ) {
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",\"background\":\"#EBF0F7FF\"") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( iArea == 3 ) {
		static const char* arrExtra[IMAGE_PER_AREA] = {
			",\"mode\":\"stretch\",\"background\":\"#E8F0FCFF\",\"borderColor\":\"#C6D0DEFF\",\"borderWidth\":1",
			",\"mode\":\"contain\",\"background\":\"#EFF7F4FF\",\"borderColor\":\"#C6D0DEFF\",\"borderWidth\":1",
			",\"mode\":\"cover\",\"background\":\"#F8F0ECFF\",\"borderColor\":\"#C6D0DEFF\",\"borderWidth\":1",
			",\"mode\":\"scaleDown\",\"background\":\"#F6F8FBFF\",\"borderColor\":\"#C6D0DEFF\",\"borderWidth\":1",
			",\"customRect\":[14,10,74,52],\"background\":\"#FFF8E6FF\",\"borderColor\":\"#D49626FF\",\"borderWidth\":2",
			",\"source\":[0,0,96,56],\"background\":\"#ECF7FAFF\",\"borderColor\":\"#C6D0DEFF\",\"borderWidth\":1",
			",\"tint\":\"#78BEFFDC\",\"background\":\"#E8EEFFFF\",\"borderColor\":\"#5C74D2FF\",\"borderWidth\":1",
			",\"enabled\":false,\"background\":\"#EEF0F4FF\",\"borderColor\":\"#C6D0DEFF\",\"borderWidth\":1",
			",\"texture\":null,\"background\":\"#FAF4F6FF\",\"borderColor\":\"#D2A0B0FF\",\"borderWidth\":1"
		};
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, arrExtra[iSlot]) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, "}");
}

static int BuildXson(app_state_t* pApp)
{
	int iOffset;
	int i;
	int j;

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
			"\"image\":{\"width\":\"100%\",\"height\":\"100%\",\"margin\":4,\"padding\":6},"
			"\"imageBorder\":{\"width\":\"100%\",\"height\":\"100%\",\"margin\":4,\"padding\":6,\"borderColor\":\"#4A7EB8FF\",\"borderWidth\":1}"
			"},"
			"\"tree\":{\"type\":\"grid\",\"id\":\"root\",\"style\":\"root\",\"children\":[") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < IMAGE_AREA_COUNT; i++ ) {
		if ( i > 0 ) {
			if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
		if ( AppendFormat(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "{\"type\":\"grid\",\"id\":\"area%d\",\"style\":\"%s\",\"children\":[", i, AreaStyle(i)) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		for ( j = 0; j < IMAGE_PER_AREA; j++ ) {
			if ( AppendImageNode(pApp->sXson, &iOffset, i, j) != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
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
	XgeXuiDemoApplyTheme(&pApp->tXui, NULL);
	if ( xgeXuiTokenSetTexture(&pApp->tXui, "demo", &pApp->tTexture) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( BuildXson(pApp) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, pApp->sXson, (int)strlen(pApp->sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_image_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
	xgeXuiWidgetSetGrid(pRoot, 2, fAreaH, 12.0f, 12.0f);
	fCellH = (fAreaH - 20.0f - 16.0f) / 3.0f;
	for ( i = 0; i < IMAGE_AREA_COUNT; i++ ) {
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
	xge_xui_widget pArea;
	xge_xui_widget pImage;
	xge_xui_image pControl;

	pArea = xgeXuiPageFind(&pApp->tPage, "area0");
	pImage = xgeXuiPageFind(&pApp->tPage, "image_3_4");
	pControl = (pImage != NULL) ? (xge_xui_image)pImage->pUser : NULL;
	pApp->bCreateOK = pApp->bTextureReady && (pApp->tPage.iImageCount == IMAGE_COUNT);
	pApp->bLayoutOK = (pArea != NULL) && (pArea->tRect.fW > 200.0f);
	pApp->bStateOK = (pControl != NULL) && (pControl->iMode == XGE_XUI_IMAGE_CUSTOM);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( CreateTexture(pApp) != XGE_OK || xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
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
	if ( pApp->bTextureReady ) {
		xgeTextureFree(&pApp->tTexture);
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
		printf("xui_image_xson final-summary frames=%d create=%d layout=%d state=%d images=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, IMAGE_COUNT);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_IMAGE_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 680;
	tDesc.sTitle = "XUI Image XSON";
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
