#include "../xge.h"
#include <stdio.h>
#include <string.h>

static int g_iProviderFreeCount;
static unsigned char g_arrProviderData[] = { 'x', 'g', 'e', 0 };

typedef struct xrf_test_blob2_t {
	xge_xrf_header_t tHeader;
	xge_xrf_range_t tRange;
	xge_xrf_glyph_t arrGlyphs[2];
	xge_xrf_page_t tPage;
	unsigned char arrPixels[16 * 16];
} xrf_test_blob2_t;

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
	xge_rect_t tRect;

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
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 10.0f;
	tRect.fH = 10.0f;
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(255, 0, 0, 255));
	xgeShapeRectFillPx(tRect, XGE_COLOR_RGBA(0, 255, 0, 255));
	xgeClipSet(tRect);
	xgeClipClear();
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

static int __testXRFMemory(void)
{
	struct xrf_test_blob_t {
		xge_xrf_header_t tHeader;
		xge_xrf_range_t tRange;
		xge_xrf_glyph_t arrGlyphs[2];
		xge_xrf_page_t tPage;
		unsigned char arrPixels[16 * 16];
	} tBlob;
	xge_font_t tFont;
	xge_glyph_t tGlyph;
	xge_glyph_metrics_t tMetrics;
	xge_vec2_t tSize;
	int i;

	memset(&tBlob, 0, sizeof(tBlob));
	tBlob.tHeader.iMagic = XGE_XRF_MAGIC;
	tBlob.tHeader.iVersion = XGE_XRF_VERSION;
	tBlob.tHeader.iHeaderSize = sizeof(xge_xrf_header_t);
	tBlob.tHeader.iGlyphCount = 2;
	tBlob.tHeader.iPageCount = 1;
	tBlob.tHeader.iRangeCount = 1;
	tBlob.tHeader.fAscent = 10.0f;
	tBlob.tHeader.fDescent = -3.0f;
	tBlob.tHeader.fLineGap = 1.0f;
	tBlob.tHeader.fLineHeight = 14.0f;
	tBlob.tHeader.iRangeOffset = sizeof(xge_xrf_header_t);
	tBlob.tHeader.iGlyphOffset = tBlob.tHeader.iRangeOffset + sizeof(xge_xrf_range_t);
	tBlob.tHeader.iPageOffset = tBlob.tHeader.iGlyphOffset + sizeof(tBlob.arrGlyphs);
	tBlob.tHeader.iPixelOffset = tBlob.tHeader.iPageOffset + sizeof(xge_xrf_page_t);
	tBlob.tRange.iFirstCodepoint = 'A';
	tBlob.tRange.iCount = 2;
	tBlob.arrGlyphs[0].iCodepoint = 'A';
	tBlob.arrGlyphs[0].iPage = 0;
	tBlob.arrGlyphs[0].iGlyph = 1;
	tBlob.arrGlyphs[0].iX = 0;
	tBlob.arrGlyphs[0].iY = 0;
	tBlob.arrGlyphs[0].iWidth = 4;
	tBlob.arrGlyphs[0].iHeight = 6;
	tBlob.arrGlyphs[0].fAdvanceX = 8.0f;
	tBlob.arrGlyphs[1].iCodepoint = 0x4E2D;
	tBlob.arrGlyphs[1].iPage = 0;
	tBlob.arrGlyphs[1].iGlyph = 2;
	tBlob.arrGlyphs[1].iX = 4;
	tBlob.arrGlyphs[1].iY = 0;
	tBlob.arrGlyphs[1].iWidth = 8;
	tBlob.arrGlyphs[1].iHeight = 8;
	tBlob.arrGlyphs[1].fAdvanceX = 12.0f;
	tBlob.tPage.iWidth = 16;
	tBlob.tPage.iHeight = 16;
	tBlob.tPage.iFormat = XGE_XRF_PAGE_A8;
	tBlob.tPage.iPixelOffset = 0;
	tBlob.tPage.iPixelSize = sizeof(tBlob.arrPixels);
	for ( i = 0; i < (int)sizeof(tBlob.arrPixels); i++ ) {
		tBlob.arrPixels[i] = (unsigned char)i;
	}

	memset(&tFont, 0, sizeof(tFont));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 110;
	}
	if ( tFont.fLineHeight != 14.0f || tFont.tAtlas.iPageCount != 1 || tFont.tAtlas.iPageWidth != 16 ) {
		xgeFontFree(&tFont);
		return 111;
	}
	if ( xgeFontGlyphAtlasGet(&tFont, 'A', &tGlyph) != XGE_OK || tGlyph.iPage != 0 || tGlyph.fAdvanceX != 8.0f ) {
		xgeFontFree(&tFont);
		return 112;
	}
	if ( xgeFontGlyphGet(&tFont, 0x4E2D, &tMetrics) != XGE_OK || tMetrics.fAdvanceX != 12.0f ) {
		xgeFontFree(&tFont);
		return 113;
	}
	tSize = xgeTextMeasure(&tFont, "A\xe4\xb8\xad");
	if ( tSize.fX != 20.0f || tSize.fY != 14.0f ) {
		xgeFontFree(&tFont);
		return 114;
	}
	xgeFontFree(&tFont);
	if ( tFont.iRefCount != 0 || tFont.tAtlas.iPageCount != 0 ) {
		return 115;
	}
	tBlob.tHeader.iMagic = 0;
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_ERROR_RESOURCE_FAILED ) {
		return 116;
	}
	return 0;
}

