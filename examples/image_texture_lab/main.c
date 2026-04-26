#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../xge.h"

static const unsigned char g_tgaImage[] = {
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 32, 40,
	40, 80, 220, 255, 60, 210, 80, 220, 220, 120, 60, 180, 240, 240, 240, 255
};

typedef struct image_lab_t {
	xge_texture_t tFromImage;
	xge_texture_t tLoad;
	xge_texture_t tLoadEx;
	xge_texture_t tMemory;
	xge_texture_t tMemoryEx;
	xge_texture_t tGenerated;
	xge_texture_t tFallback;
	int iFrameLimit;
	double fSecondLimit;
	int iFrameCount;
	int bReady;
	int bDone;
	int bAddedRef;
	int iUploadFlushCount;
	int iReadbackOK;
	int iFallbackOK;
	xge_sampler_t tSampler;
} image_lab_t;

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

static void MakeGeneratedPixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int iX;
	int iY;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			int iPos;
			int bTile;

			iPos = ((iY * iWidth) + iX) * 4;
			bTile = (((iX / 8) + (iY / 8)) & 1);
			pPixels[iPos + 0] = (unsigned char)(bTile ? 248 : 36);
			pPixels[iPos + 1] = (unsigned char)(bTile ? 206 : 96);
			pPixels[iPos + 2] = (unsigned char)(bTile ? 72 : 220);
			pPixels[iPos + 3] = 255;
		}
	}
}

static void MakePatchPixels(unsigned char* pPixels, int iWidth, int iHeight, int iStride)
{
	int iX;
	int iY;

	for ( iY = 0; iY < iHeight; iY++ ) {
		for ( iX = 0; iX < iWidth; iX++ ) {
			int iPos;

			iPos = (iY * iStride) + (iX * 4);
			pPixels[iPos + 0] = 40;
			pPixels[iPos + 1] = 236;
			pPixels[iPos + 2] = 180;
			pPixels[iPos + 3] = 255;
		}
	}
}

