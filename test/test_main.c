#include "../xge.h"
#include <stdio.h>
#include <string.h>

static int g_iProviderFreeCount;
static unsigned char g_arrProviderData[] = { 'x', 'g', 'e', 0 };

static int __testProviderLoad(const char* sURI, void** ppData, int* pSize, void* pUser)
{
	(void)sURI;
	(void)pUser;
	*ppData = g_arrProviderData;
	*pSize = 3;
	return XGE_OK;
}

static void __testProviderFree(void* pData, void* pUser)
{
	(void)pData;
	(void)pUser;
	g_iProviderFreeCount++;
}

static int __testCoreLifecycle(void)
{
	xge_desc_t objDesc;

	if ( xgeGetWidth() != 0 ) {
		return 1;
	}

	objDesc.iWidth = 320;
	objDesc.iHeight = 200;
	objDesc.sTitle = "XGE Test";
	objDesc.iFlags = 0;
	objDesc.iRunMode = XGE_RUN_MANUAL;
	objDesc.iTargetFPS = 0;
	objDesc.pNativeWindow = 0;
	objDesc.pUser = 0;

	if ( xgeInit(&objDesc) != XGE_OK ) {
		return 2;
	}
	if ( xgeGetWidth() != 320 || xgeGetHeight() != 200 ) {
		return 3;
	}
	if ( xgeInit(0) != XGE_ERROR_ALREADY_INITIALIZED ) {
		return 4;
	}
	return 0;
}

static int __testImageMemory(void)
{
	static const unsigned char arrTga[] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 0, 32, 8, 0, 0, 255, 255
	};
	static const unsigned char arrHalfAlphaTga[] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 0, 32, 8, 0, 0, 255, 128
	};
	xge_image_t objImage;
	unsigned char* pPixels;

	if ( xgeImageLoadMemory(&objImage, arrTga, (int)sizeof(arrTga)) != XGE_OK ) {
		return 10;
	}
	pPixels = (unsigned char*)xgeImageGetPixels(&objImage);
	if ( objImage.iWidth != 1 || objImage.iHeight != 1 || pPixels == 0 ) {
		xgeImageFree(&objImage);
		return 11;
	}
	if ( pPixels[0] != 255 || pPixels[1] != 0 || pPixels[2] != 0 || pPixels[3] != 255 ) {
		xgeImageFree(&objImage);
		return 12;
	}
	xgeImageFree(&objImage);
	if ( objImage.pPixels != 0 ) {
		return 13;
	}

	if ( xgeImageLoadMemory(&objImage, arrHalfAlphaTga, (int)sizeof(arrHalfAlphaTga)) != XGE_OK ) {
		return 14;
	}
	pPixels = (unsigned char*)xgeImageGetPixels(&objImage);
	if ( (objImage.iFlags & XGE_IMAGE_PREMULTIPLIED) == 0 || pPixels[0] != 128 || pPixels[3] != 128 ) {
		xgeImageFree(&objImage);
		return 15;
	}
	xgeImageFree(&objImage);

	if ( xgeImageLoadMemoryEx(&objImage, arrHalfAlphaTga, (int)sizeof(arrHalfAlphaTga), XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ) {
		return 16;
	}
	pPixels = (unsigned char*)xgeImageGetPixels(&objImage);
	if ( (objImage.iFlags & XGE_IMAGE_STRAIGHT_ALPHA) == 0 || pPixels[0] != 255 || pPixels[3] != 128 ) {
		xgeImageFree(&objImage);
		return 17;
	}
	xgeImagePremultiply(&objImage);
	if ( (objImage.iFlags & XGE_IMAGE_PREMULTIPLIED) == 0 || pPixels[0] != 128 || pPixels[3] != 128 ) {
		xgeImageFree(&objImage);
		return 18;
	}
	xgeImageFree(&objImage);
	return 0;
}