static void __testXrfBlobMake(xrf_test_blob2_t* pBlob)
{
	int i;

	memset(pBlob, 0, sizeof(*pBlob));
	pBlob->tHeader.iMagic = XGE_XRF_MAGIC;
	pBlob->tHeader.iVersion = XGE_XRF_VERSION;
	pBlob->tHeader.iHeaderSize = sizeof(xge_xrf_header_t);
	pBlob->tHeader.iGlyphCount = 2;
	pBlob->tHeader.iPageCount = 1;
	pBlob->tHeader.iRangeCount = 1;
	pBlob->tHeader.fAscent = 10.0f;
	pBlob->tHeader.fDescent = -3.0f;
	pBlob->tHeader.fLineGap = 1.0f;
	pBlob->tHeader.fLineHeight = 14.0f;
	pBlob->tHeader.iRangeOffset = sizeof(xge_xrf_header_t);
	pBlob->tHeader.iGlyphOffset = pBlob->tHeader.iRangeOffset + sizeof(xge_xrf_range_t);
	pBlob->tHeader.iPageOffset = pBlob->tHeader.iGlyphOffset + sizeof(pBlob->arrGlyphs);
	pBlob->tHeader.iPixelOffset = pBlob->tHeader.iPageOffset + sizeof(xge_xrf_page_t);
	pBlob->tRange.iFirstCodepoint = 'A';
	pBlob->tRange.iCount = 2;
	pBlob->arrGlyphs[0].iCodepoint = 'A';
	pBlob->arrGlyphs[0].iPage = 0;
	pBlob->arrGlyphs[0].iGlyph = 1;
	pBlob->arrGlyphs[0].iX = 0;
	pBlob->arrGlyphs[0].iY = 0;
	pBlob->arrGlyphs[0].iWidth = 4;
	pBlob->arrGlyphs[0].iHeight = 6;
	pBlob->arrGlyphs[0].fAdvanceX = 8.0f;
	pBlob->arrGlyphs[1].iCodepoint = 0x4E2D;
	pBlob->arrGlyphs[1].iPage = 0;
	pBlob->arrGlyphs[1].iGlyph = 2;
	pBlob->arrGlyphs[1].iX = 4;
	pBlob->arrGlyphs[1].iY = 0;
	pBlob->arrGlyphs[1].iWidth = 8;
	pBlob->arrGlyphs[1].iHeight = 8;
	pBlob->arrGlyphs[1].fAdvanceX = 12.0f;
	pBlob->tPage.iWidth = 16;
	pBlob->tPage.iHeight = 16;
	pBlob->tPage.iFormat = XGE_XRF_PAGE_A8;
	pBlob->tPage.iPixelOffset = 0;
	pBlob->tPage.iPixelSize = sizeof(pBlob->arrPixels);
	for ( i = 0; i < (int)sizeof(pBlob->arrPixels); i++ ) {
		pBlob->arrPixels[i] = (unsigned char)i;
	}
}