static int WriteAssetFile(const char* sPath)
{
	FILE* pFile;

	pFile = fopen(sPath, "wb");
	if ( pFile == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( fwrite(g_tgaImage, 1, sizeof(g_tgaImage), pFile) != sizeof(g_tgaImage) ) {
		fclose(pFile);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	fclose(pFile);
	return XGE_OK;
}

static int CheckImagePixel(const xge_image_t* pImage)
{
	const unsigned char* pPixels;

	pPixels = (const unsigned char*)xgeImageGetPixels((xge_image)pImage);
	if ( (pPixels == NULL) || (pImage->iWidth != 2) || (pImage->iHeight != 2) ) {
		return 0;
	}
	return pPixels[3] > 0;
}

static int InitImageLoads(const char* sPath)
{
	xge_image_t tFile;
	xge_image_t tFileEx;
	xge_image_t tMemory;
	xge_image_t tMemoryEx;
	int iOK;

	memset(&tFile, 0, sizeof(tFile));
	memset(&tFileEx, 0, sizeof(tFileEx));
	memset(&tMemory, 0, sizeof(tMemory));
	memset(&tMemoryEx, 0, sizeof(tMemoryEx));
	iOK = 1;
	{
		int iRet;

		iRet = xgeImageLoad(&tFile, sPath);
		if ( (iRet != XGE_OK) || (CheckImagePixel(&tFile) == 0) ) {
			fprintf(stderr, "image-texture-lab stage failed: xgeImageLoad ret=%d size=%dx%d pixels=%p\n", iRet, tFile.iWidth, tFile.iHeight, tFile.pPixels);
			iOK = 0;
		}
	}
	{
		int iRet;

		iRet = xgeImageLoadEx(&tFileEx, sPath, XGE_IMAGE_STRAIGHT_ALPHA);
		if ( (iRet != XGE_OK) || (CheckImagePixel(&tFileEx) == 0) ) {
			fprintf(stderr, "image-texture-lab stage failed: xgeImageLoadEx ret=%d size=%dx%d pixels=%p\n", iRet, tFileEx.iWidth, tFileEx.iHeight, tFileEx.pPixels);
			iOK = 0;
		}
	}
	if ( (xgeImageLoadMemory(&tMemory, g_tgaImage, (int)sizeof(g_tgaImage)) != XGE_OK) || (CheckImagePixel(&tMemory) == 0) ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeImageLoadMemory\n");
		iOK = 0;
	}
	if ( (xgeImageLoadMemoryEx(&tMemoryEx, g_tgaImage, (int)sizeof(g_tgaImage), XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK) || (CheckImagePixel(&tMemoryEx) == 0) ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeImageLoadMemoryEx\n");
		iOK = 0;
	}
	xgeImagePremultiply(&tMemoryEx);
	xgeImageFree(&tFileEx);
	xgeImageFree(&tMemory);
	xgeImageFree(&tMemoryEx);
	xgeImageFree(&tFile);
	return iOK ? XGE_OK : XGE_ERROR_RESOURCE_FAILED;
}

static int InitTextures(image_lab_t* pLab, const char* sPath)
{
	xge_image_t tImage;
	unsigned char arrGenerated[64 * 64 * 4];
	unsigned char arrPatch[20 * 16 * 4];
	unsigned char arrFallback[2 * 2 * 4] = {
		255, 0, 255, 255, 255, 255, 0, 255,
		0, 255, 255, 255, 255, 255, 255, 255
	};
	unsigned char arrReadback[64 * 64 * 4];
	int iPatchStride;

	memset(&tImage, 0, sizeof(tImage));
	if ( xgeImageLoad(&tImage, sPath) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: image for texture\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeTextureCreateFromImage(&pLab->tFromImage, &tImage) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureCreateFromImage\n");
		xgeImageFree(&tImage);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	xgeImageFree(&tImage);
	if ( xgeTextureLoad(&pLab->tLoad, sPath) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureLoad\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeTextureLoadEx(&pLab->tLoadEx, sPath, XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureLoadEx\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeTextureLoadMemory(&pLab->tMemory, g_tgaImage, (int)sizeof(g_tgaImage)) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureLoadMemory\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeTextureLoadMemoryEx(&pLab->tMemoryEx, g_tgaImage, (int)sizeof(g_tgaImage), XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureLoadMemoryEx\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( xgeTextureAddRef(&pLab->tMemoryEx) <= 1 ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureAddRef\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->bAddedRef = 1;
	MakeGeneratedPixels(arrGenerated, 64, 64);
	if ( xgeTextureCreateRGBA(&pLab->tGenerated, 64, 64, arrGenerated) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureCreateRGBA\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iPatchStride = 20 * 4;
	memset(arrPatch, 0, sizeof(arrPatch));
	MakePatchPixels(arrPatch, 16, 16, iPatchStride);
	if ( xgeTextureUpdateRGBA(&pLab->tGenerated, 16, 16, 16, 16, arrPatch, iPatchStride) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureUpdateRGBA\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->tSampler = xgeSamplerDefault();
	pLab->tSampler.iMinFilter = XGE_FILTER_NEAREST;
	pLab->tSampler.iMagFilter = XGE_FILTER_NEAREST;
	pLab->tSampler.iWrapS = XGE_WRAP_REPEAT;
	pLab->tSampler.iWrapT = XGE_WRAP_REPEAT;
	if ( xgeTextureSetSampler(&pLab->tGenerated, &pLab->tSampler) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureSetSampler\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->tSampler = xgeTextureGetSampler(&pLab->tGenerated);
	if ( xgeTextureFallbackSetRGBA(2, 2, arrFallback) != XGE_OK ) {
		fprintf(stderr, "image-texture-lab stage failed: xgeTextureFallbackSetRGBA\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pLab->iFallbackOK = (xgeTextureFallbackGet(&pLab->tFallback) == XGE_OK) ? 1 : 0;
	xgeTextureFallbackClear();
	(void)xgeTextureUploadQueue(&pLab->tGenerated);
	pLab->iUploadFlushCount = xgeTextureUploadFlush();
	memset(arrReadback, 0, sizeof(arrReadback));
	if ( xgeTextureReadPixels(&pLab->tGenerated, arrReadback, 64 * 4) == XGE_OK ) {
		int iPos;

		iPos = ((16 * 64) + 16) * 4;
		pLab->iReadbackOK = (arrReadback[iPos + 0] == 40 && arrReadback[iPos + 1] == 236 && arrReadback[iPos + 2] == 180 && arrReadback[iPos + 3] == 255) ? 1 : 0;
	}
	return XGE_OK;
}

static int InitLab(image_lab_t* pLab)
{
	const char* sWritePath;
	const char* sLoadPath;
	int iRet;

	if ( pLab->bReady != 0 ) {
		return XGE_OK;
	}
	sWritePath = "build/image_texture_lab_asset.tga";
	sLoadPath = "image_texture_lab_asset.tga";
	iRet = WriteAssetFile(sWritePath);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = InitImageLoads(sLoadPath);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = InitTextures(pLab, sLoadPath);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pLab->bReady = 1;
	printf("image-texture-lab init readback=%d fallback=%d sampler=(min=%d mag=%d wrap=%d/%d) upload_flush=%d\n",
		pLab->iReadbackOK,
		pLab->iFallbackOK,
		pLab->tSampler.iMinFilter,
		pLab->tSampler.iMagFilter,
		pLab->tSampler.iWrapS,
		pLab->tSampler.iWrapT,
		pLab->iUploadFlushCount);
	return XGE_OK;
}

static void CleanupLab(image_lab_t* pLab)
{
	if ( pLab->bDone != 0 ) {
		return;
	}
	pLab->bDone = 1;
	if ( pLab->bAddedRef != 0 ) {
		xgeTextureFree(&pLab->tMemoryEx);
		pLab->bAddedRef = 0;
	}
	xgeTextureFree(&pLab->tFromImage);
	xgeTextureFree(&pLab->tLoad);
	xgeTextureFree(&pLab->tLoadEx);
	xgeTextureFree(&pLab->tMemory);
	xgeTextureFree(&pLab->tMemoryEx);
	xgeTextureFree(&pLab->tGenerated);
	xgeTextureFree(&pLab->tFallback);
	xgeTextureFallbackClear();
}

static void DrawTextureTile(xge_texture pTexture, float fX, float fY, float fW, float fH, uint32_t iColor)
{
	xge_draw_t tDraw;

	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst.fX = fX;
	tDraw.tDst.fY = fY;
	tDraw.tDst.fW = fW;
	tDraw.tDst.fH = fH;
	tDraw.iColor = iColor;
	xgeDrawEx(&tDraw);
	xgeShapeRectStrokePx((xge_rect_t){ fX, fY, fW, fH }, 2.0f, XGE_COLOR_RGBA(230, 235, 245, 160));
}

static int ImageTextureFrame(void* pUser)
{
	image_lab_t* pLab;
	int iRet;

	pLab = (image_lab_t*)pUser;
	pLab->iFrameCount++;
	iRet = InitLab(pLab);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "image-texture-lab init failed: %d\n", iRet);
		xgeQuit();
		return 1;
	}
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		printf("image-texture-lab esc-summary frames=%d readback=%d fallback=%d\n", pLab->iFrameCount, pLab->iReadbackOK, pLab->iFallbackOK);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	xgeClear(XGE_COLOR_RGBA(18, 24, 34, 255));
	xgeShapeRectFillPx((xge_rect_t){ 28.0f, 28.0f, 684.0f, 412.0f }, XGE_COLOR_RGBA(32, 42, 54, 255));
	DrawTextureTile(&pLab->tFromImage, 54.0f, 58.0f, 96.0f, 96.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	DrawTextureTile(&pLab->tLoad, 176.0f, 58.0f, 96.0f, 96.0f, XGE_COLOR_RGBA(255, 255, 255, 235));
	DrawTextureTile(&pLab->tLoadEx, 298.0f, 58.0f, 96.0f, 96.0f, XGE_COLOR_RGBA(255, 255, 255, 210));
	DrawTextureTile(&pLab->tMemory, 420.0f, 58.0f, 96.0f, 96.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	DrawTextureTile(&pLab->tMemoryEx, 542.0f, 58.0f, 96.0f, 96.0f, XGE_COLOR_RGBA(255, 255, 255, 230));
	DrawTextureTile(&pLab->tGenerated, 96.0f, 220.0f, 160.0f, 160.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeDrawPx(&pLab->tFallback, 322, 236);
	DrawTextureTile(&pLab->tFallback, 344.0f, 220.0f, 128.0f, 128.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeDraw(&pLab->tGenerated, 540.0f, 248.0f);
	if ( ((pLab->iFrameLimit > 0) && (pLab->iFrameCount >= pLab->iFrameLimit)) || ((pLab->fSecondLimit > 0.0) && (xgeTimer() >= pLab->fSecondLimit)) ) {
		printf("image-texture-lab final-summary frames=%d readback=%d fallback=%d sampler=(%d,%d,%d,%d) upload_flush=%d textures generated=%dx%d fallback_flags=0x%X\n",
			pLab->iFrameCount,
			pLab->iReadbackOK,
			pLab->iFallbackOK,
			pLab->tSampler.iMinFilter,
			pLab->tSampler.iMagFilter,
			pLab->tSampler.iWrapS,
			pLab->tSampler.iWrapT,
			pLab->iUploadFlushCount,
			pLab->tGenerated.iWidth,
			pLab->tGenerated.iHeight,
			pLab->tFallback.iFlags);
		CleanupLab(pLab);
		xgeQuit();
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	image_lab_t tLab;
	xge_desc_t tDesc;
	int i;

	memset(&tLab, 0, sizeof(tLab));
	tLab.iFrameLimit = ArgInt(getenv("XGE_IMAGE_TEXTURE_FRAMES"), 0);
	tLab.fSecondLimit = ArgDouble(getenv("XGE_IMAGE_TEXTURE_SECONDS"), 0.0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tLab.iFrameLimit = ArgInt(argv[++i], tLab.iFrameLimit);
		} else if ( (strcmp(argv[i], "--seconds") == 0) && ((i + 1) < argc) ) {
			tLab.fSecondLimit = ArgDouble(argv[++i], tLab.fSecondLimit);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 740;
	tDesc.iHeight = 470;
	tDesc.sTitle = "XGE Image Texture Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	xgeRun(ImageTextureFrame, &tLab);
	CleanupLab(&tLab);
	xgeUnit();
	printf("image-texture-lab summary frames=%d\n", tLab.iFrameCount);
	return 0;
}