static int __testBaseTypesAndInput(void)
{
	xge_color_t tColor;
	xge_touch_point_t tPoint;
	xge_camera_t tCamera;
	xge_vec2_t tPointA;
	xge_vec2_t tPointB;
	float fX;
	float fY;

	if ( xgeColorRGBA(1, 2, 3, 4) != XGE_COLOR_RGBA(1, 2, 3, 4) ) {
		return 20;
	}
	tColor = xgeColorUnpack(XGE_COLOR_RGBA(255, 128, 0, 64));
	if ( (tColor.fR < 0.99f) || (tColor.fG < 0.49f) || (tColor.fB != 0.0f) || (tColor.fA < 0.24f) ) {
		return 21;
	}

	fX = 1.0f;
	fY = 1.0f;
	xgeMouseGetDelta(&fX, &fY);
	if ( (fX != 0.0f) || (fY != 0.0f) ) {
		return 22;
	}
	xgeMouseGetWheel(&fX, &fY);
	if ( (fX != 0.0f) || (fY != 0.0f) ) {
		return 23;
	}
	if ( xgeTextGet() != 0 ) {
		return 24;
	}
	if ( xgeTouchGetCount() != 0 ) {
		return 25;
	}
	if ( xgeTouchGet(0, &tPoint) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 26;
	}
	if ( xgeBlendGet() != XGE_BLEND_ALPHA ) {
		return 27;
	}
	xgeBlendSet(XGE_BLEND_ADD);
	if ( xgeBlendGet() != XGE_BLEND_ADD ) {
		return 28;
	}

	tCamera = xgeCameraDefault(320.0f, 200.0f);
	tCamera.tPosition.fX = 10.0f;
	tCamera.tPosition.fY = 20.0f;
	tCamera.tScale.fX = 2.0f;
	tCamera.tScale.fY = 4.0f;
	xgeCameraSet(&tCamera);
	tPointA.fX = 11.0f;
	tPointA.fY = 22.0f;
	tPointB = xgeWorldToScreen(tPointA);
	if ( (tPointB.fX != 2.0f) || (tPointB.fY != 8.0f) ) {
		return 29;
	}
	tPointA = xgeScreenToWorld(tPointB);
	if ( (tPointA.fX != 11.0f) || (tPointA.fY != 22.0f) ) {
		return 30;
	}
	tCamera.iCoordinateMode = XGE_COORD_CENTER;
	xgeCameraSet(&tCamera);
	tPointA.fX = 10.0f;
	tPointA.fY = 20.0f;
	tPointB = xgeWorldToScreen(tPointA);
	if ( (tPointB.fX != 160.0f) || (tPointB.fY != 100.0f) ) {
		return 31;
	}
	return 0;
}

static int __testTextureLifetime(void)
{
	xge_texture_t tTexture;

	tTexture.iWidth = 1;
	tTexture.iHeight = 1;
	tTexture.iFormat = XGE_PIXEL_RGBA8;
	tTexture.iRefCount = 1;
	tTexture.iFlags = 0;
	tTexture.iBackendId = 0;
	tTexture.pBackend = 0;

	if ( xgeTextureAddRef(&tTexture) != 2 ) {
		return 40;
	}
	xgeTextureFree(&tTexture);
	if ( tTexture.iRefCount != 1 ) {
		return 41;
	}
	xgeTextureFree(&tTexture);
	if ( tTexture.iRefCount != 0 || tTexture.iWidth != 0 ) {
		return 42;
	}
	return 0;
}