static int __testXuiIncubationBase(void)
{
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pChild;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	if ( xgeXuiInit(NULL) != XGE_ERROR_INVALID_ARGUMENT ) {
		return 120;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 121;
	}
	if ( xgeXuiInit(&tXui) != XGE_ERROR_ALREADY_INITIALIZED ) {
		xgeXuiUnit(&tXui);
		return 122;
	}
	pRoot = xgeXuiRoot(&tXui);
	if ( pRoot == NULL || pRoot->pFirstChild != NULL ) {
		xgeXuiUnit(&tXui);
		return 123;
	}
	pChild = xgeXuiWidgetCreate();
	if ( pChild == NULL ) {
		xgeXuiUnit(&tXui);
		return 124;
	}
	tRect.fX = 10.0f;
	tRect.fY = 20.0f;
	tRect.fW = 30.0f;
	tRect.fH = 40.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	xgeXuiWidgetSetLayout(pChild, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pChild, XGE_COLOR_RGBA(255, 0, 0, 255));
	xgeXuiWidgetSetClip(pChild, 1);
	if ( (xgeXuiWidgetGetFlags(pChild) & XGE_XUI_WIDGET_CLIP) == 0 ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 125;
	}
	if ( xgeXuiWidgetAdd(pRoot, pChild) != XGE_OK ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 126;
	}
	if ( pRoot->pFirstChild != pChild || pChild->pParent != pRoot ) {
		xgeXuiUnit(&tXui);
		return 127;
	}
	if ( xgeXuiUpdate(&tXui, 1.0f / 60.0f) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 128;
	}
	tRect = xgeXuiWidgetGetRect(pChild);
	if ( tRect.fX != 10.0f || tRect.fY != 20.0f || tRect.fW != 30.0f || tRect.fH != 40.0f ) {
		xgeXuiUnit(&tXui);
		return 129;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		return 130;
	}
	xgeXuiWidgetRemove(pChild);
	if ( pRoot->pFirstChild != NULL || pChild->pParent != NULL ) {
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 131;
	}
	xgeXuiWidgetFree(pChild);
	xgeXuiUnit(&tXui);
	if ( tXui.bInitialized != 0 || tXui.pRoot != NULL ) {
		return 132;
	}
	return 0;
}

static int __testXuiLayoutModes(void)
{
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pPanel;
	xge_xui_widget pA;
	xge_xui_widget pB;
	xge_rect_t tRect;

	memset(&tXui, 0, sizeof(tXui));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 140;
	}
	pRoot = xgeXuiRoot(&tXui);
	pPanel = xgeXuiWidgetCreate();
	pA = xgeXuiWidgetCreate();
	pB = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pPanel == NULL) || (pA == NULL) || (pB == NULL) ) {
		xgeXuiWidgetFree(pPanel);
		xgeXuiWidgetFree(pA);
		xgeXuiWidgetFree(pB);
		xgeXuiUnit(&tXui);
		return 141;
	}

	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 300.0f;
	tRect.fH = 100.0f;
	xgeXuiWidgetSetRect(pPanel, tRect);
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetPaddingPx(pPanel, 10.0f, 0.0f, 10.0f, 0.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(50.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetMarginPx(pB, 5.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pPanel);
	xgeXuiWidgetAdd(pPanel, pA);
	xgeXuiWidgetAdd(pPanel, pB);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 10.0f || tRect.fW != 50.0f || tRect.fH != 20.0f ) {
		xgeXuiUnit(&tXui);
		return 142;
	}
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fX != 65.0f || tRect.fW != 225.0f || tRect.fH != 30.0f ) {
		xgeXuiUnit(&tXui);
		return 143;
	}

	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pPanel, 0.0f, 10.0f, 0.0f, 10.0f);
	xgeXuiWidgetSetMarginPx(pB, 0.0f, 5.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePx(50.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetSize(pB, xgeXuiSizePx(30.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pB);
	if ( tRect.fY != 35.0f || tRect.fW != 30.0f || tRect.fH != 55.0f ) {
		xgeXuiUnit(&tXui);
		return 144;
	}

	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_STACK);
	xgeXuiWidgetSetPaddingPx(pPanel, 5.0f, 6.0f, 7.0f, 8.0f);
	xgeXuiWidgetSetMarginPx(pA, 1.0f, 2.0f, 3.0f, 4.0f);
	xgeXuiWidgetSetSize(pA, xgeXuiSizePercent(50.0f), xgeXuiSizePercent(50.0f));
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pA);
	if ( tRect.fX != 6.0f || tRect.fY != 8.0f || tRect.fW != 144.0f || tRect.fH != 43.0f ) {
		xgeXuiUnit(&tXui);
		return 145;
	}

	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiEventLog;

static int __testXuiEventChild(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 1;
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 3;
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEventParent(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		g_iXuiEventLog = (g_iXuiEventLog * 10) + 2;
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEventCapture(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		g_iXuiEventLog = 7;
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __testXuiEvents(void)
{
	xge_xui_context_t tXui;
	xge_xui_widget pRoot;
	xge_xui_widget pParent;
	xge_xui_widget pChild;
	xge_xui_widget pHit;
	xge_event_t tEvent;
	xge_rect_t tRect;

	memset(&tXui, 0, sizeof(tXui));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 160;
	}
	pRoot = xgeXuiRoot(&tXui);
	pParent = xgeXuiWidgetCreate();
	pChild = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pParent == NULL) || (pChild == NULL) ) {
		xgeXuiWidgetFree(pParent);
		xgeXuiWidgetFree(pChild);
		xgeXuiUnit(&tXui);
		return 161;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 200.0f;
	tRect.fH = 200.0f;
	xgeXuiWidgetSetRect(pParent, tRect);
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 50.0f;
	tRect.fH = 50.0f;
	xgeXuiWidgetSetRect(pChild, tRect);
	pParent->procEvent = __testXuiEventParent;
	pChild->procEvent = __testXuiEventChild;
	xgeXuiWidgetAdd(pRoot, pParent);
	xgeXuiWidgetAdd(pParent, pChild);
	xgeXuiUpdate(&tXui, 0.0f);

	pHit = xgeXuiHitTest(&tXui, 20.0f, 20.0f);
	if ( pHit != pChild ) {
		xgeXuiUnit(&tXui);
		return 162;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 12 ) {
		xgeXuiUnit(&tXui);
		return 163;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	xgeXuiWidgetSetFocusable(pChild, 1);
	xgeXuiSetFocus(&tXui, pChild);
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 3 ) {
		xgeXuiUnit(&tXui);
		return 164;
	}

	pParent->procEvent = __testXuiEventCapture;
	xgeXuiSetCapture(&tXui, pParent);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 500.0f;
	tEvent.fY = 500.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiEventLog != 7 ) {
		xgeXuiUnit(&tXui);
		return 165;
	}

	xgeXuiSetCapture(&tXui, NULL);
	xgeXuiSetFocus(&tXui, NULL);
	xgeXuiWidgetSetEnabled(pChild, 0);
	pHit = xgeXuiHitTest(&tXui, 20.0f, 20.0f);
	if ( pHit != pParent ) {
		xgeXuiUnit(&tXui);
		return 166;
	}
	xgeXuiWidgetSetEnabled(pChild, 1);
	xgeXuiWidgetSetVisible(pChild, 0);
	pHit = xgeXuiHitTest(&tXui, 20.0f, 20.0f);
	if ( pHit != pParent ) {
		xgeXuiUnit(&tXui);
		return 167;
	}
	xgeXuiWidgetSetVisible(pChild, 1);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 500.0f;
	tEvent.fY = 500.0f;
	g_iXuiEventLog = 0;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || g_iXuiEventLog != 0 ) {
		xgeXuiUnit(&tXui);
		return 168;
	}

	xgeXuiUnit(&tXui);
	return 0;
}

