#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../xge.h"

static const unsigned char g_tgaImage[] = {
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 32, 40,
	40, 80, 220, 255, 60, 210, 80, 220, 220, 120, 60, 180, 240, 240, 240, 255
};

typedef struct async_assets_lab_t {
	xge_async_request_t tImageRequest;
	xge_async_request_t tTextureRequest;
	xge_async_request_t tFontRequest;
	xge_async_request_t tSoundRequest;
	xge_async_request_t tCancelRequest;
	xge_image_t tImage;
	xge_texture_t tTexture;
	xge_texture_t tCancelledTexture;
	xge_font_t tFont;
	xge_sound_t tSound;
	char sImagePath[260];
	char sSoundPath[260];
	char sFontPath[260];
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bResourcesOK;
	int bThreadingOffOK;
	int bThreadingOnOK;
	int bImageLoadOK;
	int bTextureLoadOK;
	int bFontLoadOK;
	int bSoundLoadOK;
	int bCancelStartOK;
	int bCancelOK;
	int bPollOK;
	int bMainThreadModeOK;
	int bThreadedModeOK;
	int bImageReady;
	int bTextureReady;
	int bFontReady;
	int bSoundReady;
	int bSoundSkipped;
	int bCancelReady;
	int iImageStatus;
	int iTextureStatus;
	int iFontStatus;
	int iSoundStatus;
	int iCancelStatus;
	int iImageCallbacks;
	int iTextureCallbacks;
	int iFontCallbacks;
	int iSoundCallbacks;
	int iCancelCallbacks;
} async_assets_lab_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static double ArgDouble(const char* sText, double fDefault)
{
	double fValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return fDefault;
	}
	fValue = atof(sText);
	return (fValue > 0.0) ? fValue : fDefault;
}

static int WriteTGA(const char* sPath)
{
	FILE* pFile;

	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) {
		return 0;
	}
	if ( fwrite(g_tgaImage, 1, sizeof(g_tgaImage), pFile) != sizeof(g_tgaImage) ) {
		fclose(pFile);
		return 0;
	}
	fclose(pFile);
	return 1;
}

static int WriteWAV(const char* sPath)
{
	FILE* pFile;
	unsigned char arrHeader[44];
	enum { XGE_SAMPLE_RATE = 22050, XGE_SAMPLE_COUNT = 5512 };
	int16_t arrSamples[XGE_SAMPLE_COUNT];
	int i;
	uint32_t iDataBytes;
	uint32_t iFileBytes;

	for ( i = 0; i < XGE_SAMPLE_COUNT; i++ ) {
		int iPhase;
		int iValue;

		iPhase = (i / 32) & 1;
		iValue = iPhase ? 14000 : -14000;
		arrSamples[i] = (int16_t)iValue;
	}
	iDataBytes = (uint32_t)(sizeof(arrSamples));
	iFileBytes = 36u + iDataBytes;
	memset(arrHeader, 0, sizeof(arrHeader));
	memcpy(arrHeader + 0, "RIFF", 4);
	arrHeader[4] = (unsigned char)(iFileBytes & 0xFFu);
	arrHeader[5] = (unsigned char)((iFileBytes >> 8) & 0xFFu);
	arrHeader[6] = (unsigned char)((iFileBytes >> 16) & 0xFFu);
	arrHeader[7] = (unsigned char)((iFileBytes >> 24) & 0xFFu);
	memcpy(arrHeader + 8, "WAVEfmt ", 8);
	arrHeader[16] = 16;
	arrHeader[20] = 1;
	arrHeader[22] = 1;
	arrHeader[24] = (unsigned char)(XGE_SAMPLE_RATE & 0xFF);
	arrHeader[25] = (unsigned char)((XGE_SAMPLE_RATE >> 8) & 0xFF);
	arrHeader[26] = (unsigned char)((XGE_SAMPLE_RATE >> 16) & 0xFF);
	arrHeader[27] = (unsigned char)((XGE_SAMPLE_RATE >> 24) & 0xFF);
	arrHeader[28] = (unsigned char)((XGE_SAMPLE_RATE * 2) & 0xFF);
	arrHeader[29] = (unsigned char)(((XGE_SAMPLE_RATE * 2) >> 8) & 0xFF);
	arrHeader[30] = (unsigned char)(((XGE_SAMPLE_RATE * 2) >> 16) & 0xFF);
	arrHeader[31] = (unsigned char)(((XGE_SAMPLE_RATE * 2) >> 24) & 0xFF);
	arrHeader[32] = 2;
	arrHeader[34] = 16;
	memcpy(arrHeader + 36, "data", 4);
	arrHeader[40] = (unsigned char)(iDataBytes & 0xFFu);
	arrHeader[41] = (unsigned char)((iDataBytes >> 8) & 0xFFu);
	arrHeader[42] = (unsigned char)((iDataBytes >> 16) & 0xFFu);
	arrHeader[43] = (unsigned char)((iDataBytes >> 24) & 0xFFu);
	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) {
		return 0;
	}
	if ( fwrite(arrHeader, 1, sizeof(arrHeader), pFile) != sizeof(arrHeader) ) {
		fclose(pFile);
		return 0;
	}
	if ( fwrite(arrSamples, 1, sizeof(arrSamples), pFile) != sizeof(arrSamples) ) {
		fclose(pFile);
		return 0;
	}
	fclose(pFile);
	return 1;
}

