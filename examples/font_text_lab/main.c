#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

typedef struct font_lab_t {
	xge_font_t tPrimary;
	xge_font_t tMemory;
	xge_font_t tXrfMemory;
	xge_font_t tXrfFile;
	xge_font_t tCached;
	xge_font_t tFallback;
	xge_vec2_t tMeasure;
	xge_glyph_metrics_t tMetrics;
	xge_glyph_t tAtlasGlyph;
	char sFontPath[512];
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int iUtf8Count;
	int iBitmapWidth;
	int iBitmapHeight;
	int iXrfMemorySize;
	int bReady;
	int bDone;
	int bXrfMemory;
	int bXrfFile;
	int bCached;
	int bFallbackFile;
	int bFallbackMemory;
	int bFallbackGet;
	int bAddRef;
} font_lab_t;

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

static int FileExists(const char* sPath)
{
	FILE* pFile;

	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) {
		return 0;
	}
	fclose(pFile);
	return 1;
}

static const char* FindFontPath(int argc, char** argv)
{
	static const char* arrCandidates[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf",
		"C:/Windows/Fonts/segoeui.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
		NULL
	};
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--font") == 0) && ((i + 1) < argc) ) {
			return argv[i + 1];
		}
	}
	for ( i = 0; arrCandidates[i] != NULL; i++ ) {
		if ( FileExists(arrCandidates[i]) ) {
			return arrCandidates[i];
		}
	}
	return NULL;
}