static int g_iXuiButtonClicks;

static void __testXuiButtonClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiButtonClicks++;
}

static int __testXuiButton(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_button_t tButton;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tButton, 0, sizeof(tButton));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 200;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 201;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 202;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 100.0f;
	tRect.fH = 40.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiButtonInit(&tButton, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 203;
	}
	xgeXuiButtonSetClick(&tButton, __testXuiButtonClick, NULL);
	xgeXuiButtonSetText(&tButton, &tFont, "A");
	xgeXuiButtonSetTextColor(&tButton, XGE_COLOR_RGBA(10, 20, 30, 255));
	xgeXuiButtonSetColors(&tButton, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160));
	if ( xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 204;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiButtonPaintProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 205;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiButtonGetState(&tButton) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 206;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiButtonGetState(&tButton) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 207;
	}
	if ( tXui.pFocus != pWidget || tXui.pCapture != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 208;
	}
	g_iXuiButtonClicks = 0;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || g_iXuiButtonClicks != 1 || tButton.iClickCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 209;
	}
	if ( tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 210;
	}

	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiButtonGetState(&tButton) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 211;
	}

	xgeXuiButtonUnit(&tButton);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 212;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiToggleChanges;
static int g_iXuiToggleLastChecked;

static void __testXuiToggleChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiToggleChanges++;
	g_iXuiToggleLastChecked = bChecked;
}