static int ChooseFontPath(char* sPath, int iSize)
{
#if defined(_WIN32)
	static const char* arrCandidates[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/segoeui.ttf",
		"C:/Windows/Fonts/arial.ttf"
	};
#else
	static const char* arrCandidates[] = {
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf"
	};
#endif
	FILE* pFile;
	int i;

	if ( (sPath == NULL) || (iSize <= 0) ) {
		return 0;
	}
	for ( i = 0; i < (int)(sizeof(arrCandidates) / sizeof(arrCandidates[0])); i++ ) {
		pFile = fopen(arrCandidates[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			snprintf(sPath, iSize, "%s", arrCandidates[i]);
			return 1;
		}
	}
	sPath[0] = 0;
	return 0;
}

static void AsyncComplete(xge_async_request pRequest, void* pUser)
{
	async_assets_lab_t* pLab;

	pLab = (async_assets_lab_t*)pUser;
	if ( pRequest == &pLab->tImageRequest ) {
		pLab->iImageCallbacks++;
	} else if ( pRequest == &pLab->tTextureRequest ) {
		pLab->iTextureCallbacks++;
	} else if ( pRequest == &pLab->tFontRequest ) {
		pLab->iFontCallbacks++;
	} else if ( pRequest == &pLab->tSoundRequest ) {
		pLab->iSoundCallbacks++;
	} else if ( pRequest == &pLab->tCancelRequest ) {
		pLab->iCancelCallbacks++;
	}
}

static int BeginRequests(async_assets_lab_t* pLab)
{
	int iRet;
	int bFontFound;

	snprintf(pLab->sImagePath, sizeof(pLab->sImagePath), "%s", "async_assets_lab_image.tga");
	snprintf(pLab->sSoundPath, sizeof(pLab->sSoundPath), "%s", "async_assets_lab_tone.wav");
	pLab->bResourcesOK = WriteTGA("build/async_assets_lab_image.tga") && WriteWAV("build/async_assets_lab_tone.wav");
	bFontFound = ChooseFontPath(pLab->sFontPath, (int)sizeof(pLab->sFontPath));
	if ( !pLab->bResourcesOK ) {
		fprintf(stderr, "async-assets-lab stage failed: generated resources\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}

	xgeAsyncRequestInit(&pLab->tImageRequest);
	xgeAsyncRequestInit(&pLab->tTextureRequest);
	xgeAsyncRequestInit(&pLab->tFontRequest);
	xgeAsyncRequestInit(&pLab->tSoundRequest);
	xgeAsyncRequestInit(&pLab->tCancelRequest);

	pLab->bThreadingOffOK = (xgeAsyncThreadingSet(0) == XGE_OK) && (xgeAsyncThreadingGet() == 0);
	iRet = xgeAsyncImageLoad(&pLab->tImageRequest, &pLab->tImage, pLab->sImagePath, 0, AsyncComplete, pLab);
	pLab->bImageLoadOK = (iRet == XGE_OK);
	iRet = xgeAsyncTextureLoad(&pLab->tTextureRequest, &pLab->tTexture, pLab->sImagePath, 0, AsyncComplete, pLab);
	pLab->bTextureLoadOK = (iRet == XGE_OK);
	pLab->bMainThreadModeOK = pLab->bImageLoadOK && pLab->bTextureLoadOK &&
		(pLab->tImageRequest.bThreaded == 0) && (pLab->tTextureRequest.bThreaded == 0);

	pLab->bThreadingOnOK = (xgeAsyncThreadingSet(1) == XGE_OK) && (xgeAsyncThreadingGet() == 1);
	iRet = xgeAsyncFontLoad(&pLab->tFontRequest, &pLab->tFont, bFontFound ? pLab->sFontPath : "missing_async_assets_lab_font.ttf", 22.0f, AsyncComplete, pLab);
	pLab->bFontLoadOK = (iRet == XGE_OK);
	iRet = xgeAsyncSoundLoad(&pLab->tSoundRequest, &pLab->tSound, pLab->sSoundPath, AsyncComplete, pLab);
	pLab->bSoundLoadOK = (iRet == XGE_OK);
	iRet = xgeAsyncTextureLoad(&pLab->tCancelRequest, &pLab->tCancelledTexture, pLab->sImagePath, 0, AsyncComplete, pLab);
	pLab->bCancelStartOK = (iRet == XGE_OK);
	pLab->bCancelOK = pLab->bCancelStartOK && (xgeAsyncRequestCancel(&pLab->tCancelRequest) == XGE_OK);
	pLab->bThreadedModeOK = pLab->bFontLoadOK && pLab->bSoundLoadOK && pLab->bCancelStartOK &&
		(pLab->tFontRequest.bThreaded != 0) && (pLab->tSoundRequest.bThreaded != 0) && (pLab->tCancelRequest.bThreaded != 0);

	printf("async-assets-lab init resources=%d threading=%d/%d loads=%d/%d/%d/%d cancel=%d font=%s\n",
		pLab->bResourcesOK,
		pLab->bThreadingOffOK,
		pLab->bThreadingOnOK,
		pLab->bImageLoadOK,
		pLab->bTextureLoadOK,
		pLab->bFontLoadOK,
		pLab->bSoundLoadOK,
		pLab->bCancelOK,
		bFontFound ? pLab->sFontPath : "missing");
	return XGE_OK;
}

static int InitLab(async_assets_lab_t* pLab)
{
	if ( pLab->bReady ) {
		return XGE_OK;
	}
	if ( BeginRequests(pLab) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->bReady = 1;
	return XGE_OK;
}

static void PollRequests(async_assets_lab_t* pLab)
{
	pLab->iImageStatus = xgeAsyncPoll(&pLab->tImageRequest);
	pLab->iTextureStatus = xgeAsyncPoll(&pLab->tTextureRequest);
	pLab->iFontStatus = xgeAsyncPoll(&pLab->tFontRequest);
	pLab->iSoundStatus = xgeAsyncPoll(&pLab->tSoundRequest);
	pLab->iCancelStatus = xgeAsyncPoll(&pLab->tCancelRequest);
	pLab->bPollOK = 1;
	pLab->bImageReady = (pLab->iImageStatus == XGE_ASYNC_READY) && (pLab->tImage.iWidth > 0);
	pLab->bTextureReady = (pLab->iTextureStatus == XGE_ASYNC_READY) && (pLab->tTexture.iBackendId != 0);
	pLab->bFontReady = (pLab->iFontStatus == XGE_ASYNC_READY) && (pLab->tFont.iRefCount > 0);
	if ( pLab->iSoundStatus == XGE_ASYNC_READY ) {
		pLab->bSoundReady = 1;
	} else if ( pLab->iSoundStatus == XGE_ASYNC_FAILED ) {
		pLab->bSoundSkipped = 1;
	}
	pLab->bCancelReady = (pLab->iCancelStatus == XGE_ASYNC_CANCELLED);
}

static void DrawStatusBar(float fX, float fY, float fW, int iStatus, uint32_t iColorReady, uint32_t iColorLoading)
{
	uint32_t iColor;

	iColor = (iStatus == XGE_ASYNC_READY) ? iColorReady : iColorLoading;
	if ( iStatus == XGE_ASYNC_FAILED ) {
		iColor = XGE_COLOR_RGBA(190, 90, 90, 255);
	} else if ( iStatus == XGE_ASYNC_CANCELLED ) {
		iColor = XGE_COLOR_RGBA(148, 124, 214, 255);
	}
	xgeShapeRectFillPx((xge_rect_t){ fX, fY, fW, 18.0f }, iColor);
	xgeShapeRectStrokePx((xge_rect_t){ fX, fY, fW, 18.0f }, 2.0f, XGE_COLOR_RGBA(220, 232, 240, 180));
}

static void DrawTexturePreview(async_assets_lab_t* pLab)
{
	xge_draw_t tDraw;

	if ( !pLab->bTextureReady ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pLab->tTexture;
	tDraw.tSrc.fW = (float)pLab->tTexture.iWidth;
	tDraw.tSrc.fH = (float)pLab->tTexture.iHeight;
	tDraw.tDst.fX = 466.0f;
	tDraw.tDst.fY = 128.0f;
	tDraw.tDst.fW = 120.0f;
	tDraw.tDst.fH = 120.0f;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

static void DrawLoadedFont(async_assets_lab_t* pLab)
{
	if ( !pLab->bFontReady ) {
		return;
	}
	xgeTextDraw(&pLab->tFont, "Async Font Ready", 420.0f, 280.0f, XGE_COLOR_RGBA(232, 242, 248, 255));
}

static int IsDone(async_assets_lab_t* pLab)
{
	int bSoundDone;

	bSoundDone = pLab->bSoundReady || pLab->bSoundSkipped;
	return pLab->bImageReady && pLab->bTextureReady && pLab->bFontReady && bSoundDone && pLab->bCancelReady;
}

static void CleanupLab(async_assets_lab_t* pLab)
{
	xgeSoundFree(&pLab->tSound);
	xgeFontFree(&pLab->tFont);
	xgeTextureFree(&pLab->tCancelledTexture);
	xgeTextureFree(&pLab->tTexture);
	xgeImageFree(&pLab->tImage);
	xgeAsyncRequestFree(&pLab->tCancelRequest);
	xgeAsyncRequestFree(&pLab->tSoundRequest);
	xgeAsyncRequestFree(&pLab->tFontRequest);
	xgeAsyncRequestFree(&pLab->tTextureRequest);
	xgeAsyncRequestFree(&pLab->tImageRequest);
	pLab->bReady = 0;
}

static void PrintFinalSummary(async_assets_lab_t* pLab)
{
	printf("async-assets-lab final-summary frames=%d threading=%d/%d modes=%d/%d loads=%d/%d/%d/%d poll=%d ready=%d/%d/%d/%d cancel=%d callbacks=%d/%d/%d/%d/%d status=%d/%d/%d/%d/%d\n",
		pLab->iFrameCount,
		pLab->bThreadingOffOK,
		pLab->bThreadingOnOK,
		pLab->bMainThreadModeOK,
		pLab->bThreadedModeOK,
		pLab->bImageLoadOK,
		pLab->bTextureLoadOK,
		pLab->bFontLoadOK,
		pLab->bSoundLoadOK,
		pLab->bPollOK,
		pLab->bImageReady,
		pLab->bTextureReady,
		pLab->bFontReady,
		pLab->bSoundReady || pLab->bSoundSkipped,
		pLab->bCancelOK && pLab->bCancelReady,
		pLab->iImageCallbacks,
		pLab->iTextureCallbacks,
		pLab->iFontCallbacks,
		pLab->iSoundCallbacks,
		pLab->iCancelCallbacks,
		pLab->iImageStatus,
		pLab->iTextureStatus,
		pLab->iFontStatus,
		pLab->iSoundStatus,
		pLab->iCancelStatus);
}

static int AsyncAssetsFrame(void* pUser)
{
	async_assets_lab_t* pLab;
	int iRet;

	pLab = (async_assets_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		xgeQuit();
		return 1;
	}
	PollRequests(pLab);
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	xgeShapeRectFillPx((xge_rect_t){ 28.0f, 30.0f, 584.0f, 300.0f }, XGE_COLOR_RGBA(30, 36, 44, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 28.0f, 30.0f, 584.0f, 300.0f }, 2.0f, XGE_COLOR_RGBA(108, 126, 144, 255));
	DrawStatusBar(52.0f, 64.0f, 142.0f, pLab->iImageStatus, XGE_COLOR_RGBA(84, 184, 118, 255), XGE_COLOR_RGBA(186, 150, 82, 255));
	DrawStatusBar(216.0f, 64.0f, 142.0f, pLab->iTextureStatus, XGE_COLOR_RGBA(84, 184, 118, 255), XGE_COLOR_RGBA(88, 150, 214, 255));
	DrawStatusBar(52.0f, 106.0f, 142.0f, pLab->iFontStatus, XGE_COLOR_RGBA(84, 184, 118, 255), XGE_COLOR_RGBA(88, 150, 214, 255));
	DrawStatusBar(216.0f, 106.0f, 142.0f, pLab->iSoundStatus, XGE_COLOR_RGBA(84, 184, 118, 255), XGE_COLOR_RGBA(88, 150, 214, 255));
	DrawStatusBar(52.0f, 148.0f, 306.0f, pLab->iCancelStatus, XGE_COLOR_RGBA(140, 112, 224, 255), XGE_COLOR_RGBA(140, 112, 224, 255));
	xgeShapeRectFillPx((xge_rect_t){ 420.0f, 82.0f, 212.0f, 190.0f }, XGE_COLOR_RGBA(22, 28, 34, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 420.0f, 82.0f, 212.0f, 190.0f }, 2.0f, XGE_COLOR_RGBA(120, 138, 156, 255));
	DrawTexturePreview(pLab);
	DrawLoadedFont(pLab);
	xgeShapeRectFillPx((xge_rect_t){ 52.0f, 286.0f, 540.0f * ((float)(pLab->iImageCallbacks + pLab->iTextureCallbacks + pLab->iFontCallbacks + pLab->iSoundCallbacks + pLab->iCancelCallbacks) / 5.0f), 16.0f }, XGE_COLOR_RGBA(94, 182, 255, 255));
	xgeShapeRectStrokePx((xge_rect_t){ 52.0f, 286.0f, 540.0f, 16.0f }, 2.0f, XGE_COLOR_RGBA(226, 236, 246, 180));

	if ( IsDone(pLab) || ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		PrintFinalSummary(pLab);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	async_assets_lab_t tLab;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_ASYNC_ASSETS_LAB_FRAMES"), 180);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_ASYNC_ASSETS_LAB_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE Async Assets Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(AsyncAssetsFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("async-assets-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