static int __testResourceProtocol(void)
{
	FILE* pFile;
	xge_resource_provider_t tProvider;
	xge_resource_t tResource;
	static const char arrMemory[] = { 'm', 'e', 'm' };
	static const char arrFile[] = { 'r', 'e', 's' };
	char* pBytes;

	pFile = fopen("build/xge_resource_test.bin", "wb");
	if ( pFile == 0 ) {
		return 50;
	}
	if ( fwrite(arrFile, 1, sizeof(arrFile), pFile) != sizeof(arrFile) ) {
		fclose(pFile);
		return 51;
	}
	fclose(pFile);

	if ( xgeResourceLoad("res://xge_resource_test.bin", &tResource) != XGE_OK ) {
		return 52;
	}
	pBytes = (char*)tResource.pData;
	if ( (tResource.iSize != (int)sizeof(arrFile)) || (memcmp(pBytes, arrFile, sizeof(arrFile)) != 0) ) {
		xgeResourceFree(&tResource);
		return 53;
	}
	xgeResourceFree(&tResource);
	if ( tResource.pData != 0 ) {
		return 54;
	}

	if ( xgeResourceLoadMemory(arrMemory, (int)sizeof(arrMemory), &tResource) != XGE_OK ) {
		return 55;
	}
	pBytes = (char*)tResource.pData;
	if ( (pBytes == arrMemory) || (tResource.iSize != (int)sizeof(arrMemory)) || (memcmp(pBytes, arrMemory, sizeof(arrMemory)) != 0) ) {
		xgeResourceFree(&tResource);
		return 56;
	}
	xgeResourceFree(&tResource);

	memset(&tProvider, 0, sizeof(tProvider));
	tProvider.sScheme = "mock";
	tProvider.load = __testProviderLoad;
	tProvider.free = __testProviderFree;
	if ( xgeResourceProviderAdd(&tProvider) != XGE_OK ) {
		return 57;
	}
	g_iProviderFreeCount = 0;
	if ( xgeResourceLoad("mock://asset", &tResource) != XGE_OK ) {
		return 58;
	}
	if ( (tResource.pData != g_arrProviderData) || (tResource.iSize != 3) ) {
		xgeResourceFree(&tResource);
		return 59;
	}
	xgeResourceProviderClear();
	xgeResourceFree(&tResource);
	if ( g_iProviderFreeCount != 1 ) {
		return 60;
	}
	return 0;
}

static int __testAudioApiShape(void)
{
	xge_audio_listener_t tListener;
	xge_audio_group_t tGroup;
	xge_sound_t tSound;

	if ( xgeAudioIsReady() != 0 ) {
		return 70;
	}
	if ( xgeAudioGetVolume() != 0.0f ) {
		return 71;
	}
	tListener = xgeAudioListenerGet();
	if ( (tListener.tForward.fZ != -1.0f) || (tListener.tUp.fY != 1.0f) ) {
		return 72;
	}
	memset(&tSound, 0, sizeof(tSound));
	tSound.iRefCount = 1;
	tSound.iType = XGE_AUDIO_SOUND;
	if ( xgeSoundAddRef(&tSound) != 2 ) {
		return 73;
	}
	xgeSoundFree(&tSound);
	if ( tSound.iRefCount != 1 ) {
		return 74;
	}
	xgeSoundFree(&tSound);
	if ( tSound.iRefCount != 0 ) {
		return 75;
	}
	if ( xgeSoundPlay(0) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 76;
	}
	memset(&tGroup, 0, sizeof(tGroup));
	if ( xgeAudioGroupInit(&tGroup) != XGE_ERROR_NOT_INITIALIZED ) {
		return 77;
	}
	xgeAudioGroupSetVolume(&tGroup, 0.5f);
	if ( xgeAudioGroupGetVolume(&tGroup) != 0.5f ) {
		return 78;
	}
	xgeAudioGroupFade(&tGroup, 0.5f, 0.25f, 100);
	if ( xgeAudioGroupGetVolume(&tGroup) != 0.5f ) {
		return 79;
	}
	xgeAudioGroupFree(&tGroup);
	if ( xgeSoundLoadGroup(0, "none.wav", &tGroup) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 80;
	}
	return 0;
}