static int __testXuiToggle(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_toggle_t tToggle;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tToggle, 0, sizeof(tToggle));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 260;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 261;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 262;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 120.0f;
	tRect.fH = 32.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiToggleInit(&tToggle, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 263;
	}
	xgeXuiToggleSetChange(&tToggle, __testXuiToggleChange, NULL);
	xgeXuiToggleSetText(&tToggle, &tFont, "A");
	xgeXuiToggleSetTextColor(&tToggle, XGE_COLOR_RGBA(10, 20, 30, 255));
	xgeXuiToggleSetColors(&tToggle, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160), XGE_COLOR_RGBA(16, 17, 18, 255));
	if ( xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 264;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiTogglePaintProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 265;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiToggleGetState(&tToggle) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 266;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || ((xgeXuiToggleGetState(&tToggle) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 267;
	}
	g_iXuiToggleChanges = 0;
	g_iXuiToggleLastChecked = 0;
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiToggleGetChecked(&tToggle) != 1 || g_iXuiToggleChanges != 1 || g_iXuiToggleLastChecked != 1 || tToggle.iChangeCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 268;
	}
	xgeXuiToggleSetChecked(&tToggle, 0);
	if ( xgeXuiToggleGetChecked(&tToggle) != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 269;
	}
	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiToggleGetState(&tToggle) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 270;
	}
	xgeXuiToggleUnit(&tToggle);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 271;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int g_iXuiSliderChanges;
static float g_fXuiSliderLastValue;

static void __testXuiSliderChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iXuiSliderChanges++;
	g_fXuiSliderLastValue = fValue;
}

