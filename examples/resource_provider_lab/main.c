#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

static const unsigned char g_tgaImage[] = {
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 32, 40,
	64, 92, 220, 255, 72, 218, 128, 255, 236, 184, 72, 255, 246, 246, 246, 255
};

typedef struct resource_entry_t {
	const char* sPath;
	const unsigned char* pData;
	int iSize;
} resource_entry_t;

typedef struct mem_provider_t {
	resource_entry_t* pEntries;
	int iEntryCount;
	int iLoadCount;
	int iFreeCount;
	int iHitCount;
	int iMissCount;
} mem_provider_t;

typedef struct mock_pack_t {
	resource_entry_t* pEntries;
	int iEntryCount;
	int iReadCount;
	int iFreeCount;
	int iHitCount;
	int iMissCount;
	char sLastPath[256];
} mock_pack_t;

typedef struct resource_lab_t {
	xge_texture_t tMemTexture;
	xge_texture_t tPackTexture;
	xge_font_t tProviderFont;
	mem_provider_t tMemProvider;
	mock_pack_t tPack;
	resource_entry_t arrMemEntries[3];
	resource_entry_t arrPackEntries[2];
	unsigned char* pFontData;
	int iFontDataSize;
	char sFontPath[512];
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int iMemResourceOK;
	int iPackResourceOK;
	int iMemoryResourceOK;
	int iTextureOK;
	int iFontOK;
	int iXrfBytes;
	int bMemoryFreed;
	int bReady;
	int bDone;
	int bProvidersRegistered;
} resource_lab_t;

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

static const char* UriPath(const char* sURI)
{
	const char* sPath;

	sPath = strstr(sURI, "://");
	if ( sPath != NULL ) {
		sPath += 3;
	} else {
		sPath = sURI;
	}
	while ( (*sPath == '/') || (*sPath == '\\') ) {
		sPath++;
	}
	return sPath;
}