static int __testTextFontBase(void)
{
	const char* sText;
	uint32_t iCodepoint;
	xge_font_t tFont;
	xge_glyph_metrics_t tMetrics;
	xge_glyph_bitmap_t tBitmap;
	xge_glyph_t tGlyphA;
	xge_glyph_t tGlyphB;
	xge_vec2_t tSize;
	FILE* pFile;

	sText = "A\xe4\xb8\xad";
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_OK || iCodepoint != 'A' ) {
		return 90;
	}
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_OK || iCodepoint != 0x4E2D ) {
		return 91;
	}
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_ERROR ) {
		return 92;
	}
	sText = "\xF0\x9F\x98\x80";
	if ( xgeTextUTF8Next(&sText, &iCodepoint) != XGE_ERROR_UNSUPPORTED ) {
		return 93;
	}

	memset(&tFont, 0, sizeof(tFont));
	if ( xgeFontLoadMemory(&tFont, "bad", 3, 16.0f) != XGE_ERROR_RESOURCE_FAILED ) {
		return 94;
	}
	if ( xgeFontLoadMemory(&tFont, "bad", 3, 0.0f) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 95;
	}

	pFile = fopen("C:/Windows/Fonts/arial.ttf", "rb");
	if ( pFile != 0 ) {
		fclose(pFile);
		if ( xgeFontLoad(&tFont, "C:/Windows/Fonts/arial.ttf", 16.0f) != XGE_OK ) {
			return 96;
		}
		if ( tFont.iRefCount != 1 || tFont.fLineHeight <= 0.0f || tFont.pBackend == 0 ) {
			xgeFontFree(&tFont);
			return 97;
		}
		if ( xgeFontGlyphGet(&tFont, 'A', &tMetrics) != XGE_OK || tMetrics.fAdvanceX <= 0.0f ) {
			xgeFontFree(&tFont);
			return 98;
		}
		if ( xgeFontGlyphRasterize(&tFont, 'A', &tBitmap) != XGE_OK || tBitmap.iFormat != XGE_PIXEL_A8 || tBitmap.iWidth <= 0 || tBitmap.iHeight <= 0 ) {
			xgeFontFree(&tFont);
			return 103;
		}
		xgeGlyphBitmapFree(&tBitmap);
		if ( tBitmap.pPixels != 0 ) {
			xgeFontFree(&tFont);
			return 104;
		}
		if ( xgeFontGlyphAtlasGet(&tFont, 'A', &tGlyphA) != XGE_OK || tGlyphA.iPage < 0 || tGlyphA.iWidth <= 0 || tFont.tAtlas.iPageCount <= 0 ) {
			xgeFontFree(&tFont);
			return 105;
		}
		if ( xgeFontGlyphAtlasGet(&tFont, 'A', &tGlyphB) != XGE_OK || tGlyphA.iX != tGlyphB.iX || tGlyphA.iY != tGlyphB.iY || tGlyphA.iPage != tGlyphB.iPage ) {
			xgeFontFree(&tFont);
			return 106;
		}
		tSize = xgeTextMeasure(&tFont, "ABC\nD");
		if ( tSize.fX <= 0.0f || tSize.fY <= tFont.fLineHeight ) {
			xgeFontFree(&tFont);
			return 99;
		}
		if ( xgeFontAddRef(&tFont) != 2 ) {
			xgeFontFree(&tFont);
			return 100;
		}
		xgeFontFree(&tFont);
		if ( tFont.iRefCount != 1 ) {
			xgeFontFree(&tFont);
			return 101;
		}
		xgeFontFree(&tFont);
		if ( tFont.iRefCount != 0 || tFont.pData != 0 ) {
			return 102;
		}
	}
	return 0;
}

int main(void)
{
	int iRet;

	iRet = __testCoreLifecycle();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testImageMemory();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testBaseTypesAndInput();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testTextureLifetime();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testResourceProtocol();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testAudioApiShape();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testTextFontBase();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	xgeUnit();
	return 0;
}