static int __testXuiSlider(void)
{
	xge_xui_context_t tXui;
	xge_xui_slider_t tSlider;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tSlider, 0, sizeof(tSlider));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 280;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 281;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 110.0f;
	tRect.fH = 30.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 5.0f, 4.0f, 5.0f, 4.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiSliderInit(&tSlider, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 282;
	}
	xgeXuiSliderSetChange(&tSlider, __testXuiSliderChange, NULL);
	xgeXuiSliderSetRange(&tSlider, 0.0f, 100.0f);
	xgeXuiSliderSetValue(&tSlider, 25.0f);
	xgeXuiSliderSetColors(&tSlider, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255), XGE_COLOR_RGBA(13, 14, 15, 160));
	if ( xgeXuiSliderGetValue(&tSlider) != 25.0f || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		return 283;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiSliderPaintProc ) {
		xgeXuiUnit(&tXui);
		return 284;
	}

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 65.0f;
	tEvent.fY = 20.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiSliderGetState(&tSlider) & XGE_XUI_STATE_HOVER) == 0) ) {
		xgeXuiUnit(&tXui);
		return 285;
	}
	g_iXuiSliderChanges = 0;
	g_fXuiSliderLastValue = 0.0f;
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSliderGetValue(&tSlider) != 50.0f || g_iXuiSliderChanges != 1 || g_fXuiSliderLastValue != 50.0f ) {
		xgeXuiUnit(&tXui);
		return 286;
	}
	if ( tXui.pFocus != pWidget || tXui.pCapture != pWidget || ((xgeXuiSliderGetState(&tSlider) & XGE_XUI_STATE_ACTIVE) == 0) ) {
		xgeXuiUnit(&tXui);
		return 287;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 115.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiSliderGetValue(&tSlider) != 100.0f || g_iXuiSliderChanges != 2 || g_fXuiSliderLastValue != 100.0f ) {
		xgeXuiUnit(&tXui);
		return 288;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL ) {
		xgeXuiUnit(&tXui);
		return 289;
	}
	xgeXuiSliderSetRange(&tSlider, 100.0f, 0.0f);
	xgeXuiSliderSetValue(&tSlider, -20.0f);
	if ( xgeXuiSliderGetValue(&tSlider) != 0.0f ) {
		xgeXuiUnit(&tXui);
		return 290;
	}
	xgeXuiWidgetSetEnabled(pWidget, 0);
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONTINUE || ((xgeXuiSliderGetState(&tSlider) & XGE_XUI_STATE_DISABLED) == 0) ) {
		xgeXuiUnit(&tXui);
		return 291;
	}
	xgeXuiSliderUnit(&tSlider);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 292;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiProgress(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_progress_t tProgress;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tProgress, 0, sizeof(tProgress));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 300;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 301;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 302;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 120.0f;
	tRect.fH = 20.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiProgressInit(&tProgress, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 303;
	}
	xgeXuiProgressSetRange(&tProgress, 100.0f, 0.0f);
	xgeXuiProgressSetValue(&tProgress, 60.0f);
	xgeXuiProgressSetColors(&tProgress, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255));
	xgeXuiProgressSetText(&tProgress, &tFont, "A");
	xgeXuiProgressSetTextColor(&tProgress, XGE_COLOR_RGBA(7, 8, 9, 255));
	if ( xgeXuiProgressGetValue(&tProgress) != 60.0f || tProgress.fMin != 0.0f || tProgress.fMax != 100.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 304;
	}
	xgeXuiProgressSetValue(&tProgress, 200.0f);
	if ( xgeXuiProgressGetValue(&tProgress) != 100.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 305;
	}
	xgeXuiProgressSetValue(&tProgress, -10.0f);
	if ( xgeXuiProgressGetValue(&tProgress) != 0.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 306;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiProgressPaintProc || pWidget->procEvent != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 307;
	}
	if ( tProgress.pFont != &tFont || tProgress.iTextColor != XGE_COLOR_RGBA(7, 8, 9, 255) || tProgress.iColorFill != XGE_COLOR_RGBA(4, 5, 6, 255) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 308;
	}
	xgeXuiProgressUnit(&tProgress);
	if ( pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 309;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiPanel(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_panel_t tPanel;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tPanel, 0, sizeof(tPanel));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 320;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 321;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 322;
	}
	tRect.fX = 4.0f;
	tRect.fY = 6.0f;
	tRect.fW = 120.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 4.0f, 5.0f, 6.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiPanelInit(&tPanel, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 323;
	}
	xgeXuiPanelSetBackground(&tPanel, XGE_COLOR_RGBA(1, 2, 3, 255));
	xgeXuiPanelSetTitle(&tPanel, &tFont, "A");
	xgeXuiPanelSetTitleColor(&tPanel, XGE_COLOR_RGBA(4, 5, 6, 255));
	xgeXuiPanelSetTitleAlign(&tPanel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_TOP);
	xgeXuiPanelSetClip(&tPanel, 1);
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || tPanel.bClip != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 324;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiPanelPaintProc || pWidget->procEvent != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 325;
	}
	if ( tPanel.pFont != &tFont || tPanel.iBackgroundColor != XGE_COLOR_RGBA(1, 2, 3, 255) || tPanel.iTitleColor != XGE_COLOR_RGBA(4, 5, 6, 255) || ((tPanel.iTitleFlags & XGE_TEXT_CLIP) == 0) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 326;
	}
	xgeXuiPanelSetClip(&tPanel, 0);
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) != 0 || tPanel.bClip != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 327;
	}
	xgeXuiPanelUnit(&tPanel);
	if ( pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 328;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiScrollView(void)
{
	xge_xui_context_t tXui;
	xge_xui_scroll_view_t tScroll;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	float fX;
	float fY;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tScroll, 0, sizeof(tScroll));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 340;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 341;
	}
	tRect.fX = 10.0f;
	tRect.fY = 10.0f;
	tRect.fW = 100.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 5.0f, 5.0f, 5.0f, 5.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiScrollViewInit(&tScroll, &tXui, pWidget) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 342;
	}
	xgeXuiScrollViewSetContentSize(&tScroll, 220.0f, 210.0f);
	xgeXuiScrollViewSetColors(&tScroll, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255));
	if ( (xgeXuiWidgetGetFlags(pWidget) & XGE_XUI_WIDGET_CLIP) == 0 || xgeXuiWidgetIsFocusable(pWidget) == 0 ) {
		xgeXuiUnit(&tXui);
		return 343;
	}
	xgeXuiScrollViewSetOffset(&tScroll, 500.0f, 500.0f);
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 130.0f || fY != 140.0f ) {
		xgeXuiUnit(&tXui);
		return 344;
	}
	xgeXuiScrollViewSetOffset(&tScroll, 0.0f, 0.0f);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_WHEEL;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	tEvent.fDY = -1.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 345;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 0.0f || fY != 32.0f ) {
		xgeXuiUnit(&tXui);
		return 346;
	}
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 30.0f;
	tEvent.fY = 30.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != pWidget || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		return 347;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = 20.0f;
	tEvent.fY = 10.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		xgeXuiUnit(&tXui);
		return 348;
	}
	xgeXuiScrollViewGetOffset(&tScroll, &fX, &fY);
	if ( fX != 10.0f || fY != 52.0f ) {
		xgeXuiUnit(&tXui);
		return 349;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pCapture != NULL || tScroll.bDragging != 0 ) {
		xgeXuiUnit(&tXui);
		return 350;
	}
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiScrollViewPaintProc || pWidget->procEvent != xgeXuiScrollViewEventProc ) {
		xgeXuiUnit(&tXui);
		return 351;
	}
	xgeXuiScrollViewUnit(&tScroll);
	if ( pWidget->procEvent != NULL || pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 352;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiLabel(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_label_t tLabel;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_vec2_t tSize;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tLabel, 0, sizeof(tLabel));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 220;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 221;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 222;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 80.0f;
	tRect.fH = 30.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 3.0f, 4.0f, 5.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiLabelInit(&tLabel, pWidget, &tFont, "A") != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 223;
	}
	tSize = xgeXuiLabelMeasure(&tLabel);
	if ( tSize.fX != 8.0f || tSize.fY != 14.0f ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 224;
	}
	xgeXuiLabelSetText(&tLabel, "A\xe4\xb8\xad");
	xgeXuiLabelSetColor(&tLabel, XGE_COLOR_RGBA(1, 2, 3, 255));
	xgeXuiLabelSetAlign(&tLabel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiUpdate(&tXui, 0.0f);
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 225;
	}
	if ( (tLabel.iTextFlags & XGE_TEXT_CLIP) == 0 || tLabel.iColor != XGE_COLOR_RGBA(1, 2, 3, 255) ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 226;
	}
	xgeXuiLabelUnit(&tLabel);
	if ( pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 227;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiImage(void)
{
	xge_xui_context_t tXui;
	xge_xui_image_t tImage;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_texture_t tTexture;
	xge_rect_t tRect;
	int iPaintCount;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tImage, 0, sizeof(tImage));
	memset(&tTexture, 0, sizeof(tTexture));
	tTexture.iWidth = 16;
	tTexture.iHeight = 8;
	tTexture.iFormat = XGE_PIXEL_RGBA8;
	tTexture.iRefCount = 1;
	tTexture.iBackendId = 0;
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 230;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		return 231;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 40.0f;
	tRect.fH = 30.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 3.0f, 4.0f, 5.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiImageInit(&tImage, pWidget, &tTexture) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 232;
	}
	tRect.fX = 1.0f;
	tRect.fY = 2.0f;
	tRect.fW = 8.0f;
	tRect.fH = 4.0f;
	xgeXuiImageSetSource(&tImage, tRect);
	xgeXuiImageSetColor(&tImage, XGE_COLOR_RGBA(1, 2, 3, 128));
	xgeXuiImageSetMode(&tImage, XGE_XUI_IMAGE_FIT);
	xgeXuiUpdate(&tXui, 0.0f);
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiImagePaintProc ) {
		xgeXuiUnit(&tXui);
		return 233;
	}
	if ( tImage.iMode != XGE_XUI_IMAGE_FIT || tImage.iColor != XGE_COLOR_RGBA(1, 2, 3, 128) || tImage.tSrc.fW != 8.0f ) {
		xgeXuiUnit(&tXui);
		return 234;
	}
	xgeXuiImageSetTexture(&tImage, NULL);
	if ( tImage.pTexture != NULL ) {
		xgeXuiUnit(&tXui);
		return 235;
	}
	xgeXuiImageUnit(&tImage);
	if ( pWidget->procPaint != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		return 236;
	}
	xgeXuiUnit(&tXui);
	return 0;
}