static int ReadFileBytes(const char* sPath, unsigned char** ppData, int* pSize)
{
	FILE* pFile;
	long iSize;
	unsigned char* pData;

	if ( (sPath == NULL) || (ppData == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppData = NULL;
	*pSize = 0;
	pFile = fopen(sPath, "rb");
	if ( pFile == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( fseek(pFile, 0, SEEK_END) != 0 ) {
		fclose(pFile);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iSize = ftell(pFile);
	if ( iSize <= 0 ) {
		fclose(pFile);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( fseek(pFile, 0, SEEK_SET) != 0 ) {
		fclose(pFile);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pData = (unsigned char*)malloc((size_t)iSize);
	if ( pData == NULL ) {
		fclose(pFile);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( fread(pData, 1, (size_t)iSize, pFile) != (size_t)iSize ) {
		free(pData);
		fclose(pFile);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	fclose(pFile);
	*ppData = pData;
	*pSize = (int)iSize;
	return XGE_OK;
}

static int CountUTF8(const char* sText)
{
	const char* sRead;
	uint32_t iCodepoint;
	int iCount;

	sRead = sText;
	iCount = 0;
	while ( xgeTextUTF8Next(&sRead, &iCodepoint) == XGE_OK ) {
		iCount++;
	}
	return iCount;
}

static int InitLab(font_lab_t* pLab)
{
	unsigned char* pFontData;
	void* pXrfData;
	int iFontDataSize;
	int iRet;
	xge_glyph_bitmap_t tBitmap;

	if ( pLab->bReady || pLab->bDone ) {
		return XGE_OK;
	}
	pFontData = NULL;
	pXrfData = NULL;
	iFontDataSize = 0;
	memset(&tBitmap, 0, sizeof(tBitmap));
	if ( pLab->sFontPath[0] == 0 ) {
		printf("font-text-lab skip: no usable system font found; pass --font <path> to test manually\n");
		pLab->bDone = 1;
		xgeQuit();
		return XGE_OK;
	}
	iRet = ReadFileBytes(pLab->sFontPath, &pFontData, &iFontDataSize);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "font-text-lab stage failed: read font bytes ret=%d path=%s\n", iRet, pLab->sFontPath);
		return iRet;
	}
	if ( xgeFontLoad(&pLab->tPrimary, pLab->sFontPath, 30.0f) != XGE_OK ) {
		fprintf(stderr, "font-text-lab stage failed: xgeFontLoad path=%s\n", pLab->sFontPath);
		free(pFontData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeFontLoadMemory(&pLab->tMemory, pFontData, iFontDataSize, 24.0f) != XGE_OK ) {
		fprintf(stderr, "font-text-lab stage failed: xgeFontLoadMemory\n");
		free(pFontData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeFontFallbackSet(pLab->sFontPath, 22.0f) == XGE_OK ) {
		pLab->bFallbackFile = 1;
	}
	if ( xgeFontFallbackGet(&pLab->tFallback, 22.0f) == XGE_OK ) {
		pLab->bFallbackGet = 1;
		xgeFontSetFallback(&pLab->tPrimary, &pLab->tFallback);
	}
	if ( xgeFontFallbackSetMemory(pFontData, iFontDataSize, 20.0f) == XGE_OK ) {
		pLab->bFallbackMemory = 1;
	}
	pLab->tMeasure = xgeTextMeasure(&pLab->tPrimary, "ASCII + 中文 + fallback");
	pLab->iUtf8Count = CountUTF8("ASCII + 中文 + fallback");
	if ( xgeFontGlyphGet(&pLab->tPrimary, 'A', &pLab->tMetrics) != XGE_OK ) {
		fprintf(stderr, "font-text-lab stage failed: xgeFontGlyphGet\n");
		free(pFontData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeFontGlyphRasterize(&pLab->tPrimary, 'A', &tBitmap) != XGE_OK ) {
		fprintf(stderr, "font-text-lab stage failed: xgeFontGlyphRasterize\n");
		free(pFontData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->iBitmapWidth = tBitmap.iWidth;
	pLab->iBitmapHeight = tBitmap.iHeight;
	xgeGlyphBitmapFree(&tBitmap);
	if ( xgeFontGlyphAtlasGet(&pLab->tPrimary, 'A', &pLab->tAtlasGlyph) != XGE_OK ) {
		fprintf(stderr, "font-text-lab stage failed: xgeFontGlyphAtlasGet\n");
		free(pFontData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeFontBuildXRFMemory(&pLab->tPrimary, 32, 96, &pXrfData, &pLab->iXrfMemorySize) == XGE_OK ) {
		if ( xgeFontLoadXRFMemory(&pLab->tXrfMemory, pXrfData, pLab->iXrfMemorySize) == XGE_OK ) {
			pLab->bXrfMemory = 1;
		}
		xgeMemoryFree(pXrfData);
	}
	if ( xgeFontSaveXRF(&pLab->tPrimary, "font_text_lab_ascii.xrf", 32, 96) == XGE_OK ) {
		if ( xgeFontLoadXRF(&pLab->tXrfFile, "font_text_lab_ascii.xrf") == XGE_OK ) {
			pLab->bXrfFile = 1;
		}
	}
	if ( xgeFontLoadCached(&pLab->tCached, pLab->sFontPath, "font_text_lab_cached.xrf", 24.0f, 32, 96) == XGE_OK ) {
		pLab->bCached = 1;
		if ( xgeFontAddRef(&pLab->tCached) > 1 ) {
			pLab->bAddRef = 1;
		}
	}
	free(pFontData);
	pLab->bReady = 1;
	printf("font-text-lab init font=%s utf8=%d measure=%.1fx%.1f glyphA advance=%.1f bitmap=%dx%d atlas=(page=%d xy=%d,%d) xrf_memory=%d/%d xrf_file=%d cached=%d fallback=%d/%d/%d\n",
		pLab->sFontPath,
		pLab->iUtf8Count,
		pLab->tMeasure.fX,
		pLab->tMeasure.fY,
		pLab->tMetrics.fAdvanceX,
		pLab->iBitmapWidth,
		pLab->iBitmapHeight,
		pLab->tAtlasGlyph.iPage,
		pLab->tAtlasGlyph.iX,
		pLab->tAtlasGlyph.iY,
		pLab->bXrfMemory,
		pLab->iXrfMemorySize,
		pLab->bXrfFile,
		pLab->bCached,
		pLab->bFallbackFile,
		pLab->bFallbackGet,
		pLab->bFallbackMemory);
	return XGE_OK;
}

static void CleanupLab(font_lab_t* pLab)
{
	if ( pLab->bAddRef ) {
		xgeFontFree(&pLab->tCached);
		pLab->bAddRef = 0;
	}
	xgeFontFree(&pLab->tCached);
	xgeFontFree(&pLab->tXrfFile);
	xgeFontFree(&pLab->tXrfMemory);
	xgeFontFree(&pLab->tFallback);
	xgeFontFree(&pLab->tMemory);
	xgeFontFree(&pLab->tPrimary);
	xgeFontFallbackClear();
	pLab->bReady = 0;
}

static void DrawGuideBox(float fX, float fY, float fW, float fH, uint32_t iColor)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(28, 35, 46, 255));
	xgeShapeRectStroke(tRect, 1.0f, iColor);
}

static int FontLabFrame(void* pUser)
{
	font_lab_t* pLab;
	int iRet;

	pLab = (font_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "font-text-lab init failed: %d\n", iRet);
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		printf("font-text-lab esc-summary frames=%d utf8=%d xrf_memory=%d xrf_file=%d cached=%d fallback_memory=%d\n",
			pLab->iFrameCount, pLab->iUtf8Count, pLab->bXrfMemory, pLab->bXrfFile, pLab->bCached, pLab->bFallbackMemory);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(14, 18, 24, 255));
	if ( pLab->bReady ) {
		DrawGuideBox(28.0f, 28.0f, 356.0f, 154.0f, XGE_COLOR_RGBA(86, 174, 255, 255));
		DrawGuideBox(416.0f, 28.0f, 356.0f, 154.0f, XGE_COLOR_RGBA(247, 194, 82, 255));
		DrawGuideBox(28.0f, 212.0f, 356.0f, 160.0f, XGE_COLOR_RGBA(112, 214, 146, 255));
		DrawGuideBox(416.0f, 212.0f, 356.0f, 160.0f, XGE_COLOR_RGBA(236, 124, 154, 255));
		xgeTextDraw(&pLab->tPrimary, "Primary TTF\nASCII + 中文 + fallback", 44.0f, 48.0f, XGE_COLOR_RGBA(238, 244, 252, 255));
		xgeTextDraw(&pLab->tMemory, "Memory font\nxgeFontLoadMemory", 44.0f, 122.0f, XGE_COLOR_RGBA(174, 224, 255, 255));
		if ( pLab->bXrfMemory ) {
			xgeTextDraw(&pLab->tXrfMemory, "XRF memory ABC 123", 432.0f, 52.0f, XGE_COLOR_RGBA(255, 232, 156, 255));
		}
		if ( pLab->bXrfFile ) {
			xgeTextDraw(&pLab->tXrfFile, "XRF file cache", 432.0f, 88.0f, XGE_COLOR_RGBA(255, 244, 204, 255));
		}
		if ( pLab->bCached ) {
			xgeTextDraw(&pLab->tCached, "LoadCached + AddRef", 432.0f, 124.0f, XGE_COLOR_RGBA(250, 210, 112, 255));
		}
		xgeTextDrawRect(&pLab->tPrimary, "Centered\nTextDrawRect\nclip line: ABCDEFGHIJKLMNOPQRSTUVWXYZ", (xge_rect_t){ 44.0f, 232.0f, 324.0f, 120.0f }, XGE_COLOR_RGBA(196, 255, 214, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		xgeTextDrawRect(&pLab->tPrimary, "Right bottom\nmeasure / align", (xge_rect_t){ 432.0f, 232.0f, 324.0f, 120.0f }, XGE_COLOR_RGBA(255, 196, 214, 255), XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_BOTTOM | XGE_TEXT_CLIP);
	}
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		printf("font-text-lab final-summary frames=%d utf8=%d measure=%.1fx%.1f line_height=%.1f xrf_memory=%d size=%d xrf_file=%d cached=%d addref=%d fallback=(%d,%d,%d) glyphA=(advance=%.1f bitmap=%dx%d atlas_page=%d)\n",
			pLab->iFrameCount,
			pLab->iUtf8Count,
			pLab->tMeasure.fX,
			pLab->tMeasure.fY,
			pLab->tPrimary.fLineHeight,
			pLab->bXrfMemory,
			pLab->iXrfMemorySize,
			pLab->bXrfFile,
			pLab->bCached,
			pLab->bAddRef,
			pLab->bFallbackFile,
			pLab->bFallbackGet,
			pLab->bFallbackMemory,
			pLab->tMetrics.fAdvanceX,
			pLab->iBitmapWidth,
			pLab->iBitmapHeight,
			pLab->tAtlasGlyph.iPage);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	font_lab_t tLab;
	xge_desc_t tDesc;
	const char* sFontPath;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_FONT_TEXT_FRAMES"), 180);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_FONT_TEXT_SECONDS"), 0.0);
	sFontPath = FindFontPath(argc, argv);
	if ( sFontPath != NULL ) {
		snprintf(tLab.sFontPath, sizeof(tLab.sFontPath), "%s", sFontPath);
	}
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		} else if ( (strcmp(argv[i], "--font") == 0) && ((i + 1) < argc) ) {
			i++;
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 800;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE Font Text Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(FontLabFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("font-text-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