static int EntryLoadCopy(const resource_entry_t* pEntry, void** ppData, int* pSize)
{
	void* pCopy;

	if ( (pEntry == NULL) || (pEntry->pData == NULL) || (pEntry->iSize < 0) ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	pCopy = malloc((size_t)pEntry->iSize);
	if ( pCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pCopy, pEntry->pData, (size_t)pEntry->iSize);
	*ppData = pCopy;
	*pSize = pEntry->iSize;
	return XGE_OK;
}

static int MemProviderLoad(const char* sURI, void** ppData, int* pSize, void* pUser)
{
	mem_provider_t* pProvider;
	const char* sPath;
	int i;

	pProvider = (mem_provider_t*)pUser;
	if ( (pProvider == NULL) || (sURI == NULL) || (ppData == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pProvider->iLoadCount++;
	sPath = UriPath(sURI);
	for ( i = 0; i < pProvider->iEntryCount; i++ ) {
		if ( strcmp(sPath, pProvider->pEntries[i].sPath) == 0 ) {
			pProvider->iHitCount++;
			return EntryLoadCopy(&pProvider->pEntries[i], ppData, pSize);
		}
	}
	pProvider->iMissCount++;
	return XGE_ERROR_FILE_NOT_FOUND;
}

static void MemProviderFree(void* pData, void* pUser)
{
	mem_provider_t* pProvider;

	pProvider = (mem_provider_t*)pUser;
	if ( pProvider != NULL ) {
		pProvider->iFreeCount++;
	}
	free(pData);
}

static void* MockPackRead(void* pPack, const char* sPath, uint64_t* pSize, void* pUser)
{
	mock_pack_t* pMock;
	void* pData;
	int iSize;
	int i;

	(void)pUser;
	pMock = (mock_pack_t*)pPack;
	if ( (pMock == NULL) || (sPath == NULL) || (pSize == NULL) ) {
		return NULL;
	}
	pMock->iReadCount++;
	snprintf(pMock->sLastPath, sizeof(pMock->sLastPath), "%s", sPath);
	for ( i = 0; i < pMock->iEntryCount; i++ ) {
		if ( strcmp(sPath, pMock->pEntries[i].sPath) == 0 ) {
			if ( EntryLoadCopy(&pMock->pEntries[i], &pData, &iSize) != XGE_OK ) {
				return NULL;
			}
			pMock->iHitCount++;
			*pSize = (uint64_t)iSize;
			return pData;
		}
	}
	pMock->iMissCount++;
	return NULL;
}

static void MockPackFree(void* pData, void* pUser)
{
	mock_pack_t* pMock;

	pMock = (mock_pack_t*)pUser;
	if ( pMock != NULL ) {
		pMock->iFreeCount++;
	}
	free(pData);
}

static int RegisterProviders(resource_lab_t* pLab)
{
	xge_resource_provider_t tMemProvider;
	xge_xpack_provider_t tPackProvider;
	static const unsigned char sMemText[] = "mem provider resource";
	static const unsigned char sPackText[] = "mock xpack resource";

	pLab->arrMemEntries[0].sPath = "hello.txt";
	pLab->arrMemEntries[0].pData = sMemText;
	pLab->arrMemEntries[0].iSize = (int)sizeof(sMemText) - 1;
	pLab->arrMemEntries[1].sPath = "checker.tga";
	pLab->arrMemEntries[1].pData = g_tgaImage;
	pLab->arrMemEntries[1].iSize = (int)sizeof(g_tgaImage);
	pLab->arrMemEntries[2].sPath = "font.ttf";
	pLab->arrMemEntries[2].pData = pLab->pFontData;
	pLab->arrMemEntries[2].iSize = pLab->iFontDataSize;
	pLab->tMemProvider.pEntries = pLab->arrMemEntries;
	pLab->tMemProvider.iEntryCount = 3;
	pLab->arrPackEntries[0].sPath = "assets/hello.txt";
	pLab->arrPackEntries[0].pData = sPackText;
	pLab->arrPackEntries[0].iSize = (int)sizeof(sPackText) - 1;
	pLab->arrPackEntries[1].sPath = "assets/checker.tga";
	pLab->arrPackEntries[1].pData = g_tgaImage;
	pLab->arrPackEntries[1].iSize = (int)sizeof(g_tgaImage);
	pLab->tPack.pEntries = pLab->arrPackEntries;
	pLab->tPack.iEntryCount = 2;
	memset(&tMemProvider, 0, sizeof(tMemProvider));
	tMemProvider.sScheme = "mem";
	tMemProvider.load = MemProviderLoad;
	tMemProvider.free = MemProviderFree;
	tMemProvider.pUser = &pLab->tMemProvider;
	if ( xgeResourceProviderAdd(&tMemProvider) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	memset(&tPackProvider, 0, sizeof(tPackProvider));
	tPackProvider.pPack = &pLab->tPack;
	tPackProvider.sRoot = "assets";
	tPackProvider.read = MockPackRead;
	tPackProvider.free = MockPackFree;
	tPackProvider.pUser = &pLab->tPack;
	if ( xgeResourceXPackProviderAdd(&tPackProvider) != XGE_OK ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->bProvidersRegistered = 1;
	return XGE_OK;
}

static int CheckResourceText(xge_resource_t* pResource, const char* sExpected)
{
	if ( (pResource == NULL) || (pResource->pData == NULL) || (sExpected == NULL) ) {
		return 0;
	}
	if ( pResource->iSize != (int)strlen(sExpected) ) {
		return 0;
	}
	return memcmp(pResource->pData, sExpected, (size_t)pResource->iSize) == 0;
}

static int InitLab(resource_lab_t* pLab)
{
	xge_resource_t tMemResource;
	xge_resource_t tPackResource;
	xge_resource_t tMemoryResource;
	void* pXrfData;
	int iRet;

	if ( pLab->bReady || pLab->bDone ) {
		return XGE_OK;
	}
	memset(&tMemResource, 0, sizeof(tMemResource));
	memset(&tPackResource, 0, sizeof(tPackResource));
	memset(&tMemoryResource, 0, sizeof(tMemoryResource));
	pXrfData = NULL;
	if ( pLab->sFontPath[0] == 0 ) {
		printf("resource-provider-lab skip: no usable system font found; pass --font <path> to test manually\n");
		pLab->bDone = 1;
		xgeQuit();
		return XGE_OK;
	}
	iRet = ReadFileBytes(pLab->sFontPath, &pLab->pFontData, &pLab->iFontDataSize);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "resource-provider-lab stage failed: read font ret=%d path=%s\n", iRet, pLab->sFontPath);
		return iRet;
	}
	xgeResourceProviderClear();
	iRet = RegisterProviders(pLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "resource-provider-lab stage failed: register providers\n");
		return iRet;
	}
	if ( xgeResourceLoad("mem://hello.txt", &tMemResource) == XGE_OK ) {
		pLab->iMemResourceOK = CheckResourceText(&tMemResource, "mem provider resource");
		xgeResourceFree(&tMemResource);
	}
	if ( xgeResourceLoad("res://hello.txt", &tPackResource) == XGE_OK ) {
		pLab->iPackResourceOK = CheckResourceText(&tPackResource, "mock xpack resource");
		xgeResourceFree(&tPackResource);
	}
	if ( xgeResourceLoadMemory("inline resource", 15, &tMemoryResource) == XGE_OK ) {
		pLab->iMemoryResourceOK = CheckResourceText(&tMemoryResource, "inline resource");
		xgeResourceFree(&tMemoryResource);
	}
	if ( (xgeTextureLoad(&pLab->tMemTexture, "mem://checker.tga") == XGE_OK) &&
		(xgeTextureLoad(&pLab->tPackTexture, "res://checker.tga") == XGE_OK) ) {
		pLab->iTextureOK = 1;
	}
	if ( xgeFontLoad(&pLab->tProviderFont, "mem://font.ttf", 24.0f) == XGE_OK ) {
		pLab->iFontOK = 1;
		if ( xgeFontBuildXRFMemory(&pLab->tProviderFont, 32, 32, &pXrfData, &pLab->iXrfBytes) == XGE_OK ) {
			xgeMemoryFree(pXrfData);
			pLab->bMemoryFreed = 1;
		}
	}
	if ( (pLab->iMemResourceOK == 0) || (pLab->iPackResourceOK == 0) || (pLab->iMemoryResourceOK == 0) || (pLab->iTextureOK == 0) || (pLab->iFontOK == 0) || (pLab->bMemoryFreed == 0) ) {
		fprintf(stderr, "resource-provider-lab stage failed: mem=%d pack=%d memory=%d texture=%d font=%d memory_free=%d\n",
			pLab->iMemResourceOK, pLab->iPackResourceOK, pLab->iMemoryResourceOK, pLab->iTextureOK, pLab->iFontOK, pLab->bMemoryFreed);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->bReady = 1;
	printf("resource-provider-lab init mem(load=%d hit=%d miss=%d free=%d) xpack(read=%d hit=%d miss=%d free=%d last=%s) resources=(mem=%d pack=%d memory=%d) texture=%d font=%d xrf=%d memory_free=%d\n",
		pLab->tMemProvider.iLoadCount,
		pLab->tMemProvider.iHitCount,
		pLab->tMemProvider.iMissCount,
		pLab->tMemProvider.iFreeCount,
		pLab->tPack.iReadCount,
		pLab->tPack.iHitCount,
		pLab->tPack.iMissCount,
		pLab->tPack.iFreeCount,
		pLab->tPack.sLastPath,
		pLab->iMemResourceOK,
		pLab->iPackResourceOK,
		pLab->iMemoryResourceOK,
		pLab->iTextureOK,
		pLab->iFontOK,
		pLab->iXrfBytes,
		pLab->bMemoryFreed);
	return XGE_OK;
}

static void CleanupLab(resource_lab_t* pLab)
{
	xgeTextureFree(&pLab->tPackTexture);
	xgeTextureFree(&pLab->tMemTexture);
	xgeFontFree(&pLab->tProviderFont);
	if ( pLab->bProvidersRegistered ) {
		xgeResourceProviderClear();
		pLab->bProvidersRegistered = 0;
	}
	if ( pLab->pFontData != NULL ) {
		free(pLab->pFontData);
		pLab->pFontData = NULL;
	}
	pLab->bReady = 0;
}

static void DrawPanel(float fX, float fY, float fW, float fH, uint32_t iColor)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeShapeRectStroke(tRect, 1.0f, iColor);
}

static void DrawTextureScaled(xge_texture pTexture, float fX, float fY, float fW, float fH)
{
	xge_draw_t tDraw;

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tSrc.fX = 0.0f;
	tDraw.tSrc.fY = 0.0f;
	tDraw.tSrc.fW = (float)pTexture->iWidth;
	tDraw.tSrc.fH = (float)pTexture->iHeight;
	tDraw.tDst.fX = fX;
	tDraw.tDst.fY = fY;
	tDraw.tDst.fW = fW;
	tDraw.tDst.fH = fH;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	xgeDrawEx(&tDraw);
}

static int ResourceLabFrame(void* pUser)
{
	resource_lab_t* pLab;
	int iRet;

	pLab = (resource_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "resource-provider-lab init failed: %d\n", iRet);
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		printf("resource-provider-lab esc-summary frames=%d mem_load=%d pack_read=%d\n",
			pLab->iFrameCount, pLab->tMemProvider.iLoadCount, pLab->tPack.iReadCount);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(13, 17, 23, 255));
	if ( pLab->bReady ) {
		DrawPanel(28.0f, 30.0f, 344.0f, 280.0f, XGE_COLOR_RGBA(96, 180, 255, 255));
		DrawPanel(428.0f, 30.0f, 344.0f, 280.0f, XGE_COLOR_RGBA(245, 190, 82, 255));
		DrawTextureScaled(&pLab->tMemTexture, 62.0f, 72.0f, 96.0f, 96.0f);
		DrawTextureScaled(&pLab->tPackTexture, 462.0f, 72.0f, 96.0f, 96.0f);
		xgeTextDraw(&pLab->tProviderFont, "mem:// provider\nresource + texture + font", 64.0f, 190.0f, XGE_COLOR_RGBA(228, 242, 255, 255));
		xgeTextDraw(&pLab->tProviderFont, "res:// mock xpack\nroot + read/free callbacks", 464.0f, 190.0f, XGE_COLOR_RGBA(255, 234, 186, 255));
		xgeTextDraw(&pLab->tProviderFont, "inline memory resource + xgeMemoryFree", 72.0f, 336.0f, XGE_COLOR_RGBA(198, 244, 214, 255));
	}
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		printf("resource-provider-lab final-summary frames=%d mem(load=%d hit=%d miss=%d free=%d) xpack(read=%d hit=%d miss=%d free=%d last=%s) resources=(mem=%d pack=%d memory=%d) texture=%d font=%d xrf=%d memory_free=%d\n",
			pLab->iFrameCount,
			pLab->tMemProvider.iLoadCount,
			pLab->tMemProvider.iHitCount,
			pLab->tMemProvider.iMissCount,
			pLab->tMemProvider.iFreeCount,
			pLab->tPack.iReadCount,
			pLab->tPack.iHitCount,
			pLab->tPack.iMissCount,
			pLab->tPack.iFreeCount,
			pLab->tPack.sLastPath,
			pLab->iMemResourceOK,
			pLab->iPackResourceOK,
			pLab->iMemoryResourceOK,
			pLab->iTextureOK,
			pLab->iFontOK,
			pLab->iXrfBytes,
			pLab->bMemoryFreed);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	resource_lab_t tLab;
	xge_desc_t tDesc;
	const char* sFontPath;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_RESOURCE_PROVIDER_FRAMES"), 180);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_RESOURCE_PROVIDER_SECONDS"), 0.0);
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
	tDesc.sTitle = "XGE Resource Provider Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(ResourceLabFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("resource-provider-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