static int __testXuiInput(void)
{
	xrf_test_blob2_t tBlob;
	xge_font_t tFont;
	xge_xui_context_t tXui;
	xge_xui_input_t tInput;
	xge_xui_widget pRoot;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iPaintCount;

	__testXrfBlobMake(&tBlob);
	memset(&tFont, 0, sizeof(tFont));
	memset(&tXui, 0, sizeof(tXui));
	memset(&tInput, 0, sizeof(tInput));
	if ( xgeFontLoadXRFMemory(&tFont, &tBlob, sizeof(tBlob)) != XGE_OK ) {
		return 240;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeFontFree(&tFont);
		return 241;
	}
	pRoot = xgeXuiRoot(&tXui);
	pWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		xgeXuiWidgetFree(pWidget);
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 242;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 160.0f;
	tRect.fH = 80.0f;
	xgeXuiWidgetSetRect(pRoot, tRect);
	tRect.fX = 10.0f;
	tRect.fY = 12.0f;
	tRect.fW = 96.0f;
	tRect.fH = 24.0f;
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 2.0f, 3.0f, 2.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiInputInit(&tInput, &tXui, pWidget, &tFont) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 243;
	}
	xgeXuiInputSetColors(&tInput, XGE_COLOR_RGBA(1, 2, 3, 255), XGE_COLOR_RGBA(4, 5, 6, 255), XGE_COLOR_RGBA(7, 8, 9, 255), XGE_COLOR_RGBA(10, 11, 12, 255));
	xgeXuiUpdate(&tXui, 0.0f);
	iPaintCount = xgeXuiPaint(&tXui);
	if ( iPaintCount != 1 || tXui.iPaintCommandCount != 1 || pWidget->procPaint != xgeXuiInputPaintProc || pWidget->procEvent != xgeXuiInputEventProc ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 244;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.fX = 12.0f;
	tEvent.fY = 14.0f;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || tXui.pFocus != pWidget ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 245;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'A';
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "A") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 246;
	}
	tEvent.iCodepoint = 0x4E2D;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "A\xe4\xb8\xad") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 247;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_BACKSPACE;
	if ( xgeXuiDispatchEvent(&tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || strcmp(xgeXuiInputGetText(&tInput), "A") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 248;
	}
	xgeXuiInputSetText(&tInput, "B");
	xgeXuiInputSetFont(&tInput, &tFont);
	if ( strcmp(xgeXuiInputGetText(&tInput), "B") != 0 ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 249;
	}
	xgeXuiInputUnit(&tInput);
	if ( pWidget->procPaint != NULL || pWidget->procEvent != NULL || pWidget->pUser != NULL ) {
		xgeXuiUnit(&tXui);
		xgeFontFree(&tFont);
		return 250;
	}
	xgeXuiUnit(&tXui);
	xgeFontFree(&tFont);
	return 0;
}

static int __testXuiTextEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiTextInputEvent((xge_xui_text)pUser, pEvent);
}

static int __testXuiTextInput(void)
{
	xge_xui_context_t tXui;
	xge_xui_text_t tText;
	xge_xui_widget pRoot;
	xge_xui_widget pInput;
	xge_event_t tEvent;
	int iRet;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tText, 0, sizeof(tText));
	if ( xgeXuiTextInit(&tText) != XGE_OK ) {
		return 180;
	}
	if ( xgeXuiTextSet(&tText, "A") != XGE_OK ) {
		xgeXuiTextUnit(&tText);
		return 181;
	}
	if ( xgeXuiTextInsertCodepoint(&tText, 0x4E2D) != XGE_OK || strcmp(tText.sText, "A\xe4\xb8\xad") != 0 ) {
		xgeXuiTextUnit(&tText);
		return 182;
	}
	if ( xgeXuiTextDeleteBack(&tText) != XGE_OK || strcmp(tText.sText, "A") != 0 ) {
		xgeXuiTextUnit(&tText);
		return 183;
	}
	tText.iSelectStart = 0;
	tText.iSelectEnd = tText.iSize;
	if ( xgeXuiTextInsert(&tText, "B") != XGE_OK || strcmp(tText.sText, "B") != 0 ) {
		xgeXuiTextUnit(&tText);
		return 184;
	}

	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeXuiTextUnit(&tText);
		return 185;
	}
	pRoot = xgeXuiRoot(&tXui);
	pInput = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pInput == NULL) ) {
		xgeXuiWidgetFree(pInput);
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 186;
	}
	pInput->procEvent = __testXuiTextEventProc;
	pInput->pUser = &tText;
	xgeXuiWidgetSetFocusable(pInput, 1);
	xgeXuiWidgetAdd(pRoot, pInput);
	xgeXuiSetFocus(&tXui, pInput);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = 'C';
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONSUMED) || (strcmp(tText.sText, "BC") != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 187;
	}
	tEvent.iCodepoint = 0x4E2D;
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONSUMED) || (strcmp(tText.sText, "BC\xe4\xb8\xad") != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 188;
	}
	tEvent.iCodepoint = 0x1F600;
	iRet = xgeXuiDispatchEvent(&tXui, &tEvent);
	if ( (iRet != XGE_XUI_EVENT_CONTINUE) || (strcmp(tText.sText, "BC\xe4\xb8\xad") != 0) ) {
		xgeXuiUnit(&tXui);
		xgeXuiTextUnit(&tText);
		return 189;
	}
	xgeXuiUnit(&tXui);
	xgeXuiTextUnit(&tText);
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

	iRet = __testXRFMemory();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiIncubationBase();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiLayoutModes();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiEvents();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiTextInput();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiButton();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiToggle();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiSlider();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiProgress();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiPanel();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiScrollView();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiLabel();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiImage();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	iRet = __testXuiInput();
	if ( iRet != 0 ) {
		xgeUnit();
		return iRet;
	}

	xgeUnit();
	return 0;
}

